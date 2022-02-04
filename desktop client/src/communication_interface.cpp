/*
 * communication_interface.cpp
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "communication_interface.h"
#include <netinet/in.h>
#include <sys/socket.h>

CommunicationInterface* CommunicationInterface::communicationInterface = nullptr;
mutex CommunicationInterface::mutexCommunicationInterface;
ControllerDroneBridge* ControllerDroneBridge::controllerDroneBridge = nullptr;
mutex ControllerDroneBridge::mutexControllerDroneBridge;

// CommunicationInterface::CommunicationInterface()

//{
//}

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
bool CommunicationInterface::sendData(protocol_codes p, unsigned char priority, unsigned char* data)
{
	if(sizeof(data)+10 > MAX_MESSAGE_SIZE) { // we don't care about meta for now
		cerr << "CONTROLLER_INTERFACE | sendData | data is over the size limit (65KB)" << endl;
		return false;
	}

	char message[sizeof(data)+10];
	message[0] = p;
	message[1] = priority;
	message[2] = sizeof(data) >> 8;
	message[3] = sizeof(data) - (message[2] << 8);


	serverMutex.lock();
	message[4]
  if(send(sockfd, message, sizeof(message),0) <0){
			//
	}
}

// NOTE: will be thread that will end once communication is established
bool CommunicationInterface::establishConnectionToDrone()
{
	server.sin_family = AF_INET;
	while (true) {
		// NOTE; user will be able to change parametrhers, thus sockaddr_in in needs to be recreated on each iteration
		serverMutex.lock();
		server.sin_port = SERVERPORT;
		inet_aton(serverIp.c_str(), (struct in_addr*)&server.sin_addr.s_addr);
		serverMutex.unlock();

		if (connect(sockfd, (struct sockaddr*)&server, sizeof(server)) < 0) {
			cout << "COMMUNICATION_INTERFACE | establishConnectionToDrone | connection established" << endl;
			break;
		} else {
			cerr << "CONTROLLER_INTERFACE | establishConnectionToDrone | failed to establish connection" << endl;
		}
	}
	return true;
}

bool CommunicationInterface::setupSocket()
{
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		cerr << "COMMUNICATION_INTERFACE | setupSocket | failed to setup socket" << endl;
		return false;
	}

	establishConnectionToDroneThread = thread(&CommunicationInterface::establishConnectionToDrone, this);
	return true;
}

ControllerDroneBridge* ControllerDroneBridge::GetInstance()
{
	if (controllerDroneBridge == nullptr) {
		mutexControllerDroneBridge.lock();
		if (controllerDroneBridge == nullptr)
			controllerDroneBridge = new ControllerDroneBridge();
		mutexControllerDroneBridge.unlock();
	}
	return controllerDroneBridge;
}
