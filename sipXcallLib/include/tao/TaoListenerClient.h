///
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _TaoListenerClient_h_
#define _TaoListenerClient_h_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// SYSTEM INCLUDES
//#define TAO_TIME_DEBUG

#ifdef TAO_TIME_DEBUG
#include "os/OsTimeLog.h"
#endif


// APPLICATION INCLUDES
#include "os/OsServerTask.h"
#include "os/OsBSem.h"
#include "tao/TaoObject.h"
#include "tao/TaoObjectMap.h"
#include "tao/TaoMessage.h"
#include "tao/TaoDefs.h"        // Added by ClassView

// DEFINES
//#define TAO_TIME_DEBUG
//#define NUM_INITIAL_LISTENERS 20
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class TaoClientTask;
class PtEventListener;
class PtConnectionListener;
class PtTerminalComponentListener;
class PtTerminalConnectionListener;
class PtCallListener;
class PtTerminalListener;
class PtCallEvent;
class PtConnectionEvent;
class PtTerminalConnectionEvent;

//:Used to build the call originating part, establishes connection with the server
// through the TaoTransport. Maintains a db of listeners the client has registered.
class TaoListenerClientTask : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
        TaoListenerClientTask(TaoClientTask* pClient = NULL,
                                const UtlString& name="TaoListenerClientTask-%d",
                                const int maxRequestQMsgs=DEF_MAX_MSGS);
         //:Constructor

        TaoListenerClientTask(const int priority,
                                const UtlString& name="TaoListenerClientTask-%d",
                                void* pArg=NULL,
                                const int maxRequestQMsgs=DEF_MAX_MSGS,
                                const int options=DEF_OPTIONS,
                                const int stackSize=DEF_STACKSIZE);
        //:Constructor

        TaoListenerClientTask(const int maxIncomingQMsgs);
        //:Constructor

        TaoListenerClientTask(const TaoListenerClientTask& rTaoListenerClientTask);
     //:Copy constructor (not implemented for this class)

        virtual ~TaoListenerClientTask();
/* ============================ MANIPULATORS ============================== */

        virtual UtlBoolean handleMessage(OsMsg& rMsg);
         //:Handle an incoming message.
         // If the message is not one that the object is prepared to process,
         // the handleMessage() method in the derived class should return FALSE
         // which will cause the OsServerTask::handleMessage() method to be
         // invoked on the message.

        void addEventListener(PtEventListener* pListener, const char* callId = NULL);

        void removeEventListener(PtEventListener& rListener);


/* ============================ ACCESSORS ================================ */

/* //////////////////////////// PRIVATE ////////////////////////////////// */
private:
friend class PtTerminal;

/* ============================ FUNCTIONS ================================ */
        TaoStatus initInstance();

        UtlBoolean receiveEvent(TaoMessage& rMsg);

#ifdef TAO_TIME_DEBUG
        UtlBoolean receiveCallEvent(TaoMessage& rMsg,
                                                                PtCallListener* pListener,
                                                                OsTimeLog& timeLog);
#else
        UtlBoolean receiveCallEvent(TaoMessage& rMsg,
                                                                PtCallListener* pListener);
#endif

#ifdef TAO_TIME_DEBUG
        UtlBoolean receiveConnectionEvent(TaoMessage& rMsg,
                                                                PtConnectionListener* pListener,
                                                                OsTimeLog& timeLog);
#else
        UtlBoolean receiveConnectionEvent(TaoMessage& rMsg,
                                                                PtConnectionListener* pListener);
#endif

#ifdef TAO_TIME_DEBUG
        UtlBoolean receiveTerminalEvent(TaoMessage& rMsg,
                                                                PtTerminalListener* pListener,
                                                                OsTimeLog& timeLog);
#else
        UtlBoolean receiveTerminalEvent(TaoMessage& rMsg,
                                                                PtTerminalListener* pListener);
#endif

#ifdef TAO_TIME_DEBUG
        UtlBoolean receiveTerminalComponentEvent(TaoMessage& rMsg,
                                                                PtTerminalComponentListener* pListener,
                                                                OsTimeLog& timeLog);
#else
        UtlBoolean receiveTerminalComponentEvent(TaoMessage& rMsg,
                                                                PtTerminalComponentListener* pListener);
#endif

#ifdef TAO_TIME_DEBUG
        UtlBoolean receiveTerminalConnectionEvent(TaoMessage& rMsg,
                                                                PtTerminalConnectionListener* pListener,
                                                                OsTimeLog& timeLog);
#else
        UtlBoolean receiveTerminalConnectionEvent(TaoMessage& rMsg,
                                                                PtTerminalConnectionListener* pListener);
#endif


        UtlBoolean getCallEvent(TaoMessage& rMsg,
                                                   PtCallListener* pListener,
                                                   TaoEventId& evId);

        UtlBoolean getConnectionEvent(TaoMessage& rMsg,
                                                                PtConnectionListener* pListener,
                                                                TaoEventId& evId,
                                                                int& addedToCall,
                                                                int& remoteIsCallee);

        UtlBoolean getTerminalConnectionEvent(TaoMessage& rMsg,
                                                                                PtConnectionListener* pListener,
                                                                                TaoEventId& evId,
                                                                                int& addedToCall,
                                                                                int& remoteIsCallee);

#ifdef WV_DEBUG
        void getEventName(TaoEventId eventId, char *name);
        void fireUserEvent(TaoEventId eventId, TaoEventId userEventId);
#endif
/* ============================ VARIABLES ================================ */
private:

        TaoClientTask*  mpClient;
        OsBSem                  mListenerSem;

        TaoListenerDb** mpListeners;
        int                             mListenerCnt;
        int             mMaxNumListeners;

        PtCallEvent                             *mpCallEvent;
        PtConnectionEvent               *mpConnEvent;
        PtTerminalConnectionEvent               *mpTermConnEvent;


};

#endif // _TaoListenerClient_h_
