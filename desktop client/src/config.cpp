/*
 * config.cpp
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "config.h"

Config::Config()
{
#ifdef _WIN32
	os = WIN;
	configDirectory = "";
#elif _WIN64
	os = WIN;
	configDirectory = "";
#elif __APPLE__ || __MACH__
	os = MAC;
	configDirectory = "";
#elif __linux__
	os = LINUX;
	configDirectory = "$HOME/.config/uav_control/config.ini";
#elif __FreeBSD__
	os = BSD;
	configDirectory = "$HOME/.config/uav_control/config.ini";
#endif
}

bool Config::loadConfiguration()
{
	INIReader reader(configDirectory);

	if (reader.ParseError() < 0) {
		pTeleErr err(1, "Config wasn't found, or contains an error");
		mainWindow->displayError(err);
		return false;
	}
	cameraPort = reader.GetInteger("camera", "port", 5000);
	serverIP = reader.Get("connection", "piIP", "192.168.6.1");
	myIP = reader.Get("connection", "myIP", "192.168.6.11");
	serverPort = reader.GetInteger("connection", "serverPort", 8066);

	string tmp = reader.Get("control", "controllerType", "XBOX_CONTROLLER");
	if (tmp.compare("XBOX_CONTROLLER")) {
		controllerType = XBOX_CONTROLLER;
	} else {
		controllerType = XBOX_CONTROLLER;
	}
	controlEnabled = reader.GetBoolean("control", "controlEnabled", true);
	return true;
}

OperatingSystems Config::getOperatingSystem(){
	return os;
}

string Config::getMyIP()
{
	return myIP;
}
string Config::getServerIP()
{
	return serverIP;
}
int Config::getCameraPort()
{
	return cameraPort;
}
int Config::getServerPort()
{
	return serverPort;
}
ControllerTypes Config::getControllerType()
{
	return controllerType;
}
bool Config::getControlEnabled()
{
	return controlEnabled;
}
