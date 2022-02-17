/*
 * drone_telemetry.cpp
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "drone_telemetry.h"
#include "protocol_spec.h"
#include <cstring>
#include <iterator>

DroneTelemetry* DroneTelemetry::telemetry = nullptr;
mutex DroneTelemetry::telemetryMutex;

DroneTelemetry::DroneTelemetry(){}

DroneTelemetry* DroneTelemetry::GetInstance()
{
	if (telemetry == nullptr) {
		telemetryMutex.lock();
		if (telemetry == nullptr) {
			telemetry = new DroneTelemetry();
		}
		telemetryMutex.unlock();
	}
	return telemetry;
}
int DroneTelemetry::processGeneralTelemetry(ProcessingStructure ps)
{
	telemetryMutex.lock();
	pTeleGen tmp;
	memcpy(&tmp, &ps.messageBuffer,  sizeof(ps.messageBuffer));
	attitude = tmp.att;
	gps = tmp.gps;
	battery = tmp.batt;
	ioStat = tmp.io;
	pwm = tmp.pwm;
	gpsLastTimeReceived = clock();
	ioLastTimeReceived = clock();
	batteryLastTimeReceived = clock();
	attitudeLastTimeReceived = clock();
	pwmLastTimeReceived = clock();
	telemetryMutex.unlock();
	return 1;
}
int DroneTelemetry::processAttGPS(ProcessingStructure ps)
{
	pTeleATTGPS attgps;
	telemetryMutex.lock();
	memcpy(&attgps, &ps.messageBuffer,  sizeof(ps.messageBuffer));
	attitude = attgps.att;
	gps = attgps.gps;
	gpsLastTimeReceived = clock();
	telemetryMutex.unlock();
	return 1;
}
int DroneTelemetry::processBattery(ProcessingStructure ps)
{
	telemetryMutex.lock();
	memcpy(&battery, &ps.messageBuffer, sizeof(ps.messageBuffer));
	batteryLastTimeReceived = clock();
	telemetryMutex.unlock();
	return 1;
}
int DroneTelemetry::processPWM(ProcessingStructure ps)
{
	telemetryMutex.lock();
	memcpy(&pwm, &ps.messageBuffer, sizeof(ps.messageBuffer));
	pwmLastTimeReceived = clock();
	telemetryMutex.unlock();
	return 1;
}
int DroneTelemetry::processIO(ProcessingStructure ps)
{
	telemetryMutex.lock();
	memcpy(&ioStat, &ps.messageBuffer, sizeof(ps.messageBuffer));
	ioLastTimeReceived = clock();
	telemetryMutex.unlock();
	return 1;
}

int DroneTelemetry::processError(ProcessingStructure ps){
	pTeleErr tmp;
	memcpy(&tmp, &ps.messageBuffer, sizeof(ps.messageBuffer));
	errorStackMutex.lock();
	errors.push(*(new ErrorMessage(true, tmp.code, tmp.message)));
	errorStackMutex.unlock();
	return 1;
}
