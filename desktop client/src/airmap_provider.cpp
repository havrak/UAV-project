/*
 * airmap_provider.cpp
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "airmap_provider.h"
#include "main_window.h"

AirmapProvider* AirmapProvider::airmapProvider = nullptr;
mutex AirmapProvider::airmapProviderMutex;

AirmapProvider::AirmapProvider()
{
	airspaceObjects.push_back(make_unique<airspaceStruct>(UNKNOWN, "No information about current flight zone"));
	weather.status = false;

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
		fetchWeatherInfo();
		fetchAirspaceInfo();
		this_thread::sleep_for(chrono::milliseconds(10000));
	}
};

void AirmapProvider::setupFetching(string key)
{
	apiKey = key;
	updateInfoThread = thread(&AirmapProvider::updateInfo, this);
}

void AirmapProvider::fetchWeatherInfo()
{
	pair<double, double> position = DroneTelemetry::GetInstance()->getGPSPosition();
	stringstream ss;
	using namespace boost::posix_time;
	ptime t = microsec_clock::universal_time();

	ss << setprecision(12)
		 << APIurl
		 << "advisory/v2/"
		 << "weather/"
		 << "?longitude=" << position.second
		 << "&latitude=" << position.first
		 << "&start=" << to_iso_extended_string(t) << "Z"
		 << "&end=" << to_iso_extended_string(t) << "Z";

	auto r = cpr::Get(cpr::Url { ss.str() }, cpr::Header { { "X-API-Key", apiKey } });
	{
		const std::lock_guard<std::mutex> lock(weatherMutex);
		try {
			json parsedData = json::parse(r.text);

			if (parsedData.empty() || parsedData["status"].get<string>().compare("fail") == 0) {
				weather.status = false;
			}
			weather.status = true;
			weather.condition = parsedData["data"]["weather"][0]["condition"].get<string>();
			weather.temperature = parsedData["data"]["weather"][0]["temperature"].get<double>();
			weather.windDirection = parsedData["data"]["weather"][0]["wind"]["heading"].get<int>();
			weather.windSpeed = parsedData["data"]["weather"][0]["wind"]["speed"].get<double>();
			weather.timestamp = to_iso_extended_string(t);
			mainWindow->updateWeather(weather.copy());
		} catch (exception& e) {
			weather.status = false;
			weather.timestamp = to_iso_extended_string(t);
		}
	}
	return;
}

void AirmapProvider::fetchAirspaceInfo()
{
	pair<double, double> position = DroneTelemetry::GetInstance()->getGPSPosition();

	const double multiplier1 = 0.5 * searchRadius;
	const double multiplier2 = 0.8660254037844386 * searchRadius;
	stringstream ss;
	position.first = 50.7353858;
	position.second = 15.7339622;
	ss << setprecision(12)
		 << APIurl
		 << "airspace/v2/"
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

	{
		const std::lock_guard<std::mutex> lock(airspaceObjectsMutex);
		try {
			json parsedData = json::parse(r.text);

			if (parsedData.empty() || parsedData["status"].get<string>().compare("fail") == 0) {
				airspaceObjects.push_back(make_unique<airspaceStruct>(UNKNOWN, "No information about current flight zone"));
				return;
			}

			for (auto& i : parsedData["data"].items()) {
				cout << i.value() << endl;
				if (stringToAirspaceType.count(i.value()["type"].get<string>()))
					continue;
				airspaceObjects.push_back(make_unique<airspaceStruct>(stringToAirspaceType.at(i.value()["type"].get<string>()), i.value()["name"].get<string>()));

				switch (airspaceObjects.back().get()->type) {
				case AIRPORT:
					airspaceObjects.back().get()->additionalInfo = i.value()["name"].get<string>() + "(" + i.value()["icao"].get<string>() + ")";
					break;
				case PARK:
					airspaceObjects.back().get()->additionalInfo = i.value()["name"].get<string>();
					break;
				case POWER_PLANT:
					airspaceObjects.back().get()->additionalInfo = i.value()["name"].get<string>() + "(" + i.value()["properties"]["tech"].get<string>() + ")";
					break;
				case EMERGENCY:
					break;
				case CONTROLLED_AIRSPACE:
					airspaceObjects.back().get()->additionalInfo = i.value()["name"].get<string>();
					break;
				case TFR:
					airspaceObjects.back().get()->additionalInfo = i.value()["properties"]["notam_reason"].get<string>();
					break;
				default:
					break;
				}
			}
		} catch (exception& e) {
			airspaceObjects.push_back(make_unique<airspaceStruct>(UNKNOWN, "No information about current flight zone"));
		}
	}

	return;
}

weatherStruct AirmapProvider::getWeather()
{
	const std::lock_guard<std::mutex> lock(weatherMutex);
	return weather.copy();
}

vector<unique_ptr<airspaceStruct>> AirmapProvider::getAirspaceInfo()
{
	const std::lock_guard<std::mutex> lock(airspaceObjectsMutex);
	vector<unique_ptr<airspaceStruct>> toReturn;

	toReturn.reserve(airspaceObjects.size());
	memcpy(&toReturn, &airspaceObjects, sizeof(airspaceObjects));

	return toReturn;
}
