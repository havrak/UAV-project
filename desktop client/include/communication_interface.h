/*
 * communication_interface.h
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef COMMUNICATION_INTERFACE_H
#define COMMUNICATION_INTERFACE_H

#include "control_interpreter.h"
#include "protocol_spec.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <queue>
#include <sys/socket.h>
#include <sys/types.h>

#include <time.h>
#include <condition_variable>
#include <cstring>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#define SERVERPORT 8066
#define NUMBER_OF_THREADS 5

#define MAX_SEND_MESSAGE_SIZE 255
#define MAX_MESSAGE_SIZE 510 // roughly 100 numbers with some metadata end terminators

using namespace std;

struct sendingStruct {

	unsigned char MessageType = 0;
	unsigned char MessagePriority = 0;
	unsigned char* messageBuffer;
};

struct serverStruct {
	int curIndexInBuffer = 0; // position where we have left off, first not filled index
	unsigned char curMessageType = 0;
	unsigned char curMessagePriority = 0;
	unsigned int short curMessageSize = 0;
	// NOTE: cannot store data here as we should be process multiple request from client at the same time
	unsigned char curMessageBuffer[MAX_MESSAGE_SIZE + 5]; // will be used to load message during reading, if whole message hasn't arrive reader will continu where it left
};

struct processingStuct { // info about message isn't stored two times, as info in clinet struct is only for processing
	unsigned char messageType;
	unsigned char messagePriority;
	unsigned int short messageSize;
	char messageBuffer[MAX_MESSAGE_SIZE];
};

class ControllerDroneBridge : ControlInterpreter {
	private:
	static ControllerDroneBridge* controllerDroneBridge;
	static mutex mutexControllerDroneBridge;

	bool active = true;


	public:
	void getActive();
	void setActive(bool active);
	static ControllerDroneBridge* GetInstance();
};

class CommunicationInterface {
	private:
	static CommunicationInterface* communicationInterface;
	static mutex mutexCommunicationInterface;
	static mutex serverMutex;

	fd_set read_fds;
	fd_set write_fds;
	fd_set except_fds;

	string serverIp = "192.168.6.1";
	int sockfd;
	sockaddr_in serverAddress;
	serverStruct server;

	thread establishConnectionToDroneThread;
	thread checkForNewDataThread;

	int buildFdSets();
	void checkActivityOnSocket();
	void clearServerStruct();

	bool fixReceiveData();
	bool receiveDataFromServer();

	public:
	static CommunicationInterface* GetInstance();
	bool setupSocket();
	bool establishConnectionToDrone();
	bool sendData(sendingStruct ss);
};

// we don't want thing to hang up on waiting to send something
class SendingThreadPool {
	private:
	SendingThreadPool();
	static SendingThreadPool* threadPool;

	vector<thread> threads;
	condition_variable_any workQueueUpdate;

	mutex workQueueMutex;
	queue<sendingStruct> workQueue;

	// to prevent stacking up control in queue we will stack them here and control the queue size more easily
	mutex controlQueueMutex;
	condition_variable_any controlQueueUpdate;
	thread controlThread;
	deque<sendingStruct> controlQueue;
	deque<clock_t> controlQueueTimestamps;

	bool process = true;

	void endThreadPool();
	void worker();
	void controlWorker();

	public:
	static SendingThreadPool* GetInstance();
	void scheduleToSend(sendingStruct ss);
	void scheduleToSendControl(sendingStruct ss); // Schedules to send control sequence, if data in queue is older than 10 ms it is dropped
};

class ProcessingThreadPool {
	private:
	ProcessingThreadPool();
	static ProcessingThreadPool* threadPool;

	vector<thread> threads;
	condition_variable_any workQueueUpdate;
	mutex workQueueMutex;
	queue<processingStuct> workQueue;
	bool process = true;

	void endThreadPool();
	void worker();

	public:
	static ProcessingThreadPool* GetInstance();

	void addJob(processingStuct j);
};

#endif /* !COMMUNICATION_INTERFACE_H */
