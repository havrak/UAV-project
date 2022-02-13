/*
 * errorHandeling.h
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef ERROR_HANDELING_H
#define ERROR_HANDELING_H

#include "protocol_spec.h"
#include "communication_interface.h"

// this won't be a class since that would be a waste of space
// no need to have error handler or whatever when simple methods will do just fine



static mutex logFileMutex;
const bool logOn = false;

// -> err -> sendingStruct -> SendingThreadPool -> sendToAll
int errMessageAll(unsigned char code, string message);


// -> err -> sendingStruct
int errMessageCli(client *cli , unsigned char code, string message);

#endif /* !ERROR_HANDELING_H */
