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

/**
 * method that translate index in array given from controller
 * to enum of corresponding enum of ControlSurface
 *
 * @param bool button - boolean to indicate whether index corresponds to axis or button
 * @param int id - index in field
 * @return ControlSurface - ControlSurface that corresponds to id
 */
ControlSurface getControlSurfaceFor(bool button, int id);


class ControllerInterface{
	protected:

		/**
	   * notifies all observer with new button event
		 *
		 * @param ControlSurface cs - control surface that created the event
		 * @param int val - value of control surface
		 */
		void notifyObserverEvent(ControlSurface cs, int val);

		/**
	   * notifies all observer with new axis event
		 *
		 * @param ControlSurface cs - control surface that created the event
		 * @param int x - value of x axis
		 * @param int y - value of y axis
		 */
		void notifyObserverEvent(ControlSurface cs, int x, int y);
		thread loopThread;

		virtual void eventLoop(){}; // get's started by constructor

	private:
    list<ControlInterpreter *> observers;
		//ControlInterpreter ci;


	public:
		/**
		 * Constructor to create ControllerInterface intraface
		 * used by child that extends ControllerInterface
		 */
		ControllerInterface();

		/**
		 * adds new observer to send events to, after adding it it will send
		 * current state of controller to it
		 *
		 * @param ControlInterpreter *v pointer to control interpreter to add
		 */
		void addObserver(ControlInterpreter *v);

		/**
		 * removes observer to send events to
		 *
		 * @param ControlInterpreter *v pointer to control interpreter to be removed
		 */
		void removeObserver(ControlInterpreter *v);

		/**
		 * tries to kill threads of all observers
		 * tries to kill thread that creates events
		 */
		void terminateObservatoryAndObservers();

		/**
		 * generates event for every button
		 */
		virtual void generateEventForEveryButton(){};
		bool process = true;

};

#endif /* !CONTROLLERINTERFACE_H */
