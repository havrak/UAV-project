/*
 * ina226_decorator.h
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "ina226_decorator.h"


INA226Decorator::INA226Decorator(uint8_t address): I2CPeriphery(address){
	initialize();
}



void INA226Decorator::read()
{
		ina226.ina226_read(&voltage, &current, &power, &shunt);
		energy = voltage * current * 24 * 365.25 / 1000000;
		printf("%s,%d,%.3f,%.3f,%.3f,%.3f,%.3f\n", buffer, (int)rawtime, voltage, current, voltage * current, shunt, energy);
}

bool INA226Decorator::initialize()
{
	if (ina226.attach(i2cBusAddress)) {
		ina226.ina226_calibrate(0.1, 1.0);
		ina226.ina226_configure(ina226.INA226_TIME_8MS, ina226.INA226_TIME_8MS, ina226.INA226_AVERAGES_16, ina226.INA226_MODE_SHUNT_BUS_CONTINUOUS);
		error = false;
		return true;
	} else {
		cout << "BATTERYINTERFACE | attachINA226 | Failed to attach unit" << endl;
		error = true;
		return false;
	}
}
