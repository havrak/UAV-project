/*
 * control_interpreter.h
 * Copyright (C) 2021 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef CONTROL_INTERPRETER_H
#define CONTROL_INTERPRETER_H

#include <iostream>
#include "protocol_spec.h"

using namespace std;


enum ControlSurface{
	L_ANALOG = 0x11, R_ANALOG = 0x12, L_TRIGGER = 0x13, R_TRIGGER = 0x14, L_BUMPER = 0x15, R_BUMPER = 0x16,
	X =0x01, Y =0x02, A =0x03, B =0x04,
	XBOX =0x07,START =0x05, SELECT =0x06,
	L_STICK_BUTTON = 0x08, R_STICK_BUTTON = 0x09,
	D_PAD = 0x10,
	NON_DEFINED
};

class ControlInterpreter{
	public:
		virtual int update(ControlSurface cs, int x, int y){return 0;}
		virtual int update(ControlSurface cs, int val){return 0;}
		//virtual int controlState();
};

#endif /* !CONTROL_INTERPRETER_H */
