/*
 * imuInterface.h
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "imuInterface.h"
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
		mutexImuInterface.lock(); // just so the this just doesn't couse
		if (imuInterface == nullptr) imuInterface = new ImuInterface();
		mutexImuInterface.unlock();
	}
	return imuInterface;
}

bool ImuInterface::attachIMU()
{
	bool status = JY901.attach("/dev/ttyS0");
	if(!status ) {
		if(debug) cout << "IMUINTERFACE | attachIMU | Failed to attach IMU " << endl;
		return false;
	}
	/* status = JY901.changeBaudRate(115200); */
	JY901.setGPSrate(9600);
	if(debug) cout << "IMUINTERFACE | attachIMU | Status of IMU: " << status << endl;
	return status;
}

void ImuInterface::updateFunction()
{
	cout << "Loop" << endl;

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

short ImuInterface::getD0Status() {
	sensorMutex.lock();
	double toReturn = JY901.getD0Status();
	sensorMutex.unlock();
  return toReturn;
}
short ImuInterface::getD1Status() {
	sensorMutex.lock();
	double toReturn = JY901.getD1Status();
	sensorMutex.unlock();
  return toReturn;
}
short ImuInterface::getD2Status() {
	sensorMutex.lock();
	double toReturn = JY901.getD2Status();
	sensorMutex.unlock();
  return toReturn;
}
short ImuInterface::getD3Status() {
	sensorMutex.lock();
	double toReturn = JY901.getD3Status();
	sensorMutex.unlock();
  return toReturn;
}

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

int ImuInterface::getLon() {
	sensorMutex.lock();
	double toReturn = JY901.getLon();
	sensorMutex.unlock();
  return toReturn;
}
int ImuInterface::getLat() {
	sensorMutex.lock();
	double toReturn = JY901.getLat();
	sensorMutex.unlock();
  return toReturn;
}

double ImuInterface::getGPSH() {
	sensorMutex.lock();
	double toReturn = JY901.getGPSH();
	sensorMutex.unlock();
  return toReturn;
}  // get GPS Height, unit: m(meters)

double ImuInterface::getGPSY() {
	sensorMutex.lock();
	double toReturn = JY901.getGPSY();
	sensorMutex.unlock();
  return toReturn;
}  // get GPS Yaw, unit: degree(s)

double ImuInterface::getGPSV() {
	sensorMutex.lock();
	double toReturn = JY901.getGPSV();
	sensorMutex.unlock();
  return toReturn;
}  // get GPS Velocity, unit: kilometers per hour

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

double ImuInterface::getDOP(string str) {
	double toReturn =0;
	sensorMutex.lock();
  if (str.compare("sn") == 0)
    toReturn = JY901.getDOP("sn");  // get sn
  if (str.compare("pdop") == 0)
    toReturn = JY901.getDOP("pdop");  // get pdop
  if (str.compare("hdop") == 0)
    toReturn = JY901.getDOP("hdop");  // get hdop
  if (str.compare("vdop") == 0)
    toReturn = JY901.getDOP("vdop");  // get vdop
	sensorMutex.unlock();
  return toReturn;

}  // getDOP()

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
