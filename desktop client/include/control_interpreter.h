/*
 * control_interpreter.h
 * Copyright (C) 2021 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef CONTROL_INTERPRETER_H
#define CONTROL_INTERPRETER_H

#include <iostream>

using namespace std;


enum ControlSurface{
	L_ANALOG, R_ANALOG, L_TRIGGER, R_TRIGGER, L_BUMPER, R_BUMPER,
	X , Y, A, B,
	XBOX,START, SELECT,
	L_STICK_BUTTON, R_STICK_BUTTON,
	D_PAD,
	NON_DEFINED
};

class ControlInterpreter{
	public:
		virtual int update(ControlSurface, int x, int y){return 0;}
		virtual int update(ControlSurface, int val){return 0;}
};

#endif /* !CONTROL_INTERPRETER_H */
