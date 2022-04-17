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
	cma = reader.Get("control", "controller_input_adjuster", "SQUARING").compare("TRIGONOMETRIC") == 0 ? TRIGONOMETRIC : SQUARING;
	wsc = reader.Get("configuration", "wing_surface_configuration", "STANDARD_TAIL_WING").compare("V_SHAPE_TAIL_WING") == 0 ? V_SHAPE_TAIL_WING : STANDARD_TAIL_WING;
	imo = reader.Get("configuration", "imu_orientation", "STANDARD").compare("STANDARD") == 0 ? STANDART : X_Y_INVERTED;

	imuAddress = reader.GetInteger("configuration", "imu_address", 0x50);
	inaAddress = reader.GetInteger("configuration", "ina_address", 0x44);
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
