/*
 * batteryInterface.h
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "ina226_decorator.h"
#include <mutex>

INA226Decorator(uint8_t multiplexerPosition, uint8_t i2cAddress, uint8_t peripheryAddresses): Sensor(multiplexerPosition, i2cAddress, peripheryAddresses){
	initialize();
}

INA226Decorator(uint8_t multiplexerPosition, uint8_t i2cAddress, uint8_t peripheryAddresses, uint8_t peripherySubaddress): Sensor(multiplexerPosition, i2cAddress, peripheryAddresses, peripherySubaddress){
	initialize();

	};

bool INA226Decorator::initialize()
{
	if (ina226.attach(i2cAddress)) {
		ina226.ina226_calibrate(0.1, 1.0);
		ina226.ina226_configure(ina226.INA226_TIME_8MS, ina226.INA226_TIME_8MS, ina226.INA226_AVERAGES_16, ina226.INA226_MODE_SHUNT_BUS_CONTINUOUS);
		return true;
	} else {
		return false;
	}
}

bool INA226Decorator::read()
{
	ina226.ina226_read(&voltage, &current, &power, &shunt);
	energy = voltage * current * 24 * 365.25 / 1000000;
	return true;
}

float INA226Decorator::getVoltage()
{
	return voltage;
}

float INA226Decorator::getCurrent()
{
	return current;
}

float INA226Decorator::getPower()
{
	return power;
}

float INA226Decorator::getShunt()
{
	return shunt;
}

float INA226Decorator::getEnergy()
{
	return energy;
}
