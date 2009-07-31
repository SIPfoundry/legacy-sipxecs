//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _TaoEventDispatcher_h_
#define _TaoEventDispatcher_h_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsServerTask.h"
#include "tao/TaoDefs.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class TaoEvent;
class OsServerTask;
class OsEvent;

//:Receives incoming event notificatons, looks uo the corresponding listener and
// invokes the appropriate method.
class TaoEventDispatcher : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
        TaoEventDispatcher(const UtlString& name = "TaoEventDispatcher-%d", const int maxRequestQMsgs=DEF_MAX_MSGS);
         //:Constructor

        virtual ~TaoEventDispatcher();

/* ============================ ACCESSORS ============================== */
        TaoObjHandle& getObjHandle() { return mTaoObjHandle; };

/* ============================ MANIPULATORS ============================== */

        TaoEvent* getProviderEvent() { return mpProviderEvent; };

        virtual UtlBoolean handleMessage(OsMsg& rMsg);
         //:Handle an incoming message.
         // If the message is not one that the object is prepared to process,
         // the handleMessage() method in the derived class should return FALSE
         // which will cause the OsMessageTask::handleMessage() method to be
         // invoked on the message.

        virtual OsStatus setErrno(int errno);
         //:Set the errno status for the task
         // This call has no effect under Windows NT and, if the task has been
         // started, will always returns OS_SUCCESS

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
        TaoObjHandle    mTaoObjHandle;
        TaoEvent                *mpProviderEvent;


};


#endif // _TaoEventDispatcher_h_
