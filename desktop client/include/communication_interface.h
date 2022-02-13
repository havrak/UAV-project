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
#include "drone_telemetry.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <queue>
#include <sys/socket.h>
#include <sys/types.h>

#include <condition_variable>
#include <cstring>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <time.h>
#include <vector>

#define SERVERPORT 8066
#define NUMBER_OF_THREADS 5


void sendConfigurationOfCamera();

using namespace std;

class ControllerDroneBridge : ControlInterpreter {
	private:
	static ControllerDroneBridge* controllerDroneBridge;
	static mutex mutexControllerDroneBridge;
	ControllerDroneBridge();

	thread sendControlComandThread;

	bool active = true;

	mutex controllerStateMutex;
	pConStr controllerState;

	void sendControlComand();

	public:
	int update(ControlSurface cs, int x, int y);
	int update(ControlSurface cs, int val);

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

	clock_t lastTimeDataReceived;

	string serverIp = "192.168.6.1";
	int sockfd;
	sockaddr_in serverAddress;
	serverStruct server;

	bool connectionEstablished = false;;
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
	void cleanUp();
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
