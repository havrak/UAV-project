/*
 * wt901_decorator.h
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "wt901_decorator.h"
#include <chrono>
#include <iostream>

WT901Decorator::WT901Decorator()
{
}

bool WT901Decorator::attachIMU(int address)
{
	bool tmp = JY901.startI2C(address);

	if(!tmp ) {
		return false;
	}
	/* JY901.setD1mode(0x05); // change mode of D1 port to gps */
	/* JY901.saveConf(0); */
	return tmp;

}

bool WT901Decorator::resetOrientation(){
	yawOffset = JY901.getYaw();
	pitchOffset = JY901.getPitch();
	rollOffset = JY901.getRoll();
	return true;
}


