// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsDefs.h>
#include <os/OsConfigDb.h>

#include <utl/PluginHooks.h>
#include <os/OsDateTime.h>
#include <net/SipMessage.h>

#include "sipdb/LocationDB.h"
#include "sipdb/UserLocationDB.h"
#include "testlib/SipDbTestContext.h"
#include <registry/RegisterPlugin.h>
#include "SipRedirectorFallback.h"
#include "sipXecsService/SipXecsService.h"
#include "net/SipXauthIdentity.h"
#include "sipXecsService/SharedSecret.h"

class SipRedirectorFallbackTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SipRedirectorFallbackTest);
   CPPUNIT_TEST(routeByIpReceivedOnBottomViaMatchTest);
   CPPUNIT_TEST(routeByIpReceivedOnBottomViaNoMatchTest);
   CPPUNIT_TEST(routeByIpReceivedOnMiddleViaMatchTest);
   CPPUNIT_TEST(routeByIpReceivedOnMiddleViaNoMatchTest);
   CPPUNIT_TEST(routeByIpReceivedOnTopViaMatchTest);
   CPPUNIT_TEST(routeByIpReceivedOnTopViaNoMatchTest);
   CPPUNIT_TEST(routeByIpReceivedOnMultipleViasMultipleMatchesTest);
   CPPUNIT_TEST(routeByIpReceivedOnMultipleViasFirstMatchTest);
   CPPUNIT_TEST(routeByIpReceivedOnMultipleViasSecondMatchTest);
   CPPUNIT_TEST(routeByIpReceivedOnMultipleViasNoMatchTest);
   CPPUNIT_TEST(routeByIpNoReceived_SentByMatchesTest);
   CPPUNIT_TEST(routeByIpNoReceived_SentByDoesNotMatchTest);
   CPPUNIT_TEST(routeByProvisionedLocation_UserIdentityNotAsserted_LocationNotProvisionedTest);
   CPPUNIT_TEST(routeByProvisionedLocation_UserIdentityNotAsserted_LocationProvisionedTest);
   CPPUNIT_TEST(routeByProvisionedLocation_UserIdentityAssertedUnsigned_LocationNotProvisionedTest);
   CPPUNIT_TEST(routeByProvisionedLocation_UserIdentityAssertedUnsigned_LocationProvisionedTest);
   CPPUNIT_TEST(routeByProvisionedLocation_UserIdentityAssertedBadSignature_LocationNotProvisionedTest);
   CPPUNIT_TEST(routeByProvisionedLocation_UserIdentityAssertedBadSignature_LocationProvisionedTest);
   CPPUNIT_TEST(routeByProvisionedLocation_UserIdentityAsserted_LocationNotProvisionedTest);
   CPPUNIT_TEST(routeByProvisionedLocation_UserIdentityAsserted_LocationProvisionedTest);
   CPPUNIT_TEST(routeByProvisionedLocation_UserIdentityAssertedByAuthIdentity_LocationNotProvisionedTest);
   CPPUNIT_TEST(routeByProvisionedLocation_UserIdentityAssertedByAuthIdentity_LocationProvisionedTest);
   CPPUNIT_TEST(missingDatabasesHandlingTest);
   
   CPPUNIT_TEST_SUITE_END();

public:
   SipDbTestContext sipDbContext;

   SipRedirectorFallbackTest() :
      sipDbContext( TEST_DATA_DIR, TEST_WORK_DIR )
   {
      // force copy of input files into the 'work' directory
      sipDbContext.inputFile("location.xml");
      sipDbContext.inputFile("userlocation.xml");
   }
   
   void setUp()
   {
      SipXauthIdentity::setSecret("1234");   
   }

   void routeByIpReceivedOnBottomViaMatchTest()
   {
      SipRedirectorFallback fallbackRedirector( "TEST_INSTANCE" );
      
      const char* message =
         "INVITE sip:601@192.168.1.11:5060;x-sipX-pubcontact=47.135.162.145%3A29544 SIP/2.0\r\n"
         "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "Max-Forwards: 19\r\n"
         "Supported: replaces\r\n"
         "User-Agent: LG-Nortel LIP 6804 v1.2.38sp SN/00405A187376\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "Content-Length: 0\r\n"
         "Date: Thu, 12 Jun 2008 19:14:29 GMT\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-ftetrterw34t7134rfc1h4hf0t7gfgt1\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-rgwrgvrevgewrgerwvgwerg-ewrgvwergq34tg4321t1vt1a\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7;received=47.135.162.145;rport=14956\r\n"
         "\r\n";
      SipMessage testMsg( message, strlen( message ) );
      UtlString determinedLocation;
      
      CPPUNIT_ASSERT( fallbackRedirector.determineCallerLocation( testMsg, determinedLocation ) == OS_SUCCESS );
      ASSERT_STR_EQUAL( "Springfield", determinedLocation.data() );
      
   }

   void routeByIpReceivedOnBottomViaNoMatchTest()
   {
      SipRedirectorFallback fallbackRedirector( "TEST_INSTANCE" );
      
      const char* message =
         "INVITE sip:601@192.168.1.11:5060;x-sipX-pubcontact=47.135.162.145%3A29544 SIP/2.0\r\n"
         "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "Max-Forwards: 19\r\n"
         "Supported: replaces\r\n"
         "User-Agent: LG-Nortel LIP 6804 v1.2.38sp SN/00405A187376\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "Content-Length: 0\r\n"
         "Date: Thu, 12 Jun 2008 19:14:29 GMT\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-ftetrterw34t7134rfc1h4hf0t7gfgt1\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-rgwrgvrevgewrgerwvgwerg-ewrgvwergq34tg4321t1vt1a\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7;received=48.135.162.145;rport=14956\r\n"
         "\r\n";
      SipMessage testMsg( message, strlen( message ) );
      UtlString determinedLocation;
      
      CPPUNIT_ASSERT( fallbackRedirector.determineCallerLocation( testMsg, determinedLocation ) == OS_FAILED );
      CPPUNIT_ASSERT( determinedLocation.isNull() );
      
   }

   void routeByIpReceivedOnMiddleViaMatchTest()
   {
      SipRedirectorFallback fallbackRedirector( "TEST_INSTANCE" );
      
      const char* message =
         "INVITE sip:601@192.168.1.11:5060;x-sipX-pubcontact=47.135.162.145%3A29544 SIP/2.0\r\n"
         "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "Max-Forwards: 19\r\n"
         "Supported: replaces\r\n"
         "User-Agent: LG-Nortel LIP 6804 v1.2.38sp SN/00405A187376\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "Content-Length: 0\r\n"
         "Date: Thu, 12 Jun 2008 19:14:29 GMT\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-ftetrterw34t7134rfc1h4hf0t7gfgt1\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-rgwrgvrevgewrgerwvgwerg-ewrgvwergq34tg4321t1vt1a\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7;received=10.10.10.1;rport=14956\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
         "\r\n";
      SipMessage testMsg( message, strlen( message ) );
      UtlString determinedLocation;
      
      CPPUNIT_ASSERT( fallbackRedirector.determineCallerLocation( testMsg, determinedLocation ) == OS_SUCCESS );
      ASSERT_STR_EQUAL( "Montreal", determinedLocation.data() );
      
   }

   void routeByIpReceivedOnMiddleViaNoMatchTest()
   {
      SipRedirectorFallback fallbackRedirector( "TEST_INSTANCE" );
      
      const char* message =
         "INVITE sip:601@192.168.1.11:5060;x-sipX-pubcontact=47.135.162.145%3A29544 SIP/2.0\r\n"
         "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "Max-Forwards: 19\r\n"
         "Supported: replaces\r\n"
         "User-Agent: LG-Nortel LIP 6804 v1.2.38sp SN/00405A187376\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "Content-Length: 0\r\n"
         "Date: Thu, 12 Jun 2008 19:14:29 GMT\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-ftetrterw34t7134rfc1h4hf0t7gfgt1\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-rgwrgvrevgewrgerwvgwerg-ewrgvwergq34tg4321t1vt1a\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7;received=10.10.11.1;rport=14956\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
         "\r\n";
      SipMessage testMsg( message, strlen( message ) );
      UtlString determinedLocation;
      
      CPPUNIT_ASSERT( fallbackRedirector.determineCallerLocation( testMsg, determinedLocation ) == OS_FAILED );
      CPPUNIT_ASSERT( determinedLocation.isNull() );
   }

   void routeByIpReceivedOnTopViaMatchTest()
   {
      SipRedirectorFallback fallbackRedirector( "TEST_INSTANCE" );
      
      const char* message =
         "INVITE sip:601@192.168.1.11:5060;x-sipX-pubcontact=47.135.162.145%3A29544 SIP/2.0\r\n"
         "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "Max-Forwards: 19\r\n"
         "Supported: replaces\r\n"
         "User-Agent: LG-Nortel LIP 6804 v1.2.38sp SN/00405A187376\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "Content-Length: 0\r\n"
         "Date: Thu, 12 Jun 2008 19:14:29 GMT\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7;received=22.22.22.22;rport=14956\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-ftetrterw34t7134rfc1h4hf0t7gfgt1\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-rgwrgvrevgewrgerwvgwerg-ewrgvwergq34tg4321t1vt1a\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
         "\r\n";
      SipMessage testMsg( message, strlen( message ) );
      UtlString determinedLocation;
      
      CPPUNIT_ASSERT( fallbackRedirector.determineCallerLocation( testMsg, determinedLocation ) == OS_SUCCESS );
      ASSERT_STR_EQUAL( "Ottawa", determinedLocation.data() );
      
   }

   void routeByIpReceivedOnTopViaNoMatchTest()
   {
      SipRedirectorFallback fallbackRedirector( "TEST_INSTANCE" );
      
      const char* message =
         "INVITE sip:601@192.168.1.11:5060;x-sipX-pubcontact=47.135.162.145%3A29544 SIP/2.0\r\n"
         "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "Max-Forwards: 19\r\n"
         "Supported: replaces\r\n"
         "User-Agent: LG-Nortel LIP 6804 v1.2.38sp SN/00405A187376\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "Content-Length: 0\r\n"
         "Date: Thu, 12 Jun 2008 19:14:29 GMT\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7;received=22.22.22.23;rport=14956\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-ftetrterw34t7134rfc1h4hf0t7gfgt1\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-rgwrgvrevgewrgerwvgwerg-ewrgvwergq34tg4321t1vt1a\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
         "\r\n";
      SipMessage testMsg( message, strlen( message ) );
      UtlString determinedLocation;
      
      CPPUNIT_ASSERT( fallbackRedirector.determineCallerLocation( testMsg, determinedLocation ) == OS_FAILED );
      CPPUNIT_ASSERT( determinedLocation.isNull() );
   }

   void routeByIpReceivedOnMultipleViasMultipleMatchesTest()
   {
      SipRedirectorFallback fallbackRedirector( "TEST_INSTANCE" );
      
      const char* message =
         "INVITE sip:601@192.168.1.11:5060;x-sipX-pubcontact=47.135.162.145%3A29544 SIP/2.0\r\n"
         "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "Max-Forwards: 19\r\n"
         "Supported: replaces\r\n"
         "User-Agent: LG-Nortel LIP 6804 v1.2.38sp SN/00405A187376\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "Content-Length: 0\r\n"
         "Date: Thu, 12 Jun 2008 19:14:29 GMT\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7;received=22.22.22.22;rport=14956\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7;received=47.135.1.1;rport=14956\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-ftetrterw34t7134rfc1h4hf0t7gfgt1\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-rgwrgvrevgewrgerwvgwerg-ewrgvwergq34tg4321t1vt1a\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
         "\r\n";
      SipMessage testMsg( message, strlen( message ) );
      UtlString determinedLocation;
      
      CPPUNIT_ASSERT( fallbackRedirector.determineCallerLocation( testMsg, determinedLocation ) == OS_SUCCESS );
      ASSERT_STR_EQUAL( "Ottawa", determinedLocation.data() );
   }

   void routeByIpReceivedOnMultipleViasFirstMatchTest()
   {
      SipRedirectorFallback fallbackRedirector( "TEST_INSTANCE" );
      
      const char* message =
         "INVITE sip:601@192.168.1.11:5060;x-sipX-pubcontact=47.135.162.145%3A29544 SIP/2.0\r\n"
         "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "Max-Forwards: 19\r\n"
         "Supported: replaces\r\n"
         "User-Agent: LG-Nortel LIP 6804 v1.2.38sp SN/00405A187376\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "Content-Length: 0\r\n"
         "Date: Thu, 12 Jun 2008 19:14:29 GMT\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7;received=22.22.22.22;rport=14956\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7;received=48.135.1.1;rport=14956\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-ftetrterw34t7134rfc1h4hf0t7gfgt1\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-rgwrgvrevgewrgerwvgwerg-ewrgvwergq34tg4321t1vt1a\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
         "\r\n";
      SipMessage testMsg( message, strlen( message ) );
      UtlString determinedLocation;
      
      CPPUNIT_ASSERT( fallbackRedirector.determineCallerLocation( testMsg, determinedLocation ) == OS_SUCCESS );
      ASSERT_STR_EQUAL( "Ottawa", determinedLocation.data() );
      
   }

   void routeByIpReceivedOnMultipleViasSecondMatchTest()
   {
      SipRedirectorFallback fallbackRedirector( "TEST_INSTANCE" );
      
      const char* message =
         "INVITE sip:601@192.168.1.11:5060;x-sipX-pubcontact=47.135.162.145%3A29544 SIP/2.0\r\n"
         "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "Max-Forwards: 19\r\n"
         "Supported: replaces\r\n"
         "User-Agent: LG-Nortel LIP 6804 v1.2.38sp SN/00405A187376\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "Content-Length: 0\r\n"
         "Date: Thu, 12 Jun 2008 19:14:29 GMT\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7;received=48.135.1.1;rport=14956\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7;received=22.22.22.22;rport=14956\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-ftetrterw34t7134rfc1h4hf0t7gfgt1\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-rgwrgvrevgewrgerwvgwerg-ewrgvwergq34tg4321t1vt1a\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
         "\r\n";
      SipMessage testMsg( message, strlen( message ) );
      UtlString determinedLocation;
      
      CPPUNIT_ASSERT( fallbackRedirector.determineCallerLocation( testMsg, determinedLocation ) == OS_FAILED );
      CPPUNIT_ASSERT( determinedLocation.isNull() );
   }

   void routeByIpReceivedOnMultipleViasNoMatchTest()
   {
      SipRedirectorFallback fallbackRedirector( "TEST_INSTANCE" );
      
      const char* message =
         "INVITE sip:601@192.168.1.11:5060;x-sipX-pubcontact=47.135.162.145%3A29544 SIP/2.0\r\n"
         "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "Max-Forwards: 19\r\n"
         "Supported: replaces\r\n"
         "User-Agent: LG-Nortel LIP 6804 v1.2.38sp SN/00405A187376\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "Content-Length: 0\r\n"
         "Date: Thu, 12 Jun 2008 19:14:29 GMT\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7;received=48.135.1.1;rport=14956\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7;received=22.22.22.23;rport=14956\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-ftetrterw34t7134rfc1h4hf0t7gfgt1\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-rgwrgvrevgewrgerwvgwerg-ewrgvwergq34tg4321t1vt1a\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
         "\r\n";
      SipMessage testMsg( message, strlen( message ) );
      UtlString determinedLocation;
      
      CPPUNIT_ASSERT( fallbackRedirector.determineCallerLocation( testMsg, determinedLocation ) == OS_FAILED );
      CPPUNIT_ASSERT( determinedLocation.isNull() );
   }

   void routeByIpNoReceived_SentByMatchesTest()
   {
      SipRedirectorFallback fallbackRedirector( "TEST_INSTANCE" );
      
      const char* message =
         "INVITE sip:601@192.168.1.11:5060;x-sipX-pubcontact=47.135.162.145%3A29544 SIP/2.0\r\n"
         "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "Max-Forwards: 19\r\n"
         "Supported: replaces\r\n"
         "User-Agent: LG-Nortel LIP 6804 v1.2.38sp SN/00405A187376\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "Content-Length: 0\r\n"
         "Date: Thu, 12 Jun 2008 19:14:29 GMT\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-ftetrterw34t7134rfc1h4hf0t7gfgt1\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-rgwrgvrevgewrgerwvgwerg-ewrgvwergq34tg4321t1vt1a\r\n"
         "Via: SIP/2.0/UDP 47.135.1.1;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
         "\r\n";
      SipMessage testMsg( message, strlen( message ) );
      UtlString determinedLocation;
      
      CPPUNIT_ASSERT( fallbackRedirector.determineCallerLocation( testMsg, determinedLocation ) == OS_SUCCESS );
      ASSERT_STR_EQUAL( "Springfield", determinedLocation.data() );
      
   }

   void routeByIpNoReceived_SentByDoesNotMatchTest()
   {
      SipRedirectorFallback fallbackRedirector( "TEST_INSTANCE" );
      
      const char* message =
         "INVITE sip:601@192.168.1.11:5060;x-sipX-pubcontact=47.135.162.145%3A29544 SIP/2.0\r\n"
         "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "Max-Forwards: 19\r\n"
         "Supported: replaces\r\n"
         "User-Agent: LG-Nortel LIP 6804 v1.2.38sp SN/00405A187376\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "Content-Length: 0\r\n"
         "Date: Thu, 12 Jun 2008 19:14:29 GMT\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-ftetrterw34t7134rfc1h4hf0t7gfgt1\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-rgwrgvrevgewrgerwvgwerg-ewrgvwergq34tg4321t1vt1a\r\n"
         "Via: SIP/2.0/UDP 48.135.1.1;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
         "\r\n";
      SipMessage testMsg( message, strlen( message ) );
      UtlString determinedLocation;
      
      CPPUNIT_ASSERT( fallbackRedirector.determineCallerLocation( testMsg, determinedLocation ) == OS_FAILED );
      CPPUNIT_ASSERT( determinedLocation.isNull() );
   }
   
   void routeByProvisionedLocation_UserIdentityAssertedBadSignature_LocationNotProvisionedTest()
   {
      SipRedirectorFallback fallbackRedirector( "TEST_INSTANCE" );
      
      const char* message =
         "INVITE sip:601@192.168.1.11:5060;x-sipX-pubcontact=47.135.162.145%3A29544 SIP/2.0\r\n"
         "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "P-Asserted-Identity: \"R1_1 - 601\" <sip:unknown@rjolyscs2.ca.nortel.com;signature=490F529E%3AInvalid>\r\n"
         "Max-Forwards: 19\r\n"
         "Supported: replaces\r\n"
         "User-Agent: LG-Nortel LIP 6804 v1.2.38sp SN/00405A187376\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "Content-Length: 0\r\n"
         "Date: Thu, 12 Jun 2008 19:14:29 GMT\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-ftetrterw34t7134rfc1h4hf0t7gfgt1\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-rgwrgvrevgewrgerwvgwerg-ewrgvwergq34tg4321t1vt1a\r\n"
         "Via: SIP/2.0/UDP 48.135.1.1;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
         "\r\n";
      SipMessage testMsg( message, strlen( message ) );
      UtlString determinedLocation;
      
      CPPUNIT_ASSERT( fallbackRedirector.determineCallerLocation( testMsg, determinedLocation ) == OS_FAILED );
      CPPUNIT_ASSERT( determinedLocation.isNull() );

   }
   
   void routeByProvisionedLocation_UserIdentityAssertedBadSignature_LocationProvisionedTest()
   {
      SipRedirectorFallback fallbackRedirector( "TEST_INSTANCE" );
      
      const char* message =
         "INVITE sip:601@192.168.1.11:5060;x-sipX-pubcontact=47.135.162.145%3A29544 SIP/2.0\r\n"
         "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "P-Asserted-Identity: \"R1_1 - 601\" <sip:202@rjolyscs2.ca.nortel.com;signature=490F529E%3Ainvalid>\r\n"
         "Max-Forwards: 19\r\n"
         "Supported: replaces\r\n"
         "User-Agent: LG-Nortel LIP 6804 v1.2.38sp SN/00405A187376\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "Content-Length: 0\r\n"
         "Date: Thu, 12 Jun 2008 19:14:29 GMT\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-ftetrterw34t7134rfc1h4hf0t7gfgt1\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-rgwrgvrevgewrgerwvgwerg-ewrgvwergq34tg4321t1vt1a\r\n"
         "Via: SIP/2.0/UDP 48.135.1.1;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
         "\r\n";
      SipMessage testMsg( message, strlen( message ) );
      UtlString determinedLocation;
      
      CPPUNIT_ASSERT( fallbackRedirector.determineCallerLocation( testMsg, determinedLocation ) == OS_FAILED );
      CPPUNIT_ASSERT( determinedLocation.isNull() );

   }
   
   void routeByProvisionedLocation_UserIdentityNotAsserted_LocationNotProvisionedTest()
   {
      SipRedirectorFallback fallbackRedirector( "TEST_INSTANCE" );
      
      const char* message =
         "INVITE sip:601@192.168.1.11:5060;x-sipX-pubcontact=47.135.162.145%3A29544 SIP/2.0\r\n"
         "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "Max-Forwards: 19\r\n"
         "Supported: replaces\r\n"
         "User-Agent: LG-Nortel LIP 6804 v1.2.38sp SN/00405A187376\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "Content-Length: 0\r\n"
         "Date: Thu, 12 Jun 2008 19:14:29 GMT\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-ftetrterw34t7134rfc1h4hf0t7gfgt1\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-rgwrgvrevgewrgerwvgwerg-ewrgvwergq34tg4321t1vt1a\r\n"
         "Via: SIP/2.0/UDP 48.135.1.1;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
         "\r\n";
      SipMessage testMsg( message, strlen( message ) );
      UtlString determinedLocation;
      
      CPPUNIT_ASSERT( fallbackRedirector.determineCallerLocation( testMsg, determinedLocation ) == OS_FAILED );
      CPPUNIT_ASSERT( determinedLocation.isNull() );

   }
   
   void routeByProvisionedLocation_UserIdentityNotAsserted_LocationProvisionedTest()
   {
      SipRedirectorFallback fallbackRedirector( "TEST_INSTANCE" );
      
      const char* message =
         "INVITE sip:601@192.168.1.11:5060;x-sipX-pubcontact=47.135.162.145%3A29544 SIP/2.0\r\n"
         "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
         "From: caller <sip:202@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "Max-Forwards: 19\r\n"
         "Supported: replaces\r\n"
         "User-Agent: LG-Nortel LIP 6804 v1.2.38sp SN/00405A187376\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "Content-Length: 0\r\n"
         "Date: Thu, 12 Jun 2008 19:14:29 GMT\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-ftetrterw34t7134rfc1h4hf0t7gfgt1\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-rgwrgvrevgewrgerwvgwerg-ewrgvwergq34tg4321t1vt1a\r\n"
         "Via: SIP/2.0/UDP 48.135.1.1;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
         "\r\n";
      SipMessage testMsg( message, strlen( message ) );
      UtlString determinedLocation;
      
      CPPUNIT_ASSERT( fallbackRedirector.determineCallerLocation( testMsg, determinedLocation ) == OS_FAILED );
      CPPUNIT_ASSERT( determinedLocation.isNull() );
   }
   
   void routeByProvisionedLocation_UserIdentityAssertedUnsigned_LocationNotProvisionedTest()
   {
      SipRedirectorFallback fallbackRedirector( "TEST_INSTANCE" );
      
      const char* message =
         "INVITE sip:601@192.168.1.11:5060;x-sipX-pubcontact=47.135.162.145%3A29544 SIP/2.0\r\n"
         "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "P-Asserted-Identity: \"R1_1 - 601\" <sip:unknown@rjolyscs2.ca.nortel.com>\r\n"
         "Max-Forwards: 19\r\n"
         "Supported: replaces\r\n"
         "User-Agent: LG-Nortel LIP 6804 v1.2.38sp SN/00405A187376\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "Content-Length: 0\r\n"
         "Date: Thu, 12 Jun 2008 19:14:29 GMT\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-ftetrterw34t7134rfc1h4hf0t7gfgt1\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-rgwrgvrevgewrgerwvgwerg-ewrgvwergq34tg4321t1vt1a\r\n"
         "Via: SIP/2.0/UDP 48.135.1.1;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
         "\r\n";
      SipMessage testMsg( message, strlen( message ) );
      UtlString determinedLocation;
      
      CPPUNIT_ASSERT( fallbackRedirector.determineCallerLocation( testMsg, determinedLocation ) == OS_FAILED );
      CPPUNIT_ASSERT( determinedLocation.isNull() );

   }
   
   void routeByProvisionedLocation_UserIdentityAssertedUnsigned_LocationProvisionedTest()
   {
      SipRedirectorFallback fallbackRedirector( "TEST_INSTANCE" );
      
      const char* message =
         "INVITE sip:601@192.168.1.11:5060;x-sipX-pubcontact=47.135.162.145%3A29544 SIP/2.0\r\n"
         "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "P-Asserted-Identity: \"R1_1 - 601\" <sip:202@rjolyscs2.ca.nortel.com>\r\n"
         "Max-Forwards: 19\r\n"
         "Supported: replaces\r\n"
         "User-Agent: LG-Nortel LIP 6804 v1.2.38sp SN/00405A187376\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "Content-Length: 0\r\n"
         "Date: Thu, 12 Jun 2008 19:14:29 GMT\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-ftetrterw34t7134rfc1h4hf0t7gfgt1\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-rgwrgvrevgewrgerwvgwerg-ewrgvwergq34tg4321t1vt1a\r\n"
         "Via: SIP/2.0/UDP 48.135.1.1;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
         "\r\n";
      SipMessage testMsg( message, strlen( message ) );
      UtlString determinedLocation;
      
      CPPUNIT_ASSERT( fallbackRedirector.determineCallerLocation( testMsg, determinedLocation ) == OS_FAILED );
      CPPUNIT_ASSERT( determinedLocation.isNull() );

   }
 
   void routeByProvisionedLocation_UserIdentityAsserted_LocationNotProvisionedTest()
   {
      SipRedirectorFallback fallbackRedirector( "TEST_INSTANCE" );
      
      const char* message =
         "INVITE sip:601@192.168.1.11:5060;x-sipX-pubcontact=47.135.162.145%3A29544 SIP/2.0\r\n"
         "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "P-Asserted-Identity: \"R1_1 - 601\" <sip:unknown@rjolyscs2.ca.nortel.com;signature=490F529E%3A4ac6eeeb6fd53955bfffc734b2320fa6>\r\n"
         "Max-Forwards: 19\r\n"
         "Supported: replaces\r\n"
         "User-Agent: LG-Nortel LIP 6804 v1.2.38sp SN/00405A187376\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "Content-Length: 0\r\n"
         "Date: Thu, 12 Jun 2008 19:14:29 GMT\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-ftetrterw34t7134rfc1h4hf0t7gfgt1\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-rgwrgvrevgewrgerwvgwerg-ewrgvwergq34tg4321t1vt1a\r\n"
         "Via: SIP/2.0/UDP 48.135.1.1;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
         "\r\n";
      SipMessage testMsg( message, strlen( message ) );
      UtlString determinedLocation;
      
      CPPUNIT_ASSERT( fallbackRedirector.determineCallerLocation( testMsg, determinedLocation ) == OS_FAILED );
      CPPUNIT_ASSERT( determinedLocation.isNull() );

   }
   
   void routeByProvisionedLocation_UserIdentityAsserted_LocationProvisionedTest()
   {
      SipRedirectorFallback fallbackRedirector( "TEST_INSTANCE" );
      
      const char* message =
         "INVITE sip:601@192.168.1.11:5060;x-sipX-pubcontact=47.135.162.145%3A29544 SIP/2.0\r\n"
         "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "P-Asserted-Identity: \"R1_1 - 601\" <sip:203@rjolyscs2.ca.nortel.com;signature=490F529E%3Ad32c8599d7c37c8db48f818db97a2583>\r\n"
         "Max-Forwards: 19\r\n"
         "Supported: replaces\r\n"
         "User-Agent: LG-Nortel LIP 6804 v1.2.38sp SN/00405A187376\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "Content-Length: 0\r\n"
         "Date: Thu, 12 Jun 2008 19:14:29 GMT\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-ftetrterw34t7134rfc1h4hf0t7gfgt1\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-rgwrgvrevgewrgerwvgwerg-ewrgvwergq34tg4321t1vt1a\r\n"
         "Via: SIP/2.0/UDP 48.135.1.1;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
         "\r\n";
      SipMessage testMsg( message, strlen( message ) );
      UtlString determinedLocation;
      
      CPPUNIT_ASSERT( fallbackRedirector.determineCallerLocation( testMsg, determinedLocation ) == OS_SUCCESS );
      ASSERT_STR_EQUAL( "Venise-Sur-Mer", determinedLocation.data() );

   }   
   
   void routeByProvisionedLocation_UserIdentityAssertedByAuthIdentity_LocationNotProvisionedTest()
   {
      SipRedirectorFallback fallbackRedirector( "TEST_INSTANCE" );
      
      const char* message =
         "INVITE sip:601@192.168.1.11:5060;x-sipX-pubcontact=47.135.162.145%3A29544 SIP/2.0\r\n"
         "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "X-sipX-Authidentity: \"R1_1 - 601\" <sip:unknown@rjolyscs2.ca.nortel.com;signature=490F529E%3A4ac6eeeb6fd53955bfffc734b2320fa6>\r\n"
         "Max-Forwards: 19\r\n"
         "Supported: replaces\r\n"
         "User-Agent: LG-Nortel LIP 6804 v1.2.38sp SN/00405A187376\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "Content-Length: 0\r\n"
         "Date: Thu, 12 Jun 2008 19:14:29 GMT\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-ftetrterw34t7134rfc1h4hf0t7gfgt1\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-rgwrgvrevgewrgerwvgwerg-ewrgvwergq34tg4321t1vt1a\r\n"
         "Via: SIP/2.0/UDP 48.135.1.1;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
         "\r\n";
      SipMessage testMsg( message, strlen( message ) );
      UtlString determinedLocation;
      
      CPPUNIT_ASSERT( fallbackRedirector.determineCallerLocation( testMsg, determinedLocation ) == OS_FAILED );
      CPPUNIT_ASSERT( determinedLocation.isNull() );

   }
   
   void routeByProvisionedLocation_UserIdentityAssertedByAuthIdentity_LocationProvisionedTest()
   {
      SipRedirectorFallback fallbackRedirector( "TEST_INSTANCE" );
      
      const char* message =
         "INVITE sip:601@192.168.1.11:5060;x-sipX-pubcontact=47.135.162.145%3A29544 SIP/2.0\r\n"
         "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "X-sipX-Authidentity: \"R1_1 - 601\" <sip:203@rjolyscs2.ca.nortel.com;signature=490F529E%3Ad32c8599d7c37c8db48f818db97a2583>\r\n"
         "Max-Forwards: 19\r\n"
         "Supported: replaces\r\n"
         "User-Agent: LG-Nortel LIP 6804 v1.2.38sp SN/00405A187376\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "Content-Length: 0\r\n"
         "Date: Thu, 12 Jun 2008 19:14:29 GMT\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-ftetrterw34t7134rfc1h4hf0t7gfgt1\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-rgwrgvrevgewrgerwvgwerg-ewrgvwergq34tg4321t1vt1a\r\n"
         "Via: SIP/2.0/UDP 48.135.1.1;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
         "\r\n";
      SipMessage testMsg( message, strlen( message ) );
      UtlString determinedLocation;
      
      CPPUNIT_ASSERT( fallbackRedirector.determineCallerLocation( testMsg, determinedLocation ) == OS_SUCCESS );
      ASSERT_STR_EQUAL( "Venise-Sur-Mer", determinedLocation.data() );

   }   
   
   void missingDatabasesHandlingTest()
   {
      setenv( SipXecsService::DatabaseDirType, "path_to_nowhere", 1 );

      SipRedirectorFallback fallbackRedirector( "TEST_INSTANCE" );
      
      const char* message =
         "INVITE sip:601@192.168.1.11:5060;x-sipX-pubcontact=47.135.162.145%3A29544 SIP/2.0\r\n"
         "Record-Route: <sip:192.168.0.2:5060;lr>\r\n"
         "From: caller <sip:602@rjolyscs2.ca.nortel.com>;tag=94bc25b8-c0a80165-13c4-3e635-37aa1989-3e635\r\n"
         "To: <sip:601@rjolyscs2.ca.nortel.com>\r\n"
         "Call-Id: 94bb2520-c0a80165-13c4-3e635-3ccd2971-3e635@rjolyscs2.ca.nortel.com\r\n"
         "Cseq: 1 INVITE\r\n"
         "P-Asserted-Identity: \"R1_1 - 601\" <sip:203@rjolyscs2.ca.nortel.com;signature=490F529E%3Ad32c8599d7c37c8db48f818db97a2583>\r\n"
         "Max-Forwards: 19\r\n"
         "Supported: replaces\r\n"
         "User-Agent: LG-Nortel LIP 6804 v1.2.38sp SN/00405A187376\r\n"
         "Contact: <sip:602@192.168.1.101:5060;x-sipX-pubcontact=47.135.162.145%3A14956>\r\n"
         "Content-Length: 0\r\n"
         "Date: Thu, 12 Jun 2008 19:14:29 GMT\r\n"
         "Via: SIP/2.0/UDP 192.168.1.101:5060;branch=z9hG4bK-3e635-f3b41fc-310ddca7\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-ftetrterw34t7134rfc1h4hf0t7gfgt1\r\n"
         "Via: SIP/2.0/UDP 192.168.0.2;branch=z9hG4bK-rgwrgvrevgewrgerwvgwerg-ewrgvwergq34tg4321t1vt1a\r\n"
         "Via: SIP/2.0/UDP 48.135.1.1;branch=z9hG4bK-sipXecs-01a17f9662b96b1d454a71775bbda1ec6bf6~9d6590763d78764b93a42e94dbd6b75a\r\n"
         "\r\n";
      SipMessage testMsg( message, strlen( message ) );
      UtlString determinedLocation;
      
      CPPUNIT_ASSERT( fallbackRedirector.determineCallerLocation( testMsg, determinedLocation ) == OS_FAILED );

   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SipRedirectorFallbackTest);
   
