/*
 * linux_controller_implementation.cpp
 * Copyright (C) 2021 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "linux_controller_implementation.h"

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

	joy_button.resize(num_of_buttons, 0);
	joy_axis.resize(num_of_axis, 0);

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

		switch (js.type & ~JS_EVENT_INIT) {
		case JS_EVENT_AXIS:
			if ((int)js.number >= joy_axis.size()) {
				cerr << "err:" << (int)js.number << endl;
				continue;
			}
			joy_axis[(int)js.number] = js.value;
			break;
		case JS_EVENT_BUTTON:
			if ((int)js.number >= joy_button.size()) {
				cerr << "err:" << (int)js.number << endl;
				continue;
			}
			joy_button[(int)js.number] = js.value;
			break;
		}

		cout << "axis/10000: ";
		for (size_t i(0); i < joy_axis.size(); ++i)
			cout << "- [" << (int)joy_axis[i] << ", " << i << "] ";
		cout << endl;

		cout << "  button ";
		for (size_t i(0); i < joy_button.size(); ++i)
			cout << "- [" << (int)joy_button[i] << ", " << i << "] ";
		cout << endl;

		usleep(50);
	}
}
