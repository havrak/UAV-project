/*
 * communication_interface.h
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef COMMUNICATION_INTERFACE_H
#define COMMUNICATION_INTERFACE_H

#include <iostream>
#include <locale>
#include <mutex>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <list>
#include <thread>
#include <unistd.h>

#include "protocol_codes.h"

using namespace std;

#define SERVERPORT 8066
#define MAX_CLIENTS 10

//TODO: enable support for multiple clients
class CommunicationInterface{
	private:

		struct client{
			int fd = -1 ;
			sockaddr_in adress;
			bool readyToSend = true;
			// NOTE: cannot store data here as we should be process multiple request from client at the same time
		};

		struct sockaddr_in serv_addr;
		//, cli_addr;
		//int n;
		int sockfd, newsockfd;

		// sets do to
		fd_set read_fds;
		fd_set write_fds;
		fd_set except_fds;

		socklen_t clientLength;

		list<mutex> clientMutexes;
		list<client> clients;

		thread clientConnectThread;
		thread checkForNewDataThread; // will process new data and send it to thread pool, which will redistribute it
    //socklen_t clilen;

		static CommunicationInterface* communicationInterface;
		static mutex mutexCommunicationInterface;
		void checkAndConnectClient();
		int buildFdSets();
		int newClientConnect();

	public:
		static CommunicationInterface* GetInstance();
		bool setupSocket();
		void checkActivityOnSocket();
		void checkForNewData(); // -> calls callback if new data is found, that processes it (needs to be really fast, will not start new thread just for processing)
		void cleanUp();
};

#endif /* !COMMUNICATION_INTERFACE_H */
