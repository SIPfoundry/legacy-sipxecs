//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

// SYSTEM INCLUDES

#include <string.h>

// APPLICATION INCLUDES

#include "net/CallId.h"
#include <os/OsLock.h>
#include <os/OsDateTime.h>
#include <os/OsTimer.h>
#include <os/OsProcess.h>
#include <os/OsSocket.h>
#include <os/OsSysLog.h>
#include <net/NetMd5Codec.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// Number of chars of sChainValue to show in Call-Id's. (6 bits each for 60 bits)
#define CALL_ID_CHAIN_VALUE_REVEALED 10
// Number of chars of sChainValue to show in tags. (6 bits each for 36 bits)
#define TAG_CHAIN_VALUE_REVEALED 6

// STATIC VARIABLE INITIALIZATIONS

OsMutex CallId::sCallIdMutex(OsMutex::Q_FIFO);
size_t CallId::sCallNum = 0;
UtlString CallId::sChainValue;
bool CallId::sChainValueInitialized = false;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

// This implements a new strategy for generating Call-IDs after the
// problems described in http://track.sipfoundry.org/browse/XCL-51.
// This version generates crypto-random numbers.
//
// The Call-ID is composed of two concatenated fields:
// - a 60-bit (10 byte base64) pseudo-random number
// - a 16-bit (4 byte hex) sequence number.
//
// The sequence number is for human convenience, and although it
// significantly reduces the probability of collisions, it is not
// depended on for that purpose.  The use of a large pseudo-random
// field ensures that these Call-Id's hash uniformly even with poor
// hash functions.
//
// The pseudo-random number ("chain value") is generated iteratively
// by hashing the previous chain value with the current clock time.
//
// The chain value is initialized to ensure that it is unique for
// every process that uses CallId.  It is composed by hashing the host
// name, the process ID, and the start time.  The host name, process
// ID, and start time together ensure uniqueness.  The start time is
// used to microsecond resolution because on Windows, process IDs can
// be recycled quickly.
//
// For tags, 6 characters (36 bits) of the chain value are used, per
// RFC 3261 section 19.3.  For Call-Id's, 10 characters (60 bits) of
// the chain value are used, so that 2^30 Call-Id's can be current
// without there being a collision.  12 characters of the
// chain value are preserved for iteration that are never revealed in
// any output value.
//
// The generated Call-IDs are "words" according to the syntax of RFC
// 3261 by using the NetMd5Codec::SipTokenSafeAlphabet.
//
// No host portion is inserted into Call-Id values; for the rationale
// behind this, see draft-kaplan-sip-secure-call-id-*.

void CallId::getNewCallId(UtlString& callId)
{
   // Lock to protect common variables.
   OsLock lock(sCallIdMutex);

   // Increment the call number, rolling over after 0xFFFF, since we only
   // use the low-order 16 bits.
   sCallNum = (sCallNum + 1) & 0xFFFF;

   // Compute the next value.
   nextValue();

   // Compose the new Call-Id.
   callId.remove(0);
   callId.append(sChainValue, 0, CALL_ID_CHAIN_VALUE_REVEALED);
   callId.appendNumber(sCallNum, "%04x");
}

void CallId::getNewTag(UtlString& tag)
{
   // Lock to protect common variables.
   OsLock lock(sCallIdMutex);

   // Compute the next value.
   nextValue();

   // Copy the tag to the destination.
   tag.remove(0);
   tag.append(sChainValue, 0, TAG_CHAIN_VALUE_REVEALED);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

// Initialize the chain value.
void CallId::initialize()
{
   NetMd5Codec encoder;

   // Get the start time.
   OsTime currentTime;
   OsDateTime::getCurTime(currentTime);
   encoder.hash(&currentTime, sizeof(currentTime));

   // Get the process ID.
   PID processId;
   processId = OsProcess::getCurrentPID();
   encoder.hash(&processId, sizeof(processId));

   // Get the host identity.
   UtlString thisHost;
   OsSocket::getHostIp(&thisHost);
   encoder.hash(thisHost);

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "CallId::initialize sChainValue");

   // Save the initial hash used to seed the sequence
   encoder.appendBase64Sig(sChainValue);

   // Note initialization is done.
   sChainValueInitialized = true;
}

// Compute the next chain value.
void CallId::nextValue()
{
   NetMd5Codec encoder;

   // If we haven't initialized yet, do so.
   if (!sChainValueInitialized)
   {
      initialize();
   }

   // Use the previous chain value to seed the next one
   encoder.hash(sChainValue);

   // Get the time and hash it into the next value
   OsTime currentTime;
   OsDateTime::getCurTime(currentTime);
   encoder.hash(&currentTime, sizeof(currentTime));

   // Replace the old chain value with the new one
   sChainValue.remove(0);
   encoder.appendBase64Sig(sChainValue);
}
