/*
 * errorHandeling.h
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef ERRORHANDELING_H
#define ERRORHANDELING_H

#include "protocol_spec.h"
#include "communication_interface.h"

// this won't be a class since that would be a waste of space
// no need to have error handler or whatever when simple methods will do just fine



static mutex logFileMutex;
const bool log = false;

int errMessageAll(int code, string message){
	if(log){

	}
	// -> err -> sendingStruct -> SendingThreadPool -> sendToAll

};

int errMessageCli(client *cli , int code, string message){
	if(log){
	}
	// -> err -> sendingStruct
}

#endif /* !ERRORHANDELING_H */
