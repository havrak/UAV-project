/*
 * drone_telemetry.h
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef DRONE_TELEMETRY_H
#define DRONE_TELEMETRY_H

#include "protocol_spec.h"
#include <mutex>
#include <stack>

using namespace std;

enum wingSurfaceConfiguration{
	V_SHAPE_TAIL_WING = 1,
	STANDARD_TAIL_WING = 2,
};

struct StandardTailWingConfiguration{
	int leftFlapIndex = 1;
	int rightFlapIndex = 2;
	int leftElevatorIndex = 3;
	int rightElevatorIndex = 4;
	int rudderIndex = 5;
	unsigned int short leftFlap;
	unsigned int short rightFlap;
	unsigned int short leftElevator;
	unsigned int short rightElevator;
	unsigned int short rudder;

};

struct VShapeTailWingConfiguration{
	int leftFlapIndex = 1;
	int rightFlapIndex = 2;
	int leftRuddervatorIndex = 3;
	int rightRuddervatorIndex = 4;
	unsigned int short leftFlap;
	unsigned int short rightFlap;
	unsigned int short leftRuddervator;
	unsigned int short rightRuddervator;
};



// will handle prepping of all telemetry -- especially prepearing packets to send
class DroneTelemetry{
	private:
		static DroneTelemetry* telemetry;
		static mutex telemetryMutex;

		DroneTelemetry();
		wingSurfaceConfiguration configuration = V_SHAPE_TAIL_WING;

		pTeleGen data;

		pTeleBATT battery;
		clock_t batteryLastTimeReceived;
		pTeleATT attitude;
		clock_t attitudeLastTimeReceived;
		pTeleGPS gps;
		clock_t gpsLastTimeReceived;
		pTelePWM pwm;
		clock_t pwmLastTimeReceived;
		pTeleIOStat ioStat;
		clock_t ioLastTimeReceived;

	public:
		static DroneTelemetry* GetInstance();
		int setUpSensors();
		int processGeneralTelemetry(ProcessingStructure ps);
		int processAttGPS(ProcessingStructure ps);
		int processBattery(ProcessingStructure ps);
		int processPWM(ProcessingStructure ps);
		int processIO(ProcessingStructure ps);
		int processError(ProcessingStructure ps);

};

#endif /* !DRONE_TELEMETRY_H */

