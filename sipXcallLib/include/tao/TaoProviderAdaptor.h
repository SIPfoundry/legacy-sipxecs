//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _TaoProviderAdaptor_h_
#define _TaoProviderAdaptor_h_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsConfigDb.h"
#include "tao/TaoAdaptor.h"
#include "tao/TaoObjectMap.h"
#include "tao/TaoReference.h"
#include "tao/TaoServerTask.h"
#include "os/OsDefs.h"
#include "os/OsBSem.h"
#include "os/OsServerTask.h"
#include "net/HttpServer.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class CpCallManager;
class MpMediaTask;
class OsNameDb;
class OsTimerTask;
class PsPhoneTask;
class UtlMemCheck;
class SipUserAgent;
class OsConfigDb;
class PtMGCP;
class HttpServer;

class OsConfigDb;
class TaoTransportTask;
class PtProvider;
class PtCall;

class TaoProviderAdaptor : public TaoAdaptor
{
friend class TaoServerTask;

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
/* ============================ CREATORS ================================== */
        TaoProviderAdaptor(const TaoProviderAdaptor& rTaoProviderAdaptor);
     //:Copy constructor (not implemented for this class)

/* ============================ MANIPULATORS ============================== */

        virtual UtlBoolean handleMessage(OsMsg& rMsg);
         //:Handle an incoming message.
         // If the message is not one that the object is prepared to process,
         // the handleMessage() method in the derived class should return FALSE
         // which will cause the OsServerTask::handleMessage() method to be
         // invoked on the message.

//      virtual void parseMessage(TaoMessage& rMsg);
         //:Parse the incoming message.

        TaoStatus providerGetProvider(TaoMessage& rMsg);

        TaoStatus providerGetAddress(TaoMessage& rMsg);

        TaoStatus providerGetAddresses(TaoMessage& rMsg);

        TaoStatus providerNumAddresses(TaoMessage& rMsg);

        TaoStatus providerGetTerminal(TaoMessage& rMsg);

        TaoStatus providerGetTerminals(TaoMessage& rMsg);

        TaoStatus providerNumTerminals(TaoMessage& rMsg);

        TaoStatus providerCreateCall(TaoMessage& rMsg);
        TaoStatus getCreateCall(TaoMessage& rMsg);

        TaoStatus providerGetCalls(TaoMessage& rMsg);

        TaoStatus providerNumCalls(TaoMessage& rMsg);

        TaoStatus providerGetState(TaoMessage& rMsg);

        TaoStatus providerGetProviderListeners(TaoMessage& rMsg);

        TaoStatus providerNumProviderListeners(TaoMessage& rMsg);

        TaoStatus providerGetConnection(TaoMessage& rMsg);

        TaoStatus providerGetTermConnection(TaoMessage& rMsg);


        TaoStatus providerAddProviderListener(TaoMessage& rMsg);
         //:Adds a listener to a PtCall object when this TaoObjHandle for PtAddress object first
         // becomes part of that PtCall.
         //!param: (in) hAddress - a TaoObjHandle representing the PtAddress object
         //!param: (in) rhCallListener - the listener to add to calls associated with this address
         //!retcode: TAO_SUCCESS - success
         //!retcode: TAO_EXISTS - <i>rhCallListener</i> is already registered
         //!retcode: TAO_PROVIDER_UNAVAILABLE - the provider is not available

        TaoStatus providerRemoveProviderListener(TaoMessage& rMsg);
         //:Removes the indicated PtCallListener from this PtAddress TaoObjHandle.
         // This method removes a PtCallListener which was added via the
         // addCallListener() method. If successful, the listener will
         // no longer be added to new calls which are presented to this address,
         // however it does not affect PtCallListeners which have already been
         // added to a call.
         //!param: (in) rhCallListener - the listener to remove
         //!retcode: TAO_SUCCESS - success
         //!retcode: TAO_NOT_FOUND - <i>rhCallListener</i> not registered
         //!retcode: TAO_PROVIDER_UNAVAILABLE - the provider is not available

        TaoStatus providerShutdown(TaoMessage& rMsg);

protected:
        void startAdaptor();
   // initialize the SIP user agent
/*   virtual void initSipUserAgent(OsConfigDb* config);

   // initialize the MGCP stack
   virtual void initMgcpStack(OsConfigDb* config);

        // init the call processing subsystem
        virtual void initCallManager(OsConfigDb* config);

        // Set up the configuration database from the default file name
        virtual void initConfigFile(OsConfigDb* config);
*/
private:
        TaoTransportTask*       mpSvrTransport;

        TaoObjectMap*           mpObjectDb;
        TaoReference*           mpObjectCnt;
        TaoObjectMap*           mpCallDb;
        TaoReference*           mpCallCnt;
        TaoObjectMap*           mpProviderListenerDb;
        TaoReference*           mpProviderListenerCnt;

        PtProvider*                     mpProvider;
        PtCall                          *mpCall;
        int                                     mState;

        CpCallManager*    mpCallMgrTask; // call manager task
        MpMediaTask*      mpMediaTask;   // media processing task
        PsPhoneTask*      mpPhoneTask;   // phone set task
        OsTimerTask*      mpTimerTask;   // timer manager task
        SipUserAgent*     mpSipUserAgentTask; // sip stack
        PtMGCP*           mpMgcpStackTask; // MGCP stack
        HttpServer*       mpHttpServer;  // Http Server
        UtlString          mTimeServer;   // primary time server name

        TaoProviderAdaptor( CpCallManager* pCallMgr,
                                                TaoTransportTask*& rpSvrTransport,
                                                TaoMessage& rMsg,
                                                const UtlString& name = "TaoProviderAdaptor",
                                                const int maxRequestQMsgs = 60);
        //:Constructor

        virtual ~TaoProviderAdaptor();


};

#endif // _TaoProviderAdaptor_h_
