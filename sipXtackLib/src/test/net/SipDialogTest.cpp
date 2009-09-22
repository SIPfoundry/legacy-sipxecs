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
#include <net/SipDialog.h>
#include <net/SipMessage.h>
#include <net/SipUserAgent.h>
#include <net/SipDialogMgr.h>
//#include <net/SipRefreshManager.h>
//#include <net/SipSubscribeClient.h>


/**
 * Unittest for SipMessage
 */
class SipDialogTest : public CppUnit::TestCase
{
      CPPUNIT_TEST_SUITE(SipDialogTest);
      CPPUNIT_TEST(createSubDialog);
      CPPUNIT_TEST(subscribeNotifyTest);
      CPPUNIT_TEST_SUITE_END();

      public:

   void createSubDialog()
   {
         const char subscribeRequestString[] =
            "SUBSCRIBE sip:sipuaconfig@sipuaconfig.example.com SIP/2.0\r\n\
From: sip:10.1.1.10;Vendor=Pingtel;Model=xpressa_strongarm_vxworks;Version=2.4.0.0009;Serial=00d01e00f4f4;Mac=00d01e00f4f4;tag=17747cec9\r\n\
To: sip:sipuaconfig@sipuaconfig.example.com\r\n\
Call-Id: config-17747cec9-00d01e004e6f@10.1.1.10\r\n\
Cseq: 7 SUBSCRIBE\r\n\
Contact: sip:10.1.1.10\r\n\
Event: sip-config\r\n\
Config_allow: http\r\n\
Config_require: x-xpressa-apps, x-xpressa-device, x-xpressa-install, x-xpressa-user\r\n\
Expires: 86400\r\n\
Date: Tue, 26 Apr 2005 03:54:16 GMT\r\n\
Max-Forwards: 20\r\n\
User-Agent: Pingtel/2.4.0 (VxWorks)\r\n\
Accept-Language: en\r\n\
Supported: sip-cc, sip-cc-01, timer, replaces\r\n\
Via: SIP/2.0/UDP 10.1.1.10;branch=z9hG4bKedff6a4031b8220192be959d966159b6\r\n\
Content-Length: 0\r\n";
         const char subscribeResponseString[] =
             "SIP/2.0 202 Accepted\r\n\
Expires: 86400\r\n\
From: sip:10.1.1.10;Vendor=Pingtel;Model=xpressa_strongarm_vxworks;Version=2.4.0.0009;Serial=00d01e004e6f;Mac=00d01e004e6f;tag=17747cec9\r\n\
To: sip:sipuaconfig@sipuaconfig.example.com;tag=1114487634asd\r\n\
Call-Id: config-17747cec9-00d01e004e6f@10.1.1.10\r\n\
Cseq: 7 SUBSCRIBE\r\n\
Via: SIP/2.0/UDP 10.1.1.10;branch=z9hG4bKedff6a4031b8220192be959d966159b6\r\n\
Date: Tue, 26 Apr 2005 03:53:55 GMT\r\n\
Contact: sip:10.1.1.10:5090\r\n\
Allow: INVITE, ACK, CANCEL, BYE, REFER, OPTIONS, SUBSCRIBE\r\n\
User-Agent: sipX/2.8.0 (Linux)\r\n\
Accept-Language: en\r\n\
Content-Length: 0\r\n\r\n";

         SipMessage subRequest(subscribeRequestString, strlen(subscribeRequestString));
         SipMessage subResponse(subscribeResponseString, strlen(subscribeResponseString));
         SipDialog subDialog(&subRequest, TRUE);

         UtlString method;
         subDialog.getInitialMethod(method);
         ASSERT_STR_EQUAL(SIP_SUBSCRIBE_METHOD, method.data());

         UtlString dialogHandle;
         subDialog.getHandle(dialogHandle);
         ASSERT_STR_EQUAL("config-17747cec9-00d01e004e6f@10.1.1.10,17747cec9,",
             dialogHandle);

         UtlString callId;
         UtlString localTag;
         UtlString remoteTag;
         SipDialog::parseHandle(dialogHandle,
                            callId,
                            localTag,
                            remoteTag);
         ASSERT_STR_EQUAL("config-17747cec9-00d01e004e6f@10.1.1.10",
             callId.data());
         ASSERT_STR_EQUAL("17747cec9", localTag.data());
         ASSERT_STR_EQUAL("", remoteTag.data());

         subDialog.getCallId(callId);
         ASSERT_STR_EQUAL("config-17747cec9-00d01e004e6f@10.1.1.10",
             callId.data());

         Url fromUri;
         UtlString fromField;
         UtlString fromTag;
         subDialog.getLocalField(fromUri);
         fromUri.toString(fromField);
         ASSERT_STR_EQUAL("<sip:10.1.1.10>;Vendor=Pingtel;Model=xpressa_strongarm_vxworks;Version=2.4.0.0009;Serial=00d01e00f4f4;Mac=00d01e00f4f4;tag=17747cec9",fromField.data());
         subDialog.getLocalTag(fromTag);
         ASSERT_STR_EQUAL("17747cec9", fromTag.data());

         Url toUri;
         UtlString toField;
         UtlString toTag;
         subDialog.getRemoteField(toUri);
         toUri.toString(toField);
         ASSERT_STR_EQUAL("sip:sipuaconfig@sipuaconfig.example.com",toField.data());
         subDialog.getRemoteTag(toTag);
         ASSERT_STR_EQUAL("", toTag.data());

         Url remoteContactUri;
         UtlString remoteContactString;
         subDialog.getRemoteContact(remoteContactUri);
         remoteContactUri.toString(remoteContactString);
         // Not set yet as we do not have a contact from the other side
         ASSERT_STR_EQUAL("sip:", remoteContactString.data());

         Url localContactUri;
         UtlString localContactString;
         subDialog.getLocalContact(localContactUri);
         localContactUri.toString(localContactString);
         ASSERT_STR_EQUAL("sip:10.1.1.10", localContactString.data());

         UtlString remoteUriString;
         subDialog.getRemoteRequestUri(remoteUriString);
         ASSERT_STR_EQUAL("sip:sipuaconfig@sipuaconfig.example.com", remoteUriString.data());

         UtlString localUriString;
         subDialog.getLocalRequestUri(localUriString);
         ASSERT_STR_EQUAL("", localUriString.data());

         CPPUNIT_ASSERT_EQUAL(7, subDialog.getLastLocalCseq());
         CPPUNIT_ASSERT_EQUAL(-1, subDialog.getLastRemoteCseq());

         CPPUNIT_ASSERT(subDialog.isEarlyDialog());

         CPPUNIT_ASSERT(subDialog.isEarlyDialogFor("config-17747cec9-00d01e004e6f@10.1.1.10",
             "17747cec9", "foo"));
         CPPUNIT_ASSERT(subDialog.isEarlyDialogFor(subscribeRequestString));
         CPPUNIT_ASSERT(subDialog.isEarlyDialogFor(subscribeResponseString));


         CPPUNIT_ASSERT(subDialog.isSameDialog("config-17747cec9-00d01e004e6f@10.1.1.10",
             "17747cec9", ""));
         CPPUNIT_ASSERT(subDialog.isSameDialog(subRequest));
         CPPUNIT_ASSERT(subDialog.isEarlyDialogFor(subResponse));
         CPPUNIT_ASSERT(!subDialog.isSameDialog(subResponse));

         // Update to change early dialog to setup dialog
         UtlString dump;
         subDialog.toString(dump);
         subDialog.updateDialogData(subResponse);
         UtlString updatedLocalTag;
         UtlString updatedRemoteTag;
         subDialog.getLocalTag(updatedLocalTag);
         CPPUNIT_ASSERT(updatedLocalTag.compareTo("17747cec9") == 0);
         subDialog.getRemoteTag(updatedRemoteTag);
         ASSERT_STR_EQUAL("1114487634asd", updatedRemoteTag.data());
         CPPUNIT_ASSERT(subDialog.isSameDialog(subResponse));
         CPPUNIT_ASSERT(subDialog.wasEarlyDialogFor("config-17747cec9-00d01e004e6f@10.1.1.10",
             "17747cec9", ""));

      };


      void subscribeNotifyTest()
      {
          // This is just a temporary test to see if all methods
          // are defined.  It is not intended to work
          //SipUserAgent userAgent = NULL;
          //SipDialogMgr dialogMgr;
          //SipRefreshManager refreshMgr(userAgent, dialogMgr);
          //SipSubscribeClient subscribeClient(userAgent, dialogMgr, refreshMgr);

          // Test messages
char subscribe[]="SUBSCRIBE sip:111@example.com SIP/2.0\r\n\
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

char subscribe401[] = "SIP/2.0 401 Unauthorized\r\n\
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

char subscribeAuth[] = "SUBSCRIBE sip:111@example.com SIP/2.0\r\n\
From: \"Dan Petrie\"<sip:111@example.com>;tag=1612c1612\r\n\
To: \"Dan Petrie\"<sip:111@example.com>\r\n\
Call-Id: e2aab34a72a0eb18300fbec445d5d665\r\n\
Cseq: 2 SUBSCRIBE\r\n\
Contact: sip:111@10.1.2.3\r\n\
Event: message-summary\r\n\
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

char subscribe202[] = "SIP/2.0 202 Accepted\r\n\
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

char notify[] = "NOTIFY sip:111@10.1.1.177 SIP/2.0\r\n\
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

        // This first set of tests assumes a server prespective
         SipDialogMgr dialogMgr;
         SipMessage subRequest(subscribe);
         SipMessage sub401Response(subscribe401);
         SipMessage subWithAuthRequest(subscribeAuth);
         SipMessage sub202Response(subscribe202);
         SipMessage notifyRequest(notify);
         CPPUNIT_ASSERT(dialogMgr.createDialog(subWithAuthRequest, FALSE));

         UtlString earlyDialogHandle;
         subRequest.getDialogHandle(earlyDialogHandle);

         UtlString dummyDialog;
         CPPUNIT_ASSERT(!dialogMgr.getEstablishedDialogHandleFor(earlyDialogHandle,
             dummyDialog));

         UtlString establishedDialogHandle;
         dialogMgr.updateDialog(sub202Response);
         CPPUNIT_ASSERT(dialogMgr.getEstablishedDialogHandleFor(earlyDialogHandle,
             establishedDialogHandle));

         CPPUNIT_ASSERT(dialogMgr.countDialogs() == 1);
         dialogMgr.updateDialog(notifyRequest);
         SipMessage nextNotifyToSend;
         dialogMgr.setNextLocalTransactionInfo(nextNotifyToSend,
                                               SIP_NOTIFY_METHOD,
                                               establishedDialogHandle);
         int cseq;
         UtlString notifyMethod;
         nextNotifyToSend.getCSeqField(&cseq, &notifyMethod);
         CPPUNIT_ASSERT(cseq == 11);
         ASSERT_STR_EQUAL(notifyMethod.data(), SIP_NOTIFY_METHOD);
         UtlString notifyTo;
         UtlString notifyFrom;
         nextNotifyToSend.getToField(&notifyTo);
         nextNotifyToSend.getFromField(&notifyFrom);
         UtlString subTo;
         UtlString subFrom;
         sub202Response.getToField(&subTo);
         sub202Response.getFromField(&subFrom);
         ASSERT_STR_EQUAL(subTo, notifyFrom);
         ASSERT_STR_EQUAL(subFrom, notifyTo);
         UtlString nextNotifyCallId;
         nextNotifyToSend.getCallIdField(&nextNotifyCallId);
         UtlString subCallId;
         subRequest.getCallIdField(&subCallId);
         ASSERT_STR_EQUAL(nextNotifyCallId, subCallId);


         dialogMgr.deleteDialog(earlyDialogHandle);
         CPPUNIT_ASSERT(dialogMgr.countDialogs() == 0);


         // The following simulate the client side of a SUBSCRIBE dialog
         CPPUNIT_ASSERT(dialogMgr.createDialog(subRequest, TRUE));
         CPPUNIT_ASSERT(dialogMgr.countDialogs() == 1);
         CPPUNIT_ASSERT(dialogMgr.updateDialog(sub401Response));
         CPPUNIT_ASSERT(dialogMgr.earlyDialogExists(earlyDialogHandle));
         CPPUNIT_ASSERT(dialogMgr.updateDialog(subWithAuthRequest));
         CPPUNIT_ASSERT(dialogMgr.updateDialog(sub202Response));
         UtlString establishedSubDialogHandle;
         CPPUNIT_ASSERT(dialogMgr.getEstablishedDialogHandleFor(earlyDialogHandle,
                                                 establishedSubDialogHandle));
         CPPUNIT_ASSERT(dialogMgr.dialogExists(establishedDialogHandle));
         SipMessage nextSubRequest;
         // Build a new SUBSCRIBE to send (e.g. a refresh)
         dialogMgr.setNextLocalTransactionInfo(nextSubRequest,
                                               SIP_SUBSCRIBE_METHOD,
                                               establishedSubDialogHandle);
         UtlString nextSubTo;
         UtlString nextSubFrom;
         nextSubRequest.getToField(&nextSubTo);
         nextSubRequest.getFromField(&nextSubFrom);
         ASSERT_STR_EQUAL(subTo, nextSubTo);
         ASSERT_STR_EQUAL(subFrom, nextSubFrom);
         int nextSubCseq;
         UtlString subMethod;
         nextSubRequest.getCSeqField(&nextSubCseq, &subMethod);
         ASSERT_STR_EQUAL(subMethod.data(), SIP_SUBSCRIBE_METHOD);
         CPPUNIT_ASSERT(nextSubCseq == 3);

         // Already exists, should not create another dialog
         CPPUNIT_ASSERT(!dialogMgr.createDialog(nextSubRequest, TRUE));
      }

};

CPPUNIT_TEST_SUITE_REGISTRATION(SipDialogTest);
