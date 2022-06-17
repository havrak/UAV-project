/*
 * airmap_provider.cpp
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "airmap_provider.h"

AirmapProvider* AirmapProvider::airmapProvider = nullptr;
mutex AirmapProvider::airmapProviderMutex;

AirmapProvider::AirmapProvider()
{
}

AirmapProvider* AirmapProvider::GetInstance()
{
	if (airmapProvider == nullptr) {
		airmapProviderMutex.lock();
		if (airmapProvider == nullptr) {
			airmapProvider = new AirmapProvider();
		}
		airmapProviderMutex.unlock();
	}
	return airmapProvider;
};

void AirmapProvider::updateInfo()
{
	while (update) {
		cout << "AirmapProvider::updateInfo()" << endl;
		getFlightZoneInfo();
		this_thread::sleep_for(chrono::milliseconds(1500));
	}
};

void AirmapProvider::setupFetching(string key)
{
	apiKey = key;
	updateInfoThread = thread(&AirmapProvider::updateInfo, this);
}

void AirmapProvider::getWeatherInfo(){

}

void AirmapProvider::getFlightZoneInfo()
{
	// NOTE: blocking should be called somewhere away, if possible get periodically all info
	pair<double, double> position = DroneTelemetry::GetInstance()->getGPSPosition();

	const double multiplier1 = 0.5 * searchRadius;
	const double multiplier2 = 0.8660254037844386 * searchRadius;
	stringstream ss;
	position.first = 50.7353858;
	position.second = 15.7339622;
	cout << "lat" << position.first << "\n"
			 << "lon" << position.second << "\n";
	ss << setprecision(12)
		 << APIurl
		 << "search?geometry="
		 << "%7B"
		 << "%22type%22:%22Polygon%22,"
		 << "%22coordinates%22:%5B%5B"
		 << "%5B" << position.second - searchRadius << "," << position.first << "%5D,"
		 << "%5B" << position.second - multiplier2 << "," << position.first + multiplier1 << "%5D,"
		 << "%5B" << position.second - multiplier1 << "," << position.first + multiplier2 << "%5D,"
		 << "%5B" << position.second << "," << position.first + searchRadius << "%5D,"
		 << "%5B" << position.second + multiplier1 << "," << position.first + multiplier2 << "%5D,"
		 << "%5B" << position.second + multiplier2 << "," << position.first + multiplier1 << "%5D,"
		 << "%5B" << position.second + searchRadius << "," << position.first << "%5D,"
		 << "%5B" << position.second + multiplier2 << "," << position.first - multiplier1 << "%5D,"
		 << "%5B" << position.second + multiplier1 << "," << position.first - multiplier2 << "%5D,"
		 << "%5B" << position.second << "," << position.first - searchRadius << "%5D,"
		 << "%5B" << position.second - multiplier1 << "," << position.first - multiplier2 << "%5D,"
		 << "%5B" << position.second - multiplier2 << "," << position.first - multiplier1 << "%5D,"
		 << "%5B" << position.second - searchRadius << "," << position.first << "%5D"
		 << "%5D%5D"
		 << "%7D"
		 << "&types=airport,park,power_plant,emergency,controlled_airspace,tfr"
		 << "&full=true"
		 << "&geometry_format=wkt";

	/* cout << "URL: "<< ss.str() << "\n\nKEY: " << apiKey << endl; */
	auto r = cpr::Get(cpr::Url { ss.str() }, cpr::Header { { "X-API-Key", apiKey } });

	cout << "Response: " << r.text << endl;

	json parsedData = json::parse(r.text);
	airspaceObjects.clear();
	if(parsedData.empty() || parsedData["status"].get<string>().compare("fail") == 0)
		airspaceObjects.push_back(make_unique<airspaceStruct>(UNKNOWN, "No information about current flight zone"));


	for (auto& i : parsedData["data"].items()) {
		if(stringToAirspaceType.count(i.value()["type"].get<string>())) continue;
		airspaceObjects.push_back(make_unique<airspaceStruct>(stringToAirspaceType.at(i.value()["type"].get<string>()), i.value()["name"].get<string>()));

		switch (airspaceObjects.back().get()->type) {
		case AIRPORT: // airport
			{}
			break;
		case PARK: // park
			{}
			break;
		case POWER_PLANT: // power_plant
			{}
			break;
		case EMERGENCY: // emergency
			{}
			break;
		case CONTROLLED_AIRSPACE: // controlled_airspace
			{}
			break;
		case TFR: // tfr
			{}
			break;
		default:
			break;
		}

	}
}
