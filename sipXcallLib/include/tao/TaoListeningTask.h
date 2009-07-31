//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _TaoListeningTask_h_
#define _TaoListeningTask_h_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsTask.h"
#include <os/OsServerSocket.h>
#include <os/OsServerTask.h>
#include <os/OsLockingList.h>
#include <os/OsRWMutex.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
class OsServerTask;
class OsServerSocket;
class TaoTransportAgent;

class TaoListeningTask : public OsTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
        TaoListeningTask(OsServerTask*  pServer, OsServerSocket* pListenSocket);

        TaoListeningTask(const TaoListeningTask& rTaoListeningTask);
     //:Copy constructor (not implemented for this class)

        virtual ~TaoListeningTask();
     //:Destructor
     // As part of destroying the task, flush all messages from the incoming
     // OsMsgQ.

/* ============================ MANIPULATORS ============================== */

        UtlBoolean startListener();

        //void addEventConsumer(OsServerTask* messageEventListener);
        //void removeEventConsumer(OsServerTask* messageEventListener);

        void shutdownListeners();

        void shutdownAgent(TaoTransportAgent* pAgent);

        virtual int run(void* pArg);
     //:The entry point for the task.
     // This method executes a message processing loop until either
     // requestShutdown(), deleteForce(), or the destructor for this object
     // is called.

//      virtual void requestShutdown(void);
     //:Call OsTask::requestShutdown() and then post an OS_SHUTDOWN message
     //: to the incoming message queue to unblock the task.


        virtual OsStatus setErrno(int errno);
         //:Set the errno status for the task
         // This call has no effect under Windows NT and, if the task has been
         // started, will always returns OS_SUCCESS


        int getAgentCount();


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

//      TaoTransportAgent* getAgent(const char* hostAddress,
//                                                int hostPort,
//                                                const char* callId,
//                                                const char* toField,
//                                                const char* fromField);

        void deleteAgent(TaoTransportAgent* pAgent);

private:
        OsLockingList   agentList;
        OsRWMutex               agentLock;
        OsServerTask*   mpServer;
        UtlString       mRemoteHost;            // remote TaoServer
        int                     mRemotePort;            // TaoServer's listener port
        int                     mListenerPort;          // this transport's listener port

        OsServerSocket* mpListenSocket;

   TaoListeningTask& operator=(const TaoListeningTask& rhs);
     //:disable Assignment operator


};

#endif // _TaoListeningTask_h_
