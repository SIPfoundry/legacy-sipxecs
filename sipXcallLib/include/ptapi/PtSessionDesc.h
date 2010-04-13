//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtSessionDesc_h_
#define _PtSessionDesc_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <utl/UtlString.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Session attributes that are used when setting up a new connection.
// The specifics of the interface to this class are under construction.

class PtSessionDesc : public UtlString
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

   PtSessionDesc();
     //:Default constructor

   PtSessionDesc(const char* callId,
                                 const char* toUrl = NULL,
                                 const char* fromUrl = NULL,
                 const char* localContact = NULL,
                                 int nextCseq = -1,
                                 int lastFromCseq = -1,
                                 int lastToCseq = -1,
                                 int sessionState = SESSION_UNKNOWN);
     //:constructor

   PtSessionDesc(const PtSessionDesc& rPtSessionDesc);
     //:Copy constructor (not implemented for this class)

   virtual
   ~PtSessionDesc();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PtSessionDesc& operator=(const PtSessionDesc& rhs);
     //:Assignment operator (not implemented for this class)

   void setCallId(const char* callId);
   void setFromUrl(const UtlString& fromUrl);
   void setToUrl(const UtlString& toUrl);
   void setLocalContact(const UtlString& localContact);
   void setLastFromCseq(int seqNum);
   void setLastToCseq(int seqNum);


/* ============================ ACCESSORS ================================= */

   void getCallId(UtlString& callId);
   void getFromUrl(UtlString& fromUrl);
   void getToUrl(UtlString& toUrl);
   void getLocalContact(UtlString& localContact);
   int getNextFromCseq();
   int getLastFromCseq();
   int getLastToCseq();
   int getSessionState();
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    // The callId is stored in the UtlString base class data element
    UtlString mFromUrl;
    UtlString mToUrl;
    UtlString mLocalContact;

    int mNextCseq;
    int mLastFromCseq;
    int mLastToCseq;
    int mSessionState;


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtSessionDesc_h_
