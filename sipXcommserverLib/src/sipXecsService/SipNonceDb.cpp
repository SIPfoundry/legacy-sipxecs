//
// Copyright (C) 2007, 2010 Avaya, Inc., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDateTime.h"
#include "os/OsTime.h"
#include "os/OsSysLog.h"
#include "os/OsConfigDb.h"
#include "net/NetMd5Codec.h"
#include "os/OsLock.h"
#include "sipXecsService/SipXecsService.h"
#include "sipXecsService/SharedSecret.h"
#include "sipXecsService/SipNonceDb.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define HEX_TIMESTAMP_LENGTH 8

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipNonceDb::SipNonceDb()
{
   // read the (global) domain configuration
   OsConfigDb domainConfiguration;
   domainConfiguration.loadFromFile(SipXecsService::domainConfigPath());
   // get the shared secret for generating signatures
   mpNonceSignatureSecret = new SharedSecret(domainConfiguration);
}

// Destructor
SipNonceDb::~SipNonceDb()
{
   delete mpNonceSignatureSecret;
}

/* ============================ MANIPULATORS ============================== */

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
//  <timestamp>   := (hexadecimal seconds since the epoch)
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

void SipNonceDb::createNewNonce(const UtlString& callId,  ///< input
                                const UtlString& fromTag, ///< input
                                const UtlString& realm,   ///< input
                                UtlString& nonce          ///< output
                                )
{
   unsigned long now = OsDateTime::getSecsSinceEpoch();
   char dateString[HEX_TIMESTAMP_LENGTH+1];
   UtlString nonceSignature;

   // create the timestamp, which will be in the clear
   sprintf(dateString, "%08x", (int)now);
   nonce = SipNonceDb::nonceSignature(callId,fromTag,realm,dateString);
   nonce.append(dateString);
}

/* ============================ INQUIRY =================================== */

UtlBoolean SipNonceDb::isNonceValid(const UtlString& nonce,
                                    const UtlString& callId,
                                    const UtlString& fromTag,
                                    const UtlString& realm,
                                    const long expiredTime)
{
   UtlBoolean valid = FALSE;

   if (nonce.length() == (MD5_SIZE + HEX_TIMESTAMP_LENGTH))
   {
      UtlString timestamp = nonce(MD5_SIZE, HEX_TIMESTAMP_LENGTH);   // get timestamp from nonce string
      UtlString rcvdSignature = nonce(0,MD5_SIZE);                   // get signature from nonce string

      // calculate valid signature for supplied data using known secret
      UtlString msgSignature(nonceSignature(callId, fromTag, realm, timestamp.data()));
      if (0 == rcvdSignature.compareTo(msgSignature))
      {
         // check for expiration
         char* end;
         long nonceCreated = strtol(timestamp.data(), &end, 16 /* hex */);
         long now = OsDateTime::getSecsSinceEpoch();

         if ( nonceCreated+expiredTime >= now )
         {
            valid = TRUE;
         }
         else
         {
            OsSysLog::add(FAC_SIP,PRI_INFO,
                          "SipNonceDB::isNonceValid expired nonce '%s': created %ld+%ld < %ld",
                          nonce.data(), nonceCreated, expiredTime, now
                          );
         }
      }
      else
      {
         OsSysLog::add(FAC_SIP,PRI_ERR,
                       "SipNonceDB::isNonceValid nonce signature check failed '%s'",
                       nonce.data()
                       );
         OsSysLog::add(FAC_SIP,PRI_DEBUG,
                       "SipNonceDB::isNonceValid rcvd signature '%s' calculated signature '%s'",
                       rcvdSignature.data(), msgSignature.data()
                       );
      }
   }
   else
   {
      OsSysLog::add(FAC_SIP,PRI_ERR,
                    "SipNonceDb::isNonceValid invalid nonce format '%s'"
                    " length %zu expected %d",
                    nonce.data(), nonce.length(), MD5_SIZE+HEX_TIMESTAMP_LENGTH);
   }

   return(valid);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

UtlString SipNonceDb::nonceSignature(const UtlString& callId,
                                     const UtlString& fromTag,
                                     const UtlString& realm,
                                     const char* timestamp)
{
   UtlString signatureValue;

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipNonceDb::nonceSignature: callId='%s' fromTag='%s' realm='%s' "
                 "timestamp='%s'",
                 callId.data(), fromTag.data(), realm.data(), timestamp
                 );

   // Create the signature value by hashing the timestamp with
   // the callId, fromTag, the realm, and a secret.
   // Add separator commas between non-fixed-length components to
   // ensure, e.g., callid 'ABC' and from-tag 'DEF' does not hash the
   // same as callid 'ABCD' and from-tag 'EF'.
   NetMd5Codec md5;
   md5.hash(timestamp, HEX_TIMESTAMP_LENGTH);
   md5.hash(callId);
   md5.hash(",");
   md5.hash(fromTag);
   md5.hash(",");
   md5.hash(*mpNonceSignatureSecret);
   md5.hash(",");
   md5.hash(realm);

   md5.appendHashValue(signatureValue);

   return signatureValue;
}

OsBSem*     SharedNonceDb::spLock       = new OsBSem(OsBSem::Q_PRIORITY, OsBSem::FULL);
SipNonceDb* SharedNonceDb::spSipNonceDb = NULL;

SipNonceDb* SharedNonceDb::get()
{
   // critical region to ensure that only one shared secret is created
   OsLock lock(*spLock);

   if (!spSipNonceDb)
   {
      spSipNonceDb = new SipNonceDb();
   }
   return spSipNonceDb;
}
