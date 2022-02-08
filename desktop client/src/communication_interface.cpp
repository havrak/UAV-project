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
mutex CommunicationInterface::serverMutex;
ControllerDroneBridge* ControllerDroneBridge::controllerDroneBridge = nullptr;
mutex ControllerDroneBridge::mutexControllerDroneBridge;

SendingThreadPool* SendingThreadPool::threadPool = nullptr;
ProcessingThreadPool* ProcessingThreadPool::threadPool = nullptr;

/*-----------------------------------
// Declaring singleton logic
-----------------------------------*/
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
		if (communicationInterface == nullptr){
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

void CommunicationInterface::clearServerStruct()
{
	server.curIndexInBuffer = 0;
	server.curMessageType = 0;
	server.curMessagePriority = 0;
	server.curMessageSize = 0;
	memset(&server.curMessageBuffer, 0, MAX_MESSAGE_SIZE + 5);
}

bool CommunicationInterface::fixReceiveData()
{
	unsigned char buffer[5];
	bool receivingData = true;
	int index = 0;
	while (receivingData) {

		ssize_t rCount = recv(sockfd, (char*)&buffer + (index), 1, MSG_DONTWAIT);
		if (rCount < 0)
			return false;
		if (buffer[index] == terminator[0] && buffer[(index + 1) % 5] == terminator[1] && buffer[(index + 2) % 5] == terminator[2] && buffer[(index + 3) % 5] == terminator[3] && buffer[(index + 4) % 5] == terminator[4]) {
			return true;
		}
		index = (index + 1) % 5;
	}
	return false;
}

bool CommunicationInterface::receiveDataFromServer()
{
	ssize_t bytesReceived = 0;
	bool receivingData = true;
	if (server.curIndexInBuffer == 0) { // we are starting new message
		unsigned char infoBuffer[5];
		ssize_t tmp = recv(sockfd, (char*)&infoBuffer + bytesReceived, 5 - bytesReceived, MSG_DONTWAIT);
		if (tmp < 5)
			return false;

		if ((infoBuffer[0] + infoBuffer[1] + infoBuffer[2] + infoBuffer[3] + infoBuffer[4]) % 7 != 0) { // just check if first few bytes look semi valid
			cerr << "COMMUNICATION_INTERFACE | checkActivityOnSocket | checksum of received data doesn't match" << endl;
			fixReceiveData();
			return false;
		} else {
			server.curMessageType = infoBuffer[0];
			server.curMessagePriority = infoBuffer[1];
			server.curMessageSize = (infoBuffer[2] << 8) + infoBuffer[3];
		}
		bytesReceived = 0;
	} else {
		bytesReceived = server.curIndexInBuffer;
	}

	while (receivingData) {
		ssize_t rCount = recv(sockfd, (char*)&server.curMessageBuffer + bytesReceived, server.curMessageSize - server.curIndexInBuffer + 5, MSG_DONTWAIT);
		if (rCount < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
			cerr << "COMMUNICATION_INTERFACE | receiveDataFromClient | cannot read from server" << endl;
			clearServerStruct();
			return true;
		}
		server.curIndexInBuffer += rCount;

		if (server.curIndexInBuffer == server.curMessageSize + 4) {

			if (server.curMessageBuffer[server.curIndexInBuffer - 4] == terminator[0] && server.curMessageBuffer[server.curIndexInBuffer - 3] == terminator[1] && server.curMessageBuffer[server.curIndexInBuffer - 2] == terminator[2] && server.curMessageBuffer[server.curIndexInBuffer - 1] == terminator[3] && server.curMessageBuffer[server.curIndexInBuffer] == terminator[4]) {
				processingStuct j;

				j.messageType = server.curMessageType;
				j.messagePriority = server.curMessagePriority;
				j.messageSize = server.curMessageSize;
				memcpy(&j.messageBuffer, &server.curMessageBuffer, j.messageSize);

				ProcessingThreadPool::GetInstance()->addJob(j);

				clearServerStruct();
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
		state = select(sockfd, &read_fds, &write_fds, &except_fds, NULL); // NOTE: theoretically poll() is better option
		switch (state) {
		case -1:
			cerr << "COMMUNICATION_INTERFACE | checkActivityOnSocket | something went's wrong" << endl;
			break;
		case 0:
			cerr << "COMMUNICATION_INTERFACE | checkActivityOnSocket | something went's wrong" << endl;
		default:
			if (FD_ISSET(sockfd, &read_fds)) {
				receiveDataFromServer();
			}
		}
	}
	this_thread::sleep_for(chrono::milliseconds(10));
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

bool CommunicationInterface::sendData(sendingStruct ss)
{
	if (sizeof(ss.messageBuffer) + 10 > MAX_MESSAGE_SIZE) { // we don't care about meta for now
		cerr << "CONTROLLER_INTERFACE | sendData | data is over the size limit (0.5KB)" << endl;
		return false;
	}

	char message[sizeof(*ss.messageBuffer) + 10];

	// setup metadata
	message[0] = ss.MessageType;
	message[1] = ss.MessagePriority;
	message[2] = sizeof(*ss.messageBuffer) >> 8;
	message[3] = sizeof(*ss.messageBuffer) - (message[2] << 8);
	message[4] = 7 - ((message[0] + message[1] + message[2] + message[3]) % 7);

	// load message
	memcpy(message + 5, ss.messageBuffer, sizeof(*ss.messageBuffer)); // NOTE: clang gives waringing

	// setup terminator
	int li = sizeof(ss.messageBuffer) + 5;
	message[li + 1] = terminator[0];
	message[li + 2] = terminator[1];
	message[li + 3] = terminator[2];
	message[li + 4] = terminator[3];
	message[li + 5] = terminator[4];

	serverMutex.lock();
	ssize_t bytesSend = 0;
	bool sending = true;
	while (sending) {
		ssize_t sCount = send(sockfd, (char*)&message + bytesSend, (MAX_MESSAGE_SIZE < sizeof(*message) - bytesSend ? MAX_MESSAGE_SIZE : sizeof(*message) - bytesSend), 0);
		if ((sCount < 0 && errno != EAGAIN && errno != EWOULDBLOCK))
			serverMutex.unlock();
		return false;
		bytesSend += sCount;
		if (bytesSend == sizeof(*message)) {
			serverMutex.unlock();
			return true;
		}
	}
	serverMutex.unlock();
	return false;
}

bool CommunicationInterface::establishConnectionToDrone()
{
	serverAddress.sin_family = AF_INET;
	while (true) {
		// NOTE; user will be able to change parameters, thus sockaddr_in in needs to be recreated on each iteration
		serverMutex.lock();
		serverAddress.sin_port = SERVERPORT;
		inet_aton(serverIp.c_str(), (struct in_addr*)&serverAddress.sin_addr.s_addr);
		clearServerStruct();
		serverMutex.unlock();

		if (connect(sockfd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
			cout << "COMMUNICATION_INTERFACE | establishConnectionToDrone | connection established" << endl;
			checkForNewDataThread = thread(&CommunicationInterface::checkActivityOnSocket, this);

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
		processingStuct ps;
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

void ProcessingThreadPool::addJob(processingStuct ps)
{
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
	while (process) {
		sendingStruct ss;
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
		CommunicationInterface::GetInstance()->sendData(ss);
	}
}

void SendingThreadPool::scheduleToSend(sendingStruct ss)
{
	lock_guard<mutex> mutex(workQueueMutex);
	workQueue.push(ss);
	workQueueUpdate.notify_all();
}
