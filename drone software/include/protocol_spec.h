/*
 * protocol_spec.h
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef PROTOCOL_SPEC_H
#define PROTOCOL_SPEC_H

#include <mutex>
#include <netinet/in.h>
#include <type_traits>
#define BUT_A 0x01
#define BUT_B 0x02
#define BUT_X 0x03
#define BUT_Y 0x04
#define MAX_SEND_MESSAGE_SIZE 255
#define MAX_MESSAGE_SIZE 510 // roughly 100 numbers with some metadata end terminators
// 500 bytes for message, 10 for metadata

#include <cstring>
#include <string>

using namespace std;

const unsigned char terminator[5] = { 0x00, 0x00, 0xFF, 0xFF, 0xFF };

struct client {
	int fd = -1;
	sockaddr_in adress;
	mutex* cMutex;
	bool readyToSend = true;
	int noTriesToFix = 0;

	int curIndexInBuffer = 0; // position where we have left off, first not filled index
	unsigned char curMessageType = 0;
	unsigned char curMessagePriority = 0;
	unsigned int short curMessageSize = 0;
	// NOTE: cannot store data here as we should be process multiple request from client at the same time
	unsigned char curMessageBuffer[MAX_MESSAGE_SIZE + 5]; // will be used to load message during reading, if whole message hasn't arrive reader will continu where it left
	client(int fd, mutex* cMutex)
			: fd(fd)
			, cMutex(cMutex) {};
	client(int fd, sockaddr_in address, mutex* cMutex)
			: fd(fd)
			, adress(address)
			, cMutex(cMutex) {};
};

class ProcessingStructure {
	public:
	// const client* cli; // we need the client to know if he is ready to receive data

	int cfd;
	mutex* cMutex;

	const unsigned char messageType = 0;
	const unsigned char messagePriority = 0;
	unsigned int short messageSize;
	unsigned char* messageBuffer;
	ProcessingStructure(int cfd, mutex* cMutex, unsigned char messageType, unsigned char messagePriority, unsigned int short messageBufferSize)
			: cfd(cfd)
			, cMutex(cMutex)
			, messageType(messageType)
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
	int cfd;
	mutex* cMutex;

	const unsigned char messageType = 0;
	const unsigned char messagePriority = 0;
	unsigned int short messageSize;
	unsigned char* messageBuffer;
	SendingStructure(int cfd, mutex* cMutex, const unsigned char messageType, const unsigned char messagePriority, unsigned int short messageBufferSize)
			: cfd(cfd)
			, cMutex(cMutex)
			, messageType(messageType)
			, messagePriority(messagePriority)
			, messageSize(messageBufferSize)
			, messageBuffer(new unsigned char[messageBufferSize]) {};

	unsigned char* getMessageBuffer()
	{
		return (unsigned char*)messageBuffer;
	};
};

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
};

struct pTeleIOStat {
	bool ina226;
	bool pca9685;
	bool wt901;
	bool gps;
	pTeleIOStat(){};
	pTeleIOStat(bool ina226, bool pca9685, bool wt901, bool gps): ina226(ina226), pca9685(pca9685), wt901(wt901), gps(gps){};
};

struct pTeleGPS {
	bool gpsUp;
	double latitude;
	double longitude;
	double altitude;
	double numberOfSatelites;
	pTeleGPS(){};
	pTeleGPS(bool gpsUp, double latitude, double longitude, double altitude, double numberOfSatelites): gpsUp(gpsUp), latitude(latitude), longitude(longitude), altitude(altitude), numberOfSatelites(numberOfSatelites){};
};

struct pTeleATT {
	// ACC
	double accX;
	double accY;
	double accZ;
	// GYRO
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
	pTeleATT(){};
	pTeleATT(double accX, double accY, double accZ, double gyroX, double gyroY, double gyroZ, double magX, double magY, double magZ, int pressure, double temp): accX(accX), accY(accY), accZ(accZ), gyroX(gyroX), gyroY(gyroY), gyroZ(gyroZ), magX(magX), magY(magY), magZ(magZ), pressure(pressure), temp(temp) {};
};

struct pTeleBATT {
	float voltage;
	float current;
	float power;
	float shunt;
	float energy;
	pTeleBATT(){};
	pTeleBATT(float voltage, float current, float power, float shunt, float energy): voltage(voltage), current(current), power(power), shunt(shunt), energy(energy){};
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
			: code(code) { strncpy(message, msg.c_str(), msg.size() > 60 ? 60 : msg.size()) ;};
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
