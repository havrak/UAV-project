/*
 * config.h
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "../libraries/inih/cpp/INIReader.h"
#include "protocol_spec.h"
#include "imu_interface.h"
#include "servo_control.h"
#include <iostream>
#include <pwd.h>

class Config {
	private:


	const char* configDirectory;

	string myIP = "192.168.6.1";
	int serverPort = 8066;
	ControlMethodAdjuster cma = SQUARING;
	WingSurfaceConfiguration wsc = STANDARD_TAIL_WING;
	IMU_Orientation imo = STANDART;

	public:
	Config();
	bool loadConfiguration();


	string getMyIP();
	ControlMethodAdjuster getControlMethodAdjuster();
	WingSurfaceConfiguration getWingSurfaceConfiguration();
	IMU_Orientation getIMUOrientation();

	int getServerPort();


};

#endif /* !CONFIG_H */

