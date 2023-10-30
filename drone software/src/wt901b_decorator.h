/*
 * imu_interface.h
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef WT901_DECORATOR_H
#define WT901_DECORATOR_H

#include "../libraries/raspberry-pi-wt901/JY901.h"
#include "i2c_periphery.h"
#include <chrono>
#include <iostream>
#include <mutex>
#include <string>

using namespace std;

enum IMU_Orientation { //TODO: move to Kconfig
	STANDART,
	X_Y_INVERTED
};

/**
 * wrapper for WT901B library
 * all getters are made thread safe
 */
class WT901Decorator : public I2CPeriphery {
	private:
	const bool debug = true;
	CJY901 JY901;

	double yawOffset = 0;
	double pitchOffset = 0;
	double rollOffset = 0;

	// GYRO
	double yaw;
	double pitch;
	double roll;
	// ACC
	double accX;
	double accY;
	double accZ;
	// GYRO ACC
	double gyroX;
	double gyroY;
	double gyroZ;
	// MAG
	double magX;
	double magY;
	double magZ;
	// BARO
	int pressure;
	// TEMP
	double temp;


	IMU_Orientation orientation;
	const bool usingSerial = false;

	public:
	WT901Decorator(uint8_t address, IMU_Orientation orientation);

	bool initialize() override;

	void read() override;

	/**
	 * resetOrientation of the IMU as value
	 * of yaw, pitch and roll are calculated
	 * from position IMU was in when it was
	 * started
	 *
	 * @return bool - always true
	 */
	bool resetOrientation();

	double getTemp(){return temp; }

	double getAccX(){return accX; } // getAccX() unit: G(gravity)

	double getAccY(){return accY;} // getAccY() unit: G(gravity)

	double getAccZ(){return accZ;} // getAccZ() unit: G(gravity)

	double getGyroX(){return accX;} // getGyroX() unit: degree(s) per second

	double getGyroY(){return accY;} // getGyroY() unit: degree(s) per second

	double getGyroZ(){return accZ;} // getGyroZ() unit: degree(s) per second

	double getRoll(){return roll;} // getRoll() unit: degree(s)

	double getPitch(){return pitch;} // getPitch() unit: degree(s)

	double getYaw(){return yaw;} // getYaw() unit: degree(s)

	double getMagX(){return magX;} // getMagX()

	double getMagY(){return magY;} // getMagY()

	double getMagZ(){return magZ;} // getMagZ()

	int getPressure(){return pressure;} // getPressure() unit: Pa

};

#endif /* !WT901_DECORATOR_H */
