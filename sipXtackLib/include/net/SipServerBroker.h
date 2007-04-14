// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _INCLUDED_SipServerBroker_H /* [ */
#define _INCLUDED_SipServerBroker_H

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsTask.h"
#include "os/OsLock.h"
#include "os/OsSocket.h"
#include "os/OsRWMutex.h"
#include "os/OsProtectEvent.h"

class OsNotification;
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS

class OsConnectionSocket;
class OsServerSocket;
class OsSocket;



class SipServerBroker : public OsTask
{

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:


/* ============================ CREATORS ================================== */

   SipServerBroker(OsServerTask* pTask, OsServerSocket* pSocket);
   //:Constructor - takes a server task which created this broker,
   //   and also a socket to listen on
   
   virtual
   ~SipServerBroker();
     //:Destructor

/* ============================ MANIPULATORS ============================== */
    virtual int run(void* pArg);


/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    OsServerSocket* mpSocket;
    OsServerTask* const mpOwnerTask;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   SipServerBroker(const SipServerBroker& rSipServerBroker);
     //:Copy constructor (not implemented for this task)

   SipServerBroker& operator=(const SipServerBroker& rhs);
     //:Assignment operator (not implemented for this task)
};

#endif /* _INCLUDED_SipServerBrokerH ] */
