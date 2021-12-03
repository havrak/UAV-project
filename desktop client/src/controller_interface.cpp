/*
 * controller_interface.cpp
 * Copyright (C) 2021 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "controller_interface.h"
#include "control_interpreter.h"

ControllerInterface::ControllerInterface(){
	loopThread = thread(&ControllerInterface::eventLoop, this);
}

void ControllerInterface::addObserver(ControlInterpreter *v){
	observers.push_back(v);
}

void ControllerInterface::removeObserver(ControlInterpreter *v){
	observers.remove(v);

}

void ControllerInterface::notifyObserverEvent(){
    list<ControlInterpreter *>::iterator iterator = observers.begin();
    while (iterator != observers.end()) {
      (*iterator)->update();
      ++iterator;
    }
}

