//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifdef TEST
#include "utl/UtlMemCheck.h"
#endif

#include <assert.h>
#include "os/OsServerSocket.h"
#include "os/OsServerTask.h"
#include "tao/TaoMessage.h"
#include "tao/TaoListeningTask.h"
#include "tao/TaoTransportAgent.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TaoListeningTask::TaoListeningTask(OsServerTask* pServer, OsServerSocket* pListenSocket)
        : OsTask("TaoLstngTask-%d"),
        agentLock(OsMutex::Q_FIFO),
        mpServer(pServer),
        mpListenSocket(pListenSocket)
{
        osPrintf("---- TaoListeningTask::construct socket %p\n", mpListenSocket);
}

TaoListeningTask::~TaoListeningTask()
{
        int iteratorHandle = agentList.getIteratorHandle();
        TaoTransportAgent* pAgent = NULL;;
        while ((pAgent = (TaoTransportAgent*)agentList.next(iteratorHandle)))
        {
                agentList.remove(iteratorHandle);
                delete pAgent;
        }
        agentList.releaseIteratorHandle(iteratorHandle);
}

// The entry point for the task.
// This method executes a message processing loop until either
// requestShutdown(), deleteForce(), or the destructor for this object
// is called.
int TaoListeningTask::run(void* pArg)
{
        OsConnectionSocket* pClientSocket = NULL;
        TaoTransportAgent* pAgent = NULL;

    if(!mpListenSocket->isOk())
    {
                printf("!! ERROR TaoListeningTask::run: invalid server socket !!\n");
    }

        while (!isShuttingDown() && mpListenSocket->isOk())
        {
                pClientSocket = mpListenSocket->accept();
                if(pClientSocket)
                {
                        pAgent = new TaoTransportAgent(pClientSocket, mpServer);

                        UtlBoolean agentStarted = pAgent->start();
                        if(!agentStarted)
                        {
                                osPrintf("----- TaoTransportAgent failed to start");
                        }
                        agentList.push(pAgent);
                }
        }

        osPrintf("++++ TaoListeningTask::run shutting down.\n");
        return 0;        // and then exit
}

UtlBoolean TaoListeningTask::startListener()
{
   start();

        // For each client start listening
        int iteratorHandle = agentList.getIteratorHandle();
        TaoTransportAgent* pAgent = NULL;
        while ((pAgent = (TaoTransportAgent*)agentList.next(iteratorHandle)))
        {
                pAgent->start();
        }
        agentList.releaseIteratorHandle(iteratorHandle);
        return(TRUE);
}

void TaoListeningTask::shutdownListeners()
{
        requestShutdown();

        // For each client request shutdown
        int iteratorHandle = agentList.getIteratorHandle();
        TaoTransportAgent* pAgent = NULL;
        while ((pAgent = (TaoTransportAgent*)agentList.next(iteratorHandle)))
        {
                pAgent->requestShutdown();
        }
        agentList.releaseIteratorHandle(iteratorHandle);
}

void TaoListeningTask::shutdownAgent(TaoTransportAgent* pAgent)
{
        // Find the client in the list of clients and shut it down
        int iteratorHandle = agentList.getIteratorHandle();
        TaoTransportAgent* agent = NULL;
        UtlString agentName;

        osPrintf("-***- TaoListeningTask::shutdownAgent(%p)\r\n", (void*)pAgent);
        while ((agent = (TaoTransportAgent*)agentList.next(iteratorHandle)))
        {
                if(agent == pAgent)
                {
                        agentList.remove(iteratorHandle);
                        delete pAgent;
                        break;
                }
        }
        agentList.releaseIteratorHandle(iteratorHandle);
}

void TaoListeningTask::deleteAgent(TaoTransportAgent* pAgent)
{
        // Find the client in the list of clients and shut it down
        int iteratorHandle = agentList.getIteratorHandle();
        TaoTransportAgent* agent = NULL;
        UtlString agentName;

#ifdef TEST
        osPrintf("TaoListeningTask::deleteAgent(%p)\r\n", (void*)pAgent);
#endif
        while ((agent = (TaoTransportAgent*)agentList.next(iteratorHandle)))
        {
                // Remove this or any other bad client
                if(agent == pAgent || !agent->isOk())
                {
                        agent->getAgentName(&agentName);
#ifdef TEST
                        osPrintf("Removing TaoTransportAgent: %s\r\n", agentName.data());
#endif
                        agentList.remove(iteratorHandle);
                        delete agent;
                        // break;
                }
        }
        agentList.releaseIteratorHandle(iteratorHandle);
}

// Assignment operator
TaoListeningTask&
TaoListeningTask::operator=(const TaoListeningTask& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

        mpServer = rhs.mpServer;
        mpListenSocket = rhs.mpListenSocket;

        return *this;
}

/* ============================ ACCESSORS ================================= */
int TaoListeningTask::getAgentCount()
{
        return(agentList.getCount());
}

// Set the errno status for the task.
// This call has no effect under Windows NT and, if the task has been
// started, will always returns OS_SUCCESS
OsStatus TaoListeningTask::setErrno(int errno)
{
   if (!isStarted())
      return OS_TASK_NOT_STARTED;

   return OS_SUCCESS;
}
