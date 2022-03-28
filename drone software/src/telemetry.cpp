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
#include <iterator>

Telemetry* Telemetry::telemetry = nullptr;
mutex Telemetry::telemetryMutex;

Telemetry::Telemetry(){
}

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
bool Telemetry::setUpSensors()
{
	ImuInterface::GetInstance()->attachIMU();
	cout << "MAIN | main | IMU attached\n";
	BatteryInterface::GetInstance()->attachINA226();
	cout << "MAIN | main | INA226 attached\n";
	BatteryInterface::GetInstance()->startLoop();
	cout << "MAIN | main | loop started\n";
	GPSInterface::GetInstance()->attachGPS();
	cout << "MAIN | main | GPS attached\n";
	GPSInterface::GetInstance()->startLoop();
	cout << "MAIN | main | GPS Loop started\n";
	return 1;
}

pTeleATT Telemetry::createTeleAttStruct()
{
	ImuInterface* instance = ImuInterface::GetInstance();
	pTeleATT toReturn(instance->getYaw(), instance->getPitch(), instance->getRoll(), instance->getAccX(),instance->getAccY(),instance->getAccZ(),instance->getGyroX(),instance->getGyroY(),instance->getGyroZ(),instance->getMagX(),instance->getMagY(),instance->getMagZ(),instance->getPressure(),instance->getTemp()
	);
	return toReturn;
}

pTeleGPS Telemetry::createTeleGPSStruct()
{
	GPSInterface* instance = GPSInterface::GetInstance();
	pTeleGPS toReturn( instance->getAltitude(), instance->getLon(), instance->getLat(), instance->getNOS(), instance->getGPSStatus());
	return toReturn;
}

pTeleBATT Telemetry::createTeleBattStuct()
{
	BatteryInterface* instance = BatteryInterface::GetInstance();
	pTeleBATT toReturn(instance->getVoltage(), instance->getCurrent(),  instance->getPower(), instance->getEnergy(),instance->getShunt());
	return toReturn;
}

pTeleIOStat Telemetry::createTeleIOStatStruct()
{
	pTeleIOStat toReturn(BatteryInterface::GetInstance()->getINAStatus(), ServoControl::GetInstance()->getPCA9865Status(), ImuInterface::GetInstance()->getIMUStatus(), GPSInterface::GetInstance()->getGPSStatus());
	return toReturn;
}

pTelePWM Telemetry::createTelePWMStruct()
{
	pTelePWM toReturn;

	ServoControl* instance = ServoControl::GetInstance();
	toReturn.motorMS = instance->getMainMotorMS();
	pair<int, unsigned char*> tmp = instance->getControlSurfaceConfiguration();
	toReturn.configuration = tmp.first;
	memcpy(&toReturn.angle, &tmp.second, 16);
	return toReturn;
}


bool Telemetry::processGeneralTelemetryRequest(const client cli)
{
	pTeleGen data;
	data.att = createTeleAttStruct();
	data.gps = createTeleGPSStruct();
	data.batt = createTeleBattStuct();
	data.io = createTeleIOStatStruct();
	data.pwm = createTelePWMStruct();
	cout << "yaw: " << data.att.yaw << "\npitch: " << data.att.pitch << "\nroll: " << data.att.roll << "\nacc: "<< data.att.accX << "\ntemp:"<< data.att.temp << "\nvoltate: " << data.batt.voltage << "\ncurrent: " << data.batt.current << "\n\n";
	/* cout << "\nacc: "<< data.att.accX << "\ntemp:"<< data.att.temp << "\nvoltate: " << data.batt.voltage << "\ncurrent: " << data.batt.current << "\n\n"; */

	SendingStructure ss(cli.fd, cli.cMutex, P_TELE_GEN, 0x01, sizeof(data));
	memcpy(ss.getMessageBuffer(), &data, sizeof(data));
	SendingThreadPool::GetInstance()->scheduleToSend(ss);
	return true;
}

bool Telemetry::processAttGPSRequest(const client cli)
{
	pTeleATTGPS data;
	data.att = createTeleAttStruct();
	data.gps = createTeleGPSStruct();
	SendingStructure ss(cli.fd, cli.cMutex, P_TELE_ATTGPS, 0x01, sizeof(data));
	memcpy(ss.getMessageBuffer(), &data, sizeof(data));
	SendingThreadPool::GetInstance()->scheduleToSend(ss);
	return true;
}
bool Telemetry::processBatteryRequest(const client cli)
{
	pTeleBATT data = createTeleBattStuct();
	SendingStructure ss(cli.fd, cli.cMutex, P_TELE_BATT, 0x01, sizeof(data));
	memcpy(ss.getMessageBuffer(), &data, sizeof(data));
	SendingThreadPool::GetInstance()->scheduleToSend(ss);
	return true;
}

bool Telemetry::processPWMRequest(const client cli)
{
	pTelePWM data = createTelePWMStruct();
	SendingStructure ss(cli.fd, cli.cMutex, P_TELE_PWM, 0x01, sizeof(data));
	memcpy(ss.getMessageBuffer(), &data, sizeof(data));
	SendingThreadPool::GetInstance()->scheduleToSend(ss);
	return true;
}
bool Telemetry::processIORequest(const client cli)
{
	pTeleIOStat data = createTeleIOStatStruct();
	SendingStructure ss(cli.fd, cli.cMutex, P_TELE_IOSTAT, 0x01, sizeof(data));
	memcpy(ss.getMessageBuffer(), &data, sizeof(data));
	SendingThreadPool::GetInstance()->scheduleToSend(ss);
	return true;
}
