/*
 * peripherials_manager.h
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef TELEMETRY_H
#define TELEMETRY_H

#include "ina226_decorator.h"
#include "ublox_gps_decorator.h"
#include "wt901b_decorator.h"
#include "protocol_spec.h"

using namespace std;

/**
 * class that aggregates all of drones telemetry
 */
class PeripherialsManager {
	private:
	static PeripherialsManager* telemetry;

	INA226Decorator* battery;
	INA226Decorator* powerLine;
	WT901Decorator* wt901b;
	UBloxGPSDecorator* ubloxGPS;




	PeripherialsManager();

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
	 * fills in pTeleBATT structure
	 *
	 * @return pTeleBATT - filled in structure
	 */
	pTeleBATT createTeleBattStuct();

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
	bool initializePeripherials(int imuAddress, int inaAddress, int pca9685Address);


	/**
	 * Verifies if all peripheries are connected
	 * to the Raspberry Pi
	 *
	 * uses i2cdetect command to discover all devices
	 *
	 * @return bool - true if all expected devices are connected
	 */
	bool checkPeripheriesStatus();

	/**
	 * fills in pTeleGen struct and sends it to client
	 *
	 * @param const client cli - client to which answer should be sends
	 */
	bool processGeneralPeripherialsManagerRequest(const client cli);

	/**
	 * fills in pTeleATTGPS struct and sends it to client
	 *
	 * @param const client cli - client to which answer should be sends
	 */
	bool processAttGPSRequest(const client cli);

	/**
	 * fills in pTeleBATT struct and sends it to client
	 *
	 * @param const client cli - client to which answer should be sends
	 */
	bool processBatteryRequest(const client cli);

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

#endif /* !TELEMETRY_H */
