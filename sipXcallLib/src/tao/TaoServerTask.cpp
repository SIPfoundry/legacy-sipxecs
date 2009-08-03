//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#include <assert.h>

#ifdef TEST
#include "utl/UtlMemCheck.h"
#endif

#include "tao/TaoServerTask.h"
#include "tao/TaoTransportTask.h"
#include "tao/TaoString.h"
#include "tao/TaoAdaptor.h"
#include "tao/TaoAddressAdaptor.h"
#include "tao/TaoCallAdaptor.h"
#include "tao/TaoConnectionAdaptor.h"
#include "tao/TaoProviderAdaptor.h"
#include "tao/TaoTerminalAdaptor.h"
#include "tao/TaoTerminalConnectionAdaptor.h"
#include "tao/TaoPhoneComponentAdaptor.h"
#include "ptapi/PtAddress.h"
#include "ptapi/PtCall.h"
#include "ptapi/PtCallListener.h"
#include "ptapi/PtComponent.h"
#include "ptapi/PtProvider.h"
#include "ptapi/PtProviderListener.h"
#include "ptapi/PtTerminal.h"
#include "ptapi/PtTerminalListener.h"
#include "ptapi/PtTerminalConnection.h"

#include "cp/CpCallManager.h"
#include "ps/PsPhoneTask.h"

TaoServerTask*  TaoServerTask::mpInstance = NULL;
OsBSem TaoServerTask::sLock(OsBSem::Q_PRIORITY, OsBSem::FULL);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

// Constructor
TaoServerTask::TaoServerTask(CpCallManager *pCallMgr,
                PsPhoneTask *pPhoneTask,
                const UtlString& name,
                void* pArg,
                const int maxRequestQMsgs,
                const int priority,
                const int options,
                const int stackSize)
                : OsServerTask(name.data(), pArg, maxRequestQMsgs, priority, options, stackSize)
{
        mpCallMgr = pCallMgr;
        mpPhoneTask = pPhoneTask;
        mClientHandle = 0;
        mEventClient = 0;
        if (TAO_SUCCESS == initInstance())
        {
        }
}

// Constructor
TaoServerTask::TaoServerTask(const UtlString& name,
                void* pArg,
                const int maxRequestQMsgs,
                const int priority,
                const int options,
                const int stackSize)
                : OsServerTask(name.data(), pArg, maxRequestQMsgs, priority, options, stackSize)
{
        mClientHandle = 0;
        mEventClient = 0;
        if (TAO_SUCCESS == initInstance())
        {
        }
}

// Constructor
TaoServerTask::TaoServerTask(const int maxIncomingQMsgs)
        : OsServerTask("TaoServerTask-%d", NULL, maxIncomingQMsgs)
{
        mClientHandle = 0;
        mEventClient = 0;
        if (TAO_SUCCESS == initInstance())
        {

        }
}

// Initialization, called by constructor
TaoStatus TaoServerTask::initInstance()
{
        mpTransactionDb  = new TaoObjectMap();
        mpTransactionCnt = new TaoReference();

        mpSvrTransport = new TaoTransportTask(DEF_TAO_LISTEN_PORT);

        mpAdaptors = new TaoObjectMap();
        mpAdaptorCnt = new TaoReference();

        if (!mpSvrTransport || !mpTransactionDb || !mpTransactionCnt)
        {
                return TAO_FAILURE;
        }

        mpSvrTransport->setServer(this);
        if (!(mpSvrTransport->isStarted()))
        {
                mpSvrTransport->start();
        }
        mpSvrTransport->startListening();

        mpListenerMgr = new TaoListenerManager(mpCallMgr, mpPhoneTask, mpSvrTransport);
        if (!(mpListenerMgr->isStarted()))
        {
                mpListenerMgr->start();
        }

        return TAO_SUCCESS;
}


// Destructor
TaoServerTask::~TaoServerTask()
{
        if (mpSvrTransport)
        {
                mpSvrTransport->stopListening();    // shut down the transport task
                mpSvrTransport->requestShutdown();    // shut down the transport task
                delete mpSvrTransport;
                mpSvrTransport = 0;
        }

        if (mpTransactionDb)
        {
                delete mpTransactionDb;
                mpTransactionDb = 0;
        }

        if (mpListenerMgr)
        {
                delete mpListenerMgr;
                mpListenerMgr = 0;
        }

        if (mpAdaptors)
        {
                delete mpAdaptors;
                mpAdaptors = 0;
        }

        if (mpAdaptorCnt)
        {
                delete mpAdaptorCnt;
                mpAdaptorCnt = 0;
        }
}

TaoServerTask* TaoServerTask::getTaoServerTask(CpCallManager *pCallMgr)
{
   UtlBoolean isStarted;

   // If the task object already exists, and the corresponding low-level task
   // has been started, then use it
   if (mpInstance != NULL && mpInstance->isStarted())
      return mpInstance;

   // If the task does not yet exist or hasn't been started, then acquire
   // the lock to ensure that only one instance of the task is started
   sLock.acquire();
   if (mpInstance == NULL)
       mpInstance = new TaoServerTask(pCallMgr, PsPhoneTask::getPhoneTask());

   isStarted = mpInstance->isStarted();
   if (!isStarted)
   {
      isStarted = mpInstance->start();
//      assert(isStarted);
   }
   sLock.release();

#  ifdef TEST
   if (!sIsTested)
   {
      sIsTested = true;
//      mpInstance->test();
   }
#  endif //TEST

   return mpInstance;
}

void TaoServerTask::setEventClient(TaoObjHandle hEventClient)
{
        mEventClient = hEventClient;
        mpListenerMgr->setEventClient(hEventClient);
}

void TaoServerTask::setClientHandle(TaoObjHandle hClient)
{
        mClientHandle = hClient;
        mpSvrTransport->setClient(hClient);
}

//////////////////////////////////////////////////////////////////////
// MANIPULATORS
//////////////////////////////////////////////////////////////////////
UtlBoolean TaoServerTask::handleMessage(OsMsg& rMsg)
{
        UtlBoolean              handled = FALSE;
        TaoObjHandle    hAdaptor;

        switch (rMsg.getMsgSubType())
        {
        case TaoMessage::REQUEST_PROVIDER:
                if (TAO_NOT_FOUND == mpAdaptors->findValue(TaoMessage::REQUEST_PROVIDER, hAdaptor))
                {
                        TaoProviderAdaptor*     pAdaptor;
                        pAdaptor = new TaoProviderAdaptor(mpCallMgr,
                                                                                        mpSvrTransport,
                                                                                        (TaoMessage&)rMsg);
                        pAdaptor->setListenerManager(mpListenerMgr);
                        mpAdaptorCnt->add();
                        mpAdaptors->insert((TaoObjHandle)TaoMessage::REQUEST_PROVIDER, (TaoObjHandle)pAdaptor);
                        hAdaptor = (TaoObjHandle)pAdaptor;
                }
                if (hAdaptor)
                {
                        ((TaoProviderAdaptor *)hAdaptor)->postMessage(rMsg);
                        handled = TRUE;
                }
                break;
        case TaoMessage::REQUEST_ADDRESS:
                if (TAO_NOT_FOUND == mpAdaptors->findValue(TaoMessage::REQUEST_ADDRESS, hAdaptor))
                {
                        TaoAdaptor *pAdaptor = new TaoAddressAdaptor(mpSvrTransport,
                                                                                                                 mpCallMgr,
                                                                                                                 (TaoMessage&)rMsg, "");
                        pAdaptor->setListenerManager(mpListenerMgr);
                        mpAdaptorCnt->add();
                        mpAdaptors->insert((TaoObjHandle)TaoMessage::REQUEST_ADDRESS, (TaoObjHandle)pAdaptor);
                        hAdaptor = (TaoObjHandle)pAdaptor;
                }
                if (hAdaptor)
                {
                        ((TaoAddressAdaptor *)hAdaptor)->postMessage(rMsg);
                        handled = TRUE;
                }
                break;
        case TaoMessage::REQUEST_CALL:
                if (TAO_NOT_FOUND == mpAdaptors->findValue(TaoMessage::REQUEST_CALL, hAdaptor))
                {
                        TaoAdaptor *pAdaptor = new TaoCallAdaptor(mpSvrTransport,
                                                                                                        mpCallMgr,
                                                                                                        (TaoMessage&)rMsg);
                        pAdaptor->setListenerManager(mpListenerMgr);
                        mpAdaptorCnt->add();
                        mpAdaptors->insert((TaoObjHandle)TaoMessage::REQUEST_CALL, (TaoObjHandle)pAdaptor);
                        hAdaptor = (TaoObjHandle)pAdaptor;
                }
                if (hAdaptor)
                {
                        ((TaoCallAdaptor *)hAdaptor)->postMessage(rMsg);
                        handled = TRUE;
                }
                break;
        case TaoMessage::REQUEST_CONNECTION:
                if (TAO_NOT_FOUND == mpAdaptors->findValue(TaoMessage::REQUEST_CONNECTION, hAdaptor))
                {
                        TaoAdaptor *pAdaptor = new TaoConnectionAdaptor(mpSvrTransport,
                                                                                                        mpCallMgr,
                                                                                                        (TaoMessage&)rMsg);
                        pAdaptor->setListenerManager(mpListenerMgr);
                        mpAdaptorCnt->add();
                        mpAdaptors->insert((TaoObjHandle)TaoMessage::REQUEST_CONNECTION, (TaoObjHandle)pAdaptor);
                        hAdaptor = (TaoObjHandle)pAdaptor;
                }
                if (hAdaptor)
                {
                        ((TaoConnectionAdaptor *)hAdaptor)->postMessage(rMsg);
                        handled = TRUE;
                }
                break;
        case TaoMessage::REQUEST_TERMCONNECTION:
                if (TAO_NOT_FOUND == mpAdaptors->findValue(TaoMessage::REQUEST_TERMCONNECTION, hAdaptor))
                {
                        TaoAdaptor *pAdaptor = new TaoTerminalConnectionAdaptor(mpSvrTransport,
                                                                                                        mpCallMgr,
                                                                                                        (TaoMessage&)rMsg);
                        pAdaptor->setListenerManager(mpListenerMgr);
                        mpAdaptorCnt->add();
                        mpAdaptors->insert((TaoObjHandle)TaoMessage::REQUEST_TERMCONNECTION, (TaoObjHandle)pAdaptor);
                        hAdaptor = (TaoObjHandle)pAdaptor;
                }
                if (hAdaptor)
                {
                        ((TaoTerminalConnectionAdaptor *)hAdaptor)->postMessage(rMsg);
                        handled = TRUE;
                }
                break;
        case TaoMessage::REQUEST_TERMINAL:
                if (TAO_NOT_FOUND == mpAdaptors->findValue(TaoMessage::REQUEST_TERMINAL, hAdaptor))
                {
                        TaoAdaptor *pAdaptor = new TaoTerminalAdaptor(mpCallMgr,
                                                                                                                mpPhoneTask,
                                                                                                                mpSvrTransport,
                                                                                                                mpListenerMgr,
                                                                                                                (TaoMessage&)rMsg);
                        pAdaptor->setListenerManager(mpListenerMgr);
                        mpAdaptorCnt->add();
                        mpAdaptors->insert((TaoObjHandle)TaoMessage::REQUEST_TERMINAL, (TaoObjHandle)pAdaptor);
                        hAdaptor = (TaoObjHandle)pAdaptor;
                }
                if (hAdaptor)
                {
                        ((TaoTerminalAdaptor *)hAdaptor)->postMessage(rMsg);
                        handled = TRUE;
                }
                break;
        case TaoMessage::REQUEST_PHONECOMPONENT:
                if (TAO_NOT_FOUND == mpAdaptors->findValue(TaoMessage::REQUEST_PHONECOMPONENT, hAdaptor))
                {
                        TaoAdaptor *pAdaptor = new TaoPhoneComponentAdaptor(mpSvrTransport,
                                                                                                                                (TaoMessage&)rMsg);
                        pAdaptor->setListenerManager(mpListenerMgr);
                        mpAdaptorCnt->add();
                        mpAdaptors->insert((TaoObjHandle)TaoMessage::REQUEST_PHONECOMPONENT, (TaoObjHandle)pAdaptor);
                        hAdaptor = (TaoObjHandle)pAdaptor;
                }
                if (hAdaptor)
                {
                        ((TaoTerminalAdaptor *)hAdaptor)->postMessage(rMsg);
                        handled = TRUE;
                }
                break;
        case TaoMessage::UNSPECIFIED:
        default:
//         assert(FALSE);
          break;
        }

        return handled;
}

// Set the errno status for the task.
// This call has no effect under Windows NT and, if the task has been
// started, will always returns OS_SUCCESS
OsStatus TaoServerTask::setErrno(int errno)
{
   if (!isStarted())
      return OS_TASK_NOT_STARTED;

   return OS_SUCCESS;
}

#ifdef TEST

bool TaoServerTask::sIsTested = false;

#endif
