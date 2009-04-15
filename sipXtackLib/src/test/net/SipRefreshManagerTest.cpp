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

    static SipRefreshManager::RefreshRequestState smLatestSubState;
    static int smCallbackCount;
    static int smLastResponseCode;
    static long smExpiration;

    UtlBoolean respond(OsMsgQ& messageQueue,
                       int responseCode, 
                       const char* responseText, 
                       SipUserAgent& userAgent,
                       int waitMilliSeconds,
                       SipMessage*& request)
    {
        UtlBoolean gotRequest = FALSE;
        request = NULL;
        if(removeMessage(messageQueue, waitMilliSeconds, request) &&
            request)
        {
            if(!request->isResponse())
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
        return(gotRequest);
    }

    UtlBoolean removeMessage(OsMsgQ& messageQueue,
                             int waitMilliSeconds,
                             const SipMessage*& message)
    {
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
                message = ((SipMessageEvent*)osMessage)->getMessage();
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
        return(gotMessage);
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
        printf("subStateCallback \n\trequestState: %d\n\tearlyDialogHandle: %s\n\tdialogHandle: %s\n\tapplicationData: %p\n\tresponseCode: %d\n\tresponseText: %s\n\texpiration: %d\n\tresponse: %p\n\tcallback count: %d\n",
            requestState, 
            earlyDialogHandle ? earlyDialogHandle : "",
            dialogHandle ? dialogHandle : "",
            applicationData,
            responseCode,
            responseText ? responseText : "",
            expiration,
            response,
            smCallbackCount);
    }




    void refreshTest()
    {
        UtlString aor("sip:111@127.0.0.1:");
        UtlString eventTypeKey("message-summary");
        UtlString eventType(eventTypeKey);
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
        int expPeriod = 100;
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

        SipUserAgent userAgent(UNIT_TEST_SIP_PORT, UNIT_TEST_SIP_PORT);
        userAgent.start();

        int transactionTimeoutPeriod = 
            userAgent.getSipStateTransactionTimeout() / 1000;
        int errorRefreshPeriod = expPeriod * 0.1;
        int normalRefreshPeriod = expPeriod * 0.55;
        if(errorRefreshPeriod < transactionTimeoutPeriod)
        {
            errorRefreshPeriod = transactionTimeoutPeriod;
        }
        if(normalRefreshPeriod < transactionTimeoutPeriod)
        {
            normalRefreshPeriod = transactionTimeoutPeriod;
        }

        // Set up the refresh manager
        SipDialogMgr clientDialogMgr;
        SipRefreshManager refreshMgr(userAgent, clientDialogMgr);
        refreshMgr.start();


        //CPPUNIT_ASSERT(TRUE);
        //ASSERT_STR_EQUAL("a", "a");

        // Create a crude Subscription server/observer
        OsMsgQ incomingServerMsgQueue;
        // Register an interest in SUBSCRIBE requests 
        // for this event type
        userAgent.addMessageObserver(incomingServerMsgQueue,
                                    SIP_SUBSCRIBE_METHOD,
                                    TRUE, // requests
                                    FALSE, // no reponses
                                    TRUE, // incoming
                                    FALSE, // no outgoing
                                    eventType,
                                    NULL,
                                    NULL);


        // Send a request
        UtlString earlyDialogHandle;
        long start = OsDateTime::getSecsSinceEpoch();

        CPPUNIT_ASSERT(refreshMgr.initiateRefresh(mwiSubscribeRequest,
                                                    this,
                                                    subStateCallback,
                                                    earlyDialogHandle));

        // Wait for the request and send a response
        SipMessage* initialRequest = NULL;
        CPPUNIT_ASSERT(respond(incomingServerMsgQueue, 
                        202, // response code
                        "Got request and accepted",
                        userAgent,
                        5000, // milliseconds to wait for request
                        initialRequest));

        // Wait a little to let the response get through the works
        int waitIterations = 0;
        while(smCallbackCount == 0)
        {
            OsTask::delay(100);
            waitIterations++;
            if(waitIterations >= 50)
            {
                break;
            }
        }

        printf("expiration: %d lag: %d\n", 
            smExpiration, 
            smExpiration - start - expPeriod);
        CPPUNIT_ASSERT(initialRequest);
        CPPUNIT_ASSERT(smLastResponseCode == 202);
        CPPUNIT_ASSERT(smLatestSubState == SipRefreshManager::REFRESH_REQUEST_SUCCEEDED);
        CPPUNIT_ASSERT(smCallbackCount == 1);
        CPPUNIT_ASSERT(smExpiration >= start + expPeriod);
        CPPUNIT_ASSERT(smExpiration < start + expPeriod + 5); // allow 5 sec. variance
        long firstExpiration = smExpiration;

        // Wait for the refresh
        printf("waiting for refresh in %d seconds\n", normalRefreshPeriod);
        SipMessage* firstRefresh;
        CPPUNIT_ASSERT(respond(incomingServerMsgQueue, 
                        203, // response code
                        "Got request and accepted",
                        userAgent,
                        expPeriod * 1000, // milliseconds to wait for request
                        firstRefresh));
        long firstRefreshAt = OsDateTime::getSecsSinceEpoch();
        CPPUNIT_ASSERT(firstRefresh);
        CPPUNIT_ASSERT(firstRefreshAt - start >= 32); // min transaction period
        CPPUNIT_ASSERT(firstRefreshAt <= start + expPeriod + 5);

        // Wait for the response and callback
        waitIterations = 0;
        while(smCallbackCount == 1)
        {
            OsTask::delay(100);
            waitIterations++;
            if(waitIterations >= 50)
            {
                break;
            }
        }
        long secondExpiration = smExpiration;
        printf("real refresh period: %d expires at: %d\n", firstRefreshAt - start, smExpiration);
        CPPUNIT_ASSERT(smCallbackCount == 2);
        CPPUNIT_ASSERT(smLatestSubState == SipRefreshManager::REFRESH_REQUEST_SUCCEEDED);
        CPPUNIT_ASSERT(smExpiration + 5 >= firstExpiration + firstRefreshAt - start);
        CPPUNIT_ASSERT(smLastResponseCode == 203);

        // This time do not respond and confirm that the expiration
        // still stands
        SipMessage* secondRefresh = NULL;
        printf("waiting for refresh in %d seconds\n", normalRefreshPeriod);
        CPPUNIT_ASSERT(removeMessage(incomingServerMsgQueue,
                                     expPeriod * 1000, // milliseconds to wait for request
                                     secondRefresh));

        long secondRefreshAt = OsDateTime::getSecsSinceEpoch();

        CPPUNIT_ASSERT(secondRefresh);
        // Wait for the timeout response and callback
        waitIterations = 0;
        printf("waiting for callback count to be 3, currently: %d\n", 
                smCallbackCount);
        while(smCallbackCount <= 2)
        {
            OsTask::delay(1000);
            waitIterations++;
            if(waitIterations >= 60) //(transactionTimeoutPeriod / 1000) + 5) 
            {
                break;
            }
        }

        printf("callback count is: %d\n", smCallbackCount);

        // The expiration should not change on the client side as
        // the transaction failed (timed out due to no response);
        CPPUNIT_ASSERT(smCallbackCount == 3);
        CPPUNIT_ASSERT(smLatestSubState == SipRefreshManager::REFRESH_REQUEST_FAILED);
        CPPUNIT_ASSERT(smExpiration + 5 >= firstExpiration + firstRefreshAt - start);
        CPPUNIT_ASSERT(smLastResponseCode == SIP_REQUEST_TIMEOUT_CODE);
        CPPUNIT_ASSERT(secondRefreshAt - firstRefreshAt < normalRefreshPeriod + 5);

        // Empty the queue of the resends for the prior transaction that
        // we did not respond to.
        SipMessage* dupSecondRefresh = NULL;
        long startDupRemoval = OsDateTime::getSecsSinceEpoch();
        for(int duplicateRequestCount = 0; duplicateRequestCount < 3; duplicateRequestCount++)
        {
            removeMessage(incomingServerMsgQueue,
                                 50, // milliseconds to wait for request
                                 dupSecondRefresh);
            if(dupSecondRefresh)
            {
                int dupCseq;
                dupSecondRefresh->getCSeqField(&dupCseq, NULL);
                printf("Removed duplicate request Cseq: %d\n", dupCseq);
                if(dupCseq > 3) break;
            }
        }
        long endDupRemoval = OsDateTime::getSecsSinceEpoch();
        printf("duplicate removal started at %d lasted: %d\n",
            startDupRemoval, endDupRemoval - startDupRemoval);

        // The next refresh should be sooner as the prior request
        // failed
        printf("waiting for refresh in %d seconds\n", errorRefreshPeriod);
        SipMessage* thirdRefresh = NULL;
        CPPUNIT_ASSERT(respond(incomingServerMsgQueue, 
                                204, // response code
                                "Got request and accepted",
                                userAgent,
                                expPeriod * 1000, // milliseconds to wait for request
                                thirdRefresh));
        long thirdRefreshAt = OsDateTime::getSecsSinceEpoch();
        printf("actual refresh period: %d\n", thirdRefreshAt - secondRefreshAt);
        CPPUNIT_ASSERT(thirdRefreshAt - secondRefreshAt <= errorRefreshPeriod + 5);

        // Wait for the response and callback
        waitIterations = 0;
        while(smCallbackCount == 3)
        {
            OsTask::delay(1000);
            waitIterations++;
            if(waitIterations >= 500) 
            {
                break;
            }
        }

        CPPUNIT_ASSERT(smCallbackCount == 4);
        CPPUNIT_ASSERT(smLatestSubState == SipRefreshManager::REFRESH_REQUEST_SUCCEEDED);
        CPPUNIT_ASSERT(smExpiration + 5 >= secondExpiration + normalRefreshPeriod);
        CPPUNIT_ASSERT(smLastResponseCode == 204);

        userAgent.removeMessageObserver(incomingServerMsgQueue);
        refreshMgr.requestShutdown();

    }


};

SipRefreshManager::RefreshRequestState SipRefreshManagerTest::smLatestSubState = SipRefreshManager::REFRESH_REQUEST_UNKNOWN;
int SipRefreshManagerTest::smCallbackCount = 0;
int SipRefreshManagerTest::smLastResponseCode = -2;
long SipRefreshManagerTest::smExpiration = -2;


CPPUNIT_TEST_SUITE_REGISTRATION(SipRefreshManagerTest);
