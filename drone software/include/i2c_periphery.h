/*
 * sensor.h
 * Copyright (C) 2022 Kryštof Havránek <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef I2CPERIPHERY_H
#define I2CPERIPHERY_H

#include <periphery.h>


class I2CPeriphery : public Periphery {
	private:
		constexpr static char TAG[] = "I2CPeriphery";

	protected:
	uint8_t i2cBusAddress;

	public:
	virtual ~I2CPeriphery() {};

	I2CPeriphery() = default;
	I2CPeriphery(const I2CPeriphery&) = default;
	I2CPeriphery& operator=(const I2CPeriphery&) = delete;

	I2CPeriphery(uint8_t i2cBusAddress)
	{ // single periphery number
		this->i2cBusAddress = i2cBusAddress;
	}

	public:
	uint8_t getI2CBusAddress()
	{
		return i2cBusAddress;
	}

};

#endif /* !I2CPERIPHERY_H */
