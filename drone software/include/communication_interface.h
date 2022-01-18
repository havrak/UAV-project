/*
 * communication_interface.h
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef COMMUNICATION_INTERFACE_H
#define COMMUNICATION_INTERFACE_H

#include <iostream>
#include <mutex>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <list>
#include <unistd.h>

using namespace std;

#define SERVERPORT 8066
//TODO: enable support for multiple clients
class CommunicationInterface{
	private:
		struct sockaddr_in serv_addr;
		//, cli_addr;
		//int n;
		int sockfd, newsockfd;
		socklen_t clientLength;
		list<int> clientfds;
		list<sockaddr_in> clientAdresses;
    //socklen_t clilen;

		static CommunicationInterface* communicationInterface;
		static mutex mutexCommunicationInterface;
		void checkAndConnectClient();

	public:
		static CommunicationInterface* GetInstance();
		bool setupSocket();
};

#endif /* !COMMUNICATION_INTERFACE_H */
