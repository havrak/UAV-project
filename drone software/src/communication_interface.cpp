/*
 * communication_interface.cpp
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "communication_interface.h"
#include "protocol_spec.h"
#include "servo_control.h"
#include "telemetry.h"
#include <asm-generic/errno-base.h>
#include <cerrno>
#include <cstdint>
#include <mutex>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <utility>

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
	controlThread = thread(&ProcessingThreadPool::controlWorker, this);
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

void CommunicationInterface::shutdown()
{
	process = false;
	checkForNewDataThread.join();
	managementThread.join();
}

int CommunicationInterface::buildFdSets()
{

	FD_ZERO(&read_fds);
	FD_ZERO(&write_fds);
	FD_ZERO(&except_fds);

	FD_SET(STDIN_FILENO, &read_fds);
	FD_SET(sockfd, &read_fds);
	FD_SET(STDIN_FILENO, &except_fds);
	FD_SET(sockfd, &except_fds);

	for (client c : clients)
		if (c.fd != -1) {
			FD_SET(c.fd, &read_fds);
			FD_SET(c.fd, &except_fds);
			FD_SET(c.fd, &write_fds);
		}

	return 0;
}

void CommunicationInterface::clearClientStruct(client cli)
{
	cli.curIndexInBuffer = 0; // position where we have left off
	cli.curMessageType = 0;
	cli.curMessagePriority = 0;
	cli.curMessageSize = 0;
	memset(&cli.curMessageBuffer, 0, sizeof(cli.curMessageBuffer)); // just technicality, unecessary
}

void CommunicationInterface::removeClient(client cli) // just disconnect and set fd to zero, not sure if removing it from the list would be fine
{
	close(cli.fd);
	for(client c: clients){
		if(c.fd == cli.fd){
				clearClientStruct(c);
				cli.fd = -1;
		}
	}
}

// NOTE: will only move socket to last sequence of terminator and valid header data
bool CommunicationInterface::fixReceiveData(client cli)
{
	unsigned char buffer[5];
	bool receivingData = true;
	int index = 0;
	while (receivingData) {

		ssize_t rCount = recv(cli.fd, (char*)&buffer + (index), 1, MSG_DONTWAIT);
		if (rCount < 0)
			return false;
		if (buffer[index] == terminator[0] && buffer[(index + 1) % 5] == terminator[1] && buffer[(index + 2) % 5] == terminator[2] && buffer[(index + 3) % 5] == terminator[3] && buffer[(index + 4) % 5] == terminator[4]) {
			return true;
		}
		index = (index + 1) % 5;
	}
	return false;
}

bool CommunicationInterface::receiveDataFromClient(client cli)
{
	ssize_t bytesReceived = 0;

	if (cli.curIndexInBuffer == 0) { // we are starting new message
		unsigned char infoBuffer[5];
		ssize_t tmp = recv(cli.fd, (char*)&infoBuffer, 5, MSG_DONTWAIT);
		if (tmp < 5)
			return false;

		if ((infoBuffer[0] + infoBuffer[1] + infoBuffer[2] + infoBuffer[3] + infoBuffer[4]) % 7 != 0) { // just check if first few bytes look semi valid
			cerr << "COMMUNICATION_INTERFACE | receiveDataFromClient | checksum of received data doesn't match\n";
			fixReceiveData(cli);
			return false;
		} else {
			cout << "COMMUNICATION_INTERFACE | receiveDataFromClient | checksum from: " << cli.fd << " was valid\n";
			cli.curMessageType = infoBuffer[0];
			cli.curMessagePriority = infoBuffer[1];
			cli.curMessageSize = (((uint16_t)infoBuffer[2]) << 8) + ((uint16_t)infoBuffer[3]) + 5;

			cout << "COMMUNICATION_INTERFACE | receiveDataFromClient | message: ";
			cout << "\n   message type: " << int(cli.curMessageType) << "\n   message priority: " << int(cli.curMessagePriority) << "\n   message size: " << cli.curMessageSize << "\n";
		}
	} else {
		bytesReceived = cli.curIndexInBuffer;
	}

	while (true) {
		ssize_t rCount = recv(cli.fd, (char*)&cli.curMessageBuffer + bytesReceived, cli.curMessageSize - cli.curIndexInBuffer, MSG_DONTWAIT);

		if (rCount < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
			cerr << "COMMUNICATION_INTERFACE | receiveDataFromClient | cannot read from client\n";
			clearClientStruct(cli);
			return true;
		}
		cli.curIndexInBuffer += rCount;

		if (cli.curIndexInBuffer == cli.curMessageSize) {
			for (int i = 0; i < cli.curMessageSize; i++)
				cout << int(cli.curMessageBuffer[i]) << " ";
			cout << " \n";

			if (cli.curMessageBuffer[cli.curIndexInBuffer - 5] == terminator[0] && cli.curMessageBuffer[cli.curIndexInBuffer - 4] == terminator[1] && cli.curMessageBuffer[cli.curIndexInBuffer - 3] == terminator[2] && cli.curMessageBuffer[cli.curIndexInBuffer - 2] == terminator[3] && cli.curMessageBuffer[cli.curIndexInBuffer - 1] == terminator[4]) {

				cout << "COMMUNICATION_INTERFACE | receiveDataFromClient | Data was correctly terminated\n";

				ProcessingStructure ps(cli.fd, cli.cMutex, cli.curMessageType, cli.curMessagePriority, cli.curMessageSize);
				memcpy(ps.getMessageBuffer(), &cli.curMessageBuffer, cli.curMessageSize);

				if (ps.messageType == P_CON_STR) {
					// ProcessingThreadPool::GetInstance()->addJobControl(ps);
				} else {
					ProcessingThreadPool::GetInstance()->addJob(ps);
				}
				clearClientStruct(cli);
				return true;
			} else {
				cerr << "COMMUNICATION_INTERFACE | receiveDataFromClient | Data doesn't end with correct terminator\n";
				clearClientStruct(cli);
				return false;
			}
		}
	}
	return true;
}

bool CommunicationInterface::sendDataToClient(SendingStructure ss)
{
	if (ss.messageSize + 10 > MAX_MESSAGE_SIZE) { // we don't care about meta for now
		cerr << "COMMUNICATION_INTERFACE | sendDataToClient | data is over the size limit (0.5KB)\n";
		return false;
	}
	char message[ss.messageSize + 10];

	cout << "COMMUNICATION_INTERFACE | sendDataToClient | message: ";
	cout << "\n   message type: " << int(ss.messageType) << "\n   message priority: " << int(ss.messagePriority) << "\n   message size: " << ss.messageSize << "\n   data of message: ";
	for (int i = 0; i < ss.messageSize; i++) {
		cout << int(ss.messageBuffer[i]) << " ";
	}
	cout << "\n";

	// setup metadata
	message[0] = ss.messageType;
	message[1] = ss.messagePriority;
	message[2] = ((uint16_t)ss.messageSize) >> 8;
	message[3] = ((uint16_t)ss.messageSize) - ((message[2] << 8));
	message[4] = 7 - ((message[0] + message[1] + message[2] + message[3]) % 7);

	// load message
	memcpy(message + 5, ss.getMessageBuffer(), ss.messageSize);

	// setup terminator
	memcpy(message + 5 + ss.messageSize, &terminator, 5);

	ssize_t bytesSend = 0;
	bool sending = true;
	cout << "COMMUNICATION_INTERFACE | sendDataToClient | getting lock to client\n";
	cout << "COMMUNICATION_INTERFACE | sendDataToClient | clientfd: " << ss.cfd << "\n";
	cout << "COMMUNICATION_INTERFACE | sendDataToClient | client mutex: " << ss.cMutex << "\n";
	{
		//mutex m = ;
		lock_guard<mutex> m(*ss.cMutex); // NOTE: problematic
		/* ss.cli->cMutex->lock(); */
		cout << "COMMUNICATION_INTERFACE | sendDataToClient | client locked\n";
		while (sending) {
			ssize_t sCount = send(ss.cfd, (char*)&message + bytesSend, (MAX_MESSAGE_SIZE < sizeof(message) - bytesSend ? MAX_MESSAGE_SIZE : sizeof(message) - bytesSend), 0);
			if ((sCount < 0 && errno != EAGAIN && errno != EWOULDBLOCK)){
				cout << "COMMUNICATION_INTERFACE | sendDataToClient | unable to send data\n";
				return false;
			}
			bytesSend += sCount;
			if (bytesSend == sizeof(*message)) {
				cout << "COMMUNICATION_INTERFACE | sendDataToClient | data was send unlocking client\n";
				return true;
			}
		}
	}
	return false;
}

bool CommunicationInterface::sendDataToAll(SendingStructure ss)
{
	bool toReturn = true;
	for (client c : clients) {
		if (c.fd != 0) {
			cout << "sending to clinet\n";
			ss.cfd = c.fd;
			ss.cMutex= c.cMutex;
			if (!sendDataToClient(ss))
				toReturn = false;
		}
	}
	return toReturn;
}

void CommunicationInterface::checkActivityOnSocket()
{
	int state, fd;
	cout << "COMMUNICATION_INTERFACE | checkActivityOnSocket | thread started\n";
	while (process) {

		fd = sockfd;
		for (client c : clients)
			if (c.fd > fd)
				fd = c.fd;
		buildFdSets();

		cout << "COMMUNICATION_INTERFACE | checkActivityOnSocket | waiting for activity, fd:" << (fd + 1) << "\n";
		/* state = select(fd + 1, &read_fds, 0, 0, NULL); */
		state = select(fd + 1, &read_fds, 0, &except_fds, NULL); // OPTIONAL: use write_fds to control when to write to client?
		switch (state) {
		case -1:
			cerr << "COMMUNICATION_INTERFACE | checkActivityOnSocket | something went's wrong\n";
			break;
			exit(127);
		case 0:
			cerr << "COMMUNICATION_INTERFACE | checkActivityOnSocket | something went's wrong, select return 0\n";
			break;
		default:
			cout << "COMMUNICATION_INTERFACE | checkActivityOnSocket | we have an activity \n";
			if (FD_ISSET(sockfd, &read_fds)) // we will drop first packet by this method
				newClientConnect();

			for (client c : clients) {
				if (FD_ISSET(c.fd, &read_fds)) {
					cout << "COMMUNICATION_INTERFACE | checkActivityOnSocket | got data to be read from:" << c.fd << "\n";
					receiveDataFromClient(c);
				}
				if (FD_ISSET(c.fd, &except_fds)) {
					cout << "COMMUNICATION_INTERFACE | checkActivityOnSocket | client  " << c.fd << " failed with exception\n";
					removeClient(c);
				}
				/* if(FD_ISSET(c.fd, &write_fds)){ */
				/* 	cout << "COMMUNICATION_INTERFACE | checkActivityOnSocket | client " << c.fd << " is ready to be written to\n"; */
				/* } */
			}
		}
		this_thread::sleep_for(chrono::milliseconds(100));
	}
}

int CommunicationInterface::newClientConnect()
{
	sockaddr_in clientAddress;
	cout << "COMMUNICATION_INTERFACE | newClientConnect | trying to connect to new client\n";
	int clientfd = accept(sockfd, (struct sockaddr*)&clientAddress, &clientLength);

	if (clientfd == -1)
		return 0;

	char clientIPV4Address[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &clientAddress.sin_addr, clientIPV4Address, INET_ADDRSTRLEN);

	cout << "COMMUNICATION_INTERFACE | newClientConnect | ip: " << clientIPV4Address << ":" << clientAddress.sin_port << " fd:" << clientfd << "\n";
	struct client c;
	c.adress = clientAddress;
	c.fd = clientfd;
	//mutex cm;
	c.cMutex = new mutex;
	cout << "COMMUNICATION_INTERFACE | newClientConnect | client mutex: " << c.cMutex << "\n";
	clients.push_back(c);
	return clientfd;
}

void CommunicationInterface::manage()
{
	while (process) {
		// cout << "COMMUNICATION_INTERFACE | manage | sending telemetry\n";
		// Telemetry::GetInstance()->processGeneralTelemetryRequest(nullptr);
		this_thread::sleep_for(chrono::milliseconds(250));
	}
}

bool CommunicationInterface::setupSocket()
{
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		cerr << "COMMUNICATION_INTERFACE | setupSocket | failed to setup socket\n";
		return false;
	}
	int reuse = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) != 0) {
		cerr << "COMMUNICATION_INTERFACE | setupSocket | failed to setup socket options\n";
		return false;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(SERVER_PORT);
	if (bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		cerr << "COMMUNICATION_INTERFACE | setupSocket | error with binding\n";
		return false;
	}
	cout << "COMMUNICATION_INTERFACE | setupSocket | socket created successfully\n";
	listen(sockfd, 8);
	clientLength = sizeof(serv_addr);
	checkForNewDataThread = thread(&CommunicationInterface::checkActivityOnSocket, this);
	managementThread = thread(&CommunicationInterface::manage, this);

	return true;
}

void CommunicationInterface::pingClient(client cli)
{
	cout << "COMMUNICATION_INTERFACE | pingClient | ping client  \n";
	cout << "COMMUNICATION_INTERFACE | pingClient | client fd: " << cli.fd << "\n";
	SendingStructure ss(cli.fd, cli.cMutex, P_PING, 0x01, 3);
	cout << "COMMUNICATION_INTERFACE | pingClient | ping structure reated\n";
	cout << "COMMUNICATION_INTERFACE | pingClient | cli in ss: " << cli.fd << "\n";
	SendingThreadPool::GetInstance()->scheduleToSend(ss);
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
		ProcessingStructure* ps;
		{
			unique_lock<mutex> mutex(workQueueMutex);
			workQueueUpdate.wait(mutex, [&] {
				return !workQueue.empty() || !process;
			});
			if (!process)
				break;
			ps = &workQueue.front();
			workQueue.pop();
		}
		client tmp;
		tmp.cMutex = ps->cMutex;
		tmp.fd= ps->cfd;
		switch (ps->messageType) {
		case P_PING: // ping
			cout << "PING\n";
			{
			cout << "client fd: " << ps->cfd << "\n";
			CommunicationInterface::GetInstance()->pingClient(tmp);
			}
			break;
		case P_SET_RESTART: // resart of system
			/* CommunicationInterface::GetInstance()->restart(); */
			break;
		case P_SET_SHUTDOW: // shutdown
			/* CommunicationInterface::GetInstance()->shutdown(); */
			break;
		case P_SET_DISCONNECT: // disconnect client
			CommunicationInterface::GetInstance()->removeClient(tmp);
			break;
		case P_SET_CAMERA: // camera settings
		{
			CameraStreamer* cs = new CameraStreamer();
			cs->setUpCamera(*ps);
		} break;
		case P_CON_SPC: // spacial control
			break;
		case P_TELE_IOSTAT: // io status
			tmp.fd = ps->cfd;
			tmp.cMutex= ps->cMutex;
			Telemetry::GetInstance()->processIORequest(&tmp);
			break;
		case P_TELE_GEN:																										 // general information
			Telemetry::GetInstance()->processGeneralTelemetryRequest(&tmp); // sex tvoje máma
			break;
		case P_TELE_ATTGPS: // attitude sensors
			Telemetry::GetInstance()->processAttGPSRequest(&tmp);
			break;
		case P_TELE_BATT: // battery status
			Telemetry::GetInstance()->processBatteryRequest(&tmp);
			break;
		case P_TELE_PWM: // pwm settings
			Telemetry::GetInstance()->processPWMRequest(&tmp);
			break;
		case P_TELE_ERR: // general error message
			// NOTE: do we care if clients sends an error,
			cerr << "ProcessingThreadPool | worker | client send an error \n";
			break;
		}
		// here we process the request;
	}
}

void ProcessingThreadPool::controlWorker()
{
	while (process) {
		ProcessingStructure* ps;
		{
			unique_lock<mutex> mutex(controlQueueMutex);
			controlQueueUpdate.wait(mutex, [&] {
				return !controlQueue.empty() || !process;
			});
			cout << "PROCESSSING_THREAD_POOL | controlWorker | new control\n";
			ps = &controlQueue.back();
			controlQueueTimestamps.pop_back();
			controlQueue.pop_back();
		}
		cout << "PROCESSSING_THREAD_POOL | controlWorker | processing new control\n";
		ServoControl::GetInstance()->processControl(*ps);
	}
}

void ProcessingThreadPool::addJobControl(ProcessingStructure ps)
{
	lock_guard<mutex> mutex(controlQueueMutex);
	if (!controlQueueTimestamps.empty() && (((float)clock()) - controlQueueTimestamps.back()) / CLOCKS_PER_SEC > 0.01) {
		controlQueueTimestamps.pop_back();
		controlQueue.pop_back();
	}
	controlQueue.push_front(ps);
	controlQueueTimestamps.push_front(clock());
	controlQueueUpdate.notify_all();
	cout << "Control added\n";
}

void ProcessingThreadPool::addJob(ProcessingStructure ps)
{
	lock_guard<mutex> mutex(workQueueMutex);
	cout << "PROCESSSING_THREAD_POOL | addJob | adding job\n";
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
	SendingStructure* ss;
	while (process) {
		{
			unique_lock<mutex> mutex(workQueueMutex);
			workQueueUpdate.wait(mutex, [&] {
				return !workQueue.empty() || !process;
			});
			wswitch = false;
			ss = &workQueue.front();
			workQueue.pop();

		}
		//SendingStructure s = *ss;
		cout << "SENDING_THREAD_POOL | message type: " << ss->messageType << "\n";
		cout << "SENDING_THREAD_POOL | client fd: " << ss->cfd << "\n";
		if (ss->cfd == -1) {
			cout << "SENDING_THREAD_POOL | sending message to all, message type: " << (int)ss->messageType << "\n";
			CommunicationInterface::GetInstance()->sendDataToAll(*ss);
		} else
			CommunicationInterface::GetInstance()->sendDataToClient(*ss);
	}
}

void SendingThreadPool::scheduleToSend(SendingStructure ss)
{
	lock_guard<mutex> mutex(workQueueMutex);
	workQueue.push(ss);
	workQueueUpdate.notify_all();
	/* wswitch = true; */
}

/* void SendingThreadPool::scheduleToSendAll(SendingStructure ss) */
/* { */
/* 	lock_guard<mutex> mutex(workQueueMutex); */
/* 	ss.cli = nullptr; */
/* 	workQueue.push(ss); */
/* 	workQueueUpdate.notify_all(); */
/* } */
