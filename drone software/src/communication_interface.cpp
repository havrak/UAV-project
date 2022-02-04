/*
 * communication_interface.cpp
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "communication_interface.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

CommunicationInterface* CommunicationInterface::communicationInterface = nullptr;
mutex CommunicationInterface::mutexCommunicationInterface;

ThreadPool* ThreadPool::threadPool = nullptr;

ThreadPool* ThreadPool::GetInstance(){
	if(threadPool == nullptr)
		threadPool = new ThreadPool();
	return threadPool;
}

CommunicationInterface* CommunicationInterface::GetInstance()
{
	if (communicationInterface == nullptr) {
		mutexCommunicationInterface.lock();
		if (communicationInterface == nullptr){
			communicationInterface = new CommunicationInterface();
			ThreadPool::GetInstance(); // let this one also create thread pool
		}
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

void CommunicationInterface::clearClientStruct(client cli)
{
	cli.curIndexInBuffer = 0; // position where we have left off
	cli.curMessageType = 0;
	cli.curMessagePriority = 0;
	cli.curMessageSize = 0;
	// NOTE: cannot store data here as we should be process multiple request from client at the same time
	memset(&cli.curMessageBuffer, 0, MAX_MESSAGE_SIZE);
}

bool CommunicationInterface::receiveDataFromClient(client cli)
{
	ssize_t bytesReceived = 0;
	bool receivingData = true;
	if (cli.curIndexInBuffer == 0) { // we are starting new message
		char infoBuffer[5];
		ssize_t tmp = recv(cli.fd, (char*)&infoBuffer, 5, MSG_DONTWAIT);
		if ((tmp < 0 && errno != EAGAIN && errno != EWOULDBLOCK) || tmp < 5) // as we call this method recursively we can get into position where there is nothing to read NOTE: check if this doesn't cause any problems
			return false;

		if ((infoBuffer[0] + infoBuffer[1] + infoBuffer[2] + infoBuffer[3] + infoBuffer[4]) % 7 != 0) { // just check if first few bytes look semi valid
			cerr << "COMMUNICATION_INTERFACE | checkActivityOnSocket | checksum of received data doesn't match" << endl;
			return false;
		}
		cli.curMessageSize = (infoBuffer[3] << 8) + infoBuffer[4];
		if (infoBuffer[0] == P_PING) {
			// NOTE: send ping from here
			return true;
		}
	}else{
		bytesReceived = cli.curIndexInBuffer;
	}

	while (receivingData) {
		ssize_t rCount = recv(cli.fd, (char*)&cli.curMessageBuffer + bytesReceived, cli.curMessageSize - cli.curIndexInBuffer, MSG_DONTWAIT);
		if (rCount < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
			cerr << "COMMUNICATION_INTERFACE | receiveDataFromClient | cannot read from client" << endl;
			clearClientStruct(cli);
			return true;
		}
		cli.curIndexInBuffer += rCount;

		if (cli.curIndexInBuffer == cli.curMessageSize) {
			// pushtToQueue();

			clearClientStruct(cli);
			receiveDataFromClient(cli);
			return true;
		}
	}
	return true;
}

void CommunicationInterface::checkActivityOnSocket()
{
	int state;
	// 0 -- type
	// 1 -- priority
	// 2 -- higher byte of size
	// 3 -- lower byte of size
	// 4 -- addition to checksum, we want sum to be divisible by 7
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
				if (FD_ISSET(c.fd, &read_fds)) {
					cout << "COMMUNICATION_INTERFACE | checkActivityOnSocket | got something" << endl;
					receiveDataFromClient(c);
				}
			}
		}

		//
		this_thread::sleep_for(chrono::milliseconds(10));
	}
}

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

bool CommunicationInterface::setupSocket()
{
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		cerr << "COMMUNICATION_INTERFACE | setupSocket | failed to setup socket" << endl;
		return false;
	}
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = SERVER_PORT;
	if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		cerr << "COMMUNICATION_INTERFACE | setupSocket | error with binding" << endl;
		return false;
	}
	listen(sockfd, 8);
	clientLength = sizeof(serv_addr); // same structure so no problem
	/* clientConnectThread = thread(&CommunicationInterface::checkAndConnectClient, this); */
	return true;
}
