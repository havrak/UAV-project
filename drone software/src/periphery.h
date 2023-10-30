/*
 * sensor.h
 * Copyright (C) 2022 Kryštof Havránek <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef PERIPHERY_H
#define PERIPHERY_H

#include <cstdint>
class Periphery {
	protected:
	bool active = true;
	bool error = true;

	bool subaddressUsed = false;
	uint16_t scanFrequency;

	public:
	virtual ~Periphery() {};

	Periphery() = default;
	Periphery(const Periphery&) = default;
	Periphery& operator=(const Periphery&) = default;

	virtual bool initialize() { return 0; }

	virtual bool addTasks() { return 0; }

	virtual uint8_t type() { return 0; }

	bool isActive() { return active; }

	virtual void read() {}

	virtual void setActive(bool active)
	{
		this->active = active;
	}

	bool getError()
	{
		return error;
	}

	bool setError(bool error)
	{
		if (!error && this->error) {
			initialize();
			return true;
		}
		this->error = error;
		return false;
	}

};

#endif /* !PERIPHERY_H */
