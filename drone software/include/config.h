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
#include "wt901b_decorator.h"
#include "pca9685_decorator.h"
#include <iostream>
#include <pwd.h>
#include <unistd.h>
/**
 * Parses programs configuration file and provides getters
 * to get the neccesarray information
 */
class Config {
	private:


	const char* configDirectory;

	string myIP = "192.168.6.1";
	int serverPort = 8066;

	ControlMethodAdjuster cma = SQUARING;
	IMU_Orientation imo = STANDART;

	int imuAddress = 0x44;
	int inaAddress = 0x50;
	int pca9685Address = 0x40;

	public:

	/**
	 * constructor of Config class
	 *
	 */
	Config();

	/**
	 * parses configuration file and set local variables
	 *
	 * @return bool - true if config was parsed successfully
	 */
	bool loadConfiguration();


	string getMyIP();
	ControlMethodAdjuster getControlMethodAdjuster();
	IMU_Orientation getIMUOrientation();

	int getServerPort();
	int getIMUAddress();
	int getINABatAddress();
	int getINA5VAddress();
	int getPCA9865Address();

};

#endif /* !CONFIG_H */

