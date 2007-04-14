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
 * This class tracks the dialog state changes for each call and generates
 * a dialog event package as described in RFC 4235 (An
 * INVITE-Initiated Dialog Event Package for SIP) and sends it to a generic
 * RFC 3265 SUBSCRIBE server or NOTIFIER.
 */

class DialogEventPublisher: public TaoAdaptor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */

   DialogEventPublisher(CallManager* callMgr, SipPublishContentMgr* contentMgr);
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
    void getEntity(UtlString& requestUri, UtlString& entity);
    
    // Delete entry if we get a failed or disconnected event without request Url
    bool findEntryByCallId(UtlString& callId, UtlString& entity);
 
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    CallManager* mpCallManager;
    
    SipPublishContentMgr* mpSipPublishContentMgr;

    UtlHashMap mCalls;

    unsigned long mDialogId;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _DialogEventPublisher_h_
