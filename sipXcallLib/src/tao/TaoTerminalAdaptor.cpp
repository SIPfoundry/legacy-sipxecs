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

#include "tao/TaoTerminalAdaptor.h"
#include "tao/TaoTransportTask.h"
#include "tao/TaoString.h"
#include "tao/TaoListenerManager.h"
#include "ps/PsMsg.h"
#include "ps/PsPhoneTask.h"
#include "cp/CpCallManager.h"

#ifdef _WIN32
#  define CONFIG_PREFIX_USER_DATA               "..\\meta\\"
#  define CONFIG_PREFIX_SHARED_DATA             "..\\meta\\"
#  define CONFIG_FILE_NAME CONFIG_PREFIX_USER_DATA "pinger-config"
#else
#  define CONFIG_FILE_NAME "pinger-config"
#endif


#define CONFIG_FILE_NAME_IN_FLASH "/flash0/pinger-config"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TaoTerminalAdaptor::TaoTerminalAdaptor(CpCallManager *pCallMgr,
                                           PsPhoneTask *pPhoneTask,
                                           TaoTransportTask*& rpSvrTransport,
                                           TaoListenerManager* pListenerMgr,
                                           TaoMessage& rMsg,
                                           const UtlString& name,
                                           const int maxRequestQMsgs)
        : TaoAdaptor(name, maxRequestQMsgs),
        mpCallMgrTask(pCallMgr)
{
        mpSvrTransport = rpSvrTransport;
        mpPhoneTask = pPhoneTask;
        mpListenerMgr = pListenerMgr;
        parseMessage(rMsg);

        mpObjectDb  = new TaoObjectMap();
        mpObjectCnt = new TaoReference();

   const char* configFileName =
#ifdef _VXWORKS
   CONFIG_FILE_NAME_IN_FLASH;
#else
   CONFIG_FILE_NAME;
#endif

        initConfigFile(configFileName);

        if (!isStarted())
        {
                start();
        }
}

TaoTerminalAdaptor::TaoTerminalAdaptor(CpCallManager *pCallMgr,
                                           PsPhoneTask *pPhoneTask,
                                           TaoTransportTask*& rpSvrTransport,
                                           TaoListenerManager* pListenerMgr,
                                           const UtlString& name,
                                           const int maxRequestQMsgs)
        : TaoAdaptor(name, maxRequestQMsgs),
        mpCallMgrTask(pCallMgr)
{
        mpPhoneTask = pPhoneTask;
        mpSvrTransport = rpSvrTransport;
        mpListenerMgr = pListenerMgr;
        mpObjectDb  = new TaoObjectMap();
        mpObjectCnt = new TaoReference();
        if (!isStarted())
        {
                start();
        }
}

TaoTerminalAdaptor::~TaoTerminalAdaptor()
{
        if (mpObjectDb)
        {
                delete mpObjectDb;
                mpObjectDb = 0;
        }

        if (mpObjectCnt)
        {
                delete mpObjectCnt;
                mpObjectCnt = 0;
        }
}

UtlBoolean TaoTerminalAdaptor::handleMessage(OsMsg& rMsg)
{
        UtlBoolean handled = FALSE;
        int msgType = rMsg.getMsgSubType();
        int cmd = ((TaoMessage&)rMsg).getCmd();

        if( TaoMessage::REQUEST_TERMINAL == msgType)
        {
                switch (cmd)
                {
                case TaoMessage::ADD_CALL_LISTENER:
                        if (TAO_SUCCESS == terminalAddCallListener((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::ADD_TERM_LISTENER:
                        if (TAO_SUCCESS == terminalAddTermListener((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::GET_ADDRESSES:
                        if (TAO_SUCCESS == terminalGetAddresses((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::GET_CALL_LISTENERS:
                        if (TAO_SUCCESS == terminalGetCallListeners((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::GET_COMPONENT:
                        if (TAO_SUCCESS == terminalGetComponent((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::GET_COMPONENTS:
                        if (TAO_SUCCESS == terminalGetComponents((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::GET_COMPONENTGROUPS:
                        if (TAO_SUCCESS == terminalGetComponentGroups((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::GET_CONFIG:
                        if (TAO_SUCCESS == terminalGetConfig((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::GET_DONOT_DISTURB:
                        if (TAO_SUCCESS == terminalGetDoNotDisturb((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::GET_NAME:
                        if (TAO_SUCCESS == terminalGetName((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::GET_TERM_CONNECTIONS:
                        if (TAO_SUCCESS == terminalGetTermConnections((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::GET_TERM_LISTENERS:
                        if (TAO_SUCCESS == terminalGetTermListeners((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::GET_PROVIDER:
                        if (TAO_SUCCESS == terminalGetProvider((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::NUM_ADDRESSES:
                        if (TAO_SUCCESS == terminalNumAddresses((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::NUM_CALL_LISTENERS:
                        if (TAO_SUCCESS == terminalNumCallListeners((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::NUM_COMPONENTS:
                        if (TAO_SUCCESS == terminalNumComponents((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::NUM_TERM_LISTENERS:
                        if (TAO_SUCCESS == terminalNumTermListeners((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::NUM_TERM_CONNECTIONS:
                        if (TAO_SUCCESS == terminalNumTermConnectionss((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::PICKUP:
                        if (TAO_SUCCESS == terminalPickup((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::REMOVE_CALL_LISTENER:
                        if (TAO_SUCCESS == terminalRemoveCallListener((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::REMOVE_TERM_LISTENER:
                        if (TAO_SUCCESS == terminalRemoveTermListener((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                case TaoMessage::SET_DONOT_DISTURB:
                        if (TAO_SUCCESS == terminalSetDoNotDisturb((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;

                case TaoMessage::SET_INBOUND_CODEC_CPU_LIMIT:
                        if (TAO_SUCCESS == terminalSetCodecCPULimit((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;

                case TaoMessage::TERMINAL_RESULT:
                        if (TAO_SUCCESS == returnResult((TaoMessage&)rMsg))
                        {
                                handled = TRUE;
                        }
                        break;
                default:
                  break;
                }
        }
        else if (TaoMessage::RESPONSE_TERMINAL == msgType)
        {
                if (TAO_SUCCESS == returnResult((TaoMessage&)rMsg))
                        handled = TRUE;
        }


        if (!handled)
        {
                TaoMessage*     pMsg = (TaoMessage*) &rMsg;

                pMsg->setArgCnt(1);
                pMsg->setArgList("-1");

                if (mpSvrTransport->postMessage(*pMsg))
                        handled = TRUE;
        }

        return handled;
}

/* ----------------------------- TERMINAL --------------------------------- */
TaoStatus TaoTerminalAdaptor::terminalAddCallListener(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        UtlString arg = UtlString("0");
        if (mpListenerMgr)
        {
                argCnt = 0;
                mpListenerMgr->addCallListener(rMsg);
        }
        else
        {
                argCnt = 1;
                arg = UtlString("-1");  // indicating failure
        }


        TaoObjHandle clientSocket = rMsg.getSocket();
        TaoObjHandle msgId = rMsg.getMsgID();
        TaoObjHandle objId = rMsg.getTaoObjHandle();

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_TERMINAL,
                                                                        TaoMessage::ADD_CALL_LISTENER,
                                                                        msgId,
                                                                        objId,
                                                                        clientSocket,
                                                                        argCnt,
                                                                        arg);

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoTerminalAdaptor::terminalAddTermListener(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        UtlString arg = UtlString("0");
        if (mpListenerMgr)
        {
                argCnt = 0;
                mpListenerMgr->addEventListener(rMsg);
        }
        else
        {
                argCnt = 1;
                arg = UtlString("-1");  // indicating failure
        }


        TaoObjHandle clientSocket = rMsg.getSocket();
        TaoObjHandle msgId = rMsg.getMsgID();
        TaoObjHandle objId = rMsg.getTaoObjHandle();

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_TERMINAL,
                                                                        TaoMessage::ADD_TERM_LISTENER,
                                                                        msgId,
                                                                        objId,
                                                                        clientSocket,
                                                                        argCnt,
                                                                        arg);

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

/**
* getAddresses() basically does the same thing as
* TaoProviderAdapter:getAddresses() in this implementation.
* It returns the user lines as addresses.
*/
TaoStatus TaoTerminalAdaptor::terminalGetAddresses(TaoMessage& rMsg)
{
  int   argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;
   UtlString thisTerminal = rMsg.getArgList();
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
                         UtlString strIdentity = *arrayAddresses[i];
          argList += strIdentity ;

                        if (i < (actual - 1))
                        {
                                argList += TAOMESSAGE_DELIMITER;
                        }
                }
        }
        else
        {
                UtlString rNextValue;
      iAvailableAddresses = actual = 1;
                if (thisTerminal.isNull())
                        argList = UtlString("sip:");
                else
                        argList = UtlString("sip:") + thisTerminal + UtlString("@");
                UtlString strHostIP;
      OsSocket::getHostIp(&strHostIP);
                argList += strHostIP;

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
        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_TERMINAL,
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

void TaoTerminalAdaptor::initConfigFile(const char* configFileName)
{
    OsConfigDb  configDb;

        if (configDb.loadFromFile((char *)configFileName) == OS_SUCCESS)
        {
      osPrintf("Found config file %s.\n", configFileName);
          configDb.getSubHash("PINGTEL_ADDRESS.", mAddresses);
    }
        else
        {
      osPrintf("Config file %s not found.\n", configFileName);
        }
}

TaoStatus TaoTerminalAdaptor::terminalGetCallListeners(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        TaoObjHandle objId = rMsg.getTaoObjHandle();
        TaoObjHandle pTerminal;

        int      size =  atoi(rMsg.getArgList());
//      int      nItems;
//      PtCallListener  *rpCallListeners;

//      rpCallListeners = new PtCallListener[size];

        if (TAO_NOT_FOUND != mpObjectDb->findValue(objId, pTerminal))
        {
                if (pTerminal)
                {
//                      ((PtTerminal *) pTerminal)->getCallListeners(rpCallListeners, size, nItems);
                }
        }

        UtlString argList;
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

//      int actual = ((size < nItems) ? size : nItems);

        for (int i = 0; i < size; i++)
        {
                TaoObjHandle objId = mpObjectCnt->add();
//              mpObjectDb->insert((TaoObjHandle)objId, (TaoObjHandle)&rpCallListeners[i]);

                sprintf(buff, "%ld", objId);
                argList += (UtlString) buff;

                if (i < (size - 1))
                {
                        argList += TAOMESSAGE_DELIMITER;
                }
        }

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_TERMINAL,
                                                                        TaoMessage::GET_CALL_LISTENERS,
                                                                        rMsg.getMsgID(),
                                                                        size,
                                                                        clientSocket,
                                                                        size,
                                                                        argList);

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoTerminalAdaptor::terminalGetComponent(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        rMsg.setMsgSubType(TaoMessage::RESPONSE_TERMINAL);
        rMsg.setCmd(TaoMessage::GET_COMPONENT);
        rMsg.setObjHandle((TaoObjHandle) this);

        // assert
        (mpPhoneTask->getComponent((PsMsg&) rMsg));

        return TAO_SUCCESS;
}

TaoStatus TaoTerminalAdaptor::terminalGetComponents(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        rMsg.setMsgSubType(TaoMessage::RESPONSE_TERMINAL);
        rMsg.setCmd(TaoMessage::GET_COMPONENTS);

    // assert
        (mpPhoneTask->getComponents((PsMsg&) rMsg));
        if (mpSvrTransport->postMessage(rMsg))
        {
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoTerminalAdaptor::terminalGetComponentGroups(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        rMsg.setMsgSubType(TaoMessage::RESPONSE_TERMINAL);
        rMsg.setCmd(TaoMessage::GET_COMPONENTGROUPS);

    // assert
        (mpPhoneTask->getComponentGroups((PsMsg&) rMsg));
        if (mpSvrTransport->postMessage(rMsg))
        {
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoTerminalAdaptor::returnResult(TaoMessage& rMsg)
{
        if (mpSvrTransport->postMessage(rMsg))
        {
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoTerminalAdaptor:: terminalGetConfig(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 0)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();

        TaoObjHandle rTerminal = 0x00000099;    // fake

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_TERMINAL,
                                                                        TaoMessage::GET_CONFIG,
                                                                        rMsg.getMsgID(),
                                                                        rTerminal,
                                                                        clientSocket,
                                                                        1,
                                                                        "302");

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoTerminalAdaptor::terminalGetDoNotDisturb(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 0)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();

        TaoObjHandle rTerminal = 0x00000099;    // fake

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_TERMINAL,
                                                                        TaoMessage::GET_DONOT_DISTURB,
                                                                        rMsg.getMsgID(),
                                                                        rTerminal,
                                                                        clientSocket,
                                                                        1,
                                                                        "0");

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoTerminalAdaptor::terminalGetName(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 0)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        TaoObjHandle objId = rMsg.getTaoObjHandle();
        TaoObjHandle pTerminal;
        UtlString        name;

        if (TAO_NOT_FOUND != mpObjectDb->findValue(objId, pTerminal))
        {
                if (pTerminal)
                {
                        // ((PtTerminal *) pTerminal)->getName((const char *&)name);
                }
        }

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_TERMINAL,
                                                                        TaoMessage::GET_NAME,
                                                                        rMsg.getMsgID(),
                                                                        objId,
                                                                        clientSocket,
                                                                        1,
                                                                        name);

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoTerminalAdaptor::terminalGetTermConnections(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        TaoString arg(rMsg.getArgList(), TAOMESSAGE_DELIMITER);
        UtlString callId = arg[0];
        UtlString address = arg[1];

        int numConnections = 0;
        mpCallMgrTask->getNumTerminalConnections(callId.data(),
                                                                                        address.data(),
                                                                                        numConnections);

        UtlString *terminalNames;
        terminalNames = new UtlString[numConnections];

        if (terminalNames)
        {
                int maxConnections = numConnections;
                mpCallMgrTask->getTerminalConnections(callId.data(),
                                                                                        address.data(),
                                                                                        maxConnections,
                                                                                        numConnections,
                                                                                        terminalNames);

                if (numConnections > maxConnections)
                        numConnections = maxConnections;

                callId += TAOMESSAGE_DELIMITER + address;
                for (int i = 0; i< numConnections; i++)
                {
                        callId += TAOMESSAGE_DELIMITER + terminalNames[i];
                        int isLocal = mpCallMgrTask->isTerminalConnectionLocal(callId.data(),
                                                                                        address.data(),
                                                                                        terminalNames[i].data());
                        char buff[128];
                        sprintf(buff, "%d", isLocal);
                        callId += TAOMESSAGE_DELIMITER + UtlString(buff);       // isLocal
                }

                rMsg.setMsgSubType(TaoMessage::RESPONSE_CONNECTION);
                rMsg.setArgCnt(2 * numConnections + 2);
                rMsg.setArgList(callId);

                delete[] terminalNames;

                if (mpSvrTransport->postMessage(rMsg))
                        return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoTerminalAdaptor::terminalGetTermListeners(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        TaoObjHandle objId = rMsg.getTaoObjHandle();
        TaoObjHandle pTerminal;

        int      size =  atoi(rMsg.getArgList());
//      int      nItems;

        if (TAO_NOT_FOUND != mpObjectDb->findValue(objId, pTerminal))
        {
                if (pTerminal)
                {
//                      ((PtTerminal *) pTerminal)->getTerminalListeners(rpTerminalListeners, size, nItems);
                }
        }

        UtlString argList;
    char buff[MAXIMUM_INTEGER_STRING_LENGTH];


//      int actual = ((size < nItems) ? size : nItems);

        for (int i = 0; i < size; i++)
        {
                TaoObjHandle objId = mpObjectCnt->add();

                sprintf(buff, "%ld", objId);
                argList += (UtlString) buff;

                if (i < (size - 1))
                {
                        argList += TAOMESSAGE_DELIMITER;
                }
        }

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_TERMINAL,
                                                                        TaoMessage::GET_TERM_LISTENERS,
                                                                        rMsg.getMsgID(),
                                                                        size,
                                                                        clientSocket,
                                                                        size,
                                                                        argList);

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoTerminalAdaptor::terminalGetProvider(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 0)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        TaoObjHandle objId = rMsg.getTaoObjHandle();
        TaoObjHandle pTerminal;

//      PtProvider      rProvider;

        if (TAO_NOT_FOUND != mpObjectDb->findValue(objId, pTerminal))
        {
                if (pTerminal)
                {
//                      ((PtTerminal *) pTerminal)->getProvider(rProvider);
                        objId = mpObjectCnt->add();
//                      mpObjectDb->insert((TaoObjHandle)objId, (TaoObjHandle)&rProvider);
                }
        }

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_TERMINAL,
                                                                        TaoMessage::GET_PROVIDER,
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

TaoStatus TaoTerminalAdaptor::terminalNumAddresses(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        UtlString thisTerminal = rMsg.getArgList();

        UtlString terminalName;
        terminalName.remove(0); // empty string

        UtlString rNextKey;
        UtlString rNextValue;
        int nItems = 0;

        while (OS_SUCCESS == mAddresses.getNext(terminalName, rNextKey, rNextValue))
        {
                terminalName = rNextKey;
                if (!thisTerminal.compareTo(rNextKey, UtlString::ignoreCase))
                {
                        nItems++;
                }
        }

    char buff[MAXIMUM_INTEGER_STRING_LENGTH];
        sprintf(buff, "%d", nItems);
        thisTerminal += TAOMESSAGE_DELIMITER + UtlString(buff);

        rMsg.setMsgSubType(TaoMessage::RESPONSE_TERMINAL);
        rMsg.setArgCnt(2);
        rMsg.setArgList(thisTerminal);

        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;

        return TAO_FAILURE;
}

TaoStatus TaoTerminalAdaptor::terminalNumCallListeners(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();

        TaoObjHandle rTerminal = 0x00000099;    // fake

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_TERMINAL,
                                                                        TaoMessage::NUM_CALL_LISTENERS,
                                                                        rMsg.getMsgID(),
                                                                        rTerminal,
                                                                        clientSocket,
                                                                        1,
                                                                        "2");

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoTerminalAdaptor::terminalNumComponents(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        rMsg.setMsgSubType(TaoMessage::RESPONSE_TERMINAL);
        rMsg.setCmd(TaoMessage::NUM_COMPONENTS);

        // assert
        (mpPhoneTask->numComponents((PsMsg&) rMsg));
        if (mpSvrTransport->postMessage(rMsg))
        {
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoTerminalAdaptor::terminalNumTermListeners(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();

        TaoObjHandle rTerminal = 0x00000099;    // fake

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_TERMINAL,
                                                                        TaoMessage::NUM_TERM_LISTENERS,
                                                                        rMsg.getMsgID(),
                                                                        rTerminal,
                                                                        clientSocket,
                                                                        1,
                                                                        "3");

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoTerminalAdaptor::terminalNumTermConnectionss(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        UtlString argList = rMsg.getArgList();
        TaoString arg(argList, TAOMESSAGE_DELIMITER);
        UtlString callId = arg[0];
        UtlString address = arg[0];

        int numConnections = 0;
        mpCallMgrTask->getNumTerminalConnections(callId.data(),
                                                                                        address.data(),
                                                                                        numConnections);

    char buff[MAXIMUM_INTEGER_STRING_LENGTH];

    sprintf(buff, "%d", numConnections);
        argList += TAOMESSAGE_DELIMITER + UtlString(buff);

        rMsg.setMsgSubType(TaoMessage::RESPONSE_TERMINAL);
        rMsg.setArgCnt(3);
        rMsg.setArgList(argList);

        if (mpSvrTransport->postMessage(rMsg))
                return TAO_SUCCESS;

        return TAO_FAILURE;
}

TaoStatus TaoTerminalAdaptor::terminalPickup(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();
        rMsg.getArgList();

        TaoObjHandle rTerminal = 0x00000099;    // fake

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_TERMINAL,
                                                                        TaoMessage::PICKUP,
                                                                        rMsg.getMsgID(),
                                                                        rTerminal,
                                                                        clientSocket,
                                                                        1,
                                                                        "501");

        if (mpSvrTransport->postMessage(*pMsg))
        {
                delete pMsg;
                return TAO_SUCCESS;
        }

        return TAO_FAILURE;
}

TaoStatus TaoTerminalAdaptor::terminalRemoveCallListener(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        UtlString arg = UtlString("0");
        if (mpListenerMgr)
        {
                argCnt = 0;
                mpListenerMgr->removeEventListener(rMsg);
        }
        else
        {
                argCnt = 1;
                arg = UtlString("-1");  // indicating dailure
        }

        TaoMessage*     pMsg = (TaoMessage*) &rMsg;

        pMsg->setMsgSubType(TaoMessage::RESPONSE_TERMINAL);
        pMsg->setArgCnt(argCnt);
        pMsg->setArgList(arg);

        if (mpSvrTransport->postMessage(*pMsg))
                return TAO_SUCCESS;

        return TAO_FAILURE;
}

TaoStatus TaoTerminalAdaptor::terminalRemoveTermListener(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 2)
                return TAO_FAILURE;

        UtlString arg = UtlString("0");
        if (mpListenerMgr)
        {
                argCnt = 0;
                mpListenerMgr->removeEventListener(rMsg);
        }
        else
        {
                argCnt = 1;
                arg = UtlString("-1");  // indicating dailure
        }

        TaoMessage*     pMsg = (TaoMessage*) &rMsg;

        pMsg->setMsgSubType(TaoMessage::RESPONSE_TERMINAL);
        pMsg->setArgCnt(argCnt);
        pMsg->setArgList(arg);

        if (mpSvrTransport->postMessage(*pMsg))
                return TAO_SUCCESS;

        return TAO_FAILURE;
}

TaoStatus TaoTerminalAdaptor::terminalSetDoNotDisturb(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();
        rMsg.getArgList();

        TaoObjHandle rTerminal = 0x00000099;    // fake

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_TERMINAL,
                                                                        TaoMessage::SET_DONOT_DISTURB,
                                                                        rMsg.getMsgID(),
                                                                        rTerminal,
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


TaoStatus TaoTerminalAdaptor::terminalSetCodecCPULimit(TaoMessage& rMsg)
{
        int     argCnt = rMsg.getArgCnt();

        if (argCnt != 1)
                return TAO_FAILURE;

        TaoObjHandle clientSocket = rMsg.getSocket();
        rMsg.getTaoObjHandle();
        int level = atoi(rMsg.getArgList());
        TaoObjHandle rTerminal = 0x00000099;    // fake

    mpCallMgrTask->setInboundCodecCPULimit(level) ;

        TaoMessage*     pMsg = new TaoMessage(TaoMessage::RESPONSE_TERMINAL,
                                                                        TaoMessage::SET_INBOUND_CODEC_CPU_LIMIT,
                                                                        rMsg.getMsgID(),
                                                                        rTerminal,
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
