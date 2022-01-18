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

ReturnErrorMessage LinuxControllerImplementation::setupController()
{
	if ((fd = open(JOY_DEV, O_RDONLY)) < 0) {
		if (debug)
			cout << "LINUX_CONTROLLER_IMPLEMENTATION | setupController | Failed to "
							"open device";
		return *new ReturnErrorMessage(true, 1, "Failed to open device");
	}

	ioctl(fd, JSIOCGAXES, &num_of_axis);
	ioctl(fd, JSIOCGBUTTONS, &num_of_buttons);
	ioctl(fd, JSIOCGNAME(80), &name_of_joystick);

	curStateButton.resize(num_of_buttons, 0);
	curStateAxis.resize(num_of_axis, 0);

	cout << "Joystick: " << name_of_joystick << endl
			 << "  axis: " << num_of_axis << endl
			 << "  buttons: " << num_of_buttons << endl;

	fcntl(fd, F_SETFL, O_NONBLOCK); // using non-blocking mode
	loopThread = thread(&LinuxControllerImplementation::eventLoop, this);
	return *new ReturnErrorMessage(false, 0, "");
}

void LinuxControllerImplementation::eventLoop()
{
	js_event js;

	while (true) {
		/* struct js_event { */
		/* 	unsigned int time;      /1* event timestamp in milliseconds *1/ */
		/* 	short value;   /1* value *1/ */
		/* 	unsigned char type;     /1* event type *1/ */
		/* 	unsigned char number;   /1* axis/button number *1/ */
		/* }; */

		read(fd, &js, sizeof(js_event));
		int index;
		switch (js.type & ~JS_EVENT_INIT) {
			case JS_EVENT_AXIS:

				index = (int)js.number;
				if (index >= curStateAxis.size()) {
					cerr << "LINUX_CONTROLLER_IMPLEMENTATION | eventLoop | Axis index out of range" << index << endl;
					continue;
				}
				// NOTE: we don't want to call functions with every microscopic adjustment on analog stick
				// thus value is update only if change is more significant

				if (abs(curStateAxis[index] - js.value) > 1000) {
					curStateAxis[index] = js.value;
					// process here
					ControlSurface cs = getControlSurfaceFor(false, index);
					if (cs != L_TRIGGER && cs != R_TRIGGER) {
						if (index != 0 && getControlSurfaceFor(false, index - 1) == getControlSurfaceFor(false, index)) {
							// it is Y axis
						} else {
							// it is X axis
						}
						// get value for second axis
						// send data together
					} else {
						notifyObserverEvent(cs, curStateAxis[(int)js.number]);
						continue;
					}
				}
				break;
			case JS_EVENT_BUTTON:
				index = (int)js.number;
				if (index >= curStateButton.size()) {
					cerr << "LINUX_CONTROLLER_IMPLEMENTATION | eventLoop | Button index out of range" << index << endl;
					continue;
				}
				curStateButton[index] = js.value;
				break;
		}

		/* cout << "axis/10000: "; */
		/* for (size_t i(0); i < curStateAxis.size(); ++i) */
		/* 	cout << "- [" << (int)curStateAxis[i] << ", " << i << "] "; */
		/* cout << endl; */

		/* cout << "  button "; */
		/* for (size_t i(0); i < curStateButton.size(); ++i) */
		/* 	cout << "- [" << (int)curStateButton[i] << ", " << i << "] "; */
		/* cout << endl; */

		usleep(50);
	}
}
