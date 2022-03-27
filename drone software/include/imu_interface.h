/*
 * attitudeReader.h
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef IMUINTERFACE_H
#define IMUINTERFACE_H

#include "../libraries/Raspberry-JY901-Serial/JY901.h"
#include <chrono>
#include <mutex>
#include <string>
#include <thread>

using namespace std;

enum IMU_Orientation{
	STANDART,
	X_Y_INVERTED
};

/**
 * wrapper for WT901B library
 * all getters are made thread safe
 */
class ImuInterface {
	private:
	const bool debug = true;
	CJY901 JY901;
	bool imuUp = false;
  static ImuInterface* imuInterface;
  static mutex mutexImuInterface;

	protected:
	thread loopThread;

	ImuInterface();
	mutex sensorMutex;

	public:
	/**
	 * main method used to access ImuInterface
	 * if instace wasn't created it will initialize
	 * ImuInterface
	 */
	static ImuInterface* GetInstance();

	/**
	 * attaches IMU
	 *
	 * @return bool - true if IMU was attached
	 */
	bool attachIMU(); // bind serial connection


	//CJY901 getSensor();

	double yawOffset = 0;
	double pitchOffset = 0;
	double rollOffset = 0;
	IMU_Orientation orientation;
	const bool usingSerial = false;

	// copy from library just adds mutex, to make sure there are no errors due to multitherading, as data is stored in structures, thus it is not strictly safe

	/**
	 * resetOrientation of the IMU as value
	 * of yaw, pitch and roll are calculated
	 * from position IMU was in when it was
	 * started
	 *
	 * @return bool - always true
	 */
	void setIMUOrientation(IMU_Orientation orientation);
	bool resetOrientation();
	double getTemp();						 // get temperature
	double getAccX();						 // get X-axis acceleration in multiples of g
	double getAccY();						 // get Y-axis acceleration in multiples of g
	double getAccZ();						 // get Z-axis acceleration in multiples of g
	double getGyroX();					 // get X-axis angular velocity - rad/s
	double getGyroY();					 // get Y-axis angular velocity - rad/s
	double getGyroZ();					 // get Z-axis angular velocity - rad/s
	double getRoll();						 // get X-axis(Roll) angle - deg
	double getPitch();					 // get Y-axis(Pitch) angle - geg
	double getYaw();						 // get Z-axis(Yaw) angle deg
	double getMagX();						 // get X-axis magnetic field
	double getMagY();						 // get Y-axis magnetic field
	double getMagZ();						 // get Z-axis magnetic field
	int getPressure();					 // get pressure(JY-901B)
	int getAltitude();					 // get altitude(JY-901B)
	double getQuater(string);		 // get quaternion
	milliseconds getLastTime();  // get last receive time
	bool getIMUStatus(); 				 // get status of IMU
};

#endif /* !IMUINTERFACE_H */
