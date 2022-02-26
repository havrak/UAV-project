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
	static ImuInterface* GetInstance();
	bool attachIMU(); // bind serial connection
	CJY901 getSensor();

	double yawOffset = 0;
	double pitchOffset = 0;
	double rollOffset = 0;
	const bool usingSerial = false;

	// copy from library just adds mutex, to make sure there are no errors due to multitherading, as data is stored in structures, thus it is not strictly safe
	bool resetOrientation();
	double getTemp();						 // get temperature
	double getAccX();						 // get X-axis acceleration
	double getAccY();						 // get Y-axis acceleration
	double getAccZ();						 // get Z-axis acceleration
	double getGyroX();					 // get X-axis angular velocity
	double getGyroY();					 // get Y-axis angular velocity
	double getGyroZ();					 // get Z-axis angular velocity
	double getRoll();						 // get X-axis(Roll) angle
	double getPitch();					 // get Y-axis(Pitch) angle
	double getYaw();						 // get Z-axis(Yaw) angle
	double getMagX();						 // get X-axis magnetic field
	double getMagY();						 // get Y-axis magnetic field
	double getMagZ();						 // get Z-axis magnetic field
	int getPressure();					 // get pressure(JY-901B)
	int getAltitude();					 // get altitude(JY-901B)
	double getQuater(string);		 // get quaternion
	milliseconds getLastTime(); // get last receive time
	short getAccRawX();					 // get X-axis raw acceleration data
	short getAccRawY();					 // get Y-axis raw acceleration data
	short getAccRawZ();					 // get Z-axis raw acceleration data
	short getGyroRawX();				 // get X-axis raw angular velocity data
	short getGyroRawY();				 // get Y-axis raw angular velocity data
	short getGyroRawZ();				 // get Z-axis raw angular velocity data
	short getMagRawX();					 // get X-axis raw magnetic field data
	short getMagRawY();					 // get Y-axis raw magnetic field data
	short getMagRawZ();					 // get Z-axis raw magnetic field data
	bool getIMUStatus();
};

#endif /* !IMUINTERFACE_H */
