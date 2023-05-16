/*
 * gps_interface.cpp
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "gps_decorator.h"

#include <chrono>
#include <iostream>
#include <sstream>
#include <string>

GPSDecorator(uint8_t multiplexerPosition, uint8_t i2cAddress, uint8_t peripheryAddresses): Sensor(multiplexerPosition, i2cAddress, peripheryAddresses){
	initialize();
}

GPSDecorator(uint8_t multiplexerPosition, uint8_t i2cAddress, uint8_t peripheryAddresses, uint8_t peripherySubaddress): Sensor(multiplexerPosition, i2cAddress, peripheryAddresses, peripherySubaddress){
	initialize();
	};

bool GPSDecorator::initialize()
{
	fd = serialOpen("/dev/serial0", 9600);
	error = fd < 0;
	return fd >= 0;
}

bool GPSDecorator::read()
{
	bool readingLine = false;
	string data = "";

	if (serialDataAvail(fd) > 0) {
		char c = serialGetchar(fd);
		if (c == '$') {
			readingLine = true;
			data = "";
		} else if (c == '\n') {
			readingLine = false;
			vector<string> parts;
			stringstream ss(data);
			string item;
			while (getline(ss, item, ','))
				parts.push_back(item);

			if (parts.size() > 1) {
				if (parts.at(0).compare("$GPGGA") == 0) {

					numberOfSatelites = stoi(parts.at(7));
					if (numberOfSatelites > 2) {

						longitude = parts.at(4).empty() ? 0 : stod(parts.at(4));
						latitude = parts.at(2).empty() ? 0 : stod(parts.at(2));
						altitude = parts.at(9).empty() ? 0 : stod(parts.at(9));
					} else {
					}
				} else if (parts.at(0).compare("$GPVTG") == 0) { // ground speed information
					groundSpeed = parts.at(6).empty() ? -1 : stod(parts.at(6));
				} else if (parts.at(0).compare("GPHDT") == 0) {
					heading = parts.at(1).empty() ? 0 : stod(parts.at(1));
				}
			}
		}
		if (readingLine)
			data += c;
	}
	return false;
}

double GPSDecorator::getLat()
{
	return latitude;
}

double GPSDecorator::getLon()
{
	return longitude;
}

double GPSDecorator::getAltitude()
{
	return altitude;
}


int GPSDecorator::getNOS()
{
	return numberOfSatelites;
}

double GPSDecorator::getGroundSpeed()
{
	return groundSpeed;
}

double GPSDecorator::getHeading()
{
	return heading;
}
