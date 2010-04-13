//
// Copyright (C) 2007, 2010 Avaya, Inc., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
//////////////////////////////////////////////////////////////////////////////


// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "TestRegistrar.h"
#include "net/HttpMessage.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const int NUM_USERS = 4;
// STATIC VARIABLE INITIALIZATIONS
TestRegistrarUsers gUsers[] =
{
    {"mike", "1234"},
    {"andy", "3456"},
    {"bob",  "5678"},
    {"anon", ""}
};


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
TestRegistrar::TestRegistrar()
    : OsServerTask("TestRegistrarServer", NULL, 2000)
{
    mpUserAgent = new  SipUserAgent(
                5070,                    // sipTcpPort
                5070,                    // sipUdpPort
                5071,                    // sipTlsPort
                NULL,                       // publicAddress
                NULL,                       // defaultUser
                "127.0.0.1",                       // defaultSipAddress
                NULL,                       // sipProxyServers
                NULL,                       // sipDirectoryServers
                NULL,                       // sipRegistryServers
                NULL,                       // authenicateRealm
                NULL,                       // authenticateDb
                NULL,                       // authorizeUserIds
                NULL,                       // authorizePasswords
                NULL,                       // lineMgr
                SIP_DEFAULT_RTT,            // sipFirstResendTimeout
                TRUE,                       // defaultToUaTransactions
                -1,                         // readBufferSize
                OsServerTask::DEF_MAX_MSGS, // queueSize
                false) ;                    // bUseNextAvailablePort
}

// Copy constructor
TestRegistrar::TestRegistrar(const TestRegistrar& rTestRegistrar)
{
}

// Destructor
TestRegistrar::~TestRegistrar()
{
    mpUserAgent->removeMessageObserver(*this->getMessageQueue()) ;
    mpUserAgent->shutdown() ;
    delete mpUserAgent ;

    waitUntilShutDown();
}

void TestRegistrar::init()
{
    mpUserAgent->addMessageObserver(*this->getMessageQueue(), SIP_REGISTER_METHOD, 1, 0, 1, 0);
    mpUserAgent->addMessageObserver(*this->getMessageQueue(), SIP_SUBSCRIBE_METHOD, 1, 0, 1, 0);
    mpUserAgent->start();
    while(!mpUserAgent->waitUntilReady())
    {
        OsTask::delay(100);
    }

    this->start();
}

UtlBoolean TestRegistrar::handleMessage(OsMsg& rMsg)
{
    int msgType = rMsg.getMsgType();
    int msgSubType = rMsg.getMsgSubType();
    UtlBoolean messageProcessed = FALSE;

    switch(msgType)
    {
        case OsMsg::PHONE_APP:
        {
            const SipMessage& message = *((SipMessageEvent&)rMsg).getMessage();
            UtlString method;

            if (!message.isResponse())
            {
                message.getRequestMethod(&method);

                if (SIP_REGISTER_METHOD == method)
                {
                    messageProcessed = handleRegisterRequest(message);
                }
            }
            break;
        }

    }
    return messageProcessed;
}
/* ============================ MANIPULATORS ============================== */

// Assignment operator
TestRegistrar&
TestRegistrar::operator=(const TestRegistrar& rhs)
{
    return *this;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

UtlBoolean TestRegistrar::handleRegisterRequest(SipMessage message)
{
    UtlBoolean messageProcessed = false;
    SipMessage finalResponse;
    UtlString responseToAddress;
    UtlString protocol;
    int responseToPort;
    int seqNum;
    UtlString method;
    UtlString contactField;
    int expires;
    static int retrySeqNum = 0;
    UtlString callId;

    message.getCallIdField(&callId);

    message.getContactField(0, contactField);
    message.getExpiresField(&expires);

    message.getCSeqField(&seqNum, &method);
    message.getFromAddress(&responseToAddress, &responseToPort, &protocol);

    finalResponse.setContactField(contactField);
    finalResponse.setExpiresField(expires);
    finalResponse.setFromField("sip:127.0.0.1:5070", 5070);
    finalResponse.setSendAddress(contactField, responseToPort);
    finalResponse.setToField(contactField, responseToPort, protocol);
    finalResponse.setUserAgentField("TestRegistrar");
//    finalResponse.setRegisterData(responseToAddress, contactField, "127.0.0.1", contactField, callId, seqNum);

    if (contactField.contains("anon"))      // anonymous user doesnt need authentication, just say ok
    {
        finalResponse.setResponseData(&message, 200, "OK");
    }
    else if (contactField.contains("mike"))  // mike requires registration, say 401
    {
        UtlString realm;

        UtlString requestUser;
        UtlString requestRealm;
        UtlString requestNonce;
        UtlString uriParam;

        // TBD - 25-jan-2010 work might be needed if these tests are re-enabled
        message.getDigestAuthorizationData(
                   &requestUser, &requestRealm, &requestNonce,
                   NULL, NULL, &uriParam,
                   NULL,  // TBD cnonce
                   NULL,  // TBD nonceCount
                   NULL,  // TBD qop
                   HttpMessage::SERVER, 0);

        if (seqNum == retrySeqNum) // if this is a retry response
        {
            // if they've sent any auth field, just accept it.
            // TODO:  figure out if a username and password has been encrypted and sent.
            finalResponse.setCSeqField(++seqNum, SIP_REGISTER_METHOD);
            finalResponse.setResponseData(&message, 200, "OK");
        }
        else
        {
            message.getCSeqField(&seqNum, &method);
            finalResponse.setCSeqField(++seqNum, method);
            retrySeqNum = seqNum;
#ifdef _WIN32
            finalResponse.setAuthenticateData("md5", "TestRegistrar", NULL, NULL, NULL, HttpMessage::HttpEndpointEnum::SERVER );
#else
            finalResponse.setAuthenticateData("md5", "TestRegistrar", NULL, NULL, NULL, HttpMessage::SERVER );
#endif
            finalResponse.setResponseData(&message, 401, "Not authorized");
        }
    }
    else if (contactField.contains("xyzzy"))
    {
        // this is our special username that will cause a response
        // to be echoed back with the 3 digit value after xyzzy.
        // for instance, the contact xyzzy401 will cause a 401 response
        int pos = contactField.first("xyzzy");
        char szCode[4];
        szCode[0] = contactField[pos + 5];
        szCode[1] = contactField[pos + 6];
        szCode[2] = contactField[pos + 7];
        szCode[3] = '\0';

        finalResponse.setResponseData(&message, atoi(szCode), "OK");
    }


    mpUserAgent->send(finalResponse);

    return messageProcessed;
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
