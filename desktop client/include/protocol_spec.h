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

struct serverStruct {
	int curIndexInBuffer = 0; // position where we have left off, first not filled index
	unsigned char curMessageType = 0;
	unsigned char curMessagePriority = 0;
	unsigned int short curMessageSize = 0;
	// NOTE: cannot store data here as we should be process multiple request from client at the same time
	unsigned char curMessageBuffer[MAX_MESSAGE_SIZE + 5]; // will be used to load message during reading, if whole message hasn't arrive reader will continu where it left
};

// struct are nice, but i would end up wasting a lot of space
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

/* class SendingStructure { */
/* 	public: */
/* 	unsigned char messageType = 0; */
/* 	unsigned char messagePriority = 0; */
/* 	unsigned int short messageSize; */
/* 	/1* unsigned char messageBuffer[]; *1/ */
/* 	std::vector<unsigned char> messageBuffer; */
/* 	SendingStructure(unsigned char messageType, unsigned char messagePriority, unsigned int short messageBufferSize) */
/* 			: messageType(messageType) */
/* 			, messagePriority(messagePriority) */
/* 			{messageBuffer.reserve(messageBufferSize);}; */
/* }; */

using namespace std;

// const unsigned char terminator[5] = { 0x00, 0x00, 0xFF, 0xFF, 0xFF };
const unsigned char terminator[5] = { 0, 0, 255, 255, 255 };

// Settings

struct pSetCamera {
	int port;
	unsigned char ip[4];
};

struct pConStr {
	pair<int, int> lAnalog;
	int lTrigger;
	int lBumber;
	pair<int, int> rAnalog;
	int rTrigger;
	int rBumber;
	pair<int, int> dpad;
};

struct pConSpc {
	unsigned char button;
	unsigned int state;
};

struct pTeleIOStat {
	bool ina226;
	bool pca9685;
	bool wt901;
	bool gps;
};

struct pTeleGPS {
	bool gpsUp;
	double latitude;
	double longitude;
	double altitude;
	double numberOfSatelites;
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
};

struct pTeleBATT {
	float getVoltage;
	float getCurrent;
	float getPower;
	float getShunt;
	float getEnergy;
};

struct pTelePWM {
	unsigned char configuration;
	unsigned int short motorMS;
	unsigned int short angle[16];
};

struct pTeleATTGPS {
	pTeleATT att;
	pTeleGPS gps;
};

struct pTeleGen {
	pTeleIOStat io;
	pTeleATT att;
	pTeleGPS gps;
	pTeleBATT batt;
	pTelePWM pwm;
};

struct pTeleErr {
	unsigned int code;
	char message[60];
	pTeleErr(unsigned int code, string msg)
			: code(code) { strncpy(message, msg.c_str(), msg.length() > 60 ? 60 : msg.length()) ;};
	pTeleErr(unsigned int code, char *msg)
			: code(code) { strncpy(message, msg, strlen(msg) > 60 ? 60 : strlen(msg));};
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
	P_TELE_BATT = 0x44,
	P_TELE_PWM = 0x45,
	P_TELE_ERR = 0x81,
};

#endif /* !PROTOCOL_SPEC_H */
