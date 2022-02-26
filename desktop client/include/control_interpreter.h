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


class ControlInterpreter{
	public:
		virtual int update(ControlSurface cs, int x, int y){return 0;}
		virtual int update(ControlSurface cs, int val){return 0;}
		//virtual int controlState();
};

#endif /* !CONTROL_INTERPRETER_H */
