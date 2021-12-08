/*
 * attitudeReader.h
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef BATTERYINTERFACE_H
#define BATTERYINTERFACE_H

#include "../libraries/raspberry-pi-ina226/ina226.h"
#include <cmath>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <iostream>

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

	int pollingDelay = 1000;
	const bool debug = true;

	INA226 ina226;
  static BatteryInterface* batteryInterface;
  static mutex mutexBatteryInterface;

	protected:
	thread loopThread;
	BatteryInterface();
	void updateFunction();

	public:
	static BatteryInterface* GetInstance();
	bool attachINA226(int i2cAddress);
	void startLoop();
	int getPollingDelay();
	void setPollingDelay(int pollingDelay);

	float getVoltage();
	float getCurrent();
	float getPower();
	float getShunt();
	float getEnergy();


};

#endif /* !BATTERYINTERFACE_H */
