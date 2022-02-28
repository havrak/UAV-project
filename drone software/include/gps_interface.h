/*
 * gps_interface.h
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef GPS_INTERFACE_H
#define GPS_INTERFACE_H

#include <wiringPi.h>
#include <wiringSerial.h>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <sstream>
#include <iostream>

using namespace std;

class GPSInterface {
	private:
	int pollingDelay = 25; // TODO: should be more accurate
	int fd;
	const bool debug = false;
  static GPSInterface* gpsInterface;
  static mutex mutexGPSInterface;

	// values
	bool gpsUp = false;
	double longitude = 0;
	double latitude = 0;
	double altitude = 0;
	char numberOfSatelites = 0;

	protected:
	thread loopThread;
	GPSInterface();

	/**
	 * method used byt thread to regularly
	 * read data from serial
	 */
	void updateFunction();

	public:
	/**
	 * main method used to access GPSInterface
	 * if instace wasn't created it will initialize
	 * GPSInterface
	 */
	static GPSInterface* GetInstance();

	/**
	 * start loop to read data from serial
	 */
	void startLoop();

	/**
	 * returns value of how often data should be read
	 *
	 * @return int - polling delay
	 */
	int getPollingDelay();

	/**
	 * sets new time how often data should be read
	 *
	 */
	void setPollingDelay(int newPollingDelay);

	/**
	 * opens serial connections to the sensor
	 *
	 * @return bool - true if connection was open
	 */
	bool attachGPS();

	/**
	 * returns latitude of drone in degrees
	 *
	 * @return double - latitude
	 */
	double getLat();

	/**
	 * returns longitude of drone in degrees
	 *
	 * @return double - latitude
	 */
	double getLon();

	/**
	 * returns altitude of drone in meters
	 *
	 * @return double - latitude
	 */
	double getAltitude();

	/**
	 * returns status of GPS
	 *
	 * @return bool - true if it sees more than two satelites
	 */
	bool getGPSStatus();

	/**
	 * returns number of satelites that GPS
	 * module sees
	 *
	 * @return int - number of satelites
	 */
	int getNOS();

};


#endif /* !GPS_INTERFACE_H */
