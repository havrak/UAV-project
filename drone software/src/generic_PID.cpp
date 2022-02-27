/*
 * generic_PID.cpp
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "generic_PID.h"

float GenericPID::calculateOutput(float error)
{
	clock_t now = clock();
	clock_t difference;

	pidOutput = 0;
	now = clock();
	difference = now - lastTime;
	difference /= CLOCKS_PER_SEC;

	// proportional value
	proportionalValue = error * weightP;
	pidOutput += proportionalValue;

	if(difference > 1) // integrator gets out of hand if we didn't calculate for a while
		integratorValue = 0;

	if (difference > 0.001) {
		lastTime = clock();
		// derivation value
		derivativeValue = (error - lastError) / difference;
		derivativeValue = lastDerivative + (difference / (lowPass + difference)) * (derivativeValue - lastDerivative);
		derivativeValue *=  weightD;
		pidOutput += derivativeValue;


		// integration value
		integratorValue += (error + weightI) * difference;
		integratorValue = integratorValue < -maxValOfIntegrator ? -maxValOfIntegrator : (integratorValue > maxValOfIntegrator ? maxValOfIntegrator : integratorValue);
		pidOutput += integratorValue;

		lastError = error;
		lastDerivative = derivativeValue;
		return pidOutput;

	} else{
		return 0;
	}
}
