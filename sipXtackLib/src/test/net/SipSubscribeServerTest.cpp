//
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
#include <net/CallId.h>
#include <net/SipDialog.h>
#include <net/SipMessage.h>
#include <net/SipDialogMgr.h>
#include <net/SipSubscriptionMgr.h>
#include <net/SipSubscribeServerEventHandler.h>
#include <net/SipUserAgent.h>
#include <net/SipSubscribeServer.h>
#include <net/SipPublishContentMgr.h>

#include "SipSubscribeTestSupport.h"

/**
 * Unit tests for SipSubscribeServer
 */

class SipSubscribeServerTest1 : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SipSubscribeServerTest1);
   CPPUNIT_TEST(basicSubscriptionTest);
   CPPUNIT_TEST(nonexistentResourceSubscriptionTest);
   CPPUNIT_TEST(noAllowedContentSubscriptionTest);
   CPPUNIT_TEST_SUITE_END();

public:

   UtlString hostIp;
   const char* mwiStateString;
   UtlString eventName;
   UtlString mwiMimeType;
   SipUserAgent* notifierUserAgentp;
   UtlString notifier_aor;
   UtlString notifier_name_addr;
   UtlString notifier_contact_name_addr;
   UtlString notifier_resource_id;
   SipUserAgent* subscriberUserAgentp;
   UtlString subscriber_aor;
   UtlString subscriber_name_addr;
   UtlString subscriber_contact_name_addr;
   UtlString subscriber_resource_id;
   SipSubscribeServer* subServerp;
   SipSubscriptionMgr* subMgrp;
   SipDialogMgr* dialogMgrp;
   OsMsgQ incomingClientMsgQueue;

   void setUp()
      {
         hostIp = "127.0.0.1";

         mwiStateString =
            "Messages-Waiting: no\r\n"
            "Voice-Message: 0/0 (0/0)\r\n";

         eventName = SIP_EVENT_MESSAGE_SUMMARY;
         mwiMimeType = CONTENT_TYPE_SIMPLE_MESSAGE_SUMMARY;

         // Construct the notifier (Subscription Server) user agent.
         // Construct the URI of the notifier, which is also the URI of
         // the subscriber.
         // Also construct the name-addr version of the URI, which may be
         // different if it has a "transport" parameter.
         // And the resource-id to use, which is the AOR with any
         // parameters stripped off.

         createTestSipUserAgent(hostIp,
                                "notifier",
                                notifierUserAgentp,
                                notifier_aor,
                                notifier_name_addr,
                                notifier_contact_name_addr,
                                notifier_resource_id);


         // Construct the subscriber (Subscription Client) user agent.
         // Construct the URI of the notifier, which is also the URI of
         // the subscriber.
         // Also construct the name-addr version of the URI, which may be
         // different if it has a "transport" parameter.
         // And the resource-id to use, which is the AOR with any
         // parameters stripped off.

         createTestSipUserAgent(hostIp,
                                "subscriber",
                                subscriberUserAgentp,
                                subscriber_aor,
                                subscriber_name_addr,
                                subscriber_contact_name_addr,
                                subscriber_resource_id);

         subServerp =
            SipSubscribeServer::buildBasicServer(*notifierUserAgentp,
                                                 eventName);
         subServerp->start();

         // Get pointers to the Subscription Manager and Dialog Manager.
         subMgrp = subServerp->getSubscriptionMgr(eventName);
         CPPUNIT_ASSERT(subMgrp);
         dialogMgrp = subMgrp->getDialogMgr();
         CPPUNIT_ASSERT(dialogMgrp);

         // Publish content, so we can subscribe to the URI.
         SipPublishContentMgr* pubMgrp = subServerp->getPublishMgr(eventName);
         
         const char* cc = "This is a test\n";
         HttpBody* c = new HttpBody(cc, strlen(cc), mwiMimeType);
         pubMgrp->publish(notifier_resource_id,
                          eventName,
                          eventName,
                          1,
                          &c,
                          TRUE);

         // Create a simple Subscription client
         // Register an interest in SUBSCRIBE responses and NOTIFY requests
         // for this event type
         subscriberUserAgentp->addMessageObserver(incomingClientMsgQueue,
                                                  SIP_SUBSCRIBE_METHOD,
                                                  FALSE, // no requests
                                                  TRUE, // reponses
                                                  TRUE, // incoming
                                                  FALSE, // no outgoing
                                                  eventName,
                                                  NULL,
                                                  NULL);
         subscriberUserAgentp->addMessageObserver(incomingClientMsgQueue,
                                                  SIP_NOTIFY_METHOD,
                                                  TRUE, // requests
                                                  FALSE, // not reponses
                                                  TRUE, // incoming
                                                  FALSE, // no outgoing
                                                  eventName,
                                                  NULL,
                                                  NULL);
      }

   void tearDown()
      {
         // Clean up to prevent use of the queue after it goes out of scope.
         notifierUserAgentp->removeMessageObserver(incomingClientMsgQueue);
         notifierUserAgentp->removeMessageObserver(incomingClientMsgQueue);

         OsTask::delay(1000);

         // Delete the Subscribe Server before deleting the objects it uses.
         delete subServerp;

         notifierUserAgentp->shutdown(TRUE);
         delete notifierUserAgentp;

         subscriberUserAgentp->shutdown(TRUE);
         delete subscriberUserAgentp;
      }

   // Basic server functionality test.
   // Checks that server answers SUBSCRIBES, sends NOTIFY, sends NOTIFY
   // when content changes.
   void basicSubscriptionTest()
      {
         // Verify that authentication and authorization are
         // disabled by default.
         {
            SipSubscribeServerEventHandler* eventHandler =
               subServerp->getEventHandler(eventName);
            CPPUNIT_ASSERT(eventHandler);

            SipMessage bogusSubscribeRequest;
            SipMessage bogusSubscribeResponse;
            CPPUNIT_ASSERT(eventHandler->isAuthenticated(bogusSubscribeRequest,
                                                         bogusSubscribeResponse));
            CPPUNIT_ASSERT(eventHandler->isAuthorized(bogusSubscribeRequest,
                                                      bogusSubscribeResponse));
         }

         // Send a SUBSCRIBE to the notifier.
         SipMessage mwiSubscribeRequest;
         {
            UtlString c;
            CallId::getNewCallId(c);
            mwiSubscribeRequest.setSubscribeData(notifier_aor, // request URI
                                                 subscriber_name_addr, // From
                                                 notifier_name_addr, // To
                                                 c, // Call-Id
                                                 0, // CSeq
                                                 eventName, // Event
                                                 mwiMimeType, // Accept
                                                 NULL, // Event id
                                                 subscriber_name_addr, // Contact
                                                 NULL, // Route
                                                 3600 // Expires
               );
         }

         CPPUNIT_ASSERT(subscriberUserAgentp->send(mwiSubscribeRequest));

         // We should get a 202 response and a NOTIFY request in the queue
         OsTime messageTimeout(1, 0);  // 1 second
         {
            const SipMessage* subscribeResponse;
            const SipMessage* notifyRequest;
            CPPUNIT_ASSERT(runListener(incomingClientMsgQueue,
                                       *subscriberUserAgentp,
                                       messageTimeout,
                                       messageTimeout,
                                       notifyRequest,
                                       subscribeResponse,
                                       SIP_OK_CODE,
                                       FALSE,
                                       0,
                                       NULL));

            // We should have received a SUBSCRIBE response and a NOTIFY request.
            CPPUNIT_ASSERT(subscribeResponse);
            CPPUNIT_ASSERT(notifyRequest);

            // Check that the response code and CSeq method in the
            // subscribe response are OK.
            {
               CPPUNIT_ASSERT_EQUAL(SIP_ACCEPTED_CODE,
                                    subscribeResponse->getResponseStatusCode());
               UtlString subscribeMethod;
               subscribeResponse->getCSeqField(NULL, &subscribeMethod);
               ASSERT_STR_EQUAL(SIP_SUBSCRIBE_METHOD, subscribeMethod.data());
            }

            // Check that the method in the NOTIFY request is OK.
            {
               UtlString notifyMethod;
               notifyRequest->getRequestMethod(&notifyMethod);
               ASSERT_STR_EQUAL(SIP_NOTIFY_METHOD, notifyMethod.data());
            }

            // Check that the Content-Type in the NOTIFY request is OK.
            {
               UtlString contentType;
               notifyRequest->getContentType(&contentType);
               ASSERT_STR_EQUAL(mwiMimeType.data(), contentType.data());
            }

            // Check that the Event header in the NOTIFY is the same as the
            // one in the SUBSCRIBE.
            {
               UtlString notifyEventHeader;
               UtlString subscribeEventHeader;
               notifyRequest->getEventField(notifyEventHeader);
               mwiSubscribeRequest.getEventField(subscribeEventHeader);
               ASSERT_STR_EQUAL(subscribeEventHeader, notifyEventHeader);
            }

            // The NOTIFY should have a body because we have published
            // content for this request-URI.
            {
               const HttpBody* bodyPtr = notifyRequest->getBody();
               CPPUNIT_ASSERT(bodyPtr != NULL);
            }

            // The Contact in the subscribe response should be the notifier.
            ASSERT_STR_EQUAL(notifier_contact_name_addr,
                             subscribeResponse->
                                 getHeaderValue(0, SIP_CONTACT_FIELD));

            // The Contact in the NOTIFY should be the notifier.
            CPPUNIT_ASSERT(notifyRequest->
                           getHeaderValue(0, SIP_CONTACT_FIELD));
            ASSERT_STR_EQUAL(notifier_contact_name_addr,
                             notifyRequest->
                                 getHeaderValue(0, SIP_CONTACT_FIELD));
         }

         // Publish some content (mwiStateString) for this resourceID
         {
            HttpBody* newMwiBodyPtr = new HttpBody(mwiStateString,
                                                   strlen(mwiStateString),
                                                   mwiMimeType);
            SipPublishContentMgr* publishMgr = subServerp->getPublishMgr(eventName);
            CPPUNIT_ASSERT(publishMgr);
            publishMgr->publish(notifier_resource_id,
                                eventName,
                                eventName,
                                1,
                                &newMwiBodyPtr);
         }

         // Should get a NOTIFY queued up
         {
            const SipMessage* subscribeResponse;
            const SipMessage* secondNotify;
            CPPUNIT_ASSERT(runListener(incomingClientMsgQueue,
                                       *subscriberUserAgentp,
                                       messageTimeout,
                                       messageTimeout,
                                       secondNotify,
                                       subscribeResponse,
                                       SIP_OK_CODE,
                                       FALSE,
                                       0,
                                       NULL));
            CPPUNIT_ASSERT(secondNotify);
            CPPUNIT_ASSERT(subscribeResponse == NULL);

            // Check that the body of the NOTIFY is what we expect (mwiStateString).
            {
               const HttpBody* secondNotifyBody = secondNotify->getBody();
               CPPUNIT_ASSERT(secondNotifyBody);
               ssize_t notifyBodySize;
               const char* notifyBodyBytes;
               secondNotifyBody->getBytes(&notifyBodyBytes, &notifyBodySize);
               CPPUNIT_ASSERT(notifyBodyBytes);
               ASSERT_STR_EQUAL(mwiStateString, notifyBodyBytes);
            }

            // Check that the Dialog Manager reports that the dialog handle is OK.
            {
               UtlString secondNotifyDialogHandle;
               secondNotify->getDialogHandle(secondNotifyDialogHandle);
               CPPUNIT_ASSERT(!secondNotifyDialogHandle.isNull());
               CPPUNIT_ASSERT(dialogMgrp->dialogExists(secondNotifyDialogHandle));
               CPPUNIT_ASSERT(dialogMgrp->countDialogs() == 1);
            }
         }

         // Create a new one-time SUBSCRIBE
         SipMessage oneTimeMwiSubscribeRequest;
         {
            UtlString c;
            CallId::getNewCallId(c);
            oneTimeMwiSubscribeRequest.
               setSubscribeData(notifier_aor, // request URI
                                subscriber_name_addr, // From
                                notifier_name_addr, // To
                                c, // Call-Id
                                0, // CSeq
                                eventName, // Event
                                mwiMimeType, // Accept
                                NULL, // Event id
                                subscriber_name_addr, // Contact
                                NULL, // Route
                                0 // Expires
                  );
         }

         CPPUNIT_ASSERT(subscriberUserAgentp->send(oneTimeMwiSubscribeRequest));

         {
            const SipMessage* oneTimeNotifyRequest;
            const SipMessage* oneTimeSubscribeResponse;
            CPPUNIT_ASSERT(runListener(incomingClientMsgQueue,
                                       *subscriberUserAgentp,
                                       messageTimeout,
                                       messageTimeout,
                                       oneTimeNotifyRequest,
                                       oneTimeSubscribeResponse,
                                       SIP_OK_CODE,
                                       FALSE,
                                       0,
                                       NULL));

            // Validate the one time subscribe response and notify request
            CPPUNIT_ASSERT(oneTimeSubscribeResponse);
            CPPUNIT_ASSERT(oneTimeNotifyRequest);

            {
               CPPUNIT_ASSERT_EQUAL(SIP_ACCEPTED_CODE,
                                    oneTimeSubscribeResponse->getResponseStatusCode());
               UtlString oneTimeSubscribeMethod;
               oneTimeSubscribeResponse->getCSeqField(NULL, &oneTimeSubscribeMethod);
               ASSERT_STR_EQUAL(SIP_SUBSCRIBE_METHOD, oneTimeSubscribeMethod.data());
               UtlString oneTimeSubscribeEventHeader;
               oneTimeSubscribeResponse->getEventField(oneTimeSubscribeEventHeader);
               // The Event: header never appears in responses -- see RFC 3265 section 7.2.
               // The "R" in the "where" column means that "Event" appears only in
               // requests -- see RFC 3261 table 2 and page 160.
               ASSERT_STR_EQUAL("", oneTimeSubscribeEventHeader);
            }

            {
               UtlString oneTimeNotifyMethod;
               oneTimeNotifyRequest->getRequestMethod(&oneTimeNotifyMethod);
               ASSERT_STR_EQUAL(SIP_NOTIFY_METHOD, oneTimeNotifyMethod.data());
               UtlString oneTimeContentType;
               oneTimeNotifyRequest->getContentType(&oneTimeContentType);
               ASSERT_STR_EQUAL(mwiMimeType.data(), oneTimeContentType.data());
               UtlString oneTimeNotifyEventHeader;
               oneTimeNotifyRequest->getEventField(oneTimeNotifyEventHeader);
               // The Event: header should appear in the NOTIFY.
               ASSERT_STR_EQUAL(SIP_EVENT_MESSAGE_SUMMARY, oneTimeNotifyEventHeader);
            }

            {
               const HttpBody* oneTimeBodyPtr = oneTimeNotifyRequest->getBody();
               CPPUNIT_ASSERT(oneTimeBodyPtr != NULL);
               const char* oneTimeBodyString;
               ssize_t oneTimeNotifyBodySize;
               oneTimeBodyPtr->getBytes(&oneTimeBodyString, &oneTimeNotifyBodySize);
               ASSERT_STR_EQUAL(mwiStateString, oneTimeBodyString);
            }

            {
               UtlString oneTimeNotifyDialogHandle;
               oneTimeNotifyRequest->getDialogHandle(oneTimeNotifyDialogHandle);
               CPPUNIT_ASSERT(!oneTimeNotifyDialogHandle.isNull());
               CPPUNIT_ASSERT(dialogMgrp->dialogExists(oneTimeNotifyDialogHandle));
               CPPUNIT_ASSERT(dialogMgrp->countDialogs() == 2);
               long now = OsDateTime::getSecsSinceEpoch();
               subMgrp->removeOldSubscriptions(now + 1);
               // The one time subscription should get garbage collected
               // leaving only the persistant 3600 second subscription
               CPPUNIT_ASSERT(dialogMgrp->countDialogs() == 1);
            }
         }
      }

   // Basic server functionality test.
   // Checks that server answers SUBSCRIBES, sends NOTIFY, sends NOTIFY
   // when content changes.
   void nonexistentResourceSubscriptionTest()
      {
        // Remove the content for the resource URI.
        subServerp->getPublishMgr(eventName)->
            unpublish(notifier_resource_id,
                      eventName,
                      eventName,
                      SipSubscribeServer::terminationReasonNone);

         // Send a SUBSCRIBE to the notifier.
         SipMessage mwiSubscribeRequest;
         {
            UtlString c;
            CallId::getNewCallId(c);
            mwiSubscribeRequest.setSubscribeData(notifier_aor, // request URI
                                                 subscriber_name_addr, // From
                                                 notifier_name_addr, // To
                                                 c, // Call-Id
                                                 0, // CSeq
                                                 eventName, // Event
                                                 mwiMimeType, // Accept
                                                 NULL, // Event id
                                                 subscriber_name_addr, // Contact
                                                 NULL, // Route
                                                 3600 // Expires
               );
         }

         CPPUNIT_ASSERT(subscriberUserAgentp->send(mwiSubscribeRequest));

         // We should get a 404 response and no NOTIFY request in the queue
         OsTime messageTimeout(1, 0);  // 1 second
         {
            const SipMessage* subscribeResponse;
            const SipMessage* notifyRequest;
            CPPUNIT_ASSERT(runListener(incomingClientMsgQueue,
                                       *subscriberUserAgentp,
                                       messageTimeout,
                                       messageTimeout,
                                       notifyRequest,
                                       subscribeResponse,
                                       SIP_OK_CODE,
                                       FALSE,
                                       0,
                                       NULL));

            // We should have received a SUBSCRIBE response and no NOTIFY request.
            CPPUNIT_ASSERT(subscribeResponse);
            CPPUNIT_ASSERT(!notifyRequest);

            // Check that the response code and CSeq method in the
            // subscribe response are OK.
            {
               CPPUNIT_ASSERT_EQUAL(SIP_NOT_FOUND_CODE,
                                    subscribeResponse->getResponseStatusCode());
               UtlString subscribeMethod;
               subscribeResponse->getCSeqField(NULL, &subscribeMethod);
               ASSERT_STR_EQUAL(SIP_SUBSCRIBE_METHOD, subscribeMethod.data());
            }
         }
      }

   // Basic server functionality test.
   // Checks that server answers SUBSCRIBES, sends NOTIFY, sends NOTIFY
   // when content changes.
   void noAllowedContentSubscriptionTest()
      {
         // Send a SUBSCRIBE to the notifier.
         SipMessage mwiSubscribeRequest;
         {
            UtlString c;
            CallId::getNewCallId(c);
            mwiSubscribeRequest.setSubscribeData(notifier_aor, // request URI
                                                 subscriber_name_addr, // From
                                                 notifier_name_addr, // To
                                                 c, // Call-Id
                                                 0, // CSeq
                                                 eventName, // Event
                                                 "application/x-nonexistent", // Accept
                                                 NULL, // Event id
                                                 subscriber_name_addr, // Contact
                                                 NULL, // Route
                                                 3600 // Expires
               );
         }

         CPPUNIT_ASSERT(subscriberUserAgentp->send(mwiSubscribeRequest));

         // We should get a 415 response and no NOTIFY request in the queue
         OsTime messageTimeout(1, 0);  // 1 second
         {
            const SipMessage* subscribeResponse;
            const SipMessage* notifyRequest;
            CPPUNIT_ASSERT(runListener(incomingClientMsgQueue,
                                       *subscriberUserAgentp,
                                       messageTimeout,
                                       messageTimeout,
                                       notifyRequest,
                                       subscribeResponse,
                                       SIP_OK_CODE,
                                       FALSE,
                                       0,
                                       NULL));

            // We should have received a SUBSCRIBE response and no NOTIFY request.
            CPPUNIT_ASSERT(subscribeResponse);
            CPPUNIT_ASSERT(!notifyRequest);

            // Check that the response code and CSeq method in the
            // subscribe response are OK.
            {
               CPPUNIT_ASSERT_EQUAL(SIP_BAD_MEDIA_CODE,
                                    subscribeResponse->getResponseStatusCode());
               // Check that the Accept header has the right value.
               ASSERT_STR_EQUAL(mwiMimeType,
                                subscribeResponse->getHeaderValue(0, SIP_ACCEPT_FIELD));

               UtlString subscribeMethod;
               subscribeResponse->getCSeqField(NULL, &subscribeMethod);
               ASSERT_STR_EQUAL(SIP_SUBSCRIBE_METHOD, subscribeMethod.data());
            }
         }
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SipSubscribeServerTest1);


class SipSubscribeServerTest2 : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SipSubscribeServerTest2);
   CPPUNIT_TEST(resubscribeContentTest);
   CPPUNIT_TEST(terminateSubscriptionOnError);
   CPPUNIT_TEST(terminateSubscriptionOnErrorRetryAfter);
   CPPUNIT_TEST(terminateSubscriptionOnErrorTimeout);
   CPPUNIT_TEST(responseContact);
#ifdef EXECUTE_SLOW_TESTS
   CPPUNIT_TEST(notifyResend);
   CPPUNIT_TEST(fullVsPartial);
#endif // EXECUTE_SLOW_TESTS

   CPPUNIT_TEST_SUITE_END();

public:

   UtlString hostIp;
   const char* mwiStateString;
   UtlString eventName;
   UtlString mwiMimeType;
   SipUserAgent* userAgentp;
   UtlString aor;
   UtlString aor_name_addr;
   UtlString aor_contact_name_addr;
   UtlString resource_id;
   SipSubscribeServer* subServerp;
   SipSubscriptionMgr* subMgrp;
   SipDialogMgr* dialogMgrp;
   OsMsgQ incomingClientMsgQueue;

   void setUp()
      {
         hostIp = "127.0.0.1";

         eventName = SIP_EVENT_MESSAGE_SUMMARY;
         mwiMimeType = CONTENT_TYPE_SIMPLE_MESSAGE_SUMMARY;

         // Construct a user agent that will function both as the subscriber
         // and the notfier.
         // Construct the URI of the notifier, which is also the URI of
         // the subscriber.
         // Also construct the name-addr version of the URI, which may be
         // different if it has a "transport" parameter.
         // And the resource-id to use, which is the AOR with any
         // parameters stripped off.

         createTestSipUserAgent(hostIp,
                                "111",
                                userAgentp,
                                aor,
                                aor_name_addr,
                                aor_contact_name_addr,
                                resource_id);

         subServerp =
            SipSubscribeServer::buildBasicServer(*userAgentp,
                                                 eventName);
         subServerp->start();

         // Get pointers to the Subscription Manager and Dialog Manager.
         subMgrp = subServerp->getSubscriptionMgr(eventName);
         CPPUNIT_ASSERT(subMgrp);
         dialogMgrp = subMgrp->getDialogMgr();
         CPPUNIT_ASSERT(dialogMgrp);

         // Publish content, so we can subscribe to the URI.
         SipPublishContentMgr* pubMgrp = subServerp->getPublishMgr(eventName);
         
         const char* cc = "This is a test\n";
         HttpBody* c = new HttpBody(cc, strlen(cc), mwiMimeType);
         pubMgrp->publish(resource_id,
                          eventName,
                          eventName,
                          1,
                          &c,
                          TRUE);

         // Create a simple Subscription client
         // Register an interest in SUBSCRIBE responses and NOTIFY requests
         // for this event type
         userAgentp->addMessageObserver(incomingClientMsgQueue,
                                        SIP_SUBSCRIBE_METHOD,
                                        FALSE, // no requests
                                        TRUE, // reponses
                                        TRUE, // incoming
                                        FALSE, // no outgoing
                                        eventName,
                                        NULL,
                                        NULL);
         userAgentp->addMessageObserver(incomingClientMsgQueue,
                                        SIP_NOTIFY_METHOD,
                                        TRUE, // requests
                                        FALSE, // not reponses
                                        TRUE, // incoming
                                        FALSE, // no outgoing
                                        eventName,
                                        NULL,
                                        NULL);
      }

   void tearDown()
      {
         // Clean up to prevent use of the queue after it goes out of scope.
         userAgentp->removeMessageObserver(incomingClientMsgQueue);
         userAgentp->removeMessageObserver(incomingClientMsgQueue);

         OsTask::delay(1000);

         // Delete the Subscribe Server before deleting the objects it uses.
         delete subServerp;

         userAgentp->shutdown(TRUE);
         delete userAgentp;
      }

   // Test for XECS-243, Subscribe Server does not send NOTIFY when it
   // processes a re-SUBSCRIBE.
//*** Error scenario not yet reproduced.
   void resubscribeContentTest()
      {
         // Test MWI messages
         const char* mwiSubscribe =
            "SUBSCRIBE sip:111@localhost SIP/2.0\r\n"
            "From: <sip:111@example.com>;tag=1612c1612\r\n"
            "To: <sip:111@example.com>\r\n"
            "Cseq: 1 SUBSCRIBE\r\n"
            "Event: message-summary\r\n"
            "Accept: application/simple-message-summary\r\n"
            "Expires: 3600\r\n"
            "Date: Tue, 26 Apr 2005 14:59:30 GMT\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: Pingtel/2.2.0 (VxWorks)\r\n"
            "Accept-Language: en\r\n"
            "Supported: sip-cc, sip-cc-01, timer, replaces\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         // Send a SUBSCRIBE to ourselves
         SipMessage mwiSubscribeRequest(mwiSubscribe);
         {
            UtlString c;
            CallId::getNewCallId(c);
            mwiSubscribeRequest.setCallIdField(c);
         }
         mwiSubscribeRequest.setSipRequestFirstHeaderLine(SIP_SUBSCRIBE_METHOD,
                                                          aor,
                                                          SIP_PROTOCOL_VERSION);
         mwiSubscribeRequest.setContactField(aor_name_addr);

         CPPUNIT_ASSERT(userAgentp->send(mwiSubscribeRequest));

         // We should get a 202 response and a NOTIFY request in the queue
         // Send a 200 response to the NOTIFY.
         OsTime messageTimeout(1, 0);  // 1 second
         {
            const SipMessage* subscribeResponse;
            const SipMessage* notifyRequest;
            CPPUNIT_ASSERT(runListener(incomingClientMsgQueue,
                                       *userAgentp,
                                       messageTimeout,
                                       messageTimeout,
                                       notifyRequest,
                                       subscribeResponse,
                                       SIP_OK_CODE,
                                       FALSE,
                                       0,
                                       NULL));

            // We should have received a SUBSCRIBE response and a NOTIFY request.
            CPPUNIT_ASSERT(subscribeResponse);
            CPPUNIT_ASSERT(notifyRequest);

            CPPUNIT_ASSERT_EQUAL(SIP_ACCEPTED_CODE,
                                 subscribeResponse->getResponseStatusCode());

            // The NOTIFY should have a body because we have published
            // content for this request-URI.
            {
               const HttpBody* bodyPtr = notifyRequest->getBody();
               CPPUNIT_ASSERT(bodyPtr != NULL);
            }

            // Update the to-tag of the SUBSCRIBE.
            Url responseToField;
            subscribeResponse->getToUrl(responseToField);
            UtlString toTag;
            responseToField.getFieldParameter("tag", toTag);
            mwiSubscribeRequest.setToFieldTag(toTag);
         }

         // Send a re-SUBSCRIBE
         mwiSubscribeRequest.incrementCSeqNumber();
         // Leave the Expires header with the default value.

         CPPUNIT_ASSERT(userAgentp->send(mwiSubscribeRequest));

         // We should get a 202 response and a NOTIFY request in the queue
         {
            const SipMessage* subscribeResponse;
            const SipMessage* notifyRequest;
            CPPUNIT_ASSERT(runListener(incomingClientMsgQueue,
                                       *userAgentp,
                                       messageTimeout,
                                       messageTimeout,
                                       notifyRequest,
                                       subscribeResponse,
                                       SIP_OK_CODE,
                                       FALSE,
                                       0,
                                       NULL));

            // We should have received a SUBSCRIBE response and a NOTIFY request.
            CPPUNIT_ASSERT(subscribeResponse);
            CPPUNIT_ASSERT(notifyRequest);

            CPPUNIT_ASSERT_EQUAL(SIP_ACCEPTED_CODE,
                                 subscribeResponse->getResponseStatusCode());

            // The NOTIFY should have a body because we have published
            // content for this request-URI.
            {
               const HttpBody* bodyPtr = notifyRequest->getBody();
               CPPUNIT_ASSERT(bodyPtr != NULL);
            }
         }
      }

   // Test for XECS-247, When subscribe server receives 481 for a
   // NOTIFY, it does not terminate the subscription.
   // Test for XECS-282, When subscribe server receives 500 for a
   // NOTIFY, it does not terminate the subscription.
   // Note that 500 now does not terminate the subscription because in many
   // cases, alternative forking of the NOTIFY to a slow subscriber results
   // in 500 responses.
   // Verify that subscription is terminated when NOTIFY returns a variety
   // of error responses.  (Retry-After suppresses termination.  That case
   // is tested in terminateSubscriptionOnErrorRetryAfter.)
   void terminateSubscriptionOnError()
      {
         // Test MWI messages
         const char* mwiSubscribe =
            "SUBSCRIBE sip:111@localhost SIP/2.0\r\n"
            "From: <sip:111@example.com>;tag=1612c1612\r\n"
            "To: <sip:111@example.com>\r\n"
            "Cseq: 1 SUBSCRIBE\r\n"
            "Event: message-summary\r\n"
            "Accept: application/simple-message-summary\r\n"
            "Expires: 3600\r\n"
            "Date: Tue, 26 Apr 2005 14:59:30 GMT\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: Pingtel/2.2.0 (VxWorks)\r\n"
            "Accept-Language: en\r\n"
            "Supported: sip-cc, sip-cc-01, timer, replaces\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         // Loop through this scenario for a series of response codes.
         int test_codes[] = { SIP_BAD_REQUEST_CODE,
                              SIP_NOT_FOUND_CODE,
                              SIP_BAD_TRANSACTION_CODE,
                              499,
                              // We would like to test 503
                              // (SIP_SERVICE_UNAVAILABLE_CODE) here, but
                              // SipTransaction turns a received 503 into
                              // 500, so it also does not terminate a
                              // subscription.
                              599,
                              SIP_GLOBAL_BUSY_CODE,
                              699 };
         for (unsigned int i = 0;
              i < sizeof (test_codes) / sizeof (test_codes[0]);
              i++)
         {
            char message[100];
            sprintf(message, "test_codes[%d] = %d", i, test_codes[i]);

            // Send a SUBSCRIBE to ourselves
            SipMessage mwiSubscribeRequest(mwiSubscribe);
            {
               UtlString c;
               CallId::getNewCallId(c);
               mwiSubscribeRequest.setCallIdField(c);
            }
            mwiSubscribeRequest.setSipRequestFirstHeaderLine(SIP_SUBSCRIBE_METHOD,
                                                             aor,
                                                             SIP_PROTOCOL_VERSION);
            mwiSubscribeRequest.setContactField(aor_name_addr);
            mwiSubscribeRequest.incrementCSeqNumber();

            CPPUNIT_ASSERT_MESSAGE(message,
                                   userAgentp->send(mwiSubscribeRequest));

            // We should get a 202 response and a NOTIFY request in the queue
            // Send the specified response to the NOTIFY.
            OsTime messageTimeout(1, 0);  // 1 second
            {
               const SipMessage* subscribeResponse;
               const SipMessage* notifyRequest;
               const SipMessage* notifyRequest2;
               CPPUNIT_ASSERT(runListener(incomingClientMsgQueue,
                                          *userAgentp,
                                          messageTimeout,
                                          messageTimeout,
                                          notifyRequest,
                                          subscribeResponse,
                                          test_codes[i],
                                          FALSE,
                                          0,
                                          NULL,
                                          &notifyRequest2));

               // We should have received a SUBSCRIBE response and a NOTIFY request.
               CPPUNIT_ASSERT_MESSAGE(message,
                                      subscribeResponse);

               // Extract the to-tag in the response, apply it to
               // mwiSubscribeRequest.
               Url toUrl;
               subscribeResponse->getToUrl(toUrl);
               UtlString toTag;
               toUrl.getFieldParameter("tag", toTag);
               mwiSubscribeRequest.setToFieldTag(toTag);

               // Check that the subscribe response is what we expected.

               CPPUNIT_ASSERT_EQUAL_MESSAGE(message,
                                            SIP_ACCEPTED_CODE,
                                            subscribeResponse->getResponseStatusCode());

               // Check that the first NOTIFY is what we expected.

               CPPUNIT_ASSERT_MESSAGE(message, notifyRequest);
               // Have to be careful here because the server may not give us
               // the length of subscription that we requested.
               #define EXPECTED_STATE_PREFIX "active;expires="
               CPPUNIT_ASSERT_MESSAGE(
                  message,
                  0 == strncmp(EXPECTED_STATE_PREFIX,
                               notifyRequest->
                                   getHeaderValue(0, SIP_SUBSCRIPTION_STATE_FIELD),
                               strlen(EXPECTED_STATE_PREFIX)));

               // Check that we received a second NOTIFY, in which the server
               // notifies us that the subscription has been deactiveted
               // (due to an error response to the first NOTIFY).

               CPPUNIT_ASSERT_MESSAGE(message, notifyRequest2);
               ASSERT_STR_EQUAL_MESSAGE(message,
                                        "terminated;reason=deactivated",
                                        notifyRequest2->
                                            getHeaderValue(0, SIP_SUBSCRIPTION_STATE_FIELD));
            }

            // Wait for the subscription to be ended.
            OsTask::delay(100);

            // Send a re-SUBSCRIBE in the existing dialog, to find out if the
            // subscription was terminated or not.
            mwiSubscribeRequest.incrementCSeqNumber();
            // Leave the Expires header with the default value.

            CPPUNIT_ASSERT_MESSAGE(message,
                                   userAgentp->send(mwiSubscribeRequest));

            // We should get a 481 response and no NOTIFY, because the
            // subscription has been terminated.
            {
               const SipMessage* subscribeResponse;
               const SipMessage* notifyRequest;
               CPPUNIT_ASSERT(runListener(incomingClientMsgQueue,
                                          *userAgentp,
                                          messageTimeout,
                                          messageTimeout,
                                          notifyRequest,
                                          subscribeResponse,
                                          SIP_OK_CODE,
                                          FALSE,
                                          0,
                                          NULL));

               // We should have received a SUBSCRIBE response and no NOTIFY request.
               CPPUNIT_ASSERT_MESSAGE(message, subscribeResponse);

               CPPUNIT_ASSERT_EQUAL_MESSAGE(message,
                                            SIP_BAD_TRANSACTION_CODE,
                                            subscribeResponse->getResponseStatusCode());
               CPPUNIT_ASSERT(!notifyRequest);
            }
         }
      }

   // Another test for XECS-282:
   // Verify that subscription is *not* terminated when NOTIFY returns an
   // error responses containing "Retry-After: 0" header.
   // See RFC 3265 section 3.2.2
   void terminateSubscriptionOnErrorRetryAfter()
      {
         // Test MWI messages
         const char* mwiSubscribe =
            "SUBSCRIBE sip:111@localhost SIP/2.0\r\n"
            "From: <sip:111@example.com>;tag=1612c1612\r\n"
            "To: <sip:111@example.com>\r\n"
            "Cseq: 1 SUBSCRIBE\r\n"
            "Event: message-summary\r\n"
            "Accept: application/simple-message-summary\r\n"
            "Expires: 3600\r\n"
            "Date: Tue, 26 Apr 2005 14:59:30 GMT\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: Pingtel/2.2.0 (VxWorks)\r\n"
            "Accept-Language: en\r\n"
            "Supported: sip-cc, sip-cc-01, timer, replaces\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         // Send a SUBSCRIBE to ourselves
         SipMessage mwiSubscribeRequest(mwiSubscribe);
         {
            UtlString c;
            CallId::getNewCallId(c);
            mwiSubscribeRequest.setCallIdField(c);
         }
         mwiSubscribeRequest.setSipRequestFirstHeaderLine(SIP_SUBSCRIBE_METHOD,
                                                          aor,
                                                          SIP_PROTOCOL_VERSION);
         mwiSubscribeRequest.setContactField(aor_name_addr);

         // Loop through this scenario for a series of response codes.
         int test_codes[] = { SIP_SERVER_INTERNAL_ERROR_CODE };
         for (unsigned int i = 0;
              i < sizeof (test_codes) / sizeof (test_codes[0]);
              i++)
         {
            mwiSubscribeRequest.incrementCSeqNumber();
            CPPUNIT_ASSERT(userAgentp->send(mwiSubscribeRequest));

            // We should get a 202 response and a NOTIFY request in the queue
            // Send the specified response to the NOTIFY.
            OsTime messageTimeout(1, 0);  // 1 second
            {
               const SipMessage* subscribeResponse;
               const SipMessage* notifyRequest;
               CPPUNIT_ASSERT(runListener(incomingClientMsgQueue,
                                          *userAgentp,
                                          messageTimeout,
                                          messageTimeout,
                                          notifyRequest,
                                          subscribeResponse,
                                          test_codes[i],
                                          /*Retry-After:0*/ TRUE,
                                          0,
                                          NULL));

               // We should have received a SUBSCRIBE response and a NOTIFY request.
               CPPUNIT_ASSERT(subscribeResponse);
               CPPUNIT_ASSERT(notifyRequest);

               char message[100];
               sprintf(message, "test_codes[%d] = %d", i, test_codes[i]);
               CPPUNIT_ASSERT_EQUAL_MESSAGE(message,
                                            SIP_ACCEPTED_CODE,
                                            subscribeResponse->getResponseStatusCode());

               // Extract the to-tag in the response, apply it to mwiSubscribeRequest.
               // This allows the re-SUBSCRIBE below to be applied to the existing dialog.
               Url toUrl;
               subscribeResponse->getToUrl(toUrl);
               UtlString toTag;
               toUrl.getFieldParameter("tag", toTag);
               mwiSubscribeRequest.setToFieldTag(toTag);
            }

            // Send a re-SUBSCRIBE in the existing dialog, to find out if the
            // subscription was terminated or not.
            mwiSubscribeRequest.incrementCSeqNumber();
            // Leave the Expires header with the default value.

            CPPUNIT_ASSERT(userAgentp->send(mwiSubscribeRequest));

            // We should get a 202 response and a NOTIFY, because the Retry-After
            // header suppresses the termination of the subscription.
            {
               const SipMessage* subscribeResponse;
               const SipMessage* notifyRequest;
               CPPUNIT_ASSERT(runListener(incomingClientMsgQueue,
                                          *userAgentp,
                                          messageTimeout,
                                          messageTimeout,
                                          notifyRequest,
                                          subscribeResponse,
                                          SIP_OK_CODE,
                                          FALSE,
                                          0,
                                          NULL));

               // We should have received a SUBSCRIBE response and no NOTIFY request.
               CPPUNIT_ASSERT(subscribeResponse);
               CPPUNIT_ASSERT(notifyRequest);

               char message[100];
               sprintf(message, "test_codes[%d] = %d", i, test_codes[i]);
               CPPUNIT_ASSERT_EQUAL_MESSAGE(message,
                                            SIP_ACCEPTED_CODE,
                                            subscribeResponse->getResponseStatusCode());
            }
         }
      }

   // XECS-1810: Verify that subscription is *not* terminated when NOTIFY
   // returns a Timeout error.
   // This is extended to cover not only 408 errors, but also 482 and 500,
   // as those also can be caused by slow network or slow subscribers.
   void terminateSubscriptionOnErrorTimeout()
      {
         // Test MWI messages
         const char* mwiSubscribe =
            "SUBSCRIBE sip:111@localhost SIP/2.0\r\n"
            "From: <sip:111@example.com>;tag=1612c1612\r\n"
            "To: <sip:111@example.com>\r\n"
            "Cseq: 1 SUBSCRIBE\r\n"
            "Event: message-summary\r\n"
            "Accept: application/simple-message-summary\r\n"
            "Expires: 3600\r\n"
            "Date: Tue, 4 Nov 2008 15:59:30 GMT\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: Pingtel/2.2.0 (VxWorks)\r\n"
            "Accept-Language: en\r\n"
            "Supported: sip-cc, sip-cc-01, timer, replaces\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         // Loop through this scenario for a series of response codes.
         int test_codes[] = { SIP_REQUEST_TIMEOUT_CODE,
                              SIP_LOOP_DETECTED_CODE,
                              SIP_SERVER_INTERNAL_ERROR_CODE };
         for (unsigned int i = 0;
              i < sizeof (test_codes) / sizeof (test_codes[0]);
              i++)
         {
            // Send a SUBSCRIBE to ourselves
            SipMessage mwiSubscribeRequest(mwiSubscribe);
            {
               UtlString c;
               CallId::getNewCallId(c);
               mwiSubscribeRequest.setCallIdField(c);
            }
            mwiSubscribeRequest.setSipRequestFirstHeaderLine(SIP_SUBSCRIBE_METHOD,
                                                             aor,
                                                             SIP_PROTOCOL_VERSION);
            mwiSubscribeRequest.setContactField(aor_name_addr);
            mwiSubscribeRequest.incrementCSeqNumber();

            CPPUNIT_ASSERT(userAgentp->send(mwiSubscribeRequest));

            // We should get a 202 response and a NOTIFY request in the queue
            // Send a Timeout error response to the NOTIFY.
            OsTime messageTimeout(1, 0);  // 1 second
            {
               const SipMessage* subscribeResponse;
               const SipMessage* notifyRequest;
               CPPUNIT_ASSERT(runListener(incomingClientMsgQueue,
                                          *userAgentp,
                                          messageTimeout,
                                          messageTimeout,
                                          notifyRequest,
                                          subscribeResponse,
                                          test_codes[i],
                                          FALSE,
                                          0,
                                          NULL));

               // We should have received a SUBSCRIBE response and a NOTIFY request.
               CPPUNIT_ASSERT(subscribeResponse);
               char message[100];
               sprintf(message, "test_codes[%d] = %d", i, test_codes[i]);
               CPPUNIT_ASSERT_EQUAL_MESSAGE(message,
                                            SIP_ACCEPTED_CODE,
                                            subscribeResponse->getResponseStatusCode());
               CPPUNIT_ASSERT(notifyRequest);

               // Extract the to-tag in the response, apply it to mwiSubscribeRequest.
               // This allows the re-SUBSCRIBE below to be applied to the existing dialog.
               Url toUrl;
               subscribeResponse->getToUrl(toUrl);
               UtlString toTag;
               toUrl.getFieldParameter("tag", toTag);
               mwiSubscribeRequest.setToFieldTag(toTag);
            }

            // Send a re-SUBSCRIBE in the existing dialog, to find out if the
            // subscription was terminated or not.
            mwiSubscribeRequest.incrementCSeqNumber();
            // Leave the Expires header with the default value.

            CPPUNIT_ASSERT(userAgentp->send(mwiSubscribeRequest));

            // We should get a 202 response and a NOTIFY, because the Timeout
            // error suppresses the termination of the subscription.
            {
               const SipMessage* subscribeResponse;
               const SipMessage* notifyRequest;
               CPPUNIT_ASSERT(runListener(incomingClientMsgQueue,
                                          *userAgentp,
                                          messageTimeout,
                                          messageTimeout,
                                          notifyRequest,
                                          subscribeResponse,
                                          SIP_OK_CODE,
                                          FALSE,
                                          0,
                                          NULL));

               // We should have received a SUBSCRIBE response and no NOTIFY request.
               CPPUNIT_ASSERT(subscribeResponse);
               CPPUNIT_ASSERT(notifyRequest);
               CPPUNIT_ASSERT_EQUAL(SIP_ACCEPTED_CODE,
                                    subscribeResponse->getResponseStatusCode());
            }
         }
      }

   // Check the Contact header of responses to SUBSCRIBE.
   // Check the Contact header of NOTIFY.
   // XECS-297 and XECS-298.
   void responseContact()
      {
         // Test MWI messages
         const char* mwiSubscribe =
            "SUBSCRIBE sip:111@localhost SIP/2.0\r\n"
            "From: <sip:111@example.com>;tag=1612c1612\r\n"
            "To: <sip:111@example.com>\r\n"
            "Cseq: 1 SUBSCRIBE\r\n"
            "Event: message-summary\r\n"
            "Accept: application/simple-message-summary\r\n"
            "Expires: 3600\r\n"
            "Date: Tue, 26 Apr 2005 14:59:30 GMT\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: Pingtel/2.2.0 (VxWorks)\r\n"
            "Accept-Language: en\r\n"
            "Supported: sip-cc, sip-cc-01, timer, replaces\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         // Send a SUBSCRIBE to ourselves
         SipMessage mwiSubscribeRequest(mwiSubscribe);
         {
            UtlString c;
            CallId::getNewCallId(c);
            mwiSubscribeRequest.setCallIdField(c);
         }
         mwiSubscribeRequest.setSipRequestFirstHeaderLine(SIP_SUBSCRIBE_METHOD,
                                                          aor,
                                                          SIP_PROTOCOL_VERSION);
         mwiSubscribeRequest.setContactField(aor_name_addr);

         CPPUNIT_ASSERT(userAgentp->send(mwiSubscribeRequest));

         // We should get a 202 response and a NOTIFY request in the queue
         // Send the specified response to the NOTIFY.
         OsTime messageTimeout(1, 0);  // 1 second
         {
            const SipMessage* subscribeResponse;
            const SipMessage* notifyRequest;
            CPPUNIT_ASSERT(runListener(incomingClientMsgQueue,
                                       *userAgentp,
                                       messageTimeout,
                                       messageTimeout,
                                       notifyRequest,
                                       subscribeResponse,
                                       SIP_OK_CODE,
                                       FALSE,
                                       0,
                                       NULL));

            // We should have received a SUBSCRIBE response and a NOTIFY request.
            CPPUNIT_ASSERT(subscribeResponse);
            CPPUNIT_ASSERT(notifyRequest);

            CPPUNIT_ASSERT_EQUAL(SIP_ACCEPTED_CODE,
                                 subscribeResponse->getResponseStatusCode());

            // Check the Contact headers.
            UtlString c;
            subscribeResponse->getContactField(0, c);
            ASSERT_STR_EQUAL(aor_contact_name_addr, c.data());
            notifyRequest->getContactField(0, c);
            ASSERT_STR_EQUAL(aor_contact_name_addr, c.data());
         }
      }

   void notifyResend()
      {
         // Test MWI messages
         const char* mwiSubscribe =
            "SUBSCRIBE sip:111@localhost SIP/2.0\r\n"
            "From: <sip:111@example.com>;tag=1612c1612\r\n"
            "To: <sip:111@example.com>\r\n"
            "Cseq: 1 SUBSCRIBE\r\n"
            "Event: message-summary\r\n"
            "Accept: application/simple-message-summary\r\n"
            "Expires: 3600\r\n"
            "Date: Tue, 26 Apr 2005 14:59:30 GMT\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: Pingtel/2.2.0 (VxWorks)\r\n"
            "Accept-Language: en\r\n"
            "Supported: sip-cc, sip-cc-01, timer, replaces\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         // Send a SUBSCRIBE to ourselves
         SipMessage mwiSubscribeRequest(mwiSubscribe);
         {
            UtlString c;
            CallId::getNewCallId(c);
            mwiSubscribeRequest.setCallIdField(c);
         }
         mwiSubscribeRequest.setSipRequestFirstHeaderLine(SIP_SUBSCRIBE_METHOD,
                                                          aor,
                                                          SIP_PROTOCOL_VERSION);
         mwiSubscribeRequest.setContactField(aor_name_addr);
         mwiSubscribeRequest.incrementCSeqNumber();

         CPPUNIT_ASSERT(userAgentp->send(mwiSubscribeRequest));

         // We should get a 202 response and a NOTIFY request in the queue
         // Send a 408 response to the NOTIFY.
         {
            OsTime messageTimeout(1, 0);  // 1 second
            const SipMessage* subscribeResponse;
            const SipMessage* notifyRequest;
            CPPUNIT_ASSERT(runListener(incomingClientMsgQueue,
                                       *userAgentp,
                                       messageTimeout,
                                       messageTimeout,
                                       notifyRequest,
                                       subscribeResponse,
                                       SIP_REQUEST_TIMEOUT_CODE,
                                       FALSE,
                                       0,
                                       NULL));

            // We should have received a SUBSCRIBE response and a NOTIFY request.
            CPPUNIT_ASSERT(subscribeResponse);

            // Check that the subscribe response is what we expected.
            CPPUNIT_ASSERT_EQUAL(SIP_ACCEPTED_CODE,
                                 subscribeResponse->getResponseStatusCode());

            // Check that the NOTIFY is what we expected.
            CPPUNIT_ASSERT(notifyRequest);
         }

         // Wait 10 seconds for the NOTIFY to be resent.
         {
            fprintf(stderr, "Waiting 11 seconds...\n");
            OsTime firstTimeout(11, 0);  // 11 seconds
            OsTime nextTimeout(1, 0);  // 1 second
            const SipMessage* subscribeResponse;
            const SipMessage* notifyRequest;
            CPPUNIT_ASSERT(runListener(incomingClientMsgQueue,
                                       *userAgentp,
                                       firstTimeout,
                                       nextTimeout,
                                       notifyRequest,
                                       subscribeResponse,
                                       SIP_REQUEST_TIMEOUT_CODE,
                                       FALSE,
                                       0,
                                       NULL));

            // We should not have received a response.
            CPPUNIT_ASSERT(!subscribeResponse);

            // Check that the NOTIFY is what we expected.
            CPPUNIT_ASSERT(notifyRequest);
         }

         // Wait 20 seconds for the NOTIFY to be resent.
         // Give 200 response this time.
         {
            fprintf(stderr, "Waiting 21 seconds...\n");
            OsTime firstTimeout(21, 0);  // 21 seconds
            OsTime nextTimeout(1, 0);  // 1 second
            const SipMessage* subscribeResponse;
            const SipMessage* notifyRequest;
            CPPUNIT_ASSERT(runListener(incomingClientMsgQueue,
                                       *userAgentp,
                                       firstTimeout,
                                       nextTimeout,
                                       notifyRequest,
                                       subscribeResponse,
                                       SIP_OK_CODE,
                                       FALSE,
                                       0,
                                       NULL));

            // We should not have received a response.
            CPPUNIT_ASSERT(!subscribeResponse);

            // Check that the NOTIFY is what we expected.
            CPPUNIT_ASSERT(notifyRequest);
         }

         // Wait 40 seconds for the NOTIFY to be resent, which it should not be.
         {
            fprintf(stderr, "Waiting 41 seconds...\n");
            OsTime firstTimeout(41, 0);  // 41 seconds
            OsTime nextTimeout(1, 0);  // 1 second
            const SipMessage* subscribeResponse;
            const SipMessage* notifyRequest;
            CPPUNIT_ASSERT(runListener(incomingClientMsgQueue,
                                       *userAgentp,
                                       firstTimeout,
                                       nextTimeout,
                                       notifyRequest,
                                       subscribeResponse,
                                       SIP_REQUEST_TIMEOUT_CODE,
                                       FALSE,
                                       0,
                                       NULL));

            // We should not have received a response.
            CPPUNIT_ASSERT(!subscribeResponse);

            // We should not have received a NOTIFY request.
            CPPUNIT_ASSERT(!notifyRequest);
         }
      }

   // Test sending full vs. partial content.
   void fullVsPartial()
      {
         // Enable a new event type which allows partial as well as full content.
         UtlString eventName("x-test");
         UtlString mimeType("application/x-test");
         subServerp->enableEventType(eventName, NULL, NULL, NULL,
                                     SipSubscribeServer::standardVersionCallback,
                                     FALSE);
         // Register an interest in SUBSCRIBE responses and NOTIFY requests
         // for this event type
         // Do not add observer for SUBSCRIBE responses, because one was added
         // in ::setUp(), and despite appearances, the response observers do not
         // filter on event type.
         userAgentp->addMessageObserver(incomingClientMsgQueue,
                                        SIP_NOTIFY_METHOD,
                                        TRUE, // requests
                                        FALSE, // not reponses
                                        TRUE, // incoming
                                        FALSE, // no outgoing
                                        eventName,
                                        NULL,
                                        NULL);

         // Publish full and partial content.
         const char* full_content = "Full content";
         const char* partial_content = "Partial content";
         SipPublishContentMgr* pubMgrp = subServerp->getPublishMgr(eventName);
         // Publish full content first.
         // We have to copy these bodies every time we pass them to 
         // SipPublishContentMgr::publish(), because ::publish takes ownership
         // of its input bodies.
         HttpBody full_body(full_content, strlen(full_content), mimeType);
         HttpBody partial_body(partial_content, strlen(partial_content), mimeType);
         HttpBody* b;
         pubMgrp->publish(resource_id,
                          eventName,
                          eventName,
                          1,
                          (b = new HttpBody(full_body), &b),
                          TRUE,
                          TRUE);
         pubMgrp->publish(resource_id,
                          eventName,
                          eventName,
                          1,
                          (b = new HttpBody(partial_body), &b),
                          FALSE);

         // Send a SUBSCRIBE to ourselves and receive full content.

         const char* subscribe =
            "SUBSCRIBE sip:111@localhost SIP/2.0\r\n"
            "From: <sip:111@example.com>;tag=1612c1612\r\n"
            "To: <sip:111@example.com>\r\n"
            "Cseq: 1 SUBSCRIBE\r\n"
            "Event: x-test\r\n"
            "Accept: application/x-test\r\n"
            "Expires: 3600\r\n"
            "Date: Tue, 26 Apr 2005 14:59:30 GMT\r\n"
            "Max-Forwards: 20\r\n"
            "User-Agent: Pingtel/2.2.0 (VxWorks)\r\n"
            "Accept-Language: en\r\n"
            "Supported: sip-cc, sip-cc-01, timer, replaces\r\n"
            "Content-Length: 0\r\n"
            "\r\n";

         SipMessage subscribeRequest(subscribe);
         {
            UtlString c;
            CallId::getNewCallId(c);
            subscribeRequest.setCallIdField(c);
         }
         subscribeRequest.setSipRequestFirstHeaderLine(SIP_SUBSCRIBE_METHOD,
                                                       aor,
                                                       SIP_PROTOCOL_VERSION);
         subscribeRequest.setContactField(aor_name_addr);
         subscribeRequest.incrementCSeqNumber();

         CPPUNIT_ASSERT(userAgentp->send(subscribeRequest));

         // We should get a 202 response and a NOTIFY request in the queue
         // Send a 200 response to the NOTIFY.
         {
            OsTime messageTimeout(1, 0);  // 1 second
            const SipMessage* subscribeResponse;
            const SipMessage* notifyRequest;
            CPPUNIT_ASSERT(runListener(incomingClientMsgQueue,
                                       *userAgentp,
                                       messageTimeout,
                                       messageTimeout,
                                       notifyRequest,
                                       subscribeResponse,
                                       SIP_OK_CODE,
                                       FALSE,
                                       0,
                                       NULL));

            // We should have received a SUBSCRIBE response and a NOTIFY request.
            CPPUNIT_ASSERT(subscribeResponse);

            // Check that the subscribe response is what we expected.
            CPPUNIT_ASSERT_EQUAL(SIP_ACCEPTED_CODE,
                                 subscribeResponse->getResponseStatusCode());

            // Check that the NOTIFY has full content.
            CPPUNIT_ASSERT(notifyRequest);
            ASSERT_STR_EQUAL(full_content, notifyRequest->getBody()->getBytes());

            // Update the to-tag of the SUBSCRIBE.
            Url responseToField;
            subscribeResponse->getToUrl(responseToField);
            UtlString toTag;
            responseToField.getFieldParameter("tag", toTag);
            subscribeRequest.setToFieldTag(toTag);
         }

         // Update the content and see if we get a partial-content NOTIFY.

         pubMgrp->publish(resource_id,
                          eventName,
                          eventName,
                          1,
                          (b = new HttpBody(full_body), &b),
                          TRUE,
                          TRUE);
         pubMgrp->publish(resource_id,
                          eventName,
                          eventName,
                          1,
                          (b = new HttpBody(partial_body), &b),
                          FALSE);

         // We should get a NOTIFY request in the queue
         // Send a 200 response to the NOTIFY.
         {
            OsTime messageTimeout(1, 0);  // 1 second
            const SipMessage* subscribeResponse;
            const SipMessage* notifyRequest;
            CPPUNIT_ASSERT(runListener(incomingClientMsgQueue,
                                       *userAgentp,
                                       messageTimeout,
                                       messageTimeout,
                                       notifyRequest,
                                       subscribeResponse,
                                       SIP_OK_CODE,
                                       FALSE,
                                       0,
                                       NULL));

            // Check that there is no subscribe response.
            CPPUNIT_ASSERT(!subscribeResponse);

            // Check that the NOTIFY has partial content.
            CPPUNIT_ASSERT(notifyRequest);
            ASSERT_STR_EQUAL(partial_content, notifyRequest->getBody()->getBytes());
         }

         // Re-SUBSCRIBE and see if we get full content.

         subscribeRequest.incrementCSeqNumber();

         CPPUNIT_ASSERT(userAgentp->send(subscribeRequest));

         // We should get a 202 response and a NOTIFY request in the queue
         // Send a 200 response to the NOTIFY.
         {
            OsTime messageTimeout(1, 0);  // 1 second
            const SipMessage* subscribeResponse;
            const SipMessage* notifyRequest;
            CPPUNIT_ASSERT(runListener(incomingClientMsgQueue,
                                       *userAgentp,
                                       messageTimeout,
                                       messageTimeout,
                                       notifyRequest,
                                       subscribeResponse,
                                       SIP_OK_CODE,
                                       FALSE,
                                       0,
                                       NULL));

            // We should have received a SUBSCRIBE response and a NOTIFY request.
            CPPUNIT_ASSERT(subscribeResponse);

            // Check that the subscribe response is what we expected.
            CPPUNIT_ASSERT_EQUAL(SIP_ACCEPTED_CODE,
                                 subscribeResponse->getResponseStatusCode());

            // Check that the NOTIFY has full content.
            CPPUNIT_ASSERT(notifyRequest);
            ASSERT_STR_EQUAL(full_content, notifyRequest->getBody()->getBytes());
         }

         // Publish content and receive a partial-content NOTIFY.
         // Send a failure response to the NOTIFY to cause a re-send NOTIFY.

         pubMgrp->publish(resource_id,
                          eventName,
                          eventName,
                          1,
                          (b = new HttpBody(full_body), &b),
                          TRUE,
                          TRUE);
         pubMgrp->publish(resource_id,
                          eventName,
                          eventName,
                          1,
                          (b = new HttpBody(partial_body), &b),
                          FALSE);

         // We should get and a NOTIFY request in the queue
         // Send a 200 response to the NOTIFY.
         {
            OsTime messageTimeout(1, 0);  // 1 second
            const SipMessage* subscribeResponse;
            const SipMessage* notifyRequest;
            CPPUNIT_ASSERT(runListener(incomingClientMsgQueue,
                                       *userAgentp,
                                       messageTimeout,
                                       messageTimeout,
                                       notifyRequest,
                                       subscribeResponse,
                                       SIP_REQUEST_TIMEOUT_CODE,
                                       FALSE,
                                       0,
                                       NULL));

            // We should not have received a SUBSCRIBE response.
            CPPUNIT_ASSERT(!subscribeResponse);

            // Check that the NOTIFY has partial content.
            CPPUNIT_ASSERT(notifyRequest);
            ASSERT_STR_EQUAL(partial_content, notifyRequest->getBody()->getBytes());
         }

         // Wait 10 seconds for the NOTIFY to be resent.
         // Respond 200 and check that it is full content.
         {
            fprintf(stderr, "Waiting 11 seconds...\n");
            OsTime firstTimeout(11, 0);  // 11 seconds
            OsTime nextTimeout(1, 0);  // 1 second
            const SipMessage* subscribeResponse;
            const SipMessage* notifyRequest;
            CPPUNIT_ASSERT(runListener(incomingClientMsgQueue,
                                       *userAgentp,
                                       firstTimeout,
                                       nextTimeout,
                                       notifyRequest,
                                       subscribeResponse,
                                       SIP_OK_CODE,
                                       FALSE,
                                       0,
                                       NULL));

            // We should not have received a response.
            CPPUNIT_ASSERT(!subscribeResponse);

            // Check that the NOTIFY is what we expected.
            CPPUNIT_ASSERT(notifyRequest);
            ASSERT_STR_EQUAL(full_content, notifyRequest->getBody()->getBytes());
         }

         // Update the content and see if we get a partial-content NOTIFY.

         pubMgrp->publish(resource_id,
                          eventName,
                          eventName,
                          1,
                          (b = new HttpBody(full_body), &b),
                          TRUE,
                          TRUE);
         pubMgrp->publish(resource_id,
                          eventName,
                          eventName,
                          1,
                          (b = new HttpBody(partial_body), &b),
                          FALSE);

         // We should get a NOTIFY request in the queue.
         // Send a 200 response to the NOTIFY.
         {
            OsTime messageTimeout(1, 0);  // 1 second
            const SipMessage* subscribeResponse;
            const SipMessage* notifyRequest;
            CPPUNIT_ASSERT(runListener(incomingClientMsgQueue,
                                       *userAgentp,
                                       messageTimeout,
                                       messageTimeout,
                                       notifyRequest,
                                       subscribeResponse,
                                       SIP_OK_CODE,
                                       FALSE,
                                       0,
                                       NULL));

            // Check that there is no subscribe response.
            CPPUNIT_ASSERT(!subscribeResponse);

            // Check that the NOTIFY has partial content.
            CPPUNIT_ASSERT(notifyRequest);
            ASSERT_STR_EQUAL(partial_content, notifyRequest->getBody()->getBytes());
         }

         // Publish content and receive a partial-content NOTIFY.
         // Send a failure response to the NOTIFY to cause a re-send NOTIFY.

         pubMgrp->publish(resource_id,
                          eventName,
                          eventName,
                          1,
                          (b = new HttpBody(full_body), &b),
                          TRUE,
                          TRUE);
         pubMgrp->publish(resource_id,
                          eventName,
                          eventName,
                          1,
                          (b = new HttpBody(partial_body), &b),
                          FALSE);

         // We should get and a NOTIFY request in the queue
         // Send a 200 response to the NOTIFY.
         {
            OsTime messageTimeout(1, 0);  // 1 second
            const SipMessage* subscribeResponse;
            const SipMessage* notifyRequest;
            CPPUNIT_ASSERT(runListener(incomingClientMsgQueue,
                                       *userAgentp,
                                       messageTimeout,
                                       messageTimeout,
                                       notifyRequest,
                                       subscribeResponse,
                                       SIP_REQUEST_TIMEOUT_CODE,
                                       FALSE,
                                       0,
                                       NULL));

            // We should not have received a SUBSCRIBE response.
            CPPUNIT_ASSERT(!subscribeResponse);

            // Check that the NOTIFY has partial content.
            CPPUNIT_ASSERT(notifyRequest);
            ASSERT_STR_EQUAL(partial_content, notifyRequest->getBody()->getBytes());
         }

         // While the re-send of the last NOTIFY is pending:
         // Publish content and receive a full-content NOTIFY.
         // Send a success response to the NOTIFY.

         pubMgrp->publish(resource_id,
                          eventName,
                          eventName,
                          1,
                          (b = new HttpBody(full_body), &b),
                          TRUE,
                          TRUE);
         pubMgrp->publish(resource_id,
                          eventName,
                          eventName,
                          1,
                          (b = new HttpBody(partial_body), &b),
                          FALSE);

         // We should get a NOTIFY request in the queue.
         // Send a 200 response to the NOTIFY.
         {
            OsTime messageTimeout(1, 0);  // 1 second
            const SipMessage* subscribeResponse;
            const SipMessage* notifyRequest;
            CPPUNIT_ASSERT(runListener(incomingClientMsgQueue,
                                       *userAgentp,
                                       messageTimeout,
                                       messageTimeout,
                                       notifyRequest,
                                       subscribeResponse,
                                       SIP_OK_CODE,
                                       FALSE,
                                       0,
                                       NULL));

            // We should not have received a SUBSCRIBE response.
            CPPUNIT_ASSERT(!subscribeResponse);

            // Check that the NOTIFY has partial content.
            CPPUNIT_ASSERT(notifyRequest);
            ASSERT_STR_EQUAL(full_content, notifyRequest->getBody()->getBytes());
         }

         // Wait 10 seconds for the NOTIFY to be resent.
         // Respond 200 and check that it is partial content.
         // (We expect partial content because the last NOTIFY sent got a success
         // response.)
         {
            fprintf(stderr, "Waiting 11 seconds...\n");
            OsTime firstTimeout(11, 0);  // 11 seconds
            OsTime nextTimeout(1, 0);  // 1 second
            const SipMessage* subscribeResponse;
            const SipMessage* notifyRequest;
            CPPUNIT_ASSERT(runListener(incomingClientMsgQueue,
                                       *userAgentp,
                                       firstTimeout,
                                       nextTimeout,
                                       notifyRequest,
                                       subscribeResponse,
                                       SIP_REQUEST_TIMEOUT_CODE,
                                       FALSE,
                                       0,
                                       NULL));

            // We should not have received a response.
            CPPUNIT_ASSERT(!subscribeResponse);

            // Check that the NOTIFY is what we expected.
            CPPUNIT_ASSERT(notifyRequest);
            ASSERT_STR_EQUAL(partial_content, notifyRequest->getBody()->getBytes());
         }

         // Clean up to prevent use of the queue after it goes out of scope.
         userAgentp->removeMessageObserver(incomingClientMsgQueue);
         userAgentp->removeMessageObserver(incomingClientMsgQueue);
      }

};

CPPUNIT_TEST_SUITE_REGISTRATION(SipSubscribeServerTest2);
