//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//
//////////////////////////////////////////////////////////////////////////////

#include <sys/time.h>
#include <unistd.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <utl/UtlSList.h>
#include <utl/UtlString.h>
#include <os/OsDefs.h>
#include <net/SipMessage.h>
#include <net/SipOutputProcessor.h>
#include <net/SipUserAgent.h>

// Class that represents an Output Processor
// This class saves the destination IP:port information of the
// first callback it receives.
class OutputCatcher : public SipOutputProcessor
{
public:
   UtlString destIpAddress;
   int       destPort;
   int       callbackCount;
   
   OutputCatcher( uint prio = 0 ) : 
      SipOutputProcessor( prio ),
      callbackCount( 0 )
   {}
   
   void handleOutputMessage( SipMessage& message,
                             const char* address,
                             int port )
   {
      if( callbackCount == 0 )
      {
         callbackCount++;
         destIpAddress = address;
         destPort      = port;
      }
   }
   
};

// Unittest for Sip Output Processor functionality
class SipXNatRouteTest : public CppUnit::TestCase
{
      CPPUNIT_TEST_SUITE(SipXNatRouteTest);
      CPPUNIT_TEST(testNatRouteWithoutRoute);
      CPPUNIT_TEST(testNatRouteWithRoute);
      CPPUNIT_TEST_SUITE_END();

public:
   
   void testNatRouteWithoutRoute()
   {
      OutputCatcher* pProcessor1 = new OutputCatcher();
      
      SipUserAgent sipUA( 7777
                         ,7777
                         ,7778 ); // proxy requests to receivingSipUA
      sipUA.start();

      const char* simpleMessage = 
          "INVITE sip:1@192.168.0.6 SIP/2.0\r\n"
          "From: <sip:200@10.1.1.144;user=phone>;tag=bbb\r\n"
          "To: <sip:3000@192.168.0.3:3000;user=phone>\r\n"
          "Call-Id: 8\r\n"
          "Cseq: 1 INVITE\r\n"
          "Content-Length: 0\r\n"
          "\r\n";
      SipMessage testMsg( simpleMessage, strlen( simpleMessage ) );

      // add output processor so that we can see the destination IP:port
      // selected by the sipXtack 
      sipUA.addSipOutputProcessor( pProcessor1 );

      // add a NAT route
      const UtlString temporaryRoute( "10.10.10.1:10491;transport=udp" );
      testMsg.setSipXNatRoute( temporaryRoute );
      sipUA.send( testMsg );

      // wait more than enough time to let the output processor catch the message
      sleep( 4 );
      CPPUNIT_ASSERT( pProcessor1->destIpAddress == "10.10.10.1" );
      CPPUNIT_ASSERT( pProcessor1->destPort == 10491 );
   }
   
   void testNatRouteWithRoute()
   {
      OutputCatcher* pProcessor1 = new OutputCatcher();
      
      SipUserAgent sipUA( 7777
                         ,7777
                         ,7778 ); // proxy requests to receivingSipUA
      
      sipUA.start();

      const char* simpleMessage = 
          "INVITE sip:1@192.168.0.6 SIP/2.0\r\n"
          "From: <sip:200@10.1.1.144;user=phone>;tag=bbb\r\n"
          "To: <sip:3000@192.168.0.3:3000;user=phone>\r\n"
          "Route:  <sip:192.168.10.10:11111;lr>"
          "Call-Id: 8\r\n"
          "Cseq: 1 INVITE\r\n"
          "Content-Length: 0\r\n"
          "\r\n";
      SipMessage testMsg( simpleMessage, strlen( simpleMessage ) );

      // add output processor so that we can see the destination IP:port
      // selected by the sipXtack 
      sipUA.addSipOutputProcessor( pProcessor1 );

      // add a NAT route
      const UtlString temporaryRoute( "10.10.10.1:10491;transport=udp" );
      testMsg.setSipXNatRoute( temporaryRoute );
      sipUA.send( testMsg );

      // wait more than enough time to let the output processor catch the message
      sleep( 4 );
      CPPUNIT_ASSERT( pProcessor1->destIpAddress == "192.168.10.10" );
      CPPUNIT_ASSERT( pProcessor1->destPort == 11111 );
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SipXNatRouteTest);


