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
	char *homedir = getpwuid(getuid())->pw_dir;
	configDirectory = strcat(homedir,"/.config/uav_control/config.ini");
#elif __FreeBSD__
	os = BSD;
	char *homedir = getpwuid(getuid())->pw_dir;
	configDirectory = strcat(homedir,"/.config/uav_control/config.ini");
#endif
}

bool Config::loadConfiguration()
{
	INIReader reader(configDirectory);
	cout << "CONFIG | loadConfiguration | loading from: " << configDirectory << "\n";

	if (reader.ParseError() < 0) {
		cerr  << "CONFIG | loadConfiguration | parsing failed \n";
		pTeleErr err(1, "Config wasn't found, or contains an error");
		mainWindow->displayError(err);
		return false;
	}else{

	}
	cameraPort = reader.GetInteger("camera", "port", 5000);
	serverIP = reader.Get("connection", "pi_IP", "192.168.6.1");
	myIP = reader.Get("connection", "my_IP", "192.168.6.11");
	serverPort = reader.GetInteger("connection", "server_port", 8066);

	cout << "camera :" << getCameraPort() << "\n";

	string tmp = reader.Get("control", "controller_type", "XBOX_CONTROLLER");
	if (tmp.compare("PS4_DUALSHOCK")) {
		controllerType = PS4_DUALSHOCK;
	} else {
		controllerType = XBOX_CONTROLLER;
	}
	controlEnabled = reader.GetBoolean("control", "control_enabled", true);
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
