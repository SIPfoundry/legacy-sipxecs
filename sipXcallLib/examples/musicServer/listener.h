//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#ifndef _Listener_h_
#define _Listener_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <tao/TaoAdaptor.h>
#include <mp/MpStreamPlaylistPlayer.h>
#include <ActiveCall.h>
#include <utl/UtlSortedList.h>
#include <os/OsRWMutex.h>

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

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class Listener : public TaoAdaptor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   Listener(CallManager* callManager = NULL, UtlString playfile = "default.wav");

   //:Default constructor
   ~Listener();

/* ============================ MANIPULATORS ============================== */

   UtlBoolean handleMessage(OsMsg& rMsg);
     //: Method to process messages which get queued on this
     /**  The is the method that does all the work.  Telephony
       *  events get queued on this which consequently get
       *  processed in handleMessage in the contect of this
       *  task.
       */

/* ============================ ACCESSORS ================================= */

   int totalCalls() {
      return mTotalCalls;
   };

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    void dumpTaoMessageArgs(unsigned char eventId, TaoString& args);

    void insertEntry(UtlString& callId, CallObject* call);
    CallObject* removeEntry(UtlString& callId);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    UtlString mPlayfile;

    Listener(const Listener& rListener);
     //:Copy constructor

    Listener& operator=(const Listener& rhs);
     //:Assignment operator

    CallManager* mpCallManager;

    UtlSortedList mCalls;

    /** reader/writer lock for synchronization */
    OsRWMutex mRWMutex;

    int mTotalCalls;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _Listener_h_
