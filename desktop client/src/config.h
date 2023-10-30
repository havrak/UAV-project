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

/**
 * Parses programs configuration file and provides getters
 * to get the neccesarray information
 */
class Config {
	private:


	const char* configDirectory;
	/* {"$HOME/.config/uav_control/config.ini","$HOME/.config/uav_control/"}; */

	// configuration, default values need to be set, in case configuration doesn't exist
	OperatingSystems os;
	string myIP = "192.168.6.11";
	string serverIP= "192.168.6.1";
	string airmapAPIKey = "";
	int cameraPort = 5000;
	int serverPort = 8066;
	ControllerTypes controllerType = XBOX_CONTROLLER;
	bool controlEnabled = true;


	public:

	/**
   * Constructor of config class, depending on the operating system
	 * it select correct file path
	 */
	Config();

	/**
	 * parses configuration file and set local variables
	 *
	 * @return bool - true if config was parsed successfully
	 */
	bool loadConfiguration();

	OperatingSystems getOperatingSystem();
	string getMyIP();
	string getServerIP();
	int getCameraPort();
	int getServerPort();
	string getAirmapAPIKey();

	ControllerTypes getControllerType();
	bool getControlEnabled();

};

#endif /* !CONFIG_H */
