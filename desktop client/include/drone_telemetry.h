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



/**
 * Class that contains all telemetry from the drone, it also sends request
 * to update data on screen
 */
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

		/**
		 * main method used to access DroneTelemetry,
		 * if instace isn't created it will create it
		 */
		static DroneTelemetry* GetInstance();

		/**
		 * processes data from general telemetry packet
		 * after data is processed it will update data
		 * on screen
		 *
		 * @param ProcessingStructure ps - data to be processed
		 */
		int processGeneralTelemetry(ProcessingStructure ps);

		/**
		 * processes data from attitude+GPS telemetry packet
		 * after data is processed it will update data
		 * on screen
		 *
		 * @param ProcessingStructure ps - data to be processed
		 */
		int processAttGPS(ProcessingStructure ps);

		/**
		 * processes data from battery telemetry packet
		 * after data is processed it will update data
		 * on screen
		 *
		 * @param ProcessingStructure ps - data to be processed
		 */
		int processBattery(ProcessingStructure ps);

		/**
		 * processes data from pwm telemetry packet
		 * after data is processed it will update data
		 * on screen
		 *
		 * @param ProcessingStructure ps - data to be processed
		 */
		int processPWM(ProcessingStructure ps);

		/**
		 * processes data from io telemetry packet
		 * after data is processed it will update data
		 * on screen
		 *
		 * @param ProcessingStructure ps - data to be processed
		 */
		int processIO(ProcessingStructure ps);

		/**
		 * processes error packet
		 * after error is processed it will update data
		 * display on screen dialog with error message
		 *
		 * @param ProcessingStructure ps - data to be processed
		 */
		int processError(ProcessingStructure ps);

};

#endif /* !DRONE_TELEMETRY_H */

