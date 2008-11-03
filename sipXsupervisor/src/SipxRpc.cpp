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
#include "net/XmlRpcDispatch.h"
#include "SipxRpc.h"
#include "ProcMgmtRpc.h"
#include "AlarmRpc.h"
#include "ImdbRpc.h"
#include "FileRpc.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipxRpc::SipxRpc( const int port, UtlSList& allowedPeers) :
mXmlRpcPort(port),
mpXmlRpcDispatch(NULL)
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
}

// Destructor
SipxRpc::~SipxRpc()
{
   if ( mpXmlRpcDispatch )
   {
      delete mpXmlRpcDispatch;
      mpXmlRpcDispatch = NULL;
   }

   mAllowedPeers.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipxRpc& 
SipxRpc::operator=(const SipxRpc& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}
   

void SipxRpc::startRpcServer()
{
   // Begin operation of the XML-RPC service.
   mpXmlRpcDispatch = new XmlRpcDispatch(mXmlRpcPort, true /* use https */);

   // Register the XML-RPC methods.
   ProcMgmtRpcGetStateAll::registerSelf(*this);
   ProcMgmtRpcStart::registerSelf(*this);
   ProcMgmtRpcStop::registerSelf(*this);
   ProcMgmtRpcRestart::registerSelf(*this);
   ProcMgmtRpcGetConfigVersion::registerSelf(*this);
   ProcMgmtRpcSetConfigVersion::registerSelf(*this);

   ImdbRpcReplaceTable::registerSelf(*this);
   ImdbRpcRetrieveTable::registerSelf(*this);
   ImdbRpcAddTableRecords::registerSelf(*this);
   ImdbRpcDeleteTableRecords::registerSelf(*this);

   FileRpcReplaceFile::registerSelf(*this);

   AlarmRpcGetAlarmCount::registerSelf(*this);
   AlarmRpcRaiseAlarm::registerSelf(*this);
   AlarmRpcReloadAlarms::registerSelf(*this);
   
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


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

