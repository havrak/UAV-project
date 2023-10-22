/*
 * config.cpp
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "config.h"

Config::Config()
{
	char* homedir = getpwuid(getuid())->pw_dir;
	configDirectory = strcat(homedir, "/.config/uav_control/config.ini");
}

bool Config::loadConfiguration()
{
	cout << "configDirectory " << configDirectory<< "\n";
	INIReader reader(configDirectory);

	if (reader.ParseError() < 0) {
		cerr << "Config | loadConfiguration | ini config parsing failed\n";
		return false;
	}

	myIP = reader.GetString("connection", "my_IP", "192.168.6.1");
	serverPort = reader.GetInteger("connection", "server_port", 8066);
	cma = reader.Get("control", "controller_input_adjuster", "SQUARING").compare("TRIGONOMETRIC") == 0 ? TRIGONOMETRIC : SQUARING;
	imo = reader.Get("configuration", "imu_orientation", "STANDARD").compare("STANDARD") == 0 ? STANDART : X_Y_INVERTED;

	imuAddress = reader.GetInteger("configuration", "imu_address", 0x50);
	inaAddress = reader.GetInteger("configuration", "ina_address", 0x44);
	pca9685Address = reader.GetInteger("configuration", "pca_address", 0x40);
	return true;
}

