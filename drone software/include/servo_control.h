/*
 * servoControl.h
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#define MIXING_GAIN 0.5

#define MIN_PULSE_LENGTH 1000 // Minimum pulse length in µs
#define MAX_PULSE_LENGTH 3000 // Maximum pulse length in µs

/* #include <PCA9685.h> */
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
	void testServo();
};

#endif /* !SERVOCONTROL_H */