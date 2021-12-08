/*
 * main.cpp
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "batteryInterface.h"
#include "imuInterface.h"
#include "bcm2835.h"
#include "servoControl.h"
#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
	ImuInterface::GetInstance()->attachIMU();
	cout << "MAIN | main | IMU attached" << endl;
	ImuInterface::GetInstance()->startLoop();
	cout << "MAIN | main | IMU Loop started" << endl;

	BatteryInterface::GetInstance()->attachINA226(0x44);
	cout << "MAIN | main | INA226 attached" << endl;
	BatteryInterface::GetInstance()->startLoop();
	cout << "MAIN | main | INA226 Loop started" << endl;

	if (!bcm2835_init()) {
			cerr << "MAIN | main | failed to open I2C device" << endl;
	} else {
			cout << "MAIN | main | bcm2835 initialized, version: " << bcm2835_version() << endl;
	}

	ServoControl::GetInstance();
	cout << "ServoControl created" << endl;

	while (true) {
		cout << "GPS H: " << ImuInterface::GetInstance()->getD1Status() << endl;
		cout << "GPS H: " << ImuInterface::GetInstance()->getGPSH() << endl;
		cout << "GPS V: " << ImuInterface::GetInstance()->getGPSV() << endl;
		cout << "GPS Y: " << ImuInterface::GetInstance()->getGPSY() << endl;
		cout << endl;
		cout << "Voltage: " << BatteryInterface::GetInstance()->getVoltage() << endl;
		cout << "Current: " << BatteryInterface::GetInstance()->getCurrent() << endl;

		ServoControl::GetInstance()->testServo();
	}
	return 1;
}
