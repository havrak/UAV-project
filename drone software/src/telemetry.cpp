/*
 * telemetry.cpp
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "telemetry.h"
#include "battery_interface.h"
#include "communication_interface.h"
#include "gps_interface.h"
#include "imu_interface.h"
#include "protocol_spec.h"
#include "servo_control.h"
#include <cstring>

Telemetry* Telemetry::telemetry = nullptr;
mutex Telemetry::telemetryMutex;

Telemetry* Telemetry::GetInstance()
{
	if (telemetry == nullptr) {
		telemetryMutex.lock();
		if (telemetry == nullptr) {
			telemetry = new Telemetry();
		}
		telemetryMutex.unlock();
	}
	return telemetry;
}

// TODO; make sure everything generates propper messages
int Telemetry::setUpSensors()
{
	ImuInterface::GetInstance()->attachIMU();
	cout << "MAIN | main | IMU attached" << endl;
	BatteryInterface::GetInstance()->attachINA226();
	cout << "MAIN | main | INA226 attached" << endl;
	BatteryInterface::GetInstance()->startLoop();
	cout << "MAIN | main | loop started" << endl;
	GPSInterface::GetInstance()->attachGPS();
	cout << "MAIN | main | GPS attached" << endl;
	GPSInterface::GetInstance()->startLoop();
	cout << "MAIN | main | GPS Loop started" << endl;
	return 1;
}

pTeleATT Telemetry::createTeleAttStruct()
{
	pTeleATT toReturn;
	ImuInterface* instance = ImuInterface::GetInstance();
	toReturn.accX = instance->getAccX();
	toReturn.accY = instance->getAccY();
	toReturn.accZ = instance->getAccZ();
	toReturn.gyroX = instance->getGyroX();
	toReturn.gyroY = instance->getGyroY();
	toReturn.gyroZ = instance->getGyroZ();
	toReturn.magX = instance->getMagX();
	toReturn.magY = instance->getMagY();
	toReturn.magZ = instance->getMagZ();
	toReturn.pressure = instance->getPressure();
	toReturn.temp = instance->getTemp();
	return toReturn;
}

pTeleGPS Telemetry::createTeleGPSStruct()
{
	pTeleGPS toReturn;
	GPSInterface* instance = GPSInterface::GetInstance();
	toReturn.altitude = instance->getAltitude();
	toReturn.longitude = instance->getLon();
	toReturn.latitude = instance->getLan();
	toReturn.numberOfSatelites = instance->getNOS();
	toReturn.gpsUp = instance->getGPSStatus();
	return toReturn;
}

pTeleBATT Telemetry::createTeleBattStuct()
{
	pTeleBATT toReturn;
	BatteryInterface* instance = BatteryInterface::GetInstance();
	toReturn.getCurrent = instance->getCurrent();
	toReturn.getEnergy = instance->getEnergy();
	toReturn.getPower = instance->getPower();
	toReturn.getShunt = instance->getShunt();
	toReturn.getVoltage = instance->getVoltage();
	return toReturn;
}

pTeleIOStat Telemetry::createTeleIOStatStruct()
{
	pTeleIOStat toReturn;
	toReturn.gps = GPSInterface::GetInstance()->getGPSStatus();
	toReturn.ina226 = BatteryInterface::GetInstance()->getINAStatus();
	toReturn.wt901 = ImuInterface::GetInstance()->getIMUStatus();
	toReturn.pca9685 = ServoControl::GetInstance()->getPCA9865Status();
	return toReturn;
}

pTelePWM Telemetry::createTelePWMStruct()
{
	pTelePWM toReturn;

	ServoControl* instance = ServoControl::GetInstance();
	toReturn.motorMS = instance->getMainMotorMS();
	pair<int, unsigned int short*> tmp = instance->getControlSurfaceConfiguration();
	toReturn.configuration = tmp.first;
	memcpy(&toReturn.angle, &tmp.second, 16);
}

int Telemetry::processGeneralTelemetryRequest(client* cli)
{
	pTeleGen toSend;
	toSend.att = createTeleAttStruct();
	toSend.gps = createTeleGPSStruct();
	toSend.batt = createTeleBattStuct();
	toSend.io = createTeleIOStatStruct();
	sendingStruct ss;
	ss.MessagePriority = 0x01;
	ss.MessageType = P_TELE_GEN;
	ss.cli = cli;
	SendingThreadPool::GetInstance()->scheduleToSend(ss);
}

int Telemetry::processAttGPSRequest(client* cli)
{
}
int Telemetry::processBatteryRequest(client* cli)
{
}

int Telemetry::processPWMRequest(client* cli)
{
}
int Telemetry::processIORequest(client* cli)
{
}
