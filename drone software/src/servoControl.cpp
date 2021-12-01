/*
 * servoControl.cpp
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "servoControl.h"
#include <iostream>
#include <stdexcept>

using namespace std;

ServoControl* ServoControl::servoControl = nullptr;
mutex ServoControl::mutexServoControl;

ServoControl::ServoControl()
{
	fd = PCA9685_openI2C(1, PCA9685_ADDRESS);
	if(fd < 0 && debug) cerr << "SERVO CONTROL | ServoControl | failed to open I2C device" << endl;
	else if(debug) cout << "SERVO CONTROL | ServoControl | successfully setted up PCA9685, fd: " << fd << endl;

	int ret = PCA9685_initPWM(fd,PCA9685_ADDRESS, 60);
	if(ret !=0)	cerr << "SERVO CONTROL | ServoControl | failed to setup frequency" << endl;
}

ServoControl* ServoControl::GetInstance()
{
	if (servoControl == nullptr) {
		mutexServoControl.lock();
		if (servoControl == nullptr) servoControl = new ServoControl();
		mutexServoControl.unlock();
	}
	return servoControl;
}

void ServoControl::getPWMValues(){
	int ret = PCA9685_getPWMVals(fd, PCA9685_ADDRESS, gOnVals, gOffVals);
	if(ret !=0 && debug)	cerr << "SERVO CONTROL | getPWMValues | failed to query" << endl;
	cout << "PWM values: " << endl;
	cout << "getOnValues: ";
	for(unsigned int i : gOnVals){
		cout << i << " ";
	}
	cout << std::endl << "getOffValues: ";
	for(unsigned int i : gOffVals){
		cout << i << " ";
	}
	cout << std::endl;
}

void ServoControl::setPWMValues(){
	int ret = PCA9685_setPWMVals(fd, PCA9685_ADDRESS, sOnVals, sOffVals);
	if(ret != 0 && debug)	cerr << "SERVO CONTROL | setPWMValues | failed to set PWM values" << endl;
}
