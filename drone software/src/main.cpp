/*
 * main.cpp
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "attitudeReader.h"
#include "imuInterface.h"
#include <iostream>
#include <wiringPi.h>
using namespace std;

int main(int argc, char** argv)
{
	//wiringPiSetup();
	ImuInterface::GetInstance()->attachIMU();
	cout << "Imu attached" << endl;
	ImuInterface::GetInstance()->startLoop();
	cout << "Loop started" << endl;

	while (true) {
		cout << "X: " << endl;
		cout << ImuInterface::GetInstance()->getGPSH() << endl;
		cout << "Y: " << endl;
		cout << ImuInterface::GetInstance()->getGPSV() << endl;
		cout << "Z: " << endl;
		cout << ImuInterface::GetInstance()->getGPSY() << endl;
		this_thread::sleep_for(chrono::milliseconds(100));
	}
	return 1;
}
