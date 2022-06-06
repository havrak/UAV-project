/*
 * airmap_provider.cpp
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "airmap_provider.h"

AirmapProvider* AirmapProvider::airmapProvider = nullptr;
mutex AirmapProvider::airmapProviderMutex;

AirmapProvider::AirmapProvider(){
	updateInfoThread = thread(&AirmapProvider::updateInfo, airmapProvider);

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

void AirmapProvider::updateInfo(){
	while(update){
		getFlightZoneInfo();
		this_thread::sleep_for(chrono::milliseconds(1500));
	}
};

void AirmapProvider::getFlightZoneInfo(){
// NOTE: blocking should be called somewhere away, if possible get periodically all info
  pair<double,double> position = DroneTelemetry::GetInstance()->getGPSPosition();

	const double multiplier1 = 0.5*searchRadius;
	const double multiplier2 = 0.8660254037844386* searchRadius;
	stringstream ss;
	ss << APIurl
	<< "search?geometry="
	<< "%7B"
	<< "%22type%22:%22Polygon%22,"
	<< "%22coordinates%22:%5B%5B"
	<< "%5B" << position.first-searchRadius << "," << position.second   						<< "%5D,"
	<< "%5B" << position.first-multiplier2 	<< "," << position.second+multiplier1 	<< "%5D,"
	<< "%5B" << position.first-multiplier1 	<< "," << position.second+multiplier2 	<< "%5D,"
	<< "%5B" << position.first  						<< "," << position.second+searchRadius  << "%5D,"
	<< "%5B" << position.first+multiplier1 	<< "," << position.second+multiplier2   << "%5D,"
	<< "%5B" << position.first+multiplier2 	<< "," << position.second+multiplier1  	<< "%5D,"
	<< "%5B" << position.first+searchRadius << "," << position.second   						<< "%5D,"
	<< "%5B" << position.first+multiplier2 	<< "," << position.second-multiplier1  	<< "%5D,"
	<< "%5B" << position.first+multiplier1 	<< "," << position.second-multiplier2   << "%5D,"
	<< "%5B" << position.first   						<< "," << position.second-searchRadius  << "%5D,"
	<< "%5B" << position.first-multiplier1 	<< "," << position.second-multiplier2   << "%5D,"
	<< "%5B" << position.first-multiplier2 	<< "," << position.second-multiplier1  	<< "%5D,"
	<< "%5D%5D"
	<< "%7D"
	<< "&types=airport,controlled_airspace,tfr,wildfire"
	<< "&full=true"
	<< "&geometry_format=wkt";

	auto r = cpr::Get(cpr::Url{ss.str()}, cpr::Header{ { "X-API-Key",  airmapAPIKey} });
  std::cout << "Returned Text:" << r.text << std::endl;
}
