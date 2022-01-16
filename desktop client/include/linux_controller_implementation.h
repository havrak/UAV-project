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
#include "return_error_message.h"
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

class LinuxControllerImplementation : public ControllerInterface {
	protected:
		void eventLoop() override;

	private:
		const bool debug = true;
		int fd = -1;
		int num_of_axis = 0;
		int num_of_buttons =0;
		char name_of_joystick[80];
		vector<char> joy_button;
		vector<int> joy_axis;

	public:
		LinuxControllerImplementation();
		ReturnErrorMessage setupController();
};

#endif /* !LINUX_CONTROLLER_IMPLEMENTATION_H */
