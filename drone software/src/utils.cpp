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
	ss.MessagePriority = 0x01;
	ss.MessageType = 0x81;
	ss.cli = nullptr;
	memcpy(&errmsg, &ss.messageBuffer, sizeof(errmsg));
	CommunicationInterface::GetInstance()->sendDataToAll(ss);
}

int errMessageCli(client *cli, unsigned char code, string message){
	if(logOn){
		// TODO: log errors into file
	}
	sendingStruct ss;
	pTeleErr errmsg;
	errmsg.code = code;
	errmsg.message = message;
	ss.MessagePriority = 0x01;
	ss.MessageType = 0x81;
	ss.cli = cli;
	memcpy(&errmsg, &ss.messageBuffer, sizeof(errmsg));
	CommunicationInterface::GetInstance()->sendDataToClient(ss);

}


