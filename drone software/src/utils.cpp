/*
 * utils.cpp
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "communication_interface.h"
#include "error_handeling.h"
#include "protocol_spec.h"

int errMessageAll(unsigned char code, string message){
	if(logOn){
		// TODO: log errors into file
	}
	SendingStructure ss(nullptr, P_TELE_ERR, 0x04, sizeof(message));
	memcpy(ss.messageBuffer, &message, sizeof(message));
	SendingThreadPool::GetInstance()->scheduleToSend(ss);
	return 1;
}

int errMessageCli(client *cli, unsigned char code, string message){
	if(logOn){
		// TODO: log errors into file
	}
	SendingStructure ss(cli, P_TELE_ERR, 0x04, sizeof(message));
	memcpy(ss.messageBuffer, &message, sizeof(message));
	SendingThreadPool::GetInstance()->scheduleToSend(ss);
	return 1;
}


