/*
 * batteryInterface.h
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "battery_interface.h"
#include <mutex>

BatteryInterface* BatteryInterface::batteryInterface = nullptr;
mutex BatteryInterface::mutexBatteryInterface;

BatteryInterface::BatteryInterface()
{
}

BatteryInterface* BatteryInterface::GetInstance()
{
	if (batteryInterface == nullptr) {
		cout << "BATTERYINTERFACE | GetInstance | BatteryInterface creation" << endl;
		mutexBatteryInterface.lock();
		if (batteryInterface == nullptr)
			batteryInterface = new BatteryInterface();
		mutexBatteryInterface.unlock();
		cout << "BATTERYINTERFACE | GetInstance | BatteryInterface created" << endl;
	}
	return batteryInterface;
}

bool BatteryInterface::attachINA226()
{
	if (ina226.attach(INA_I2C_ADDRESS)) {
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

void BatteryInterface::updateFunction()
{
	while (true) {
		mutexBatteryInterface.lock();
		ina226.ina226_read(&voltage, &current, &power, &shunt);
		mutexBatteryInterface.unlock();
		energy = voltage * current * 24 * 365.25 / 1000000;
		// time(&rawtime);
		// struct tm* info = localtime(&rawtime);
		// strftime(buffer, 80, "%Y-%m-%d,%H:%M:%S", info);

		// printf("%s,%d,%.3f,%.3f,%.3f,%.3f,%.3f\n", buffer, (int)rawtime, voltage, current, voltage * current, shunt, energy);
		this_thread::sleep_for(chrono::milliseconds(pollingDelay));
	}
}

int BatteryInterface::getPollingDelay()
{
	return pollingDelay;
}

void BatteryInterface::setPollingDelay(int newPollingDelay)
{
	pollingDelay = newPollingDelay;
}

void BatteryInterface::startLoop()
{
	loopThread = thread(&BatteryInterface::updateFunction, this);
}

float BatteryInterface::getVoltage()
{
	lock_guard<mutex> mutex(mutexBatteryInterface);
	return voltage;
}

float BatteryInterface::getCurrent()
{
	lock_guard<mutex> mutex(mutexBatteryInterface);
	return current;
}

float BatteryInterface::getPower()
{
	lock_guard<mutex> mutex(mutexBatteryInterface);
	return power;
}

float BatteryInterface::getShunt()
{
	lock_guard<mutex> mutex(mutexBatteryInterface);
	return shunt;
}

float BatteryInterface::getEnergy()
{
	lock_guard<mutex> mutex(mutexBatteryInterface);
	return energy;
}

bool BatteryInterface::getINAStatus(){
	lock_guard<mutex> mutex(mutexBatteryInterface);
	return inaUp;
}
