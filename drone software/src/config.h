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
	int inaBatAddress = 0x50;
	int inaV5Address = 0x50;
	int pca9685Address = 0x40;
	float inaBatShunt = 0;
	float inaV5Shunt = 0;

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


	string getMyIP(){return myIP;};
	ControlMethodAdjuster getControlMethodAdjuster(){return cma;};
	IMU_Orientation getIMUOrientation(){return imo;};

	int getServerPort(){return serverPort;};
	int getIMUAddress(){return imuAddress;};

	int getINABatAddress(){return inaBatAddress;};
	int getINABatShunt(){return inaBatShunt;};
	float getINA5VAddress(){return inaV5Address;};
	float getINA5VShunt(){return inaV5Shunt;};

	int getPCA9865Address(){return pca9685Address;};

};

#endif /* !CONFIG_H */

