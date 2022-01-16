/*
 * main.cpp
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "battery_interface.h"
#include "bcm2835.h"
#include "camera_streamer.h"
#include "gps_interface.h"
#include "imu_interface.h"
#include "servo_control.h"
#include <cstring>
#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
	if (argc > 1){
		if(strcmp(argv[1], "-r") == 0){
			cout << "MAIN | main | reseting ServoControl" << endl;
			ServoControl::GetInstance();
		}
		return 1;
	}
	/* ImuInterface::GetInstance()->attachIMU(); */
	/* cout << "MAIN | main | IMU attached" << endl; */

	BatteryInterface::GetInstance()->attachINA226(0x44);
	cout << "MAIN | main | INA226 attached" << endl;
	BatteryInterface::GetInstance()->startLoop();
	cout << "MAIN | main | INA226 Loop started" << endl;

	/* GPSInterface::GetInstance()->attachGPS(); */
	/* cout << "MAIN | main | GPS attached" << endl; */
	/* GPSInterface::GetInstance()->startLoop(); */
	/* cout << "MAIN | main | GPS Loop started" << endl; */

	if (!bcm2835_init()) {
		cerr << "MAIN | main | failed to open I2C device" << endl;
	} else {
		cout << "MAIN | main | bcm2835 initialized, version: " << bcm2835_version() << endl;
	}
	/* cout << "MAIN | main | Setting up CAMERA_STREAMER" << endl; */
	/* CameraStreamer *cs1 = new CameraStreamer(0, 5000, "192.168.6.11"); */
	/* cs1->setupStream(); */
	/* cout << "MAIN | main | CAMERA_STREAMER setted up" << endl; */

	//IMPORTANT: ALWAYS KILL PROGRAMM WHEN MOTOR IS TURNED OFF, OTHERWISE ESC GOES CRAZY
	ServoControl::GetInstance();
	// cout << "MAIN | main | ServoControl created" << endl;

	// cout << "MAIN | main | servo Calibrating"
	//ServoControl::GetInstance()->calibrateESC();
	cout << "MAIN | main | entering main loop" << endl;
	nanosleep((const struct timespec[]) { { 0, 500000000L } }, NULL);

	while (true) {

		/* 	cout << "MAIN | main | GPS Lan: " << GPSInterface::GetInstance()->getLan() << endl; */
		/* 	cout << "MAIN | main | GPS Lon: " << GPSInterface::GetInstance()->getLon() << endl; */

		/* 	cout << "MAIN | main | ACC X: " << ImuInterface::GetInstance()->getAccX() << endl; */
		/* 	cout << "MAIN | main | ACC Y: " << ImuInterface::GetInstance()->getAccY() << endl; */
		/* 	cout << "MAIN | main | ACC Z: " << ImuInterface::GetInstance()->getAccZ() << endl; */

		/* 	cout << "MAIN | main | GYRO X: " << ImuInterface::GetInstance()->getGyroX() << endl; */
		/* 	cout << "MAIN | main | GYRO Y: " << ImuInterface::GetInstance()->getGyroY() << endl; */
		/* 	cout << "MAIN | main | GYRO Z: " << ImuInterface::GetInstance()->getGyroZ() << endl; */
		cout << "MAIN | main | Voltage: " << BatteryInterface::GetInstance()->getVoltage() << endl;
		cout << "MAIN | main | Current: " << BatteryInterface::GetInstance()->getCurrent() << endl;
		/* 	cout << endl; */

		ServoControl::GetInstance()->testServo();
		nanosleep((const struct timespec[]) { { 0, 500000000L } }, NULL);
	}
	return 1;
}
