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
	memcpy(&data, ps.getMessageBuffer(),  ps.messageSize);
	gpsLastTimeReceived = clock();
	ioLastTimeReceived = clock();
	batteryLastTimeReceived = clock();
	attitudeLastTimeReceived = clock();
	pwmLastTimeReceived = clock();
	telemetryMutex.unlock();
	mainWindow->updateData(data, &telemetryMutex);
	return 1;
}



int DroneTelemetry::processAttGPS(ProcessingStructure ps)
{
	telemetryMutex.lock();
	memcpy(&data.att, ps.getMessageBuffer(),  ps.messageSize); //  gps is right after att
	gpsLastTimeReceived = clock();
	telemetryMutex.unlock();
	mainWindow->updateData(data, &telemetryMutex);
	return 1;
}
int DroneTelemetry::processBattery(ProcessingStructure ps)
{
	telemetryMutex.lock();
	memcpy(&data.batt, ps.getMessageBuffer(),  ps.messageSize);
	batteryLastTimeReceived = clock();
	telemetryMutex.unlock();
	mainWindow->updateData(data, &telemetryMutex);
	return 1;
}
int DroneTelemetry::processPWM(ProcessingStructure ps)
{
	telemetryMutex.lock();
	memcpy(&data.pwm, ps.getMessageBuffer(),  ps.messageSize);
	pwmLastTimeReceived = clock();
	telemetryMutex.unlock();
	mainWindow->updateData(data, &telemetryMutex);
	return 1;
}
int DroneTelemetry::processIO(ProcessingStructure ps)
{
	telemetryMutex.lock();
	memcpy(&data.io, ps.getMessageBuffer(), ps.messageSize);
	ioLastTimeReceived = clock();
	telemetryMutex.unlock();
	mainWindow->updateData(data, &telemetryMutex);
	return 1;
}

int DroneTelemetry::processError(ProcessingStructure ps){
	char tmpArray[sizeof(ps.getMessageBuffer())-4]; // first 4 bytes corerspond to integrer of err_code, rest is error_message
	memcpy(&tmpArray, ps.getMessageBuffer()+4, sizeof(tmpArray));
	pTeleErr tmp(ps.getMessageBuffer()[0], tmpArray);
	mainWindow->displayError(pTeleErr(tmp.code, tmp.message));
	return 1;
}
