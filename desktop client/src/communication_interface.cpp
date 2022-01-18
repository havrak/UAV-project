/*
 * communication_interface.cpp
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "communication_interface.h"


CommunicationInterface* CommunicationInterface::communicationInterface = nullptr;
mutex CommunicationInterface::mutexCommunicationInterface;
ControllerDroneBridge* ControllerDroneBridge::controllerDroneBridge = nullptr;
mutex ControllerDroneBridge::mutexControllerDroneBridge;

//CommunicationInterface::CommunicationInterface()

//{
//}

CommunicationInterface* CommunicationInterface::GetInstance()
{
	if (communicationInterface == nullptr) {
		mutexCommunicationInterface.lock();
		if (communicationInterface == nullptr)
			communicationInterface = new CommunicationInterface();
		mutexCommunicationInterface.unlock();
	}
	return communicationInterface;
}


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





