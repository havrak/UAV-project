/*
 * telemetry.cpp
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "telemetry.h"


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

//TODO; make sure everything generates propper messages
int Telemetry::setUpSensors(){
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


