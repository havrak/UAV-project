/*
 * gps_decorator.h
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef GPS_DECORATOR_H
#define GPS_DECORATOR_H

#include <wiringPi.h>
#include <wiringSerial.h>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <sstream>
#include <iostream>
#include "../sensor.h"

using namespace std;

class GPSDecorator : Sensor {
	private:
	int pollingDelay = 25; // TODO: should be more accurate
	int fd;
	double longitude = 0;
	double latitude = 0;
	double altitude = 0;
	double groundSpeed = 0;
	double heading;

	char numberOfSatelites = 0;

	/**
	 * method used byt thread to regularly
	 * read data from serial
	 */
	void updateFunction();

	public:

		GPSDecorator(uint8_t multiplexerPosition, uint8_t i2cAddress, uint8_t peripheryAddresses);
		GPSDecorator(uint8_t multiplexerPosition, uint8_t i2cAddress, uint8_t peripheryAddresses, uint8_t peripherySubaddress);

		/**
		 * reads data from sensor
		 *
		 * when anomaly is detected will trigger emergency sending mode
		 */
		bool read() override;

		/**
		 * initializes MPU, if initialization fails
		 * false will be returned end MPUerror set to true
		 *
		 * @return bool - true if initialization was successful
		 */
		bool initialize() override;

		uint8_t type() const override {
			return 0;
		}

		/**
		 * scans for 1-Wire devices connected to the module
		 * schedules reading of values from thermometers connected
		 *
		 */
		uint8_t call(uint16_t id) override;

		/**
		 * adds sensor tasks to Tasker
		 * tasks are: TSID_SENSOR_DS248X
		 *
		 * @return bool - true if tasks were added
		 */
		bool addTasks() override;

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
	 * return ground speed of drone in knots
	 *
	 * @return double - ground speed
	 */
	double getGroundSpeed();

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

	/**
	 * returns heading of the plane
	 *
	 * @return double - heading in degrees;
	 */
	double getHeading();

};


#endif /* !GPS_DECORATOR_H */
