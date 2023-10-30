/*
 * protocol_spec.h
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef PROTOCOL_SPEC_H
#define PROTOCOL_SPEC_H

#include <vector>
#define BUT_A 0x01
#define BUT_B 0x02
#define BUT_X 0x03
#define BUT_Y 0x04

#include <array>
#include <cstring>
#include <string>


#define MAX_SEND_MESSAGE_SIZE 255
#define MAX_MESSAGE_SIZE 510 // roughly 100 numbers with some metadata end terminators


/**
 * Enum to distinguish different type of controllers
 */
enum ControllerTypes{
	XBOX_CONTROLLER, PS4_DUALSHOCK
};

/**
 * Enum to make working with controller more friendly
 */
enum ControlSurface{
	L_ANALOG = 0x11, R_ANALOG = 0x12, L_TRIGGER = 0x13, R_TRIGGER = 0x14, L_BUMPER = 0x15, R_BUMPER = 0x16,
	X =0x01, Y =0x02, A =0x03, B =0x04,
	XBOX =0x07,START =0x05, SELECT =0x06,
	L_STICK_BUTTON = 0x08, R_STICK_BUTTON = 0x09,
	D_PAD = 0x10,
	NON_DEFINED
};


/**
 * structure to save data about server
 * it also contains information about message being read
 * currrently being read
 */
struct serverStruct {
	int curIndexInBuffer = 0; // position where we have left off, first not filled index
	unsigned char curMessageType = 0;
	unsigned char curMessagePriority = 0;
	unsigned int short curMessageSize = 0;
	// NOTE: cannot store data here as we should be process multiple request from client at the same time
	unsigned char curMessageBuffer[MAX_MESSAGE_SIZE + 5]; // will be used to load message during reading, if whole message hasn't arrive reader will continu where it left
};

// struct are nice, but i would end up wasting a lot of space


/**
 * Object used to store data waiting to be processed
 */
class ProcessingStructure {
	public:
	unsigned char messageType = 0;
	unsigned char messagePriority = 0;
	unsigned int short messageSize;
	unsigned char* messageBuffer;
	ProcessingStructure(unsigned char messageType, unsigned char messagePriority, unsigned int short messageBufferSize)
			: messageType(messageType)
			, messagePriority(messagePriority)
			, messageSize(messageBufferSize)
			, messageBuffer(new unsigned char[messageBufferSize]) {};

	unsigned char* getMessageBuffer()
	{
		return (unsigned char*)messageBuffer;
	};
};

/**
 * Object used to store data waiting to be send
 */
class SendingStructure {
	public:
	unsigned char messageType = 0;
	unsigned char messagePriority = 0;
	unsigned int short messageSize;
	unsigned char* messageBuffer;
	SendingStructure(unsigned char messageType, unsigned char messagePriority, unsigned int short messageBufferSize)
			: messageType(messageType)
			, messagePriority(messagePriority)
			, messageSize(messageBufferSize)
			, messageBuffer(new unsigned char[messageBufferSize]) {};

	unsigned char* getMessageBuffer()
	{
		return (unsigned char*)messageBuffer;
	};
};

using namespace std;

// const unsigned char terminator[5] = { 0x00, 0x00, 0xFF, 0xFF, 0xFF };
const unsigned char terminator[5] = { 0, 0, 255, 255, 255 };

// Settings

struct pSetCamera {
	int port;
	unsigned char ip[4];
};

struct pConStr {
	pair<float, float> lAnalog;
	uint lTrigger;
	uint lBumber;
	pair<float, float> rAnalog;
	uint rTrigger;
	uint rBumber;
	pair<uint, uint> dpad;
};

struct pConSpc {
	ControlSurface cs;
	int val;
};

struct pTeleIOStat {
	bool ina226Battery;
	bool ina226Internals;
	bool pca9685;
	bool wt901;
	bool gps;
	pTeleIOStat() {};
	pTeleIOStat(bool ina226Battery, bool ina226Internals, bool pca9685, bool wt901, bool gps)
			: ina226Battery(ina226Battery)
			, ina226Internals(ina226Internals)
			, pca9685(pca9685)
			, wt901(wt901)
			, gps(gps) {};
};

struct pTeleGPS {
	bool gpsUp;
	double latitude;
	double longitude;
	double altitude;
	int numberOfSatelites;
	double groundSpeed;
	double heading;

	pTeleGPS() {};
	pTeleGPS(bool gpsUp, double latitude, double longitude, double altitude, double groundSpeed, double heading, int numberOfSatelites)
			: gpsUp(gpsUp)
			, latitude(latitude)
			, longitude(longitude)
			, altitude(altitude)
			, numberOfSatelites(numberOfSatelites)
			, groundSpeed(groundSpeed)
			, heading(heading) {};
};

struct pTeleATT {
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
	pTeleATT() {};
	pTeleATT(double yaw, double pitch, double roll, double accX, double accY, double accZ, double gyroX, double gyroY, double gyroZ, double magX, double magY, double magZ, int pressure, double temp)
			: yaw(yaw)
			, pitch(pitch)
			, roll(roll)
			, accX(accX)
			, accY(accY)
			, accZ(accZ)
			, gyroX(gyroX)
			, gyroY(gyroY)
			, gyroZ(gyroZ)
			, magX(magX)
			, magY(magY)
			, magZ(magZ)
			, pressure(pressure)
			, temp(temp) {};
};

struct pTelePOW {
	float batVoltage;
	float batCurrent;
	float batPower;
	float intVoltage;
	float intCurrent;
	float intPower;
	pTelePOW() {};
	pTelePOW(float batVoltage, float batCurrent, float batPower, float intVoltage, float intCurrent, float intPower)
			: batVoltage(batVoltage)
			, batCurrent(batCurrent)
			, batPower(batPower)
			, intVoltage(intVoltage)
			, intCurrent(intCurrent)
			, intPower(intPower) {};
};

struct pTelePWM {
	unsigned char configuration;
	unsigned int short motorMS;
	unsigned char angle[16];
};

struct pTeleATTGPS {
	pTeleATT att;
	pTeleGPS gps;
};

struct pTeleGen {
	pTeleIOStat io;
	pTeleATT att;
	pTeleGPS gps;
	pTelePOW pow;
	pTelePWM pwm;
};

struct pTeleErr {
	unsigned int code;
	char message[60];
	pTeleErr(unsigned int code, string msg)
			: code(code)
	{
		strncpy(message, msg.c_str(), msg.size() > 60 ? 60 : msg.size());
	};
	pTeleErr(unsigned int code, char* msg)
			: code(code)
	{
		strncpy(message, msg, strlen(msg) > 60 ? 60 : strlen(msg));
	};
};

enum protocol_codes {
	// Settings
	P_PING = 0x01,
	P_SET_RESTART = 0x02,
	P_SET_SHUTDOW = 0x03,
	P_SET_DISCONNECT = 0x04,
	P_SET_CAMERA = 0x05,
	// Control
	P_CON_STR = 0x21,
	P_CON_SPC = 0x22,
	// Telemetry
	P_TELE_IOSTAT = 0x41,
	P_TELE_GEN = 0x42,
	P_TELE_ATTGPS = 0x43,
	P_TELE_POW = 0x44,
	P_TELE_PWM = 0x45,
	P_TELE_ERR = 0x81,
};

#endif /* !PROTOCOL_SPEC_H */
