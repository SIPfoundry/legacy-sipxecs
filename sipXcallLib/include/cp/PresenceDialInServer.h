//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _PresenceDialInServer_h_
#define _PresenceDialInServer_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsBSem.h>
#include <os/OsConfigDb.h>
#include <net/StateChangeNotifier.h>
#include <tao/TaoAdaptor.h>
#include <utl/UtlHashMap.h>

// DEFINES
#define DEFAULT_SIGNIN_FEATURE_CODE   "*88"
#define DEFAULT_SIGNOUT_FEATURE_CODE  "*86"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class CallManager;
class TaoString;

/**
 * A PresenceDialInServer is an object that allows an extension to sign in and
 * sign off to a ACD queue by simply using a feature code.
 *
 */

class PresenceDialInServer: public TaoAdaptor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */

   /// Constructor
   PresenceDialInServer(CallManager* callMgr, OsConfigDb* configFile);

   /// Destructor
   virtual ~PresenceDialInServer();

/* ============================ MANIPULATORS ============================== */

   virtual UtlBoolean handleMessage(OsMsg& eventMessage);

/* ============================ ACCESSORS ================================= */

   /// Register a StateChangeNotifier
   void addStateChangeNotifier(const char* fileUrl, StateChangeNotifier* notifier);

   /// Unregister a StateChangeNotifier
   void removeStateChangeNotifier(const char* fileUrl);

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   void dumpTaoMessageArgs(unsigned char eventId, TaoString& args);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   void parseConfig(UtlString& configFile);

   // Returns TRUE if the requested state is different from the current state.
   bool notifyStateChange(UtlString& contact, bool signIn);

   CallManager* mpCallManager;
   OsBSem mLock;

   UtlString mSignInFC;       /// Sign in feature code
   UtlString mSignOutFC;      /// Sign out feature code

   UtlString mSignInConfirmationAudio;
   UtlString mSignOutConfirmationAudio;
   UtlString mErrorAudio;

   static const char *  confirmationTone;       // Confirmation Tone audio data
   static unsigned long confirmationToneLength; // and length.  See: ConfirmationTone.h

   static const char *  dialTone;       // Busy Tone audio data
   static unsigned long dialToneLength; // and length.  See: BusyTone.h

   static const char *  busyTone;       // Dial Tone audio data
   static unsigned long busyToneLength; // and length.  See: DialTone.h

   OsMsgQ* mpIncomingQ;

   UtlHashMap mCalls;
   UtlHashMap mStateChangeNotifiers;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _PresenceDialInServer_h_
