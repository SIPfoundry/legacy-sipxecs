//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// Get value of EXECUTE_SLOW_TESTS.
#include "config.h"

#include <stdio.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>
#include <utl/UtlHashMap.h>
#include <utl/UtlString.h>
#include <os/OsDefs.h>
#include <os/OsDateTime.h>
#include <os/OsSysLog.h>
#include <net/SipDialog.h>
#include <net/SipMessage.h>
#include <net/SipUserAgent.h>
#include <net/SipDialogMgr.h>
#include <net/SipSubscriptionMgr.h>
#include <net/SipSubscribeClient.h>
#include <net/SipSubscribeServer.h>
#include <net/SipPublishContentMgr.h>
#include <net/CallId.h>

#include "SipSubscribeTestSupport.h"

/**
 * Unit tests for SipSubscribeClient
 */

// Static variables to communicate with the callback routines.

// Notify affected states:
static int smNumClientNotifiesReceived;
static SipMessage* smLastClientNotifyReceived;
static UtlString smClientNotifyEarlyDialog;
static UtlString smClientNotifyEstablishedDialog;

// Subscribe affected states
static int smNumClientSubResponsesReceived;
static long smClientExpiration;
static UtlString smClientSubEarlyDialog;
static UtlString smClientSubEstablishedDialog;
static SipMessage* smLastClientSubResponseReceived;

// NOTIFY callback.
static bool notifyCallback(const char* earlyDialogHandle,
                           const char* dialogHandle,
                           void* applicationData,
                           const SipMessage* notifyRequest)
   {
      //printf("notifyCallback: %d\n", smNumClientNotifiesReceived);
      smNumClientNotifiesReceived++;
      if(smLastClientNotifyReceived)
      {
         // Don't delete as we take the pointer rather than copy
         // in the unit tests.
         //delete smLastClientNotifyReceived;
      }
      smLastClientNotifyReceived = new SipMessage(*notifyRequest);
      smClientNotifyEarlyDialog = earlyDialogHandle;
      smClientNotifyEstablishedDialog = dialogHandle;
      return true;
   };

// SUBSCRIBE callback.
static void subStateCallback(SipSubscribeClient::SubscriptionState newState,
                             const char* earlyDialogHandle,
                             const char* dialogHandle,
                             void* applicationData,
                             int responseCode,
                             const char* responseText,
                             long expiration,
                             const SipMessage* subscribeResponse)
   {
      smNumClientSubResponsesReceived++;
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "subStateCallback: smNumClientSubResponsesReceived=%d", smNumClientSubResponsesReceived);
      smClientExpiration = expiration;
      if(smLastClientSubResponseReceived)
      {
         // Don't delete as we take the pointer rather than copy in the
         // unit tests
         //delete smLastClientSubResponseReceived;
      }
      if(subscribeResponse)
      {
         smLastClientSubResponseReceived = new SipMessage(*subscribeResponse);
      }
      smClientSubEarlyDialog = earlyDialogHandle;
      smClientSubEstablishedDialog = dialogHandle;
   };

class SipSubscribeClientTest1 : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SipSubscribeClientTest1);
   CPPUNIT_TEST(subscribeHandleTest);
   CPPUNIT_TEST_SUITE_END();

public:

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

   // Function to compare dialog handles -- returns true if they are the same.
   // Dialog handles should be presented consistently, but they are not.
   // (See XECS-252.)  Therefore, this function dissects handles and compares
   // them both directly and with the tags reversed.  Once XECS-252 is fixed,
   // this can be replaced with strcmp().
   UtlBoolean compareHandles(const char *a,
                             const char* b)
      {
         UtlBoolean result;

         if (strcmp(a, b) == 0)
         {
            result = TRUE;
         }
         else
         {
            // Cut each handle into the Call-Id, to-tag, and from-tag.
            const char* a1 = strchr(a, ',');
            if (!a1)
            {
               result = FALSE;
            }
            else
            {
               const char* a2 = strchr(a1+1, ',');
               if (!a2)
               {
                  result = FALSE;
               }
               else
               {
                  const char* b1 = strchr(b, ',');
                  if (!b1)
                  {
                     result = FALSE;
                  }
                  else
                  {
                     const char* b2 = strchr(b1+1, ',');
                     if (!b2)
                     {
                        result = FALSE;
                     }
                     else
                     {
                        // The first segments have to be the same.
                        ssize_t n;
                        n = a1 - a;
                        if (b1 - b == n && strncmp(a, b, n) == 0)
                        {
                           // Cross-compare the second and third segments.
                           n = a2 - (a1+1);
                           if ((ssize_t)strlen(b2+1) == n && strncmp(a1+1, b2+1, n) == 0)
                           {
                              n = strlen(a2+1);
                              result = b2 - (b1+1) == n && strncmp(a2+1, b1+1, n) == 0;
                           }
                           else
                           {
                              result = FALSE;
                           }
                        }
                        else
                        {
                           result = FALSE;
                        }
                     }
                  }
               }
            }
         }

         return result;
      }

   // Variables used by setUp/tearDown and the tests.

   UtlString hostIp;
   SipUserAgent* userAgentp;
   UtlString notifier_addr_spec;
   UtlString notifier_name_addr;
   UtlString subscriber_addr_spec;
   UtlString subscriber_name_addr;
   UtlString eventTypeKey;
   UtlString eventType;
   SipDialogMgr* clientDialogMgrp;
   SipRefreshManager* refreshMgrp;
   SipSubscribeClient* subClientp;
   SipSubscribeServer* subServerp;
   SipSubscriptionMgr* subMgrp;

   void setUp()
      {
         hostIp = "127.0.0.1";
         eventTypeKey = "message-summary";
         eventType = eventTypeKey;

         // defaultSipAddress is set to force the SipUserAgent to open
         // ports on that interface.
         // publicAddress is set to force the SipUserAgent to report
         // that as its address.
         userAgentp = new SipUserAgent(PORT_DEFAULT, PORT_NONE, PORT_NONE,
                                       hostIp, NULL, hostIp);
         userAgentp->start();

         // Construct the URI of the notifier.
         // Also construct the name-addr version of the URI, which may be
         // different if it has a "transport" parameter.
         // And the resource-id to use, which is the AOR with any
         // parameters stripped off.
         {
            char buffer[100];
            sprintf(buffer, "sip:222@%s:%d", hostIp.data(),
                    userAgentp->getTcpPort());

            // Specify TCP transport, because that's all the UA listens to.
            // Using an address with a transport parameter also exercises
            // SipDialog to ensure it handles contact addresses with parameters.
            strcat(buffer, ";transport=tcp");
            notifier_addr_spec = buffer;

            Url notifier_uri(buffer, TRUE);
            // Don't set display-name, because it will be lost when converted
            // to addr-spec, making some of the tests harder to do.
            notifier_uri.toString(notifier_name_addr);
         }

         // Construct the URI of the subscriber.
         // Also construct the name-addr version of the URI, which may be
         // different if it has a "transport" parameter.
         {
            char buffer[100];
            sprintf(buffer, "sip:111@%s:%d", hostIp.data(),
                    userAgentp->getTcpPort());

            // Specify TCP transport, because that's all the UA listens to.
            // Using an address with a transport parameter also exercises
            // SipDialog to ensure it handles contact addresses with parameters.
            strcat(buffer, ";transport=tcp");
            subscriber_addr_spec = buffer;

            Url subscriber_uri(buffer, TRUE);
            // Don't set display-name, because it will be lost when converted
            // to addr-spec, making some of the tests harder to do.
            subscriber_uri.toString(subscriber_name_addr);
         }

         // Set up the subscribe client
         clientDialogMgrp = new SipDialogMgr;
         refreshMgrp = new SipRefreshManager(*userAgentp, *clientDialogMgrp);
         refreshMgrp->start();
         subClientp = new SipSubscribeClient(*userAgentp, *clientDialogMgrp, *refreshMgrp);
         subClientp->start();

         // Set up the subscribe server
         subServerp = SipSubscribeServer::buildBasicServer(*userAgentp,
                                                           eventType);
         subMgrp = subServerp->getSubscriptionMgr(eventType);
         subMgrp->getDialogMgr();

         subServerp->start();
         // Enable the handler for the MWI server
         subServerp->enableEventType(eventType, userAgentp);
      }

   void tearDown()
      {
         subClientp->requestShutdown();
         refreshMgrp->requestShutdown();
         subServerp->requestShutdown();
         subServerp->shutdown();
         userAgentp->shutdown(TRUE);

         OsTask::delay(1000);   // 1 second to let other threads clean up

         delete subMgrp;
         delete subServerp;
         delete subClientp;
         delete refreshMgrp;
         delete clientDialogMgrp;
         delete userAgentp;
      }

   // Test to check that subscribe events and notify events return the same
   // dialog handle.
   void subscribeHandleTest()
      {
         // Create a subscription.
         UtlString earlyDialogHandle;
         CPPUNIT_ASSERT(subClientp->addSubscription(notifier_addr_spec,
                                                    eventType,
                                                    NULL,
                                                    subscriber_name_addr,
                                                    notifier_name_addr,
                                                    subscriber_name_addr,
                                                    60, // seconds expiration
                                                    this,
                                                    subStateCallback,
                                                    notifyCallback,
                                                    earlyDialogHandle));

         // Wait 1 second for the callbacks to happen.
         OsTask::delay(1000);

         UtlString earlyDialogMsg("Early dialog handles are different");
         earlyDialogMsg.append(" \n  sub:    '");
         earlyDialogMsg.append(smClientSubEarlyDialog);
         earlyDialogMsg.append("'\n  notify: '");
         earlyDialogMsg.append(smClientNotifyEarlyDialog);
         earlyDialogMsg.append("'");
         KNOWN_BUG("INTERMITTENT failures", "XX-6383");
         CPPUNIT_ASSERT_MESSAGE(earlyDialogMsg.data(),
                                compareHandles(smClientSubEarlyDialog.data(),
                                               smClientNotifyEarlyDialog.data()));
         UtlString establishedDialogMsg("Established dialog handles are different");
         establishedDialogMsg.append(" \n  sub:    '");
         establishedDialogMsg.append(smClientSubEstablishedDialog);
         establishedDialogMsg.append("'\n  notify: '");
         establishedDialogMsg.append(smClientNotifyEstablishedDialog);
         establishedDialogMsg.append("'");
         CPPUNIT_ASSERT_MESSAGE("Established dialog handles are different.",
                                compareHandles(smClientSubEstablishedDialog.data(),
                                               smClientNotifyEstablishedDialog.data()));
      }

};

CPPUNIT_TEST_SUITE_REGISTRATION(SipSubscribeClientTest1);

#if 0 // Removed - see XECS-2465

class SipSubscribeClientTest2 : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SipSubscribeClientTest2);
   CPPUNIT_TEST(subscribeCountNotifyMwiClientTest);
   CPPUNIT_TEST_SUITE_END();

public:

   SipSubscribeClientTest2()
      : incomingClientMsgQueue("SipSubscribeClientTest2::incomingClientMsgQueue")
      {
      }

   // Variables used by setUp/tearDown and the tests.

   UtlString hostIp;
   UtlString eventType;
   UtlString eventTypeKey;
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
   SipDialogMgr* clientDialogMgrp;
   SipRefreshManager* refreshMgrp;
   SipSubscribeClient* subClientp;
   OsMsgQ incomingClientMsgQueue;

   void setUp()
      {
         hostIp = "127.0.0.1";
         eventTypeKey = "message-summary";
         eventType = eventTypeKey;

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

         // Set up the subscribe client
         clientDialogMgrp = new SipDialogMgr;
         refreshMgrp = new SipRefreshManager(*subscriberUserAgentp, *clientDialogMgrp);
         refreshMgrp->start();
         subClientp = new SipSubscribeClient(*subscriberUserAgentp, *clientDialogMgrp,
                                             *refreshMgrp);
         subClientp->start();

         // Set up the subscribe server
         subServerp = SipSubscribeServer::buildBasicServer(*notifierUserAgentp,
                                                           eventType);
         subMgrp = subServerp->getSubscriptionMgr(eventType);
         subMgrp->getDialogMgr();
         subServerp->getPublishMgr(eventType);

         subServerp->start();
         // Enable the handler for the MWI server
         subServerp->enableEventType(eventType, notifierUserAgentp);
      }

   void tearDown()
      {
         subClientp->requestShutdown();
         refreshMgrp->requestShutdown();
         subServerp->requestShutdown();
         subServerp->shutdown();

         OsTask::delay(1000);   // 1 second to let other threads clean up

         subscriberUserAgentp->shutdown(TRUE);
         notifierUserAgentp->shutdown(TRUE);

         delete subscriberUserAgentp;
         delete notifierUserAgentp;
         delete subServerp;
      }

   void subscribeCountNotifyMwiClientTest()
      {
         int notifiesRxed = smNumClientNotifiesReceived;
         int substateRxed = smNumClientSubResponsesReceived;

         // Tell the Subscribe Client to set up a subscription.
         UtlString earlyDialogHandle;
         CPPUNIT_ASSERT(subClientp->addSubscription(notifier_aor,
                                                    eventType,
                                                    NULL,
                                                    subscriber_name_addr.data(),
                                                    notifier_name_addr.data(),
                                                    subscriber_name_addr.data(),
                                                    300, // seconds expiration
                                                    this,
                                                    subStateCallback,
                                                    notifyCallback,
                                                    earlyDialogHandle));

         OsTask::delay(1000);   // 1 second to establish dialog and first NOTIFY to arrive

         // can depend on the ordering of the first NOTIFY and the 202 response to SUBSCRIBE
         CPPUNIT_ASSERT(   (smNumClientSubResponsesReceived == substateRxed + 1)
                        || (smNumClientSubResponsesReceived == substateRxed + 2)
                        );
         CPPUNIT_ASSERT_EQUAL(notifiesRxed + 1, smNumClientNotifiesReceived);
      }

};

CPPUNIT_TEST_SUITE_REGISTRATION(SipSubscribeClientTest2);

class SipSubscribeClientTest3 : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SipSubscribeClientTest3);
#ifdef EXECUTE_SLOW_TESTS
   CPPUNIT_TEST(subscribeMwiClientTest);
#endif // EXECUTE_SLOW_TESTS
   CPPUNIT_TEST(contactTest);
   CPPUNIT_TEST_SUITE_END();

public:

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

   // Variables used by setUp/tearDown and the tests.

   UtlString hostIp;
   SipUserAgent* userAgentp;
   UtlString notifier_addr_spec;
   UtlString notifier_name_addr;
   UtlString notifier_contact_name_addr;
   UtlString resource_id;
   UtlString subscriber_addr_spec;
   UtlString subscriber_name_addr;
   UtlString subscriber_contact_name_addr;
   UtlString eventTypeKey;
   UtlString eventType;
   SipDialogMgr* clientDialogMgrp;
   SipRefreshManager* refreshMgrp;
   SipSubscribeClient* subClientp;
   SipSubscribeServer* subServerp;
   SipSubscriptionMgr* subMgrp;
   OsMsgQ subIncomingServerMsgQueue;
   OsMsgQ subIncomingClientMsgQueue;
   OsMsgQ notIncomingServerMsgQueue;
   OsMsgQ notIncomingClientMsgQueue;

   void setUp()
      {
         hostIp = "127.0.0.1";
         eventTypeKey = SIP_EVENT_MESSAGE_SUMMARY;
         eventType = eventTypeKey;

         // defaultSipAddress is set to force the SipUserAgent to open
         // ports on that interface.
         // publicAddress is set to force the SipUserAgent to report
         // that as its address.
         userAgentp = new SipUserAgent(PORT_NONE, PORT_DEFAULT, PORT_NONE,
                                       hostIp, NULL, hostIp);
         userAgentp->start();

         // Construct the URI of the notifier.
         // Also construct the name-addr version of the URI, which may be
         // different if it has a "transport" parameter.
         // And the resource-id to use, which is the AOR with any
         // parameters stripped off.
         {
            char buffer[100];
            sprintf(buffer, "sip:222@%s:%d", hostIp.data(),
                    userAgentp->getUdpPort());
            resource_id = buffer;

            // Specify UDP transport, because that's all the UA listens to.
            // Using an address with a transport parameter also exercises
            // SipDialog to ensure it handles contact addresses with parameters.
            strcat(buffer, ";transport=udp");
            notifier_addr_spec = buffer;

            Url notifier_uri(buffer, TRUE);
            notifier_uri.toString(notifier_name_addr);
            notifier_uri.setUserId(NULL);
            notifier_uri.toString(notifier_contact_name_addr);
         }

         // Construct the URI of the subscriber.
         // Also construct the name-addr version of the URI, which may be
         // different if it has a "transport" parameter.
         {
            char buffer[100];
            sprintf(buffer, "sip:111@%s:%d", hostIp.data(),
                    userAgentp->getUdpPort());

            // Specify UDP transport, because that's all the UA listens to.
            // Using an address with a transport parameter also exercises
            // SipDialog to ensure it handles contact addresses with parameters.
            strcat(buffer, ";transport=udp");
            subscriber_addr_spec = buffer;

            Url subscriber_uri(buffer, TRUE);
            subscriber_uri.toString(subscriber_name_addr);
            subscriber_uri.setUserId(NULL);
            subscriber_uri.toString(subscriber_contact_name_addr);
         }

         // Set up the subscribe client
         clientDialogMgrp = new SipDialogMgr;
         refreshMgrp = new SipRefreshManager(*userAgentp, *clientDialogMgrp);
         refreshMgrp->start();
         subClientp = new SipSubscribeClient(*userAgentp, *clientDialogMgrp, *refreshMgrp);
         subClientp->start();

         // Set up the subscribe server
         subServerp = SipSubscribeServer::buildBasicServer(*userAgentp,
                                                           eventType);
         subMgrp = subServerp->getSubscriptionMgr(eventType);
         subMgrp->getDialogMgr();
         subServerp->getPublishMgr(eventType);

         subServerp->start();
         // Enable the handler for the MWI server
         subServerp->enableEventType(eventType, userAgentp);

         // Create a SUBSCRIBE observer
         // Register an interest in SUBSCRIBE requests
         // for this event type
         userAgentp->addMessageObserver(subIncomingServerMsgQueue,
                                        SIP_SUBSCRIBE_METHOD,
                                        TRUE, // requests
                                        FALSE, // no reponses
                                        TRUE, // incoming
                                        FALSE, // no outgoing
                                        eventType,
                                        NULL,
                                        NULL);

         userAgentp->addMessageObserver(subIncomingClientMsgQueue,
                                        SIP_SUBSCRIBE_METHOD,
                                        FALSE, // no requests
                                        TRUE, // reponses
                                        TRUE, // incoming
                                        FALSE, // no outgoing
                                        eventType,
                                        NULL,
                                        NULL);

         // Create a NOTIFY observer
         // Register an interest in NOTIFY requests
         // for this event type
         userAgentp->addMessageObserver(notIncomingClientMsgQueue,
                                        SIP_NOTIFY_METHOD,
                                        TRUE, // requests
                                        FALSE, // no reponses
                                        TRUE, // incoming
                                        FALSE, // no outgoing
                                        eventType,
                                        NULL,
                                        NULL);

         userAgentp->addMessageObserver(notIncomingServerMsgQueue,
                                        SIP_NOTIFY_METHOD,
                                        FALSE, // no requests
                                        TRUE, // reponses
                                        TRUE, // incoming
                                        FALSE, // no outgoing
                                        eventType,
                                        NULL,
                                        NULL);
      }

   void tearDown()
      {
         // Unregister the queues so we stop receiving messages on them
         userAgentp->removeMessageObserver(subIncomingServerMsgQueue);
         userAgentp->removeMessageObserver(subIncomingClientMsgQueue);
         userAgentp->removeMessageObserver(notIncomingServerMsgQueue);
         userAgentp->removeMessageObserver(notIncomingClientMsgQueue);

         OsTask::delay(1000);   // 1 second to let other threads clean up

         subClientp->requestShutdown();
         refreshMgrp->requestShutdown();
         subServerp->requestShutdown();
         subServerp->shutdown();
         userAgentp->shutdown(TRUE);

         OsTask::delay(1000);   // 1 second to let other threads clean up

         delete subMgrp;
         delete subServerp;
         delete subClientp;
         delete refreshMgrp;
         delete clientDialogMgrp;
         delete userAgentp;
      }

   void subscribeMwiClientTest()
      {
         smClientExpiration = -1;
         smNumClientNotifiesReceived = 0;
         smLastClientNotifyReceived = NULL;
         smNumClientSubResponsesReceived = 0;
         smLastClientSubResponseReceived = NULL;

         // Get the Content Manager.
         SipPublishContentMgr* contentMgrp = subServerp->getPublishMgr(eventType);

         // Should not be any pre-existing content
         HttpBody* preexistingBodyPtr;
         UtlBoolean isDefaultContent;
         int version;
         CPPUNIT_ASSERT(!contentMgrp->getContent(resource_id,
                                                 eventTypeKey,
                                                 eventType,
                                                 NULL,
                                                 preexistingBodyPtr,
                                                 version,
                                                 isDefaultContent));
         int numDefaultContent = -1;
         int numDefaultConstructor = -1;
         int numResourceSpecificContent = -1;
         int numCallbacksRegistered = -1;
         contentMgrp->getStats(numDefaultContent,
                               numDefaultConstructor,
                               numResourceSpecificContent,
                               numCallbacksRegistered);
         CPPUNIT_ASSERT(numDefaultContent == 0);
         CPPUNIT_ASSERT(numDefaultConstructor == 0);
         CPPUNIT_ASSERT(numResourceSpecificContent == 0);
         CPPUNIT_ASSERT(numCallbacksRegistered == 1);

         // Set up a subscription.
         UtlString earlyDialogHandle;
         CPPUNIT_ASSERT(subClientp->addSubscription(notifier_addr_spec,
                                                    eventType,
                                                    NULL,
                                                    subscriber_name_addr.data(),
                                                    notifier_name_addr.data(),
                                                    NULL, // Let SipUserAgent choose Contact
                                                    60, // seconds expiration
                                                    this,
                                                    subStateCallback,
                                                    notifyCallback,
                                                    earlyDialogHandle));

         contentMgrp->getStats(numDefaultContent,
                               numDefaultConstructor,
                               numResourceSpecificContent,
                               numCallbacksRegistered);
         CPPUNIT_ASSERT(numDefaultContent == 0);
         CPPUNIT_ASSERT(numDefaultConstructor == 0);
         CPPUNIT_ASSERT(numResourceSpecificContent == 0);
         CPPUNIT_ASSERT(numCallbacksRegistered == 1);

         // See if a subscribe was sent and received
         const SipMessage* serverSideSubRequest = NULL;
         CPPUNIT_ASSERT(removeMessage(subIncomingServerMsgQueue,
                                      5000, // milliseconds
                                      serverSideSubRequest));
         CPPUNIT_ASSERT(serverSideSubRequest); // Sub request got to server

         // Check the Contact in the subscribe request.
         ASSERT_STR_EQUAL(subscriber_contact_name_addr,
                          serverSideSubRequest->
                              getHeaderValue(0, SIP_CONTACT_FIELD));

         const SipMessage* clientSideSubResponse = NULL;
         CPPUNIT_ASSERT(removeMessage(subIncomingClientMsgQueue,
                                      5000, // milliseconds
                                      clientSideSubResponse));
         CPPUNIT_ASSERT(clientSideSubResponse);

         // Check the Contact in the subscribe response.
         ASSERT_STR_EQUAL(notifier_name_addr,
                          clientSideSubResponse->
                              getHeaderValue(0, SIP_CONTACT_FIELD));

         int waitIterations = 0;
         while(smLastClientNotifyReceived == NULL ||
               smLastClientSubResponseReceived == NULL)
         {
            OsTask::delay(100);
            waitIterations++;
            if(waitIterations >= 100)
            {
               break;
            }
         }

         CPPUNIT_ASSERT(smLastClientSubResponseReceived);
         CPPUNIT_ASSERT(smLastClientNotifyReceived);
         SipMessage* firstSubResponse = smLastClientSubResponseReceived;
         smLastClientSubResponseReceived = NULL;
         int firstSubCseq;
         firstSubResponse->getCSeqField(&firstSubCseq, NULL);
         SipMessage* firstNotifyRequest = smLastClientNotifyReceived;
         smLastClientNotifyReceived = NULL;
         int firstNotifyCseq;
         firstNotifyRequest->getCSeqField(&firstNotifyCseq, NULL);
         CPPUNIT_ASSERT(firstSubCseq == 1);
         CPPUNIT_ASSERT(firstNotifyCseq == 1);

         // The refresh manager should re-SUBSCRIBE
         // Wait for the next notify request and subscribe response
         int secondMessageWait = 60;
         int resendTimeout = (int) (0.55 * secondMessageWait);
         if(resendTimeout < 40)
         {
            resendTimeout = 40;
         }
         for(int i = 0; i < secondMessageWait - 1; i++)
         {
            if(i == resendTimeout - 1)
            {
               printf("v");
            }
            else
            {
               printf("=");
            }
         }
         printf("v\n");
         SipMessage* secondSubResponse = NULL;
         SipMessage* secondNotifyRequest = NULL;

         while(secondNotifyRequest == NULL ||
               secondSubResponse == NULL)
         {
            OsTask::delay(1000);
            if(smLastClientSubResponseReceived)
            {
               secondSubResponse = smLastClientSubResponseReceived;
               smLastClientSubResponseReceived = NULL;
            }
            if(smLastClientNotifyReceived)
            {
               secondNotifyRequest = smLastClientNotifyReceived;
               smLastClientNotifyReceived = NULL;
            }
            printf(".");
            fflush(stdout);
            waitIterations++;
            if(waitIterations >= secondMessageWait)
            {
               printf("!");
               break;
            }
         }
         printf("\n");

         CPPUNIT_ASSERT(removeMessage(subIncomingServerMsgQueue,
                                      5000, // milliseconds
                                      serverSideSubRequest));
         CPPUNIT_ASSERT(serverSideSubRequest); // Sub request got to server

         CPPUNIT_ASSERT(removeMessage(subIncomingClientMsgQueue,
                                      5000, // milliseconds
                                      clientSideSubResponse));
         CPPUNIT_ASSERT(clientSideSubResponse); // Sub response got to client

         CPPUNIT_ASSERT(secondNotifyRequest);
         CPPUNIT_ASSERT(secondSubResponse);
         int secondSubCseq = -1;
         int secondNotifyCseq = -1;
         smLastClientSubResponseReceived = NULL;
         secondSubResponse->getCSeqField(&secondSubCseq, NULL);
         smLastClientNotifyReceived = NULL;
         secondNotifyRequest->getCSeqField(&secondNotifyCseq, NULL);
         CPPUNIT_ASSERT(firstSubCseq < secondSubCseq);
         CPPUNIT_ASSERT(firstNotifyCseq < secondNotifyCseq);
      }

   // Test that the Contact headers on SUBSCRIBE and NOTIFY requests
   // and responses are what is expected.
   // This tests XECS-297, XECS-298, and XECS-299.
   void contactTest()
      {
         // Set up a subscription.
         UtlString earlyDialogHandle;
         CPPUNIT_ASSERT(subClientp->addSubscription(notifier_addr_spec,
                                                    eventType,
                                                    NULL,
                                                    subscriber_name_addr.data(),
                                                    notifier_name_addr.data(),
                                                    NULL, // Let SipUserAgent choose Contact
                                                    60, // seconds expiration
                                                    this,
                                                    subStateCallback,
                                                    notifyCallback,
                                                    earlyDialogHandle));

         // See if a subscribe was sent and received
         const SipMessage* serverSideSubRequest = NULL;
         CPPUNIT_ASSERT(removeMessage(subIncomingServerMsgQueue,
                                      5000, // milliseconds
                                      serverSideSubRequest));
         CPPUNIT_ASSERT(serverSideSubRequest); // Sub request got to server

         // Check the Contact in the subscribe request.
         ASSERT_STR_EQUAL(subscriber_contact_name_addr,
                          serverSideSubRequest->
                              getHeaderValue(0, SIP_CONTACT_FIELD));

         const SipMessage* clientSideSubResponse = NULL;
         CPPUNIT_ASSERT(removeMessage(subIncomingClientMsgQueue,
                                      5000, // milliseconds
                                      clientSideSubResponse));
         CPPUNIT_ASSERT(clientSideSubResponse);
         CPPUNIT_ASSERT_EQUAL(SIP_ACCEPTED_CODE,
                              clientSideSubResponse->getResponseStatusCode());

         // Check the Contact in the subscribe response.
         ASSERT_STR_EQUAL(notifier_name_addr,
                          clientSideSubResponse->
                              getHeaderValue(0, SIP_CONTACT_FIELD));

         // See if a notify was sent and received
         const SipMessage* clientSideNotRequest = NULL;
         CPPUNIT_ASSERT(removeMessage(notIncomingClientMsgQueue,
                                      5000, // milliseconds
                                      clientSideNotRequest));
         CPPUNIT_ASSERT(clientSideNotRequest); // NOTIFY request got to client

         // Check the Contact in the NOTIFY request.
         ASSERT_STR_EQUAL(notifier_name_addr,
                          clientSideNotRequest->
                              getHeaderValue(0, SIP_CONTACT_FIELD));

         const SipMessage* serverSideNotResponse = NULL;
         CPPUNIT_ASSERT(removeMessage(notIncomingServerMsgQueue,
                                      5000, // milliseconds
                                      serverSideNotResponse));
         CPPUNIT_ASSERT(serverSideNotResponse);
         CPPUNIT_ASSERT_EQUAL(SIP_OK_CODE,
                              serverSideNotResponse->getResponseStatusCode());

         // Check the Contact in the NOTIFY response.
         ASSERT_STR_EQUAL(notifier_contact_name_addr,
                          serverSideNotResponse->
                              getHeaderValue(0, SIP_CONTACT_FIELD));
      }

};

CPPUNIT_TEST_SUITE_REGISTRATION(SipSubscribeClientTest3);

#endif // Removed - see XECS-2465

class SipSubscribeClientTest4 : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SipSubscribeClientTest4);
   CPPUNIT_TEST(outOfOrder);
   CPPUNIT_TEST(duplicateNotify);
#ifdef EXECUTE_SLOW_TESTS
   CPPUNIT_TEST(terminate481);
   CPPUNIT_TEST(reest2);
   CPPUNIT_TEST(forkedSubscribe);
#endif // EXECUTE_SLOW_TESTS
   CPPUNIT_TEST_SUITE_END();

public:

   SipSubscribeClientTest4()
      : incomingServerMsgQueue("SipSubscribeClientTest4::incomingServerMsgQueue")
      {
      }

   // Variables used by setUp/tearDown and the tests.

   UtlString hostIp;
   SipUserAgent* userAgentp;
   UtlString notifier_addr_spec;
   UtlString notifier_name_addr;
   UtlString resource_id;
   UtlString subscriber_addr_spec;
   UtlString subscriber_name_addr;
   UtlString eventTypeKey;
   UtlString eventType;
   SipDialogMgr* clientDialogMgrp;
   SipRefreshManager* refreshMgrp;
   SipSubscribeClient* subClientp;
   OsMsgQ incomingServerMsgQueue;

   void setUp()
      {
         hostIp = "127.0.0.1";
         eventTypeKey = SIP_EVENT_MESSAGE_SUMMARY;
         eventType = eventTypeKey;

         // defaultSipAddress is set to force the SipUserAgent to open
         // ports on that interface.
         // publicAddress is set to force the SipUserAgent to report
         // that as its address.
         userAgentp = new SipUserAgent(PORT_NONE, PORT_DEFAULT, PORT_NONE,
                                       hostIp, NULL, hostIp);
         userAgentp->start();

         // Construct the URI of the notifier.
         // Also construct the name-addr version of the URI, which may be
         // different if it has a "transport" parameter.
         // And the resource-id to use, which is the AOR with any
         // parameters stripped off.
         {
            char buffer[100];
            sprintf(buffer, "sip:222@%s:%d", hostIp.data(),
                    userAgentp->getUdpPort());
            resource_id = buffer;

            // Specify UDP transport, because that's all the UA listens to.
            // Using an address with a transport parameter also exercises
            // SipDialog to ensure it handles contact addresses with parameters.
            strcat(buffer, ";transport=udp");
            notifier_addr_spec = buffer;

            Url notifier_uri(buffer, TRUE);
            notifier_uri.toString(notifier_name_addr);
         }

         // Construct the URI of the subscriber.
         // Also construct the name-addr version of the URI, which may be
         // different if it has a "transport" parameter.
         {
            char buffer[100];
            sprintf(buffer, "sip:111@%s:%d", hostIp.data(),
                    userAgentp->getUdpPort());

            // Specify UDP transport, because that's all the UA listens to.
            // Using an address with a transport parameter also exercises
            // SipDialog to ensure it handles contact addresses with parameters.
            strcat(buffer, ";transport=udp");
            subscriber_addr_spec = buffer;

            Url subscriber_uri(buffer, TRUE);
            subscriber_uri.toString(subscriber_name_addr);
         }

         // Set up the subscribe client
         clientDialogMgrp = new SipDialogMgr;
         refreshMgrp = new SipRefreshManager(*userAgentp, *clientDialogMgrp);
         refreshMgrp->start();
         subClientp = new SipSubscribeClient(*userAgentp, *clientDialogMgrp, *refreshMgrp);
         subClientp->start();

         // Create a crude Subscription server/observer
         // Register an interest in SUBSCRIBE requests and NOTIFY responses
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

         userAgentp->addMessageObserver(incomingServerMsgQueue,
                                        SIP_NOTIFY_METHOD,
                                        FALSE, // no requests
                                        TRUE, // reponses
                                        TRUE, // incoming
                                        FALSE, // no outgoing
                                        eventType,
                                        NULL,
                                        NULL);
      }

   void tearDown()
      {
         // Unregister the queues so we stop receiving messages on them
         userAgentp->removeMessageObserver(incomingServerMsgQueue);

         OsTask::delay(1000);   // 1 second to let other threads clean up

         subClientp->requestShutdown();
         refreshMgrp->requestShutdown();
         userAgentp->shutdown(TRUE);

         OsTask::delay(1000);   // 1 second to let other threads clean up

         delete subClientp;
         delete refreshMgrp;
         delete clientDialogMgrp;
         delete userAgentp;
      }

   // XECS-244: When a re-SUBSCRIBE receives a 481 response, it terminates the
   // subscription.
   // Also, verify that subscription reestablishment works.
   void terminate481()
      {
         UtlString mwiMimeType(CONTENT_TYPE_SIMPLE_MESSAGE_SUMMARY);

         // Tell the Subscribe Client to establish a subscription.
         const int refreshTime = 40;
         UtlString earlyDialogHandle;
         CPPUNIT_ASSERT(subClientp->addSubscription(notifier_addr_spec,
                                                    eventType,
                                                    NULL,
                                                    subscriber_name_addr,
                                                    notifier_name_addr,
                                                    subscriber_name_addr,
                                                    refreshTime, // seconds expiration
                                                    this,
                                                    subStateCallback,
                                                    notifyCallback,
                                                    earlyDialogHandle));

         // The Subscribe Client will now send a SUBSCRIBE.
         // Receive it and send a 202 response.

         OsTime timeout1sec(1, 0);  // 1 second
         const SipMessage* subscribeRequest;
         const SipMessage* notifyResponse;
         UtlString subscribeToTag;
         CPPUNIT_ASSERT(runListener(incomingServerMsgQueue,
                                    *userAgentp,
                                    timeout1sec,
                                    timeout1sec,
                                    subscribeRequest,
                                    notifyResponse,
                                    SIP_ACCEPTED_CODE,
                                    FALSE,
                                    refreshTime,
                                    &subscribeToTag));
         CPPUNIT_ASSERT(subscribeRequest);
         // Save the Call-Id for later test.
         UtlString firstCallId;
         subscribeRequest->getCallIdField(&firstCallId);
         CPPUNIT_ASSERT(!notifyResponse);

         // Send the NOTIFY in response to the SUBSCRIBE.

         SipMessage notifyMessage;
         char subscriptionState[20];
         sprintf(subscriptionState, "active;expires=%d", refreshTime);
         notifyMessage.setNotifyData(subscribeRequest,
                                     1, // CSeq
                                     NULL, // Route
                                     subscriptionState, // Subscription-State
                                     eventType, // Event
                                     NULL // Event id
            );
         notifyMessage.setFromFieldTag(subscribeToTag);
         CPPUNIT_ASSERT(userAgentp->send(notifyMessage));

         // The Subscribe Client will now send a 200 response to the NOTIFY.

         CPPUNIT_ASSERT(runListener(incomingServerMsgQueue,
                                    *userAgentp,
                                    timeout1sec,
                                    timeout1sec,
                                    subscribeRequest,
                                    notifyResponse,
                                    SIP_ACCEPTED_CODE,
                                    FALSE,
                                    0,
                                    NULL));
         CPPUNIT_ASSERT(!subscribeRequest);
         CPPUNIT_ASSERT(notifyResponse);
         CPPUNIT_ASSERT_EQUAL(SIP_OK_CODE,
                              notifyResponse->getResponseStatusCode());

         // Wait for the re-SUBSCRIBE, and respond with 481.

         fprintf(stderr, "Waiting %d seconds...\n", refreshTime);
         OsTime timeoutRefreshTime(refreshTime, 0);
         OsTime timeoutZero(0, 0);
         CPPUNIT_ASSERT(runListener(incomingServerMsgQueue,
                                    *userAgentp,
                                    timeoutRefreshTime,
                                    timeoutZero,
                                    subscribeRequest,
                                    notifyResponse,
                                    SIP_BAD_TRANSACTION_CODE,
                                    FALSE,
                                    0,
                                    NULL));
         CPPUNIT_ASSERT(subscribeRequest);
         CPPUNIT_ASSERT(!notifyResponse);

         // Wait 100 msec, then send a NOTIFY and verify that it gets a 481
         // response (because Subscribe Client has terminated the subscription).

         OsTask::delay(100);      // 100 msec
         notifyMessage.incrementCSeqNumber();
         CPPUNIT_ASSERT(userAgentp->send(notifyMessage));
         CPPUNIT_ASSERT(runListener(incomingServerMsgQueue,
                                    *userAgentp,
                                    timeout1sec,
                                    timeout1sec,
                                    subscribeRequest,
                                    notifyResponse,
                                    SIP_ACCEPTED_CODE,
                                    FALSE,
                                    refreshTime,
                                    &subscribeToTag));
         CPPUNIT_ASSERT(notifyResponse);
         CPPUNIT_ASSERT_EQUAL(SIP_BAD_TRANSACTION_CODE,
                              notifyResponse->getResponseStatusCode());

         // Now check reestablishment of subscriptions.

         // Verify that Subscribe Client has sent a new SUBSCRIBE.
         CPPUNIT_ASSERT(subscribeRequest);
         // Check that it has a different Call-Id than the first SUBSCRIBE, and no to-tag.
         UtlString newCallId;
         subscribeRequest->getCallIdField(&newCallId);
         CPPUNIT_ASSERT(firstCallId != newCallId);
         Url toUrl;
         subscribeRequest->getToUrl(toUrl);
         UtlString toTag;
         toUrl.getFieldParameter("tag", toTag);
         CPPUNIT_ASSERT(toTag.isNull());

         // Send the NOTIFY in response to the SUBSCRIBE.

         sprintf(subscriptionState, "active;expires=%d", refreshTime);
         SipMessage newNotifyMessage;
         newNotifyMessage.setNotifyData(subscribeRequest,
                                        1, // CSeq
                                        NULL, // Route
                                        subscriptionState, // Subscription-State
                                        eventType, // Event
                                        NULL // Event id
            );
         newNotifyMessage.setFromFieldTag(subscribeToTag);
         CPPUNIT_ASSERT(userAgentp->send(newNotifyMessage));

         // The Subscribe Client will now send a 200 response to the NOTIFY.

         CPPUNIT_ASSERT(runListener(incomingServerMsgQueue,
                                    *userAgentp,
                                    timeout1sec,
                                    timeout1sec,
                                    subscribeRequest,
                                    notifyResponse,
                                    SIP_ACCEPTED_CODE,
                                    FALSE,
                                    0,
                                    NULL));
         CPPUNIT_ASSERT(!subscribeRequest);
         CPPUNIT_ASSERT(notifyResponse);
         CPPUNIT_ASSERT_EQUAL(SIP_OK_CODE,
                              notifyResponse->getResponseStatusCode());

         // Wait for the re-SUBSCRIBE, and respond with 200.

         fprintf(stderr, "Waiting %d seconds...\n", refreshTime);
         CPPUNIT_ASSERT(runListener(incomingServerMsgQueue,
                                    *userAgentp,
                                    timeoutRefreshTime,
                                    timeoutZero,
                                    subscribeRequest,
                                    notifyResponse,
                                    SIP_OK_CODE,
                                    FALSE,
                                    0,
                                    NULL));
         CPPUNIT_ASSERT(subscribeRequest);
         CPPUNIT_ASSERT(!notifyResponse);
         // Verify this is a re-SUBSCRIBE for the second subscription.
         UtlString resubCallId;
         subscribeRequest->getCallIdField(&resubCallId);
         CPPUNIT_ASSERT(resubCallId == newCallId);
      }

   // Check that subscription reestablishment will try a second time.
   // Test that NOTIFY with "Subscription-State: terminated" will
   // force reestablishment of a subscription.
   void reest2()
      {
         UtlString mwiMimeType(CONTENT_TYPE_SIMPLE_MESSAGE_SUMMARY);

         // Tell the Subscribe Client to establish a subscription.

         const int refreshTime = 40;
         UtlString earlyDialogHandle;
         CPPUNIT_ASSERT(subClientp->addSubscription(notifier_addr_spec,
                                                    eventType,
                                                    NULL,
                                                    subscriber_name_addr,
                                                    notifier_name_addr,
                                                    subscriber_name_addr,
                                                    refreshTime, // seconds expiration
                                                    this,
                                                    subStateCallback,
                                                    notifyCallback,
                                                    earlyDialogHandle));

         // The Subscribe Client will now send a SUBSCRIBE.
         // Receive it and send a 202 response.

         OsTime timeout1sec(1, 0);  // 1 second
         const SipMessage* subscribeRequest;
         const SipMessage* notifyResponse;
         UtlString subscribeToTag;
         CPPUNIT_ASSERT(runListener(incomingServerMsgQueue,
                                    *userAgentp,
                                    timeout1sec,
                                    timeout1sec,
                                    subscribeRequest,
                                    notifyResponse,
                                    SIP_ACCEPTED_CODE,
                                    FALSE,
                                    refreshTime,
                                    &subscribeToTag));
         CPPUNIT_ASSERT(subscribeRequest);
         // Save the SUBSCRIBE request.
         const SipMessage* firstSubscribeRequest = subscribeRequest;
         // Save the Call-Id for later test.
         UtlString firstCallId;
         subscribeRequest->getCallIdField(&firstCallId);
         CPPUNIT_ASSERT(!notifyResponse);

         // Send the NOTIFY in response to the SUBSCRIBE.

         SipMessage notifyMessage;
         char subscriptionState[20];
         sprintf(subscriptionState, "active;expires=%d", refreshTime);
         notifyMessage.setNotifyData(firstSubscribeRequest,
                                     1, // CSeq
                                     NULL, // Route
                                     subscriptionState, // Subscription-State
                                     eventType, // Event
                                     NULL // Event id
            );
         notifyMessage.setFromFieldTag(subscribeToTag);
         CPPUNIT_ASSERT(userAgentp->send(notifyMessage));

         // The Subscribe Client will now send a 200 response to the NOTIFY.

         CPPUNIT_ASSERT(runListener(incomingServerMsgQueue,
                                    *userAgentp,
                                    timeout1sec,
                                    timeout1sec,
                                    subscribeRequest,
                                    notifyResponse,
                                    SIP_ACCEPTED_CODE,
                                    FALSE,
                                    0,
                                    NULL));
         CPPUNIT_ASSERT(!subscribeRequest);
         CPPUNIT_ASSERT(notifyResponse);
         CPPUNIT_ASSERT_EQUAL(SIP_OK_CODE,
                              notifyResponse->getResponseStatusCode());

         // Delay more than 15 seconds so that the Subscribe Client recognizes
         // the subscription as having been successfully established.
         fprintf(stderr, "Waiting %d seconds...\n", 15+1);
         OsTask::delay((15+1) * 1000);

         // Send a NOTIFY with state "terminated" to force reestablishment
         // of the subscription.

         notifyMessage.setNotifyData(firstSubscribeRequest,
                                     2, // CSeq
                                     NULL, // Route
                                     "terminated", // Subscription-State
                                     eventType, // Event
                                     NULL // Event id
            );
         notifyMessage.setFromFieldTag(subscribeToTag);
         CPPUNIT_ASSERT(userAgentp->send(notifyMessage));

         // The Subscribe Client will now send a 200 response to the NOTIFY,
         // and then the first reestablishing SUBSCRIBE.
         // Make the SUBSCRIBE fail by responding 404.

         CPPUNIT_ASSERT(runListener(incomingServerMsgQueue,
                                    *userAgentp,
                                    timeout1sec,
                                    timeout1sec,
                                    subscribeRequest,
                                    notifyResponse,
                                    SIP_NOT_FOUND_CODE,
                                    FALSE,
                                    0,
                                    NULL));
         CPPUNIT_ASSERT(notifyResponse);
         CPPUNIT_ASSERT_EQUAL(SIP_OK_CODE,
                              notifyResponse->getResponseStatusCode());
         CPPUNIT_ASSERT(subscribeRequest);

         // Wait for the next reestablishing SUBSCRIBE, which should happen
         // within 15 seconds, and respond with 200.

         const int delay = 15 + 1;
         OsTime timeoutSecondReest(delay, 0);
         OsTime timeoutZero(0, 0);
         fprintf(stderr, "Waiting %d seconds...\n", delay);
         CPPUNIT_ASSERT(runListener(incomingServerMsgQueue,
                                    *userAgentp,
                                    timeoutSecondReest,
                                    timeoutZero,
                                    subscribeRequest,
                                    notifyResponse,
                                    SIP_OK_CODE,
                                    FALSE,
                                    0,
                                    NULL));
         CPPUNIT_ASSERT(subscribeRequest);
      }

   // XECS-283: When a NOTIFY is out-of-order, add "Retry-After: 0" to the 500
   // response so that the response does not terminate the subscription.
   // XECS-245: An out-of-order NOTIFY receives a 481 response rather than 500.
   void outOfOrder()
      {
         UtlString mwiMimeType(CONTENT_TYPE_SIMPLE_MESSAGE_SUMMARY);

         // Tell the Subscribe Client to establish a subscription.
         UtlString earlyDialogHandle;
         CPPUNIT_ASSERT(subClientp->addSubscription(notifier_addr_spec,
                                                    eventType,
                                                    NULL,
                                                    subscriber_name_addr,
                                                    notifier_name_addr,
                                                    subscriber_name_addr,
                                                    300, // seconds expiration
                                                    this,
                                                    subStateCallback,
                                                    notifyCallback,
                                                    earlyDialogHandle));

         // The Subscribe Client will now send a SUBSCRIBE.
         // Receive it and send a 202 response.

         OsTime timeout1sec(1, 0);  // 1 second
         const SipMessage* subscribeRequest;
         const SipMessage* notifyResponse;
         UtlString subscribeToTag;
         CPPUNIT_ASSERT(runListener(incomingServerMsgQueue,
                                    *userAgentp,
                                    timeout1sec,
                                    timeout1sec,
                                    subscribeRequest,
                                    notifyResponse,
                                    SIP_ACCEPTED_CODE,
                                    FALSE,
                                    5,
                                    &subscribeToTag));
         CPPUNIT_ASSERT(subscribeRequest);
         CPPUNIT_ASSERT(!notifyResponse);

         // Send a NOTIFY in response to the SUBSCRIBE.

         SipMessage notifyMessage;
         notifyMessage.setNotifyData(subscribeRequest,
                                     10, // CSeq
                                     NULL, // Route
                                     "active;expires=300", // Subscription-State
                                     eventType, // Event
                                     NULL // Event id
            );
         notifyMessage.setFromFieldTag(subscribeToTag);
         CPPUNIT_ASSERT(userAgentp->send(notifyMessage));

         // The Subscribe Client will now send a 200 response to the NOTIFY.

         const SipMessage* noSubscribeRequest;
         CPPUNIT_ASSERT(runListener(incomingServerMsgQueue,
                                    *userAgentp,
                                    timeout1sec,
                                    timeout1sec,
                                    noSubscribeRequest,
                                    notifyResponse,
                                    SIP_ACCEPTED_CODE,
                                    FALSE,
                                    0,
                                    NULL));
         CPPUNIT_ASSERT(!noSubscribeRequest);
         CPPUNIT_ASSERT(notifyResponse);
         CPPUNIT_ASSERT_EQUAL(SIP_OK_CODE,
                              notifyResponse->getResponseStatusCode());

         // Send another NOTIFY, which is out-of-order.

         notifyMessage.setNotifyData(subscribeRequest,
                                     9, // CSeq
                                     NULL, // Route
                                     "active;expires=300", // Subscription-State
                                     eventType, // Event
                                     NULL // Event id
            );
         notifyMessage.setFromFieldTag(subscribeToTag);
         CPPUNIT_ASSERT(userAgentp->send(notifyMessage));

         // The Subscribe Client will now send a 500 response to the NOTIFY.

         CPPUNIT_ASSERT(runListener(incomingServerMsgQueue,
                                    *userAgentp,
                                    timeout1sec,
                                    timeout1sec,
                                    subscribeRequest,
                                    notifyResponse,
                                    SIP_ACCEPTED_CODE,
                                    FALSE,
                                    0,
                                    NULL));
         CPPUNIT_ASSERT(!subscribeRequest);
         CPPUNIT_ASSERT(notifyResponse);
         CPPUNIT_ASSERT_EQUAL(SIP_SERVER_INTERNAL_ERROR_CODE,
                              notifyResponse->getResponseStatusCode());

         // Verify that there is a "Retry-After: 0" header.
         const char* v =
            notifyResponse->getHeaderValue(0, SIP_RETRY_AFTER_FIELD);
         CPPUNIT_ASSERT(v);
         ASSERT_STR_EQUAL("0", v);
      }

   // XECS-246: A duplicated NOTIFY receives a 481 response rather
   // than a 482 Loop Detected response.
   void duplicateNotify()
      {
         UtlString mwiMimeType(CONTENT_TYPE_SIMPLE_MESSAGE_SUMMARY);

         // Tell the Subscribe Client to establish a subscription.
         UtlString earlyDialogHandle;
         CPPUNIT_ASSERT(subClientp->addSubscription(notifier_addr_spec,
                                                    eventType,
                                                    NULL,
                                                    subscriber_name_addr,
                                                    notifier_name_addr,
                                                    subscriber_name_addr,
                                                    300, // seconds expiration
                                                    this,
                                                    subStateCallback,
                                                    notifyCallback,
                                                    earlyDialogHandle));

         // The Subscribe Client will now send a SUBSCRIBE.
         // Receive it and send a 202 response.

         OsTime timeout1sec(1, 0);  // 1 second
         const SipMessage* subscribeRequest;
         const SipMessage* notifyResponse;
         UtlString subscribeToTag;
         CPPUNIT_ASSERT(runListener(incomingServerMsgQueue,
                                    *userAgentp,
                                    timeout1sec,
                                    timeout1sec,
                                    subscribeRequest,
                                    notifyResponse,
                                    SIP_ACCEPTED_CODE,
                                    FALSE,
                                    5,
                                    &subscribeToTag));
         CPPUNIT_ASSERT(subscribeRequest);
         CPPUNIT_ASSERT(!notifyResponse);

         // Send a NOTIFY in response to the SUBSCRIBE.

         SipMessage notifyMessage;
         notifyMessage.setNotifyData(subscribeRequest,
                                     10, // CSeq
                                     NULL, // Route
                                     "active;expires=300", // Subscription-State
                                     eventType, // Event
                                     NULL // Event id
            );
         notifyMessage.setFromFieldTag(subscribeToTag);
         CPPUNIT_ASSERT(userAgentp->send(notifyMessage));

         // The Subscribe Client will now send a 200 response to the NOTIFY.

         const SipMessage* noSubscribeRequest;
         CPPUNIT_ASSERT(runListener(incomingServerMsgQueue,
                                    *userAgentp,
                                    timeout1sec,
                                    timeout1sec,
                                    noSubscribeRequest,
                                    notifyResponse,
                                    SIP_ACCEPTED_CODE,
                                    FALSE,
                                    0,
                                    NULL));
         CPPUNIT_ASSERT(!noSubscribeRequest);
         CPPUNIT_ASSERT(notifyResponse);
         CPPUNIT_ASSERT_EQUAL(SIP_OK_CODE,
                              notifyResponse->getResponseStatusCode());

         // Send a duplicate NOTIFY.

         // Note that this re-send will get a new Via branch parameter.
         // The processing of this case is complex -- RFC 3261 requires
         // it to be considered a separate transaction (which is out-of-order),
         // but it probably would be better to treat it as a re-send.
         // This will probably require fixing a bug in RFC 3261 to work correctly.

         CPPUNIT_ASSERT(userAgentp->send(notifyMessage));

         // The Subscribe Client will now send a 200 response to the NOTIFY.

         CPPUNIT_ASSERT(runListener(incomingServerMsgQueue,
                                    *userAgentp,
                                    timeout1sec,
                                    timeout1sec,
                                    subscribeRequest,
                                    notifyResponse,
                                    SIP_ACCEPTED_CODE,
                                    FALSE,
                                    0,
                                    NULL));
         CPPUNIT_ASSERT(!subscribeRequest);
         CPPUNIT_ASSERT(notifyResponse);
         /** Verify that response code is 482, as required by RFC 3261,
          *  section 16.3, item 4 -- See XECS-246 for discussion. */
         CPPUNIT_ASSERT_EQUAL(SIP_LOOP_DETECTED_CODE,
                              notifyResponse->getResponseStatusCode());
         /** Verify that there is a "Retry-After: 0" header to prevent
          *  terminating subscriptions. */
         const char* v =
            notifyResponse->getHeaderValue(0, SIP_RETRY_AFTER_FIELD);
         CPPUNIT_ASSERT(v);
         ASSERT_STR_EQUAL("0", v);
      }

   // XECS-255: Forked SUBSCRIBE.
   void forkedSubscribe()
      {
         UtlString mwiMimeType(CONTENT_TYPE_SIMPLE_MESSAGE_SUMMARY);
         const int refreshTime = 40;
         char subscriptionState[20];
         sprintf(subscriptionState, "active;expires=%d", refreshTime);

         // Tell the Subscribe Client to establish a subscription.
         UtlString earlyDialogHandle;
         CPPUNIT_ASSERT(subClientp->addSubscription(notifier_addr_spec,
                                                    eventType,
                                                    NULL,
                                                    subscriber_name_addr,
                                                    notifier_name_addr,
                                                    subscriber_name_addr,
                                                    refreshTime, // seconds expiration
                                                    this,
                                                    subStateCallback,
                                                    notifyCallback,
                                                    earlyDialogHandle));

         // The Subscribe Client will now send a SUBSCRIBE.
         // Receive it and send a 202 response.

         OsTime timeout1sec(1, 0);  // 1 second
         const SipMessage* subscribeRequest;
         const SipMessage* notifyResponse;
         UtlString subscribeToTag;
         CPPUNIT_ASSERT(runListener(incomingServerMsgQueue,
                                    *userAgentp,
                                    timeout1sec,
                                    timeout1sec,
                                    subscribeRequest,
                                    notifyResponse,
                                    SIP_ACCEPTED_CODE,
                                    FALSE,
                                    refreshTime,
                                    &subscribeToTag));
         CPPUNIT_ASSERT(subscribeRequest);
         CPPUNIT_ASSERT(!notifyResponse);

         // Send a NOTIFY in response to the SUBSCRIBE.

         SipMessage notifyMessage;
         notifyMessage.setNotifyData(subscribeRequest,
                                     10, // CSeq
                                     NULL, // Route
                                     subscriptionState, // Subscription-State
                                     eventType, // Event
                                     NULL // Event id
            );
         notifyMessage.setFromFieldTag(subscribeToTag);
         CPPUNIT_ASSERT(userAgentp->send(notifyMessage));

         // The Subscribe Client will now send a 200 response to the NOTIFY.

         const SipMessage* noSubscribeRequest;
         CPPUNIT_ASSERT(runListener(incomingServerMsgQueue,
                                    *userAgentp,
                                    timeout1sec,
                                    timeout1sec,
                                    noSubscribeRequest,
                                    notifyResponse,
                                    SIP_ACCEPTED_CODE,
                                    FALSE,
                                    0,
                                    NULL));
         CPPUNIT_ASSERT(!noSubscribeRequest);
         CPPUNIT_ASSERT(notifyResponse);
         CPPUNIT_ASSERT_EQUAL(SIP_OK_CODE,
                              notifyResponse->getResponseStatusCode());

         // Send a second NOTIFY, on another fork.

         SipMessage secondNotifyMessage;
         secondNotifyMessage.setNotifyData(subscribeRequest,
                                           10, // CSeq
                                           NULL, // Route
                                           subscriptionState, // Subscription-State
                                           eventType, // Event
                                           NULL // Event id
            );
         // Set a different to-tag.
         UtlString secondToTag;
         CallId::getNewTag(secondToTag);
         secondNotifyMessage.setFromFieldTag(secondToTag);
         CPPUNIT_ASSERT(userAgentp->send(secondNotifyMessage));

         // The Subscribe Client will now send a 200 response to the NOTIFY.

         CPPUNIT_ASSERT(runListener(incomingServerMsgQueue,
                                    *userAgentp,
                                    timeout1sec,
                                    timeout1sec,
                                    subscribeRequest,
                                    notifyResponse,
                                    SIP_ACCEPTED_CODE,
                                    FALSE,
                                    0,
                                    NULL));
         CPPUNIT_ASSERT(!subscribeRequest);
         CPPUNIT_ASSERT(notifyResponse);
         CPPUNIT_ASSERT_EQUAL(SIP_OK_CODE,
                              notifyResponse->getResponseStatusCode());

         // Wait a second.

         OsTask::delay(1000);

         // Send a second set of NOTIFYs and check that they are accepted.
         // This verifies that both subscriptions are established.

         notifyMessage.incrementCSeqNumber();
         CPPUNIT_ASSERT(userAgentp->send(notifyMessage));

         CPPUNIT_ASSERT(runListener(incomingServerMsgQueue,
                                    *userAgentp,
                                    timeout1sec,
                                    timeout1sec,
                                    noSubscribeRequest,
                                    notifyResponse,
                                    SIP_ACCEPTED_CODE,
                                    FALSE,
                                    0,
                                    NULL));
         CPPUNIT_ASSERT(!noSubscribeRequest);
         CPPUNIT_ASSERT(notifyResponse);
         CPPUNIT_ASSERT_EQUAL(SIP_OK_CODE,
                              notifyResponse->getResponseStatusCode());

         secondNotifyMessage.incrementCSeqNumber();
         CPPUNIT_ASSERT(userAgentp->send(secondNotifyMessage));

         CPPUNIT_ASSERT(runListener(incomingServerMsgQueue,
                                    *userAgentp,
                                    timeout1sec,
                                    timeout1sec,
                                    noSubscribeRequest,
                                    notifyResponse,
                                    SIP_ACCEPTED_CODE,
                                    FALSE,
                                    0,
                                    NULL));
         CPPUNIT_ASSERT(!subscribeRequest);
         CPPUNIT_ASSERT(notifyResponse);
         CPPUNIT_ASSERT_EQUAL(SIP_OK_CODE,
                              notifyResponse->getResponseStatusCode());

         // Wait until both subscriptions should have sent re-SUBSCRIBEs.

         fprintf(stderr, "Waiting %d seconds...\n", refreshTime);
         OsTask::delay(refreshTime * 1000);

         // Wait for SUBSCRIBEs.
         UtlBoolean firstForkResubscribe = FALSE;
         UtlBoolean secondForkResubscribe = FALSE;
         {
            // Read messages until no more arrive.
            OsMsg* message;
            while (incomingServerMsgQueue.receive(message, timeout1sec) ==
                   OS_SUCCESS)
            {
               int msgType = message->getMsgType();
               int msgSubType = message->getMsgSubType();
               CPPUNIT_ASSERT(msgType == OsMsg::PHONE_APP);
               CPPUNIT_ASSERT(msgSubType == SipMessage::NET_SIP_MESSAGE);
               const SipMessage* sipMessage = ((SipMessageEvent*) message)->getMessage();
               CPPUNIT_ASSERT(sipMessage);
               int messageType = ((SipMessageEvent*) message)->getMessageStatus();
               CPPUNIT_ASSERT(messageType == SipMessageEvent::APPLICATION);

               CPPUNIT_ASSERT(!sipMessage->isResponse());
               UtlString method;
               sipMessage->getRequestMethod(&method);
               CPPUNIT_ASSERT(method.compareTo(SIP_SUBSCRIBE_METHOD) == 0);

               // Check the to-tag of the message.
               // (There may be repetitions of the re-SUBSCRIBEs.)
               Url toUrl;
               sipMessage->getToUrl(toUrl);
               UtlString toTag;
               toUrl.getFieldParameter("tag", toTag);
               if (toTag.compareTo(subscribeToTag) == 0)
               {
                  firstForkResubscribe = TRUE;
               }
               else if (toTag.compareTo(secondToTag) == 0)
               {
                  secondForkResubscribe = TRUE;
               }
            }
         }
         CPPUNIT_ASSERT(firstForkResubscribe);
         CPPUNIT_ASSERT(secondForkResubscribe);
      }

};

CPPUNIT_TEST_SUITE_REGISTRATION(SipSubscribeClientTest4);

// Repeat the tests in SipSubscribeClientTest3, but force TCP transport.
class SipSubscribeClientTest5 : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SipSubscribeClientTest5);
#ifdef EXECUTE_SLOW_TESTS
   CPPUNIT_TEST(subscribeMwiClientTest);
#endif // EXECUTE_SLOW_TESTS
   CPPUNIT_TEST(contactTest);
   CPPUNIT_TEST_SUITE_END();

public:

   SipSubscribeClientTest5()
      : subIncomingServerMsgQueue("SipSubscribeClientTest5::subIncomingServerMsgQueue")
      , subIncomingClientMsgQueue("SipSubscribeClientTest5::subIncomingClientMsgQueue")
      , notIncomingServerMsgQueue("SipSubscribeClientTest5::notIncomingServerMsgQueue")
      , notIncomingClientMsgQueue("SipSubscribeClientTest5::notIncomingClientMsgQueue")
      {
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
                  int len;
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

   // Variables used by setUp/tearDown and the tests.

   UtlString hostIp;
   SipUserAgent* userAgentp;
   UtlString notifier_addr_spec;
   UtlString notifier_name_addr;
   UtlString notifier_contact_name_addr;
   UtlString resource_id;
   UtlString subscriber_addr_spec;
   UtlString subscriber_name_addr;
   UtlString subscriber_contact_name_addr;
   UtlString eventTypeKey;
   UtlString eventType;
   SipDialogMgr* clientDialogMgrp;
   SipRefreshManager* refreshMgrp;
   SipSubscribeClient* subClientp;
   SipSubscribeServer* subServerp;
   SipSubscriptionMgr* subMgrp;
   OsMsgQ subIncomingServerMsgQueue;
   OsMsgQ subIncomingClientMsgQueue;
   OsMsgQ notIncomingServerMsgQueue;
   OsMsgQ notIncomingClientMsgQueue;

   void setUp()
      {
         hostIp = "127.0.0.1";
         eventTypeKey = SIP_EVENT_MESSAGE_SUMMARY;
         eventType = eventTypeKey;

         // defaultSipAddress is set to force the SipUserAgent to open
         // ports on that interface.
         // publicAddress is set to force the SipUserAgent to report
         // that as its address.
         userAgentp = new SipUserAgent(PORT_DEFAULT, PORT_NONE, PORT_NONE,
                                       hostIp, NULL, hostIp);
         userAgentp->start();

         // Construct the URI of the notifier.
         // Also construct the name-addr version of the URI, which may be
         // different if it has a "transport" parameter.
         // And the resource-id to use, which is the AOR with any
         // parameters stripped off.
         {
            char buffer[100];
            sprintf(buffer, "sip:222@%s:%d", hostIp.data(),
                    userAgentp->getTcpPort());
            resource_id = buffer;

            // Specify TCP transport, because that's all the UA listens to.
            // Using an address with a transport parameter also exercises
            // SipDialog to ensure it handles contact addresses with parameters.
            strcat(buffer, ";transport=tcp");
            notifier_addr_spec = buffer;

            Url notifier_uri(buffer, TRUE);
            notifier_uri.toString(notifier_name_addr);
            notifier_uri.setUserId(NULL);
            notifier_uri.toString(notifier_contact_name_addr);
         }

         // Construct the URI of the subscriber.
         // Also construct the name-addr version of the URI, which may be
         // different if it has a "transport" parameter.
         {
            char buffer[100];
            sprintf(buffer, "sip:111@%s:%d", hostIp.data(),
                    userAgentp->getTcpPort());

            // Specify TCP transport, because that's all the UA listens to.
            // Using an address with a transport parameter also exercises
            // SipDialog to ensure it handles contact addresses with parameters.
            strcat(buffer, ";transport=tcp");
            subscriber_addr_spec = buffer;

            Url subscriber_uri(buffer, TRUE);
            subscriber_uri.toString(subscriber_name_addr);
            subscriber_uri.setUserId(NULL);
            subscriber_uri.toString(subscriber_contact_name_addr);
         }

         // Set up the subscribe client
         clientDialogMgrp = new SipDialogMgr;
         refreshMgrp = new SipRefreshManager(*userAgentp, *clientDialogMgrp);
         refreshMgrp->start();
         subClientp = new SipSubscribeClient(*userAgentp, *clientDialogMgrp, *refreshMgrp);
         subClientp->start();

         // Set up the subscribe server
         subServerp = SipSubscribeServer::buildBasicServer(*userAgentp,
                                                           eventType);
         subMgrp = subServerp->getSubscriptionMgr(eventType);
         subMgrp->getDialogMgr();
         subServerp->getPublishMgr(eventType);

         subServerp->start();
         // Enable the handler for the MWI server
         subServerp->enableEventType(eventType, userAgentp);

         // Publish content, so we can subscribe to the URI.
         SipPublishContentMgr* pubMgrp = subServerp->getPublishMgr(eventType);
         
         const char* cc = "This is a test\n";
         HttpBody* c = new HttpBody(cc, strlen(cc), eventType);
         pubMgrp->publish(resource_id,
                          eventType,
                          eventType,
                          1,
                          &c,
                          TRUE);

         // Create a SUBSCRIBE observer
         // Register an interest in SUBSCRIBE requests
         // for this event type
         userAgentp->addMessageObserver(subIncomingServerMsgQueue,
                                        SIP_SUBSCRIBE_METHOD,
                                        TRUE, // requests
                                        FALSE, // no reponses
                                        TRUE, // incoming
                                        FALSE, // no outgoing
                                        eventType,
                                        NULL,
                                        NULL);

         userAgentp->addMessageObserver(subIncomingClientMsgQueue,
                                        SIP_SUBSCRIBE_METHOD,
                                        FALSE, // no requests
                                        TRUE, // reponses
                                        TRUE, // incoming
                                        FALSE, // no outgoing
                                        eventType,
                                        NULL,
                                        NULL);

         // Create a NOTIFY observer
         // Register an interest in NOTIFY requests
         // for this event type
         userAgentp->addMessageObserver(notIncomingClientMsgQueue,
                                        SIP_NOTIFY_METHOD,
                                        TRUE, // requests
                                        FALSE, // no reponses
                                        TRUE, // incoming
                                        FALSE, // no outgoing
                                        eventType,
                                        NULL,
                                        NULL);

         userAgentp->addMessageObserver(notIncomingServerMsgQueue,
                                        SIP_NOTIFY_METHOD,
                                        FALSE, // no requests
                                        TRUE, // reponses
                                        TRUE, // incoming
                                        FALSE, // no outgoing
                                        eventType,
                                        NULL,
                                        NULL);
      }

   void tearDown()
      {
         // Unregister the queues so we stop receiving messages on them
         userAgentp->removeMessageObserver(subIncomingServerMsgQueue);
         userAgentp->removeMessageObserver(subIncomingClientMsgQueue);
         userAgentp->removeMessageObserver(notIncomingServerMsgQueue);
         userAgentp->removeMessageObserver(notIncomingClientMsgQueue);

         OsTask::delay(1000);   // 1 second to let other threads clean up

         subClientp->requestShutdown();
         refreshMgrp->requestShutdown();
         subServerp->requestShutdown();
         subServerp->shutdown();
         userAgentp->shutdown(TRUE);

         OsTask::delay(1000);   // 1 second to let other threads clean up

         delete subMgrp;
         delete subServerp;
         delete subClientp;
         delete refreshMgrp;
         delete clientDialogMgrp;
         delete userAgentp;
      }

   void subscribeMwiClientTest()
      {
         smClientExpiration = -1;
         smNumClientNotifiesReceived = 0;
         smLastClientNotifyReceived = NULL;
         smNumClientSubResponsesReceived = 0;
         smLastClientSubResponseReceived = NULL;

         // Get the Content Manager.
         SipPublishContentMgr* contentMgrp = subServerp->getPublishMgr(eventType);

         // Should not be any pre-existing content
         HttpBody* preexistingBodyPtr;
         UtlBoolean isDefaultContent;
         CPPUNIT_ASSERT(contentMgrp->getContent(resource_id,
                                                eventTypeKey,
                                                eventType,
                                                TRUE,
                                                SipPublishContentMgr::acceptAllTypes,
                                                preexistingBodyPtr,
                                                isDefaultContent,
                                                NULL));
         int numDefaultContent = -1;
         int numDefaultConstructor = -1;
         int numResourceSpecificContent = -1;
         int numCallbacksRegistered = -1;
         contentMgrp->getStats(numDefaultContent,
                               numDefaultConstructor,
                               numResourceSpecificContent,
                               numCallbacksRegistered);
         CPPUNIT_ASSERT(numDefaultContent == 0);
         CPPUNIT_ASSERT(numDefaultConstructor == 0);
         CPPUNIT_ASSERT(numResourceSpecificContent == 1);
         CPPUNIT_ASSERT(numCallbacksRegistered == 1);

         // Set up a subscription.
         UtlString earlyDialogHandle;
         CPPUNIT_ASSERT(subClientp->addSubscription(notifier_addr_spec,
                                                    eventType,
                                                    NULL,
                                                    subscriber_name_addr.data(),
                                                    notifier_name_addr.data(),
                                                    NULL, // Let SipUserAgent choose Contact
                                                    60, // seconds expiration
                                                    this,
                                                    subStateCallback,
                                                    notifyCallback,
                                                    earlyDialogHandle));

         contentMgrp->getStats(numDefaultContent,
                               numDefaultConstructor,
                               numResourceSpecificContent,
                               numCallbacksRegistered);
         CPPUNIT_ASSERT(numDefaultContent == 0);
         CPPUNIT_ASSERT(numDefaultConstructor == 0);
         CPPUNIT_ASSERT(numResourceSpecificContent == 1);
         CPPUNIT_ASSERT(numCallbacksRegistered == 1);

         // See if a subscribe was sent and received
         const SipMessage* serverSideSubRequest = NULL;
         CPPUNIT_ASSERT(removeMessage(subIncomingServerMsgQueue,
                                      5000, // milliseconds
                                      serverSideSubRequest));
         CPPUNIT_ASSERT(serverSideSubRequest); // Sub request got to server

         // Check the Contact in the subscribe request.
         ASSERT_STR_EQUAL(subscriber_contact_name_addr,
                          serverSideSubRequest->
                              getHeaderValue(0, SIP_CONTACT_FIELD));

         const SipMessage* clientSideSubResponse = NULL;
         CPPUNIT_ASSERT(removeMessage(subIncomingClientMsgQueue,
                                      5000, // milliseconds
                                      clientSideSubResponse));
         CPPUNIT_ASSERT(clientSideSubResponse);
         CPPUNIT_ASSERT_EQUAL(SIP_ACCEPTED_CODE,
                              clientSideSubResponse->getResponseStatusCode());

         // Check the Contact in the subscribe response.
         ASSERT_STR_EQUAL(notifier_name_addr,
                          clientSideSubResponse->
                              getHeaderValue(0, SIP_CONTACT_FIELD));

         int waitIterations = 0;
         while(smLastClientNotifyReceived == NULL ||
               smLastClientSubResponseReceived == NULL)
         {
            OsTask::delay(100);
            waitIterations++;
            if(waitIterations >= 100)
            {
               break;
            }
         }

         CPPUNIT_ASSERT(smLastClientSubResponseReceived);
         CPPUNIT_ASSERT(smLastClientNotifyReceived);
         SipMessage* firstSubResponse = smLastClientSubResponseReceived;
         smLastClientSubResponseReceived = NULL;
         int firstSubCseq;
         firstSubResponse->getCSeqField(&firstSubCseq, NULL);
         SipMessage* firstNotifyRequest = smLastClientNotifyReceived;
         smLastClientNotifyReceived = NULL;
         int firstNotifyCseq;
         firstNotifyRequest->getCSeqField(&firstNotifyCseq, NULL);
         CPPUNIT_ASSERT(firstSubCseq == 1);
         CPPUNIT_ASSERT(firstNotifyCseq == 1);

         // The refresh manager should re-SUBSCRIBE
         // Wait for the next notify request and subscribe response
         int secondMessageWait = 60;
         int resendTimeout = (int) (0.55 * secondMessageWait);
         if(resendTimeout < 40)
         {
            resendTimeout = 40;
         }
         for(int i = 0; i < secondMessageWait - 1; i++)
         {
            if(i == resendTimeout - 1)
            {
               printf("v");
            }
            else
            {
               printf("=");
            }
         }
         printf("v\n");
         SipMessage* secondSubResponse = NULL;
         SipMessage* secondNotifyRequest = NULL;

         while(secondNotifyRequest == NULL ||
               secondSubResponse == NULL)
         {
            OsTask::delay(1000);
            if(smLastClientSubResponseReceived)
            {
               secondSubResponse = smLastClientSubResponseReceived;
               smLastClientSubResponseReceived = NULL;
            }
            if(smLastClientNotifyReceived)
            {
               secondNotifyRequest = smLastClientNotifyReceived;
               smLastClientNotifyReceived = NULL;
            }
            printf(".");
            fflush(stdout);
            waitIterations++;
            if(waitIterations >= secondMessageWait)
            {
               printf("!");
               break;
            }
         }
         printf("\n");

         CPPUNIT_ASSERT(removeMessage(subIncomingServerMsgQueue,
                                      5000, // milliseconds
                                      serverSideSubRequest));
         CPPUNIT_ASSERT(serverSideSubRequest); // Sub request got to server

         CPPUNIT_ASSERT(removeMessage(subIncomingClientMsgQueue,
                                      5000, // milliseconds
                                      clientSideSubResponse));
         CPPUNIT_ASSERT(clientSideSubResponse); // Sub response got to client
         CPPUNIT_ASSERT_EQUAL(SIP_ACCEPTED_CODE,
                              clientSideSubResponse->getResponseStatusCode());

         CPPUNIT_ASSERT(secondNotifyRequest);
         CPPUNIT_ASSERT(secondSubResponse);
         int secondSubCseq = -1;
         int secondNotifyCseq = -1;
         smLastClientSubResponseReceived = NULL;
         secondSubResponse->getCSeqField(&secondSubCseq, NULL);
         smLastClientNotifyReceived = NULL;
         secondNotifyRequest->getCSeqField(&secondNotifyCseq, NULL);
         CPPUNIT_ASSERT(firstSubCseq < secondSubCseq);
         CPPUNIT_ASSERT(firstNotifyCseq < secondNotifyCseq);
      }

   // Test that the Contact headers on SUBSCRIBE and NOTIFY requests
   // and responses are what is expected.
   // This tests XECS-297, XECS-298, and XECS-299.
   void contactTest()
      {
         // Set up a subscription.
         UtlString earlyDialogHandle;
         CPPUNIT_ASSERT(subClientp->addSubscription(notifier_addr_spec,
                                                    eventType,
                                                    NULL,
                                                    subscriber_name_addr.data(),
                                                    notifier_name_addr.data(),
                                                    NULL, // Let SipUserAgent choose Contact
                                                    60, // seconds expiration
                                                    this,
                                                    subStateCallback,
                                                    notifyCallback,
                                                    earlyDialogHandle));

         // See if a subscribe was sent and received
         const SipMessage* serverSideSubRequest = NULL;
         CPPUNIT_ASSERT(removeMessage(subIncomingServerMsgQueue,
                                      5000, // milliseconds
                                      serverSideSubRequest));
         CPPUNIT_ASSERT(serverSideSubRequest); // Sub request got to server

         // Check the Contact in the subscribe request.
         ASSERT_STR_EQUAL(subscriber_contact_name_addr,
                          serverSideSubRequest->
                              getHeaderValue(0, SIP_CONTACT_FIELD));

         const SipMessage* clientSideSubResponse = NULL;
         CPPUNIT_ASSERT(removeMessage(subIncomingClientMsgQueue,
                                      5000, // milliseconds
                                      clientSideSubResponse));
         CPPUNIT_ASSERT(clientSideSubResponse);
         CPPUNIT_ASSERT_EQUAL(SIP_ACCEPTED_CODE,
                              clientSideSubResponse->getResponseStatusCode());

         // Check the Contact in the subscribe response.
         ASSERT_STR_EQUAL(notifier_name_addr,
                          clientSideSubResponse->
                              getHeaderValue(0, SIP_CONTACT_FIELD));

         // See if a notify was sent and received
         const SipMessage* clientSideNotRequest = NULL;
         CPPUNIT_ASSERT(removeMessage(notIncomingClientMsgQueue,
                                      5000, // milliseconds
                                      clientSideNotRequest));
         CPPUNIT_ASSERT(clientSideNotRequest); // NOTIFY request got to client

         // Check the Contact in the NOTIFY request.
         ASSERT_STR_EQUAL(notifier_name_addr,
                          clientSideNotRequest->
                              getHeaderValue(0, SIP_CONTACT_FIELD));

         const SipMessage* serverSideNotResponse = NULL;
         CPPUNIT_ASSERT(removeMessage(notIncomingServerMsgQueue,
                                      5000, // milliseconds
                                      serverSideNotResponse));
         CPPUNIT_ASSERT(serverSideNotResponse);
         CPPUNIT_ASSERT_EQUAL(SIP_OK_CODE,
                              serverSideNotResponse->getResponseStatusCode());

         // Check the Contact in the NOTIFY response.
         CPPUNIT_ASSERT(serverSideNotResponse->
                        getHeaderValue(0, SIP_CONTACT_FIELD));
         ASSERT_STR_EQUAL(subscriber_contact_name_addr,
                          serverSideNotResponse->
                              getHeaderValue(0, SIP_CONTACT_FIELD));
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SipSubscribeClientTest5);
