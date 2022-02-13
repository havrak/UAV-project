/*
 * linux_controller_implementation.cpp
 * Copyright (C) 2021 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "linux_controller_implementation.h"
#include "controller_interface.h"

LinuxControllerImplementation::LinuxControllerImplementation()
{
	setupController();
}

ErrorMessage LinuxControllerImplementation::setupController()
{
	if ((fd = open(JOY_DEV, O_RDONLY)) < 0) {
		if (debug)
			cout << "LINUX_CONTROLLER_IMPLEMENTATION | setupController | Failed to "
							"open device";
		return *new ErrorMessage(true, 1, "Failed to open device");
	}

	ioctl(fd, JSIOCGAXES, &num_of_axis);
	ioctl(fd, JSIOCGBUTTONS, &num_of_buttons);
	ioctl(fd, JSIOCGNAME(80), &name_of_joystick);

	buttonsStates.resize(num_of_buttons, 0);
	axisStates.resize(num_of_axis, 0);

	cout << "LINUX_CONTROLLER_IMPLEMENTATION | setupController Joystick: " << name_of_joystick << endl
			 << "  axis: " << num_of_axis << endl
			 << "  buttons: " << num_of_buttons << endl;

	fcntl(fd, F_SETFL, O_NONBLOCK); // using non-blocking mode
	loopThread = thread(&LinuxControllerImplementation::eventLoop, this);
	return *new ErrorMessage(false, 0, "");
}
void LinuxControllerImplementation::eventLoop()
{
	js_event js;
	unsigned int long index;

	while (true) {
		read(fd, &js, sizeof(js_event));
		switch (js.type & ~JS_EVENT_INIT) {
		case JS_EVENT_AXIS:

			index = (int)js.number;
			if (index >= axisStates.size()) {
				cerr << "LINUX_CONTROLLER_IMPLEMENTATION | eventLoop | Axis index out of range" << index << endl;
				continue;
			}

			// NOTE: we don't want to call functions with every microscopic adjustment on analog stick, thus value is update only if change is more significant
			if (abs(axisStates[index] - js.value) > 2500) {
				axisStates[index] = js.value;
				// process here
				ControlSurface cs = getControlSurfaceFor(false, index);
				if (cs != L_TRIGGER && cs != R_TRIGGER) {
					if (index != 0 && getControlSurfaceFor(false, index - 1) == getControlSurfaceFor(false, index)) { // NOTE: as far as i know axis are always right after each other and x is firs
						notifyObserverEvent(cs, axisStates[index - 1], axisStates[index]);
						continue;
					} else {
						notifyObserverEvent(cs, axisStates[index], axisStates[index + 1]);
						continue;
					}
				} else {
					notifyObserverEvent(cs, axisStates[index], 0);
					continue;
				}
			}
			break;
		case JS_EVENT_BUTTON:
			index = (int)js.number;
			if (index >= buttonsStates.size()) {
				cerr << "LINUX_CONTROLLER_IMPLEMENTATION | eventLoop | Button index out of range" << index << endl;
				continue;
			}
			if (buttonsStates[index] != js.value) { // NOTE: not sure if this behaviour is desirable
				buttonsStates[index] = js.value;
				notifyObserverEvent(getControlSurfaceFor(true, index), buttonsStates[index]);
				break;
			}
		}

		usleep(50);
	}
}


void LinuxControllerImplementation::generateEventForEveryButton()
{
	for (int i = 0; i + 1 < sizeof(axisStates) / sizeof(axisStates[0]); i++) {
		ControlSurface cs = getControlSurfaceFor(false, i);
		if (cs != L_TRIGGER && cs != R_TRIGGER) {
			notifyObserverEvent(cs, axisStates[i], axisStates[i + 1]);
			i++;
			continue;
		} else {
			notifyObserverEvent(cs, axisStates[i], 0);
			continue;
		}
	}
	for (int i = 0; i < sizeof(buttonsStates) / sizeof(buttonsStates[0]); i++) {
		notifyObserverEvent(getControlSurfaceFor(true, i), buttonsStates[i]);
	}
}
