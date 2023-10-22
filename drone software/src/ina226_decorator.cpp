/*
 * ina226_decorator.h
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "ina226_decorator.h"
#include <mutex>

INA226Decorator* INA226Decorator::batteryInterface = nullptr;
mutex INA226Decorator::mutexINA226Decorator;

INA226Decorator::INA226Decorator()
{
}

INA226Decorator* INA226Decorator::GetInstance()
{
	if (batteryInterface == nullptr) {
		cout << "BATTERYINTERFACE | GetInstance | INA226Decorator creation" << endl;
		mutexINA226Decorator.lock();
		if (batteryInterface == nullptr)
			batteryInterface = new INA226Decorator();
		mutexINA226Decorator.unlock();
		cout << "BATTERYINTERFACE | GetInstance | INA226Decorator created" << endl;
	}
	return batteryInterface;
}

bool INA226Decorator::attachINA226(int address)
{
	if (ina226.attach(address)) {
		ina226.ina226_calibrate(0.1, 1.0);
		ina226.ina226_configure(ina226.INA226_TIME_8MS, ina226.INA226_TIME_8MS, ina226.INA226_AVERAGES_16, ina226.INA226_MODE_SHUNT_BUS_CONTINUOUS);
		inaUp = true;
		return true;
	} else {
		if(debug) cout << "BATTERYINTERFACE | attachINA226 | Failed to attach unit" << endl;
		inaUp = false;
		return false;
	}
}

void INA226Decorator::updateFunction()
{
	while (true) {
		mutexINA226Decorator.lock();
		ina226.ina226_read(&voltage, &current, &power, &shunt);
		mutexINA226Decorator.unlock();
		energy = voltage * current * 24 * 365.25 / 1000000;
		// time(&rawtime);
		// struct tm* info = localtime(&rawtime);
		// strftime(buffer, 80, "%Y-%m-%d,%H:%M:%S", info);

		// printf("%s,%d,%.3f,%.3f,%.3f,%.3f,%.3f\n", buffer, (int)rawtime, voltage, current, voltage * current, shunt, energy);
		this_thread::sleep_for(chrono::milliseconds(pollingDelay));
	}
}

int INA226Decorator::getPollingDelay()
{
	return pollingDelay;
}

void INA226Decorator::setPollingDelay(int newPollingDelay)
{
	pollingDelay = newPollingDelay;
}

void INA226Decorator::startLoop()
{
	loopThread = thread(&INA226Decorator::updateFunction, this);
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

bool INA226Decorator::getINAStatus(){
	return inaUp;
}

void INA226Decorator::setINAStatus(bool status){
	inaUp = status;
}
