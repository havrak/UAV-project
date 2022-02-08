/*
 * communication_interface.cpp
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "communication_interface.h"
#include "protocol_codes.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <cerrno>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>

CommunicationInterface* CommunicationInterface::communicationInterface = nullptr;
mutex CommunicationInterface::mutexCommunicationInterface;

SendingThreadPool* SendingThreadPool::threadPool = nullptr;
ProcessingThreadPool* ProcessingThreadPool::threadPool = nullptr;

/*-----------------------------------
// Declaring singleton logic
-----------------------------------*/

SendingThreadPool::SendingThreadPool()
{
	for (unsigned i = 0; i < NUMBER_OF_THREADS; i++)
		threads.push_back(std::thread(&SendingThreadPool::worker, this));
}

SendingThreadPool* SendingThreadPool::GetInstance()
{
	if (threadPool == nullptr) {
		threadPool = new SendingThreadPool();
	}
	return threadPool;
}


ProcessingThreadPool::ProcessingThreadPool()
{
	for (unsigned i = 0; i < NUMBER_OF_THREADS; i++)
		threads.push_back(std::thread(&ProcessingThreadPool::worker, this));
}

ProcessingThreadPool* ProcessingThreadPool::GetInstance()
{
	if (threadPool == nullptr) {
		threadPool = new ProcessingThreadPool();
	}
	return threadPool;
}

CommunicationInterface* CommunicationInterface::GetInstance()
{
	if (communicationInterface == nullptr) {
		mutexCommunicationInterface.lock();
		if (communicationInterface == nullptr) {
			communicationInterface = new CommunicationInterface();
			ProcessingThreadPool::GetInstance(); // let this one also create thread pool
			SendingThreadPool::GetInstance();

		}
		mutexCommunicationInterface.unlock();
	}
	return communicationInterface;
}

/*-----------------------------------
// CommunicationInterface section
----------------------------------**/

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
	memset(&cli.curMessageBuffer, 0, MAX_MESSAGE_SIZE+5);
}

// NOTE: will only move socket to last sequence of terminator and valid header data
bool CommunicationInterface::fixReceiveData(client cli)
{
	unsigned char buffer[5];
	bool receivingData = true;
	int index = 0;
	while (receivingData) {

		ssize_t rCount = recv(cli.fd, (char*)&buffer + (index), 1, MSG_DONTWAIT);
		if (rCount < 0 )
			return false;
		if (buffer[index] == terminator[0] && buffer[(index+1)%5] == terminator[1] && buffer[(index+2)%5] == terminator[2] && buffer[(index+3)%5] == terminator[3] && buffer[(index+4)%5] == terminator[4]) {
			return true;
		}
		index = (index + 1) % 5;

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
			fixReceiveData(cli);
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

			if (cli.curMessageBuffer[cli.curIndexInBuffer - 4] == terminator[0] && cli.curMessageBuffer[cli.curIndexInBuffer - 3] == terminator[1] && cli.curMessageBuffer[cli.curIndexInBuffer - 2] == terminator[2] && cli.curMessageBuffer[cli.curIndexInBuffer - 1] == terminator[3] && cli.curMessageBuffer[cli.curIndexInBuffer] == terminator[4]) {
				processingStruct j;
				j.cli = &cli;
				j.messageType = cli.curMessageType;
				j.messagePriority = cli.curMessagePriority;
				j.messageSize = cli.curMessageSize;
				memcpy(&j.messageBuffer, &cli.curMessageBuffer, j.messageSize);

				ProcessingThreadPool::GetInstance()->addJob(j);

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

bool CommunicationInterface::sendDataToClient(sendingStruct ss){
	if (sizeof(*ss.messageBuffer) + 10 > MAX_MESSAGE_SIZE) { // we don't care about meta for now
		cerr << "CONTROLLER_INTERFACE | sendData | data is over the size limit (0.5KB)" << endl;
		return false;
	}
	char message[sizeof(*ss.messageBuffer) + 10];

	// setup metadata
	message[0] = ss.MessageType;
	message[1] = ss.MessagePriority;
	message[2] = ((uint16_t) sizeof(*ss.messageBuffer)) >> 8;
	message[3] = ((uint16_t) sizeof(*ss.messageBuffer)) - (message[2] << 8);
	message[4] = 7 - ((message[0] + message[1] + message[2] + message[3]) % 7);

	// load message
	memcpy(message + 5, ss.messageBuffer, sizeof(*ss.messageBuffer));

	// setup terminator
	int li = sizeof(*ss.messageBuffer) + 4;
	message[li + 1] = terminator[0];
	message[li + 2] = terminator[1];
	message[li + 3] = terminator[2];
	message[li + 4] = terminator[3];
	message[li + 5] = terminator[4];

	ssize_t bytesSend = 0;
	bool sending = true;
	ss.cli->cMutex->lock();
	while (sending) {
		ssize_t sCount = send(sockfd, (char*)&message + bytesSend, (MAX_MESSAGE_SIZE < sizeof(*message) - bytesSend ? MAX_MESSAGE_SIZE : sizeof(*message) - bytesSend), 0);
		if ((sCount < 0 && errno != EAGAIN && errno != EWOULDBLOCK))
			return false;
		bytesSend += sCount;
		if (bytesSend == sizeof(*message)) {
			return true;
		}
	}
	ss.cli->cMutex->unlock();
	return false;
}

void CommunicationInterface::checkActivityOnSocket()
{
	int state;
	while (true) {
		buildFdSets();
		state = select(sockfd, &read_fds, &write_fds, &except_fds, NULL); // NOTE: theoretically poll() is better option
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
		mutex newMutex;
		c.cMutex = &newMutex;
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
	checkForNewDataThread = thread(&CommunicationInterface::checkActivityOnSocket, this);
	return true;
}

/*-----------------------------------
// ProcessingThreadPool section
----------------------------------**/

void ProcessingThreadPool::endThreadPool()
{
	process = false;
	workQueueUpdate.notify_all();
	for (thread& t : threads)
		t.join();
}

void ProcessingThreadPool::worker()
{
	while (process) {
		processingStruct ps;
		{
			unique_lock<mutex> mutex(workQueueMutex);
			workQueueUpdate.wait(mutex, [&] {
				return !workQueue.empty() || !process;
			});
			if (!process)
				break;
			ps = workQueue.front();
			workQueue.pop();
		}
		// here we process the request;
	}
}

void ProcessingThreadPool::addJob(processingStruct ps){
	lock_guard<mutex> mutex(workQueueMutex);
	workQueue.push(ps);
	workQueueUpdate.notify_all();
}

/*-----------------------------------
// SendingThreadPool section
----------------------------------**/


void SendingThreadPool::endThreadPool()
{
	process = false;
	workQueueUpdate.notify_all();
	for (thread& t : threads)
		t.join();
}

void SendingThreadPool::worker()
{
	sendingStruct ss;
	while (process) {
		{
			unique_lock<mutex> mutex(workQueueMutex);
			workQueueUpdate.wait(mutex, [&] {
				return !workQueue.empty() || !process;
			});
			if (!process)
				break;
			ss = workQueue.front();
			workQueue.pop();
		}
		CommunicationInterface::GetInstance()->sendDataToClient(ss);
	}
}

void SendingThreadPool::scheduleToSend(sendingStruct ss)
{
	lock_guard<mutex> mutex(workQueueMutex);
	workQueue.push(ss);
	workQueueUpdate.notify_all();
}

