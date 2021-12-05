/*
 * linux_controller_implementation.cpp
 * Copyright (C) 2021 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "linux_controller_implementation.h"
#include "return_error_message.h"

LinuxControllerImplementation::LinuxControllerImplementation() {
  setupController();
}

ReturnErrorMessage LinuxControllerImplementation::setupController() {
  if ((gamepadFD = open(JOY_DEV, O_RDONLY)) < 0) {
    if (debug)
      cout << "LINUX_CONTROLLER_IMPLEMENTATION | setupController | Failed to "
              "open device";
    return *new ReturnErrorMessage(true, 1, "Failed to open device");
  }

  ioctl(gamepadFD, JSIOCGAXES, &num_of_axis);
  ioctl(gamepadFD, JSIOCGBUTTONS, &num_of_buttons);
  ioctl(gamepadFD, JSIOCGNAME(80), &name_of_joystick);

  joy_button.resize(num_of_buttons, 0);
  joy_axis.resize(num_of_axis, 0);

  cout << "Joystick: " << name_of_joystick << endl
       << "  axis: " << num_of_axis << endl
       << "  buttons: " << num_of_buttons << endl;

  fcntl(gamepadFD, F_SETFL, O_NONBLOCK); // using non-blocking mode
  return *new ReturnErrorMessage(false, 0, "");
}

void LinuxControllerImplementation::eventLoop() {

  while (true) {
    js_event js;

    read(gamepadFD, &js, sizeof(js_event));

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
      cout << " " << setw(2) << joy_axis[i] / 10000;
    cout << endl;

    cout << "  button: ";
    for (size_t i(0); i < joy_button.size(); ++i)
      cout << " " << (int)joy_button[i];
    cout << endl;

    usleep(50);
  }
}
