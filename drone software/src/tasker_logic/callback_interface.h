// vim: set ft=arduino:
/*
 * callbackInterface.h
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef CALLBACK_INTERFACE_H
#define CALLBACK_INTERFACE_H
#include <cstdint>
#include <iomanip>

enum CallbackArgsType {
	NO_ARGS = 0x01,
	BYTE_ARRAY = 0x02,
	SINGLE_INPUT = 0x03,
	NON_ACTIVE = 0x00
};

class CallbackInterface {
	public:
	virtual ~CallbackInterface() { }
	virtual uint8_t call(uint16_t id) { return 0; }
	virtual uint8_t callArgs(uint16_t id, uint8_t* args, uint64_t argc) { return 0; } // arguments will be stored in structure
	virtual uint8_t callArg(uint16_t id, uint64_t arg) { return 0; }
};

#endif /* !CALLBACK_INTERFACE_H */
