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
class Telemetry{
	private:
		static Telemetry* telemetry;
		static mutex telemetryMutex;
		Telemetry();

		pTeleATT createTeleAttStruct();
		pTeleGPS createTeleGPSStruct();
		pTeleBATT createTeleBattStuct();
		pTeleIOStat createTeleIOStatStruct();
		pTelePWM createTelePWMStruct();

	public:
		static Telemetry* GetInstance();
		int setUpSensors();
		int processGeneralTelemetryRequest(const client *cli);
		int processAttGPSRequest(const client *cli);
		int processBatteryRequest(const client *cli);
		int processPWMRequest(const client *cli);
		int processIORequest(const client *cli);


};

#endif /* !TELEMETRY_H */
