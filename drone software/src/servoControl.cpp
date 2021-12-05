/*
 * servoControl.cpp
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "servoControl.h"
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <time.h>

using namespace std;

ServoControl* ServoControl::servoControl = nullptr;
mutex ServoControl::mutexServoControl;

ServoControl::ServoControl()
{
	if (bcm2835_init() != 1) {
		fprintf(stderr, "bcm2835_init() failed\n");
		if(fd < 0 && debug) cerr << "SERVO CONTROL | ServoControl | failed to open I2C device" << endl;
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

void ServoControl::testServo(){
	for(int i = 1; i < 10; i++){
		servo.SetAngle(CHANNEL(0), ANGLE(90));
		servo.SetAngle(CHANNEL(1), ANGLE(0));
		servo.SetAngle(CHANNEL(2), ANGLE(0));
		servo.SetAngle(CHANNEL(3), ANGLE(0));

		cout << endl;
		cout << "Servo 0:90  Servo 1:0"<<endl;
		cout << "Servo 2:0  Servo 3:0"<<endl;
		nanosleep((const struct timespec[]){{0, 500000000L}}, NULL);

		servo.SetAngle(CHANNEL(0), ANGLE(180));
		servo.SetAngle(CHANNEL(1), ANGLE(180));
		servo.SetAngle(CHANNEL(2), ANGLE(90));
		servo.SetAngle(CHANNEL(3), ANGLE(90));

		cout << endl;
		cout << "Servo 0:180 Servo 1:180"<<endl;
		cout << "Servo 2:90  Servo 3:90"<<endl;
		nanosleep((const struct timespec[]){{0, 500000000L}}, NULL);

		servo.SetAngle(CHANNEL(0), ANGLE(0));
		servo.SetAngle(CHANNEL(1), ANGLE(90));
		servo.SetAngle(CHANNEL(2), ANGLE(180));
		servo.SetAngle(CHANNEL(3), ANGLE(180));

		cout << endl;
		cout << "Servo 0:0   Servo 1:90" <<endl;
		cout << "Servo 2:180  Servo 3:180" << endl;
		nanosleep((const struct timespec[]){{0, 500000000L}}, NULL);
	}
}
