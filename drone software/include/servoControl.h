/*
 * servoControl.h
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef SERVOCONTROL_H
#define SERVOCONTROL_H

#include <chrono>
#include <mutex>
#include <string>
#include <thread>

using namespace std;

class ServoControl {
	private:
	int pollingDelay = 10;
  static ServoControl* servoControl;
  static mutex mutexServoControl;

	protected:
	ServoControl();

	public:
	static ServoControl* GetInstance();
};


#endif /* !SERVOCONTROL_H */
