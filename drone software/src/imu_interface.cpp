/*
 * imuInterface.h
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "imu_interface.h"
#include <chrono>
#include <iostream>

ImuInterface* ImuInterface::imuInterface = nullptr;
mutex ImuInterface::mutexImuInterface;

ImuInterface::ImuInterface()
{
}

ImuInterface* ImuInterface::GetInstance()
{
	if (imuInterface == nullptr) {
		mutexImuInterface.lock();
		if (imuInterface == nullptr) imuInterface = new ImuInterface();
		mutexImuInterface.unlock();
	}
	return imuInterface;
}

bool ImuInterface::attachIMU(int address)
{
	bool tmp = JY901.startI2C(address);

	if(!tmp ) {
		if(debug) cout << "IMUINTERFACE | attachIMU | Failed to attach IMU " << endl;
		return false;
	}
	/* JY901.setD1mode(0x05); // change mode of D1 port to gps */
	/* JY901.saveConf(0); */

	if(debug) cout << "IMUINTERFACE | attachIMU | Status of IMU: " << tmp << endl;
	return tmp;

}

bool ImuInterface::resetOrientation(){
	if(usingSerial) lock_guard<mutex> mutex(sensorMutex);
	yawOffset = JY901.getYaw();
	pitchOffset = JY901.getPitch();
	rollOffset = JY901.getRoll();
	return true;
}

void ImuInterface::setIMUStatus(bool status){
		imuUp = status;
}

void ImuInterface::setIMUOrientation(IMU_Orientation orientation){
	this->orientation = orientation;
}

bool ImuInterface::getIMUStatus(){
	return imuUp;
}

double ImuInterface::getTemp()
{
	if(usingSerial) lock_guard<mutex> mutex(sensorMutex);
	return JY901.getTemp();
}

double ImuInterface::getAccX() {
	if(usingSerial) lock_guard<mutex> mutex(sensorMutex);
	return JY901.getAccX();
}  // getAccX() unit: G(gravity)

double ImuInterface::getAccY() {
	if(usingSerial) lock_guard<mutex> mutex(sensorMutex);
	return JY901.getAccY();
}  // getAccY() unit: G(gravity)

double ImuInterface::getAccZ() {
	if(usingSerial) lock_guard<mutex> mutex(sensorMutex);
	return JY901.getAccZ();
}  // getAccZ() unit: G(gravity)

double ImuInterface::getGyroX() {
	if(usingSerial) lock_guard<mutex> mutex(sensorMutex);
	return JY901.getGyroX();
}  // getGyroX() unit: degree(s) per second

double ImuInterface::getGyroY() {
	if(usingSerial) lock_guard<mutex> mutex(sensorMutex);
	return JY901.getGyroY();
}  // getGyroY() unit: degree(s) per second

double ImuInterface::getGyroZ() {
	if(usingSerial) lock_guard<mutex> mutex(sensorMutex);
	return JY901.getGyroZ();
}  // getGyroZ() unit: degree(s) per second


// pitch and roll is switched on hardware
double ImuInterface::getRoll() {  // X-axis
	if(usingSerial) lock_guard<mutex> mutex(sensorMutex);
	if(orientation== X_Y_INVERTED){
		return JY901.getPitch() - pitchOffset;
	}else{
		return JY901.getRoll() - rollOffset;

	}
}  // getRoll() unit: degree(s)

double ImuInterface::getPitch() {  // Y-axis
	if(usingSerial) lock_guard<mutex> mutex(sensorMutex);
	if(orientation== X_Y_INVERTED){
	return JY901.getRoll() - rollOffset;
	}else{
	return JY901.getPitch() - pitchOffset;

	}
}  // getPitch() unit: degree(s)

double ImuInterface::getYaw() {  // Z-axis
	if(usingSerial) lock_guard<mutex> mutex(sensorMutex);
	return JY901.getYaw() - yawOffset;
}  // getYaw() unit: degree(s)

double ImuInterface::getMagX() {
	if(usingSerial) lock_guard<mutex> mutex(sensorMutex);
	return JY901.getMagX();
}  // getMagX()

double ImuInterface::getMagY() {
	if(usingSerial) lock_guard<mutex> mutex(sensorMutex);
	return JY901.getMagY();
}  // getMagY()

double ImuInterface::getMagZ() {
	if(usingSerial) lock_guard<mutex> mutex(sensorMutex);
	return JY901.getMagZ();
}  // getMagZ()


int ImuInterface::getPressure() {
	if(usingSerial) lock_guard<mutex> mutex(sensorMutex);
	return JY901.getPressure();
}  // getPressure() unit: Pa

int ImuInterface::getAltitude() {
	if(usingSerial) lock_guard<mutex> mutex(sensorMutex);
	return JY901.getAltitude();
}  // getAltitude() unit: cm


double ImuInterface::getQuater(string str) {
	if(usingSerial) lock_guard<mutex> mutex(sensorMutex);
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


milliseconds ImuInterface::getLastTime() {
	if(usingSerial) lock_guard<mutex> mutex(sensorMutex);
	return JY901.getLastTime();
}  // get last receive time

