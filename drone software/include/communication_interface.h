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

using namespace std;

#define SERVER_PORT 8066
#define NUMBER_OF_THREADS 5
#define MAX_CLIENTS 10
#define MAX_SEND_MESSAGE_SIZE 255
#define MAX_MESSAGE_SIZE 510 // roughly 100 numbers with some metadata end terminators
// 500 bytes for message, 10 for metadata

struct client{
	int fd = -1 ;
	mutex *cMutex;
	sockaddr_in adress;
	bool readyToSend = true;
	int noTriesToFix = 0;


	int curIndexInBuffer=0; // position where we have left off, first not filled index
	unsigned char curMessageType = 0;
	unsigned char curMessagePriority = 0;
	unsigned int short curMessageSize = 0;
	// NOTE: cannot store data here as we should be process multiple request from client at the same time
	unsigned char curMessageBuffer[MAX_MESSAGE_SIZE+5]; // will be used to load message during reading, if whole message hasn't arrive reader will continu where it left
};

struct sendingStruct{
	client *cli;

	unsigned char MessageType = 0;
	unsigned char MessagePriority = 0;
	unsigned char *messageBuffer;

};


struct processingStruct{ // info about message isn't stored two times, as info in clinet struct is only for processing
	client *cli; // we need the client to know if he is ready to receive data

	unsigned char messageType;
	unsigned char messagePriority;
	unsigned int short messageSize;
	char messageBuffer[MAX_MESSAGE_SIZE];
};

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
		bool sendDataToClient(sendingStruct ss);
		bool sendDataToAll(sendingStruct ss);
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
		void scheduleToSendAll(sendingStruct ss);
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
