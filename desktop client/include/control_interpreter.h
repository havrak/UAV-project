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


class ControlInterpreter{
	public:
		virtual int update(){return 0;}
};

#endif /* !CONTROL_INTERPRETER_H */
