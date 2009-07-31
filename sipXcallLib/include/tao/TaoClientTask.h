//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _TaoClient_h_
#define _TaoClient_h_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsServerTask.h"
#include "os/OsBSem.h"
#include "os/OsLock.h"
#include "tao/TaoMessage.h"
#include "tao/TaoServerTask.h"
#include "tao/TaoTransportAgent.h"
#include "TaoDefs.h"    // Added by ClassView

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class TaoListenerClientTask;
class TaoTransportTask;
class OsConnectionSocket;
class PtEventListener;
class PtConnectionListener;
class PtTerminalComponentListener;
class PtTerminalConnectionListener;
class PtCallListener;
class PtTerminalListener;

//:Used to build the call originating part, establishes connection with the server
// through the TaoTransport. Maintains a db of listeners the client has registered.
class TaoClientTask : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
        TaoClientTask(int remotePort,
                                  UtlString remoteHost,
                                  TaoServerTask *pTaoServerTask = NULL,
                                  const UtlString& name="TaoClientTask-%d",
                                  const int maxRequestQMsgs=DEF_MAX_MSGS);
         //:Constructor

        TaoClientTask(TaoServerTask *pTaoServerTask = NULL,
                                const UtlString& name="TaoClientTask-%d",
                                void* pArg=NULL,
                                const int maxRequestQMsgs=DEF_MAX_MSGS,
                                const int priority=DEF_PRIO,
                                const int options=DEF_OPTIONS,
                                const int stackSize=DEF_STACKSIZE);
        //:Constructor

        TaoClientTask(const int maxIncomingQMsgs, TaoServerTask *pTaoServerTask = NULL);
        //:Constructor

        TaoClientTask(const TaoClientTask& rTaoClientTask);
     //:Copy constructor (not implemented for this class)

        virtual ~TaoClientTask();
/* ============================ MANIPULATORS ============================== */

    virtual void requestShutdown(void);

        int sendRequest(TaoMessage& rMsg, OsMutex* pMutex = 0, const OsTime& rTimeout=OsTime::OS_INFINITY);

        virtual UtlBoolean handleMessage(OsMsg& rMsg);
         //:Handle an incoming message.
         // If the message is not one that the object is prepared to process,
         // the handleMessage() method in the derived class should return FALSE
         // which will cause the OsServerTask::handleMessage() method to be
         // invoked on the message.

        void addEventListener(PtEventListener* pListener, const char* callId = NULL);

        void removeEventListener(PtEventListener& rListener);

        int resetConnectionSocket(int transactionId);

/* ============================ ACCESSORS ================================ */

        TaoObjHandle getEventServer() { return (TaoObjHandle) mpListenerTask; };

/* //////////////////////////// PRIVATE ////////////////////////////////// */
private:
friend class PtTerminal;

/* ============================ FUNCTIONS ================================ */
        TaoStatus initInstance();

        UtlBoolean receiveMsg(TaoMessage& rMsg);

        int readUntilDone(OsConnectionSocket* pSocket, char *pBuf, int iLength) ;
        //: read iLength bytes from passed socket (waiting until completion)

/* ============================ VARIABLES ================================ */
        TaoListenerClientTask *mpListenerTask;

        TaoTransportTask*       mpTransport;
        OsConnectionSocket*     mpConnectionSocket;

        int                             mRemotePort;            // TaoServer's listener port
        UtlString               mRemoteHost;            // remote TaoServer
        OsRWMutex               mMutex;                         // mutex for synchonizing access to data

        TaoServerTask*  mpTaoServerTask;

        TaoTransportAgent* mpAgent;

private:


};

#endif // _TaoClient_h_
