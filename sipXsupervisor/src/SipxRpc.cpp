//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsSocket.h"
#include "os/OsDateTime.h"
#include "utl/UtlSListIterator.h"
#include "net/XmlRpcDispatch.h"
#include "SipxRpc.h"
#include "ProcMgmtRpc.h"
#include "AlarmRpc.h"
#include "ImdbRpc.h"
#include "FileRpc.h"
#include "SwAdminRpc.h"
#include "ZoneAdminRpc.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipxRpc::SipxRpc(XmlRpcDispatch* dispatcher, UtlSList& allowedPeers)
  : mpXmlRpcDispatch(dispatcher)
{
   // Take ownership of the UtlString object memory.
   UtlContainable* peer;
   while((peer = allowedPeers.get()))
   {
      mAllowedPeers.insert(peer);
   }

   // Make sure the localhost is among the allowed peers.
   UtlString myName;
   OsSocket::getHostName(&myName);
   if (!mAllowedPeers.contains(&myName))
   {
      mAllowedPeers.insert(new UtlString(myName));
   }

   // Register the XML-RPC methods.
   ProcMgmtRpcGetStateAll::registerSelf(*this);
   ProcMgmtRpcGetStatusMessage::registerSelf(*this);
   ProcMgmtRpcStart::registerSelf(*this);
   ProcMgmtRpcStop::registerSelf(*this);
   ProcMgmtRpcRestart::registerSelf(*this);
   ProcMgmtRpcGetConfigVersion::registerSelf(*this);
   ProcMgmtRpcSetConfigVersion::registerSelf(*this);
   ProcMgmtRpcRunConfigtest::registerSelf(*this);
   ProcMgmtRpcGetConfigtestMessages::registerSelf(*this);

   ImdbRpcReplaceTable::registerSelf(*this);
   ImdbRpcRetrieveTable::registerSelf(*this);
   ImdbRpcAddTableRecords::registerSelf(*this);
   ImdbRpcDeleteTableRecords::registerSelf(*this);

   FileRpcReplaceFile::registerSelf(*this);

   AlarmRpcGetAlarmCount::registerSelf(*this);
   AlarmRpcRaiseAlarm::registerSelf(*this);
   AlarmRpcReloadAlarms::registerSelf(*this);
   SwAdminRpcExec::registerSelf(*this);
   SwAdminRpcSnapshot::registerSelf(*this);
   SwAdminRpcExecStatus::registerSelf(*this);
   ZoneAdminRpcExec::registerSelf(*this);
}

// Destructor
SipxRpc::~SipxRpc()
{
   // the mpXmlRpcDispatch pointer is owned by sipXsupervisor main
   mAllowedPeers.destroyAll();

   if ( mpXmlRpcDispatch != NULL )
   {
      mpXmlRpcDispatch->removeAllMethods();
   }
}

/* ============================ ACCESSORS ================================= */

XmlRpcDispatch* SipxRpc::getXmlRpcDispatch()
{
   return mpXmlRpcDispatch;
}

bool SipxRpc::isAllowedPeer(const UtlString& peer) const
{
   return mAllowedPeers.contains(&peer);
}

/// Whether or not an HTTP request is from some allowed peer, and if so which one.
bool SipxRpc::isAllowedPeer(const HttpRequestContext& context, ///< the request to be checked
                            UtlString& peer                    ///< if allowed, the name of the peer
                            ) const
{
   bool isAllowed = false;
   peer.remove(0);

   UtlSListIterator allowedPeers(mAllowedPeers);
   UtlString* tryPeer;
   while (!isAllowed && (tryPeer = dynamic_cast<UtlString*>(allowedPeers())))
   {
      isAllowed = context.isTrustedPeer(*tryPeer);
   }
   if (isAllowed)
   {
      peer = *tryPeer;
   }
   return isAllowed;
}

