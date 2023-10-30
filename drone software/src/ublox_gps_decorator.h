/*
 * ublox_gps_decorator.h
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef UBLOX_GPS_DECORATOR_H
#define UBLOX_GPS_DECORATOR_H

#include <wiringPi.h>
#include <wiringSerial.h>
#include <chrono>
#include <string>
#include <thread>
#include <vector>
#include <sstream>
#include <iostream>
#include "periphery.h"

using namespace std;

class UBloxGPSDecorator : public Periphery{
	private:
	int fd;
	const bool debug = false;

	// values
	double longitude = 0;
	double latitude = 0;
	double altitude = 0;
	double groundSpeed = 0;
	double heading;

	char numberOfSatelites = 0;

	protected:


	/**
	 * method used byt thread to regularly
	 * read data from serial
	 */
	void updateFunction();

	public:
	UBloxGPSDecorator();

	bool initialize() override;


	/**
	 * returns latitude of drone in degrees
	 *
	 * @return double - latitude
	 */
	double getLat(){return latitude;}

	/**
	 * returns longitude of drone in degrees
	 *
	 * @return double - latitude
	 */
	double getLon(){return longitude;}

	/**
	 * returns altitude of drone in meters
	 *
	 * @return double - latitude
	 */
	double getAltitude(){return altitude;}

	/**
	 * return ground speed of drone in knots
	 *
	 * @return double - ground speed
	 */
	double getGroundSpeed(){return groundSpeed;}


	/**
	 * returns number of satelites that GPS
	 * module sees
	 *
	 * @return int - number of satelites
	 */
	int getNOS(){return numberOfSatelites;}

	/**
	 * returns heading of the plane
	 *
	 * @return double - heading in degrees;
	 */
	double getHeading(){return heading;}

};


#endif /* !UBLOX_GPS_DECORATOR_H */
