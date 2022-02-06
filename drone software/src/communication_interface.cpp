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

ThreadPool* ThreadPool::GetInstance()
{
	if (threadPool == nullptr)
		threadPool = new ThreadPool();
	return threadPool;
}

CommunicationInterface* CommunicationInterface::GetInstance()
{
	if (communicationInterface == nullptr) {
		mutexCommunicationInterface.lock();
		if (communicationInterface == nullptr) {
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

// NOTE: called when data seems broken (packet was incomplete, size din't match or whatever)
bool CommunicationInterface::fixReceiveData(client cli)
{
	if (cli.noTriesToFix >= 5) {
		cli.noTriesToFix = false;
		return false;
	}

	char messageBuffer[MAX_SEND_MESSAGE_SIZE];
	bool receivingData = true;
	unsigned char infoBuffer[5];
	int index = 0;
	bool found = false;
	int i = 0;
	int j = 0;

	while (receivingData) {
		ssize_t rCount = recv(cli.fd, (char*)&messageBuffer, MAX_MESSAGE_SIZE, MSG_DONTWAIT);
		if (rCount < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
			cerr << "COMMUNICATION_INTERFACE | receiveDataFromClient | cannot read from client" << endl;
			return false; // just give up for now
		}
		if (found) {
			memcpy(&infoBuffer, &messageBuffer + index, 5 - index);
			goto fixEscape; // just want to jump there
		}
		found = false;
		while (i < MAX_MESSAGE_SIZE - 1 && j < 5 && !found) { // NOTE: handeling case when sequence is right at the end would be pretty annoying, thus we will handle this by sending more pings than necessary (which will cause indexes to sooner or later align)
			if (messageBuffer[i] == terminator[j]) {
				i++;
				j++;
				if (j == 4)
					found = true;
				if (i == MAX_MESSAGE_SIZE - 1) // we have mathching sequence but it is towards the end of the loop
					i = 0;											 // j stays the same, i resets
			} else {
				if (j > i) // sequence was towards the end of the message, i need to be reset in a different way
					i = i - (MAX_MESSAGE_SIZE - j);
				i = i - j + 1;
				j = 0;
			}
		}
		if (found) {
			if (i + 6 > MAX_MESSAGE_SIZE) { // indexing from 0
				memcpy(&infoBuffer, &messageBuffer[i + 1], MAX_MESSAGE_SIZE - i - 1);
				index = MAX_MESSAGE_SIZE - i - 1;
			} else {
				memcpy(&infoBuffer, &messageBuffer[i + 1], 5);
			fixEscape:
				if ((infoBuffer[0] + infoBuffer[1] + infoBuffer[2] + infoBuffer[3] + infoBuffer[4]) % 7 != 0) {
					cli.noTriesToFix++;
					fixReceiveData(cli); // just 5 times at max, so recursion will not be a problem
				} else {
					if (i + 6 < MAX_MESSAGE_SIZE) {
						memcpy(&cli.curMessageBuffer, &messageBuffer + (i + 6), MAX_MESSAGE_SIZE - i - 7); //
					}
					cli.curIndexInBuffer = MAX_MESSAGE_SIZE - i - 1;
					cli.curMessageType = infoBuffer[0];
					cli.curMessagePriority = infoBuffer[1];
					cli.curMessageSize = (infoBuffer[2] << 8) + infoBuffer[3];
					return true;
				}
			}
		}
	}
	return false;
}

bool CommunicationInterface::receiveDataFromClient(client cli)
{
	ssize_t bytesReceived = 0;
	bool receivingData = true;
	if (cli.curIndexInBuffer == 0) { // we are starting new message
		unsigned char infoBuffer[5];
		ssize_t tmp = recv(cli.fd, (char*)&infoBuffer, 5, MSG_DONTWAIT);
		if ((tmp < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) // as we call this method recursively we can get into position where there is nothing to read NOTE: check if this doesn't cause any problems
			return false;
		if (tmp < 5 && tmp > 0) {
		}

		if ((infoBuffer[0] + infoBuffer[1] + infoBuffer[2] + infoBuffer[3] + infoBuffer[4]) % 7 != 0) { // just check if first few bytes look semi valid
			cerr << "COMMUNICATION_INTERFACE | checkActivityOnSocket | checksum of received data doesn't match" << endl;
			bool fixed = fixReceiveData(cli);
			if (!fixed)
				return false;
		} else {
			cli.curMessageType = infoBuffer[0];
			cli.curMessagePriority = infoBuffer[1];
			cli.curMessageSize = (infoBuffer[2] << 8) + infoBuffer[3];
		}
		if (infoBuffer[0] == P_PING) {
			// NOTE: send ping from here
			return true;
		}
	} else {
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
			unsigned char infoBuffer[5];

			recv(cli.fd, (char*)&infoBuffer, 5, MSG_DONTWAIT);
			if (infoBuffer[0] == 0x00 && infoBuffer[1] == 0x00 && infoBuffer[2] == 0xFF && infoBuffer[3] == 0xFF && infoBuffer[4] == 0xFF) {
				// pushtToQueue();

				clearClientStruct(cli);
				return true;
			} else {
				cerr << "COMMUNICATION_INTERFACE | receiveDataFromClient | data doesn't end with correct terminator" << endl;
				cli.noTriesToFix++;
				if (cli.noTriesToFix > 4)
					return false;
				receiveDataFromClient(cli);
			}
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
		state = select(sockfd, &read_fds, &write_fds, &except_fds, NULL); // NOTE: theoreticaly poll() is better option
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
