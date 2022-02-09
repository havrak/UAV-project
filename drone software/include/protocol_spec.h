/*
 * protocol_spec.h
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef PROTOCOL_SPEC_H
#define PROTOCOL_SPEC_H

#include <cstring>
#include <string>

using namespace std;

const unsigned char terminator[5] = {0x00, 0x00, 0xFF, 0xFF, 0xFF};

// Settings

struct pSetCamera{
	int port;
	string ip_addr;
};

struct pConStr{
	int lAnalogX;
	int lAnalogY;
	int lTrigger;
	int lBumber;
	int rAnalogX;
	int rAnalogY;
	int rTrigger;
	int rBumber;
};

struct pConSpc{
	bool A;
	bool B;
	bool X;
	bool Y;
};

struct pTeleIOStat{
	bool ina226;
	bool pca9685;
	bool wt901;
	bool gps;
};



struct pTeleErr{
	byte code;
	string message;
};

enum protocol_codes{
// Settings
	P_PING=0x01,
	P_SET_RESTART=0x02,
	P_SET_SHUTDOW=0x03,
	P_SET_DISCONNECT=0x04,
	P_SET_CAMERA=0x05,
// Control
	P_CON_STR=0x21,
	P_CON_SPC=0x22,
// Telemetry
	P_TELE_IOSTAT = 0x41,
	P_TELE_GEN = 0x42,
	P_TELE_ATTGPS = 0x43,
	P_TELE_BATT = 0x44,
	P_TELE_PWM = 0x45,
	P_TELE_ERR = 0x81,
};



#endif /* !PROTOCOL_SPEC_H */
