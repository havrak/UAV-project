/*
 * peripherials_manager.h
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef PERIPHERIALS_MANAGER_H
#define PERIPHERIALS_MANAGER_H

#include "ina226_decorator.h"
#include "ublox_gps_decorator.h"
#include "wt901b_decorator.h"
#include "pca9685_decorator.h"
#include "protocol_spec.h"
#include <map>


/**
 * class that aggregates all of drones telemetry
 */
class PeripherialsManager {
	private:
	static PeripherialsManager* telemetry;
	static mutex peripherialsMutex;

	std::thread telemetryThread;

	// TODO: There should be generic periphery array to which we can add new peripheries
	INA226Decorator* vaBattery;
	INA226Decorator* vaInternals;
	PCA9685Decorator* pca9685;
	WT901Decorator* wt901b;
	UBloxGPSDecorator* ubloxGPS;




	PeripherialsManager();

	void telemetryThreadMethod();

	// No such conrete methods to get data should exist
	/**
	 * fills in pTeleATT structure
	 *
	 * @return pTeleATT - filled in structure
	 */
	pTeleATT createTeleAttStruct();

	/**
	 * fills in pTeleGPS structure
	 *
	 * @return pTeleGPS - filled in structure
	 */
	pTeleGPS createTeleGPSStruct();

	/**
	 * fills in pTelePOW structure
	 *
	 * @return pTelePOW - filled in structure
	 */
	pTelePOW createTelePOWStuct();

	/**
	 * fills in pTeleIOStat structure
	 *
	 * @return pTeleIOStat - filled in structure
	 */
	pTeleIOStat createTeleIOStatStruct();

	/**
	 * fills in pTelePWM structure
	 *
	 * @return pTelePWM - filled in structure
	 */
	pTelePWM createTelePWMStruct();

	public:
	/**
	 * main method used to access PeripherialsManager
	 * if instace wasn't created it will
	 * initialize PeripherialsManager
	 */
	static PeripherialsManager* GetInstance();

	/**
	 * sets up all senors used for telemetry
	 */
	bool initializePeripherials(uint8_t inaBatAddress, uint8_t inaPowerAddress, uint8_t pcaAddress, uint8_t imuAddress);


	/**
	 * Verifies if all peripheries are connected
	 * to the Raspberry Pi
	 *
	 * uses i2cdetect command to discover all devices
	 *
	 * @return bool - true if all expected devices are connected
	 */
	bool i2cScan();


	void resetIMUOrientation(){wt901b->resetOrientation();};

	void servoControlRequest(ProcessingStructure* ps){
		lock_guard<mutex> lock(peripherialsMutex);
			pca9685->processControl(ps);
	};

	void servoControllCalibrate(){
		lock_guard<mutex> lock(peripherialsMutex);
		pca9685->calibrateESC();
	};
	void servoControllArm(){
		lock_guard<mutex> lock(peripherialsMutex);
		pca9685->armESC();
	};
	void servoControllMin(){
		lock_guard<mutex> lock(peripherialsMutex);
		pca9685->slowDownToMin();
	};

	/**
	 * fills in pTeleGen struct and sends it to client
	 *
	 * @param const client cli - client to which answer should be sends
	 */
	bool processGeneralTelemetryRequest(const client cli);

	/**
	 * fills in pTeleATTGPS struct and sends it to client
	 *
	 * @param const client cli - client to which answer should be sends
	 */
	bool processAttGPSRequest(const client cli);

	/**
	 * fills in pTelePOW struct and sends it to client
	 *
	 * @param const client cli - client to which answer should be sends
	 */
	bool processPowerRequest(const client cli);

	/**
	 * fills in pTelePWM struct and sends it to client
	 *
	 * @param const client cli - client to which answer should be sends
	 */
	bool processPWMRequest(const client cli);

	/**
	 * fills in pTeleIO struct and sends it to client
	 *
	 * @param const client cli - client to which answer should be sends
	 */
	bool processIORequest(const client cli);
};

#endif /* !PERIPHERIALS_MANAGER_H */
