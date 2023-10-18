/*
 * servoControl.cpp
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "servo_control.h"
#include "protocol_spec.h"
#include <bits/types/clock_t.h>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <iterator>
#include <math.h>
#include <ostream>
#include <stdexcept>
#include <thread>
#include <time.h>
#include <unistd.h>

using namespace std;

ServoControl* ServoControl::servoControl = nullptr;
mutex ServoControl::mutexServoControl;

ServoControl::ServoControl()
{

}

ServoControl* ServoControl::GetInstance()
{

	if (servoControl == nullptr) {
		mutexServoControl.lock();
		if (servoControl == nullptr)
			servoControl = new ServoControl();
		mutexServoControl.unlock();
	}

	return servoControl;
}


bool ServoControl::attachPCA9685(int address){
	servo = PCA9685Servo(address);

	servo.SetFrequency(60);
	servo.SetLeftUs(MIN_PULSE_LENGTH);
	servo.SetCenterUs(CEN_PULSE_LENGTH);
	servo.SetRightUs(MAX_PULSE_LENGTH);
	servo.SetInvert(false);
	/* armESC(); */

	servo.SetAngle(CHANNEL(2), ANGLE(135));
	servo.SetAngle(CHANNEL(3), ANGLE(135));
	servo.SetAngle(CHANNEL(4), ANGLE(135));
	servo.SetAngle(CHANNEL(5), ANGLE(135));

	/* if (debug) */
	cout << "SERVOCONTROL | ServoControl | servos setted up, ESC armed\n";

}

void ServoControl::setPichAndRoll(float pitch, float roll)
{
	this->pitch = pitch;
	this->roll = roll;
}

void ServoControl::setAngleOfServo(int channel, bool right, unsigned char angle)
{
	if (right) {
		servo.SetAngle(channel, 90 + angle);
	} else {
		servo.SetAngle(channel, 180 - angle);
	}
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

void ServoControl::setOperationalParameters(WingSurfaceConfiguration wsc, ControlMethodAdjuster cma)
{
	planeConfiguration = wsc;
	controllAdjuster = cma;
}

bool ServoControl::getPCA9865Status()
{
	return pca9685Up;
}
void ServoControl::setPCA9865Status(bool status)
{
	pca9685Up= status;
}


pair<int, unsigned char*> ServoControl::getControlSurfaceConfiguration()
{
	pair<int, unsigned char*> toReturn;
	toReturn.second = new unsigned char[16];
	switch (planeConfiguration) {
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

bool ServoControl::adjustMainMotorSpeed(pConStr ps)
{
	if (ps.lTrigger < 2500)
		ps.lTrigger = 0;
	if (ps.rTrigger < 2500)
		ps.rTrigger = 0;
	if (ps.lTrigger > 63000)
		ps.lTrigger = MAX_CONTROLLER_AXIS_VALUE;
	if (ps.rTrigger > 63000)
		ps.rTrigger = MAX_CONTROLLER_AXIS_VALUE;
	int valL = (ps.lTrigger << 10) >> 10;
	int valR = (ps.rTrigger << 10) >> 10;

	if (valL < valR) {
		// accelerate
		if (mainMotorMS + log(valR - valL) * SERVO_ACCELERATION_MULTIPLIER <= MAX_MOTOR_PULSE_LENGTH) {
			mainMotorMS += log(valR - valL) * SERVO_ACCELERATION_MULTIPLIER;
			servo.Set(CHANNEL(0), mainMotorMS);
			/* cout << "SERVO_CONTROL | processMovementForVTail | accelerating: " << mainMotorMS << "\n"; */
		}
	} else {
		// decelerate
		if (mainMotorMS - log(valL - valR) * SERVO_ACCELERATION_MULTIPLIER >= MIN_PULSE_LENGTH) {
			mainMotorMS -= log(valL - valR) * SERVO_ACCELERATION_MULTIPLIER;
			if (mainMotorMS - 80 > MIN_PULSE_LENGTH)
				mainMotorMS = MIN_PULSE_LENGTH;
			/* cout << "SERVO_CONTROL | processMovementForVTail | decelerating: " << mainMotorMS << "\n"; */
			servo.Set(CHANNEL(0), mainMotorMS);
		}
	}

	return true;
}

float scalerX, scalerY;
float tmpX, tmpY;
int yaw, pitch;

bool ServoControl::processMovementForVTail(pConStr ps)
{
	adjustMainMotorSpeed(ps);
	vTail.leftRuddervator = (roll + pitch) * MIXING_GAIN;
	vTail.rightRuddervator = (90 - roll + pitch) * MIXING_GAIN;
	setAngleOfServo(vTail.leftRuddervatorIndex, false, vTail.leftRuddervator);
	setAngleOfServo(vTail.rightRuddervatorIndex, true, vTail.rightRuddervator);
	setAngleOfServo(vTail.leftFlapIndex, false, roll);
	setAngleOfServo(vTail.rightFlapIndex, true, (90 - roll));
	return true;
}

bool ServoControl::processMovementForStandart(pConStr ps)
{
	return true;
}

bool ServoControl::processControl(ProcessingStructure ps)
{
	pConStr control;
	memcpy(&control, ps.getMessageBuffer(), sizeof(control));
	if (pidOn) {
		adjustMainMotorSpeed(control);
		return 0;
	}


	switch (controllAdjuster) {

	case TRIGONOMETRIC: {
		float tmp;
		if (control.lAnalog.first == 0) {
			tmp = atan(control.lAnalog.second / control.rAnalog.first);
			if (tmp < 1.047) // we don't want to decrease value
				control.lAnalog.first *= cos(tmp) * 2;
			if (tmp > 0.524)
				control.lAnalog.second *= sin(tmp) * 2;
		}
	}
	case SQUARING: {
		tmpX = control.lAnalog.first / MAX_CONTROLLER_AXIS_VALUE;
		tmpY = control.lAnalog.second / MAX_CONTROLLER_AXIS_VALUE;
		scalerX = tmpX * sqrt(1 - tmpY * tmpY / 2);
		scalerY = tmpY * sqrt(1 - tmpX * tmpX / 2);
		control.lAnalog.first = MAX_CONTROLLER_AXIS_VALUE * scalerX;
		control.lAnalog.second = MAX_CONTROLLER_AXIS_VALUE * scalerY;
	} break;
	}

	if (control.lAnalog.first == 0)
		roll = 0;
	else
		roll = ((float)control.lAnalog.first / MAX_CONTROLLER_AXIS_VALUE) * 90;
	if (control.lAnalog.second == 0)
		pitch = 0;
	else
		pitch = ((float)control.lAnalog.second / MAX_CONTROLLER_AXIS_VALUE) * 90;

	switch (planeConfiguration) {
	case V_SHAPE_TAIL_WING:
		return processMovementForVTail(control);
		break;
	case STANDARD_TAIL_WING:
		return processMovementForStandart(control);
		break;
	}
	cout << "Control processed\n";
	return true;
}

unsigned int short ServoControl::getMainMotorMS()
{
	return mainMotorMS;
}

