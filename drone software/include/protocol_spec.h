/*
 * protocol_spec.h
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef PROTOCOL_SPEC_H
#define PROTOCOL_SPEC_H

const unsigned char terminator[5] = {0x00, 0x00, 0xFF, 0xFF, 0xFF};


enum protocol_codes{
	P_PING=0x01,
};



#endif /* !PROTOCOL_SPEC_H */
