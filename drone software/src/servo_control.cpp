/*
 * servoControl.cpp
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "servo_control.h"
#include "bcm2835.h"
#include "generic_PID.h"
#include "imu_interface.h"
#include "protocol_spec.h"
#include "telemetry.h"
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
	servo.SetFrequency(60);
	servo.SetLeftUs(MIN_PULSE_LENGTH);
	servo.SetCenterUs(CEN_PULSE_LENGTH);
	servo.SetRightUs(MAX_PULSE_LENGTH);
	servo.SetInvert(false);
	armESC();

	servo.SetAngle(CHANNEL(2), ANGLE(135));
	servo.SetAngle(CHANNEL(3), ANGLE(135));
	servo.SetAngle(CHANNEL(4), ANGLE(135));
	servo.SetAngle(CHANNEL(5), ANGLE(135));

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

void ServoControl::setAngleOfServo(int channel, bool right, unsigned char angle)
{
	if (right) {
		/* cout << "Right: channel: " << channel << ", i: " << (90 + angle) << "\n"; */
		servo.SetAngle(channel, 90 + angle);
	} else {
		cout << "Left:  channel: " << channel << ", i: " << (180 - angle) << "\n";
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

void ServoControl::testServo()
{
	cout << "SERVOCONTROL | testServo | reaching max speed on motor\n";
	for (mainMotorMS = MIN_PULSE_LENGTH; mainMotorMS <= MAX_PULSE_LENGTH; mainMotorMS += 10) {
		cout << mainMotorMS << "\n";
		servo.Set(CHANNEL(0), mainMotorMS);
		nanosleep((const struct timespec[]) { { 0, 50000000L } }, NULL);
	}
	nanosleep((const struct timespec[]) { { 3, 0L } }, NULL);

	cout << "SERVOCONTROL | testServo | slowing down motor motor\n";
	for (mainMotorMS = MAX_MOTOR_PULSE_LENGTH; mainMotorMS >= MAX_MOTOR_PULSE_LENGTH; mainMotorMS -= 10) {
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

bool ServoControl::pidController()
{
	ImuInterface::GetInstance()->resetOrientation(); // we want to maintain current orientation

	GenericPID pitchPID(7, 0.5, 2, 1);
	GenericPID rollPID(7, 1, 3, 1);
	while (pidOn) {

		if(abs(ImuInterface::GetInstance()->getPitch()) <1.5 &&  abs(ImuInterface::GetInstance()->getRoll()) <1 ){
			this_thread::sleep_for(chrono::milliseconds(100));
			continue;
		}
		float pitchOutput = pitchPID.calculateOutput(ImuInterface::GetInstance()->getPitch()/2);
		float rollOutput = rollPID.calculateOutput(ImuInterface::GetInstance()->getRoll()/2);

		switch (configuration) {
		case V_SHAPE_TAIL_WING:
				//roll
				setAngleOfServo(vTail.leftFlapIndex, false, 45-rollOutput);
				setAngleOfServo(vTail.rightFlapIndex, true, 45+rollOutput);
				//pitch
				setAngleOfServo(vTail.leftRuddervatorIndex, false, 45-pitchOutput);
				setAngleOfServo(vTail.rightRuddervatorIndex, true, 45+pitchOutput);

			break;
		case STANDARD_TAIL_WING:

			break;
		}
			this_thread::sleep_for(chrono::milliseconds(100));
	}
}

bool ServoControl::togglePIDController()
{
	if ((pidOn = !pidOn) == true) {
		pidControllerThread = thread(&ServoControl::pidController, this);
	} else {
		pidControllerThread.join();
	}
	return true;
}

float scalerX, scalerY;
float tmpX, tmpY;
int yaw, pitch;

int ServoControl::processMovementForVTail(pConStr ps)
{
	adjustMainMotorSpeed(ps);

	/* float tmp; */

	tmpX = ps.lAnalog.first / MAX_CONTROLLER_AXIS_VALUE;
	tmpY = ps.lAnalog.second / MAX_CONTROLLER_AXIS_VALUE;
	scalerX = tmpX * sqrt(1 - tmpY * tmpY / 2);
	scalerY = tmpY * sqrt(1 - tmpX * tmpX / 2);
	ps.lAnalog.first = MAX_CONTROLLER_AXIS_VALUE * scalerX;
	ps.lAnalog.second = MAX_CONTROLLER_AXIS_VALUE * scalerY;

	/* if (ps.lAnalog.first == 0){ */
	/* 	tmp = atan(ps.lAnalog.second / ps.rAnalog.first); */
	/* 	if(tmp < 1.047) // we don't want to decrease value */
	/* 		ps.lAnalog.first*=cos(tmp)*2; */
	/* 	if(tmp > 0.524) */
	/* 		ps.lAnalog.second*=sin(tmp)*2; */
	/* } */

	if (ps.lAnalog.first == 0)
		yaw = 0;
	else
		yaw = ((float)ps.lAnalog.first / MAX_CONTROLLER_AXIS_VALUE) * 90;
	if (ps.lAnalog.second == 0)
		pitch = 0;
	else
		pitch = ((float)ps.lAnalog.second / MAX_CONTROLLER_AXIS_VALUE) * 90;

	vTail.leftRuddervator = (yaw + pitch) * MIXING_GAIN;
	vTail.rightRuddervator = (90 - yaw + pitch) * MIXING_GAIN;
	setAngleOfServo(vTail.leftRuddervatorIndex, false, vTail.leftRuddervator);
	setAngleOfServo(vTail.rightRuddervatorIndex, true, vTail.rightRuddervator);
	setAngleOfServo(vTail.leftFlapIndex, false, yaw);
	setAngleOfServo(vTail.rightFlapIndex, true, (90 - yaw));
	return 1;
}

int ServoControl::processMovementForStandart(pConStr ps)
{
	return 0;
}

int ServoControl::processControl(ProcessingStructure ps)
{
	pConStr control;
	memcpy(&control, ps.getMessageBuffer(), ps.messageSize);
	if (pidOn) {
		adjustMainMotorSpeed(control);
		return 0;
	}
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
