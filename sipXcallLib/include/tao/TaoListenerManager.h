//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _TaoListenerManager_h_
#define _TaoListenerManager_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES

#include <os/OsServerTask.h>
#include <os/OsRWMutex.h>
#include <os/OsBSem.h>

#include "tao/TaoDefs.h"
#include "tao/TaoObjectMap.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlHashBagIterator;
class TaoEventListener;
class TaoTransportTask;
class TaoMessage;
class CpCallManager;
class PsPhoneTask;
class PsHookswTask;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class TaoListenerManager : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   TaoListenerManager();
     //:Default constructor

   TaoListenerManager(CpCallManager *pCallMgr,
                PsPhoneTask *pPhoneTask,
                TaoTransportTask*& rpSvrTransport);
     //:Default constructor

   TaoListenerManager(const TaoListenerManager& rTaoListenerManager);
     //:Copy constructor

   virtual
   ~TaoListenerManager();
     //:Destructor

/* ============================ MANIPULATORS ============================== */


   virtual UtlBoolean handleMessage(OsMsg& eventMessage);

   TaoStatus addEventListener(const char* terminalName, UtlBoolean call);

   TaoStatus addEventListener(TaoMessage& rMsg);

   TaoStatus addCallListener(TaoMessage& rMsg);

   TaoStatus removeEventListener(const char* terminalName);

   TaoStatus removeEventListener(TaoMessage& rMsg);

   TaoListenerManager& operator=(const TaoListenerManager& rhs);
     //:Assignment operator

   void setEventClient(TaoObjHandle hEventClient) { mEventClient = hEventClient; };


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
        TaoObjectMap*                   mpConnectionSockets;
        TaoObjectMap*                   mpAgents;
        TaoTransportTask*               mpSvrTransport;
        OsRWMutex                               mListenerRWLock;
        TaoObjHandle                    mEventClient;

        TaoListenerDb**                 mpListeners;
        int                             mListenerCnt;
        int                             mMaxNumListeners;

        CpCallManager*                  mpCallMgr;
        PsPhoneTask*                    mpPhoneTask;
        PsHookswTask*                   mpHookswTask;

        bool                                            mListenerAdded;

    // NOTE: THIS DOES NOT LOCK
    void resetEventListenerIterator();

    // NOTE: THIS DOES NOT LOCK
    void removeThisEventListener();


};

/* ============================ INLINE METHODS ============================ */

#endif  // _TaoListenerManager_h_
