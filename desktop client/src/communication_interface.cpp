/*
 * communication_interface.cpp
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "communication_interface.h"
#include "control_interpreter.h"
#include "gtkmm/enums.h"
#include "protocol_spec.h"
#include <cstring>
#include <ctime>
#include <mutex>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>

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

ControllerDroneBridge::ControllerDroneBridge()
{
	memset(&controllerState, 0, sizeof(controllerState));
	sendControlComandThread = std::thread(&ControllerDroneBridge::sendControlComand, this);
}

ControllerDroneBridge* ControllerDroneBridge::GetInstance()
{
	if (controllerDroneBridge == nullptr) {
		mutexControllerDroneBridge.lock();
		if (controllerDroneBridge == nullptr) {
			controllerDroneBridge = new ControllerDroneBridge();
		}
		mutexControllerDroneBridge.unlock();
	}

	return controllerDroneBridge;
}

SendingThreadPool::SendingThreadPool()
{
	for (unsigned i = 0; i < NUMBER_OF_THREADS; i++)
		threads.push_back(std::thread(&SendingThreadPool::worker, this));
	controlThread = (std::thread(&SendingThreadPool::controlWorker, this));
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

void CommunicationInterface::clearServerStruct()
{
	server.curIndexInBuffer = 0;
	server.curMessageType = 0;
	server.curMessagePriority = 0;
	server.curMessageSize = 0;
	memset(&server.curMessageBuffer, 0, MAX_MESSAGE_SIZE + 5);
}

void CommunicationInterface::cleanUp()
{
	cout << "COMMUNICATION_INTERFACE | cleanUp | killing communicationInterface\n";
	close(sockfd);
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
				ProccessingStructure ps;

				j.messageType = server.curMessageType;
				j.messagePriority = server.curMessagePriority;
				j.messageSize = server.curMessageSize;
				memcpy(&j.messageBuffer, &server.curMessageBuffer, j.messageSize);
				lastTimeDataReceived = clock();
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
	if (!connectionEstablished)
		return false;
	if (sizeof(ss.messageBuffer) + 10 > MAX_MESSAGE_SIZE) { // we don't care about meta for now
		cerr << "CONTROLLER_INTERFACE | sendData | data is over the size limit (0.5KB)" << endl;
		return false;
	}

	char message[sizeof(*ss.messageBuffer) + 10];

	// setup metadata
	message[0] = ss.MessageType;
	message[1] = ss.MessagePriority;
	message[2] = ((uint16_t)sizeof(*ss.messageBuffer)) >> 8;
	message[3] = ((uint16_t)sizeof(*ss.messageBuffer)) - (message[2] << 8);
	message[4] = 7 - ((message[0] + message[1] + message[2] + message[3]) % 7);

	// load message
	memcpy(message + 5, ss.messageBuffer, sizeof(*ss.messageBuffer)); // NOTE: clang gives waringing

	// setup terminator
	int li = sizeof(ss.messageBuffer) + 4;
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

		if (connect(sockfd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) > 0) {
			cout << "COMMUNICATION_INTERFACE | establishConnectionToDrone | connection established" << endl;
			connectionEstablished = true;
			sendConfigurationOfCamera();
			checkForNewDataThread = thread(&CommunicationInterface::checkActivityOnSocket, this);
			break;
		} else {
			cerr << "COMMUNICATION_INTERFACE | establishConnectionToDrone | failed to establish connection" << endl;
		}
		this_thread::sleep_for(chrono::milliseconds(100));
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
	cout << "COMMUNICATION_INTERFACE | setupSocket | socket was successfully setted up \n";
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
		switch (ps.messageType) {
		case P_PING: // ping
			break;
		case P_SET_RESTART: // resart of system
			break;
		case P_SET_SHUTDOW: // shutdown
			break;
		case P_SET_DISCONNECT: // disconnect client
			break;
		case P_SET_CAMERA: // camera settings
			break;
		case P_CON_SPC: // spacial control
			break;
		case P_TELE_IOSTAT: // io status
			DroneTelemetry::GetInstance()->processIO(ps);
			break;
		case P_TELE_GEN: // general information
			DroneTelemetry::GetInstance()->processIO(ps);
			break;
		case P_TELE_ATTGPS: // attitude sensors
			DroneTelemetry::GetInstance()->processIO(ps);
			break;
		case P_TELE_BATT: // battery status
			DroneTelemetry::GetInstance()->processIO(ps);
			break;
		case P_TELE_PWM: // pwm settings
			DroneTelemetry::GetInstance()->processIO(ps);
			break;
		case P_TELE_ERR: // general error message
			cerr << "ProcessingThreadPool | worker | server send an error \n";
			break;
		}
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

void SendingThreadPool::controlWorker()
{
	while (process) {
		sendingStruct ss;
		{
			unique_lock<mutex> mutex(controlQueueMutex);
			controlQueueUpdate.wait(mutex, [&] {
				return !controlQueue.empty() || !process;
			});
			if (!process)
				break;
		/* lock_guard<mutex> mutex(controlQueueMutex); */
			cout << "size: " << controlQueue.size() << endl;
			ss = controlQueue.back();
			controlQueue.pop_front();
		}
		/* { */
		/* } */

		CommunicationInterface::GetInstance()->sendData(ss);
	}
	cout << "Ending this thread \n";
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

void SendingThreadPool::scheduleToSendControl(sendingStruct ss)
{
	lock_guard<mutex> mutex(controlQueueMutex);
	// with each add we will check how old is the oldest element in queue if it is older than delete it
	if (!controlQueueTimestamps.empty() && (((float)clock()) - controlQueueTimestamps.back()) / CLOCKS_PER_SEC > 0.01) {
		controlQueueTimestamps.pop_back();
		controlQueue.pop_back();
	}
	cout << "size of sendingStruct " << sizeof(sendingStruct) << endl;
	cout << "Size before adding " << controlQueue.size() << endl;
	controlQueue.push_front(ss);
	controlQueueTimestamps.push_front(clock());
	controlQueueUpdate.notify_all();
}

void SendingThreadPool::scheduleToSend(sendingStruct ss)
{
	lock_guard<mutex> mutex(workQueueMutex);
	workQueue.push(ss);
	workQueueUpdate.notify_all();
}

/*-----------------------------------
// ControllerDroneBridge
-----------------------------------*/

int ControllerDroneBridge::update(ControlSurface cs, int x, int y)
{
	controllerStateMutex.lock();
	switch (cs) {
	case L_TRIGGER:
		controllerState.lTrigger = x;
		break;
	case L_ANALOG:
		controllerState.lAnalog.first = x;
		controllerState.lAnalog.second = y;
		break;
	case R_ANALOG:
		controllerState.rAnalog.first = x;
		controllerState.rAnalog.second = y;
		break;
	case R_TRIGGER:
		controllerState.lTrigger = x;
		break;
	case D_PAD:
		controllerState.dpad.first = x;
		controllerState.dpad.second = y;
		break;
	}
	controllerStateMutex.unlock();
	return 1;
}

int ControllerDroneBridge::update(ControlSurface cs, int val)
{
	if (cs == L_BUMPER) {
		controllerStateMutex.lock();
		controllerState.lBumber = val;
		controllerStateMutex.unlock();
	} else if (cs == R_BUMPER) {
		controllerStateMutex.lock();
		controllerState.rBumber = val;
		controllerStateMutex.unlock();
	} else if (active) {
		pConSpc pcs;
		sendingStruct ss;
		pcs.button = cs;
		pcs.state = val;
		ss.MessagePriority = 0x01;
		ss.MessageType = P_CON_SPC;
		memcpy(&pcs, &ss.messageBuffer, sizeof(pcs));
		SendingThreadPool::GetInstance()->scheduleToSend(ss);
	}
	return 1;
}

void ControllerDroneBridge::sendControlComand()
{
	sendingStruct ss;

	while (true) {
		ss.MessageType = P_CON_STR;
		ss.MessagePriority = 0x02;
		controllerStateMutex.lock();
		memcpy(&controllerState, &ss.messageBuffer, sizeof(controllerState));
		for(int i = 0; i < sizeof(controllerState); i++){
			cout << ((int) ss.messageBuffer[i]) << " ";
		}
		cout << endl;
		controllerStateMutex.unlock();
		SendingThreadPool::GetInstance()->scheduleToSendControl(ss);
		this_thread::sleep_for(chrono::milliseconds(50));
	}
}

/*-----------------------------------
// Other
-----------------------------------*/

void sendConfigurationOfCamera()
{
	cout << "COMMUNICATION_INTERFACE | sendConfigurationOfCamera | sending the configuration of the camera\n";
	pSetCamera cameraSetup;
	cameraSetup.ip[0] = 192;
	cameraSetup.ip[1] = 192;
	cameraSetup.ip[2] = 6;
	cameraSetup.ip[3] = 11;
	cameraSetup.port = 5000;
	sendingStruct<sizeof(cameraSetup)> ss;
	ss.MessageType = P_SET_CAMERA;
	ss.MessagePriority = 0x02;
	memcpy(&ss.messageBuffer, &cameraSetup, sizeof(cameraSetup));
	CommunicationInterface::GetInstance()->sendData(ss);
}
