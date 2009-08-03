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

#ifdef __pingtel_on_posix__
#include <stdlib.h>
#endif

#include "net/Url.h"
#include "tao/TaoProviderAdaptor.h"
#include "tao/TaoTransportTask.h"
#include "tao/TaoString.h"
#include "tao/TaoProviderListener.h"
#include "ptapi/PtProvider.h"
#include "cp/CallManager.h"
#include "ps/PsPhoneTask.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TaoProviderAdaptor::TaoProviderAdaptor(CpCallManager *pCallMgr,
                                                                           TaoTransportTask*& rpSvrTransport,
                                                                           TaoMessage& rMsg,
                                                                           const UtlString& name,
                                                                           const int maxRequestQMsgs)
        : TaoAdaptor("TaoProviderAdaptor-%d", maxRequestQMsgs),
        mpCallMgrTask(pCallMgr),
        mpMediaTask(NULL),
        mpPhoneTask(NULL),
        mpTimerTask(NULL)
{
        mpSvrTransport = rpSvrTransport;
        parseMessage(rMsg);

        mpCallDb  = new TaoObjectMap();
        mpCallCnt = new TaoReference();
        mpProviderListenerDb  = new TaoObjectMap();
        mpProviderListenerCnt = new TaoReference();

        startAdaptor();
}

TaoProviderAdaptor::~TaoProviderAdaptor()
{
        mState = PtProvider::OUT_OF_SERVICE;

        if (mpCallDb)
        {
                delete mpCallDb;
                mpCallDb = 0;
        }

        if (mpCallDb)
        {
                delete mpCallDb;
                mpCallDb = 0;
        }

        if (mpCallCnt)
        {
                delete mpCallCnt;
                mpCallCnt = 0;
        }

        if (mpProviderListenerDb)
        {
                delete mpProviderListenerDb;
                mpProviderListenerDb = 0;
        }

        if (mpProviderListenerCnt)
        {
                delete mpProviderListenerCnt;
                mpProviderListenerCnt = 0;
        }
}

void TaoProviderAdaptor::startAdaptor()
{
        if (!isStarted())
        {
                start();
        }
        mState = PtProvider::IN_SERVICE;
}

////////////////////////////////////////////////////////

UtlBoolean TaoProviderAdaptor::handleMessage(OsMsg& rMsg)
{
        UtlBoolean handled = FALSE;

        switch (((TaoMessage&)rMsg).getCmd())
        {
        case TaoMessage::GET_PROVIDER:
                if (TAO_SUCCESS == providerGetProvider((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_ADDRESS:
                if (TAO_SUCCESS == providerGetAddress((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_ADDRESSES:
                if (TAO_SUCCESS == providerGetAddresses((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_TERMINAL:
                if (TAO_SUCCESS == providerGetTerminal((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::CREATE_CALL:
                if (TAO_SUCCESS == providerCreateCall((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_TERMINALS:
                if (TAO_SUCCESS == providerGetTerminals((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_CALLS:
                if (TAO_SUCCESS == providerGetCalls((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_STATE:
                if (TAO_SUCCESS == providerGetState((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::GET_PROVIDER_LISTENERS:
                if (TAO_SUCCESS == providerGetProviderListeners((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::ADD_PROVIDER_LISTENER:
                if (TAO_SUCCESS == providerAddProviderListener((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::REMOVE_PROVIDER_LISTENER:
                if (TAO_SUCCESS == providerRemoveProviderListener((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::NUM_ADDRESSES:
                if (TAO_SUCCESS == providerNumAddresses((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::NUM_TERMINALS:
                if (TAO_SUCCESS == providerNumTerminals((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::NUM_CALLS:
                if (TAO_SUCCESS == providerNumCalls((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::NUM_PROVIDER_LISTENERS:
                if (TAO_SUCCESS == providerNumProviderListeners((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::SHUTDOWN:
                if (TAO_SUCCESS == providerShutdown((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        case TaoMessage::CREATE_CALL_RESULT:
                if (TAO_SUCCESS == getCreateCall((TaoMessage&)rMsg))
                {
                        handled = TRUE;
                }
                break;
        default:
//         assert(FALSE);
          break;
        }

        return handled;
}

TaoStatus TaoProviderAdaptor::providerGetProvider(TaoMessage& rMsg)
{
        // UtlString argList = rMsg.getArgList();
        int              argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;


        TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        UtlString  login = arg[0];
        UtlString  pass  = arg[1];

        UtlString               value;
        TaoObjHandle    clientSocket = rMsg.getSocket();
        TaoMessage*             pMsg;

        pMsg = new TaoMessage(TaoMessage::RESPONSE_PROVIDER,
                                                                                TaoMessage::GET_PROVIDER,
                                                                                rMsg.getMsgID(),
                                                                                OS_SUCCESS,
                                                                                clientSocket,
                                                                                0,
                                                                                "");

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoProviderAdaptor::providerGetAddress(TaoMessage& rMsg)
{
   int argCnt = rMsg.getArgCnt();

   if (argCnt != 1)
      return TAO_FAILURE;

   UtlString value;

   TaoObjHandle objId = TAO_FAILURE;
   UtlString     strPhoneURL = rMsg.getArgList();

   int iAvailableAddresses = 0 ;
   int actual = 0;

   int iMaxAddressesRequested = mpCallMgrTask->getNumLines();

   UtlString** arrayAddresses = new UtlString*[iMaxAddressesRequested];
   for (int j=0; j<iMaxAddressesRequested; j++)
   {
      arrayAddresses[j] = new UtlString("");
   }

   mpCallMgrTask->getOutboundAddresses(iMaxAddressesRequested,
         iAvailableAddresses, arrayAddresses);

   actual = ((iMaxAddressesRequested < iAvailableAddresses) ?
            iMaxAddressesRequested : iAvailableAddresses);
   if (actual > 0)
   {
                for (int i = 0; i < actual; i++)
                {
            Url identityUrl(*arrayAddresses[i]);
            Url phoneURL(strPhoneURL);
            UtlString strIdentity;
            UtlString strPhone;
            identityUrl.getUri(strIdentity);
            phoneURL.getUri(strPhone);

            if(strPhone.compareTo(strIdentity,UtlString::ignoreCase) == 0)
            {
               value = identityUrl.toString();
               break;
            }
        }
   }


   if( arrayAddresses != NULL )
   {
      for (int i=0; i<iMaxAddressesRequested; i++)
      {
         delete arrayAddresses[i] ;
         arrayAddresses[i] = NULL ;
      }
      delete[] arrayAddresses ;
   }

   TaoObjHandle clientSocket = rMsg.getSocket();
   TaoMessage* pMsg = new TaoMessage(TaoMessage::RESPONSE_PROVIDER,
                                                                         TaoMessage::GET_ADDRESS,
                                                                         rMsg.getMsgID(),
                                                                         (TaoObjHandle)objId,
                                                                         clientSocket,
                                                                         iAvailableAddresses,
                                                                         value);

   if (mpSvrTransport->postMessage(*pMsg))
   {
      delete pMsg;
      return TAO_SUCCESS;
   }

   return TAO_FAILURE;
}

TaoStatus TaoProviderAdaptor::providerGetAddresses(TaoMessage& rMsg)
{
   int  argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

   UtlString argList;

   int iAvailableAddresses = 0 ;
   int actual = 0;

   int iMaxAddressesRequested = mpCallMgrTask->getNumLines();
   if( iMaxAddressesRequested == 0)
      return TAO_FAILURE;

   UtlString** arrayAddresses = new UtlString*[iMaxAddressesRequested];
   for (int j=0; j<iMaxAddressesRequested; j++)
   {
      arrayAddresses[j] = new UtlString("");
   }

   mpCallMgrTask->getOutboundAddresses(iMaxAddressesRequested, iAvailableAddresses, arrayAddresses);

   actual = ((iMaxAddressesRequested < iAvailableAddresses) ? iMaxAddressesRequested : iAvailableAddresses);

   if (actual > 0)
   {
                for (int i = 0; i < actual; i++)
                {
            argList += *arrayAddresses[i] ;
                        if (i < (actual - 1))
                        {
                                argList += TAOMESSAGE_DELIMITER;
                        }
                }
        }


   if( arrayAddresses != NULL )
   {
      for (int i=0; i<iMaxAddressesRequested; i++)
      {
         delete arrayAddresses[i] ;
         arrayAddresses[i] = NULL ;
      }
      delete[] arrayAddresses ;
   }




        TaoObjHandle clientSocket = rMsg.getSocket();
        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_PROVIDER,
                                                                        TaoMessage::GET_ADDRESSES,
                                                                        rMsg.getMsgID(),
                                                                        iAvailableAddresses,
                                                                        clientSocket,
                                                                        actual,
                                                                        argList);

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}


TaoStatus TaoProviderAdaptor::providerGetTerminal(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        TaoObjHandle objId = rMsg.getTaoObjHandle();
        UtlString        name = rMsg.getArgList();
        UtlString        value ="localterminal";

        TaoObjHandle clientSocket = rMsg.getSocket();
        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_PROVIDER,
                                                                        TaoMessage::GET_TERMINAL,
                                                                        rMsg.getMsgID(),
                                                                        objId,
                                                                        clientSocket,
                                                                        1,
                                                                        value);

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoProviderAdaptor::providerGetTerminals(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        UtlString argList = "localterminal" ;

        TaoObjHandle clientSocket = rMsg.getSocket();
        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_PROVIDER,
                                                                        TaoMessage::GET_TERMINALS,
                                                                        rMsg.getMsgID(),
                                                                        1,
                                                                        clientSocket,
                                                                        1,
                                                                        argList);

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoProviderAdaptor::providerCreateCall(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 0)
                return TAO_FAILURE;

        UtlString callId;

        if (mState == PtProvider::IN_SERVICE)
        {
                mpCallMgrTask->createCall(&callId);

                argCnt = 1;
        }
        else
        {
                rMsg.setMsgSubType(TaoMessage::RESPONSE_PROVIDER);
                argCnt = 0;
        }

        TaoObjHandle clientSocket = rMsg.getSocket();
        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_PROVIDER,
                                                                        TaoMessage::CREATE_CALL,
                                                                        rMsg.getMsgID(),
                                                                        rMsg.getTaoObjHandle(),
                                                                        clientSocket,
                                                                        argCnt,
                                                                        callId);

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoProviderAdaptor::getCreateCall(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 0)
                return TAO_FAILURE;

        TaoObjHandle  clientSocket = rMsg.getSocket();
        UtlString callId;
        TaoObjHandle objId = rMsg.getTaoObjHandle();

        mpCallDb->insert(mpCallCnt->add(), objId);
        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_PROVIDER,
                                                                        TaoMessage::CREATE_CALL,
                                                                        rMsg.getMsgID(),
                                                                        objId,
                                                                        clientSocket,
                                                                        0,
                                                                        "");

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }


        return TAO_FAILURE;
}

TaoStatus TaoProviderAdaptor::providerGetCalls(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        int nItems = mpCallCnt->getRef();
        int     size   = atoi(rMsg.getArgList());
        int actual = ((size < nItems) ? size : nItems);

        TaoObjHandle    *pCalls;
        pCalls = new TaoObjHandle[nItems];

        mpCallDb->getActiveObjects(pCalls, nItems);

        UtlString argList;
        UtlString rNextKey;
        UtlString rNextValue;

    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

        for (int i = 0; i < actual; i++)
        {
                sprintf(buff, "%ld", pCalls[i]);
                argList += (UtlString) buff;

                if (i < (actual - 1))
                {
                        argList += TAOMESSAGE_DELIMITER;
                }
        }

        TaoObjHandle clientSocket = rMsg.getSocket();
        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_PROVIDER,
                                                                        TaoMessage::GET_ADDRESSES,
                                                                        rMsg.getMsgID(),
                                                                        nItems,
                                                                        clientSocket,
                                                                        actual,
                                                                        argList);

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoProviderAdaptor::providerGetState(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 0)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_PROVIDER,
                                                                        TaoMessage::GET_STATE,
                                                                        rMsg.getMsgID(),
                                                                        mState,
                                                                        clientSocket,
                                                                        0,
                                                                        "");

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoProviderAdaptor::providerAddProviderListener(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getMsgID();
        TaoObjHandle objHdl = rMsg.getTaoObjHandle();
        TaoString        argList(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        UtlString        sProviderListener =  argList[0];
        UtlString        sTerminalName =  argList[1];

        if (mObjId)
        {
                TaoObjHandle objId = atol(sProviderListener);
                TaoProviderListener *pListener;
                pListener = new TaoProviderListener(objId, clientSocket, mpSvrTransport, sTerminalName);
                objId = mpProviderListenerCnt->add();
                mpProviderListenerDb->insert(objId, (TaoObjHandle)pListener);
        }


        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_PROVIDER,
                                                                        TaoMessage::ADD_PROVIDER_LISTENER,
                                                                        rMsg.getMsgID(),
                                                                        objHdl,
                                                                        clientSocket,
                                                                        0,
                                                                        "");

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoProviderAdaptor::providerRemoveProviderListener(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        TaoObjHandle objId = rMsg.getTaoObjHandle();
//////////////

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_PROVIDER,
                                                                        TaoMessage::REMOVE_PROVIDER_LISTENER,
                                                                        rMsg.getMsgID(),
                                                                        objId,
                                                                        clientSocket,
                                                                        0,
                                                                        "");

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoProviderAdaptor::providerGetProviderListeners(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        int nItems = mpProviderListenerCnt->getRef();
        int     size   = atoi(rMsg.getArgList());
        int actual = ((size < nItems) ? size : nItems);

        TaoObjHandle    *pProviderListeners;
        pProviderListeners = new TaoObjHandle[nItems];

        mpProviderListenerDb->getActiveObjects(pProviderListeners, nItems);

        UtlString argList;
        UtlString rNextKey;
        UtlString rNextValue;

    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

        for (int i = 0; i < actual; i++)
        {
                sprintf(buff, "%ld", pProviderListeners[i]);
                argList += (UtlString) buff;

                if (i < (actual - 1))
                {
                        argList += TAOMESSAGE_DELIMITER;
                }
        }

        TaoObjHandle clientSocket = rMsg.getSocket();
        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_PROVIDER,
                                                                        TaoMessage::GET_PROVIDER_LISTENERS,
                                                                        rMsg.getMsgID(),
                                                                        nItems,
                                                                        clientSocket,
                                                                        actual,
                                                                        argList);

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoProviderAdaptor::providerNumAddresses(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 0)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();
        int count = 1;

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_PROVIDER,
                                                                        TaoMessage::NUM_ADDRESSES,
                                                                        rMsg.getMsgID(),
                                                                        count,
                                                                        clientSocket,
                                                                        0,
                                                                        "");

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoProviderAdaptor::providerNumTerminals(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 0)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();
        int count = 1 ;

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_PROVIDER,
                                                                        TaoMessage::NUM_TERMINALS,
                                                                        rMsg.getMsgID(),
                                                                        count,
                                                                        clientSocket,
                                                                        0,
                                                                        "");

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoProviderAdaptor::providerNumCalls(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 0)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();
        int count = mpCallCnt->getRef();

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_PROVIDER,
                                                                        TaoMessage::NUM_CALLS,
                                                                        rMsg.getMsgID(),
                                                                        count,
                                                                        clientSocket,
                                                                        0,
                                                                        "");

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoProviderAdaptor::providerNumProviderListeners(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 0)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();
        int count = mpProviderListenerCnt->getRef();

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_PROVIDER,
                                                                        TaoMessage::NUM_PROVIDER_LISTENERS,
                                                                        rMsg.getMsgID(),
                                                                        count,
                                                                        clientSocket,
                                                                        0,
                                                                        "");

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoProviderAdaptor::providerShutdown(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 0)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        TaoObjHandle objId = rMsg.getTaoObjHandle();
///////////////////
//      mState = SHUT_DOWN;
        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_PROVIDER,
                                                                        TaoMessage::SHUTDOWN,
                                                                        rMsg.getMsgID(),
                                                                        objId,
                                                                        clientSocket,
                                                                        0,
                                                                        "");

        osPrintf("==== TaoProviderAdaptor::providerShutdown %ld\n", objId);
        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}
