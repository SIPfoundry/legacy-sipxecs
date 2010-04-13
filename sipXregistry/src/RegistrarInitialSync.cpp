//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <memory>
using std::auto_ptr;

// APPLICATION INCLUDES
#include "sipdb/RegistrationBinding.h"
#include "os/OsDateTime.h"
#include "utl/UtlSListIterator.h"
#include "registry/SipRegistrar.h"
#include "SipRegistrarServer.h"
#include "RegistrarInitialSync.h"
#include "SyncRpc.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Create the startup phase thread.
RegistrarInitialSync::RegistrarInitialSync(SipRegistrar& registrar)
   : OsTask("RegistrarInitSync-%d"),
     mRegistrar(registrar),
     mFinished(OsBSem::Q_PRIORITY, OsBSem::EMPTY)
{
};

int RegistrarInitialSync::run(void* pArg)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "RegistrarInitialSync started");

   // This function performs steps (1) through (4) in section 5.7.1 of
   // sipXregistry/doc/SyncDesign.*.

   // get the max update number for local updates from the local database
   // Step (1)(bullet 1).
   getRegistrarServer().restoreDbUpdateNumber();

   // get the received update numbers for each peer from the local database
   // Step (1)(bullet 2).
   restorePeerUpdateNumbers();

   // having done that, we can begin accepting pull requests from peers
   // Step (2).
   SyncRpcPullUpdates::registerSelf(mRegistrar);

   // Get from peers any of our own updates that we have lost
   // Step (3)(1).
   pullLocalUpdatesFromPeers();

   // Get from peers any peer updates that we missed or lost while down
   // Step (3)(2).
   pullPeerUpdatesFromPeers();

   // Get any updates for unreachable peers from reachable ones
   // Step (4).
   recoverUnReachablePeers();

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "RegistrarInitialSync complete");

   // allow SipRegistrar to proceed to operational phase
   mFinished.release();
   return 0; // exit thread
}

/// Recover the latest received update number for each peer from the local database
void RegistrarInitialSync::restorePeerUpdateNumbers()
{
   auto_ptr<UtlSListIterator> peers(mRegistrar.getPeers());
   RegistrarPeer* peer;
   while ((peer = static_cast<RegistrarPeer*>((*peers)())))
   {
      const char* name = peer->name();
      assert(name);

      // Set the last received update number for the peer to the max update number
      // for the peer that we see in the registration DB
      Int64 maxUpdateNumber = getRegistrarServer().getMaxUpdateNumberForRegistrar(name);
      peer->setReceivedFrom(maxUpdateNumber);
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "RegistrarInitialSync::restorePeerUpdateNumbers "
                    "for peer '%s' last received update # = %0#16" FORMAT_INTLL "x",
                    name, maxUpdateNumber);
   }
}


/// Get from peers any of our own updates that we have lost
void RegistrarInitialSync::pullLocalUpdatesFromPeers()
{
   auto_ptr<UtlSListIterator> peers(mRegistrar.getPeers());
   RegistrarPeer* peer;
   while (   (peer = static_cast<RegistrarPeer*>((*peers)()))
          && !isShuttingDown())
   {
      RegistrarPeer::SynchronizationState state = peer->synchronizationState();

      // Only RegistrarTest can declare a peer Reachable, via a reset, that happens later
      assert(state != RegistrarPeer::Reachable);

      if (state == RegistrarPeer::Uninitialized)
      {
         // Call pullUpdates, passing the local registrar host name and DbUpdateNumber.
         // The purpose of this call is to recover any registrations for which the local
         // host was the primary but which for some reason were not saved in the local
         // persistent store (the canonical case is that the local file was lost or
         // corrupted - when this is the case, the local DbUpdateNumber will usually be zero).
         // If we can't reach the peer, then the invoke method marks it UnReachable.

         // Pulling updates changes maxUpdateNumber, so fetch it on each iteration
         const char* primaryName = getPrimaryName();
         Int64 maxUpdateNumber = getRegistrarServer().getDbUpdateNumber();
         UtlSList bindings;
         state = SyncRpcPullUpdates::invoke(
            peer,            // the peer we're contacting
            primaryName,     // name of the calling registrar (this one)
            primaryName,     // name of the registrar whose updates we're pulling (this one)
            maxUpdateNumber, // pull all updates more recent than this
            &bindings);      // return bindings in this list
         assert(state != RegistrarPeer::Reachable);

         // Apply the resulting updates to the DB, if pullUpdates succeeded.
         // A return value of Uninitialized indicates success.  If pullUpdates fails
         // then the peer's state is downgraded to UnReachable or Incompatible.
         if (   state == RegistrarPeer::Uninitialized
             && bindings.entries() > 0)
         {
            applyUpdatesToDirectory(bindings);
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "RegistrarInitialSync::pullLocalUpdatesFromPeers "
                          "received %zu local updates from peer '%s'",
                          bindings.entries(), peer->name());
         }
         else
         {
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "RegistrarInitialSync::pullPeerUpdatesFromPeers "
                          "'%s' is %s",
                          peer->name(), peer->getStateName());
         }

         bindings.destroyAll();
      }
   }
}


/// Get from peers any peer updates that we missed or lost while down
void RegistrarInitialSync::pullPeerUpdatesFromPeers()
{
   auto_ptr<UtlSListIterator> peers(mRegistrar.getPeers());
   RegistrarPeer* peer;
   const char* primaryName = getPrimaryName();
   while (   (peer = static_cast<RegistrarPeer*>((*peers)()))
          && !isShuttingDown())
   {
      RegistrarPeer::SynchronizationState state = peer->synchronizationState();
      assert(state != RegistrarPeer::Reachable);
      if (state == RegistrarPeer::Uninitialized)
      {
         const char* peerName = peer->name();
         assert(peerName);
         UtlSList bindings;
         state = SyncRpcPullUpdates::invoke(
            peer, primaryName, peerName, peer->receivedFrom(), &bindings);
         assert(state != RegistrarPeer::Reachable);

         // Apply the resulting updates to the DB
         if (   state == RegistrarPeer::Uninitialized
             && bindings.entries() > 0
             )
         {
            applyUpdatesToDirectory(bindings);
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "RegistrarInitialSync::pullPeerUpdatesFromPeers "
                          "received %zu peer updates from peer '%s'",
                          bindings.entries(), peer->name());
         }
         else
         {
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "RegistrarInitialSync::pullPeerUpdatesFromPeers "
                          "'%s' is %s",
                          peer->name(), peer->getStateName());
         }
      }
   }
}


/// Get any updates for unreachable peers from reachable ones.
void RegistrarInitialSync::recoverUnReachablePeers()
{
   // In the startup phase no peers are formally in the Reachable state, instead we
   // look for peers who are reachable but formally in the Uninitialized state.
   // :TODO: The state machine is confusing because the Reachable state doesn't
   // match the practical notion of reachability.  Simplify.
   // loop over reachable peers
   auto_ptr<UtlSListIterator> peers(mRegistrar.getPeers());
   RegistrarPeer* reachablePeer;
   const char* primaryName = getPrimaryName();
   while (   (reachablePeer = static_cast<RegistrarPeer*>((*peers)()))
          && !isShuttingDown())
   {
      if (reachablePeer->synchronizationState() == RegistrarPeer::Uninitialized)
      {
         // loop over unreachable peers
         auto_ptr<UtlSListIterator> peers2(mRegistrar.getPeers());
         RegistrarPeer* unreachablePeer;
         while (   (unreachablePeer = static_cast<RegistrarPeer*>((*peers)()))
                && !isShuttingDown())
         {
            if (unreachablePeer->synchronizationState() == RegistrarPeer::UnReachable)
            {
               UtlSList bindings;
               RegistrarPeer::SynchronizationState state =
                  SyncRpcPullUpdates::invoke(
                     reachablePeer,                      // the peer we are calling
                     primaryName,                        // the calling registrar (us)
                     unreachablePeer->name(),            // peer whose updates we want
                     unreachablePeer->receivedFrom(),    // want updates above this number
                     &bindings);                         // put the bindings here
               assert(state != RegistrarPeer::Reachable);

               // Apply the resulting updates to the DB
               if (   state == RegistrarPeer::Uninitialized
                   && bindings.entries() > 0
                   )
               {
                  applyUpdatesToDirectory(bindings);
                  OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                "RegistrarInitialSync::recoverUnReachablePeers "
                                "received %zu peer updates from peer '%s' for peer '%s'",
                                bindings.entries(),
                                reachablePeer->name(),
                                unreachablePeer->name());
               }
               else
               {
                  OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                "RegistrarInitialSync::recoverUnReachablePeers "
                                "'%s' is %s",
                                reachablePeer->name(), reachablePeer->getStateName());
               }
            }
         }
      }
   }
}


void RegistrarInitialSync::waitForCompletion()
{
   mFinished.acquire();
}


const char* RegistrarInitialSync::getPrimaryName()
{
   const char* primaryName = mRegistrar.primaryName();
   assert(primaryName);
   return primaryName;
}


SipRegistrarServer& RegistrarInitialSync::getRegistrarServer()
{
   return mRegistrar.getRegistrarServer();
}


void RegistrarInitialSync::applyUpdatesToDirectory(UtlSList& bindings)
{
   getRegistrarServer().applyUpdatesToDirectory(bindings);
}


/// destructor
RegistrarInitialSync::~RegistrarInitialSync()
{
};
