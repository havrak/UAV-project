/*
 * drone_telemetry.cpp
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "drone_telemetry.h"
#include <cstring>

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
int DroneTelemetry::processGeneralTelemetry(processingStuct ps)
{
	telemetryMutex.lock();
	pTeleGen tmp;
	memcpy(&tmp, &ps.messageBuffer,  ps.messageSize);
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
int DroneTelemetry::processAttGPS(processingStuct ps)
{
	pTeleATTGPS tmp;
	telemetryMutex.lock();
	memcpy(&tmp, &ps.messageBuffer,  ps.messageSize);
	attitude = tmp.att;
	gps = tmp.gps;
	gpsLastTimeReceived = clock();
	telemetryMutex.unlock();
	return 1;
}
int DroneTelemetry::processBattery(processingStuct ps)
{
	telemetryMutex.lock();
	memcpy(&battery, &ps.messageBuffer, ps.messageSize);
	batteryLastTimeReceived = clock();
	telemetryMutex.unlock();
	return 1;
}
int DroneTelemetry::processPWM(processingStuct ps)
{
	telemetryMutex.lock();
	memcpy(&pwm, &ps.messageBuffer, ps.messageSize);
	pwmLastTimeReceived = clock();
	telemetryMutex.unlock();
	return 1;
}
int DroneTelemetry::processIO(processingStuct ps)
{
	telemetryMutex.lock();
	memcpy(&ioStat, &ps.messageBuffer, ps.messageSize);
	ioLastTimeReceived = clock();
	telemetryMutex.unlock();
	return 1;
}

int DroneTelemetry::processError(processingStuct ps){
	pTeleErr tmp;
	memcpy(&tmp, &ps.messageBuffer, ps.messageSize);
	errorStackMutex.lock();
	errors.push(*(new ErrorMessage(true, tmp.code, tmp.message)));
	errorStackMutex.unlock();
	return 1;
}
