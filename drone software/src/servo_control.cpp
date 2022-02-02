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
	/* servo.Set(CHANNEL(0), servo.GetRightUs()); */
	/* cout << "SERVOCONTROL | calibrateESC | Press key" << endl; */
	/* cin >> b; */
	/* servo.Set(CHANNEL(0), servo.GetLeftUs()); */

	//servo.Set(CHANNEL(0), servo.GetRightUs());
	cout << "SERVOCONTROL | calibrateESC | setting max" << endl;
	servo.Set(CHANNEL(0), servo.GetRightUs());
	cout << "SERVOCONTROL | calibrateESC | press key to set min" << endl;
	cin >> b;

	cout << "SERVOCONTROL | calibrateESC | setting min" << endl;
	servo.Set(CHANNEL(0), servo.GetLeftUs());
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
	uint16_t min = MIN_PULSE_LENGTH;
	servo.Set(CHANNEL(0), min);
}

void ServoControl::testServo()
{
	cout << "SERVOCONTROL | testServo | reaching max speed on motor" << endl;
	for (uint16_t i = MIN_PULSE_LENGTH; i <= MAX_PULSE_LENGTH; i += 10) {
		cout << i << endl;
		servo.Set(CHANNEL(0), i);
		nanosleep((const struct timespec[]) { { 0, 50000000L } }, NULL);
	}
	cout << "SERVOCONTROL | testServo | max speed reached" << endl;
	nanosleep((const struct timespec[]) { { 3, 0L } }, NULL);
	cout << "SERVOCONTROL | testServo | slowing down motor motor" << endl;
	for (uint16_t i = MAX_PULSE_LENGTH; i >= MIN_PULSE_LENGTH; i -= 10) {
		cout << i << endl;
		servo.Set(CHANNEL(0), i);
		nanosleep((const struct timespec[]) { { 0, 50000000L } }, NULL);
	}
	cout << "SERVOCONTROL | testServo | ZERO" << endl;

	nanosleep((const struct timespec[]) { { 5, 0L } }, NULL);

	/* cout  << "SERVOCONTROL | testServo | testing servos" << endl; */
	/* for (int i = 1; i < 5; i++) { */
	/* 	for (int j = 0; j < 360; j++) { */
	/* 		servo.SetAngle(CHANNEL(1), ANGLE(j)); */
	/* 		servo.SetAngle(CHANNEL(2), ANGLE(j)); */
	/* 		nanosleep((const struct timespec[]) { { 0, 5000000L } }, NULL); */
	/* 	} */
	/* 	nanosleep((const struct timespec[]) { { 0, 500000000L } }, NULL); */
	/* } */
}
