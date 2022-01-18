/*
 * communication_interface.cpp
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "communication_interface.h"
#include <cerrno>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>

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

void CommunicationInterface::checkAndConnectClient(){
	while(true){
		sockaddr_in clientAdress;

		int newClientfd = accept(sockfd, (struct sockaddr *) &clientAdress, &clientLength);

		if(newClientfd > 0){
			clientAdresses.push_back(clientAdress);
			clientfds.push_back(newClientfd);
		}else if (errno != EWOULDBLOCK){
			cerr << "COMMUNICATION_INTERFACE | checkAndConnectClient | something failed, errno: " << errno << endl;
		}

		// accept()
		this_thread::sleep_for(chrono::milliseconds(500));

	}
}

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
	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
		cerr << "COMMUNICATION_INTERFACE | setupSocket | error with binding" << endl;
		return false;
	}
 	listen(sockfd,8);
	clientLength = sizeof(serv_addr); // same structure so no problem
	return true;
}
