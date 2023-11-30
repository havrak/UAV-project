/*
 * main.cpp
 * Copyright (C) 2021 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include <wiringPi.h>

#include "communication_interface.h"
#include "config.h"
#include "peripherials_manager.h"
#include <cstring>
#include <iostream>
#include <csignal>

using namespace std;

void signalHandler( int sigNum){
	cout << "MAIN | signalHandler | caught signal slowing down\n";
	CommunicationInterface::GetInstance()->cleanUp();
	PeripherialsManager::GetInstance()->servoControllMin();
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

	cout << "MAIN | main | initializing peripherials" << endl;

	// TODO: peripherals should be created here and added to the peripherials manager
	PeripherialsManager::GetInstance()->initializePeripherials(config.getINABatAddress(), config.getINA5VAddress(), config.getPCA9865Address(), config.getIMUAddress());

	// NOTE: this is not the right place to setup INA226
	PeripherialsManager::GetInstance()->setupINA(config.getINABatShunt(), config.getINA5VShunt());

	cout << "MAIN | main | initializing servo" << endl;
	bool suppressArming = false; // don't want to wait 8 second for servos to arm
	if (argc > 1){
		if(strcmp(argv[1], "-r") == 0){
			cout << "MAIN | main | calirating ESC" << endl;
			PeripherialsManager::GetInstance()->servoControllCalibrate();
			return 1;
		}
		if(strcmp(argv[1], "-a") == 0)
				suppressArming = true;
	}

	if(!suppressArming) PeripherialsManager::GetInstance()->servoControllArm();

	cout << "MAIN | main | starting communication interface" << endl;
	CommunicationInterface::GetInstance()->setupSocket(config.getMyIP(), config.getServerPort());

	CommunicationInterface::GetInstance()->checkForNewDataThread.join();
	return 1;
}
