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

bool ImuInterface::attachIMU()
{
	bool status = JY901.startI2C(0x50);

	if(!status ) {
		if(debug) cout << "IMUINTERFACE | attachIMU | Failed to attach IMU " << endl;
		return false;
	}
	JY901.setD1mode(0x05); // change mode of D1 port to gps
	JY901.saveConf(0);

	if(debug) cout << "IMUINTERFACE | attachIMU | Status of IMU: " << status << endl;
	return status;

}

void ImuInterface::updateFunction()
{

	while (true) {
		sensorMutex.lock();
		JY901.receiveSerialData();
		sensorMutex.unlock();
		this_thread::sleep_for(chrono::milliseconds(pollingDelay));
	}
}

int ImuInterface::getPollingDelay()
{
	return pollingDelay;
}

void ImuInterface::setPollingDelay(int newPollingDelay)
{
	pollingDelay = newPollingDelay;
}

void ImuInterface::startLoop()
{
	loopThread = thread(&ImuInterface::updateFunction, this);

}


double ImuInterface::getTemp()
{
	sensorMutex.lock();
	double toReturn = JY901.getTemp();
	sensorMutex.unlock();
	return toReturn;
}

double ImuInterface::getAccX() {
	sensorMutex.lock();
	double toReturn = JY901.getAccX();
	sensorMutex.unlock();
  return toReturn;
}  // getAccX() unit: G(gravity)

double ImuInterface::getAccY() {
	sensorMutex.lock();
	double toReturn = JY901.getAccY();
	sensorMutex.unlock();
  return toReturn;
}  // getAccY() unit: G(gravity)

double ImuInterface::getAccZ() {
	sensorMutex.lock();
	double toReturn = JY901.getAccZ();
	sensorMutex.unlock();
  return toReturn;
}  // getAccZ() unit: G(gravity)

double ImuInterface::getGyroX() {
	sensorMutex.lock();
	double toReturn = JY901.getGyroX();
	sensorMutex.unlock();
  return toReturn;
}  // getGyroX() unit: degree(s) per second

double ImuInterface::getGyroY() {
	sensorMutex.lock();
	double toReturn = JY901.getGyroY();
	sensorMutex.unlock();
  return toReturn;
}  // getGyroY() unit: degree(s) per second

double ImuInterface::getGyroZ() {
	sensorMutex.lock();
	double toReturn = JY901.getGyroZ();
	sensorMutex.unlock();
  return toReturn;
}  // getGyroZ() unit: degree(s) per second
double ImuInterface::getRoll() {  // X-axis
	sensorMutex.lock();
	double toReturn = JY901.getRoll();
	sensorMutex.unlock();
  return toReturn;
}  // getRoll() unit: degree(s)

double ImuInterface::getPitch() {  // Y-axis
	sensorMutex.lock();
	double toReturn = JY901.getPitch();
	sensorMutex.unlock();
  return toReturn;
}  // getPitch() unit: degree(s)

double ImuInterface::getYaw() {  // Z-axis
	sensorMutex.lock();
	double toReturn = JY901.getYaw();
	sensorMutex.unlock();
  return toReturn;
}  // getYaw() unit: degree(s)

double ImuInterface::getMagX() {
	sensorMutex.lock();
	double toReturn = JY901.getMagX();
	sensorMutex.unlock();
  return toReturn;
}  // getMagX()

double ImuInterface::getMagY() {
	sensorMutex.lock();
	double toReturn = JY901.getMagY();
	sensorMutex.unlock();
  return toReturn;
}  // getMagY()

double ImuInterface::getMagZ() {
	sensorMutex.lock();
	double toReturn = JY901.getMagZ();
	sensorMutex.unlock();
  return toReturn;
}  // getMagZ()


int ImuInterface::getPressure() {
	sensorMutex.lock();
	double toReturn = JY901.getPressure();
	sensorMutex.unlock();
  return toReturn;
}  // getPressure() unit: Pa

int ImuInterface::getAltitude() {
	sensorMutex.lock();
	double toReturn = JY901.getAltitude();
	sensorMutex.unlock();
  return toReturn;
}  // getAltitude() unit: cm


double ImuInterface::getQuater(string str) {
	double toReturn =0;
	sensorMutex.lock();
  if (str.compare("q0") == 0)
    toReturn = JY901.getQuater("q0");  // get q0
  if (str.compare("q1") == 0)
    toReturn = JY901.getQuater("q1");  // get q1
  if (str.compare("q2") == 0)
    toReturn = JY901.getQuater("q2");  // get q2
  if (str.compare("q3") == 0)
    toReturn = JY901.getQuater("q3");  // get q3
	sensorMutex.unlock();
  return toReturn;
}  // getQuater()


milliseconds ImuInterface::getLastTime() {
	sensorMutex.lock();
	milliseconds toReturn = JY901.getLastTime();
	sensorMutex.unlock();
  return toReturn;
}  // get last receive time

/* ----------------- Get Raw data if needed ----------------- */
short ImuInterface::getAccRawX() {
	sensorMutex.lock();
	short toReturn = JY901.getAccRawX();
	sensorMutex.unlock();
  return toReturn;
}
short ImuInterface::getAccRawY() {
	sensorMutex.lock();
	short toReturn = JY901.getAccRawX();
	sensorMutex.unlock();
  return toReturn;
}
short ImuInterface::getAccRawZ() {
	sensorMutex.lock();
	short toReturn = JY901.getAccRawZ();
	sensorMutex.unlock();
  return toReturn;
}

short ImuInterface::getGyroRawX() {
	sensorMutex.lock();
	short toReturn = JY901.getGyroRawX();
	sensorMutex.unlock();
  return toReturn;
}
short ImuInterface::getGyroRawY() {
	sensorMutex.lock();
	short toReturn = JY901.getGyroRawY();
	sensorMutex.unlock();
  return toReturn;
}
short ImuInterface::getGyroRawZ() {
	sensorMutex.lock();
	short toReturn = JY901.getGyroRawZ();
	sensorMutex.unlock();
  return toReturn;
}

short ImuInterface::getMagRawX() {
	sensorMutex.lock();
	short toReturn = JY901.getMagRawX();
	sensorMutex.unlock();
  return toReturn;
}
short ImuInterface::getMagRawY() {
	sensorMutex.lock();
	short toReturn = JY901.getMagRawY();
	sensorMutex.unlock();
  return toReturn;
}
short ImuInterface::getMagRawZ() {
	sensorMutex.lock();
	short toReturn = JY901.getMagRawZ();
	sensorMutex.unlock();
  return toReturn;
}
