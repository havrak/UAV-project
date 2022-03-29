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
	INIReader reader(configDirectory);

	if (reader.ParseError() < 0) {
		cerr << "Config | loadConfiguration | ini config parsing failed\n";
		return false;
	}

	myIP = reader.GetString("connection", "my_IP", "192.168.6.1");
	serverPort = reader.GetInteger("connection", "server_port", 8066);
	string tmp = reader.Get("control", "controller_input_adjuster", "SQUARING");
	if (tmp.compare("TRIGONOMETRIC")) {
		cma = TRIGONOMETRIC;
	} else {
		cma = SQUARING;
	}

	tmp = reader.Get("configuration", "wing_surface_configuration", "STANDARD_TAIL_WING");
	if (tmp.compare("X_Y_INVERTED")) {
		wsc = V_SHAPE_TAIL_WING;
	} else {
		wsc = STANDARD_TAIL_WING;
	}

	tmp = reader.Get("configuration", "imu_orientation", "STANDARD");
	if (tmp.compare("X_Y_INVERTED")) {
		imo = X_Y_INVERTED;
	} else {
		imo = STANDART;
	}
	imuAddress = reader.GetInteger("configuration", "imu_address", 0x44);
	inaAddress = reader.GetInteger("configuration", "ina_address", 0x50);
	pca9685Address = reader.GetInteger("configuration", "pca_address", 0x40);
	return true;
}

string Config::getMyIP()
{
	return myIP;
}

int Config::getServerPort()
{
	return serverPort;
}

ControlMethodAdjuster Config::getControlMethodAdjuster()
{
	return cma;
}

WingSurfaceConfiguration Config::getWingSurfaceConfiguration()
{
	return wsc;
}

IMU_Orientation Config::getIMUOrientation()
{
	return imo;
}

int Config::getIMUAddress()
{
	return imuAddress;
}

int Config::getINAAddress()
{
	return inaAddress;
}

int Config::getPCA9865Address()
{
	return pca9685Address;
}
