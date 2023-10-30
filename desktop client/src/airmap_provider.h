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
#include <boost/date_time/posix_time/posix_time.hpp>

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
	bool status; // false -> no weather data
	double windSpeed; // m/s
	int windDirection; // in degrees
	string condition;
	double temperature; // in Celsius
	string timestamp;

	weatherStruct() : status(false), windSpeed(0), windDirection(0), condition(""), temperature(0), timestamp(""){}
	weatherStruct(bool status, double windSpeed, int windDirection, string condition, double temperature, string timestamp) : status(status), windSpeed(windSpeed), windDirection(windDirection), condition(condition), temperature(temperature),timestamp(timestamp){}
	weatherStruct copy(){
		return weatherStruct(status, windSpeed, windDirection, condition, temperature, timestamp);
	}

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
		char* APIurl =  "https://api.airmap.com/";
		string apiKey = "";
		double searchRadius = 0.00017; // cca 200meters


		mutex airspaceObjectsMutex;
		vector<unique_ptr<airspaceStruct>> airspaceObjects;

		mutex weatherMutex;
		weatherStruct weather;


		AirmapProvider();
		void updateInfo();

		/**
		 * updates information about current weather
		 */
		void fetchWeatherInfo();

		/**
		 * upddates information about airspace (formed by drawing dodecagon) around the drone
		 * size of search airspace is dictated by searchRadius
		 *
		 * As informational value of airmap.com data for drone pilot in Europe is questionable,
		 * logic is rather rudimentary
		 */
		void fetchAirspaceInfo();


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
		 * returns current weather information
		 *
		 * @return weatherStruct - structure with info about weather
		 */
    weatherStruct getWeather();

		/**
		 * returns list of all object in current airspace
		 *
		 * @return vector<unique_ptr<airspaceStruct>> - list of airspaceStructs
		 */
		vector<unique_ptr<airspaceStruct>>	 getAirspaceInfo();
};

#endif /* !AIRSPACE_INFO_PROVIDER_H */
