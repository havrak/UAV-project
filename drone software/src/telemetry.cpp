/*
 * telemetry.cpp
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "telemetry.h"
#include "communication_interface.h"
#include "protocol_spec.h"
#include "servo_control.h"
#include <cstring>
#include <iterator>

Telemetry* Telemetry::telemetry = nullptr;
mutex Telemetry::telemetryMutex;

Telemetry::Telemetry()
{
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
bool Telemetry::setUpSensors(int imuAddress, int inaAddress, int pca9685Address)
{
	/* this->imuAddress = imuAddress; */
	/* this->inaAddress = inaAddress; */
	/* this->pca9685Address = pca9685Address; */
	/* ImuInterface::GetInstance()->attachIMU(imuAddress); */
	/* cout << "MAIN | main | IMU attached\n"; */
	/* BatteryInterface::GetInstance()->attachINA226(inaAddress); */
	/* cout << "MAIN | main | INA226 attached\n"; */
	/* BatteryInterface::GetInstance()->startLoop(); */
	/* cout << "MAIN | main | loop started\n"; */
	/* GPSInterface::GetInstance()->attachGPS(); */
	/* cout << "MAIN | main | GPS attached\n"; */
	/* GPSInterface::GetInstance()->startLoop(); */
	/* cout << "MAIN | main | GPS Loop started\n"; */
	/* return checkPeripheriesStatus(); */
}

bool Telemetry::checkPeripheriesStatus()
{
	return 1;

	char buffer[128];
	std::string result = "";
	FILE* pipe = popen("echo $(sudo i2cdetect -y 1 2>/dev/null | tail -7 | cut -d':' -f2 | sed 's/[^0-9]*\\(.\\)/\\1/g')", "r");
	if (!pipe) {
		cerr << "Telemetry | checkSensorStatus | popen() failed!\n";
		return false;
	}
	try {
		while (fgets(buffer, sizeof buffer, pipe) != NULL) {
			result += buffer;
		}
	} catch (...) {
		pclose(pipe);
		cerr << "Telemetry | checkSensorStatus | reading failed!\n";
		return false;
	}
	// 40, 50, 44

	int tmp;


	pclose(pipe);
}

pTeleATT Telemetry::createTeleAttStruct()
{
	/* ImuInterface* instance = ImuInterface::GetInstance(); */
	/* pTeleATT toReturn(instance->getYaw(), instance->getPitch(), instance->getRoll(), instance->getAccX(), instance->getAccY(), instance->getAccZ(), instance->getGyroX(), instance->getGyroY(), instance->getGyroZ(), instance->getMagX(), instance->getMagY(), instance->getMagZ(), instance->getPressure(), instance->getTemp()); */
	return pTeleATT();
}

pTeleGPS Telemetry::createTeleGPSStruct()
{
	/* GPSInterface* instance = GPSInterface::GetInstance(); */
	/* pTeleGPS toReturn(instance->getGPSStatus(), instance->getAltitude(), instance->getLon(), instance->getLat(), instance->getGroundSpeed(), instance->getHeading(), instance->getNOS()); */
	return pTeleGPS();
}

pTeleBATT Telemetry::createTeleBattStuct()
{
	/* BatteryInterface* instance = BatteryInterface::GetInstance(); */
	/* pTeleBATT toReturn(instance->getVoltage(), instance->getCurrent(), instance->getPower(), instance->getEnergy(), instance->getShunt()); */
	return pTeleBATT();
}

pTeleIOStat Telemetry::createTeleIOStatStruct()
{
	/* pTeleIOStat toReturn(BatteryInterface::GetInstance()->getINAStatus(), ServoControl::GetInstance()->getPCA9865Status(), ImuInterface::GetInstance()->getIMUStatus(), GPSInterface::GetInstance()->getGPSStatus()); */
	return pTeleIOStat();
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

bool Telemetry::processGeneralTelemetryRequest(const Client cli)
{
	pTeleGen data;
	data.att = createTeleAttStruct();
	data.gps = createTeleGPSStruct();
	data.batt = createTeleBattStuct();
	data.io = createTeleIOStatStruct();
	data.pwm = createTelePWMStruct();
	SendingStructure ss(cli.fd, cli.cMutex, P_TELE_GEN, 0x01, sizeof(data));
	memcpy(ss.getMessageBuffer(), &data, sizeof(data));
	SendingThreadPool::GetInstance()->scheduleToSend(ss);
	return true;
}

bool Telemetry::processAttGPSRequest(const Client cli)
{
	pTeleATTGPS data;
	data.att = createTeleAttStruct();
	data.gps = createTeleGPSStruct();
	SendingStructure ss(cli.fd, cli.cMutex, P_TELE_ATTGPS, 0x01, sizeof(data));
	memcpy(ss.getMessageBuffer(), &data, sizeof(data));
	SendingThreadPool::GetInstance()->scheduleToSend(ss);
	return true;
}
bool Telemetry::processBatteryRequest(const Client cli)
{
	pTeleBATT data = createTeleBattStuct();
	SendingStructure ss(cli.fd, cli.cMutex, P_TELE_BATT, 0x01, sizeof(data));
	memcpy(ss.getMessageBuffer(), &data, sizeof(data));
	SendingThreadPool::GetInstance()->scheduleToSend(ss);
	return true;
}

bool Telemetry::processPWMRequest(const Client cli)
{
	pTelePWM data = createTelePWMStruct();
	SendingStructure ss(cli.fd, cli.cMutex, P_TELE_PWM, 0x01, sizeof(data));
	memcpy(ss.getMessageBuffer(), &data, sizeof(data));
	SendingThreadPool::GetInstance()->scheduleToSend(ss);
	return true;
}
bool Telemetry::processIORequest(const Client cli)
{
	pTeleIOStat data = createTeleIOStatStruct();
	SendingStructure ss(cli.fd, cli.cMutex, P_TELE_IOSTAT, 0x01, sizeof(data));
	memcpy(ss.getMessageBuffer(), &data, sizeof(data));
	SendingThreadPool::GetInstance()->scheduleToSend(ss);
	return true;
}
