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
#include "utl/UtlContainableAtomic.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS

class OsConnectionSocket;
class OsNotification;
class OsServerSocket;
class OsServerTask;
class OsSocket;

class SipServerBroker : public OsTask, public UtlContainableAtomic
{

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:


/* ============================ CREATORS ================================== */

   SipServerBroker(OsServerTask* pTask, OsServerSocket* pSocket);
   //:Constructor - takes a server task which created this broker,
   //   and also a socket to listen on
   // Takes ownership of *pSocket.

   virtual
   ~SipServerBroker();
     //:Destructor

/* ============================ MANIPULATORS ============================== */
    virtual int run(void* pArg);


/* ============================ ACCESSORS ================================= */

    virtual UtlBoolean isOk() const;

/* ============================ INQUIRY =================================== */

    virtual UtlContainableType getContainableType(void) const
    {
       return SipServerBroker::TYPE;
    };

    /** Class type used for runtime checking */
    static const UtlContainableType TYPE;

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
