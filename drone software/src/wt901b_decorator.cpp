/*
 * imuInterface.h
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "wt901b_decorator.h"


WT901Decorator::WT901Decorator(uint8_t address, IMU_Orientation orientation):I2CPeriphery(address), orientation(orientation){
	initialize();
};

bool WT901Decorator::initialize(){
	error = !JY901.startI2C(i2cBusAddress);
	return !error;
}

bool WT901Decorator::resetOrientation(){

	yawOffset = JY901.getYaw();
	pitchOffset = JY901.getPitch();
	rollOffset = JY901.getRoll();
	return true;
}

void WT901Decorator::read(){
	if(error) return;
	yaw = JY901.getYaw() - yawOffset;
	pitch = JY901.getPitch() - pitchOffset;
	roll = JY901.getRoll() - rollOffset;
	temp = JY901.getTemp();
	if (orientation == X_Y_INVERTED) {
		roll = JY901.getPitch() - pitchOffset;
		pitch = JY901.getRoll() - rollOffset;
		accX = JY901.getAccY();
		accY = JY901.getAccX();
		gyroX = JY901.getGyroY();
		gyroY = JY901.getGyroX();
		magX = JY901.getMagY();
		magY = JY901.getMagX();
	} else {
		pitch = JY901.getPitch() - pitchOffset;
		roll = JY901.getRoll() - rollOffset;
		accX = JY901.getAccX();
		accY = JY901.getAccY();
		gyroX = JY901.getGyroX();
		gyroY = JY901.getGyroY();
		magX = JY901.getMagX();
		magY = JY901.getMagY();
	}
	accZ = JY901.getAccZ();
	gyroZ = JY901.getGyroZ();
	magZ = JY901.getMagZ();
	pressure = JY901.getPressure();

}

