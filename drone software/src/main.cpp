/*
 * main.cpp
 * Copyright (C) 2021 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include <wiringPi.h>

#include "communication_interface.h"
#include "config.h"
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
	signal(SIGPIPE, SIG_IGN);
	Config config;
	config.loadConfiguration();


	bool suppressArming = false;
	if (argc > 1){
		if(strcmp(argv[1], "-r") == 0){
			cout << "MAIN | main | calirating ESC" << endl;
			ServoControl::GetInstance()->calibrateESC();
			return 1;
		}
		if(strcmp(argv[1], "-a") == 0)
				suppressArming = true;
	}
	Telemetry::GetInstance()->setUpSensors(config.getIMUAddress(), config.getINAAddress(), config.getPCA9865Address());
	ServoControl::GetInstance()->attachPCA9685(config.getPCA9865Address());

	if(!suppressArming) ServoControl::GetInstance()->armESC();


	ImuInterface::GetInstance()->setIMUOrientation(config.getIMUOrientation());
	CommunicationInterface::GetInstance()->setupSocket(config.getMyIP(), config.getServerPort());
	ServoControl::GetInstance()->setOperationalParameters(config.getWingSurfaceConfiguration(), config.getControlMethodAdjuster());

	//ServoControl::GetInstance()->calibrateESC();
	cout << "MAIN | main | entering main loop\n";
	CommunicationInterface::GetInstance()->checkForNewDataThread.join();
	return 1;
}
