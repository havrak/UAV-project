/*
 * gps_interface.cpp
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "gps_interface.h"

#include <chrono>
#include <iostream>
#include <sstream>
#include <string>

GPSInterface* GPSInterface::gpsInterface = nullptr;
mutex GPSInterface::mutexGPSInterface;

GPSInterface::GPSInterface()
{
}

GPSInterface* GPSInterface::GetInstance()
{
	if (gpsInterface == nullptr) {
		mutexGPSInterface.lock();
		if (gpsInterface == nullptr)
			gpsInterface = new GPSInterface();
		mutexGPSInterface.unlock();
	}
	return gpsInterface;
}

bool GPSInterface::attachGPS()
{
	fd = serialOpen("/dev/serial0", 9600);

	if (debug)
		cout << "GPSINTERFACE | attachGPS | Status of GPS: " << (fd >= 0) << "\n";
	return fd >= 0;
}

void GPSInterface::updateFunction()
{
	bool readingLine = false;
	string data = "";
	while (true) {

		if (serialDataAvail(fd) > 0) {
			char c = serialGetchar(fd);
			if (c == '$') {
				readingLine = true;
				data = "";
			} else if (c == '\n') {
				readingLine = false;
				if (debug)
					cout << "GPS_INTERFACE | updateFunction | Processing: " << data << "\n";
				vector<string> parts;
				stringstream ss(data);
				string item;
				while (getline(ss, item, ','))
					parts.push_back(item);

				if (parts.size() > 1) {
					if (parts.at(0).compare("$GPGGA") == 0) { // position inforamtion

						if (debug) {
							cout << "GPS_INTERFACE | updateFunction | found GPGGA string \n";
							cout << "GPS_INTERFACE | updateFunction | satellites: " << stoi(parts.at(7)) << "\n";
						}
						numberOfSatelites = stoi(parts.at(7));
						if (numberOfSatelites > 2) {
							gpsUp = true;
							longitude = stod(parts.at(4));
							latitude = stod(parts.at(2));
							altitude = stod(parts.at(9));
						} else {
							gpsUp = false;
							if (debug)
								cout << "GPS_INTERFACE | updateFunction | no satellites were found" << endl;
						}
					}else if(parts.at(0).compare("$GPVTG") == 0){ // ground speed information
							groundSpeed = stod(parts.at(6));
					}
				}
			}
			if (readingLine)
				data += c;
		}
		this_thread::sleep_for(chrono::milliseconds(pollingDelay));
	}
}

int GPSInterface::getPollingDelay()
{
	return pollingDelay;
}

void GPSInterface::setPollingDelay(int newPollingDelay)
{
	pollingDelay = newPollingDelay;
}

void GPSInterface::startLoop()
{
	loopThread = thread(&GPSInterface::updateFunction, this);
}

double GPSInterface::getLat()
{
	return latitude;
}

double GPSInterface::getLon()
{
	return longitude;
}

double GPSInterface::getAltitude()
{
	return altitude;
}

bool GPSInterface::getGPSStatus()
{
	return gpsUp;
}

int GPSInterface::getNOS()
{
	return numberOfSatelites;
}
