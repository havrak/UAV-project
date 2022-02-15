/*
 * controllerInterface.h
 * Copyright (C) 2021 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef CONTROLLER_INTERFACE_H
#define CONTROLLER_INTERFACE_H

#include <mutex>
#include <thread>
#include <list>
#include <cstdlib>
#include "control_interpreter.h"

#define JOY_DEV "/dev/input/js0"

using namespace std;




// OPTIONAL: this should take into account controller type
ControlSurface getControlSurfaceFor(bool button, int id);


class ControllerInterface{
	protected:
		void notifyObserverEvent(ControlSurface, int val);
		void notifyObserverEvent(ControlSurface, int x, int y);
		thread loopThread;
		virtual void eventLoop(){}; // get's started by constructor

	private:
    list<ControlInterpreter *> observers;
		//ControlInterpreter ci;


	public:
		ControllerInterface();
		void addObserver(ControlInterpreter *v);
		void removeObserver(ControlInterpreter *v);
		virtual void generateEventForEveryButton(){};

};

#endif /* !CONTROLLERINTERFACE_H */
