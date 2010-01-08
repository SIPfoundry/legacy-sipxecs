//
//
// Copyright (C) 2009 Nortel Networks, certain elements licensed under a Contributor Agreement.
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// Get value of EXECUTE_SLOW_TESTS.
#include "config.h"

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
#ifdef EXECUTE_SLOW_TESTS
    CPPUNIT_TEST(refreshTestBasic);
    CPPUNIT_TEST(refreshTestTransientRetry);
    CPPUNIT_TEST(refreshTestTransientRetryAndFail);
#endif // EXECUTE_SLOW_TESTS
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
            int messageType =
               (dynamic_cast <SipMessageEvent*> (osMessage))->getMessageStatus();
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
             int expires;
             request->getExpiresField(&expires);
             response.setExpiresField(expires);
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
       // Initialize the variables that hold the values from the last
       // callback.
       smLatestSubState = SipRefreshManager::REFRESH_REQUEST_UNKNOWN;
       smCallbackCount = 0;
       smLastResponseCode = -2;
       smExpiration = -2;

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

       refreshManagerp->stopAllRefreshes();
       delete refreshManagerp;

       userAgentp->shutdown(TRUE);
       delete userAgentp;
    }

    void refreshTestBasic()
    {
        // Construct the SUBSCRIBE request.
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
        SipMessage* mwiSubscribeRequest = new SipMessage;
        mwiSubscribeRequest->setSipRequestFirstHeaderLine(SIP_SUBSCRIBE_METHOD,
                                                          aor,
                                                          SIP_PROTOCOL_VERSION);
        mwiSubscribeRequest->setContactField(aor);
        mwiSubscribeRequest->setRawFromField(from);
        mwiSubscribeRequest->setRawToField(to);
        mwiSubscribeRequest->setEventField(eventType);
        mwiSubscribeRequest->setCallIdField("123456");
        mwiSubscribeRequest->setCSeqField(1, SIP_SUBSCRIBE_METHOD);
        mwiSubscribeRequest->setExpiresField(expPeriod);

        ssize_t len;
        UtlString requestDump;
        mwiSubscribeRequest->getBytes(&requestDump, &len);
#ifdef TEST_PRINT
        printf("%s", requestDump.data());
#endif

        // Calculate the expected refresh period.
        int transactionTimeoutPeriod =
            userAgentp->getSipStateTransactionTimeout() / 1000;
        int normalRefreshPeriod = (int) (expPeriod * 0.65);
        if (normalRefreshPeriod < transactionTimeoutPeriod)
        {
            normalRefreshPeriod = transactionTimeoutPeriod;
        }

        // Call initiateRefresh to send the SUBSCRIBE.
        UtlString earlyDialogHandle;
        long start = OsDateTime::getSecsSinceEpoch();
#ifdef TEST_PRINT
        printf("now: %ld\n", OsDateTime::getSecsSinceEpoch());
#endif
        CPPUNIT_ASSERT(refreshManagerp->initiateRefresh(mwiSubscribeRequest,
                                                        this,
                                                        subStateCallback,
                                                        earlyDialogHandle));

        // Wait for the request and send a response
        // Use response codes starting with 250, so that we can verify that
        // the callback receives the information from the SUBSCRIBE response.
        // Receive SUBSCRIBE, send 252 response.
        // Since the test framework doesn't send NOTIFY, we don't need machinery
        // to receive NOTIFY.
        const SipMessage* initialRequest = NULL;
        CPPUNIT_ASSERT(respond(incomingServerMsgQueue,
                               252, // response code
                               "Got request and accepted",
                               *userAgentp,
                               1000, // milliseconds to wait for request
                               initialRequest));

        // Wait for the callback when SipRefreshManager receives the 252.
        OsTask::delay(200);     // 200 msec

#ifdef TEST_PRINT
        printf("now: %ld\n", OsDateTime::getSecsSinceEpoch());
        printf("expiration: %ld from now: %ld lag: %ld\n",
               smExpiration,
               smExpiration - OsDateTime::getSecsSinceEpoch(),
               (smExpiration - start) - expPeriod);
#endif
        CPPUNIT_ASSERT(initialRequest);
        CPPUNIT_ASSERT_EQUAL(2, smCallbackCount);
        CPPUNIT_ASSERT_EQUAL(252, smLastResponseCode);
        CPPUNIT_ASSERT_EQUAL(SipRefreshManager::REFRESH_REQUEST_SUCCEEDED, smLatestSubState);
        CPPUNIT_ASSERT(smExpiration >= start + expPeriod - 1); // allow 1 sec. variance
        CPPUNIT_ASSERT(smExpiration <= start + expPeriod + 1);

        // Wait for SipRefreshManager to send the refresh SUBSCRIBE.
        printf("waiting for refresh in %d seconds\n", normalRefreshPeriod);
        const SipMessage* firstRefresh;
        CPPUNIT_ASSERT(respond(incomingServerMsgQueue,
                               253, // response code
                               "Got request and accepted",
                               *userAgentp,
                               (normalRefreshPeriod + 1) * 1000, // msec
                               firstRefresh));
        long firstRefreshAt = OsDateTime::getSecsSinceEpoch();
        // Wait for the callback generated by the refresh SUBSCRIBE.
        OsTask::delay(200);     // 200 msec

#ifdef TEST_PRINT
        printf("now: %ld\n", OsDateTime::getSecsSinceEpoch());
        printf("expiration: %ld from now: %ld\n",
               smExpiration,
               smExpiration - OsDateTime::getSecsSinceEpoch());
#endif
        CPPUNIT_ASSERT(firstRefresh);
        CPPUNIT_ASSERT_EQUAL(3, smCallbackCount);
        CPPUNIT_ASSERT_EQUAL(253, smLastResponseCode);
        CPPUNIT_ASSERT_EQUAL(SipRefreshManager::REFRESH_REQUEST_SUCCEEDED, smLatestSubState);
        CPPUNIT_ASSERT(firstRefreshAt - start >= normalRefreshPeriod - 1); // allow 1 sec. variance
        CPPUNIT_ASSERT(firstRefreshAt - start <= normalRefreshPeriod + 1);
        CPPUNIT_ASSERT(smExpiration >= firstRefreshAt + expPeriod - 1); // allow 1 sec. variance
        CPPUNIT_ASSERT(smExpiration <= firstRefreshAt + expPeriod + 1);
   }

    void refreshTestTransientRetry()
    {
        // Construct the SUBSCRIBE request.
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
        SipMessage* mwiSubscribeRequest = new SipMessage;
        mwiSubscribeRequest->setSipRequestFirstHeaderLine(SIP_SUBSCRIBE_METHOD,
                                                          aor,
                                                          SIP_PROTOCOL_VERSION);
        mwiSubscribeRequest->setContactField(aor);
        mwiSubscribeRequest->setRawFromField(from);
        mwiSubscribeRequest->setRawToField(to);
        mwiSubscribeRequest->setEventField(eventType);
        mwiSubscribeRequest->setCallIdField("123456");
        mwiSubscribeRequest->setCSeqField(1, SIP_SUBSCRIBE_METHOD);
        mwiSubscribeRequest->setExpiresField(expPeriod);

        ssize_t len;
        UtlString requestDump;
        mwiSubscribeRequest->getBytes(&requestDump, &len);
#ifdef TEST_PRINT
        printf("%s", requestDump.data());
#endif

        // Calculate the expected refresh period.
        int transactionTimeoutPeriod =
            userAgentp->getSipStateTransactionTimeout() / 1000;
        int normalRefreshPeriod = (int) (expPeriod * 0.65);
        if (normalRefreshPeriod < transactionTimeoutPeriod)
        {
            normalRefreshPeriod = transactionTimeoutPeriod;
        }

        // Call initiateRefresh to send the SUBSCRIBE.
        UtlString earlyDialogHandle;
        long start = OsDateTime::getSecsSinceEpoch();
#ifdef TEST_PRINT
        printf("now: %ld\n", OsDateTime::getSecsSinceEpoch());
#endif
        CPPUNIT_ASSERT(refreshManagerp->initiateRefresh(mwiSubscribeRequest,
                                                        this,
                                                        subStateCallback,
                                                        earlyDialogHandle));

        // Wait for the request and send a response
        // Use response codes starting with 250, so that we can verify that
        // the callback receives the information from the SUBSCRIBE response.
        // Receive SUBSCRIBE, send 252 response.
        // Since the test framework doesn't send NOTIFY, we don't need machinery
        // to receive NOTIFY.
        const SipMessage* initialRequest = NULL;
        CPPUNIT_ASSERT(respond(incomingServerMsgQueue,
                               252, // response code
                               "Got request and accepted",
                               *userAgentp,
                               1000, // milliseconds to wait for request
                               initialRequest));

        // Wait for the callback when SipRefreshManager receives the 252.
        OsTask::delay(200);     // 200 msec

#ifdef TEST_PRINT
        printf("now: %ld\n", OsDateTime::getSecsSinceEpoch());
        printf("expiration: %ld from now: %ld lag: %ld\n",
               smExpiration,
               smExpiration - OsDateTime::getSecsSinceEpoch(),
               (smExpiration - start) - expPeriod);
#endif
        CPPUNIT_ASSERT(initialRequest);
        CPPUNIT_ASSERT_EQUAL(2, smCallbackCount);
        CPPUNIT_ASSERT_EQUAL(252, smLastResponseCode);
        CPPUNIT_ASSERT_EQUAL(SipRefreshManager::REFRESH_REQUEST_SUCCEEDED, smLatestSubState);
        CPPUNIT_ASSERT(smExpiration >= start + expPeriod - 1); // allow 1 sec. variance
        CPPUNIT_ASSERT(smExpiration <= start + expPeriod + 1);
        long initialExpiration = smExpiration;

        // Wait for SipRefreshManager to send the refresh SUBSCRIBE.
        printf("waiting for refresh in %d seconds\n", normalRefreshPeriod);
        const SipMessage* firstRefresh;
        CPPUNIT_ASSERT(respond(incomingServerMsgQueue,
                               253, // response code
                               "Got request and accepted",
                               *userAgentp,
                               (normalRefreshPeriod + 1) * 1000, // msec
                               firstRefresh));
        long firstRefreshAt = OsDateTime::getSecsSinceEpoch();
        // Wait for the callback generated by the refresh SUBSCRIBE.
        OsTask::delay(200);     // 200 msec

#ifdef TEST_PRINT
        printf("now: %ld\n", OsDateTime::getSecsSinceEpoch());
        printf("expiration: %ld from now: %ld\n",
               smExpiration,
               smExpiration - OsDateTime::getSecsSinceEpoch());
#endif
        CPPUNIT_ASSERT(firstRefresh);
        CPPUNIT_ASSERT_EQUAL(3, smCallbackCount);
        CPPUNIT_ASSERT_EQUAL(253, smLastResponseCode);
        CPPUNIT_ASSERT_EQUAL(SipRefreshManager::REFRESH_REQUEST_SUCCEEDED, smLatestSubState);
        CPPUNIT_ASSERT(firstRefreshAt - start >= normalRefreshPeriod - 1); // allow 1 sec. variance
        CPPUNIT_ASSERT(firstRefreshAt - start <= normalRefreshPeriod + 1);
        CPPUNIT_ASSERT(smExpiration >= firstRefreshAt + expPeriod - 1); // allow 1 sec. variance
        CPPUNIT_ASSERT(smExpiration <= firstRefreshAt + expPeriod + 1);

#ifdef TEST_PRINT
        printf("now: %ld\n", OsDateTime::getSecsSinceEpoch());
        printf("real refresh period: %ld expires at: %ld from now: %ld\n",
               firstRefreshAt - start,
               smExpiration,
               smExpiration - OsDateTime::getSecsSinceEpoch());
#endif
        long firstRefreshExpiration = smExpiration;
        CPPUNIT_ASSERT_EQUAL(3, smCallbackCount);
        CPPUNIT_ASSERT_EQUAL(SipRefreshManager::REFRESH_REQUEST_SUCCEEDED, smLatestSubState);
        CPPUNIT_ASSERT(smExpiration + 5 >= initialExpiration + firstRefreshAt - start);
        CPPUNIT_ASSERT_EQUAL(253, smLastResponseCode);

        // This time respond with 408 and confirm that the expiration
        // has not changed.
        const SipMessage* secondRefresh = NULL;
        printf("waiting for refresh in %d seconds\n", normalRefreshPeriod);
        CPPUNIT_ASSERT(respond(incomingServerMsgQueue,
                               408, // response code
                               "Timeout",
                               *userAgentp,
                               (normalRefreshPeriod + 1) * 1000, // msec
                               secondRefresh));
        long secondRefreshAt = OsDateTime::getSecsSinceEpoch();
        // Wait for any callback generated by the refresh SUBSCRIBE.
        OsTask::delay(200);     // 200 msec

#ifdef TEST_PRINT
        printf("now: %ld\n", OsDateTime::getSecsSinceEpoch());
#endif
        CPPUNIT_ASSERT(secondRefresh);

        // There should be no callback, and the last seen expiration time
        // should not change.
        CPPUNIT_ASSERT_EQUAL(3, smCallbackCount);
        CPPUNIT_ASSERT_EQUAL(253, smLastResponseCode);
        CPPUNIT_ASSERT(secondRefreshAt - firstRefreshAt >= normalRefreshPeriod - 1); // allow 1 sec. variance
        CPPUNIT_ASSERT(secondRefreshAt - firstRefreshAt <= normalRefreshPeriod + 1);
        CPPUNIT_ASSERT_EQUAL(SipRefreshManager::REFRESH_REQUEST_SUCCEEDED, smLatestSubState); // the subscription is still OK
        CPPUNIT_ASSERT_EQUAL(firstRefreshExpiration, smExpiration);

        // The next refresh is an attempt to resend after a transient
        // error, which should happen in 1 second.
        printf("waiting for refresh in %d seconds\n", 1);
        const SipMessage* thirdRefresh = NULL;
        CPPUNIT_ASSERT(respond(incomingServerMsgQueue,
                               254, // response code
                               "Accepted",
                               *userAgentp,
                               1000, // milliseconds to wait for request
                               thirdRefresh));
        long thirdRefreshAt = OsDateTime::getSecsSinceEpoch();
        // Wait for the callback generated by the refresh SUBSCRIBE.
        OsTask::delay(200);     // 200 msec

#ifdef TEST_PRINT
        printf("now: %ld\n", OsDateTime::getSecsSinceEpoch());
        printf("expiration: %ld from now: %ld\n",
               smExpiration,
               smExpiration - OsDateTime::getSecsSinceEpoch());
#endif
        CPPUNIT_ASSERT(thirdRefresh);
        CPPUNIT_ASSERT_EQUAL(4, smCallbackCount);
        CPPUNIT_ASSERT_EQUAL(254, smLastResponseCode);
        CPPUNIT_ASSERT_EQUAL(SipRefreshManager::REFRESH_REQUEST_SUCCEEDED, smLatestSubState);
        CPPUNIT_ASSERT(thirdRefreshAt - secondRefreshAt >= 1 - 1); // allow 1 sec. variance
        CPPUNIT_ASSERT(thirdRefreshAt - secondRefreshAt <= 1 + 1);
        CPPUNIT_ASSERT(smExpiration >= thirdRefreshAt + expPeriod - 1); // allow 1 sec. variance
        CPPUNIT_ASSERT(smExpiration <= thirdRefreshAt + expPeriod + 1);
    }

    void refreshTestTransientRetryAndFail()
    {
        // Construct the SUBSCRIBE request.
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
        SipMessage* mwiSubscribeRequest = new SipMessage;
        mwiSubscribeRequest->setSipRequestFirstHeaderLine(SIP_SUBSCRIBE_METHOD,
                                                          aor,
                                                          SIP_PROTOCOL_VERSION);
        mwiSubscribeRequest->setContactField(aor);
        mwiSubscribeRequest->setRawFromField(from);
        mwiSubscribeRequest->setRawToField(to);
        mwiSubscribeRequest->setEventField(eventType);
        mwiSubscribeRequest->setCallIdField("123456");
        mwiSubscribeRequest->setCSeqField(1, SIP_SUBSCRIBE_METHOD);
        mwiSubscribeRequest->setExpiresField(expPeriod);

        ssize_t len;
        UtlString requestDump;
        mwiSubscribeRequest->getBytes(&requestDump, &len);
#ifdef TEST_PRINT
        printf("%s", requestDump.data());
#endif

        // Calculate the expected refresh period.
        int transactionTimeoutPeriod =
            userAgentp->getSipStateTransactionTimeout() / 1000;
        int normalRefreshPeriod = (int) (expPeriod * 0.65);
        if (normalRefreshPeriod < transactionTimeoutPeriod)
        {
            normalRefreshPeriod = transactionTimeoutPeriod;
        }

        // Call initiateRefresh to send the SUBSCRIBE.
        UtlString earlyDialogHandle;
        long start = OsDateTime::getSecsSinceEpoch();
#ifdef TEST_PRINT
        printf("now: %ld\n", OsDateTime::getSecsSinceEpoch());
#endif
        CPPUNIT_ASSERT(refreshManagerp->initiateRefresh(mwiSubscribeRequest,
                                                        this,
                                                        subStateCallback,
                                                        earlyDialogHandle));

        // Wait for the request and send a response
        // Use response codes starting with 250, so that we can verify that
        // the callback receives the information from the SUBSCRIBE response.
        // Receive SUBSCRIBE, send 252 response.
        // Since the test framework doesn't send NOTIFY, we don't need machinery
        // to receive NOTIFY.
        const SipMessage* initialRequest = NULL;
        CPPUNIT_ASSERT(respond(incomingServerMsgQueue,
                               252, // response code
                               "Got request and accepted",
                               *userAgentp,
                               1000, // milliseconds to wait for request
                               initialRequest));

        // Wait for the callback when SipRefreshManager receives the 252.
        OsTask::delay(200);     // 200 msec

#ifdef TEST_PRINT
        printf("now: %ld\n", OsDateTime::getSecsSinceEpoch());
        printf("expiration: %ld from now: %ld lag: %ld\n",
               smExpiration,
               smExpiration - OsDateTime::getSecsSinceEpoch(),
               (smExpiration - start) - expPeriod);
#endif
        CPPUNIT_ASSERT(initialRequest);
        CPPUNIT_ASSERT_EQUAL(2, smCallbackCount);
        CPPUNIT_ASSERT_EQUAL(252, smLastResponseCode);
        CPPUNIT_ASSERT_EQUAL(SipRefreshManager::REFRESH_REQUEST_SUCCEEDED, smLatestSubState);
        CPPUNIT_ASSERT(smExpiration >= start + expPeriod - 1); // allow 1 sec. variance
        CPPUNIT_ASSERT(smExpiration <= start + expPeriod + 1);
        long initialExpiration = smExpiration;

        // Wait for SipRefreshManager to send the refresh SUBSCRIBE.
        printf("waiting for refresh in %d seconds\n", normalRefreshPeriod);
        const SipMessage* firstRefresh;
        CPPUNIT_ASSERT(respond(incomingServerMsgQueue,
                               253, // response code
                               "Got request and accepted",
                               *userAgentp,
                               (normalRefreshPeriod + 1) * 1000, // msec
                               firstRefresh));
        long firstRefreshAt = OsDateTime::getSecsSinceEpoch();
        // Wait for the callback generated by the refresh SUBSCRIBE.
        OsTask::delay(200);     // 200 msec

#ifdef TEST_PRINT
        printf("now: %ld\n", OsDateTime::getSecsSinceEpoch());
        printf("expiration: %ld from now: %ld\n",
               smExpiration,
               smExpiration - OsDateTime::getSecsSinceEpoch());
#endif
        CPPUNIT_ASSERT(firstRefresh);
        CPPUNIT_ASSERT_EQUAL(3, smCallbackCount);
        CPPUNIT_ASSERT_EQUAL(253, smLastResponseCode);
        CPPUNIT_ASSERT_EQUAL(SipRefreshManager::REFRESH_REQUEST_SUCCEEDED, smLatestSubState);
        CPPUNIT_ASSERT(firstRefreshAt - start >= normalRefreshPeriod - 1); // allow 1 sec. variance
        CPPUNIT_ASSERT(firstRefreshAt - start <= normalRefreshPeriod + 1);
        CPPUNIT_ASSERT(smExpiration >= firstRefreshAt + expPeriod - 1); // allow 1 sec. variance
        CPPUNIT_ASSERT(smExpiration <= firstRefreshAt + expPeriod + 1);

#ifdef TEST_PRINT
        printf("now: %ld\n", OsDateTime::getSecsSinceEpoch());
        printf("real refresh period: %ld expires at: %ld from now: %ld\n",
               firstRefreshAt - start,
               smExpiration,
               smExpiration - OsDateTime::getSecsSinceEpoch());
#endif
        long firstRefreshExpiration = smExpiration;
        CPPUNIT_ASSERT_EQUAL(3, smCallbackCount);
        CPPUNIT_ASSERT_EQUAL(SipRefreshManager::REFRESH_REQUEST_SUCCEEDED, smLatestSubState);
        CPPUNIT_ASSERT(smExpiration + 5 >= initialExpiration + firstRefreshAt - start);
        CPPUNIT_ASSERT_EQUAL(253, smLastResponseCode);

        // This time respond with 408 and confirm that the expiration
        // has not changed.
        const SipMessage* secondRefresh = NULL;
        printf("waiting for refresh in %d seconds\n", normalRefreshPeriod);
        CPPUNIT_ASSERT(respond(incomingServerMsgQueue,
                               408, // response code
                               "Timeout",
                               *userAgentp,
                               (normalRefreshPeriod + 1) * 1000, // msec
                               secondRefresh));
        long secondRefreshAt = OsDateTime::getSecsSinceEpoch();
        // Wait for any callback generated by the refresh SUBSCRIBE.
        OsTask::delay(200);     // 200 msec

#ifdef TEST_PRINT
        printf("now: %ld\n", OsDateTime::getSecsSinceEpoch());
#endif
        CPPUNIT_ASSERT(secondRefresh);

        // There should be no callback, and the last seen expiration time
        // should not change.
        CPPUNIT_ASSERT_EQUAL(3, smCallbackCount);
        CPPUNIT_ASSERT_EQUAL(253, smLastResponseCode);
        CPPUNIT_ASSERT(secondRefreshAt - firstRefreshAt >= normalRefreshPeriod - 1); // allow 1 sec. variance
        CPPUNIT_ASSERT(secondRefreshAt - firstRefreshAt <= normalRefreshPeriod + 1);
        CPPUNIT_ASSERT_EQUAL(SipRefreshManager::REFRESH_REQUEST_SUCCEEDED, smLatestSubState); // the subscription is still OK
        CPPUNIT_ASSERT_EQUAL(firstRefreshExpiration, smExpiration);

        // The next refresh is an attempt to resend after a transient
        // error, which should happen in 1 second.
        int resendTime = 1;
        printf("waiting for refresh in %d seconds\n", resendTime);
        const SipMessage* thirdRefresh = NULL;
        CPPUNIT_ASSERT(respond(incomingServerMsgQueue,
                               408, // response code
                               "Timeout",
                               *userAgentp,
                               (resendTime + 1) * 1000, // msec
                               thirdRefresh));
        long thirdRefreshAt = OsDateTime::getSecsSinceEpoch();
        // Wait for any callback generated by the refresh SUBSCRIBE.
        OsTask::delay(200);     // 200 msec

#ifdef TEST_PRINT
        printf("now: %ld\n", OsDateTime::getSecsSinceEpoch());
#endif
        CPPUNIT_ASSERT(thirdRefresh);

        // There should be no callback, and the last seen expiration time
        // should not change.
        CPPUNIT_ASSERT_EQUAL(3, smCallbackCount);
        CPPUNIT_ASSERT_EQUAL(253, smLastResponseCode);
        CPPUNIT_ASSERT(thirdRefreshAt - secondRefreshAt >= resendTime - 1); // allow 1 sec. variance
        CPPUNIT_ASSERT(thirdRefreshAt - secondRefreshAt <= resendTime + 1);
        CPPUNIT_ASSERT_EQUAL(SipRefreshManager::REFRESH_REQUEST_SUCCEEDED, smLatestSubState); // the subscription is still OK
        CPPUNIT_ASSERT_EQUAL(firstRefreshExpiration, smExpiration);

        // The next refresh is the second attempt to resend after a
        // transient error, which should happen in 2 seconds.
        resendTime = 2;
        printf("waiting for refresh in %d seconds\n", resendTime);
        const SipMessage* fourthRefresh = NULL;
        CPPUNIT_ASSERT(respond(incomingServerMsgQueue,
                               408, // response code
                               "Timeout",
                               *userAgentp,
                               (resendTime + 1) * 1000, // msec
                               fourthRefresh));
        long fourthRefreshAt = OsDateTime::getSecsSinceEpoch();
        // Wait for any callback generated by the refresh SUBSCRIBE.
        OsTask::delay(200);     // 200 msec

#ifdef TEST_PRINT
        printf("now: %ld\n", OsDateTime::getSecsSinceEpoch());
#endif
        CPPUNIT_ASSERT(fourthRefresh);

        // There should be no callback, and the last seen expiration time
        // should not change.
        CPPUNIT_ASSERT_EQUAL(3, smCallbackCount);
        CPPUNIT_ASSERT_EQUAL(253, smLastResponseCode);
        CPPUNIT_ASSERT(fourthRefreshAt - thirdRefreshAt >= resendTime - 1); // allow 1 sec. variance
        CPPUNIT_ASSERT(fourthRefreshAt - thirdRefreshAt <= resendTime + 1);
        CPPUNIT_ASSERT_EQUAL(SipRefreshManager::REFRESH_REQUEST_SUCCEEDED, smLatestSubState); // the subscription is still OK
        CPPUNIT_ASSERT_EQUAL(firstRefreshExpiration, smExpiration);

        // The next refresh is the third attempt to resend after a
        // transient error, which should happen in 4 seconds.
        // We give a 481 response, which should termiante the refresh.
        resendTime = 4;
        printf("waiting for refresh in %d seconds\n", resendTime);
        const SipMessage* fifthRefresh = NULL;
        CPPUNIT_ASSERT(respond(incomingServerMsgQueue,
                               481, // response code
                               "Permanent Error",
                               *userAgentp,
                               (resendTime + 1) * 1000, // msec
                               fifthRefresh));
        long fifthRefreshAt = OsDateTime::getSecsSinceEpoch();
        // Wait for any callback generated by the refresh SUBSCRIBE.
        OsTask::delay(200);     // 200 msec

#ifdef TEST_PRINT
        printf("now: %ld\n", OsDateTime::getSecsSinceEpoch());
#endif
        CPPUNIT_ASSERT(fifthRefresh);

        // There should be a callback, showing that the refresh failed.
        CPPUNIT_ASSERT_EQUAL(4, smCallbackCount);
        CPPUNIT_ASSERT_EQUAL(481, smLastResponseCode);
        CPPUNIT_ASSERT(fifthRefreshAt - fourthRefreshAt >= resendTime - 1); // allow 1 sec. variance
        CPPUNIT_ASSERT(fifthRefreshAt - fourthRefreshAt <= resendTime + 1);
        CPPUNIT_ASSERT_EQUAL(SipRefreshManager::REFRESH_REQUEST_FAILED, smLatestSubState);
        CPPUNIT_ASSERT_EQUAL(0L, smExpiration);

        // Wait to make sure that no further refresh is sent.
        printf("waiting for refresh in %d seconds\n", normalRefreshPeriod);
        const SipMessage* sixthRefresh = NULL;
        CPPUNIT_ASSERT(!removeMessage(incomingServerMsgQueue,
                                      (resendTime + 1) * 1000, // msec
                                      sixthRefresh));
    }
};

// The initial values here are for documentation.
// The real initial values are set before each test by setUp().
SipRefreshManager::RefreshRequestState SipRefreshManagerTest::smLatestSubState = SipRefreshManager::REFRESH_REQUEST_UNKNOWN;
int SipRefreshManagerTest::smCallbackCount = 0;
int SipRefreshManagerTest::smLastResponseCode = -2;
long SipRefreshManagerTest::smExpiration = -2;


CPPUNIT_TEST_SUITE_REGISTRATION(SipRefreshManagerTest);
