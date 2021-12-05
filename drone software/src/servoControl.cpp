/*
 * servoControl.cpp
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "servoControl.h"
#include "bcm2835.h"
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <time.h>
#include <cstdio>
#include <cstdint>
#include <unistd.h>

using namespace std;

ServoControl* ServoControl::servoControl = nullptr;
mutex ServoControl::mutexServoControl;

ServoControl::ServoControl()
{
	/* if (debug) { */
	/* 	cout << "SERVOCONTROL | ServoControl | bcm2835 enableing debug"; */
	/* 	bcm2835_set_debug(1); */
	/* 	cout << "SERVOCONTROL | ServoControl | bcm2835 debug enabled"; */
	/* } */
	if (!bcm2835_init()) {
		if (debug)
			cerr << "SERVOCONTROL | ServoControl | failed to open I2C device" << endl;
	} else {
		if (debug)
			cout << "SERVOCONTROL | ServoControl | bcm2835 initialized, version: " << bcm2835_version() << endl;
	}
	servo.SetFrequency(60);
	servo.SetLeftUs(700);
	servo.SetRightUs(2400);
	servo.SetAngle(CHANNEL(1), ANGLE(90));
	servo.SetAngle(CHANNEL(2), ANGLE(90));
	servo.SetAngle(CHANNEL(3), ANGLE(90));
	servo.SetAngle(CHANNEL(4), ANGLE(90));
	servo.SetAngle(CHANNEL(5), ANGLE(90));
	servo.SetAngle(CHANNEL(6), ANGLE(90));
	if (debug)
		cout << "SERVOCONTROL | ServoControl | servos setted up" << endl;
}

ServoControl* ServoControl::GetInstance()
{
	cout << "SERVOCONTROL | GetInstance | ServoControl creation" << endl;

	if (servoControl == nullptr) {
		mutexServoControl.lock();
		if (servoControl == nullptr)
			servoControl = new ServoControl();
		mutexServoControl.unlock();
	}
	cout << "SERVOCONTROL | GetInstance | ServoControl created" << endl;

	return servoControl;
}

bool ServoControl::calibrateESC()
{
	servo.Set(CHANNEL(0), servo.GetRightUs());
	nanosleep((const struct timespec[]) { { 2, 100000000L } }, NULL);
	servo.Set(CHANNEL(1), servo.GetLeftUs());
	nanosleep((const struct timespec[]) { { 2, 100000000L } }, NULL);
	// servo.Set(CHANNEL(1), servo.GetCenterUs());
	return true;
}

void ServoControl::testServo()
{
	for (int i = 1; i < 10; i++) {
		servo.SetAngle(CHANNEL(0), ANGLE(90));
		servo.SetAngle(CHANNEL(1), ANGLE(0));
		servo.SetAngle(CHANNEL(2), ANGLE(0));
		servo.SetAngle(CHANNEL(3), ANGLE(0));

		cout << endl;
		cout << "Servo 0:90  Servo 1:0" << endl;
		cout << "Servo 2:0  Servo 3:0" << endl;
		nanosleep((const struct timespec[]) { { 0, 500000000L } }, NULL);

		servo.SetAngle(CHANNEL(0), ANGLE(180));
		servo.SetAngle(CHANNEL(1), ANGLE(180));
		servo.SetAngle(CHANNEL(2), ANGLE(90));
		servo.SetAngle(CHANNEL(3), ANGLE(90));

		cout << endl;
		cout << "Servo 0:180 Servo 1:180" << endl;
		cout << "Servo 2:90  Servo 3:90" << endl;
		nanosleep((const struct timespec[]) { { 0, 500000000L } }, NULL);

		servo.SetAngle(CHANNEL(0), ANGLE(0));
		servo.SetAngle(CHANNEL(1), ANGLE(90));
		servo.SetAngle(CHANNEL(2), ANGLE(180));
		servo.SetAngle(CHANNEL(3), ANGLE(180));

		cout << endl;
		cout << "Servo 0:0   Servo 1:90" << endl;
		cout << "Servo 2:180  Servo 3:180" << endl;
		nanosleep((const struct timespec[]) { { 0, 500000000L } }, NULL);
	}
}
