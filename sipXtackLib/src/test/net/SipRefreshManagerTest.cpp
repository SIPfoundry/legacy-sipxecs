// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>
#include <utl/UtlHashMap.h>
#include <utl/UtlString.h>
#include <os/OsDefs.h>
#include <os/OsDateTime.h>
#include <net/SipDialog.h>
#include <net/SipMessage.h>
#include <net/SipUserAgent.h>
#include <net/SipDialogMgr.h>
#include <net/SipRefreshManager.h>

#define UNIT_TEST_SIP_PORT 44544

/**
 * Unittest for SipRefreshManager
 */

class SipRefreshManagerTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(SipRefreshManagerTest);
    CPPUNIT_TEST(refreshTest);
    CPPUNIT_TEST_SUITE_END();

    public:

    // Initializations of these variables are at the bottom of this file.
    static SipRefreshManager::RefreshRequestState smLatestSubState;
    static int smCallbackCount;
    static int smLastResponseCode;
    static long smExpiration;

    UtlBoolean removeMessage(OsMsgQ& messageQueue,
                             int waitMilliSeconds,
                             const SipMessage*& message)
    {
#ifdef TEST_PRINT
        printf("removeMessage() entered, wait = %d\n", waitMilliSeconds);
#endif
        UtlBoolean gotMessage = FALSE;
        message = NULL;
        OsTime messageTimeout(0, waitMilliSeconds * 1000);
        OsMsg* osMessage = NULL;
        messageQueue.receive(osMessage, messageTimeout);
        if(osMessage)
        {
            int msgType = osMessage->getMsgType();
            int msgSubType = osMessage->getMsgSubType();
            int messageType = ((SipMessageEvent*)osMessage)->getMessageStatus();
            if(msgType == OsMsg::PHONE_APP &&
               msgSubType == SipMessage::NET_SIP_MESSAGE &&
               messageType == SipMessageEvent::APPLICATION)
            {
                message =
                   (dynamic_cast <SipMessageEvent*> (osMessage))->getMessage();
                gotMessage = TRUE;

#ifdef TEST_PRINT
                if(message)
                {
                    UtlString messageBytes;
                    ssize_t len;
                    message->getBytes(&messageBytes, &len);
                    printf("%s", messageBytes.data());
                }
                else
                {
                    printf("removeMessage: messageBytes: <null>\n");
                }
#endif
            }
        }
#ifdef TEST_PRINT
        printf("removeMessage() exited, gotMessage = %d\n", gotMessage);
#endif
        return(gotMessage);
    }

    UtlBoolean respond(OsMsgQ& messageQueue,
                       int responseCode, 
                       const char* responseText, 
                       SipUserAgent& userAgent,
                       int waitMilliSeconds,
                       const SipMessage*& request)
    {
#ifdef TEST_PRINT
       printf("respond() entered, wait = %d\n", waitMilliSeconds);
#endif
       UtlBoolean gotRequest = FALSE;
       request = NULL;
       if (removeMessage(messageQueue, waitMilliSeconds, request) &&
           request)
       {
          if (!request->isResponse())
          {
             gotRequest = TRUE;
             SipMessage response;
             response.setResponseData(request, responseCode, responseText);
             userAgent.send(response);

#ifdef TEST_PRINT
             UtlString responseBytes;
             ssize_t len;
             response.getBytes(&responseBytes, &len);
             printf("%s", responseBytes.data());
#endif

          }
       }
#ifdef TEST_PRINT
       printf("respond() exited, gotRequest = %d\n", gotRequest);
#endif
       return gotRequest;
    }

    static void subStateCallback(SipRefreshManager::RefreshRequestState requestState,
                                 const char* earlyDialogHandle,
                                 const char* dialogHandle,
                                 void* applicationData,
                                 int responseCode,
                                 const char* responseText,
                                 long expiration, // epoch seconds
                                 const SipMessage* response)
    {
        smLatestSubState = requestState;
        smLastResponseCode = responseCode;
        smExpiration = expiration;
        smCallbackCount++;
#ifdef TEST_PRINT
        printf("subStateCallback smCallbackCount = %d\n\trequestState: %s\n\tearlyDialogHandle: '%s'\n\tdialogHandle: '%s'\n\tapplicationData: %p\n\tresponseCode: %d\n\tresponseText: %s\n\texpiration: %ld\n\tresponse: %p\n",
               smCallbackCount,
               SipRefreshManager::refreshRequestStateText(requestState), 
               earlyDialogHandle ? earlyDialogHandle : "",
               dialogHandle ? dialogHandle : "",
               applicationData,
               responseCode,
               responseText ? responseText : "",
               expiration,
               response);
#endif
    }

    // setUp() and tearDown() are needed so that we can disconnect
    // incomingServerMsgQueue from userAgent before destroying
    // those objects.

    UtlString eventTypeKey;
    UtlString eventType;
    SipUserAgent* userAgentp;
    OsMsgQ incomingServerMsgQueue;
    SipDialogMgr clientDialogMgr;
    SipRefreshManager* refreshManagerp;

    void setUp()
    {
       eventTypeKey = "message-summary";
       eventType = eventTypeKey;

       UtlString hostIp = "127.0.0.1";
       userAgentp = new SipUserAgent(UNIT_TEST_SIP_PORT, UNIT_TEST_SIP_PORT,
                                     PORT_NONE,
                                     hostIp, NULL, hostIp);
       userAgentp->start();

       // Create a crude Subscription server/observer

       // Register an interest in SUBSCRIBE requests 
       // for this event type
       userAgentp->addMessageObserver(incomingServerMsgQueue,
                                      SIP_SUBSCRIBE_METHOD,
                                      TRUE, // requests
                                      FALSE, // no reponses
                                      TRUE, // incoming
                                      FALSE, // no outgoing
                                      eventType,
                                      NULL,
                                      NULL);

       // Set up the refresh manager
       refreshManagerp = new SipRefreshManager(*userAgentp, clientDialogMgr);
       refreshManagerp->start();
    }

    void tearDown()
    {
       userAgentp->removeMessageObserver(incomingServerMsgQueue);
    }

    void refreshTest()
    {
        UtlString aor("sip:111@127.0.0.1:");
        UtlString from("Frida<sip:111@localhost:");
        UtlString to("Tia<sip:222@localhost:");
        UtlString contact("sip:111@127.0.0.1:");
        char portString[20];
        sprintf(portString, "%d", UNIT_TEST_SIP_PORT);
        aor.append(portString);
        from.append(portString);
        from.append(">;tag=kadhsflasjkdfh");
        to.append(portString);
        to.append('>');
        contact.append(portString);
        int expPeriod = 10;
        SipMessage mwiSubscribeRequest;
        mwiSubscribeRequest.setSipRequestFirstHeaderLine(SIP_SUBSCRIBE_METHOD, 
                                                         aor, 
                                                         SIP_PROTOCOL_VERSION);
        mwiSubscribeRequest.setContactField(aor);
        mwiSubscribeRequest.setRawFromField(from);
        mwiSubscribeRequest.setRawToField(to);
        mwiSubscribeRequest.setEventField(eventType);
        mwiSubscribeRequest.setCallIdField("123456");
        mwiSubscribeRequest.setCSeqField(1, SIP_SUBSCRIBE_METHOD);
        mwiSubscribeRequest.setExpiresField(expPeriod);

        ssize_t len;
        UtlString requestDump;
        mwiSubscribeRequest.getBytes(&requestDump, &len);

        int transactionTimeoutPeriod = 
            userAgentp->getSipStateTransactionTimeout() / 1000;
        int errorRefreshPeriod = (int) (expPeriod * 0.1);
        int normalRefreshPeriod = (int) (expPeriod * 0.65);
        if (errorRefreshPeriod < transactionTimeoutPeriod)
        {
            errorRefreshPeriod = transactionTimeoutPeriod;
        }
        if (normalRefreshPeriod < transactionTimeoutPeriod)
        {
            normalRefreshPeriod = transactionTimeoutPeriod;
        }

        // Send a request
        UtlString earlyDialogHandle;
        long start = OsDateTime::getSecsSinceEpoch();

        CPPUNIT_ASSERT(refreshManagerp->initiateRefresh(mwiSubscribeRequest,
                                                        this,
                                                        subStateCallback,
                                                        earlyDialogHandle));

        // Wait for the request and send a response
        const SipMessage* initialRequest = NULL;
        CPPUNIT_ASSERT(respond(incomingServerMsgQueue, 
                               202, // response code
                               "Got request and accepted",
                               *userAgentp,
                               5000, // milliseconds to wait for request
                               initialRequest));

        // Wait a little to let the response get through the works
        for (int waitIterations = 0;
             smCallbackCount < 2 && waitIterations < 50;
             waitIterations++)
        {
            OsTask::delay(100);
        }

#ifdef TEST_PRINT
        printf("expiration: %ld from now: %ld lag: %ld\n", 
               smExpiration, 
               smExpiration - OsDateTime::getSecsSinceEpoch(),
               smExpiration - start - expPeriod);
#endif
        CPPUNIT_ASSERT(initialRequest);
        CPPUNIT_ASSERT(smLastResponseCode == 202);
        CPPUNIT_ASSERT(smLatestSubState == SipRefreshManager::REFRESH_REQUEST_SUCCEEDED);
        CPPUNIT_ASSERT(smCallbackCount == 2);
        CPPUNIT_ASSERT(smExpiration >= start + expPeriod);
        CPPUNIT_ASSERT(smExpiration < start + expPeriod + 5); // allow 5 sec. variance
        long firstExpiration = smExpiration;

        // Wait for the refresh
        printf("waiting for refresh in %d seconds\n", normalRefreshPeriod);
        const SipMessage* firstRefresh;
        CPPUNIT_ASSERT(respond(incomingServerMsgQueue, 
                               203, // response code
                               "Got request and accepted",
                               *userAgentp,
                               expPeriod * 1000, // milliseconds to wait for request
                               firstRefresh));
        long firstRefreshAt = OsDateTime::getSecsSinceEpoch();
        CPPUNIT_ASSERT(firstRefresh);
        CPPUNIT_ASSERT(firstRefreshAt - start >= normalRefreshPeriod - 5); // min transaction period
        CPPUNIT_ASSERT(firstRefreshAt <= start + expPeriod + 5);

        // Wait for the response and callback
        for (int waitIterations = 0;
             smCallbackCount < 3 && waitIterations < 50;
             waitIterations++)
        {
            OsTask::delay(100);
        }

#ifdef TEST_PRINT
        printf("real refresh period: %ld expires at: %ld from now: %ld\n",
               firstRefreshAt - start,
               smExpiration,
               smExpiration - OsDateTime::getSecsSinceEpoch());
#endif
        CPPUNIT_ASSERT(smCallbackCount == 3);
        CPPUNIT_ASSERT(smLatestSubState == SipRefreshManager::REFRESH_REQUEST_SUCCEEDED);
        CPPUNIT_ASSERT(smExpiration + 5 >= firstExpiration + firstRefreshAt - start);
        CPPUNIT_ASSERT(smLastResponseCode == 203);
    }
};

SipRefreshManager::RefreshRequestState SipRefreshManagerTest::smLatestSubState = SipRefreshManager::REFRESH_REQUEST_UNKNOWN;
int SipRefreshManagerTest::smCallbackCount = 0;
int SipRefreshManagerTest::smLastResponseCode = -2;
long SipRefreshManagerTest::smExpiration = -2;


CPPUNIT_TEST_SUITE_REGISTRATION(SipRefreshManagerTest);
