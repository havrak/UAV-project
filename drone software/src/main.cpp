/*
 * main.cpp
 * Copyright (C) 2021 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include <wiringPi.h>
//#define BCM2835_NO_DELAY_COMPATIBILITY

#include "bcm2835.h"
#include "communication_interface.h"
#include "servo_control.h"
#include "telemetry.h"
#include <cstring>
#include <iostream>
#include <csignal>

using namespace std;

void signalHandler( int sigNum){
	cout << "MAIN | signalHandler | caught signal slowing down\n";
	CommunicationInterface::GetInstance()->cleanUp();
	ServoControl::GetInstance()->slowDownToMin();
	this_thread::sleep_for(chrono::milliseconds(10));
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
		cerr << "MAIN | main | failed to open I2C device\n";
	} else {
		cout << "MAIN | main | bcm2835 initialized, version: " << bcm2835_version() << "\n";
	}
	//ServoControl::GetInstance();
	/* cout << "MAIN | main | Setting up CAMERA_STREAMER" << endl; */
	/* CameraStreamer *cs1 = new CameraStreamer(0, 5000, "192.168.6.11"); */
	/* cs1->setupStream(); */
	/* cout << "MAIN | main | CAMERA_STREAMER setted up" << endl; */

	//IMPORTANT: ALWAYS KILL PROGRAM WHEN MOTOR IS TURNED OFF, OTHERWISE ESC GOES CRAZY
	Telemetry::GetInstance()->setUpSensors();
	CommunicationInterface::GetInstance()->setupSocket();

	// cout << "MAIN | main | ServoControl created" << endl;

	// cout << "MAIN | main | servo Calibrating"
	//ServoControl::GetInstance()->calibrateESC();
	cout << "MAIN | main | entering main loop\n";
	while (true) this_thread::sleep_for(chrono::milliseconds(1000));
	return 1;
}
