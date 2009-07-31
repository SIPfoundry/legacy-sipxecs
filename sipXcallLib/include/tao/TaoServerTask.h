//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _TaoServer_h_
#define _TaoServer_h_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMsgQ.h"
#include "os/OsServerTask.h"
#include "os/OsTime.h"
#include "tao/TaoObject.h"
#include "tao/TaoMessage.h"
#include "tao/TaoObjectMap.h"
#include "tao/TaoReference.h"
#include "tao/TaoListenerManager.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class TaoTransportTask;
class PtProvider;
class PtCall;
class CpCallManager;
class PsPhoneTask;

//:The server is the place where all requests from clients are received and
// processed accordingly. It contains a db of addresses, a registry of listeners,
// a registry of clients, a list of invoked PTAPI objects, and an incoming message
// queue.
class TaoServerTask : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */

        static TaoServerTask* getTaoServerTask(CpCallManager *pCallMgr);

        TaoServerTask(const TaoServerTask& rTaoServerTask);
     //:Copy constructor (not implemented for this class)

        virtual ~TaoServerTask();


/* ============================ MANIPULATORS ============================== */

        virtual UtlBoolean handleMessage(OsMsg& rMsg);
         //:Handle an incoming message.
         // If the message is not one that the object is prepared to process,
         // the handleMessage() method in the derived class should return FALSE
         // which will cause the OsServerTask::handleMessage() method to be
         // invoked on the message.

        virtual OsStatus setErrno(int errno);
         //:Set the errno status for the task
         // This call has no effect under Windows NT and, if the task has been
         // started, will always returns OS_SUCCESS

        void setClientHandle(TaoObjHandle hClient);
        //:Set the handle for the client that is at the same location of the TaoServer

        void setEventClient(TaoObjHandle hEventClient);
        //:Set the handle for the client event handler that will receive the event messages from TaoListenerManager

/* ============================ ACCESSORS ================================= */
        TaoListenerManager*     getTaoListenerManager() { return mpListenerMgr; };

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    // Singleton globals
    // Note: this class does not need to be a singleton.  The only method that
    // assumes singleton is getTaoServerTask
    static TaoServerTask* mpInstance;
    static OsBSem sLock;

        TaoServerTask(CpCallManager *pCallMgr,
                                PsPhoneTask *pPhoneTask,
                                const UtlString& name="TaoServerTask-%d",
                                void* pArg=NULL,
                                const int maxRequestQMsgs=DEF_MAX_MSGS,
                                const int priority=DEF_PRIO,
                                const int options=DEF_OPTIONS,
                                const int stackSize=DEF_STACKSIZE);
        //:Constructor

        TaoServerTask(const UtlString& name="TaoServerTask-%d",
                                void* pArg=NULL,
                                const int maxRequestQMsgs=DEF_MAX_MSGS,
                                const int priority=DEF_PRIO,
                                const int options=DEF_OPTIONS,
                                const int stackSize=DEF_STACKSIZE);
        //:Constructor

        TaoServerTask(const int maxIncomingQMsgs);
        //:Constructor

/* ============================ MANIPULATORS ============================== */
        TaoStatus initInstance();

/* ============================ MEMBERS ============================== */
        CpCallManager* mpCallMgr;
        PsPhoneTask*   mpPhoneTask;   // phone set task

        TaoObjHandle    mClientHandle;
        TaoObjHandle    mEventClient;
        TaoTransportTask*       mpSvrTransport;
        TaoListenerManager*     mpListenerMgr;

        TaoObjectMap*   mpConnectionDb;
        TaoObjectMap*   mpTransactionDb;
        TaoObjectMap*   mpListeners;
        TaoObjectMap*   mpClients;
        TaoObjectMap*   mpAdaptors;

        TaoReference*   mpConnectionCnt;
        TaoReference*   mpClientCnt;
        TaoReference*   mpListenerCnt;
        TaoReference*   mpTransactionCnt;
        TaoReference*   mpAdaptorCnt;
        OsMsgQ                  mOutgoingQ;                 // queue for outgoing messages

        PtProvider*             mpProvider;
        PtCall                  *mpCall;


};

#endif // _TaoServer_h_
