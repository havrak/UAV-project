/*
 * main.cpp
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "imuInterface.h"
#include "servoControl.h"
#include <iostream>
#include <wiringPi.h>
using namespace std;

int main(int argc, char** argv)
{
	//wiringPiSetup();
	ImuInterface::GetInstance()->attachIMU();
	cout << "Imu attached" << endl;
	//ImuInterface::GetInstance()->startLoop();
	cout << "Loop started" << endl;

	if (!bcm2835_init()) {
			cerr << "MAIN | main | failed to open I2C device" << endl;
	} else {
			cout << "MAIN | main | bcm2835 initialized, version: " << bcm2835_version() << endl;
	}

	ServoControl::GetInstance();
	cout << "ServoControl created" << endl;

	while (true) {
		cout << "GPS H: " << ImuInterface::GetInstance()->getGPSH() << endl;
		cout << "GPS V: " << ImuInterface::GetInstance()->getGPSV() << endl;
		cout << "GPS Y: " << ImuInterface::GetInstance()->getGPSY() << endl;
		ServoControl::GetInstance()->testServo();
		this_thread::sleep_for(chrono::milliseconds(1000));
	}
	return 1;
}
