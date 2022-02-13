/*
 * telemetry.cpp
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "telemetry.h"
#include "imu_interface.h"
#include "protocol_spec.h"

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
	BatteryInterface::GetInstance()->attachINA226(0x44);
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

	return toReturn;
}
pTeleGPS Telemetry::createTeleGPSStruct()
{
}
pTeleBATT Telemetry::createTeleBattStuct()
{
}
pTeleIOStat Telemetry::createTeleIOStatStruct()
{
}
pTelePWM Telemetry::createTelePWMStruct()
{
}
int Telemetry::processGeneralTelemetryRequest(client* cli)
{
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
