//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdlib.h>

// APPLICATION INCLUDES
#include <os/OsSysLog.h>
#include <net/Url.h>
#include <net/SipMessage.h>
#include <sipdb/ResultSet.h>
#include <registry/SipRegistrar.h>
#include <SipRedirectorTest.h>
#include <registry/SipRedirectServer.h>

//*
// Special test redirector.
//
// Looks for a parameter named "t<instance-name>".  If it is
// present, the redirector will execute a series of suspensions.  The
// string is a set of fields separated by slashes, which are processed
// one for each cycle of redirector processing in the request.  An
// empty field means to not request suspension (for this processing),
// a non-empty field means to request suspension and then request
// resume after that number of seconds.  A field of '*' means to return
// ERROR on that cycle.  The redirector will never add a contact
// to the call.
//
// E.g., "t1=10" means to suspend for 10 seconds.
// "t1=10;t2=5" means that redirector "1" will suspend for 10 seconds
// and redirector "2" will suspend for 5 seconds.  (And the reprocessing
// will begin at 10 seconds.)
// "t1=10/;t2=/10" means that redirector 1 will request a 10 second
// suspension, and upon reprocessing, redirector 2 will request a 10
// second suspension.  The request will finish on the third cycle.
// "t1=10/*" will suspend for 10 seconds on the first cycle and then return
// ERROR on the second cycle.

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
UtlContainableType SipRedirectorPrivateStorageTest::TYPE =
    "SipRedirectorPrivateStorageTest";

// Static factory function.
extern "C" RedirectPlugin* getRedirectPlugin(const UtlString& instanceName)
{
   return new SipRedirectorTest(instanceName);
}

// Constructor
SipRedirectorTest::SipRedirectorTest(const UtlString& instanceName) :
   RedirectPlugin(instanceName)
{
   mLogName.append("[");
   mLogName.append(instanceName);
   mLogName.append("] SipRedirectorTest");

   strcpy(mParameterName, "t");
   strcat(mParameterName, mLogName.data());
}

// Destructor
SipRedirectorTest::~SipRedirectorTest()
{
}

// Initializer
OsStatus
SipRedirectorTest::initialize(OsConfigDb& configDb,
                              int redirectorNo,
                               const UtlString& localDomainHost)
{
   return OS_SUCCESS;
}

// Finalizer
void
SipRedirectorTest::finalize()
{
}

RedirectPlugin::LookUpStatus SipRedirectorPrivateStorageTest::actOnString()
{
   // Check if we should return an error.
   if (mPtr[0] == '*')
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipRedirectorPrivateStorageTest::actOnString Returning "
                    "ERROR, from string '%s', remaining '%s'",
                    mString, mPtr);
      // We don't have to update anything, since we will not be called again.
      return RedirectPlugin::ERROR;
   }
   // strtol will return 0 if mPtr[0] == / or NUL.
   int wait = strtol(mPtr, &mPtr, 10);
   if (mPtr[0] == '/')
   {
      mPtr++;
   }
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipRedirectorPrivateStorageTest::actOnString Wait time %d, from string '%s', remaining '%s'",
                 wait, mString, mPtr);
   // 0 is not a valid wait time.
   if (wait == 0)
   {
      return RedirectPlugin::SUCCESS;
   }
   else
   {
      mTimer.oneshotAfter(OsTime(wait, 0));
      return RedirectPlugin::SEARCH_PENDING;
   }
}

RedirectPlugin::LookUpStatus
SipRedirectorTest::lookUp(
   const SipMessage& message,
   const UtlString& requestString,
   const Url& requestUri,
   const UtlString& method,
   ContactList& contactList,
   RequestSeqNo requestSeqNo,
   int redirectorNo,
   SipRedirectorPrivateStorage*& privateStorage,
   ErrorDescriptor& errorDescriptor)
{
   UtlString parameter;

   //OsSysLog::add(FAC_SIP, PRI_DEBUG,
   //              "%s::LookUp redirectorNo %d, mParameterName '%s'",
   //              mLogName.data(), redirectorNo, mParameterName);

   if (!requestUri.getFieldParameter(mParameterName, parameter))
   {
      return RedirectPlugin::SUCCESS;
   }

   if (!privateStorage)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "%s::LookUp Creating storage for parameter '%s', requestSeqNo %d, redirectorNo %d",
                    mLogName.data(),
                    parameter.data(), requestSeqNo, redirectorNo);
      SipRedirectorPrivateStorageTest *storage =
         new SipRedirectorPrivateStorageTest(parameter, requestSeqNo,
                                             redirectorNo);
      privateStorage = storage;
   }

   return ((SipRedirectorPrivateStorageTest*) privateStorage)->actOnString();
}


const UtlString& SipRedirectorTest::name( void ) const
{
   return mLogName;
}

SipRedirectorPrivateStorageTest::SipRedirectorPrivateStorageTest(
   const char *string,
   RedirectPlugin::RequestSeqNo requestSeqNo,
   int redirectorNo) :
   mNotification(requestSeqNo, redirectorNo),
   mTimer(mNotification)
{
   // Copy the string.
   mString = new char[strlen(string) + 1];
   strcpy(mString, string);
   // Initialize the parsing pointer.
   mPtr = mString;
}

SipRedirectorPrivateStorageTest::~SipRedirectorPrivateStorageTest()
{
   free(mString);
}

UtlContainableType SipRedirectorPrivateStorageTest::getContainableType() const
{
    return SipRedirectorPrivateStorageTest::TYPE;
}

SipRedirectorTestNotification::SipRedirectorTestNotification(
   RedirectPlugin::RequestSeqNo requestSeqNo,
   int redirectorNo) :
   mRequestSeqNo(requestSeqNo),
   mRedirectorNo(redirectorNo)
{
}

OsStatus SipRedirectorTestNotification::signal(const intptr_t eventData)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipRedirectorTestNotification::signal Fired mRequestSeqNo %d, mRedirectorNo %d",
                 mRequestSeqNo, mRedirectorNo);
   SipRedirectServer::getInstance()->
      resumeRequest(mRequestSeqNo, mRedirectorNo);
   return OS_SUCCESS;
}
