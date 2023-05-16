/*
 * wt901_decorator.h
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef WT901_DECORATOR_H
#define WT901_DECORATOR_H

#include "JY901.h"
#include "../sensor.h"
#include <chrono>
#include <mutex>
#include <string>
#include <thread>

using namespace std;

enum IMU_Orientation {
	STANDART,
	X_Y_INVERTED
};

class WT901Decorator : Sensor {
	private:
	CJY901 JY901;

	public:
	WT901Decorator();
	/**
	 * attaches IMU
	 *
	 * @return bool - true if IMU was attached
	 */
	bool attachIMU(int address); // bind serial connection

	// CJY901 getSensor();

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
	bool resetOrientation();

	void setIMUOrientation(IMU_Orientation orientation);

	double getTemp(){return JY901.getTemp();};
	double getAccX(){return JY901.getAccX();};
	double getAccY(){return JY901.getAccY();};
	double getAccZ(){return JY901.getAccZ();};
	double getGyroX(){return JY901.getGyroX();};
	double getGyroY(){return JY901.getGyroY();};
	double getGyroZ(){return JY901.getGyroZ();};

	double getRoll()
	{
		if (orientation == X_Y_INVERTED) {
			return JY901.getPitch() - pitchOffset;
		} else {
			return JY901.getRoll() - rollOffset;
		}
	};

	double getPitch()
	{
		if (orientation == X_Y_INVERTED) {
			return JY901.getRoll() - rollOffset;
		} else {
			return JY901.getPitch() - pitchOffset;
		}
	};

	double getYaw(){ return JY901.getYaw() - yawOffset;}
	double getMagX(){return JY901.getMagX();};
	double getMagY(){return JY901.getMagY();};
	double getMagZ(){return JY901.getMagZ();};
	int getPressure(){return JY901.getPressure();};
	int getAltitude(){return JY901.getAltitude();};

	double getQuater(uint8_t quater)
	{
		switch (quater) {
		case 0:
			return JY901.getQuater("q0");
		case 1:
			return JY901.getQuater("q1");
		case 2:
			return JY901.getQuater("q2");
		case 3:
			return JY901.getQuater("q3");
		}
		return 0;
	};

	milliseconds getLastTime(){	return JY901.getLastTime();};
};

#endif /* !WT901_DECORATOR_H */
