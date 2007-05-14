// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

/*
The RegistrarSync thread is responsible for propagating updates to Reachable peer registrars.

The RegistrarSync thread operation is governed by a private static OsBSem (binary semaphore).
The thread main loop waits on that semaphore. The static C++ method RegistrarSync::sendUpdates;
when invoked, increments the semaphore value, indicating to the RegistrarSync thread that there
may be updates available to be propagated, or that connectivity to a previously UnReachable
peer has been restored. On each pass through the loop, the thread does:

   For each Reachable peer, if the localDbUpdateNumber is greater than the
   peerSentDbUpdateNumber, the pushUpdates XML-RPC method is used to push a single update.
   A successful return in turn updates the peerSentDbUpdateNumber.
   If any fault is returned by pushUpdate, the peer is marked UnReachable, which triggers
   the RegistrarTest (RegistrarTest) thread to begin attempting to reestablish contact.

   After completing one pass over the Reachable peers, if DbUpdateNumber is less than the
   lowest peerSentDbUpdateNumber for all Reachable peers (indicating that there remains at
   least one update to be propagated), the RegistrarSync thread calls sendUpdates itself.
*/

// SYSTEM INCLUDES
#include <memory>
using std::auto_ptr;

// APPLICATION INCLUDES
#include "utl/UtlSListIterator.h"
#include "RegistrarPeer.h"
#include "RegistrarSync.h"
#include "registry/SipRegistrar.h"
#include "SipRegistrarServer.h"
#include "SyncRpc.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// constructor
RegistrarSync::RegistrarSync(SipRegistrar& registrar) :
   mRegistrar(registrar),
   mMutex(OsBSem::Q_PRIORITY, OsBSem::EMPTY)
{
};

/// Signal that there may be updates ready to send
void RegistrarSync::sendUpdates()
{
   mMutex.release();
}


/// Task main loop
int RegistrarSync::run(void* pArg)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "RegistrarSync started");

   while (!isShuttingDown())
   {
      // Wait until there is work to do - this is signalled by the sendUpdates method
      mMutex.acquire();
      
      bool pushedUpdate = true;
      while (pushedUpdate && !isShuttingDown())
      {
         pushedUpdate = false;
         // Loop over all peers, pushing a single update for each peer.  Keep going until
         // there are no updates left to push to any peer.
         // There is a benign race condition here: a local registration might happen just
         // after we have checked the last peer and decided that there is nothing to do.
         // That's OK because the mutex will have been signalled -- we'll acquire it again
         // immediately and come right back to the loop.

         // For each Reachable peer, if the local DbUpdateNumber is greater than the
         // PeerSentDbUpdateNumber, then push a single update.
         auto_ptr<UtlSListIterator> peers(mRegistrar.getPeers());
         RegistrarPeer* peer;
         while (   (peer = static_cast<RegistrarPeer*>((*peers)()))
                && !isShuttingDown()
                )
         {
            if (peer->isReachable())
            {
               UtlSList bindings;
               bool isUpdateToSend = getRegistrarServer().getNextUpdateToSend(peer, bindings);
               if (isUpdateToSend)
               {
                  // :LATER: move updating of PeerSentDbUpdateNumber out of
                  // SyncRpcPushUpdates::invoke and into RegistrarSync?
                  SyncRpcPushUpdates::invoke(peer, mRegistrar.primaryName(), &bindings);
                  pushedUpdate = true;
                  bindings.destroyAll();
               }
            }
         }
      }
   }
    
   return 0;
}


SipRegistrarServer& RegistrarSync::getRegistrarServer()
{
   return mRegistrar.getRegistrarServer();
}


/// destructor
RegistrarSync::~RegistrarSync()
{
};
