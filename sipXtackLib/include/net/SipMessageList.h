//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _SipMessageList_h_
#define _SipMessageList_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <net/SipMessage.h>
#include <os/OsLockingList.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class SipMessageList
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    SipMessageList();
    //:Default constructor

    virtual
    ~SipMessageList();
    //:Destructor

/* ============================ MANIPULATORS ============================== */



/* ============================ ACCESSORS ================================= */
    SipMessage* getResponseFor(SipMessage* request);
    SipMessage* getRequestFor(SipMessage* response);
    SipMessage* getDuplicate(SipMessage* message,
      UtlBoolean responseCodesMustMatch = FALSE);
    SipMessage* getAckFor(SipMessage* inviteRequest);
    //SDUA
    SipMessage* getInviteFor(SipMessage* cancelRequest);
    SipMessage* isSameFrom(const Url& fromUrl);
    SipMessage* isSameTo(const Url& toUrl);
    SipMessage* isSameCallId(const UtlString& callId);

    int getListSize();
    int getIterator();
    SipMessage* getSipMessageForIndex( int iteratorHandle );
    void releaseIterator( int iteratorHandle );

    UtlBoolean remove(SipMessage* message);
    void add(SipMessage* message);

    void removeOldMessages(long oldTime, UtlBoolean deleteMessages = TRUE);
    void remove(int iteratorHandle);

    void toString(UtlString& listDumpString);

    void printDebugTable() ;
/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    OsLockingList messageList;

    SipMessageList(const SipMessageList& rSipMessageList);
    //:Copy constructor (disabled)

    SipMessageList& operator=(const SipMessageList& rhs);
    //:Assignment operator (disabled)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipMessageList_h_
