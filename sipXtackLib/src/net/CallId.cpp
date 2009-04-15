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

// Number of chars of sChainValue to keep.  (4 bits each)
#define CHAIN_VALUE_LENGTH 24
// Number of chars of sChainValue to show in Call-Id's.  (4 bits each)
#define CALL_ID_CHAIN_VALUE_REVEALED 16
// Number of chars of sChainValue to show in tags.  (4 bits each)
#define TAG_CHAIN_VALUE_REVEALED 8
// Maximum number of chars from a seed string to use.
// (8 chars gives 32 bits if seed string is hex.)
#define MAX_SEED_CHARS 8

// STATIC VARIABLE INITIALIZATIONS

OsMutex CallId::sCallIdMutex(OsMutex::Q_FIFO);
unsigned int CallId::sCallNum = 0;
UtlString CallId::sChainValue;
UtlBoolean CallId::sChainValueInitialized = FALSE;
UtlString CallId::sKey;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

void CallId::getNewCallId(UtlString& callId)
{
    getNewCallId("s", callId);
}

// This implements a new strategy for generating Call-IDs after the
// problems described in http://track.sipfoundry.org/browse/XCL-51.
// This version generates crypto-random numbers.
//
// The Call-ID is composed of three fields:
// - a prefix supplied by the caller
// - a 64-bit (hex) pseudo-random number
// - a 32-bit (decimal) sequence number.
//
// The sequence number is for human convenience, and although it
// significantly reduces the probability of collisions, it is not
// depended on for that purpose.  The use of a large pseudo-random
// field ensures that these Call-Id's hash uniformly even with poor
// hash functions.
//
// The pseudo-random number ("chain value") is generated iteratively
// by hashing the previous chain value with a secret 32-bit key, the
// current clock time, and (when generating tags) a seed value
// supplied by the caller.  This structure is from RFC 1750.
//
// The seed value, when used, should be an externally-generated
// crypto-quality random number, to provide extra entropy for the
// process.  Fortunately, incoming from-tags provide a source of such
// strings.
//
// The chain value is initialized to ensure that it is unique for
// every process that uses CallId.  It is composed by hashing the host
// name, the process ID, and the start time.  The host name, process
// ID, and start time together ensure uniqueness.  The start time is
// used to microsecond resolution because on Windows, process IDs can
// be recycled quickly.
//
// For tags, 8 characters (32 bits) of the chain value are used, per
// RFC 3261 section 19.3.  For Call-Id's, 16 characters (64 bits) of
// the chain value are used, so that 2^32 Call-Id's can be current
// without there being a collision.  24 characters (96) bits of the
// chain value are preserved for iteration to ensure that 32 bits are
// never revealed in an output value.
//
// The sections of the Call-ID are separated with "-" because "-"
// cannot appear in the last two components, and so a
// generated Call-ID can be unambiguously parsed into its components
// (from back to front), regardless of the user-supplied prefix, which
// ensures that regardless of the prefix, this function can never
// (probabilistically) generate duplicate Call-IDs.  "-" is also not
// a special character in the shell or Perl, and so is easy to use in
// the --containing option of merge-logs.
//
// The generated Call-IDs are "words" according to the syntax of RFC
// 3261.  We do not append "@host" for simplicity.  The callIdPrefix
// is assumed to contain only "word" characters, but we check to
// ensure that "@" does not appear because an earlier version of this
// routine checked for "@" and replaced it with "_".

void CallId::getNewCallId(const char* callIdPrefix,
                          UtlString& callId)
{
   // Lock to protect common variables.
   OsLock lock(sCallIdMutex);

   // Increment the call number.
   sCallNum += 1;
    
   // callID prefix shouldn't contain an @.
   if (strchr(callIdPrefix, '@') != NULL)
   {
      OsSysLog::add(FAC_SIP, PRI_ERR,
                    "CallId::getNewCallId callIdPrefix='%s' contains '@'",
                    callIdPrefix);
   }

   // Compute the next value.
   nextValue("");

   // Compose the new Call-Id.
   char buffer[256];
   sprintf(buffer, "%s-%.*s-%d",
           callIdPrefix, 
           CALL_ID_CHAIN_VALUE_REVEALED, sChainValue.data(),
           sCallNum);

   // Copy it to the destination.
   callId.remove(0);
   callId.append(buffer);
}

void CallId::getNewTag(const char* seed,
                       UtlString& tag)
{
   // Lock to protect common variables.
   OsLock lock(sCallIdMutex);

   // Compute the next value.
   nextValue(seed);

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
   // Get the start time.
   OsTime currentTime;
   OsDateTime::getCurTime(currentTime);

   // Get the process ID.
   PID processId;
   processId = OsProcess::getCurrentPID();

   // Get the host identity.
   UtlString thisHost;
   OsSocket::getHostIp(&thisHost);
   // Ensure it does not contain @.
   thisHost.replace('@','*');

   // Force usecs. to be 6 digits with leading zeros so that we do
   // not have to do 64 bit integer math just to build a big unique
   // string.
   char buffer[256];
   sprintf(buffer, "%d/%d.%.6d/%s",
           processId,
           currentTime.seconds(), currentTime.usecs(),
           thisHost.data());
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "CallId::initialize sChainValue generated from '%s'",
                 buffer);
   // Hash them.
   NetMd5Codec encoder;
   encoder.encode(buffer, sChainValue);
   // Truncate the hash to CHAIN_VALUE_LENGTH characters.
   sChainValue.remove(CHAIN_VALUE_LENGTH);

   // Note initialization is done.
   sChainValueInitialized = TRUE;
}

// Compute the next chain value.
void CallId::nextValue(const char* seed)
{
   // If we haven't initialized yet, do so.
   if (!sChainValueInitialized)
   {
      initialize();
   }

   // Get the time.
   OsTime currentTime;
   OsDateTime::getCurTime(currentTime);

   // Force usecs. to be 6 digits with leading zeros so that we do
   // not have to do 64 bit integer math just to build a big unique
   // string.
   char buffer[256];
   sprintf(buffer, "%s/%d%.6d/%.*s/%s",
           sKey.data(),
           currentTime.seconds(), currentTime.usecs(),
           MAX_SEED_CHARS, seed,
           sChainValue.data());

   // Hash them.
   NetMd5Codec encoder;
   encoder.encode(buffer, sChainValue);
   // Truncate the hash to CHAIN_VALUE_LENGTH characters.
   sChainValue.remove(CHAIN_VALUE_LENGTH);
}
