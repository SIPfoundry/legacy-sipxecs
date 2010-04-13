// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////

#ifndef _SipXProxyCseObserver_h_
#define _SipXProxyCseObserver_h_

// SYSTEM INCLUDES
#include "utl/UtlString.h"
#include <os/OsCallback.h>
#include <os/OsMutex.h>
#include <os/OsQueuedEvent.h>
#include <os/OsTimer.h>
#include "net/SipOutputProcessor.h"

// APPLICATION INCLUDES
#include <os/OsServerTask.h>
#include "CallStateEventBuilder_XML.h"
#include "CallStateEventBuilder_DB.h"
#include "CallStateEventWriter_XML.h"
#include "CallStateEventWriter_DB.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipUserAgent;

/// Observe and record Call State Events in the Forking Proxy
class SipXProxyCseObserver : public OsServerTask, SipOutputProcessor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   SipXProxyCseObserver(SipUserAgent&            sipUserAgent,
                           const UtlString&      dnsName,
                           CallStateEventWriter* pEventWriter
                           );
     //:Default constructor

   virtual
   ~SipXProxyCseObserver();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual UtlBoolean handleMessage(OsMsg& rMsg);


   /// Called when SIP messages are about to be sent by proxy
   virtual void handleOutputMessage( SipMessage& message,
                                     const char* address,
                                     int port );

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   SipUserAgent*             mpSipUserAgent;
   CallStateEventBuilder*    mpBuilder;       // Event builder
   CallStateEventWriter*     mpWriter;        // Event writer
   int                       mSequenceNumber;
   OsTimer                   mFlushTimer;
   UtlHashMap                mCallTransMap;
   OsTimer*                  mpCleanupMapTimer;
   OsCallback*               mpCleanupTimeoutCallback;
   OsMutex                   mCallTransMutex;
   
   /// no copy constructor or assignment operator
   SipXProxyCseObserver(const SipXProxyCseObserver& rSipXProxyCseObserver);
   SipXProxyCseObserver operator=(const SipXProxyCseObserver& rSipXProxyCseObserver);

   static void CleanupTransMap(void* userData, const intptr_t eventData);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipXProxyCseObserver_h_
