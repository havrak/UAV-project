/*
 * main.cpp
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "bcm2835.h"
#include "communication_interface.h"
#include "servo_control.h"
#include "telemetry.h"
#include <cstring>
#include <iostream>
#include <csignal>

using namespace std;

void signalHandler( int sigNum){
	cout << "MAIN | signalHandler | caught signal slowing down" << endl;
	ServoControl::GetInstance()->slowDownToMin();
	CommunicationInterface::GetInstance()->cleanUp();
	exit(127);
}

int main(int argc, char** argv)
{
	signal(SIGKILL, signalHandler);

	/* if (argc > 1){ */
	/* 	if(strcmp(argv[1], "-r") == 0){ */
	/* 		cout << "MAIN | main | reseting ServoControl" << endl; */
	/* 		ServoControl::GetInstance(); */
	/* 	} */
	/* 	return 1; */
	/* } */


	if (!bcm2835_init()) {
		cerr << "MAIN | main | failed to open I2C device" << endl;
	} else {
		cout << "MAIN | main | bcm2835 initialized, version: " << bcm2835_version() << endl;
	}
	ServoControl::GetInstance();
	/* cout << "MAIN | main | Setting up CAMERA_STREAMER" << endl; */
	/* CameraStreamer *cs1 = new CameraStreamer(0, 5000, "192.168.6.11"); */
	/* cs1->setupStream(); */
	/* cout << "MAIN | main | CAMERA_STREAMER setted up" << endl; */

	//IMPORTANT: ALWAYS KILL PROGRAM WHEN MOTOR IS TURNED OFF, OTHERWISE ESC GOES CRAZY
	Telemetry::GetInstance()->setUpSensors();
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
		/* cout << "MAIN | main | GYRO Y: " << ImuInterface::GetInstance()->getGyroY() << endl; */
		/* 	cout << "MAIN | main | GYRO Z: " << ImuInterface::GetInstance()->getGyroZ() << endl; */
		/* 	cout << endl; */

		ServoControl::GetInstance()->testServo();
		nanosleep((const struct timespec[]) { { 0, 500000000L } }, NULL);
	}
	return 1;
}
