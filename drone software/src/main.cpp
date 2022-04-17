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
	cout << "WARNING: This program relyes on command i2cdetect to detect the I2C bus. This command cannot be executed without sudo privileges. To run the program add i2cdetect to your /etc/sudoers file\n\n";


	signal(SIGKILL, signalHandler);
	signal(SIGPIPE, SIG_IGN);
	Config config;
	config.loadConfiguration();


	bool suppressArming = false; // don't want to wait 8 second for servos to arm
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
	ImuInterface::GetInstance()->setIMUOrientation(config.getIMUOrientation());

	while (true) {
		Telemetry::GetInstance()->processGeneralTelemetryRequest(client(-1, new mutex));
	}
	ServoControl::GetInstance()->attachPCA9685(config.getPCA9865Address());

	if(!suppressArming) ServoControl::GetInstance()->armESC();


	CommunicationInterface::GetInstance()->setupSocket(config.getMyIP(), config.getServerPort());
	ServoControl::GetInstance()->setOperationalParameters(config.getWingSurfaceConfiguration(), config.getControlMethodAdjuster());

	CommunicationInterface::GetInstance()->checkForNewDataThread.join();
	return 1;
}
