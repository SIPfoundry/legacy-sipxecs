//
// Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/TestCase.h"
#include "sipxunit/TestUtilities.h"
#include "utl/PluginHooks.h"
#include "os/OsConfigDb.h"

#include "net/SipMessage.h"

#include "CallDestination.h"

class CallDestinationTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(CallDestinationTest);

   CPPUNIT_TEST(testConstructor);

   CPPUNIT_TEST(normalInviteWithCallTag);
   
   CPPUNIT_TEST(normalInviteNoCallTag);

   CPPUNIT_TEST(normalRefer);
   
   CPPUNIT_TEST_SUITE_END();

public:

   static CallDestination* calldest;
   
   void setUp()
      {
      }

   void tearDown()
      {
      }

   void testConstructor()
      {
         /*
          * This test exists to initialize the singleton plugin.
          * Doing it as a static ran into ordering problems.
          */
         CPPUNIT_ASSERT((calldest = dynamic_cast<CallDestination*>(getAuthPlugin("calldestination"))));
      }

   // Test that an INVITE with call tags moves the tags to the Record-Route
   void normalInviteWithCallTag()
      {
         UtlString identity; // no authenticated identity
         Url requestUri("sip:someone@somewhere");

         const char* message =
            "INVITE sip:someone@somewhere;sipXecs-CallDest=AL%2CSTS SIP/2.0\r\n"
            "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:someone@somewhere\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message));


         const char* modmessage =
            "INVITE sip:someone@somewhere SIP/2.0\r\n"
            "Record-Route: <sip:192.168.0.2:5060;lr;sipXecs-CallDest=AL%2CSTS>\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:someone@somewhere\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testModMsg(modmessage, strlen(modmessage));

         UtlSList noRemovedRoutes;
         UtlString myRouteName("myhost.example.com");
         RouteState routeState( testMsg, noRemovedRoutes, myRouteName );

         const char unmodifiedRejectReason[] = "unmodified";
         UtlString rejectReason(unmodifiedRejectReason);
         
         UtlString method("INVITE");
         bool bSpiralingRequest = false;
         AuthPlugin::AuthResult priorResult = AuthPlugin::CONTINUE;
         
         CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                        == calldest->authorizeAndModify(identity,
                                                       requestUri,
                                                       routeState,
                                                       method,
                                                       priorResult,
                                                       testMsg,
                                                       bSpiralingRequest,
                                                       rejectReason
                                                       ));
         // verify that the call tags were moved to the Record-Route.  
         UtlString forwardedMsg;
         ssize_t length;
         testMsg.getBytes(&forwardedMsg,&length);
         ASSERT_STR_EQUAL(modmessage, forwardedMsg.data());
      }

   // Test that an INVITE with no call tags leaves the message untouched.
   void normalInviteNoCallTag()
      {
         UtlString identity; // no authenticated identity
         Url requestUri("sip:someone@somewhere");

         const char* message =
            "INVITE sip:someone@somewhere SIP/2.0\r\n"
            "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:someone@somewhere\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 1 INVITE\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message));


         UtlSList noRemovedRoutes;
         UtlString myRouteName("myhost.example.com");
         RouteState routeState( testMsg, noRemovedRoutes, myRouteName );

         const char unmodifiedRejectReason[] = "unmodified";
         UtlString rejectReason(unmodifiedRejectReason);
         
         UtlString method("INVITE");
         bool bSpiralingRequest = false;
         AuthPlugin::AuthResult priorResult = AuthPlugin::CONTINUE;
         
         CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                        == calldest->authorizeAndModify(identity,
                                                       requestUri,
                                                       routeState,
                                                       method,
                                                       priorResult,
                                                       testMsg,
                                                       bSpiralingRequest,
                                                       rejectReason
                                                       ));
         // verify that the message remains untouched.  
         UtlString forwardedMsg;
         ssize_t length;
         testMsg.getBytes(&forwardedMsg,&length);
         ASSERT_STR_EQUAL(message, forwardedMsg.data());
      }

   // Test that a request method (i.e. REFER) is not affected.
   void normalRefer()
      {
         UtlString identity; // no authenticated identity
         Url requestUri("sip:someone@somewhere");

         const char* message =
            "REFER sip:someone@somewhere SIP/2.0\r\n"
            "Refer-To: <sip:else@where;method=INFO>\r\n"
            "Via: SIP/2.0/TCP 10.1.1.3:33855\r\n"
            "To: sip:someone@somewhere;tag=totag\r\n"
            "From: Caller <sip:caller@example.org>; tag=30543f3483e1cb11ecb40866edd3295b\r\n"
            "Call-Id: f88dfabce84b6a2787ef024a7dbe8749\r\n"
            "Cseq: 5 INFO\r\n"
            "Max-Forwards: 20\r\n"
            "Contact: caller@127.0.0.1\r\n"
            "Content-Length: 0\r\n"
            "\r\n";
         SipMessage testMsg(message, strlen(message));

         UtlSList noRemovedRoutes;
         UtlString myRouteName("myhost.example.com");
         RouteState routeState( testMsg, noRemovedRoutes, myRouteName );

         const char unmodifiedRejectReason[] = "unmodified";
         UtlString rejectReason(unmodifiedRejectReason);
         
         UtlString method("REFER");
         bool bSpiralingRequest = false;
         AuthPlugin::AuthResult priorResult = AuthPlugin::CONTINUE;
         
         CPPUNIT_ASSERT(AuthPlugin::CONTINUE
                        == calldest->authorizeAndModify(identity,
                                                       requestUri,
                                                       routeState,
                                                       method,
                                                       priorResult,
                                                       testMsg,
                                                       bSpiralingRequest,
                                                       rejectReason
                                                       ));

         UtlString forwardedMsg;
         ssize_t length;
         testMsg.getBytes(&forwardedMsg,&length);
         ASSERT_STR_EQUAL(message, forwardedMsg.data());
      }
   
private:
};

CPPUNIT_TEST_SUITE_REGISTRATION(CallDestinationTest);

CallDestination* CallDestinationTest::calldest;
