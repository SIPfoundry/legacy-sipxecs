//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef MP_AUDIO_FILE_UTILS_H
#define MP_AUDIO_FILE_UTILS_H

#include <os/iostream>

typedef short AudioSample;

// Utils functions to handle the audiofile

int32_t readIntMsb(istream &in, int bytes);
//: Read int from Most Significant Bytes
int32_t bytesToIntMsb(void *buff, int bytes);
//: Most Significant Bytes: change bytes to int
int32_t readIntLsb(istream &in, int bytes);
//: Least Siginificant Bytes: read int
int32_t bytesToIntLsb(void *buff, int bytes);
//: Least significant byte: byte to int
void skipBytes(istream &in, int bytes);
//: Skip one byte
void writeIntMsb(ostream &out, int32_t l, int bytes);
//: write int in Most Significant bytes
void writeIntLsb(ostream &out, int32_t l, int bytes);
//: Write int in Least Significant bytes


#endif
