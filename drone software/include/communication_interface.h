/*
 * communication_interface.h
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef COMMUNICATION_INTERFACE_H
#define COMMUNICATION_INTERFACE_H

#include <iostream>
#include <locale>
#include <mutex>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <queue>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <condition_variable>
#include <list>
#include <thread>
#include <unistd.h>
#include <vector>
#include "protocol_spec.h"
#include "telemetry.h"
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

#include "protocol_spec.h"
#include "telemetry.h"
#include "camera_streamer.h"

using namespace std;

#define SERVER_PORT 8066
#define NUMBER_OF_THREADS 5
#define MAX_CLIENTS 10

class CommunicationInterface{
	private:

		struct sockaddr_in serv_addr;
		//, cli_addr;
		//int n;
		int sockfd, newsockfd;

		// sets do to
		fd_set read_fds;
		fd_set write_fds;
		fd_set except_fds;

		socklen_t clientLength;
		list<client> clients;

		thread clientConnectThread;
		thread checkForNewDataThread;

		static CommunicationInterface* communicationInterface;
		static mutex mutexCommunicationInterface;


		void checkAndConnectClient();
		int buildFdSets();
		bool receiveDataFromClient(client cli);
		int newClientConnect();
		void clearClientStruct(client cli);
		bool fixReceiveData(client cli);
	public:
		static CommunicationInterface* GetInstance();
		bool setupSocket();
		void checkActivityOnSocket();
		void checkForNewData(); // -> calls callback if new data is found, that processes it (needs to be really fast, will not start new thread just for processing)
		void cleanUp();
		void removeClient(client cli);
		bool sendDataToClient(sendingStruct ss);
		bool sendDataToAll(sendingStruct ss);

		//Destructive

		void restart();
		void shutdown();
};

class SendingThreadPool{
	private:
		SendingThreadPool();
		static SendingThreadPool* threadPool;

		vector<thread> threads;
 		condition_variable_any workQueueUpdate;
		mutex workQueueMutex;

		queue<sendingStruct> workQueue;
		bool process = true;

		void endThreadPool();
		void worker();

	public:
		static SendingThreadPool* GetInstance();

		void scheduleToSend(sendingStruct ss);

		void	scheduleToSendAll(sendingStruct ss);
};

class ProcessingThreadPool{
	private:
		ProcessingThreadPool();
		static ProcessingThreadPool* threadPool;

		vector<thread> threads;
 		condition_variable_any workQueueUpdate;
		mutex workQueueMutex;
		queue<processingStruct> workQueue;
		bool process = true;

		mutex controlQueueMutex;
		condition_variable_any controlQueueUpdate;
		thread controlThread;
		deque<processingStruct> controlQueue;
		deque<clock_t> controlQueueTimestamps;



		void endThreadPool();
		void worker();
		void controlWorker();
		void processControlCommands();

	public:
		static ProcessingThreadPool* GetInstance();

		void addJob(processingStruct j);
		void addJobControl(processingStruct j);
};





#endif /* !COMMUNICATION_INTERFACE_H */
