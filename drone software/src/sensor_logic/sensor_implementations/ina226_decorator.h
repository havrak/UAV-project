/*
 * ina226_decorator.h
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef INA226_DECORATOR_H
#define INA226_DECORATOR_H

#include "ina226.h"
#include <cmath>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <chrono>
#include <string>
#include <iostream>
#include "../sensor.h"

#define INA_I2C_ADDRESS 0x44

using namespace std;

class INA226Decorator : Sensor {
	private:
	INA226 ina226;
	float voltage = 0;
	float current = 0;
	float power = 0;
	float shunt = 0;
	float energy =0;
	time_t rawtime =0;
	char buffer[80];
	int trig=1;

	protected:


	public:

		INA226Decorator(uint8_t multiplexerPosition, uint8_t i2cAddress, uint8_t peripheryAddresses);
		INA226Decorator(uint8_t multiplexerPosition, uint8_t i2cAddress, uint8_t peripheryAddresses, uint8_t peripherySubaddress);

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
	 * initializes INA226 library
	 *
	 * @return bool - true if connection was established
	 */
	bool attachINA226(int address);

	float getVoltage();
	float getCurrent();
	float getPower();
	float getShunt();
	float getEnergy();


};

#endif /* !BATTERYINTERFACE_H */
