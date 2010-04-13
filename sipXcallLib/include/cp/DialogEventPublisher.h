//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _DialogEventPublisher_h_
#define _DialogEventPublisher_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <tao/TaoAdaptor.h>
#include <net/SipDialogEvent.h>
#include <net/SipPublishContentMgr.h>
#include <utl/UtlHashMap.h>
#include <utl/UtlHashMapIterator.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class CallManager;
class TaoString;

//! Class for publishing the dialog state changes for each call
/**
 * This class tracks the dialog state changes for each call and publishes
 * dialog event packages, so that a SipSubscribeServer can provide them
 * to subscribers.
 *
 * The DialogEventPublisher object is also a TaoAdaptor, so it is
 * attached to the CallManager using addTaoListener() in order to
 * monitor dialog state changes.  When a call state changes, the
 * DialogEventPublisher generates a new dialog event package and
 * publishes it via the SipPublishContentMgr.  The
 * SipPublishContentMgr is provided to a SipSubscribeServer which
 * handles the subscriptions.  The SipSubscribeServer is usually
 * attached to the SipUserAgent used by the CallManager, so dialog
 * event subscriptions come in to the same port that the monitored
 * dialogs do.
 *
 * The resourceId used to publish the dialog events, and the 'entity'
 * name listed in the dialog events, is computed from the request-URI
 * of incoming INVITEs using DialogEventPublisher::getEntity().
 * getEntity() attaches the user-part of the request-URI to the
 * host-port specified in the arguments to DialogEventPublisher::_, or
 * more commonly, returned by SipUserAgent::getLocalAddress().  This
 * resourceId must be the request-URI of an incoming SUBSCRIBE in order
 * for a subscription to retrieve the desired event packages.
 *
 * (I don't know if DialogEventPublisher tracks outgoing calls correctly.)
 */
class DialogEventPublisher: public TaoAdaptor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */

   DialogEventPublisher(CallManager* callMgr,
                        SipPublishContentMgr* contentMgr,
                        const UtlString& entityHost = "",
                        /**< The host part of the entity URI.
                         *   If "", constructor will use
                         *   mpCallManager->getUserAgent()->getLocalAddress()
                         *   to determine the host and port.
                         */
                        int entityPort = PORT_NONE,
                        ///< The port part of the entity URI.
                        bool maskEstablished = FALSE
                        ///< Whether or not to ignore Established messages and not publish an event for it.
      );
     //:Default constructor

   virtual
   ~DialogEventPublisher();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual UtlBoolean handleMessage(OsMsg& eventMessage);

/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    void dumpTaoMessageArgs(unsigned char eventId, TaoString& args);

    void insertEntry(UtlString& callId, SipDialogEvent* call);
    SipDialogEvent* getEntry(UtlString& callId);
    SipDialogEvent* removeEntry(UtlString& callId);

    // Construct entity from requestUri and local address information
    void getEntity(const UtlString& requestUri,
                   ///< request-URI (from an incoming INVITE)
                   UtlString& entity
                   ///< (output) the entity URI for publishing
       );

    // Delete entry if we get a failed or disconnected event without request Url
    bool findEntryByCallId(UtlString& callId, UtlString& entity);

    // Delete all provisional responses associated with this call
    void deleteProvisionalResponses(UtlString callId, UtlString localTag, SipDialogEvent* pThisCall);

    // Set all provisional response states terminated
    void terminateProvisionalResponses(UtlString callId, UtlString localTag, SipDialogEvent* pThisCall);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    CallManager* mpCallManager;

    SipPublishContentMgr* mpSipPublishContentMgr;

    UtlHashMap mCalls;

    UtlHashMap mProvisionalResponses;

    unsigned long mDialogId;

    /// Host and port for constructing entity URIs.
    UtlString mEntityHost;
    int mEntityPort;
    bool mMaskEstablishedasEarly;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _DialogEventPublisher_h_
