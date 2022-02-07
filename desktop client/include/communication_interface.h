/*
 * communication_interface.h
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef COMMUNICATION_INTERFACE_H
#define COMMUNICATION_INTERFACE_H

#include "control_interpreter.h"
#include "protocol_codes.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string>
#include <cstring>
#include <iostream>
#include <mutex>
#include <thread>

#define SERVERPORT 8066
#define MAX_MESSAGE_SIZE 500

using namespace std;

class ControllerDroneBridge : ControlInterpreter{
	private:
		static ControllerDroneBridge* controllerDroneBridge;
		static mutex mutexControllerDroneBridge;

		bool active = true;

	public:
		void getActive();
		void setActive(bool active);
		static ControllerDroneBridge* GetInstance();


};

class CommunicationInterface{
	private:
		static CommunicationInterface* communicationInterface;
		static mutex mutexCommunicationInterface;
		static mutex serverMutex;

		fd_set read_fds;
		fd_set write_fds;
		fd_set except_fds;

		string serverIp = "192.168.6.1";
		int sockfd;
		sockaddr_in server;
		thread establishConnectionToDroneThread;

		int buildFdSets();


	public:
		static CommunicationInterface* GetInstance();
		bool setupSocket();
		bool establishConnectionToDrone();
		bool sendData(protocol_codes p, unsigned char priority, unsigned char *data);

};

#endif /* !COMMUNICATION_INTERFACE_H */
