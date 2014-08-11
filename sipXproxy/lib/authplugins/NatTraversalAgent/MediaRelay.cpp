//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "MediaRelay.h"
#include "NatTraversalRules.h"
#include "net/XmlRpcRequest.h"
#include "net/XmlRpcResponse.h"
#include "os/OsLock.h"
#include "os/OsLogger.h"
#include "os/OsProcess.h"
#include "os/OsDateTime.h"
#include "os/OsTask.h"
#include "utl/UtlSListIterator.h"
#include "utl/UtlHashMapIterator.h"
#include "utl/UtlBool.h"


// DEFINES

// common name part of our handle
#define COMMON_HANDLE_NAME "ntap-mediarelay"


MediaRelay::MediaRelay() :
   mMutex( OsMutex::Q_FIFO ),
   mPort(0)
{
  mbIsPartOfsipXLocalPrivateNetwork = false;
  //mMaxMediaRelaySessions = DEFAULT_MAX_MEDIA_RELAY;
   // start the timer that will periodically ping the symmitron and query bridge stats
   //OsTime genericTimerPeriod( GENERIC_TIMER_IN_SECS, 0 );
   //mGenericTimer.periodicEvery( genericTimerPeriod, genericTimerPeriod );

   // seed random number generator
   srand( (unsigned)time(NULL) );   
}

bool MediaRelay::initialize( const  UtlString& publicAddress,
                             const  UtlString& nativeAddress,
                             bool   isPartOfsipXLocalPrivateNetwork,
                             int port)
{
   OsLock lock( mMutex );
   mPublicAddress = publicAddress;
   mNativeAddress = nativeAddress;
   mbIsPartOfsipXLocalPrivateNetwork = isPartOfsipXLocalPrivateNetwork;
   mPort = port;

   return true;
}

MediaRelay::~MediaRelay()
{
}
