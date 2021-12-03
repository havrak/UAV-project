/*
 * servoControl.h
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef SERVOCONTROL_H
#define SERVOCONTROL_H

#define MIXING_GAIN 0.5
#define PCA9685_ADDRESS 0x40

#include <PCA9685.h>
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
  unsigned int gOnVals[_PCA9685_CHANS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  unsigned int gOffVals[_PCA9685_CHANS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

protected:
  ServoControl();

public:
  unsigned int sOnVals[_PCA9685_CHANS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  unsigned int sOffVals[_PCA9685_CHANS] = {0, 500, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  static ServoControl *GetInstance();
  bool calibrateESC();
  void getPWMValues();
  void setPWMValues();
};

#endif /* !SERVOCONTROL_H */
