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
	void updateFunction();

	public:
	static GPSInterface* GetInstance();
	void startLoop();
	int getPollingDelay();
	void setPollingDelay(int newPollingDelay);

	bool attachGPS();
	double getLan();
	double getLon();
	double getAltitude();
	bool getGPSStatus();

};


#endif /* !GPS_INTERFACE_H */
