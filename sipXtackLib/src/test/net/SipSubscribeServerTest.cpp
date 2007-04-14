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
#include <net/SipUserAgent.h>
#include <net/SipSubscribeServer.h>
#include <net/SipPublishContentMgr.h>

#define UNIT_TEST_SIP_PORT 44444

/**
 * Unittest for SipSubscriptionMgr
 */
class SipSubscribeServerTest : public CppUnit::TestCase
{
      CPPUNIT_TEST_SUITE(SipSubscribeServerTest);
      CPPUNIT_TEST(subscriptionTest);
      CPPUNIT_TEST_SUITE_END();

      public:

   void subscriptionTest()
   {
#    ifdef __linux__
      KNOWN_BUG("SipSubscribeServerTest hangs on linux (skipped)", "XSL-150");
      CPPUNIT_ASSERT(false);
#    else
       UtlString hostIp;
       OsSocket::getHostIp(&hostIp);

        // Test MWI messages
        char* mwiSubscribe="SUBSCRIBE sip:111@localhost SIP/2.0\r\n\
From: \"Dan Petrie\"<sip:111@example.com>;tag=1612c1612\r\n\
To: \"Dan Petrie\"<sip:111@example.com>\r\n\
Call-Id: e2aab34a72a0eb18300fbec445d5d665\r\n\
Cseq: 1 SUBSCRIBE\r\n\
Event: message-summary\r\n\
Accept: application/simple-message-summary\r\n\
Expires: 3600\r\n\
Date: Tue, 26 Apr 2005 14:59:30 GMT\r\n\
Max-Forwards: 20\r\n\
User-Agent: Pingtel/2.2.0 (VxWorks)\r\n\
Accept-Language: en\r\n\
Supported: sip-cc, sip-cc-01, timer, replaces\r\n\
Content-Length: 0\r\n\
\r\n";

        const char* mwiStateString = "Messages-Waiting: no\r\n\
Voice-Message: 0/0 (0/0)\r\n";

       UtlString eventName("message-summary");
       UtlString mwiMimeType("application/simple-message-summary");
       SipUserAgent userAgent(UNIT_TEST_SIP_PORT, UNIT_TEST_SIP_PORT, 0, 0, hostIp );
       userAgent.start();

       SipSubscribeServer* subServer = 
           SipSubscribeServer::buildBasicServer(userAgent, 
                                                eventName);
       subServer->start();

       // Create a crude Subscription client
       OsMsgQ incomingClientMsgQueue;
        // Register an interest in SUBSCRIBE responses and NOTIFY requests
        // for this event type
       userAgent.addMessageObserver(incomingClientMsgQueue,
                                    SIP_SUBSCRIBE_METHOD,
                                    FALSE, // no requests
                                    TRUE, // reponses
                                    TRUE, // incoming
                                    FALSE, // no outgoing
                                    eventName,
                                    NULL,
                                    NULL);
       userAgent.addMessageObserver(incomingClientMsgQueue,
                                    SIP_NOTIFY_METHOD,
                                    TRUE, // requests
                                    FALSE, // not reponses
                                    TRUE, // incoming
                                    FALSE, // no outgoing
                                    eventName,
                                    NULL,
                                    NULL);

         //CPPUNIT_ASSERT(TRUE);
         //ASSERT_STR_EQUAL("a", "a");

       // Validate that authentication and authorization are
       // disabled by default.
       SipSubscribeServerEventHandler* eventHandler = 
           subServer->getEventHandler(eventName);
       CPPUNIT_ASSERT(eventHandler);
       SipSubscriptionMgr* subMgr = subServer->getSubscriptionMgr(eventName);
       CPPUNIT_ASSERT(subMgr);
       SipDialogMgr* dialogMgr = subMgr->getDialogMgr();
       CPPUNIT_ASSERT(dialogMgr);

       SipMessage bogusSubscribeRequest;
       SipMessage bogusSubscribeResponse;
       CPPUNIT_ASSERT(eventHandler->isAuthenticated(bogusSubscribeRequest,
                                                     "foo@bar.com",
                                                     eventName,
                                                     bogusSubscribeResponse));
       CPPUNIT_ASSERT(eventHandler->isAuthorized(bogusSubscribeRequest,
                                                 "foo@bar.com",
                                                 eventName,
                                                 bogusSubscribeResponse));



       // Send a subscribe to ourselves
       UtlString resourceId("111@localhost");
       char portString[20];
       sprintf(portString, "%d", UNIT_TEST_SIP_PORT);
       resourceId.append(':');
       resourceId.append(portString);
       UtlString aor("sip:");
       aor.append(resourceId);
       SipMessage mwiSubscribeRequest(mwiSubscribe);
       mwiSubscribeRequest.setSipRequestFirstHeaderLine(SIP_SUBSCRIBE_METHOD, 
                                                       aor, 
                                                       SIP_PROTOCOL_VERSION);
       mwiSubscribeRequest.setContactField(aor);

       UtlBoolean bRet(userAgent.send(mwiSubscribeRequest));
       CPPUNIT_ASSERT(bRet);

       // We should get a 202 response and a NOTIFY request in the queue
       OsTime messageTimeout(5, 0);  // 5 seconds
       OsMsg* osMessage = NULL;
       const SipMessage* subscribeResponse = NULL;
       const SipMessage* notifyRequest = NULL;
       incomingClientMsgQueue.receive(osMessage, messageTimeout);
       CPPUNIT_ASSERT(osMessage);
       int msgType = osMessage->getMsgType();
       int msgSubType = osMessage->getMsgSubType();
       CPPUNIT_ASSERT(msgType == OsMsg::PHONE_APP);
       CPPUNIT_ASSERT(msgSubType == SipMessage::NET_SIP_MESSAGE);
       const SipMessage* sipMessage = ((SipMessageEvent*)osMessage)->getMessage();
       int messageType = ((SipMessageEvent*)osMessage)->getMessageStatus();
       CPPUNIT_ASSERT(sipMessage);
       CPPUNIT_ASSERT(messageType == SipMessageEvent::APPLICATION);
       // SUBSCRIBE response and NOTIFY request can come in either order
       if(sipMessage->isResponse())
       {
           subscribeResponse = sipMessage;
       }
       else
       {
           notifyRequest = sipMessage;
           // Immediately generate a NOTIFY response
           SipMessage notifyResponse;
           notifyResponse.setResponseData(notifyRequest, 
                                          SIP_OK_CODE,
                                          SIP_OK_TEXT);
           userAgent.send(notifyResponse);
       }

       incomingClientMsgQueue.receive(osMessage, messageTimeout);
       msgType = osMessage->getMsgType();
       msgSubType = osMessage->getMsgSubType();
       CPPUNIT_ASSERT(msgType == OsMsg::PHONE_APP);
       CPPUNIT_ASSERT(msgSubType == SipMessage::NET_SIP_MESSAGE);
       sipMessage = ((SipMessageEvent*)osMessage)->getMessage();
       messageType = ((SipMessageEvent*)osMessage)->getMessageStatus();
       CPPUNIT_ASSERT(sipMessage);
       CPPUNIT_ASSERT(messageType == SipMessageEvent::APPLICATION);
       // SUBSCRIBE response and NOTIFY request can come in either order
       if(sipMessage->isResponse())
       {
           subscribeResponse = sipMessage;
       }
       else
       {
           notifyRequest = sipMessage;
           // Immediately generate a NOTIFY response
           SipMessage notifyResponse;
           notifyResponse.setResponseData(notifyRequest, 
                                          SIP_OK_CODE,
                                          SIP_OK_TEXT);
           userAgent.send(notifyResponse);
       }

       CPPUNIT_ASSERT(subscribeResponse);
       CPPUNIT_ASSERT(notifyRequest);

       UtlString subscribeMethod;
       UtlString notifyMethod;
       notifyRequest->getRequestMethod(&notifyMethod);
       subscribeResponse->getCSeqField(NULL, &subscribeMethod);
       ASSERT_STR_EQUAL(SIP_SUBSCRIBE_METHOD, subscribeMethod.data());
       ASSERT_STR_EQUAL(SIP_NOTIFY_METHOD, notifyMethod.data());
       UtlString notifyEventHeader;
       UtlString subscribeEventHeader;
       notifyRequest->getEventField(notifyEventHeader);
       mwiSubscribeRequest.getEventField(subscribeEventHeader);
       ASSERT_STR_EQUAL(subscribeEventHeader, notifyEventHeader);

       // No body because non have been published yet.
       const HttpBody* bodyPtr = NULL;
       bodyPtr = notifyRequest->getBody();
       CPPUNIT_ASSERT(bodyPtr == NULL);

       // Publish some content for this resourceID
       HttpBody newMwiBody(mwiStateString, 
                           strlen(mwiStateString), 
                           mwiMimeType);
       HttpBody* newMwiBodyPtr = &newMwiBody;
       HttpBody* oldMwiBody = NULL;
       int numOldContent;
       SipPublishContentMgr* publishMgr = subServer->getPublishMgr(eventName);
       CPPUNIT_ASSERT(publishMgr);
       CPPUNIT_ASSERT(publishMgr->publish(resourceId, 
                                          eventName, 
                                          eventName, 
                                          1, 
                                          &newMwiBodyPtr,
                                          1, // max
                                          numOldContent, 
                                          &oldMwiBody));

       // Should be no prior content for this resource or eventTypeKey
       CPPUNIT_ASSERT(numOldContent == 0);
       CPPUNIT_ASSERT(oldMwiBody == NULL);


       // SHould get a NOFITY queued up
       incomingClientMsgQueue.receive(osMessage, messageTimeout);
       msgType = osMessage->getMsgType();
       msgSubType = osMessage->getMsgSubType();
       CPPUNIT_ASSERT(msgType == OsMsg::PHONE_APP);
       CPPUNIT_ASSERT(msgSubType == SipMessage::NET_SIP_MESSAGE);
       const SipMessage* secondNotify = ((SipMessageEvent*)osMessage)->getMessage();
       messageType = ((SipMessageEvent*)osMessage)->getMessageStatus();
       CPPUNIT_ASSERT(secondNotify);
       CPPUNIT_ASSERT(messageType == SipMessageEvent::APPLICATION);

       CPPUNIT_ASSERT(!secondNotify->isResponse());
       // Immediately generate a NOTIFY response
       SipMessage secondNotifyResponse;
       secondNotifyResponse.setResponseData(notifyRequest, 
                                      SIP_OK_CODE,
                                      SIP_OK_TEXT);
       userAgent.send(secondNotifyResponse);

       const HttpBody* secondNotifyBody = secondNotify->getBody();
       CPPUNIT_ASSERT(secondNotifyBody);
       int notifyBodySize = 0;
       const char* notifyBodyBytes = NULL;
       secondNotifyBody->getBytes(&notifyBodyBytes, &notifyBodySize);
       CPPUNIT_ASSERT(notifyBodyBytes);
       ASSERT_STR_EQUAL(mwiStateString, notifyBodyBytes);
       CPPUNIT_ASSERT(notifyBodySize == strlen(mwiStateString));
       CPPUNIT_ASSERT(notifyBodySize > 10);  // just to make sure both aren't null
       UtlString secondNotifyDialogHandle;
       secondNotify->getDialogHandle(secondNotifyDialogHandle);
       CPPUNIT_ASSERT(!secondNotifyDialogHandle.isNull());
       CPPUNIT_ASSERT(dialogMgr->dialogExists(secondNotifyDialogHandle));
       CPPUNIT_ASSERT(dialogMgr->countDialogs() == 1);


       // Create a new one time subscribe
       SipMessage oneTimeMwiSubscribeRequest(mwiSubscribe);
       oneTimeMwiSubscribeRequest.setCallIdField("1234567890");
       oneTimeMwiSubscribeRequest.setExpiresField(0);
       oneTimeMwiSubscribeRequest.setSipRequestFirstHeaderLine(SIP_SUBSCRIBE_METHOD, 
                                                       aor, 
                                                       SIP_PROTOCOL_VERSION);
       oneTimeMwiSubscribeRequest.setContactField(aor);

       userAgent.send(oneTimeMwiSubscribeRequest);
       const SipMessage* oneTimeSubscribeResponse = NULL;
       const SipMessage* oneTimeNotifyRequest = NULL;

       // Get the subscribe response or notify request
       incomingClientMsgQueue.receive(osMessage, messageTimeout);
       msgType = osMessage->getMsgType();
       msgSubType = osMessage->getMsgSubType();
       CPPUNIT_ASSERT(msgType == OsMsg::PHONE_APP);
       CPPUNIT_ASSERT(msgSubType == SipMessage::NET_SIP_MESSAGE);
       sipMessage = ((SipMessageEvent*)osMessage)->getMessage();
       messageType = ((SipMessageEvent*)osMessage)->getMessageStatus();
       CPPUNIT_ASSERT(sipMessage);
       CPPUNIT_ASSERT(messageType == SipMessageEvent::APPLICATION);
       // SUBSCRIBE response and NOTIFY request can come in either order
       if(sipMessage->isResponse())
       {
           oneTimeSubscribeResponse = sipMessage;
       }
       else
       {
           oneTimeNotifyRequest = sipMessage;
           // Immediately generate a NOTIFY response
           SipMessage notifyResponse;
           notifyResponse.setResponseData(oneTimeNotifyRequest, 
                                          SIP_OK_CODE,
                                          SIP_OK_TEXT);
           userAgent.send(notifyResponse);
       }

       // Get the subscribe response or notify request
       incomingClientMsgQueue.receive(osMessage, messageTimeout);
       msgType = osMessage->getMsgType();
       msgSubType = osMessage->getMsgSubType();
       CPPUNIT_ASSERT(msgType == OsMsg::PHONE_APP);
       CPPUNIT_ASSERT(msgSubType == SipMessage::NET_SIP_MESSAGE);
       sipMessage = ((SipMessageEvent*)osMessage)->getMessage();
       messageType = ((SipMessageEvent*)osMessage)->getMessageStatus();
       CPPUNIT_ASSERT(sipMessage);
       CPPUNIT_ASSERT(messageType == SipMessageEvent::APPLICATION);
       // SUBSCRIBE response and NOTIFY request can come in either order
       if(sipMessage->isResponse())
       {
           oneTimeSubscribeResponse = sipMessage;
       }
       else
       {
           oneTimeNotifyRequest = sipMessage;
           // Immediately generate a NOTIFY response
           SipMessage notifyResponse;
           notifyResponse.setResponseData(oneTimeNotifyRequest, 
                                          SIP_OK_CODE,
                                          SIP_OK_TEXT);
           userAgent.send(notifyResponse);
       }

       // Validate the one time subscribe response and notify request
       CPPUNIT_ASSERT(oneTimeSubscribeResponse);
       CPPUNIT_ASSERT(oneTimeNotifyRequest);

       UtlString oneTimeSubscribeMethod;
       UtlString oneTimeNotifyMethod;
       oneTimeNotifyRequest->getRequestMethod(&oneTimeNotifyMethod);
       oneTimeSubscribeResponse->getCSeqField(NULL, &oneTimeSubscribeMethod);
       ASSERT_STR_EQUAL(SIP_SUBSCRIBE_METHOD, oneTimeSubscribeMethod.data());
       ASSERT_STR_EQUAL(SIP_NOTIFY_METHOD, oneTimeNotifyMethod.data());
       UtlString oneTimeNotifyEventHeader;
       UtlString oneTimeSubscribeEventHeader;
       oneTimeNotifyRequest->getEventField(notifyEventHeader);
       oneTimeSubscribeResponse->getEventField(subscribeEventHeader);
       ASSERT_STR_EQUAL(oneTimeSubscribeEventHeader, oneTimeNotifyEventHeader);

       const HttpBody* oneTimeBodyPtr = NULL;
       oneTimeBodyPtr = oneTimeNotifyRequest->getBody();
       CPPUNIT_ASSERT(oneTimeBodyPtr != NULL);
       const char* oneTimeBodyString = NULL;
       int oneTimeNotifyBodySize;
       oneTimeBodyPtr->getBytes(&oneTimeBodyString, &oneTimeNotifyBodySize);
       ASSERT_STR_EQUAL(mwiStateString, oneTimeBodyString);
       UtlString oneTimeNotifyDialogHandle;
       secondNotify->getDialogHandle(oneTimeNotifyDialogHandle);
       CPPUNIT_ASSERT(!oneTimeNotifyDialogHandle.isNull());
       CPPUNIT_ASSERT(dialogMgr->dialogExists(oneTimeNotifyDialogHandle));
       CPPUNIT_ASSERT(dialogMgr->countDialogs() == 2);
       long now = OsDateTime::getSecsSinceEpoch();
       UtlString dialogDump;
       dialogMgr->toString(dialogDump);
       //ASSERT_STR_EQUAL(dialogDump.isNull() ? "" : dialogDump.data(), "");
       subMgr->removeOldSubscriptions(now + 1);
       // The one time subscription should get garbage collected
       // leaving only the persistant 3600 second subscription
       CPPUNIT_ASSERT(dialogMgr->countDialogs() == 1);


       // Cleanup to prevent access of the queue after it goes out of
       // scope
       userAgent.removeMessageObserver(incomingClientMsgQueue);
       userAgent.removeMessageObserver(incomingClientMsgQueue);
       

       userAgent.shutdown(TRUE);
       
       delete subServer;
       subServer = NULL;
#    endif
   }

};

CPPUNIT_TEST_SUITE_REGISTRATION(SipSubscribeServerTest);
