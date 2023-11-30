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
	cout << "Config | loadConfiguration | loading from: " << configDirectory<< "\n";
	INIReader reader(configDirectory);

	if (reader.ParseError() < 0) {
		cerr << "Config | loadConfiguration | ini config parsing failed\n";
		return false;
	}

	myIP = reader.GetString("connection", "my_IP", "192.168.4.1");
	serverPort = reader.GetInteger("connection", "server_port", 8066);
	cma = reader.Get("control", "controller_input_adjuster", "SQUARING").compare("TRIGONOMETRIC") == 0 ? TRIGONOMETRIC : SQUARING;

	imuAddress = reader.GetInteger("configuration", "wt901b_address", 0x50);
	imo = reader.Get("configuration", "wt901b_orientation", "STANDARD").compare("STANDARD") == 0 ? STANDART : X_Y_INVERTED;
	inaBatAddress = reader.GetInteger("configuration", "ina226_bat_address", 0x44);
	inaBatShunt = reader.GetReal("configuration", "ina226_bat_shunt", 0.1);
	inaV5Address = reader.GetInteger("configuration", "ina226_v5_address", 0x44);
	inaV5Shunt = reader.GetReal("configuration", "ina226_v5_shunt", 0.1);
	pca9685Address = reader.GetInteger("configuration", "pca9685_address", 0x40);
	return true;
}

