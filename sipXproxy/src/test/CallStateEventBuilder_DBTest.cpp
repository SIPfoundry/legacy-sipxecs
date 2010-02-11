//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include "os/OsTime.h"
#include "os/OsDateTime.h"
#include "CallStateEventBuilder_DB.h"

const OsDateTime testBaseTime(2004, // year
                              11,   // month (zero based)
                              15,   // day
                              11,   // hour
                              42,   // minute
                              41,   // second
                              10000 // microsecond
                              );

class CallStateEventBuilder_DBTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(CallStateEventBuilder_DBTest);
   CPPUNIT_TEST(testInitial);
   CPPUNIT_TEST(testRequest);
   CPPUNIT_TEST(testSetup);
   CPPUNIT_TEST(testFailure);
   CPPUNIT_TEST(testRefer);
   CPPUNIT_TEST(testEnd);
   CPPUNIT_TEST(testSingleQuotes);
   CPPUNIT_TEST_SUITE_END();


public:
   OsTime testTime;

   void setUp()
      {
         testBaseTime.cvtToTimeSinceEpoch(testTime);
      }

   void incrementTime(int milliseconds)
      {
         OsTime timeStep(0,1000*milliseconds);
         testTime += timeStep;
      }

   bool expect(const UtlString& actual, const char* expected)
      {
         bool matches = 0==actual.compareTo(expected);

         if (!matches)
         {
            printf("\n"
                   "Expected: %s\n"
                   "Actual  : %s\n",
                   expected, actual.data()
                   );
         }
         return matches;
      }

   void testInitial()
      {
         UtlString event;

         CallStateEventBuilder_DB builder("observer.example.com");
         builder.observerEvent(0, testTime, CallStateEventBuilder::ObserverReset, "testInitial");

         CPPUNIT_ASSERT(builder.finishElement(event));
         CPPUNIT_ASSERT(expect(event,"INSERT INTO observer_state_events VALUES (DEFAULT,'observer.example.com',0,timestamp '2004-12-15 11:42:41.010',101,'testInitial');"));

         builder.finishElement(event);
         CPPUNIT_ASSERT(event.isNull());
      }

   void testRequest()
      {
         UtlString event;

         incrementTime(1);

         CallStateEventBuilder_DB builder("observer.example.com");
         builder.observerEvent(0, testTime, CallStateEventBuilder::ObserverReset, "testRequest");

         CPPUNIT_ASSERT(builder.finishElement(event));
         CPPUNIT_ASSERT(expect(event,"INSERT INTO observer_state_events VALUES (DEFAULT,'observer.example.com',0,timestamp '2004-12-15 11:42:41.011',101,'testRequest');"));

         incrementTime(1);

         builder.callRequestEvent(1, testTime, "Contact <sip:requestor@sip.net>", "abcdef-ghijkl-mnopqrs@sip.net;rel=xfer","z9hG4bK354ef9578fadfc082aa9b53f7e7db83c373737", 2, false);
         CPPUNIT_ASSERT(!builder.finishElement(event));

         UtlString callId("08799710-9147-486B-A28D-FFDEB031106B@10.90.10.98");
         UtlString fromTag("8633744");
         UtlString toTag;
         UtlString fromField("\"Éê½­ÌÎ\"<sip:1002@sip.net>;tag=8633744");
         UtlString toField("\"Joe Caller\"<sip:jcaller@rhe-sipx.example.com>");
         builder.addCallData(1,callId, fromTag, toTag, fromField, toField);
         CPPUNIT_ASSERT(!builder.finishElement(event));

         UtlString viaField("SIP/2.0/UDP 10.1.30.248:7003");
         builder.addEventVia(viaField);
         CPPUNIT_ASSERT(!builder.finishElement(event));

         builder.completeCallEvent();
         CPPUNIT_ASSERT(builder.finishElement(event));
         CPPUNIT_ASSERT(expect(event,"INSERT INTO call_state_events VALUES (DEFAULT,'observer.example.com',1,timestamp '2004-12-15 11:42:41.012','R',1,'08799710-9147-486B-A28D-FFDEB031106B@10.90.10.98','8633744','','\"Éê½­ÌÎ\"<sip:1002@sip.net>;tag=8633744','\"Joe Caller\"<sip:jcaller@rhe-sipx.example.com>','Contact <sip:requestor@sip.net>','','',0,'','','abcdef-ghijkl-mnopqrs@sip.net;rel=xfer','f','','z9hG4bK354ef9578fadfc082aa9b53f7e7db83c373737',2);"));

         CPPUNIT_ASSERT(!builder.finishElement(event));
         CPPUNIT_ASSERT(event.isNull());
      }

   void testSetup()
      {
         UtlString event;

         incrementTime(3);

         CallStateEventBuilder_DB builder("observer.example.com");
         builder.observerEvent(0, testTime, CallStateEventBuilder::ObserverReset, "testSetup");

         CPPUNIT_ASSERT(builder.finishElement(event));
         CPPUNIT_ASSERT(expect(event,"INSERT INTO observer_state_events VALUES (DEFAULT,'observer.example.com',0,timestamp '2004-12-15 11:42:41.013',101,'testSetup');"));

         incrementTime(1);

         builder.callSetupEvent(1, testTime, "Contact <sip:responder@sip.net>","callTag=LOCAL", "z9hG4bK354ef9578fadfc082aa9b53f7e7db83c373737", 1);
         CPPUNIT_ASSERT(!builder.finishElement(event));

         UtlString callId("08799710-9147-A28D-486B-FFDEB031106B@10.90.10.98");
         UtlString fromTag("3744863");
         UtlString toTag("19b8e5bK3a");
         UtlString fromField("\"Éê½­ÌÎ\"<sip:1002@sip.net>;tag=3744863");
         UtlString toField("\"Joe Caller\"<sip:jcaller@rhe-sipx.example.com>;tag=19b8e5bK3a");
         builder.addCallData(2,callId, fromTag, toTag, fromField, toField);
         CPPUNIT_ASSERT(!builder.finishElement(event));

         UtlString viaField("SIP/2.0/UDP 10.1.30.248:7004");
         builder.addEventVia(viaField);
         CPPUNIT_ASSERT(!builder.finishElement(event));

         builder.completeCallEvent();
         CPPUNIT_ASSERT(builder.finishElement(event));
         CPPUNIT_ASSERT(expect(event,"INSERT INTO call_state_events VALUES (DEFAULT,'observer.example.com',1,timestamp '2004-12-15 11:42:41.014','S',2,'08799710-9147-A28D-486B-FFDEB031106B@10.90.10.98','3744863','19b8e5bK3a','\"Éê½­ÌÎ\"<sip:1002@sip.net>;tag=3744863','\"Joe Caller\"<sip:jcaller@rhe-sipx.example.com>;tag=19b8e5bK3a','Contact <sip:responder@sip.net>','','',0,'','','',null,'callTag=LOCAL','z9hG4bK354ef9578fadfc082aa9b53f7e7db83c373737',1);"));

         CPPUNIT_ASSERT(!builder.finishElement(event));
         CPPUNIT_ASSERT(event.isNull());
      }

   void testFailure()
      {
         UtlString event;

         incrementTime(5);

         CallStateEventBuilder_DB builder("observer.example.com");
         builder.observerEvent(0, testTime, CallStateEventBuilder::ObserverReset, "testFailure");

         CPPUNIT_ASSERT(builder.finishElement(event));
         CPPUNIT_ASSERT(expect(event,"INSERT INTO observer_state_events VALUES (DEFAULT,'observer.example.com',0,timestamp '2004-12-15 11:42:41.015',101,'testFailure');"));

         incrementTime(1);

         builder.callFailureEvent(1, testTime, "z9hG4bK354ef9578fadfc082aa9b53f7e7db83c373737", 2, 403, "Forbidden <dummy>");
         CPPUNIT_ASSERT(!builder.finishElement(event));

         UtlString callId("9147-08799710-A28D-486B-FFDEB031106B@10.90.10.98");
         UtlString fromTag("7448633");
         UtlString toTag("b8e5bK3a19");
         UtlString fromField("\"Éê½­ÌÎ\"<sip:1002@sip.net>;tag=7448633");
         UtlString toField("\"Joe Caller\"<sip:jcaller@rhe-sipx.example.com>;tag=b8e5bK3a19");
         builder.addCallData(3, callId, fromTag, toTag, fromField, toField);
         CPPUNIT_ASSERT(!builder.finishElement(event));

         UtlString viaField("SIP/2.0/UDP 10.1.30.248:7005");
         builder.addEventVia(viaField);
         CPPUNIT_ASSERT(!builder.finishElement(event));

         builder.completeCallEvent();
         CPPUNIT_ASSERT(builder.finishElement(event));
         CPPUNIT_ASSERT(expect(event,"INSERT INTO call_state_events VALUES (DEFAULT,'observer.example.com',1,timestamp '2004-12-15 11:42:41.016','F',3,'9147-08799710-A28D-486B-FFDEB031106B@10.90.10.98','7448633','b8e5bK3a19','\"Éê½­ÌÎ\"<sip:1002@sip.net>;tag=7448633','\"Joe Caller\"<sip:jcaller@rhe-sipx.example.com>;tag=b8e5bK3a19','','','',403,'Forbidden <dummy>','','',null,'','z9hG4bK354ef9578fadfc082aa9b53f7e7db83c373737',2);"));

         CPPUNIT_ASSERT(!builder.finishElement(event));
         CPPUNIT_ASSERT(event.isNull());
      }

   void testRefer()
      {
         UtlString event;

         incrementTime(5);

         CallStateEventBuilder_DB builder("observer.example.com");
         builder.observerEvent(0, testTime, CallStateEventBuilder::ObserverReset, "testFailure");

         CPPUNIT_ASSERT(builder.finishElement(event));
         CPPUNIT_ASSERT(expect(event,"INSERT INTO observer_state_events VALUES (DEFAULT,'observer.example.com',0,timestamp '2004-12-15 11:42:41.015',101,'testFailure');"));

         incrementTime(1);

         UtlString referTo("<sip:200@example.com>");
         UtlString referredBy("\'Joe Caller\'<sip:jcaller@rhe-sipx.example.com>;tag=b8e5bK3a19");
         UtlString requestUri("<sip:200@example.com>");

         builder.callTransferEvent(1, testTime, "Contact <sip:requestor@sip.net>", referTo, referredBy, requestUri);
         CPPUNIT_ASSERT(!builder.finishElement(event));

         UtlString callId("9147-08799710-A28D-486B-FFDEB031106B@10.90.10.98");
         UtlString fromTag("7448633");
         UtlString toTag("b8e5bK3a19");
         UtlString fromField("\"Éê½­ÌÎ\"<sip:1002@sip.net>;tag=7448633");
         UtlString toField("\"Joe Caller\"<sip:jcaller@rhe-sipx.example.com>;tag=b8e5bK3a19");
         builder.addCallData(4, callId, fromTag, toTag, fromField, toField);
         CPPUNIT_ASSERT(!builder.finishElement(event));

         UtlString viaField("SIP/2.0/UDP 10.1.30.248:7005");
         builder.addEventVia(viaField);
         CPPUNIT_ASSERT(!builder.finishElement(event));

         builder.completeCallEvent();
         CPPUNIT_ASSERT(builder.finishElement(event));
         CPPUNIT_ASSERT(expect(event,"INSERT INTO call_state_events VALUES (DEFAULT,'observer.example.com',1,timestamp '2004-12-15 11:42:41.016','T',4,'9147-08799710-A28D-486B-FFDEB031106B@10.90.10.98','7448633','b8e5bK3a19','\"Éê½­ÌÎ\"<sip:1002@sip.net>;tag=7448633','\"Joe Caller\"<sip:jcaller@rhe-sipx.example.com>;tag=b8e5bK3a19','Contact <sip:requestor@sip.net>','<sip:200@example.com>','\\'Joe Caller\\'<sip:jcaller@rhe-sipx.example.com>;tag=b8e5bK3a19',0,'','<sip:200@example.com>','',null,'','',0);"));

         CPPUNIT_ASSERT(!builder.finishElement(event));
         CPPUNIT_ASSERT(event.isNull());
      }

   void testSingleQuotes()
      {
         UtlString event;

         incrementTime(5);

         CallStateEventBuilder_DB builder("observer.example.com");
         builder.observerEvent(0, testTime, CallStateEventBuilder::ObserverReset, "testFailure");

         CPPUNIT_ASSERT(builder.finishElement(event));
         CPPUNIT_ASSERT(expect(event,"INSERT INTO observer_state_events VALUES (DEFAULT,'observer.example.com',0,timestamp '2004-12-15 11:42:41.015',101,'testFailure');"));

         incrementTime(1);

         UtlString referTo("<sip:200@example.com>");
         UtlString referredBy("\'Joe Caller\'<sip:jcaller@rhe-sipx.example.com>;tag=b8e5bK3a19");
         UtlString requestUri("<sip:200@example.com>");

         builder.callTransferEvent(1, testTime, "Contact <sip:requestor@sip.net>", referTo, referredBy, requestUri);
         CPPUNIT_ASSERT(!builder.finishElement(event));

         UtlString callId("9147-08799710-A28D-486B-FFDEB031106B@10.90.10.98");
         UtlString fromTag("7448633");
         UtlString toTag("b8e5bK3a19");
         UtlString fromField("\'Éê½­ÌÎ\'<sip:1002@sip.net>;tag=7448633");
         UtlString toField("\'Joe Caller\'<sip:jcaller@rhe-sipx.example.com>;tag=b8e5bK3a19");
         builder.addCallData(5, callId, fromTag, toTag, fromField, toField);
         CPPUNIT_ASSERT(!builder.finishElement(event));

         UtlString viaField("SIP/2.0/UDP 10.1.30.248:7005");
         builder.addEventVia(viaField);
         CPPUNIT_ASSERT(!builder.finishElement(event));

         builder.completeCallEvent();
         CPPUNIT_ASSERT(builder.finishElement(event));
         CPPUNIT_ASSERT(expect(event,"INSERT INTO call_state_events VALUES (DEFAULT,'observer.example.com',1,timestamp '2004-12-15 11:42:41.016','T',5,'9147-08799710-A28D-486B-FFDEB031106B@10.90.10.98','7448633','b8e5bK3a19','\\'Éê½­ÌÎ\\'<sip:1002@sip.net>;tag=7448633','\\'Joe Caller\\'<sip:jcaller@rhe-sipx.example.com>;tag=b8e5bK3a19','Contact <sip:requestor@sip.net>','<sip:200@example.com>','\\'Joe Caller\\'<sip:jcaller@rhe-sipx.example.com>;tag=b8e5bK3a19',0,'','<sip:200@example.com>','',null,'','',0);"));

         CPPUNIT_ASSERT(!builder.finishElement(event));
         CPPUNIT_ASSERT(event.isNull());
      }

   void testEnd()
      {
         UtlString event;

         incrementTime(7);

         CallStateEventBuilder_DB builder("observer.example.com");
         builder.observerEvent(0, testTime, CallStateEventBuilder::ObserverReset, "testEnd");

         CPPUNIT_ASSERT(builder.finishElement(event));
         CPPUNIT_ASSERT(expect(event,"INSERT INTO observer_state_events VALUES (DEFAULT,'observer.example.com',0,timestamp '2004-12-15 11:42:41.017',101,'testEnd');"));

         incrementTime(1);

         builder.callEndEvent(1, testTime);
         CPPUNIT_ASSERT(!builder.finishElement(event));

         UtlString callId("9147-A28D-08799710-486B-FFDEB031106B@10.90.10.98");
         UtlString fromTag("3374486");
         UtlString toTag("a19b8e5bK3");
         UtlString fromField("\"Éê½­ÌÎ\"<sip:1002@sip.net>;tag=3374486");
         UtlString toField("\"Joe Caller\"<sip:jcaller@rhe-sipx.example.com>;tag=a19b8e5bK3");
         builder.addCallData(6,callId, fromTag, toTag, fromField, toField);
         CPPUNIT_ASSERT(!builder.finishElement(event));

         UtlString viaField("SIP/2.0/UDP 10.1.30.248:7006");
         builder.addEventVia(viaField);
         CPPUNIT_ASSERT(!builder.finishElement(event));

         builder.completeCallEvent();
         CPPUNIT_ASSERT(builder.finishElement(event));
         CPPUNIT_ASSERT(expect(event,"INSERT INTO call_state_events VALUES (DEFAULT,'observer.example.com',1,timestamp '2004-12-15 11:42:41.018','E',6,'9147-A28D-08799710-486B-FFDEB031106B@10.90.10.98','3374486','a19b8e5bK3','\"Éê½­ÌÎ\"<sip:1002@sip.net>;tag=3374486','\"Joe Caller\"<sip:jcaller@rhe-sipx.example.com>;tag=a19b8e5bK3','','','',0,'','','',null,'','',0);"));

         CPPUNIT_ASSERT(!builder.finishElement(event));
         CPPUNIT_ASSERT(event.isNull());
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CallStateEventBuilder_DBTest);
