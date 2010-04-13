// 
// Copyright (C) 2008 Nortel, certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>
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
#include "testlib/SipDbTestContext.h"
#include "utl/UtlSList.h"
#include "RlsTestFixtures.h"

CallbackTrace::CallbackTrace( SipMessage& message,
                const char* address,
                int port ) : 
      mMessage( message ),
      mAddress( address ),
      mPort( port )
{
   struct timeval tv;
   gettimeofday(&tv, NULL);
   mTimestampInMillisecs = tv.tv_sec*1000ull + tv.tv_usec / 1000ull;    
}

long long CallbackTrace::getTimeStamp( void ) const
{
   return mTimestampInMillisecs;
}

void CallbackTrace::setMessage( SipMessage& message )
{
   mMessage = message;
}
   
SipMessage& CallbackTrace::getMessage( void ) 
{
   return mMessage;
}
   
void CallbackTrace::setAddress( UtlString address )
{
   mAddress = address;
}
   
const UtlString& CallbackTrace::getAddress( void ) const
{
   return mAddress;
}
   
void CallbackTrace::setPort( int port )
{
   mPort = port;
}

int CallbackTrace::getPort( void ) const
{
   return mPort;
}

bool CallbackTrace::operator==( const CallbackTrace& rhs ) const
{
   UtlString localMessage, rhsMessage;
   ssize_t       localLen,     rhsLen;
   
   // this method is often used to compare a message
   // captured by the output processor against a reference message
   // sent.  Given that, headers that have non-deterministric or 
   // hard-to-predict values are not considered when doing the 
   // comparison.  These headers are Date: Via: and User-Agent:.
   // We also remove the To-tag, as it is not predictable.
   SipMessage tmpMessage1( mMessage );
   tmpMessage1.removeTopVia();
   tmpMessage1.removeHeader( HTTP_DATE_FIELD, 0 );
   tmpMessage1.removeHeader( HTTP_USER_AGENT_FIELD, 0 );

   UtlString toField;
   tmpMessage1.getToField(&toField);
   Url toUri(toField, Url::NameAddr);
   toUri.removeFieldParameter("tag");
   // Force <...> to be used when serializing toUri.
   toUri.includeAngleBrackets();
   toUri.toString(toField);
   tmpMessage1.setRawToField(toField);

   SipMessage tmpMessage2( rhs.mMessage );
   tmpMessage2.removeTopVia();
   tmpMessage2.removeHeader( HTTP_DATE_FIELD, 0 );
   tmpMessage2.removeHeader( HTTP_USER_AGENT_FIELD, 0 );
   
   tmpMessage2.getToField(&toField);
   toUri.removeFieldParameter("tag");
   // Force <...> to be used when serializing toUri.
   toUri.includeAngleBrackets();
   toUri.toString(toField);
   tmpMessage2.setRawToField(toField);

   tmpMessage1.getBytes( &localMessage, &localLen );
   tmpMessage2.getBytes( &rhsMessage, &rhsLen );
   
   return ( ( localLen == rhsLen )                        &&
            ( localMessage.compareTo( rhsMessage ) == 0 ) &&
            ( mAddress.compareTo( rhs.mAddress ) == 0 )   &&
            ( mPort == rhs.mPort ) );
}

UtlContainableType CallbackTrace::getContainableType() const 
{ 
   return "CallbackTrace"; 
}

unsigned CallbackTrace::hash() const
{ 
   return directHash(); 
}

int CallbackTrace::compareTo(UtlContainable const *rhsContainable ) const
{
   CallbackTrace* rhs = (CallbackTrace*)rhsContainable;
   if( mPort != rhs->mPort )
   {
      return ( mPort < rhs->mPort ) ? -1 : 1;  
   }
   else
   {
      UtlString thisCallbackTraceString;
      UtlString rhsCallbackTraceString;
      ssize_t len;
      
      mMessage.getBytes( &thisCallbackTraceString, &len );
      thisCallbackTraceString.append( mAddress );
      
      rhs->mMessage.getBytes( &rhsCallbackTraceString, &len );
      rhsCallbackTraceString.append( rhs->mAddress );

      return( thisCallbackTraceString.compareTo( rhsCallbackTraceString ) );
   }
}

OutputProcessorFixture::OutputProcessorFixture( uint prio, bool restDuringCallback ) : 
      SipOutputProcessor( prio ),
      mRestDuringCallback( restDuringCallback ),
      mMutex( OsMutex::Q_FIFO )
{
}

void OutputProcessorFixture::handleOutputMessage( SipMessage& message,
                          const char* address,
                          int port )
{
   OsLock lock( mMutex );
   if( mRestDuringCallback )
   {
      sleep(1);
   }
   UtlContainable* pCallbackTrace = new CallbackTrace( message, address, port );
   mCallbackTraceList.append( pCallbackTrace );
   mEvent.signal(0);

}

// function that waits no more than 'maxWaitInSecs' for
// 'numberOfMessages' received by the processor
bool OutputProcessorFixture::waitForMessage(long maxWaitInSecs )
{
   bool bMessagesReceived = false;

   {
      OsLock lock(mMutex);
      if(mCallbackTraceList.entries() > 0)
      {
         bMessagesReceived = true;
      }
      mEvent.reset();  
   }

   OsTime wTime(maxWaitInSecs*1000);
 
   if(!bMessagesReceived &&
      OS_SUCCESS == mEvent.wait(wTime))
   {
      // check to make sure that the CallbackTrace is not empty
      OsLock lock(mMutex);
      if(mCallbackTraceList.entries() > 0)
      {
         bMessagesReceived = true;
      }
      mEvent.reset();
   }

   return bMessagesReceived;
}

CallbackTrace& OutputProcessorFixture::getLastStoredCallbackTrace( void ) 
{
   OsLock lock( mMutex );
   return *( (CallbackTrace*)(mCallbackTraceList.last()) );
}

CallbackTrace& OutputProcessorFixture::getStoredCallbackTrace( size_t index )
{
   OsLock lock( mMutex );
   return *( (CallbackTrace*)(mCallbackTraceList.at( index )) );
}

bool OutputProcessorFixture::popNextCallbackTrace( CallbackTrace& trace )
{
   OsLock lock( mMutex );
   bool result = false;
   CallbackTrace* pCallbackTrace = (CallbackTrace*)(mCallbackTraceList.removeAt( 0 ));
   if( pCallbackTrace )
   {
      trace = *pCallbackTrace;
      result = true;
   }
   return result;
}

void OutputProcessorFixture::resetCallbackTrace( void )
{
   OsLock lock( mMutex );
   mCallbackTraceList.destroyAll();
}

int OutputProcessorFixture::getCallbackCount( void )
{
   OsLock lock( mMutex );
   return mCallbackTraceList.entries();
}

