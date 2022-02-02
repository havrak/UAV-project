/*
 * communication_interface.cpp
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "communication_interface.h"
#include <cerrno>
#include <chrono>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

CommunicationInterface* CommunicationInterface::communicationInterface = nullptr;
mutex CommunicationInterface::mutexCommunicationInterface;

CommunicationInterface* CommunicationInterface::GetInstance()
{
	if (communicationInterface == nullptr) {
		mutexCommunicationInterface.lock();
		if (communicationInterface == nullptr)
			communicationInterface = new CommunicationInterface();
		mutexCommunicationInterface.unlock();
	}
	return communicationInterface;
}

void CommunicationInterface::cleanUp()
{
	for (client c : clients) {
		close(c.fd);
	}
	close(sockfd);
}

int CommunicationInterface::buildFdSets()
{
	FD_SET(STDIN_FILENO, &read_fds);
	FD_SET(sockfd, &read_fds);

	FD_ZERO(&read_fds);
	for (client c : clients)
		if (c.fd != -1)
			FD_SET(c.fd, &read_fds);

	FD_ZERO(&write_fds);

	for (client c : clients)
		if (c.fd != -1)
			FD_SET(c.fd, &write_fds);

	FD_ZERO(&except_fds);
	FD_SET(STDIN_FILENO, &except_fds);
	FD_SET(sockfd, &except_fds);

	for (client c : clients)
		if (c.fd != -1)
			FD_SET(c.fd, &except_fds);

	return 0;
}

void CommunicationInterface::checkActivityOnSocket()
{
	// unsigned char type, priority, checkVal;
	unsigned short int dataSize;
	// 0 -- type
	// 1 -- priority
	// 2 -- higher byte of size
	// 3 -- lower byte of size
	// 4 -- addition to checksum, we want sum to be divisible by 7
	char infoBuffer[5];
	int state;
	while (true) {
		buildFdSets();
		// select;
		state = select(sockfd, &read_fds, &write_fds, &except_fds, NULL);
		switch (state) {
		case -1:
			cerr << "COMMUNICATION_INTERFACE | checkActivityOnSocket | something went's wrong" << endl;
			break;
		case 0:
			cerr << "COMMUNICATION_INTERFACE | checkActivityOnSocket | something went's wrong" << endl;
		default:
			if (FD_ISSET(sockfd, &read_fds))
				newClientConnect();
			for (client c : clients) {
				if (FD_ISSET(c.fd, &read_fds)) { // TODO: this should take care of data split into multiple packets, or something else
						cout << "COMMUNICATION_INTERFACE | checkActivityOnSocket | got something" << endl;
					read(c.fd, infoBuffer, 5); // NOTE: can result in problem, if they arise rewrite it to standart method using recv()
					if ((infoBuffer[0] + infoBuffer[1] + infoBuffer[2] + infoBuffer[3] + infoBuffer[4]) % 7 != 0) {
						cerr << "COMMUNICATION_INTERFACE | checkActivityOnSocket | checksum of received data doesn't match" << endl;
						continue;
					}
					dataSize = (infoBuffer[3] << 8) + infoBuffer[4];
					if (infoBuffer[0] == P_PING ){
						// NOTE: send ping from here
					}else{
						char buffer[dataSize];
						read(c.fd, buffer, dataSize);
						// pushToQueue( char* buffer)
					}
				}
			}
		}

		//
		this_thread::sleep_for(chrono::milliseconds(10));
	}
}

/* void CommunicationInterface::checkForNewData(){ */
/* 	unsigned char type, priority; */
/* 	unsigned short int dataSize; */
/* 	while(true){ */
/* 		fd_set rfds; */

/*     FD_ZERO(&rfds); */
/*     FD_SET(sockfd, &rfds); */

/* 		/1* for(int clientfd : clientfds ){ *1/ */
/* 		/1* 	if(true){ *1/ */
/* 		/1* 	} *1/ */
/* 		/1* } *1/ */
/* 		this_thread::sleep_for(chrono::milliseconds(10)); */
/* 	} */
/* } */

int CommunicationInterface::newClientConnect()
{
	while (true) {
		struct sockaddr_in clientAddress;
		int clientfd = accept(sockfd, (struct sockaddr*)&clientAddress, &clientLength);
		if (clientfd <= 0) // we will end the loop once all clients waiting have been accepter
			break;

		char clientIPV4Address[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientAddress.sin_addr, clientIPV4Address, INET_ADDRSTRLEN);

		cout << "COMMUNICATION_INTERFACE | newClientConnect | ip: " << clientIPV4Address << ":" << clientAddress.sin_port << endl;
		struct client c;
		c.adress = clientAddress;
		c.fd = clientfd;
		clients.push_back(c);

		close(clientfd);
	}
	return -1;
}

/* void CommunicationInterface::checkAndConnectClient(){ */
/* 	checkForNewDataThread = thread(&CommunicationInterface::checkForNewData, this); */
/* 	while(true){ */
/* 		sockaddr_in clientAdress; */

/* 		int newClientfd = accept(sockfd, (struct sockaddr *) &clientAdress, &clientLength); */

/* 		if(newClientfd > 0){ */
/* 			clientAdresses.push_back(clientAdress); */
/* 			clientfds.push_back(newClientfd); */
/* 		}else if (errno != EWOULDBLOCK){ */
/* 			cerr << "COMMUNICATION_INTERFACE | checkAndConnectClient | something failed, errno: " << errno << endl; */
/* 		} */
/* 		this_thread::sleep_for(chrono::milliseconds(500)); */

/* 	} */
/* } */

bool CommunicationInterface::setupSocket()
{
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		cerr << "COMMUNICATION_INTERFACE | setupSocket | failed to setup socket" << endl;
		return false;
	}
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = SERVERPORT;
	if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		cerr << "COMMUNICATION_INTERFACE | setupSocket | error with binding" << endl;
		return false;
	}
	listen(sockfd, 8);
	clientLength = sizeof(serv_addr); // same structure so no problem
	/* clientConnectThread = thread(&CommunicationInterface::checkAndConnectClient, this); */
	return true;
}
