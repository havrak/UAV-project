/*
 * linux_controller_implementation.h
 * Copyright (C) 2021 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef LINUX_CONTROLLER_IMPLEMENTATION_H
#define LINUX_CONTROLLER_IMPLEMENTATION_H


#include "control_interpreter.h"
#include "controller_interface.h"
#include "protocol_spec.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>

using namespace std;

#define DEV_FILE "/dev/input/js0"
#define CONTROLLER_AXIS_VALUE 32767


/**
 * Class used to interface with controller from Linux machine
 * it extends ControllerInterface which handles observer logic
 */
class LinuxControllerImplementation : public ControllerInterface {
	protected:
		/**
		 * Loop that will try to initialize controller
		 * after it successeds it will handle processing new
		 * control sate and call notifies observers
		 */
		void eventLoop() override;

	private:
		const bool debug = true;
		bool shownError = false;
		bool settingUpFinished = false;
		int fd = -1;
		int num_of_axis = 0;
		int num_of_buttons =0;
		char name_of_joystick[80];
		vector<int> prevStateAxis;
		ControllerTypes controllerType;

		// A - 0
		// B - 1
		// X - 2
		// Y - 3
		// Left bumper - 4
		// Right bumber - 5
		// select - 6
		// start -7
		// XBOX - 8
		// Pressed left analog  - 9
		// Pressed right analog - 10
		vector<char> buttonsStates;
		// Left analog X - 0 (left is negative)
		// Left analog Y - 1 (up is negative)
		// Left trigger - 2 (starts negative)
		// Right analog X - 3 (left is negative)
		// Right analog Y - 4 (up is negative)
		// Right trigger - 5 (starts negative)
		// D-PAD X - 6 (left is negative: value of 32767)
		// D-PAD Y - 7 (up is negative: value of 32767)
		vector<int> axisStates;
	public:
		/* LinuxControllerImplementation(){}; */
		LinuxControllerImplementation(ControllerTypes ct);

		/**
		 * tries to setup controller, if it fails on the
		 * first try it will  generate errro message
		 */
		void setupController();

		/**
		 * Method  used to generate event for every button on the controller
		 * used to setup initial state for the observer
		 */
		void generateEventForEveryButton() override;
};

#endif /* !LINUX_CONTROLLER_IMPLEMENTATION_H */
