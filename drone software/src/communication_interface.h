/*
 * communication_interface.h
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef COMMUNICATION_INTERFACE_H
#define COMMUNICATION_INTERFACE_H

#include "protocol_spec.h"
#include "peripherials_manager.h"
#include <arpa/inet.h>
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <cerrno>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <list>
#include <locale>
#include <mutex>
#include <netdb.h>
#include <netinet/in.h>
#include <queue>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <vector>
#include <utility>
#include <asm-generic/errno-base.h>
#include <bits/types/clock_t.h>


#include "camera_streamer.h"
#include "protocol_spec.h"
#include "telemetry.h"

using namespace std;

/* #define SERVER_PORT 8066 */
#define NUMBER_OF_THREADS 5
#define MAX_CLIENTS 10

class CommunicationInterface {
	private:
	struct sockaddr_in serv_addr;
	//, cli_addr;
	// int n;
	int sockfd, newsockfd;

	// sets do to
	fd_set read_fds;
	fd_set write_fds;
	fd_set except_fds;
	string myIP;
	int serverPort = 8066;

	socklen_t clientLength;
	list<client> clients;
	thread managementThread;
	bool process = true;
	const bool debug = false;

	/* thread checkForNewDataThread; */

	static CommunicationInterface* communicationInterface;
	static mutex mutexCommunicationInterface;

	/**
	 * Creates file descriptor set used by select()
	 *
	 * @return bool - true if FD_SET was setted up correctly
	 */
	bool buildFdSets();

	/**
	 * Reads data from client and sends it to be processes
	 *
	 * @param client cli - client to be read from
	 */
	bool receiveDataFromClient(client cli);

	/**
	 * tries to accept new client
	 * called by checkActivityOnSocket
	 */
	int newClientConnect();

	/**
	 * clears information in client struct
	 * about message that was last read
	 *
	 * @param client cli - client which structure should be cleaned
	 */
	void clearClientStruct(client cli);

	/**
	 * Move reading header to point to start of new message
	 * Works by searching for terminator string in data stream
	 *
	 * @param client cli - client for which reading has to be fixed
	 * @return bool - true if terminator was found
	 */
	bool fixReceiveData(client cli);


	public:

	/**
	 * main method used to access CommunicationInterface
	 * if instace wasn't created it will initialize
	 * CommunicationInterface, it will also initialize
	 * SendingThreadPool and ProcessingThreadPool
	 */
	static CommunicationInterface* GetInstance();

	/**
	 * setups server sockets, start thread to check
	 * new activity
	 */
	bool setupSocket(string myIP, int serverPort);

	/**
	 * waits for new data on the socket
	 * if event is generated it will
	 * either connect new client or
	 * read data from it
	 */
	void checkActivityOnSocket();

	/**
	 * clean up of server, closes all sockets
	 */
	void cleanUp();

	/**
	 * removes client, closes its socket
	 *
	 * @param client cli - client to be removed
	 */
	void removeClient(client cli);

	/**
	 * sends data to client
	 *
	 * @param SendingStructure ss - structure with data to be sent
	 * @return true if data was sent successfully
	 */
	bool sendDataToClient(SendingStructure ss);

	/**
	 * sends data to all clients
	 *
	 * @param SendingStructure ss - structure with data to be sent
	 * @return true if data was sent successfully to all clients
	 */
	bool sendDataToAll(SendingStructure ss);

	/**
	 * sends error message to specified client
	 *
	 * @param client cli - client to send error to
	 * @param int errCode - error code
	 * @param char* errMessage - char array with error message, will be cropped to 60 characters
	 */
	void sendErrorMessage(client cli, int errCode, char* errMessage);

	/**
	 * sends error message to all clients
	 *
	 * @param int errCode - error code
	 * @param char* errMessage - char array with error message, will be cropped to 60 characters
	 */
	void sendErrorMessageToAll(int errCode, char* errMessage);

	/**
	 * restarts whole CommunicationInterface and all its logic
	 */
	void restart();

	/**
	 * shutdowns whole server
	 */
	void shutdown();

	// TODO: move these methods into solo group, outside class COMMUNICATION_INTERFACE

	/**
	 * method used by thread to periodically update data in ServoControl control
	 * and send telemetry to all clients
	 */
	void manage();

	/**
	 * pings given client
	 *
	 * @pram client cli - client to be pinged
	 */
	void pingClient(client cli);

	/**
	 * NOTE: this method shouldn't be here and will be moved
	 *
	 * Method used to processes special control
	 *
	 * @param ProcessingStructure ps - processesing structure with special control stored
	 */
	void processSpecialControl(ProcessingStructure ps);

	thread checkForNewDataThread;
};

class SendingThreadPool {
	private:
	SendingThreadPool();
	static SendingThreadPool* threadPool;

	vector<thread> threads;
	condition_variable_any workQueueUpdate;
	mutex workQueueMutex;

	queue<SendingStructure> workQueue;
	bool process = true;
	volatile bool wswitch = false;

	/**
	 * worker method that runs int thread
	 * sleeps until it get notified by scheduling methods
	 * then it will send data to client
	 *
	 * if client fd is set to -1 it will send data to all clients
	 */
	void worker();

	public:
	/**
	 * main method used to access SendingThreadPool
	 * if instace wasn't created it will initialize
	 * SendingThreadPool
	 */
	static SendingThreadPool* GetInstance();

	/**
	 * Restarts thread pool
	 */
	void restart();

	/**
	 * ends thread pool, kills all worker threads
	 */
	void endThreadPool();

	/**
	 * schedules data to send, if data is added to the queue
	 * it will notify workers to send it
	 *
	 * @param SendingStructure ss - sending structure with data to be sent
	 */
	void scheduleToSend(SendingStructure ss);
};

class ProcessingThreadPool {
	private:
	ProcessingThreadPool();
	static ProcessingThreadPool* threadPool;

	vector<thread> threads;
	condition_variable_any workQueueUpdate;
	mutex workQueueMutex;
	queue<ProcessingStructure> workQueue;
	bool process = true;

	mutex controlQueueMutex;
	condition_variable_any controlQueueUpdate;
	thread controlThread;
	deque<ProcessingStructure> controlQueue;
	deque<clock_t> controlQueueTimestamps;

	/**
	 * worker method runned in thread, waits for new data and after
	 * it is notified it will process the data
	 */
	void worker();

	/**
	 * worker method runned in thread, waits for new control data
	 * and after it is notified it will process the data
	 */
	void controlWorker();

	public:
	/**
	 * main method used to access ProcessingThreadPool
	 * if instace wasn't created it will initialize
	 * ProcessingThreadPool
	 */
	static ProcessingThreadPool* GetInstance();

	/**
	 * Restarts thread pool
	 */
	void restart();

	/**
	 * ends thread pool, kills all worker threads
	 */
	void endThreadPool();

	/**
	 * adds new job to be processed
	 *
	 * @param ProcessingStructure ps - data to be processed
	 */
	void addJob(ProcessingStructure ps);

	/**
	 * adds new job with controll to be processed. if request at
	 * the top of the queue wasn't processed in last 100 ms it
	 * will be removed
	 *
	 * @param ProcessingStructure ps - data to be processed
	 */
	void addJobControl(ProcessingStructure ps);
};

#endif /* !COMMUNICATION_INTERFACE_H */
