//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _TaoTransportTask_h_
#define _TaoTransportTask_h_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// SYSTEM INCLUDES

// APPLICATION INCLUDES
//#include <os/OsTask.h>
#include <os/OsServerTask.h>
#include <os/OsServerSocket.h>
#include <os/OsMsgQ.h>
#include "tao/TaoObject.h"
#include "tao/TaoMessage.h"
#include "tao/TaoObjectMap.h"
#include "tao/TaoReference.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class TaoServerTask;
class TaoListeningTask;

//:Used to send messages, resposible for generating and maintaining transactions,
// resposible for putting TaoMessage out to the wire, maintains "keep alive"
// session with server, maintains sockets for incoming notification response
// messages, maintains db of outstanding transactions.
class TaoTransportTask : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
        TaoTransportTask();
        //:Default constructor

        TaoTransportTask(const char * listenerHost, const  char * listenerPort);
        //:Constructor, establishes socket connection to host

        TaoTransportTask(const int listenerPort);
        //:Constructor, establishes socket connection to host

        TaoTransportTask(const TaoTransportTask& rTaoTransportTask);
     //:Copy constructor (not implemented for this class)

        virtual ~TaoTransportTask();
        //:Destructor

/* ============================ MANIPULATORS ============================== */

        virtual UtlBoolean handleMessage(OsMsg& rMsg);
         //:Handle an incoming message.
         // If the message is not one that the object is prepared to process,
         // the handleMessage() method in the derived class should return FALSE
         // which will cause the OsServerTask::handleMessage() method to be
         // invoked on the message.

        int startListening(void);

        int stopListening(void);

        OsServerTask*   getServer() { return mpServer; };

        void setServer(OsServerTask* pServer) { mpServer = pServer; };

        void setClient(TaoObjHandle hClient) { mClientHandle = hClient; };

/* ============================ ACCESSORS ============================== */
        TaoStatus       getListenSocket(OsConnectionSocket& socket);

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

        void initialize();

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
/* ============================ MANIPULATORS ============================== */
        int send(TaoMessage& rMsg);

/* ============================ VARIABLES ================================ */

        UtlString       mRemoteHost;            // remote TaoServer
        int                     mRemotePort;            // TaoServer's listener port
        int                     mListenerPort;          // this transport's listener port

        TaoObjHandle    mClientHandle;
        OsServerSocket* mpListenSocket;
        TaoListeningTask* mpTaoListeningTask;
        TaoReference*   mpSocketCnt;

        OsServerTask*   mpServer;
        TaoObjectMap*   mpSockets;

        static TaoReference*    mpTransactionCnt;
        static int                              mRef;

};

#endif // _TaoTransportTask_h_
