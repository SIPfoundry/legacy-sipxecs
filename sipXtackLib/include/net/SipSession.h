//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _SipSession_h_
#define _SipSession_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES

#include <os/OsDefs.h>
#include <net/Url.h>
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
class SipMessage;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class SipSession : public UtlString
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    enum SessionState
    {
        SESSION_UNKNOWN,
        SESSION_INITIATED,
        SESSION_SETUP,
        SESSION_FAILED,
        SESSION_TERMINATED
    };

/* ============================ CREATORS ================================== */

   SipSession(const SipMessage* initialMessage = NULL,
              UtlBoolean isFromLocal = TRUE);
     //:Default constructor

   SipSession(const char* callId, const char* toUrl, const char* fromUrl);
     //:Constructor accepting the basic pieces of a session callId, toUrl,
     // and from Url.

   virtual
   ~SipSession();
     //:Destructor

   SipSession(const SipSession& rSipSession);
     //:Copy constructor

   SipSession& operator=(const SipSession& rhs);
     //:Assignment operator

/* ============================ MANIPULATORS ============================== */

   void updateSessionData(SipMessage& message);

/* ============================ ACCESSORS ================================= */

   void getCallId(UtlString& callId);
   void setCallId(const char* callId);

   void getFromUrl(Url& fromUrl);
   void setFromUrl(const Url& fromUrl);

   void getToUrl(Url& toUrl);
   void setToUrl(const Url& toUrl);

   void getRemoteContact(Url& remoteContact);
   void setRemoteContact(const Url& remoteContact);

   void getLocalContact(Url& localContact);
   void setLocalContact(const Url& localContact);

   void getInitialMethod(UtlString& method);
   void setInitialMethod(const char* method);

   int getNextFromCseq();
   int getLastFromCseq();
   void setLastFromCseq(int seqNum);

   int getLastToCseq();
   void setLastToCseq(int seqNum);

   void getLocalRequestUri(UtlString& requestUri);
   void setLocalRequestUri(UtlString& requestUri);
   void getRemoteRequestUri(UtlString& requestUri);
   void setRemoteRequestUri(UtlString& requestUri);
   void getProvisionalToTags(UtlHashMap& provisionalToTags);
   void setProvisionalToTags(UtlHashMap& provisionalToTags);

   int getSessionState() { return mSessionState;};

   void toString(UtlString& output) const;

/* ============================ INQUIRY =================================== */

   UtlBoolean isSameSession(const SipMessage& message) const;

   // Returns TRUE if the message is a part of this dialog/session AND was
   // sent by the local UA
   UtlBoolean isMessageFromInitiator(const SipMessage& message) const;

   // Returns TRUE if the message is a part of this dialog/session AND was
   // sent by the remote UA
   UtlBoolean isMessageFromDestination(const SipMessage& message) const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    // The callId is stored in the UtlString base class data element
    Url mLocalUrl;
    Url mRemoteUrl;
    Url mLocalContact;
    Url mRemoteContact;
    UtlString mInitialMethod;
    UtlString msLocalRequestUri;
    UtlString msRemoteRequestUri;
    int mInitialLocalCseq;
    int mInitialRemoteCseq;
    int mLastFromCseq;
    int mLastToCseq;
    int mSessionState;
    UtlHashMap mProvisionalToTags;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipSession_h_
