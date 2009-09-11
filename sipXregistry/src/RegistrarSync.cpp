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

/// Simple message (only the type is significant) to signal that updates may need to be sent.
class SyncMsg : public OsMsg
{
public:

   typedef enum
   {
      RegistrationChange = OsMsg::USER_START
   } RegistrarMsgType;

   SyncMsg()
      : OsMsg( RegistrationChange, 0 )
      {
      };

   virtual ~SyncMsg()
      {
      };
};

/// constructor
RegistrarSync::RegistrarSync(SipRegistrar& registrar) :
   OsServerTask("RegistrarSync"),
   mRegistrar(registrar)
{
};

/// Signal that there may be updates ready to send
void RegistrarSync::sendUpdates()
{
   SyncMsg syncMsg;

   getMessageQueue()->send(syncMsg);
}

/// Send any updates that we can.
int RegistrarSync::handleMessage(OsMsg& eventMessage)
{
   UtlBoolean handled = FALSE;

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "RegistrarSync::handleMessage started");

   if (SyncMsg::RegistrationChange == eventMessage.getMsgType())
   {
      /*
       * Loop over all peers, pushing a single update to each peer.  Keep going until
       * we get through the loop without pushing to any peer or we are asked to shut down.
       *
       * On some invocations, this loop will do the 'normal' thing and push the one change
       * to each peer, but this is not the only possibility.  The change may already have
       * been sent by a pullUpdates from the peer, or by some earlier call to this routine,
       * or it may fail and need to be sent by a subsequent push attempt.
       *
       * Failure to push to a peer marks that peer as UnReachable, which prevents this
       * loop from further attempts to that peer, and also triggers the RegistrarTest
       * task to begin polling for when it comes back.
       */
      bool pushedUpdate = true;
      while (pushedUpdate && !isShuttingDown())
      {
         pushedUpdate = false;

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
                  OsSysLog::add(FAC_SIP, PRI_DEBUG, "RegistrarSync::handleMessage "
                                "attempting to push an update to peer '%s'",
                                peer->name()
                                );
                  // :LATER: move updating of PeerSentDbUpdateNumber out of
                  // SyncRpcPushUpdates::invoke and into RegistrarSync?
                  SyncRpcPushUpdates::invoke(peer, mRegistrar.primaryName(), &bindings);
                  if (peer->isReachable() /* will not be true if the pushUpdate failed */ )
                  {
                     pushedUpdate = true;
                  }
                  bindings.destroyAll();
               }
            }
         }
      }
      handled = TRUE;
   }
   else
   {
      // let the base class handle any other event - should be just the shutdown request.
   }

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "RegistrarSync::handleMessage finished");

   return handled;
}


SipRegistrarServer& RegistrarSync::getRegistrarServer()
{
   return mRegistrar.getRegistrarServer();
}


/// destructor
RegistrarSync::~RegistrarSync()
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "RegistrarSync::~ waiting for task completion");
   waitUntilShutDown();
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "RegistrarSync::~ task complete");
};
