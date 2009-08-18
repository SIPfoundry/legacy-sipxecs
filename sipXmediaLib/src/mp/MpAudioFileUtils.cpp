//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#include "mp/MpAudioFileUtils.h"

// Read int from Most Significant Bytes
int32_t readIntMsb(istream &in, int size) {
   if (size <= 0) return 0;
   int32_t l = readIntMsb(in,size-1) << 8;
   l |= static_cast<int32_t>(in.get()) & 255;
   return l;
}

// Most Significant Bytes: change bytes to int
int32_t bytesToIntMsb(void *vBuff, int size) {
   unsigned char *buff = reinterpret_cast<unsigned char *>(vBuff);
   if (size <= 0) return 0;
   int32_t l = bytesToIntMsb(buff,size-1) << 8;
   l |= static_cast<int32_t>(buff[size-1]) & 255;
   return l;
}

// Least Siginificant Bytes: read int
int32_t readIntLsb(istream &in, int size) {
   if (size <= 0) return 0;
   int32_t l = static_cast<int32_t>(in.get()) & 255;
   l |= readIntLsb(in,size-1)<<8;
   return l;
}

// Least significant byte: byte to int
int32_t bytesToIntLsb(void *vBuff, int size) {
   unsigned char *buff = reinterpret_cast<unsigned char *>(vBuff);
   if (size <= 0) return 0;
   int32_t l = static_cast<int32_t>(*buff) & 255;
   l |= bytesToIntLsb(buff+1,size-1)<<8;
   return l;
}

// Skip one byte
void skipBytes(istream &in, int size) {
   while (size-- > 0)
      in.get();
}

// write int in Most Significant bytes
void writeIntMsb(ostream &out, int32_t l, int size) {
   if (size <= 0) return;
   writeIntMsb(out, l>>8, size-1);
   out.put(l&255);
}

// Write int in Least Significant bytes
void writeIntLsb(ostream &out, int32_t l, int size) {
   if (size <= 0) return;
   out.put(l&255);
   writeIntLsb(out, l>>8, size-1);
}
