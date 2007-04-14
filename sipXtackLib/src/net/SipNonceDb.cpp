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

// APPLICATION INCLUDES
#include <net/SipNonceDb.h>
#include <os/OsDateTime.h>
#include <os/OsTime.h>
#include <os/OsSysLog.h>
#include <net/NetMd5Codec.h>
#include "os/OsLock.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// :TBD: replace this with a cluster-specific secret
#define NONCE_SIGNATURE_INNER_SECRET "66287423e9e289fc9d78a3bd04475b06"

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipNonceDb::SipNonceDb()
{
   mNonceSignatureSecret = NONCE_SIGNATURE_INNER_SECRET;
   // :TBD: append some configuration-specific value here
}

// Copy constructor
SipNonceDb::SipNonceDb(const SipNonceDb& rSipNonceDb)
{
}

// Destructor
SipNonceDb::~SipNonceDb()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipNonceDb&
SipNonceDb::operator=(const SipNonceDb& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}


// **********************************************************
// Sip Nonce construction
//
// We want our servers to be able to use a common algorithm
// to construct the authentication nonce values so that they
// can create authenticated requests to each other without
// sending an unauthenticated request to get the challenge.
// At the same time, it must not be possible for any other party
// to produce a valid nonce or predict what any future valid
// nonce would be, since that would undermine the security
// of Digest authentication.
//
// We accomplish this by constructing the nonce value as:
//  <nonce>       := <signature><timestamp>
//  <timestamp>   := (decimal seconds since the epoch)
//  <signature>   := H(<timestamp><call-params><secret><realm>)
//  <call-params> := <callId><fromTag>
//  <secret>      := NONCE_SIGNATURE_INNER_SECRET
//
//  H(x) is the hexadecimal encoding of MD5(x)
//
// Using the call parameters incorporates them into the
// integrity protection of the authentication response,
// while allowing the nonce to be usefull for any request
// within the call.
//
// The timestamp allows us to detect that an unacceptably old
// nonce is being used, and ensures that the inputs to the hash
// are changed by something not in the control of the requestor.
//
// NB: The timestamp creates the requirement that all servers
// have clocks that are roughly syncronized (the expiration
// time is set by the caller requesting validation of a nonce,
// at present to 5 minutes)

void SipNonceDb::createNewNonce(const UtlString& callId, //input
                                const UtlString& fromTag, // input
                                const UtlString& uri, // :TBD: no longer used
                                const UtlString& realm, // input
                                UtlString& nonce) // output
{
   long now = OsDateTime::getSecsSinceEpoch();
   char dateString[20];
   UtlString nonceSignature;

   // create the timestamp, which will be in the clear
   sprintf(dateString, "%ld", now);
   nonce = SipNonceDb::nonceSignature(callId,fromTag,uri,realm,dateString);
   nonce.append(dateString);
}

void SipNonceDb::removeOldNonces(long oldTime)
{
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

UtlBoolean SipNonceDb::isNonceValid(const UtlString& nonce,
                                    const UtlString& callId,
                                    const UtlString& fromTag,
                                    const UtlString& uri, //:TBD: no longer used
                                    const UtlString& realm,
                                    const long expiredTime)
{
   UtlBoolean valid = FALSE;

   if (nonce.length() > MD5_SIZE)
   {
      UtlString timestamp = nonce(MD5_SIZE,nonce.length()-MD5_SIZE);
      UtlString rcvdSignature = nonce(0,MD5_SIZE);
      if (0 == rcvdSignature.compareTo(nonceSignature(callId, fromTag,
                                                      uri, realm,
                                                      timestamp)))
      {
         // check for expiration
         int nonceCreated = atoi(timestamp.data());
         long now = OsDateTime::getSecsSinceEpoch();

         if ( nonceCreated+expiredTime >= now )
         {
            valid = TRUE;
         }
         else
         {
            OsSysLog::add(FAC_SIP,PRI_INFO,
                          "SipNonceDB::isNonceValid expired nonce: created %d+%ld < %ld"
                          ,nonceCreated, expiredTime, now
                          );
         }
      }
      else
      {
         OsSysLog::add(FAC_SIP,PRI_ERR,
                       "SipNonceDB::isNonceValid nonce signature check failed"
                       );
      }
   }
   else
   {
      OsSysLog::add(FAC_SIP,PRI_ERR,
                    "SipNonceDb::isNonceValid invalid nonce format \"%s\"\n",
                    nonce.data());
   }

   return(valid);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

UtlString SipNonceDb::nonceSignature(const UtlString& callId,
                                     const UtlString& fromTag,
                                     const UtlString& uri, // :TBD: no longer used
                                     const UtlString& realm,
                                     const UtlString& timestamp)
{
   UtlString signature;
   UtlString nonceSigningData(timestamp);

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "nonceSignature: callId='%s' fromTag='%s' realm='%s' timestamp='%s'",
                 callId.data(), fromTag.data(), realm.data(), timestamp.data()
                 );
    
   // create the signature value by hashing the timestamp with
   //   the callId, fromTag, the realm, and a secret
   nonceSigningData.append(callId);
   nonceSigningData.append(fromTag);
   nonceSigningData.append(mNonceSignatureSecret);
   nonceSigningData.append(realm);

   NetMd5Codec::encode(nonceSigningData.data(), signature);

   return signature;
}

OsBSem*     SharedNonceDb::spLock       = new OsBSem(OsBSem::Q_PRIORITY, OsBSem::FULL);
SipNonceDb* SharedNonceDb::spSipNonceDb = NULL;

SipNonceDb* SharedNonceDb::get()
{
   // critical region to ensure that only one shared ssl context is created
   OsLock lock(*spLock);

   if (!spSipNonceDb)
   {
      spSipNonceDb = new SipNonceDb();
   }
   return spSipNonceDb;
}
