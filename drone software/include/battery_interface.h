/*
 * attitudeReader.h
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef BATTERY_INTERFACE_H
#define BATTERY_INTERFACE_H

#include "../libraries/raspberry-pi-ina226/ina226.h"
#include <cmath>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <iostream>

#define INA_I2C_ADDRESS 0x44

using namespace std;

class BatteryInterface {
	private:
	//int fd;
	float voltage = 0;
	float current = 0;
	float power = 0;
	float shunt = 0;
	float energy =0;
	time_t rawtime =0;
	char buffer[80];
	int trig=1;

	bool inaUp = false;
	int pollingDelay = 1000;
	const bool debug = true;

	INA226 ina226;
  static BatteryInterface* batteryInterface;
  static mutex mutexBatteryInterface;

	protected:
	thread loopThread;
	BatteryInterface();

	/**
	 * TODO: should send notification if voltage is low
	 * starts loop that will periodically load info about power
	 */
	void updateFunction();

	public:
	/**
	 * main method used to access BatteryInterface
	 * if instace wasn't created it will initialize
	 * BatteryInterface
	 */
	static BatteryInterface* GetInstance();

	/**
	 * initializes INA226 library
	 *
	 * @return bool - true if connection was established
	 */
	bool attachINA226(int address);

	/**
	 * starts loop to periodically load data
	 */
	void startLoop();

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

	float getVoltage();
	float getCurrent();
	float getPower();
	float getShunt();
	float getEnergy();

	/**
	 * return whether connection to sensor was established
	 *
	 * @return bool - true if connection was established
	 */
	bool getINAStatus();

	/**
	 * set status of INA226 as it is telemetry.h
	 * who checks i2c devices
	 *
	 * @param bool status - new status
	 */
	void setINAStatus(bool status);

};

#endif /* !BATTERYINTERFACE_H */
