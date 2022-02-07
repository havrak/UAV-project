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
#include <vector>

CommunicationInterface* CommunicationInterface::communicationInterface = nullptr;
mutex CommunicationInterface::mutexCommunicationInterface;

ThreadPool* ThreadPool::threadPool = nullptr;

ThreadPool::ThreadPool()
{
	for (unsigned i = 0; i < NUMBER_OF_THREADS; i++)
		threads.push_back(std::thread(&ThreadPool::worker, this));
}

ThreadPool* ThreadPool::GetInstance()
{
	if (threadPool == nullptr) {
		threadPool = new ThreadPool();
	}
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

int findSequencesInBuffer(char* message, vector<int>* indexes)
{
	return findSequencesInBuffer(message, indexes, 0);
}

// start with remanding number of chars to match
int findSequencesInBuffer(char* message, vector<int>* indexes, int charsMatched)
{
	int i = 0;
	int j = -charsMatched;
	while (i < FIX_SIZE - 1 && j < 5) {
		if (message[i] == terminator[j]) {
			i++;
			j++;
			if (j == 4) {
				indexes->push_back(i);
				j = 0;
			} else if (i == FIX_SIZE - 1) {
				i = 0; // j stays the same, i resets
				indexes->push_back(-j);
			}
		} else {
			i = i - j + 1;
			j = 0;
		}
	}
	return indexes->size();
}

// NOTE: called when data seems broken (packet was incomplete, size didn't match or whatever)
bool CommunicationInterface::fixReceiveData(client cli)
{
	char messageBuffer[FIX_SIZE];
	bool receivingData = true;
	unsigned char infoBuffer[5];
	vector<int> indexes;
	int charsOfTerminatorMatched = 0;
	int charsOfInfoSequenceLoaded = -1;

	while (receivingData) {

		ssize_t rCount = recv(cli.fd, (char*)&messageBuffer, FIX_SIZE, MSG_DONTWAIT);

		if (rCount < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
			cerr << "COMMUNICATION_INTERFACE | receiveDataFromClient | cannot read from client" << endl;
			return false; // just give up for now
		}

		if (charsOfInfoSequenceLoaded > 0) {
			memcpy(&infoBuffer, &messageBuffer, 5 - charsOfInfoSequenceLoaded);

		} else {
			findSequencesInBuffer(messageBuffer, &indexes, charsOfTerminatorMatched);
		}

		for (int i : indexes) {
			if (i < 0) {
				cout << "COMMUNICATION_INTERFACE | fixReceiveData | we need to check if last: " << i << "math" << endl;
				charsOfTerminatorMatched = i;
				break;
			}

			if (i + 6 > FIX_SIZE) { // NOTE; we cannot load whole info data
				memcpy(&infoBuffer, &messageBuffer[i + 1], FIX_SIZE - (i + 6));
				charsOfInfoSequenceLoaded = FIX_SIZE - (i + 1);
				break;
			}

			memcpy(&infoBuffer, &messageBuffer[i + 1], 5);

			if ((infoBuffer[0] + infoBuffer[1] + infoBuffer[2] + infoBuffer[3] + infoBuffer[4]) % 7 != 0) {
				cli.curMessageType = infoBuffer[0];
				cli.curMessagePriority = infoBuffer[1];
				cli.curMessageSize = (infoBuffer[2] << 8) + infoBuffer[3];
				cli.curIndexInBuffer = FIX_SIZE - i - 1;
				cli.noTriesToFix = 0;
				if (cli.curMessageSize > FIX_SIZE - i - 6) {
					memcpy(&cli.curMessageBuffer, &messageBuffer[i + 6], FIX_SIZE - i - 6);
					cli.curIndexInBuffer = FIX_SIZE - i - 6;

				} else {
					memcpy(&cli.curMessageBuffer, &messageBuffer[i + 6], cli.curMessageSize);
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
		ssize_t tmp = recv(cli.fd, (char*)&infoBuffer + bytesReceived, 5 - bytesReceived, MSG_DONTWAIT);
		if (tmp < 5)
			return false;

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
		bytesReceived = 0;
	} else {
		bytesReceived = cli.curIndexInBuffer;
	}

	while (receivingData) {
		ssize_t rCount = recv(cli.fd, (char*)&cli.curMessageBuffer + bytesReceived, cli.curMessageSize - cli.curIndexInBuffer + 5, MSG_DONTWAIT);
		if (rCount < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
			cerr << "COMMUNICATION_INTERFACE | receiveDataFromClient | cannot read from client" << endl;
			clearClientStruct(cli);
			return true;
		}
		cli.curIndexInBuffer += rCount;

		if (cli.curIndexInBuffer == cli.curMessageSize + 4) {

			if (cli.curMessageBuffer[cli.curIndexInBuffer - 4] == 0x00 && cli.curMessageBuffer[cli.curIndexInBuffer - 3] == 0x00 && cli.curMessageBuffer[cli.curIndexInBuffer - 2] == 0xFF && cli.curMessageBuffer[cli.curIndexInBuffer - 1] == 0xFF && cli.curMessageBuffer[cli.curIndexInBuffer] == 0xFF) {
				// pushtToQueue();

				clearClientStruct(cli);
				return true;
			} else {
				cerr << "COMMUNICATION_INTERFACE | receiveDataFromClient | Data doesn't end with correct terminator" << endl;
				return false;
			}
		}
	}
	return true;
}

void CommunicationInterface::checkActivityOnSocket()
{
	int state;
	while (true) {
		buildFdSets();
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

void ThreadPool::endThreadPool()
{
	process = false;
	workQueueConditionVariable.notify_all();
	for (thread& t : threads)
		t.join();
}

void ThreadPool::worker()
{
	while (process) {
		job j;
		{
			unique_lock<mutex> qMutex(workQueueMutex);
			workQueueConditionVariable.wait(qMutex, [&] {
				return !workQueue.empty() || !process;
			});
			if (!process)
				break;
			j = workQueue.front();
			workQueue.pop();
		}
		// here we process the request;
	}
}
