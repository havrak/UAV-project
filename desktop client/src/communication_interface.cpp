/*
 * communication_interface.cpp
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "communication_interface.h"
#include "gtkmm/enums.h"
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

int CommunicationInterface::buildFdSets()
{
	FD_SET(STDIN_FILENO, &read_fds);
	FD_SET(sockfd, &read_fds);

	FD_ZERO(&read_fds);
	if (sockfd != -1)
		FD_SET(sockfd, &read_fds);

	FD_ZERO(&write_fds);

	if (sockfd != -1)
		FD_SET(sockfd, &write_fds);

	FD_ZERO(&except_fds);
	FD_SET(STDIN_FILENO, &except_fds);
	FD_SET(sockfd, &except_fds);

	if (sockfd != -1)
		FD_SET(sockfd, &except_fds);

	return 0;
}

bool CommunicationInterface::sendData(protocol_codes p, unsigned char priority, unsigned char* data)
{
	if (sizeof(data) + 10 > MAX_MESSAGE_SIZE) { // we don't care about meta for now
		cerr << "CONTROLLER_INTERFACE | sendData | data is over the size limit (0.5KB)" << endl;
		return false;
	}

	char message[sizeof(*data) + 10];

	// setup metadata
	message[0] = p;
	message[1] = priority;
	message[2] = sizeof(data) >> 8;
	message[3] = sizeof(data) - (message[2] << 8);
	message[4] = 7 - ((message[0] + message[1] + message[2] + message[3] + message[4]) % 7);

	// load message
	memcpy(message + 5, data, sizeof(*data)); // NOTE: clang gives waringing

	// setup terminator
	int li = sizeof(data) + 5;
	message[li + 1] = 0x00;
	message[li + 2] = 0x00;
	message[li + 3] = 0xFF;
	message[li + 4] = 0xFF;
	message[li + 5] = 0XFF;

	serverMutex.lock();
	ssize_t bytesSend = 0;
	bool sending = true;
	while (sending) {
		ssize_t sCount = send(sockfd, (char*)&message + bytesSend, (MAX_MESSAGE_SIZE < sizeof(*message) - bytesSend ? MAX_MESSAGE_SIZE : sizeof(*message) - bytesSend), 0);
		if ((sCount < 0 && errno != EAGAIN && errno != EWOULDBLOCK))
			return false;
		bytesSend += sCount;
		if (bytesSend == sizeof(*message)) {
			return true;
		}
	}
	serverMutex.unlock();
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
