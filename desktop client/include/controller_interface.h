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
#include "control_interpreter.h"

#define JOY_DEV "/dev/input/js0"

using namespace std;



enum controlSurface{
	L_ANALOG, R_ANALOG, L_TRIGGER, R_TRIGGER, L_BUMPER, R_BUMBER,
	X, Y, A, B,
	START, SELECT,
	L_STICK_BUTTON, R_STICK_BUTTON,
	D_PAD
};




class ControllerInterface{
	private:
    list<ControlInterpreter *> observers;
		thread loopThread;
		ControlInterpreter ci;
		virtual void eventLoop(); // get's started by constuctor
		void notifyObserverEvent();


	public:
		ControllerInterface();
		void addObserver(ControlInterpreter *v);
		void removeObserver(ControlInterpreter *v);

};

#endif /* !CONTROLLERINTERFACE_H */
