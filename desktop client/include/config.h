/*
 * config.h
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "../libraries/inih/cpp/INIReader.h"
#include "main_window.h"
#include "protocol_spec.h"
#include <pwd.h>

enum OperatingSystems{
	WIN, MAC, LINUX, FREEBSD
};

class Config {
	private:


	const char* configDirectory;
	/* {"$HOME/.config/uav_control/config.ini","$HOME/.config/uav_control/"}; */

	// configuration, default values need to be set, in case configuration doesn't exist
	OperatingSystems os;
	string myIP = "192.168.6.11";
	string serverIP= "192.168.6.1";
	int cameraPort = 5000;
	int serverPort = 8066;
	ControllerTypes controllerType = XBOX_CONTROLLER;
	bool controlEnabled = true;


	public:
	Config();
	bool loadConfiguration();

	OperatingSystems getOperatingSystem();
	string getMyIP();
	string getServerIP();
	int getCameraPort();
	int getServerPort();

	ControllerTypes getControllerType();
	bool getControlEnabled();

};

#endif /* !CONFIG_H */
