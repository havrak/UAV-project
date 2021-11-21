/*
 * servoControl.cpp
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "servoControl.h"

ServoControl* ServoControl::servoControl = nullptr;
mutex ServoControl::mutexServoControl;

ServoControl::ServoControl()
{
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


