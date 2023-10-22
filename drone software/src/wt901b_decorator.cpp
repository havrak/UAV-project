/*
 * imuInterface.h
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "wt901_decorator.h"



bool WT901Decorator::attachIMU(int address)
{
	bool tmp = JY901.startI2C(address);

	if(!tmp ) {
		if(debug) cout << "IMUINTERFACE | attachIMU | Failed to attach IMU " << endl;
		return false;
	}
	return tmp;

}

bool WT901Decorator::resetOrientation(){

	yawOffset = JY901.getYaw();
	pitchOffset = JY901.getPitch();
	rollOffset = JY901.getRoll();
	return true;
}

void WT901Decorator::setIMUStatus(bool status){
		imuUp = status;
}

void WT901Decorator::setIMUOrientation(IMU_Orientation orientation){
}


double WT901Decorator::getTemp()
{

	return JY901.getTemp();
}

double WT901Decorator::getAccX() {

	return JY901.getAccX();
}  // getAccX() unit: G(gravity)

double WT901Decorator::getAccY() {

	return JY901.getAccY();
}  // getAccY() unit: G(gravity)

double WT901Decorator::getAccZ() {

	return JY901.getAccZ();
}  // getAccZ() unit: G(gravity)

double WT901Decorator::getGyroX() {

	return JY901.getGyroX();
}  // getGyroX() unit: degree(s) per second

double WT901Decorator::getGyroY() {

	return JY901.getGyroY();
}  // getGyroY() unit: degree(s) per second

double WT901Decorator::getGyroZ() {

	return JY901.getGyroZ();
}  // getGyroZ() unit: degree(s) per second


// pitch and roll is switched on hardware
double WT901Decorator::getRoll() {  // X-axis

	if(orientation == X_Y_INVERTED){
		return JY901.getPitch() - pitchOffset;
	}else{
		return JY901.getRoll() - rollOffset;
	}
}  // getRoll() unit: degree(s)

double WT901Decorator::getPitch() {  // Y-axis

	if(orientation== X_Y_INVERTED){
	return JY901.getRoll() - rollOffset;
	}else{
	return JY901.getPitch() - pitchOffset;

	}
}  // getPitch() unit: degree(s)

double WT901Decorator::getYaw() {  // Z-axis

	return JY901.getYaw() - yawOffset;
}  // getYaw() unit: degree(s)

double WT901Decorator::getMagX() {

	return JY901.getMagX();
}  // getMagX()

double WT901Decorator::getMagY() {

	return JY901.getMagY();
}  // getMagY()

double WT901Decorator::getMagZ() {

	return JY901.getMagZ();
}  // getMagZ()


int WT901Decorator::getPressure() {

	return JY901.getPressure();
}  // getPressure() unit: Pa

int WT901Decorator::getAltitude() {

	return JY901.getAltitude();
}  // getAltitude() unit: cm


double WT901Decorator::getQuater(string str) {

  if (str.compare("q0") == 0)
     return JY901.getQuater("q0");  // get q0
  if (str.compare("q1") == 0)
     return JY901.getQuater("q1");  // get q1
  if (str.compare("q2") == 0)
     return JY901.getQuater("q2");  // get q2
  if (str.compare("q3") == 0)
     return JY901.getQuater("q3");  // get q3
	return 0;
}  // getQuater()


milliseconds WT901Decorator::getLastTime() {

	return JY901.getLastTime();
}  // get last receive time

