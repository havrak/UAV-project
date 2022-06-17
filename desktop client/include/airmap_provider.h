/*
 * airspace_info_provider.h
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef AIRMAP_PROVIDER_H
#define AIRMAP_PROVIDER_H

#include <cpr.h>
#include <iostream>
#include "drone_telemetry.h"
#include <thread>
#include <sstream>
#include <iomanip>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace std;

// TFR - temporary flight restrictions

enum airspaceType{
	CONTROLLED_AIRSPACE, AIRPORT, POWER_PLANT, EMERGENCY, PARK, TFR, UNKNOWN
};

const static std::unordered_map<std::string, airspaceType> stringToAirspaceType = {
		{ "airport", airspaceType::AIRPORT },
		{ "park", airspaceType::PARK },
		{ "power_plant", airspaceType::POWER_PLANT },
		{ "emergency", airspaceType::EMERGENCY },
		{ "controlled_airspace", airspaceType::CONTROLLED_AIRSPACE },
		{ "tfr", airspaceType::TFR }
	};

struct airspaceStruct{
	airspaceType type;
	string name;
	string additionalInfo;
	airspaceStruct(airspaceType type, string name) : type(type), name(name){}
};

struct weatherStruct{
	double windSpeed; // m/s
	int windDirection; // in degrees
};

/**
 * intefaces with Airmap.com via its REST API
 * gets information about flight zone, etc.
 *
 * NOTE: consider whether it should be singleton
 */
class AirmapProvider{
	private:
		static AirmapProvider* airmapProvider;
		static mutex airmapProviderMutex;
		thread updateInfoThread;
		bool update = true;
		char* APIurl =  "https://api.airmap.com/airspace/v2/";
		string apiKey = "";
		double searchRadius = 0.00017; // cca 200meters
		vector<unique_ptr<airspaceStruct>> airspaceObjects;


		AirmapProvider();
		void updateInfo();

	public:
		/**
		 * main method used to access AirmapProvider
		 * if instace isn't created it will create it
		 */
		static AirmapProvider* GetInstance();

		/**
		 * sets API key
		 */
		void setupFetching(string key);


		/**
		 * updates information about current weather
		 */
		void getWeatherInfo();

		/**
		 * upddates information about airspace (formed by drawing dodecagon) around the drone
		 * size of search airspace is dictated by searchRadius
		 */
		void getFlightZoneInfo();



};

#endif /* !AIRSPACE_INFO_PROVIDER_H */
