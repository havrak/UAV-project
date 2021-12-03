/*
 * return_error_message.h
 * Copyright (C) 2021 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef RETURN_ERROR_MESSAGE_H
#define RETURN_ERROR_MESSAGE_H

#include <string>


using namespace std;

class ReturnErrorMessage{
	public:
		const bool err;
		const int err_code;
		const string err_message;
		ReturnErrorMessage(const bool err,const int err_code, const string err_message):err(err), err_code(err_code), err_message(err_message){
		};


};

#endif /* !RETURN_ERROR_MESSAGE_H */
