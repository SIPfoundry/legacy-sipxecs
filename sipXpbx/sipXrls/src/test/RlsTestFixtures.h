// 
// Copyright (C) 2008 Nortel, certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef __RlsTestFixtures_h__
#define __RlsTestFixtures_h__

#include <sys/time.h>
#include <utl/UtlSList.h>
#include <utl/UtlString.h>
#include <os/OsDefs.h>
#include <net/SipDialogEvent.h>
#include <net/SipMessage.h>
#include <net/SipOutputProcessor.h>
#include <net/SipUserAgent.h>
#include <string>
#include "os/OsSysLog.h"
#include "os/OsMutex.h"
#include "os/OsLock.h"
#include "os/OsEvent.h"
#include "testlib/SipDbTestContext.h"
#include "utl/UtlSList.h"





// Class used to store SIP message, IP address and port
// information provided by an  output processor callback call.
class CallbackTrace : public UtlContainable
{
private:
   SipMessage mMessage;
   UtlString  mAddress;
   int        mPort;
   unsigned long long  mTimestampInMillisecs;
   
public:
   CallbackTrace(){}
   
   CallbackTrace( SipMessage& message,
                const char* address,
                int port ); 

   long long getTimeStamp( void ) const;
   void setMessage( SipMessage& message );
   SipMessage& getMessage( void ); 
   void setAddress( UtlString address );
   const UtlString& getAddress( void ) const;
   void setPort( int port );
   int getPort( void ) const;
   bool operator==( const CallbackTrace& rhs ) const;
   virtual UtlContainableType getContainableType() const;
   virtual unsigned hash() const;
   virtual int compareTo(UtlContainable const *rhsContainable ) const;
};

// Class that represents an Output Processor
// This class saves all the information it receives via
// the output processor callbacks into a list for later retrieval
// by unit test cases.
class OutputProcessorFixture : public SipOutputProcessor
{
private:
   UtlSList      mCallbackTraceList;
   bool          mRestDuringCallback;
   OsMutex       mMutex;
   OsEvent       mEvent;

public:
   OutputProcessorFixture( uint prio = 0, bool restDuringCallback = false ); 
   void handleOutputMessage( SipMessage& message,
                             const char* address,
                             int port );
   
   // function that waits no more than 'maxWaitInSecs'
   bool waitForMessage( long maxWaitInSecs );
   CallbackTrace& getLastStoredCallbackTrace( void ); 
   CallbackTrace& getStoredCallbackTrace( size_t index );
   bool popNextCallbackTrace( CallbackTrace& trace );
   void resetCallbackTrace( void );
   int getCallbackCount( void );
};

#endif //__RlsTestFixtures_h__
