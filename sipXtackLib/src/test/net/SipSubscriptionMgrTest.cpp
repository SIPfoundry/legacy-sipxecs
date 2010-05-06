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

#include <os/OsDefs.h>
#include <os/OsDateTime.h>
#include <net/SipDialog.h>
#include <net/SipMessage.h>
#include <net/SipDialogMgr.h>
#include <net/SipSubscriptionMgr.h>
#include <net/SipSubscribeServerEventHandler.h>


/**
 * Unittest for SipSubscriptionMgr
 */
class SipSubscriptionMgrTest : public CppUnit::TestCase
{
      CPPUNIT_TEST_SUITE(SipSubscriptionMgrTest);
      CPPUNIT_TEST(subscriptionTest);
      CPPUNIT_TEST_SUITE_END();

      public:

   void subscriptionTest()
   {

          // Test MWI messages
const char* mwiSubscribe="SUBSCRIBE sip:111@example.com SIP/2.0\r\n\
From: \"Dan Petrie\"<sip:111@example.com>;tag=1612c1612\r\n\
To: \"Dan Petrie\"<sip:111@example.com>\r\n\
Call-Id: e2aab34a72a0eb18300fbec445d5d665\r\n\
Cseq: 1 SUBSCRIBE\r\n\
Contact: sip:111@10.1.2.3\r\n\
Event: message-summary\r\n\
Accept: application/simple-message-summary\r\n\
Expires: 3600\r\n\
Date: Tue, 26 Apr 2005 14:59:30 GMT\r\n\
Max-Forwards: 20\r\n\
User-Agent: Pingtel/2.2.0 (VxWorks)\r\n\
Accept-Language: en\r\n\
Supported: sip-cc, sip-cc-01, timer, replaces\r\n\
Via: SIP/2.0/UDP 10.1.2.3;branch=z9hG4bK7ce947ad9439bfeb6226852d87f5cca8\r\n\
Content-Length: 0\r\n\
\r\n";

const char* mwiSubscribe401 = "SIP/2.0 401 Unauthorized\r\n\
From: \"Dan Petrie\"<sip:111@example.com>;tag=1612c1612\r\n\
To: \"Dan Petrie\"<sip:111@example.com>\r\n\
Call-Id: e2aab34a72a0eb18300fbec445d5d665\r\n\
Cseq: 1 SUBSCRIBE\r\n\
Via: SIP/2.0/UDP 10.1.2.3;branch=z9hG4bK7ce947ad9439bfeb6226852d87f5cca8\r\n\
Www-Authenticate: Digest realm=\"example.com\", nonce=\"606a7e9c58258179f966b0987a1bf38d1114527548\", opaque=\"change4\"\r\n\
Date: Tue, 26 Apr 2005 14:59:08 GMT\r\n\
Allow: INVITE, ACK, CANCEL, BYE, REFER, OPTIONS, NOTIFY, SUBSCRIBE\r\n\
User-Agent: sipX/2.8.0 (Linux)\r\n\
Accept-Language: en\r\n\
Supported: sip-cc-01, timer\r\n\
Contact: sip:10.1.2.4:5110\r\n\
\r\n";

const char* mwiSubscribeAuth = "SUBSCRIBE sip:111@example.com SIP/2.0\r\n\
From: \"Dan Petrie\"<sip:111@example.com>;tag=1612c1612\r\n\
To: \"Dan Petrie\"<sip:111@example.com>\r\n\
Call-Id: e2aab34a72a0eb18300fbec445d5d665\r\n\
Cseq: 2 SUBSCRIBE\r\n\
Contact: sip:111@10.1.2.3\r\n\
Event: message-summary;foo=bar;id=44\r\n\
Accept: application/simple-message-summary\r\n\
Expires: 3600\r\n\
Date: Tue, 26 Apr 2005 14:59:30 GMT\r\n\
Max-Forwards: 20\r\n\
User-Agent: Pingtel/2.2.0 (VxWorks)\r\n\
Accept-Language: en\r\n\
Supported: sip-cc, sip-cc-01, timer, replaces\r\n\
Authorization: Digest username=\"111\", realm=\"example.com\", nonce=\"606a7e9c58258179f966b0987a1bf38d1114527548\", uri=\"sip:111@example.com\", response=\"feaa478e10ee7d3ef6037746696bace6\", opaque=\"change4\"\r\n\
Via: SIP/2.0/UDP 10.1.1.177;branch=z9hG4bK64807d4040eecf1d8b0ae759505b81b0\r\n\
Content-Length: 0\r\n\
\r\n";

const char* mwiSubscribe202 = "SIP/2.0 202 Accepted\r\n\
Expires: 3600\r\n\
From: \"Dan Petrie\"<sip:111@example.com>;tag=1612c1612\r\n\
To: \"Dan Petrie\"<sip:111@example.com>;tag=435889744\r\n\
Call-Id: e2aab34a72a0eb18300fbec445d5d665\r\n\
Cseq: 2 SUBSCRIBE\r\n\
Via: SIP/2.0/UDP 10.1.1.177;branch=z9hG4bK64807d4040eecf1d8b0ae759505b81b0\r\n\
Contact: sip:111@example.com\r\n\
Date: Tue, 26 Apr 2005 14:59:08 GMT\r\n\
Allow: INVITE, ACK, CANCEL, BYE, REFER, OPTIONS, NOTIFY, SUBSCRIBE\r\n\
User-Agent: sipX/2.8.0 (Linux)\r\n\
Accept-Language: en\r\n\
Supported: sip-cc-01, timer\r\n\
Content-Length: 0\r\n\
\r\n";

const char* mwiNotify = "NOTIFY sip:111@10.1.1.177 SIP/2.0\r\n\
Content-Type: application/simple-message-summary\r\n\
Content-Length: 50\r\n\
Event: message-summary\r\n\
Subscription-State: active;expires=3600\r\n\
From: \"Dan Petrie\"<sip:111@example.com>;tag=435889744\r\n\
To: \"Dan Petrie\"<sip:111@example.com>;tag=1612c1612\r\n\
Call-Id: e2aab34a72a0eb18300fbec445d5d665\r\n\
Cseq: 10 NOTIFY\r\n\
Contact: sip:10.1.20.3:5110\r\n\
Date: Tue, 26 Apr 2005 14:59:08 GMT\r\n\
Max-Forwards: 20\r\n\
User-Agent: sipX/2.8.0 (Linux)\r\n\
Accept-Language: en\r\n\
Supported: sip-cc-01, timer\r\n\
Via: SIP/2.0/UDP 10.1.20.3:5110;branch=z9hG4bK-1334bee34ff713f3b58e898d1a2eaf06\r\n\
\r\n\
Messages-Waiting: no\r\n\
Voice-Message: 0/0 (0/0)\r\n";

        // Set up some subscribe messages
         SipMessage mwiSubRequest(mwiSubscribe);
         SipMessage mwiSub401Response(mwiSubscribe401);
         SipMessage mwiSubWithAuthRequest(mwiSubscribeAuth);
         SipMessage mwiSub202Response(mwiSubscribe202);
         SipMessage mwiNotifyRequest(mwiNotify);

         // Dummy OsMsgQ to receive resend messages from subMgr.
         // Allocate before subMgr, which will point to it.
         OsMsgQ msgQ("SipSubscriptionMgrTest::subscriptionTest::magQ");
         SipSubscriptionMgr subMgr;
         subMgr.initialize(&msgQ);
         
         SipDialogMgr* dialogMgr = subMgr.getDialogMgr();
         SipSubscribeServerEventHandler eventHandler;
         UtlString resourceId;
         UtlString eventTypeKey, eventType;
         eventHandler.getKeys(mwiSubWithAuthRequest,
                              resourceId,
                              eventTypeKey,
                              eventType);
         ASSERT_STR_EQUAL(resourceId.data(), "sip:111@example.com");
         ASSERT_STR_EQUAL(eventTypeKey.data(), "message-summary");
         ASSERT_STR_EQUAL(eventType.data(), "message-summary");

         UtlString subscribeDialogHandle;
         UtlString earlyDialogHandle;
         mwiSubWithAuthRequest.getDialogHandle(earlyDialogHandle);
         UtlBoolean isNew;
         UtlBoolean isExpired;
         SipMessage createdSubscribeResponse;
         UtlString mwiSubcribeFromField;
         mwiSubWithAuthRequest.getFromField(&mwiSubcribeFromField);

         // Create the subscription with updateDialogInfo
         CPPUNIT_ASSERT(subMgr.updateDialogInfo(mwiSubWithAuthRequest,
                                                resourceId,
                                                eventTypeKey,
                                                eventType,
                                                subscribeDialogHandle,
                                                isNew,
                                                isExpired,
                                                createdSubscribeResponse,
                                                eventHandler));

         // Validate the results from updateDialogInfo
         CPPUNIT_ASSERT(isNew);
         CPPUNIT_ASSERT(!isExpired);
         UtlString responseDialogHandle;
         createdSubscribeResponse.getDialogHandle(responseDialogHandle);
         ASSERT_STR_EQUAL(responseDialogHandle, subscribeDialogHandle);
         CPPUNIT_ASSERT(!SipDialog::isEarlyDialog(subscribeDialogHandle));
         CPPUNIT_ASSERT(!SipDialog::isEarlyDialog(responseDialogHandle));
         CPPUNIT_ASSERT(dialogMgr->countDialogs() == 1);
         UtlString mgrEstablishedDialogHandle;
         CPPUNIT_ASSERT(dialogMgr->getEstablishedDialogHandleFor(earlyDialogHandle,
             mgrEstablishedDialogHandle));
         CPPUNIT_ASSERT(dialogMgr->dialogExists(subscribeDialogHandle));
         int expiration = 0;
         CPPUNIT_ASSERT(createdSubscribeResponse.getExpiresField(&expiration));
         CPPUNIT_ASSERT(expiration <= 3600);
         UtlString mwiSubResponseToField;
         createdSubscribeResponse.getToField(&mwiSubResponseToField);

         // Create a notify with getNotifyDialogInfo
         SipMessage nextNotify;
         CPPUNIT_ASSERT(subMgr.getNotifyDialogInfo(subscribeDialogHandle,
                                                   nextNotify,
                                                   "active;expires=1234"));

         // Validate the results from getNotifyDialogInfo
         int nextNotifyCseq;
         UtlString nextNotifyMethod;
         nextNotify.getCSeqField(&nextNotifyCseq, &nextNotifyMethod);
         CPPUNIT_ASSERT(nextNotifyCseq == 1);
         ASSERT_STR_EQUAL(SIP_NOTIFY_METHOD, nextNotifyMethod);
         UtlString nextNotifyDialogHandle;
         nextNotify.getDialogHandle(nextNotifyDialogHandle);
         ASSERT_STR_EQUAL(mgrEstablishedDialogHandle, nextNotifyDialogHandle);
         UtlString mwiEventHeader;
         mwiSubWithAuthRequest.getEventField(mwiEventHeader);
         UtlString nextNotifyEventHeader;
         CPPUNIT_ASSERT(nextNotify.getEventField(nextNotifyEventHeader));
         ASSERT_STR_EQUAL(mwiEventHeader, nextNotifyEventHeader);
         ASSERT_STR_EQUAL("message-summary;foo=bar;id=44", nextNotifyEventHeader);

         // Get notifies for all of the subscriptions for a resourceId
         // TODO: Should probably add more subscriptions for the same resource
         int numNotifiesCreated;
         UtlString* acceptHeaderValuesArray = NULL;
         SipMessage* notifyArray = NULL;
         bool* fullContentArray = NULL;
         subMgr.createNotifiesDialogInfo(resourceId,
                                         eventTypeKey,
                                         "active;expires=%ld",
                                         numNotifiesCreated,
                                         acceptHeaderValuesArray,
                                         fullContentArray,
                                         notifyArray);
         CPPUNIT_ASSERT(numNotifiesCreated == 1);
         CPPUNIT_ASSERT(acceptHeaderValuesArray);
         CPPUNIT_ASSERT(notifyArray);
         SipMessage* notify0FromArray = &notifyArray[0];
         CPPUNIT_ASSERT(notify0FromArray);
         int arrayNotify0Cseq;
         UtlString arrayNotify0Method;
         CPPUNIT_ASSERT(notify0FromArray->getCSeqField(&arrayNotify0Cseq,
                                                       &arrayNotify0Method));
         CPPUNIT_ASSERT(arrayNotify0Cseq == 2);
         UtlString arrayNotify0DialogHandle;
         notify0FromArray->getDialogHandle(arrayNotify0DialogHandle);
         ASSERT_STR_EQUAL(mgrEstablishedDialogHandle, arrayNotify0DialogHandle);
         UtlString notify0FromArrayToField;
         UtlString notify0FromArrayFromField;
         notify0FromArray->getToField(&notify0FromArrayToField);
         notify0FromArray->getFromField(&notify0FromArrayFromField);
         ASSERT_STR_EQUAL(mwiSubcribeFromField, notify0FromArrayToField);
         ASSERT_STR_EQUAL(mwiSubResponseToField, notify0FromArrayFromField);
         UtlString notify0FromArrayEventField;
         notify0FromArray->getEventField(notify0FromArrayEventField);
         ASSERT_STR_EQUAL("message-summary;foo=bar;id=44", notify0FromArrayEventField);

         delete[] acceptHeaderValuesArray;
         delete[] notifyArray;
         delete[] fullContentArray;

         CPPUNIT_ASSERT(subMgr.dialogExists(subscribeDialogHandle));
         CPPUNIT_ASSERT(!subMgr.isExpired(subscribeDialogHandle));

         // End the dialog and subscription
         subMgr.endSubscription(subscribeDialogHandle,
                                SipSubscriptionMgr::subscriptionTerminated);
         CPPUNIT_ASSERT(dialogMgr->countDialogs() == 0);
         CPPUNIT_ASSERT(!subMgr.dialogExists(subscribeDialogHandle));
         CPPUNIT_ASSERT(subMgr.isExpired(subscribeDialogHandle));

         // Add the subscription back in again
         CPPUNIT_ASSERT(subMgr.updateDialogInfo(mwiSubWithAuthRequest,
                                                resourceId,
                                                eventTypeKey,
                                                eventType,
                                                subscribeDialogHandle,
                                                isNew,
                                                isExpired,
                                                createdSubscribeResponse,
                                                eventHandler));
         CPPUNIT_ASSERT(dialogMgr->countDialogs() == 1);

         long now = OsDateTime::getSecsSinceEpoch();
         now+=3700;
         subMgr.removeOldSubscriptions(now);
         CPPUNIT_ASSERT(dialogMgr->countDialogs() == 0);
         CPPUNIT_ASSERT(subMgr.isExpired(subscribeDialogHandle));
         CPPUNIT_ASSERT(!subMgr.dialogExists(subscribeDialogHandle));

      }

};

CPPUNIT_TEST_SUITE_REGISTRATION(SipSubscriptionMgrTest);
