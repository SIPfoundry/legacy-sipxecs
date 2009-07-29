//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#ifndef _CallObject_h_
#define _CallObject_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <cp/CallManager.h>
#include <mp/MpStreamPlaylistPlayer.h>

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
class CallObject
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   CallObject(CallManager* callManager, UtlString callId, UtlString playFile);
   ~CallObject();

   OsStatus playAudio();

   void cleanUp();

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    CallManager* mpCallManager;
    UtlString mCallId;

    MpStreamPlaylistPlayer* mpPlayer;
    UtlString mFile;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _CallObject_h_
