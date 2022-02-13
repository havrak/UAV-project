/*
 * servoControl.cpp
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "servo_control.h"
#include "bcm2835.h"
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <time.h>
#include <unistd.h>

using namespace std;

ServoControl* ServoControl::servoControl = nullptr;
mutex ServoControl::mutexServoControl;

ServoControl::ServoControl()
{
	pca9685Up = true;
	servo.SetFrequency(60);
	servo.SetLeftUs(MIN_PULSE_LENGTH);
	servo.SetCenterUs(CEN_PULSE_LENGTH);
	servo.SetRightUs(MAX_PULSE_LENGTH);
	servo.SetInvert(false);
	armESC();
	servo.SetAngle(CHANNEL(1), ANGLE(90));
	servo.SetAngle(CHANNEL(2), ANGLE(90));
	servo.SetAngle(CHANNEL(3), ANGLE(90));
	servo.SetAngle(CHANNEL(4), ANGLE(90));
	servo.SetAngle(CHANNEL(5), ANGLE(90));
	servo.SetAngle(CHANNEL(6), ANGLE(90));

	if (debug)
		cout << "SERVOCONTROL | ServoControl | servos setted up, ESC armed" << endl;
}

ServoControl* ServoControl::GetInstance()
{

	if (servoControl == nullptr) {
		cout << "SERVOCONTROL | GetInstance | ServoControl creation" << endl;
		mutexServoControl.lock();
		if (servoControl == nullptr)
			servoControl = new ServoControl();
		mutexServoControl.unlock();
		cout << "SERVOCONTROL | GetInstance | ServoControl created" << endl;
	}

	return servoControl;
}

bool ServoControl::calibrateESC()
{
	int b;
	cout << "SERVOCONTROL | calibrateESC | setting max" << endl;
	servo.Set(CHANNEL(0), servo.GetRightUs());
	mainMotorMS = servo.GetRightUs();
	cout << "SERVOCONTROL | calibrateESC | press key to set min" << endl;
	cin >> b;

	cout << "SERVOCONTROL | calibrateESC | setting min" << endl;
	servo.Set(CHANNEL(0), servo.GetLeftUs());
	mainMotorMS = servo.GetLeftUs();
	nanosleep((const struct timespec[]) { { 8, 0L } }, NULL);


return true;
}

bool ServoControl::armESC()
{
	cout << "SERVOCONTROL | armESC | arming ESC" << endl;
	slowDownToMin();
	nanosleep((const struct timespec[]) { { 8, 0L } }, NULL);
	return true;
}


void ServoControl::slowDownToMin(){
	mainMotorMS = MIN_PULSE_LENGTH;
	servo.Set(CHANNEL(0), mainMotorMS);
}

void ServoControl::testServo()
{
	cout << "SERVOCONTROL | testServo | reaching max speed on motor" << endl;
	for (mainMotorMS = MIN_PULSE_LENGTH; mainMotorMS <= MAX_PULSE_LENGTH; mainMotorMS += 10) {
		cout << mainMotorMS << endl;
		servo.Set(CHANNEL(0), mainMotorMS);
		nanosleep((const struct timespec[]) { { 0, 50000000L } }, NULL);
	}
	cout << "SERVOCONTROL | testServo | max speed reached" << endl;
	nanosleep((const struct timespec[]) { { 3, 0L } }, NULL);
	cout << "SERVOCONTROL | testServo | slowing down motor motor" << endl;
	for (mainMotorMS = MAX_PULSE_LENGTH; mainMotorMS >= MIN_PULSE_LENGTH; mainMotorMS -= 10) {
		cout << mainMotorMS << endl;
		servo.Set(CHANNEL(0), mainMotorMS);
		nanosleep((const struct timespec[]) { { 0, 50000000L } }, NULL);
	}
	cout << "SERVOCONTROL | testServo | ZERO" << endl;

	nanosleep((const struct timespec[]) { { 5, 0L } }, NULL);

}

bool ServoControl::getPCA9865Status(){
	return pca9685Up;
}


pair<int,unsigned int short*> ServoControl::getControlSurfaceConfiguration(){
	pair<int, unsigned int short*> toReturn;
	toReturn.second = new unsigned int short[16];
	switch(configuration){
		case V_SHAPE_TAIL_WING:
			toReturn.first = V_SHAPE_TAIL_WING;
			toReturn.second[vTail.leftFlapIndex] = vTail.leftFlap;
			toReturn.second[vTail.rightFlapIndex] = vTail.leftFlap;
			toReturn.second[vTail.leftRuddervatorIndex] = vTail.leftFlap;
			toReturn.second[vTail.leftRuddervatorIndex] = vTail.leftFlap;
			break;
	}
	return toReturn;
}

int ServoControl::processMovementForVTail(processingStruct ps){

}

int ServoControl::processControl(processingStruct ps){
	switch(configuration){
			case V_SHAPE_TAIL_WING:
				return processMovementForVTail(ps);
			break;
	}
	return 0;
}


unsigned int short ServoControl::getMainMotorMS(){
	return mainMotorMS;
}
