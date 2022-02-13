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


// will handle prepping of all telemetry -- especialy prepearing packets to send
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
		int processGeneralTelemetryRequest(client *cli);
		int processAttGPSRequest(client *cli);
		int processBatteryRequest(client *cli);
		int processPWMRequest(client *cli);
		int processIORequest(client *cli);


};

#endif /* !TELEMETRY_H */
