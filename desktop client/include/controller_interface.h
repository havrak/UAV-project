/*
 * controllerInterface.h
 * Copyright (C) 2021 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef CONTROLLERINTERFACE_H
#define CONTROLLERINTERFACE_H

#include <mutex>
#include <thread>
#include <vector>
#include "control_interpreter.h"

using namespace std;

class ControllerInterface{
	private:
    vector<ControlInterpreter> observers;
		thread loopThread;
		ControlInterpreter ci;
		ControllerInterface();
		virtual void eventLoop(); // get's started by constuctor
		void notifyObserverEvent();


	public:
		void addObserver(ControlInterpreter v);
		void removeObserver(ControlInterpreter v);

};

#endif /* !CONTROLLERINTERFACE_H */
