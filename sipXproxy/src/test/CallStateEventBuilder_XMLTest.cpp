//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>
#include "utl/UtlString.h"
#include "os/OsTime.h"
#include "os/OsDateTime.h"
#include "CallStateEventBuilder_XML.h"

// Note: these tests will fail if PRETTYPRINT_EVENTS is defined in CallStateEventBuilder_XML.cpp

const OsDateTime testBaseTime(2004, // year
                              11,   // month (zero based)
                              15,   // day
                              11,   // hour
                              42,   // minute
                              41,   // second
                              10000 // microsecond
                              );

class CallStateEventBuilder_XMLTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(CallStateEventBuilder_XMLTest);
   CPPUNIT_TEST(testInitial);
   CPPUNIT_TEST(testRequest);
   CPPUNIT_TEST(testSetup);
   CPPUNIT_TEST(testFailure);
   CPPUNIT_TEST(testEnd);
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
                   "Expected: %s"
                   "Actual  : %s",
                   expected, actual.data()
                   );
         }
         return matches;
      }

   void testInitial()
      {
         UtlString event;

         CallStateEventBuilder_XML builder("observer.example.com");
         builder.observerEvent(0, testTime, CallStateEventBuilder::ObserverReset, "testInitial");

         CPPUNIT_ASSERT(builder.finishElement(event));
         CPPUNIT_ASSERT(expect(event,"<call_event><observer>observer.example.com</observer><obs_seq>0</obs_seq><obs_time>2004-12-15T11:42:41.010Z</obs_time><obs_msg><obs_status>101</obs_status><obs_text>testInitial</obs_text><uri>http://www.sipfoundry.org/sipX/schema/xml/cse-01-00</uri></obs_msg></call_event>\n"));

         builder.finishElement(event);
         CPPUNIT_ASSERT(event.isNull());
      }

   void testRequest()
      {
         UtlString event;

         incrementTime(1);

         CallStateEventBuilder_XML builder("observer.example.com");
         builder.observerEvent(0, testTime, CallStateEventBuilder::ObserverReset, "testRequest");

         CPPUNIT_ASSERT(builder.finishElement(event));
         CPPUNIT_ASSERT(expect(event,"<call_event><observer>observer.example.com</observer><obs_seq>0</obs_seq><obs_time>2004-12-15T11:42:41.011Z</obs_time><obs_msg><obs_status>101</obs_status><obs_text>testRequest</obs_text><uri>http://www.sipfoundry.org/sipX/schema/xml/cse-01-00</uri></obs_msg></call_event>\n"));

         incrementTime(1);

         builder.callRequestEvent(1, testTime, "Contact <sip:requestor@sip.net>", "abcd-efgh-hijkl@sip.net;rel=refer", "z9hG4bK354ef9578fadfc082aa9b53f7e7db83c373737", 2, true);
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
         CPPUNIT_ASSERT(expect(event,"<call_event><observer>observer.example.com</observer><obs_seq>1</obs_seq><obs_time>2004-12-15T11:42:41.012Z</obs_time><call_request><call><dialog><call_id>08799710-9147-486B-A28D-FFDEB031106B@10.90.10.98</call_id><from_tag>8633744</from_tag></dialog><from>&quot;Éê½­ÌÎ&quot;&lt;sip:1002@sip.net&gt;;tag=8633744</from><to>&quot;Joe Caller&quot;&lt;sip:jcaller@rhe-sipx.example.com&gt;</to></call><contact>Contact &lt;sip:requestor@sip.net&gt;</contact><via>SIP/2.0/UDP 10.1.30.248:7003</via></call_request></call_event>\n"));

         CPPUNIT_ASSERT(!builder.finishElement(event));
         CPPUNIT_ASSERT(event.isNull());
      }

   void testSetup()
      {
         UtlString event;

         incrementTime(3);

         CallStateEventBuilder_XML builder("observer.example.com");
         builder.observerEvent(0, testTime, CallStateEventBuilder::ObserverReset, "testSetup");

         CPPUNIT_ASSERT(builder.finishElement(event));
         CPPUNIT_ASSERT(expect(event,"<call_event><observer>observer.example.com</observer><obs_seq>0</obs_seq><obs_time>2004-12-15T11:42:41.013Z</obs_time><obs_msg><obs_status>101</obs_status><obs_text>testSetup</obs_text><uri>http://www.sipfoundry.org/sipX/schema/xml/cse-01-00</uri></obs_msg></call_event>\n"));

         incrementTime(1);

         builder.callSetupEvent(1, testTime, "Contact <sip:responder@sip.net>", "INT", "z9hG4bK354ef9578fadfc082aa9b53f7e7db83c373737", 1);
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
         CPPUNIT_ASSERT(expect(event,"<call_event><observer>observer.example.com</observer><obs_seq>1</obs_seq><obs_time>2004-12-15T11:42:41.014Z</obs_time><call_setup><call><dialog><call_id>08799710-9147-A28D-486B-FFDEB031106B@10.90.10.98</call_id><from_tag>3744863</from_tag><to_tag>19b8e5bK3a</to_tag></dialog><from>&quot;Éê½­ÌÎ&quot;&lt;sip:1002@sip.net&gt;;tag=3744863</from><to>&quot;Joe Caller&quot;&lt;sip:jcaller@rhe-sipx.example.com&gt;;tag=19b8e5bK3a</to></call><contact>Contact &lt;sip:responder@sip.net&gt;</contact><via>SIP/2.0/UDP 10.1.30.248:7004</via></call_setup></call_event>\n"));

         CPPUNIT_ASSERT(!builder.finishElement(event));
         CPPUNIT_ASSERT(event.isNull());
      }

   void testFailure()
      {
         UtlString event;

         incrementTime(5);

         CallStateEventBuilder_XML builder("observer.example.com");
         builder.observerEvent(0, testTime, CallStateEventBuilder::ObserverReset, "testFailure");

         CPPUNIT_ASSERT(builder.finishElement(event));
         CPPUNIT_ASSERT(expect(event,"<call_event><observer>observer.example.com</observer><obs_seq>0</obs_seq><obs_time>2004-12-15T11:42:41.015Z</obs_time><obs_msg><obs_status>101</obs_status><obs_text>testFailure</obs_text><uri>http://www.sipfoundry.org/sipX/schema/xml/cse-01-00</uri></obs_msg></call_event>\n"));

         incrementTime(1);

         builder.callFailureEvent(1, testTime, "z9hG4bK354ef9578fadfc082aa9b53f7e7db83c373737", 2, 403, "Forbidden <dummy>");
         CPPUNIT_ASSERT(!builder.finishElement(event));

         UtlString callId("9147-08799710-A28D-486B-FFDEB031106B@10.90.10.98");
         UtlString fromTag("7448633");
         UtlString toTag("b8e5bK3a19");
         UtlString fromField("\"Éê½­ÌÎ\"<sip:1002@sip.net>;tag=7448633");
         UtlString toField("\"Joe Caller\"<sip:jcaller@rhe-sipx.example.com>;tag=b8e5bK3a19");
         builder.addCallData(3,callId, fromTag, toTag, fromField, toField);
         CPPUNIT_ASSERT(!builder.finishElement(event));

         UtlString viaField("SIP/2.0/UDP 10.1.30.248:7005");
         builder.addEventVia(viaField);
         CPPUNIT_ASSERT(!builder.finishElement(event));

         builder.completeCallEvent();
         CPPUNIT_ASSERT(builder.finishElement(event));
         CPPUNIT_ASSERT(expect(event,"<call_event><observer>observer.example.com</observer><obs_seq>1</obs_seq><obs_time>2004-12-15T11:42:41.016Z</obs_time><call_failure><call><dialog><call_id>9147-08799710-A28D-486B-FFDEB031106B@10.90.10.98</call_id><from_tag>7448633</from_tag><to_tag>b8e5bK3a19</to_tag></dialog><from>&quot;Éê½­ÌÎ&quot;&lt;sip:1002@sip.net&gt;;tag=7448633</from><to>&quot;Joe Caller&quot;&lt;sip:jcaller@rhe-sipx.example.com&gt;;tag=b8e5bK3a19</to></call><response><status>403</status><reason>Forbidden &lt;dummy&gt;</reason></response><via>SIP/2.0/UDP 10.1.30.248:7005</via></call_failure></call_event>\n"));

         CPPUNIT_ASSERT(!builder.finishElement(event));
         CPPUNIT_ASSERT(event.isNull());
      }

   void testEnd()
      {
         UtlString event;

         incrementTime(7);

         CallStateEventBuilder_XML builder("observer.example.com");
         builder.observerEvent(0, testTime, CallStateEventBuilder::ObserverReset, "testEnd");

         CPPUNIT_ASSERT(builder.finishElement(event));
         CPPUNIT_ASSERT(expect(event,"<call_event><observer>observer.example.com</observer><obs_seq>0</obs_seq><obs_time>2004-12-15T11:42:41.017Z</obs_time><obs_msg><obs_status>101</obs_status><obs_text>testEnd</obs_text><uri>http://www.sipfoundry.org/sipX/schema/xml/cse-01-00</uri></obs_msg></call_event>\n"));

         incrementTime(1);

         builder.callEndEvent(1, testTime);
         CPPUNIT_ASSERT(!builder.finishElement(event));

         UtlString callId("9147-A28D-08799710-486B-FFDEB031106B@10.90.10.98");
         UtlString fromTag("3374486");
         UtlString toTag("a19b8e5bK3");
         UtlString fromField("\"Éê½­ÌÎ\"<sip:1002@sip.net>;tag=3374486");
         UtlString toField("\"Joe Caller\"<sip:jcaller@rhe-sipx.example.com>;tag=a19b8e5bK3");
         builder.addCallData(4,callId, fromTag, toTag, fromField, toField);
         CPPUNIT_ASSERT(!builder.finishElement(event));

         UtlString viaField("SIP/2.0/UDP 10.1.30.248:7006");
         builder.addEventVia(viaField);
         CPPUNIT_ASSERT(!builder.finishElement(event));

         builder.completeCallEvent();
         CPPUNIT_ASSERT(builder.finishElement(event));
         CPPUNIT_ASSERT(expect(event,"<call_event><observer>observer.example.com</observer><obs_seq>1</obs_seq><obs_time>2004-12-15T11:42:41.018Z</obs_time><call_end><call><dialog><call_id>9147-A28D-08799710-486B-FFDEB031106B@10.90.10.98</call_id><from_tag>3374486</from_tag><to_tag>a19b8e5bK3</to_tag></dialog><from>&quot;Éê½­ÌÎ&quot;&lt;sip:1002@sip.net&gt;;tag=3374486</from><to>&quot;Joe Caller&quot;&lt;sip:jcaller@rhe-sipx.example.com&gt;;tag=a19b8e5bK3</to></call><via>SIP/2.0/UDP 10.1.30.248:7006</via></call_end></call_event>\n"));

         CPPUNIT_ASSERT(!builder.finishElement(event));
         CPPUNIT_ASSERT(event.isNull());
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION(CallStateEventBuilder_XMLTest);
