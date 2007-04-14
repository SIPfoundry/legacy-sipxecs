// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////

#ifndef _AuthProxyCseObserver_h_
#define _AuthProxyCseObserver_h_

// SYSTEM INCLUDES
#include "utl/UtlString.h"
#include <os/OsQueuedEvent.h>
#include <os/OsTimer.h>

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
class AuthProxyCseObserver : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   AuthProxyCseObserver(SipUserAgent&            sipUserAgent,
                           const UtlString&      dnsName,
                           CallStateEventWriter* pEventWriter
                           );
     //:Default constructor

   virtual
   ~AuthProxyCseObserver();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual UtlBoolean handleMessage(OsMsg& rMsg);

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   SipUserAgent*             mpSipUserAgent;
   CallStateEventBuilder*    mpBuilder;       // Event builder
   CallStateEventWriter*     mpWriter;        // Event writer
   int                       mSequenceNumber;
   OsTimer                   mFlushTimer;
   OsQueuedEvent             mTimerEvent;
   
   /// no copy constructor or assignment operator
   AuthProxyCseObserver(const AuthProxyCseObserver& rAuthProxyCseObserver);
   AuthProxyCseObserver operator=(const AuthProxyCseObserver& rAuthProxyCseObserver);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _AuthProxyCseObserver_h_
