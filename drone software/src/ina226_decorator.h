/*
 * ina226_decorator.h
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef INA226_DECORATOR_H
#define INA226_DECORATOR_H

#include "../libraries/raspberry-pi-ina226/ina226.h"
#include <cmath>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <iostream>
#include "i2c_periphery.h"


class INA226Decorator : public I2CPeriphery {
	private:

	float voltage = 0;
	float current = 0;
	float power = 0;
	float shunt = 0;
	float energy =0;
	time_t rawtime =0;
	char buffer[80];
	int trig=1;

	int pollingDelay = 1000;
	const bool debug = true;

	INA226 ina226;

	protected:


	public:
	INA226Decorator(uint8_t address);


	void read() override;

	bool initialize() override;



	/**
   * return how often should data be read from sensor
	 *
	 * @return int - polling delay
	 */
	int getPollingDelay();

	/**
   * sets how often should data be read from sensor
	 *
	 * @return int - polling delay
	 */
	void setPollingDelay(int pollingDelay);

	void setShunt(float shunt){ina226.ina226_calibrate(shunt, 2);};

	float getVoltage(){return voltage;}

	float getCurrent(){return current;}

	float getPower(){return power;}

	float getShunt(){return shunt;}

	float getEnergy(){return energy;}


};

#endif /* !BATTERYINTERFACE_H */
