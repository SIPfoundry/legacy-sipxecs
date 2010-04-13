//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _SipPublishServer_h_
#define _SipPublishServer_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsServerTask.h>
#include <os/OsDefs.h>
#include <os/OsRWMutex.h>
#include <utl/UtlString.h>
#include <utl/UtlHashMap.h>
#include <net/SipUserAgent.h>


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// FORWARD DECLARATIONS
class SipPublishServerEventStateCompositor;
class SipUserAgent;
class SipPublishServerEventStateMgr;
class OsMsg;
class SipMessage;


// TYPEDEFS


//! Top level class for accepting and processing PUBLISH requests
/*! This implements a generic RFC 3903 PUBLISH server.  This class
 *  receives PUBLISH requests, passes the event content to the event
 *  state compositor class, and send back the appropriate response.
 *  The SipPublishServer is designed to handle several different event
 *  types so that you can have multiple instances of the SipPublishServer
 *  each handling different event type.  However you can not have an
 *  event type that is handled by more than one SipPublishServer.
 *
 *  \par Event Specific Handling and Processing
 *  Event types are enabled with the enableEventType method.  This method
 *  handling and processing of the specified Event type to be specialized
 *  by providing an Event specific: SipEventPlugin and SipUserAgent.
 *
 *  \par Event State Compositor
 *
 *  \par Event State
 *  The SipPublishServerEventStateMgr is used by SipPublishServer to maintain
 *  the event state (PUBLISH event state not event state content).
 *
 *  \par Overall Data Flow
 *  The SipPublishServer needs to address 2 general stimulus:
 *  1) Respond to incoming PUBLISH requests.
 *  2) Some notification error responses should cause the subscription to expire
 *
 *  When enabling a SIP event type via the enableEventType method, the SipPublishServer
 *  registers with the SipUserAgent to receive PUBLISH requests
 *  for the event type which are processed by the handleMessage method.
 *  The SipPublishServer uses timers to keep track of when event publication expire.
 *  When a timer fires, a message gets queued on the SipPublishServer which is that
 *  passed to handleMessage.
 */

class SipPublishServer : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:


/* ============================ CREATORS ================================== */

    //! Helper utility to build a basic server with default behavior
    static SipPublishServer* buildBasicServer(SipUserAgent& userAgent,
                                              const char* eventType);

    //! Default SipPublishServer constructor
    SipPublishServer(SipUserAgent& defaultUserAgent,
                     SipPublishServerEventStateMgr& defaultEventStateMgr,
                     SipPublishServerEventStateCompositor& defaultCompositor);


    //! Destructor
    virtual
    ~SipPublishServer();


/* ============================ MANIPULATORS ============================== */

    //! Tell the publish server to support given event type
    UtlBoolean enableEventType(const char* eventType,
                               SipUserAgent* userAgent = NULL,
                               SipPublishServerEventStateMgr* eventStateMgr = NULL,
                               SipPublishServerEventStateCompositor* compositor = NULL);

    //! Tell the publish server to stop supporting given event type
    UtlBoolean disableEventType(const char* eventType);

    //! Handler for PUBLISH requests and timers
    UtlBoolean handleMessage(OsMsg &eventMessage);

/* ============================ ACCESSORS ================================= */

    //! Get the event state compositor for the given eventType
    /*! WARNING: there is no locking of the event state compositor once it is
     *  returned.  If the eventStateCompositor is removed via disableEventType
     *  and destroyed, there is no locking protection.  The eventStateCompositor
     *  is only safe to use if the application knows that it is not going
     *  to get the rug pulled out from under it.  Returns the default
     *  event state compositor if there is not an event specific state compositor.
     */
    SipPublishServerEventStateCompositor*
        getEventStateCompositor(const UtlString& eventType);

    //! Get the event state manager for the given event type
    /*! WARNING: there is no locking of the event state manager once it is
     *  returned.  If the event state manager is removed via disableEventType
     *  and destroyed, there is no locking protection.  The event state manager
     *  is only safe to use if the application knows that it is not going
     *  to get the rug pulled out from under it.  Returns the default
     *  event state manager if there is not an event specific state
     *  manager.
     */
    SipPublishServerEventStateMgr* getEventStateMgr(const UtlString& eventType);

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    //! Copy constructor NOT ALLOWED
    SipPublishServer(const SipPublishServer& rSipPublishServer);

    //! Assignment operator NOT ALLOWED
    SipPublishServer& operator=(const SipPublishServer& rhs);

    //! Handle PUBLISH requests
    UtlBoolean handlePublish(const SipMessage& publishRequest);

    //! lock for single thread write access (add/remove event handlers)
    void lockForWrite();

    //! unlock for use
    void unlockForWrite();

    //! lock for multiple-thread read access
    void lockForRead();

    //! unlock for use
    void unlockForRead();

    SipUserAgent* mpDefaultUserAgent;
    SipPublishServerEventStateMgr* mpDefaultEventStateMgr;
    SipPublishServerEventStateCompositor* mpDefaultCompositor;
    UtlHashMap mEventDefinitions;
    OsRWMutex mPublishServerMutex;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipPublishServer_h_
