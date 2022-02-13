/*
 * return_error_message.h
 * Copyright (C) 2021 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef ERROR_MESSAGE_H
#define ERROR_MESSAGE_H

#include <string>


using namespace std;

class ErrorMessage{
	public:
		const bool err;
		const unsigned int err_code;
		const string err_message;
		ErrorMessage(const bool err,const unsigned int err_code, const string err_message):err(err), err_code(err_code), err_message(err_message){
		};


};

#endif /* !ERROR_MESSAGE_H */
