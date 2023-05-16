/*
 * telemetry.h
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef TELEMETRY_H
#define TELEMETRY_H

#include "protocol_spec.h"

using namespace std;

/**
 * class that aggregates all of drones telemetry
 */
class Telemetry {
	private:
	static Telemetry* telemetry;
	static mutex telemetryMutex;

	Telemetry();

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
	 * main method used to access Telemetry
	 * if instace wasn't created it will
	 * initialize Telemetry
	 */
	static Telemetry* GetInstance();

	/**
	 * sets up all senors used for telemetry
	 */
	bool setUpSensors(int imuAddress, int inaAddress, int pca9685Address);


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
	bool processGeneralTelemetryRequest(const Client cli);

	/**
	 * fills in pTeleATTGPS struct and sends it to client
	 *
	 * @param const client cli - client to which answer should be sends
	 */
	bool processAttGPSRequest(const Client cli);

	/**
	 * fills in pTeleBATT struct and sends it to client
	 *
	 * @param const client cli - client to which answer should be sends
	 */
	bool processBatteryRequest(const Client cli);

	/**
	 * fills in pTelePWM struct and sends it to client
	 *
	 * @param const client cli - client to which answer should be sends
	 */
	bool processPWMRequest(const Client cli);

	/**
	 * fills in pTeleIO struct and sends it to client
	 *
	 * @param const client cli - client to which answer should be sends
	 */
	bool processIORequest(const Client cli);
};

#endif /* !TELEMETRY_H */
