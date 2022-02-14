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
	sendingStruct ss;
	pTeleErr errmsg;
	errmsg.code = code;
	errmsg.message = message;
	ss.messagePriority = 0x01;
	ss.messageType = 0x81;
	ss.cli = nullptr;
	ss.messageSize = sizeof(errmsg);
	memcpy(&errmsg, &ss.messageBuffer, sizeof(errmsg));
	SendingThreadPool::GetInstance()->scheduleToSendAll(ss);
	return 1;
}

int errMessageCli(client *cli, unsigned char code, string message){
	if(logOn){
		// TODO: log errors into file
	}
	sendingStruct ss;
	pTeleErr errmsg;
	errmsg.code = code;
	errmsg.message = message;
	ss.messagePriority = 0x01;
	ss.messageType = 0x81;
	ss.cli = cli;
	ss.messageSize = sizeof(errmsg);
	memcpy(&errmsg, &ss.messageBuffer, sizeof(errmsg));
	SendingThreadPool::GetInstance()->scheduleToSend(ss);
	return 1;
}


