//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "os/OsDefs.h"
#include <assert.h>
#include <string.h>
#include "os/OsTask.h"
#include "os/OsPtrMsg.h"

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsTask.h"
#include "os/OsServerTask.h"
#include "os/OsSysLog.h"
#include "os/OsConnectionSocket.h"
#include "os/OsEvent.h"
#include "os/OsNotification.h"
#include "net/SipServerBroker.h"
#include "os/OsServerSocket.h"
#include "os/OsEventMsg.h"
#include "net/SipTcpServer.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

const UtlContainableType SipServerBroker::TYPE = "SipServerBroker";

SipServerBroker::SipServerBroker(OsServerTask* pTask, OsServerSocket* pSocket) :
    OsTask("SipServerBroker-%d"),
    mpSocket(pSocket),
    mpOwnerTask(pTask)
{
    start();
}

int SipServerBroker::run(void *pNotUsed)
{
    OsConnectionSocket* clientSocket = NULL;

    while(!isShuttingDown() && mpSocket && mpSocket->isOk())
    {
        clientSocket = mpSocket->accept();

        // post a message, containing the the client socket, to the owner
        // @TODO - what about when we are shutting down?
        if(clientSocket)
        {
            OsPtrMsg ptrMsg(OsMsg::OS_EVENT, SipTcpServer::SIP_SERVER_BROKER_NOTIFY,
                            (void*)clientSocket);
            mpOwnerTask->postMessage(ptrMsg);
        }
    }
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipServerBroker::run '%s' terminating %s OsSocket %p status %s",
                  mName.data(),
                  isShuttingDown() ? "task shutdown" : "socket problem",
                  mpSocket,
                  mpSocket ? ( mpSocket->isOk() ? "ok" : "not ok" ) : "deleted"
                  );
    return 0;
}

// Destructor
SipServerBroker::~SipServerBroker()
{
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipServerBroker::~ OsSocket %p status %s",
                  mpSocket,
                  mpSocket ? ( mpSocket->isOk() ? "ok" : "not ok" ) : "deleted"
                  );

    if (mpSocket)
    {
        mpSocket->close();
    }
    waitUntilShutDown();
    if (mpSocket)
    {
       delete mpSocket;
       mpSocket = NULL;
    }
}

UtlBoolean SipServerBroker::isOk() const
{
    return mpSocket->isOk() ;
}
/************************************************************************/

/* ============================ FUNCTIONS ================================= */
