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

using namespace std;


// will handle prepping of all telemetry -- especialy prepearing packets to send
class Telemetry{
	private:
		static Telemetry* telemetry;
		static mutex telemetryMutex;
		Telemetry();

	public:
		static Telemetry* GetInstance();
		int setUpSensors();

};

#endif /* !TELEMETRY_H */
