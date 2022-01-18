/*
 * communication_interface.h
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef COMMUNICATION_INTERFACE_H
#define COMMUNICATION_INTERFACE_H

#include "control_interpreter.h"
#include <iostream>
#include <mutex>
#include <thread>


using namespace std;

class ControllerDroneBridge : ControlInterpreter{
	private:
		static ControllerDroneBridge* controllerDroneBridge;
		static mutex mutexControllerDroneBridge;
		bool active = true;
	public:
		void getActive();
		void setActive(bool active);
		static ControllerDroneBridge* GetInstance();


};

class CommunicationInterface{
	private:
		static CommunicationInterface* communicationInterface;
		static mutex mutexCommunicationInterface;
	public:
		static CommunicationInterface* GetInstance();

};

#endif /* !COMMUNICATION_INTERFACE_H */
