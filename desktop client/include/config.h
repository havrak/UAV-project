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

enum OperatingSystems{
	WIN, MAC, LINUX, FREEBSD
};

class Config {
	private:


	const char* configDirectory;
	/* {"$HOME/.config/uav_control/config.ini","$HOME/.config/uav_control/"}; */

	// configuration
	OperatingSystems os;
	string myIP;
	string serverIP;
	int cameraPort;
	int serverPort;
	ControllerTypes controllerType;
	bool controlEnabled;


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
