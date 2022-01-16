/*
 * servoControl.h
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#define MIXING_GAIN 0.5

#define MIN_PULSE_LENGTH 200 // Minimum pulse length in µs1
#define CEN_PULSE_LENGTH 1000 // Central pulse length in µs
#define MAX_PULSE_LENGTH 1800 // Maximum pulse length in µs
// NOTE: current max is over 1900

#include "../libraries/rpidmx512-Lib-PCA9685/pca9685servo.h"
#include <bcm2835.h>

#include <chrono>
#include <mutex>
#include <string>
#include <thread>

using namespace std;



class ServoControl {
private:
  int fd = -1;
  const bool debug = true;
  static ServoControl *servoControl;
  static mutex mutexServoControl;
	PCA9685Servo servo;

protected:
  ServoControl();

public:
  static ServoControl *GetInstance();
  bool calibrateESC();
	bool armESC();
	void slowDownToMin();
	void testServo();
};

#endif /* !SERVOCONTROL_H */
