/*
 * ublox_gps_decorator.cpp
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "ublox_gps_decorator.h"

#include <chrono>
#include <iostream>
#include <sstream>
#include <string>

mutex UBloxGPSDecorator::mutexUBloxGPSDecorator;

UBloxGPSDecorator::UBloxGPSDecorator()
{
}

bool UBloxGPSDecorator::attachGPS()
{
	fd = serialOpen("/dev/serial0", 9600);

	if (debug)
		cout << "GPSINTERFACE | attachGPS | Status of GPS: " << (fd >= 0) << "\n";
	return fd >= 0;
}

void UBloxGPSDecorator::updateFunction()
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
							error = false;

							longitude = parts.at(4).empty() ? 0 : stod(parts.at(4));
							latitude = parts.at(2).empty() ? 0 : stod(parts.at(2));
							altitude = parts.at(9).empty() ? 0 : stod(parts.at(9));
						} else {
							error = true;
							if (debug)
								cout << "GPS_INTERFACE | updateFunction | no satellites were found" << endl;
						}
					}else if(parts.at(0).compare("$GPVTG") == 0){ // ground speed information
						groundSpeed = parts.at(6).empty() ? -1 : stod(parts.at(6));
					}else if(parts.at(0).compare("GPHDT")){
						heading = parts.at(1).empty() ? 0 : stod(parts.at(1));
					}
				}
			}
			if (readingLine)
				data += c;
		}
		this_thread::sleep_for(chrono::milliseconds(pollingDelay));
	}
}

int UBloxGPSDecorator::getPollingDelay()
{
	return pollingDelay;
}

void UBloxGPSDecorator::setPollingDelay(int newPollingDelay)
{
	pollingDelay = newPollingDelay;
}

void UBloxGPSDecorator::startLoop()
{
	loopThread = thread(&UBloxGPSDecorator::updateFunction, this);
}

