/*
 * drone_telemetry.cpp
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "main_window.h"
#include "drone_telemetry.h"
#include "protocol_spec.h"
#include <cstring>
#include <iterator>
#include <string>

DroneTelemetry* DroneTelemetry::telemetry = nullptr;
mutex DroneTelemetry::telemetryMutex;

DroneTelemetry::DroneTelemetry(){
	data.gps.latitude = 0;
	data.gps.longitude = 0;
	data.gps.altitude = 0;
}

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
int DroneTelemetry::processGeneralTelemetry(ProcessingStructure* ps)
{
	telemetryMutex.lock();
	memcpy(&data, ps->getMessageBuffer(),  sizeof(data));
	gpsLastTimeReceived = clock();
	ioLastTimeReceived = clock();
	batteryLastTimeReceived = clock();
	attitudeLastTimeReceived = clock();
	pwmLastTimeReceived = clock();
	telemetryMutex.unlock();
	mainWindow->updateTelemetry(data, &telemetryMutex);
	return 1;
}



int DroneTelemetry::processAttGPS(ProcessingStructure* ps)
{
	telemetryMutex.lock();
	memcpy(&data.att, ps->getMessageBuffer(),  sizeof(data.att)); //  gps is right after att
	gpsLastTimeReceived = clock();
	telemetryMutex.unlock();
	mainWindow->updateTelemetry(data, &telemetryMutex);
	return 1;
}
int DroneTelemetry::processPOW(ProcessingStructure* ps)
{
	telemetryMutex.lock();
	memcpy(&data.pow, ps->getMessageBuffer(),  sizeof(data.pow));
	batteryLastTimeReceived = clock();
	telemetryMutex.unlock();
	mainWindow->updateTelemetry(data, &telemetryMutex);
	return 1;
}
int DroneTelemetry::processPWM(ProcessingStructure* ps)
{
	telemetryMutex.lock();
	memcpy(&data.pwm, ps->getMessageBuffer(),  sizeof(data.pwm));
	pwmLastTimeReceived = clock();
	telemetryMutex.unlock();
	mainWindow->updateTelemetry(data, &telemetryMutex);
	return 1;
}
int DroneTelemetry::processIO(ProcessingStructure* ps)
{
	telemetryMutex.lock();
	memcpy(&data.io, ps->getMessageBuffer(), sizeof(data.io));
	ioLastTimeReceived = clock();
	telemetryMutex.unlock();
	mainWindow->updateTelemetry(data, &telemetryMutex);
	return 1;
}

int DroneTelemetry::processError(ProcessingStructure* ps){
	char tmpArray[sizeof(ps->getMessageBuffer())-4]; // first 4 bytes corerspond to integrer of err_code, rest is error_message
	memcpy(&tmpArray, ps->getMessageBuffer()+4, sizeof(tmpArray));
	pTeleErr tmp(ps->getMessageBuffer()[0], tmpArray);
	mainWindow->displayError(pTeleErr(tmp.code, tmp.message));
	return 1;
}

pair<double, double> DroneTelemetry::getGPSPosition(){
	return pair<double, double>(data.gps.latitude, data.gps.longitude);
}

