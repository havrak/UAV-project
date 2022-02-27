/*
 * generic_PID.h
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef GENERIC_PID_H
#define GENERIC_PID_H

#include <chrono>
class GenericPID{
private:
	const float weightP;
	const float weightI;
	const float weightD;
	const float maxValOfIntegrator;

	float lowPass = 0.00795774715459476;// infintie impulse response filter (1/(40*pi)) for for 2 times 20Hz
	float integratorValue = 0;
	float proportionalValue = 0;
	float derivativeValue = 0;
	float lastError = 0;// last error for derivative
	float lastDerivative = 0;// last derivative for low-pass filter
	float pidOutput;

	clock_t lastTime;

public:
	GenericPID(float weightP, float weightI, float weightD, float maxValOfIntegrator): weightP(weightP), weightI(weightI), weightD(weightD), maxValOfIntegrator(maxValOfIntegrator){};

	float calculateOutput(float error);

};

#endif /* !GENERIC_PID_H */
