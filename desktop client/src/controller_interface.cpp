/*
 * controller_interface.cpp
 * Copyright (C) 2021 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "controller_interface.h"
#include "control_interpreter.h"

ControllerInterface::ControllerInterface()
{
	// loopThread = thread(&ControllerInterface::eventLoop, this);
}

void ControllerInterface::addObserver(ControlInterpreter* v)
{
	observers.push_back(v);
	generateEventForEveryButton();
}

void ControllerInterface::removeObserver(ControlInterpreter* v)
{
	observers.remove(v);
}

void ControllerInterface::notifyObserverEvent(ControlSurface cs, int val)
{
	list<ControlInterpreter*>::iterator iterator = observers.begin();
	while (iterator != observers.end()) {
		(*iterator)->update(cs, val);
		++iterator;
	}
}

void ControllerInterface::notifyObserverEvent(ControlSurface cs, int x, int y)
{
	list<ControlInterpreter*>::iterator iterator = observers.begin();
	while (iterator != observers.end()) {
		(*iterator)->update(cs, x, y);
		++iterator;
	}
}


ControlSurface getControlSurfaceFor(bool button, int id)
{
	if (button) {
		switch (id) {
		case 0:
			return A;
		case 1:
			return B;
		case 2:
			return X;
		case 3:
			return Y;
		case 4:
			return L_BUMPER;
		case 5:
			return R_BUMPER;
		case 6:
			return SELECT;
		case 7:
			return START;
		case 8:
			return XBOX;
		case 9:
			return L_STICK_BUTTON;
		case 10:
			return R_STICK_BUTTON;
		}
	} else {
		switch (id) {
		case 0:
			return L_ANALOG;
		case 1:
			return L_ANALOG;
		case 2:
			return L_TRIGGER;
		case 3:
			return R_ANALOG;
		case 4:
			return R_ANALOG;
		case 5:
			return R_TRIGGER;
		case 6:
			return D_PAD;
		case 7:
			return D_PAD;
		}
	}
	return NON_DEFINED;
}
