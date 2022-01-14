/*
 * main.cpp
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "battery_interface.h"
#include "imu_interface.h"
#include "gps_interface.h"
#include "servo_control.h"
#include "camera_streamer.h"
#include "bcm2835.h"
#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
	ImuInterface::GetInstance()->attachIMU();
	cout << "MAIN | main | IMU attached" << endl;
	//ImuInterface::GetInstance()->startLoop();
	//cout << "MAIN | main | IMU Loop started" << endl;

	BatteryInterface::GetInstance()->attachINA226(0x44);
	cout << "MAIN | main | INA226 attached" << endl;
	BatteryInterface::GetInstance()->startLoop();
	cout << "MAIN | main | INA226 Loop started" << endl;

	GPSInterface::GetInstance()->attachGPS();
	cout << "MAIN | main | GPS attached" << endl;
	GPSInterface::GetInstance()->startLoop();
	cout << "MAIN | main | GPS Loop started" << endl;


	if (!bcm2835_init()) {
			cerr << "MAIN | main | failed to open I2C device" << endl;
	} else {
			cout << "MAIN | main | bcm2835 initialized, version: " << bcm2835_version() << endl;
	}
	/* cout << "MAIN | main | Setting up CAMERA_STREAMER" << endl; */
	/* CameraStreamer *cs1 = new CameraStreamer(0, 5000, "192.168.6.11"); */
	/* cs1->setupStream(); */
	/* cout << "MAIN | main | CAMERA_STREAMER setted up" << endl; */

	ServoControl::GetInstance();
	cout << "ServoControl created" << endl;

	while (true) {
		/* cout << "NEW OUT" << endl; */

		cout << "GPS Lan: " << GPSInterface::GetInstance()->getLan() << endl;
		cout << "GPS Lon: " << GPSInterface::GetInstance()->getLon() << endl;

		cout << "ACC X: " << ImuInterface::GetInstance()->getAccX() << endl;
		cout << "ACC Y: " << ImuInterface::GetInstance()->getAccY() << endl;
		cout << "ACC Z: " << ImuInterface::GetInstance()->getAccZ() << endl;

		cout << "GYRO X: " << ImuInterface::GetInstance()->getGyroX() << endl;
		cout << "GYRO Y: " << ImuInterface::GetInstance()->getGyroY() << endl;
		cout << "GYRO Z: " << ImuInterface::GetInstance()->getGyroZ() << endl;

		cout << endl;
		cout << "Voltage: " << BatteryInterface::GetInstance()->getVoltage() << endl;
		cout << "Current: " << BatteryInterface::GetInstance()->getCurrent() << endl;

		/* //ServoControl::GetInstance()->testServo(); */
		nanosleep((const struct timespec[]) { { 0, 500000000L } }, NULL);

	}
	return 1;
}
