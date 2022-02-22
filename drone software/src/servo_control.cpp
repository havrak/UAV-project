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
	servo.SetFrequency(60);
	servo.SetLeftUs(MIN_PULSE_LENGTH);
	servo.SetCenterUs(CEN_PULSE_LENGTH);
	servo.SetRightUs(MAX_PULSE_LENGTH);
	servo.SetInvert(false);
	/* slowDownToMin(); */
	armESC();
	/* servo.SetAngle(CHANNEL(1), ANGLE(90)); */

	servo.SetAngle(CHANNEL(2), ANGLE(180));
	servo.SetAngle(CHANNEL(3), ANGLE(90));
	servo.SetAngle(CHANNEL(4), ANGLE(90));
	servo.SetAngle(CHANNEL(5), ANGLE(90));

	/* servo.SetAngle(CHANNEL(10), ANGLE(90)); */
	/* servo.SetAngle(CHANNEL(11), ANGLE(90)); */
	/* servo.SetAngle(CHANNEL(14), ANGLE(90)); */
	/* servo.SetAngle(CHANNEL(15), ANGLE(90)); */

	if (debug)
		cout << "SERVOCONTROL | ServoControl | servos setted up, ESC armed\n";
}

ServoControl* ServoControl::GetInstance()
{

	if (servoControl == nullptr) {
		cout << "SERVOCONTROL | GetInstance | ServoControl creation\n";
		mutexServoControl.lock();
		if (servoControl == nullptr)
			servoControl = new ServoControl();
		mutexServoControl.unlock();
		cout << "SERVOCONTROL | GetInstance | ServoControl created\n";
	}

	return servoControl;
}

bool ServoControl::calibrateESC()
{
	int b;
	cout << "SERVOCONTROL | calibrateESC | setting max\n";
	servo.Set(CHANNEL(0), servo.GetRightUs());
	mainMotorMS = servo.GetRightUs();
	cout << "SERVOCONTROL | calibrateESC | press key to set min\n";
	cin >> b;

	cout << "SERVOCONTROL | calibrateESC | setting min\n";
	servo.Set(CHANNEL(0), servo.GetLeftUs());
	mainMotorMS = servo.GetLeftUs();
	nanosleep((const struct timespec[]) { { 8, 0L } }, NULL);

	return true;
}

bool ServoControl::armESC()
{
	cout << "SERVOCONTROL | armESC | arming ESC\n";
	slowDownToMin();
	nanosleep((const struct timespec[]) { { 8, 0L } }, NULL);
	return true;
}

void ServoControl::slowDownToMin()
{
	mainMotorMS = MIN_PULSE_LENGTH;
	servo.Set(CHANNEL(0), mainMotorMS);
}

void ServoControl::testServo()
{
	cout << "SERVOCONTROL | testServo | reaching max speed on motor\n";
	for (mainMotorMS = MIN_PULSE_LENGTH; mainMotorMS <= MAX_PULSE_LENGTH; mainMotorMS += 10) {
		cout << mainMotorMS << "\n";
		servo.Set(CHANNEL(0), mainMotorMS);
		nanosleep((const struct timespec[]) { { 0, 50000000L } }, NULL);
	}
	cout << "SERVOCONTROL | testServo | max speed reached\n";
	nanosleep((const struct timespec[]) { { 3, 0L } }, NULL);
	cout << "SERVOCONTROL | testServo | slowing down motor motor\n";
	for (mainMotorMS = MAX_PULSE_LENGTH; mainMotorMS >= MIN_PULSE_LENGTH; mainMotorMS -= 10) {
		cout << mainMotorMS << "\n";
		servo.Set(CHANNEL(0), mainMotorMS);
		nanosleep((const struct timespec[]) { { 0, 50000000L } }, NULL);
	}
	cout << "SERVOCONTROL | testServo | ZERO\n";

	nanosleep((const struct timespec[]) { { 5, 0L } }, NULL);
}

bool ServoControl::getPCA9865Status()
{
	return pca9685Up;
}

pair<int, unsigned int short*> ServoControl::getControlSurfaceConfiguration()
{
	pair<int, unsigned int short*> toReturn;
	toReturn.second = new unsigned int short[16];
	switch (configuration) {
	case V_SHAPE_TAIL_WING:
		toReturn.first = V_SHAPE_TAIL_WING;
		toReturn.second[vTail.leftFlapIndex] = vTail.leftFlap;
		toReturn.second[vTail.rightFlapIndex] = vTail.leftFlap;
		toReturn.second[vTail.leftRuddervatorIndex] = vTail.leftRuddervator;
		toReturn.second[vTail.leftRuddervatorIndex] = vTail.rightRuddervator;
		break;
	case STANDARD_TAIL_WING:
		break;
		toReturn.first = STANDARD_TAIL_WING;
		toReturn.second[standard.leftFlapIndex] = standard.leftFlap;
		toReturn.second[standard.rightFlapIndex] = standard.leftFlap;
		toReturn.second[standard.leftElevatorIndex] = standard.leftElevator;
		toReturn.second[standard.rightElevatorIndex] = standard.rightElevator;
		toReturn.second[standard.rudderIndex] = standard.rudder;
	}
	return toReturn;
}

int ServoControl::processMovementForVTail(pConStr ps)
{
	cout << "WE GOT CONTROLINO" << endl;
	// left trigger -> slow down
	// right trigger -> speed up
}

int ServoControl::processMovementForStandart(pConStr ps)
{
}

int ServoControl::processControl(ProcessingStructure ps)
{
	pConStr control;
	memcpy(&control, &ps.messageBuffer, sizeof(ps.messageBuffer));
	switch (configuration) {
	case V_SHAPE_TAIL_WING:
		return processMovementForVTail(control);
		break;
	case STANDARD_TAIL_WING:
		return processMovementForStandart(control);
		break;
	}
	cout << "Control processed\n";
	return 0;
}

unsigned int short ServoControl::getMainMotorMS()
{
	return mainMotorMS;
}
