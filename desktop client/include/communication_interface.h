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
#include <fcntl.h>

#define SERVER_PORT 8066
#define NUMBER_OF_THREADS 5


using namespace std;

/**
 * One of observers called by Controller Interface
 * Processes callback from controller and generates
 * packet to send to server
 *
 * can be accessed by GetInstance()
 */
class ControllerDroneBridge : ControlInterpreter {
	private:
	/* static ControllerDroneBridge* controllerDroneBridge; */
	/* static mutex mutexControllerDroneBridge; */

	thread sendControlComandThread;

	bool active = true;

	mutex controllerStateMutex;
	pConStr controllerState;

	/**
	 * Thread to send packt with current
	 * controller state to the Raspberry
	 */
	void sendControlComand();

	public:
	ControllerDroneBridge();

	/**
	 * update method called by Controller Interface
	 *
	 * @param ControlSurface cs - type of control surface for which callback is generated
	 * @param int x - value of X axis
	 * @param int y - value of Y axis
	 */
	int update(ControlSurface cs, int x, int y) override;

	/**
	 * update method called by Controller Interface
	 *
	 * @param ControlSurface cs - type of control surface for which callback is generated
	 * @param int val - value of button
	 */
	int update(ControlSurface cs, int val) override;

	/**
	 * getter on active variable
	 *
	 * @return bool - true if active == true
	 */
	bool getActive();

	/**
	 * sets value of active
	 *
	 * @param bool active - new value of active value
	 */
	void setActive(bool active);

	/**
   * Access method to the singleton
	 * if ControllerDroneBridge is not initialized
	 * this method will create it and
	 * start sendControlComandThread
	 */
	/* static ControllerDroneBridge* GetInstance(); */
};

/**
 * Interface used to communicate with Raspberry
 *
 * can be access by GetInstance()
 */
class CommunicationInterface {
	private:
	static CommunicationInterface* communicationInterface;
	static mutex mutexCommunicationInterface;
	static mutex serverMutex;

	fd_set read_fds;
	fd_set write_fds;
	fd_set except_fds;

	clock_t lastTimeDataReceived;

	string serverIP = "192.168.6.1";
	string myIP;
	int serverPort = 8066;
	int cameraPort;

	int sockfd;
	sockaddr_in serverAddress;
	serverStruct server;

	const bool debug = false;
	bool process = true;
	bool connectionEstablished = false;;
	thread establishConnectionToDroneThread;
	thread checkForNewDataThread;

	/**
	 * Creates file descriptor set used by select()
	 *
	 * @return bool - true if FD_SET was setted up correctly
	 */
	bool buildFdSets();

	/**
	 * Method that waits for activity on socket
	 * if activity is detect it will load messages
	 * or print errors
	 */
	void checkActivityOnSocket();

	/**
	 * clears information in server struct
	 * about message that was last read
	 */
	void clearServerStruct();

	/**
	 * Move reading header to point to start of new message
	 * Works by searching for terminator string in data stream
	 *
	 * @return bool - true if terminator was found
	 */
	bool fixReceiveData();

	/**
	 * reads data from server and send request to ProcessingThreadPool
	 * to process new data
	 *
	 * @return bool - true if data was successfully read
	 */
	bool receiveDataFromServer();

	/**
	 * method used to check if file descriptor
	 * is valid
	 *
	 * @return bool - true if fd is valid
	 */
	bool isFdValid(int fd);


	public:

	/**
	 * main method to acces CommunicationInterface
	 * if instance hadn't been created GetInstance()
	 * will create it, it will also initialize
	 * SendingThreadPool and ProcessingThreadPool
	 */
	static CommunicationInterface* GetInstance();

	/**
	 * setups server socket
	 *
	 * @return bool - true id socket was setup
	 */
	bool setupSocket(string serverIP, string myIP, int serverPort);

	/**
	 * sets camera port
	 *
	 * @param int cameraPort - port number
	 */
	void setCameraPort(int cameraPort);

	/**
	 * will try to establish connection until
	 * it successeds. If connection is establish
	 * method will starts thread to listen
	 * for new data and send camera settings
	 * to the server
	 */
	bool establishConnectionToDrone();

	/**
	 * sends data to server
	 *
	 * @param SendingStructure ss - SendingStructure with data to be send
	 */
	bool sendData(SendingStructure ss);

	/**
	 * method called to cleanUp CommunicationInterface interface
	 * closes socket
	 */
	void cleanUp();

	/**
	 * sends ping to sever
	 */
	void pingServer();

	/**
	 * sends request to server to create camera stream
	 *
	 */
	void requestCameraStream();
};

/**
 * Thread poll used to schedule sending of data to the server
 * can be access by GetInstance
 */
class SendingThreadPool {
	private:

	SendingThreadPool();
	static SendingThreadPool* threadPool;

	vector<thread> threads;
	condition_variable_any workQueueUpdate;

	mutex workQueueMutex;
	queue<SendingStructure> workQueue;

	// to prevent stacking up control in queue we will stack them here and control the queue size more easily
	mutex controlQueueMutex;
	condition_variable_any controlQueueUpdate;
	thread controlThread;
	deque<SendingStructure> controlQueue;
	deque<clock_t> controlQueueTimestamps;

	bool process = true;

	/**
	 * worker method that runs int thread
	 * sleeps until it get notified by scheduling methods
	 * then it will send data to server
	 */
	void worker();

	/**
	 * method that send control commands
	 * to the server
	 */
	void controlWorker();

	public:

	/**
	 * ends thread pool, kills all worker threads
	 */
	void endThreadPool();

	/**
	 * main method used to access SendingThreadPool
	 * if instace wasn't created it will initialize
	 * SendingThreadPool
	 */
	static SendingThreadPool* GetInstance();

	/**
	 * schedules data to send, if data is added to the queue
	 * it will notify workers to send it
	 *
	 * @param SendingStructure ss - sending structure with data to be sent
	 */
	void scheduleToSend(SendingStructure ss);

	/**
	 * schedule to send control data, if at the top of the queue
	 * is packet older than 100 ms it will remove this element
	 *
	 * @return SendingStructure ss - sending structure with data to be sent
	 */
	void scheduleToSendControl(SendingStructure ss); // Schedules to send control sequence, if data in queue is older than 10 ms it is dropped


};

/**
 * Thread pool used to process new data
 * can be access by GetInstance
 */
class ProcessingThreadPool {
	private:
	ProcessingThreadPool();
	static ProcessingThreadPool* threadPool;

	vector<thread> threads;
	condition_variable_any workQueueUpdate;
	mutex workQueueMutex;
	queue<ProcessingStructure> workQueue;
	bool process = true;

	/**
	 * worker method runned in thread, waits for new data and after
	 * it is notified it will process the data
	 */
	void worker();

	public:
	/**
	 * main method used to access ProcessingThreadPool
	 * if instace wasn't created it will create it
	 */
	static ProcessingThreadPool* GetInstance();

	/**
	 * ends thread pool, kills all threads
	 */
	void endThreadPool();

	/**
	 * adds new job to be processed
	 *
	 * @param ProcessingStructure ps - data to be processed
	 */
	void addJob(ProcessingStructure ps);
};
#endif /* !COMMUNICATION_INTERFACE_H */
