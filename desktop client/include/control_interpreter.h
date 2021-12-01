/*
 * control_interpreter.h
 * Copyright (C) 2021 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef CONTROL_INTERPRETER_H
#define CONTROL_INTERPRETER_H

class Observer{
	virtual int update();
};

class ControlInterpreter : Observer {

};

#endif /* !CONTROL_INTERPRETER_H */
