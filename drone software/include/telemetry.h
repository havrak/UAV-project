/*
 * telemetry.h
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef TELEMETRY_H
#define TELEMETRY_H

#include "battery_interface.h"
#include "gps_interface.h"
#include "imu_interface.h"
#include "protocol_spec.h"

using namespace std;

// will handle prepping of all telemetry -- especially prepearing packets to send

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
	bool setUpSensors();

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
