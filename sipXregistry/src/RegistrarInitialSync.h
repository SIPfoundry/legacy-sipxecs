//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _REGISTERINITIALSYNC_H_
#define _REGISTERINITIALSYNC_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsTask.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipRegistrar;

/**
 * RegistrarInitialSync is an OsTask that implements the startup
 * phase of operation: recover the local registration
 * database (if possible), and resynchronize with all peer
 * registrars.  Any updates are pulled from peer registrars so that the
 * local registrar can tell when its database is up to date and that no
 * more updates are available.  During the startup phase the database is
 * not yet known to be up to date, so the Registry/Redirect service does
 * not accept either any SIP request or any request to push updates from
 * any peer registrar.
 */
class RegistrarInitialSync : public OsTask
{
public:
   /// Create the startup phase thread.
   RegistrarInitialSync(SipRegistrar& registrar);

   virtual int run(void* pArg);

   /// Wait until the startup phase has completed
   void waitForCompletion();

   /// destructor
   virtual ~RegistrarInitialSync();

protected:
   friend class SipRegistrar;

   /// Recover the latest received update number for each peer from the local database
   void restorePeerUpdateNumbers();

   /// Get from peers any of our own updates that we have lost
   void pullLocalUpdatesFromPeers();

   /// Get from peers any peer updates that we missed or lost while down
   void pullPeerUpdatesFromPeers();

   /// Get any updates for unreachable peers from reachable ones.
   void recoverUnReachablePeers();

   /// Return the name of this registrar
   const char* getPrimaryName();

   SipRegistrarServer& getRegistrarServer();

   /// Apply registry updates to the database
   void applyUpdatesToDirectory(UtlSList& bindings);

private:
   SipRegistrar&   mRegistrar;
   OsBSem          mFinished;

   /// There is no copy constructor.
   RegistrarInitialSync(const RegistrarInitialSync&);

   /// There is no assignment operator.
   RegistrarInitialSync& operator=(const RegistrarInitialSync&);
};

#endif // _REGISTERINITIALSYNC_H_
