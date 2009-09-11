//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include "os/OsDefs.h"
#include "net/Url.h"
#include "net/SipMessage.h"
#include "NatTraversalRules.h"

class NatTraversalRulesTest : public CppUnit::TestCase
{
      CPPUNIT_TEST_SUITE(NatTraversalRulesTest);
      CPPUNIT_TEST(testEnabled);
      CPPUNIT_TEST(testBehindNat);
      CPPUNIT_TEST(testGetPublicIpAddress);
      CPPUNIT_TEST(testGetPublicPort);
      CPPUNIT_TEST(testGetProxyHostPort);
      CPPUNIT_TEST(testIsXmlRpcSecured);
      CPPUNIT_TEST(testIsPartOfLocalTopology);
      CPPUNIT_TEST(testGetMaxMediaRelaySessions);
      CPPUNIT_TEST_SUITE_END();

   public:
      void testEnabled()
      {
         NatTraversalRules theRules;
         UtlString     rulesFile1(TEST_DATA_DIR "/NatTraversalAgent/nattraversalrules1.xml");
         CPPUNIT_ASSERT( theRules.loadRules( rulesFile1 ) == OS_SUCCESS );
         CPPUNIT_ASSERT( theRules.isEnabled() == true );

         UtlString     rulesFile2(TEST_DATA_DIR "/NatTraversalAgent/nattraversalrules2.xml");
         CPPUNIT_ASSERT( theRules.loadRules( rulesFile2 ) == OS_SUCCESS );
         CPPUNIT_ASSERT( theRules.isEnabled() == false );
      }

      void testBehindNat()
      {
         NatTraversalRules theRules;
         UtlString     rulesFile1(TEST_DATA_DIR "/NatTraversalAgent/nattraversalrules1.xml");
         CPPUNIT_ASSERT( theRules.loadRules( rulesFile1 ) == OS_SUCCESS );
         CPPUNIT_ASSERT( theRules.isBehindNat() == true );

         UtlString     rulesFile2(TEST_DATA_DIR "/NatTraversalAgent/nattraversalrules2.xml");
         CPPUNIT_ASSERT( theRules.loadRules( rulesFile2 ) == OS_SUCCESS );
         CPPUNIT_ASSERT( theRules.isBehindNat() == false );
      }

      void testGetPublicIpAddress()
      {
         NatTraversalRules theRules;
         UtlString     rulesFile(TEST_DATA_DIR "/NatTraversalAgent/nattraversalrules1.xml");
         CPPUNIT_ASSERT( theRules.loadRules( rulesFile ) == OS_SUCCESS );
         ASSERT_STR_EQUAL( "200.200.200.1", theRules.getPublicTransportInfo().getAddress().data() );
      }

      void testGetPublicPort()
      {
         NatTraversalRules theRules;
         UtlString     rulesFile(TEST_DATA_DIR "/NatTraversalAgent/nattraversalrules1.xml");
         CPPUNIT_ASSERT( theRules.loadRules( rulesFile ) == OS_SUCCESS );
         CPPUNIT_ASSERT( theRules.getPublicTransportInfo().getPort() == 6666 );
      }

      void testGetProxyHostPort()
      {
         NatTraversalRules theRules1;
         UtlString     rulesFile1(TEST_DATA_DIR "/NatTraversalAgent/nattraversalrules1.xml");
         CPPUNIT_ASSERT( theRules1.loadRules( rulesFile1 ) == OS_SUCCESS );
         ASSERT_STR_EQUAL( "10.10.10.1", theRules1.getProxyTransportInfo().getAddress().data() );
         ASSERT_STR_EQUAL( "udp", theRules1.getProxyTransportInfo().getTransportProtocol().data() );
         CPPUNIT_ASSERT( theRules1.getProxyTransportInfo().getPort() == 5060 );

         NatTraversalRules theRules2;
         UtlString     rulesFile2(TEST_DATA_DIR "/NatTraversalAgent/proxyhostport_var1.xml");
         CPPUNIT_ASSERT( theRules2.loadRules( rulesFile2 ) == OS_SUCCESS );
         ASSERT_STR_EQUAL( "10.10.10.2", theRules2.getProxyTransportInfo().getAddress().data() );
         ASSERT_STR_EQUAL( "udp", theRules2.getProxyTransportInfo().getTransportProtocol().data() );
         CPPUNIT_ASSERT( theRules2.getProxyTransportInfo().getPort() == 5060 );

         NatTraversalRules theRules3;
         UtlString     rulesFile3(TEST_DATA_DIR "/NatTraversalAgent/proxyhostport_var2.xml");
         CPPUNIT_ASSERT( theRules3.loadRules( rulesFile3 ) == OS_SUCCESS );
         ASSERT_STR_EQUAL( "10.10.10.3", theRules3.getProxyTransportInfo().getAddress().data() );
         ASSERT_STR_EQUAL( "tcp", theRules3.getProxyTransportInfo().getTransportProtocol().data() );
         CPPUNIT_ASSERT( theRules3.getProxyTransportInfo().getPort() == 5060 );

         NatTraversalRules theRules4;
         UtlString     rulesFile4(TEST_DATA_DIR "/NatTraversalAgent/proxyhostport_var3.xml");
         CPPUNIT_ASSERT( theRules4.loadRules( rulesFile4 ) == OS_SUCCESS );
         ASSERT_STR_EQUAL( "10.10.10.4", theRules4.getProxyTransportInfo().getAddress().data() );
         ASSERT_STR_EQUAL( "udp", theRules4.getProxyTransportInfo().getTransportProtocol().data() );
         CPPUNIT_ASSERT( theRules4.getProxyTransportInfo().getPort() == 6060 );
      }

      void testIsXmlRpcSecured()
      {
         NatTraversalRules theRules;
         // test default value
         UtlString     rulesFile3(TEST_DATA_DIR "/NatTraversalAgent/nattraversalrules3.xml");
         CPPUNIT_ASSERT( theRules.loadRules( rulesFile3 ) == OS_SUCCESS );
         CPPUNIT_ASSERT( theRules.isXmlRpcSecured() == true );

         // test secure set to true
         UtlString     rulesFile1(TEST_DATA_DIR "/NatTraversalAgent/nattraversalrules1.xml");
         CPPUNIT_ASSERT( theRules.loadRules( rulesFile1 ) == OS_SUCCESS );
         CPPUNIT_ASSERT( theRules.isXmlRpcSecured() == true );

         // test secure set to false
         UtlString     rulesFile2(TEST_DATA_DIR "/NatTraversalAgent/nattraversalrules2.xml");
         CPPUNIT_ASSERT( theRules.loadRules( rulesFile2 ) == OS_SUCCESS );
         CPPUNIT_ASSERT( theRules.isXmlRpcSecured() == false );
      }

      void testIsPartOfLocalTopology()
      {
         NatTraversalRules theRules;
         UtlString     rulesFile(TEST_DATA_DIR "/NatTraversalAgent/nattraversalrules1.xml");
         CPPUNIT_ASSERT( theRules.loadRules( rulesFile ) == OS_SUCCESS );

         // check IP subnets
         CPPUNIT_ASSERT( theRules.isPartOfLocalTopology( "9.0.0.1" ) == 0 );
         CPPUNIT_ASSERT( theRules.isPartOfLocalTopology( "10.0.0.1" ) == 1 );
         CPPUNIT_ASSERT( theRules.isPartOfLocalTopology( "11.0.0.1" ) == 0 );

         CPPUNIT_ASSERT( theRules.isPartOfLocalTopology( "172.15.0.1" ) == 0 );
         CPPUNIT_ASSERT( theRules.isPartOfLocalTopology( "172.32.0.1" ) == 0 );
         CPPUNIT_ASSERT( theRules.isPartOfLocalTopology( "172.16.0.1" ) == 1 );
         CPPUNIT_ASSERT( theRules.isPartOfLocalTopology( "172.31.0.1" ) == 1 );

         CPPUNIT_ASSERT( theRules.isPartOfLocalTopology( "192.167.0.1" ) == 0 );
         CPPUNIT_ASSERT( theRules.isPartOfLocalTopology( "192.168.0.1" ) == 1 );
         CPPUNIT_ASSERT( theRules.isPartOfLocalTopology( "192.169.0.1" ) == 0 );

         CPPUNIT_ASSERT( theRules.isPartOfLocalTopology( "10.0.0.1"   , false ) == 0 );
         CPPUNIT_ASSERT( theRules.isPartOfLocalTopology( "172.16.0.1" , false ) == 0 );
         CPPUNIT_ASSERT( theRules.isPartOfLocalTopology( "172.31.0.1" , false ) == 0 );
         CPPUNIT_ASSERT( theRules.isPartOfLocalTopology( "192.168.0.1", false ) == 0 );

         // check DNS wildcards
         CPPUNIT_ASSERT( theRules.isPartOfLocalTopology( "some.host.aaa.com"  ) == 0 );
         CPPUNIT_ASSERT( theRules.isPartOfLocalTopology( "host.aaaa.com"      ) == 1 );
         CPPUNIT_ASSERT( theRules.isPartOfLocalTopology( "some.host.aaaa.com" ) == 1 );
         CPPUNIT_ASSERT( theRules.isPartOfLocalTopology( "aaaa.com"           ) == 1 );

         CPPUNIT_ASSERT( theRules.isPartOfLocalTopology( "some.host.bbb.bbbb.com"  ) == 0 );
         CPPUNIT_ASSERT( theRules.isPartOfLocalTopology( "host.bbbb.bbbb.com"      ) == 1 );
         CPPUNIT_ASSERT( theRules.isPartOfLocalTopology( "some.host.bbbb.bbbb.com" ) == 1 );
         CPPUNIT_ASSERT( theRules.isPartOfLocalTopology( "bbbb.bbbb.com"           ) == 1 );

         CPPUNIT_ASSERT( theRules.isPartOfLocalTopology( "host.aaaa.com"          , true, false ) == 0 );
         CPPUNIT_ASSERT( theRules.isPartOfLocalTopology( "some.host.aaaa.com"     , true, false ) == 0 );
         CPPUNIT_ASSERT( theRules.isPartOfLocalTopology( "aaaa.com"               , true, false ) == 0 );
         CPPUNIT_ASSERT( theRules.isPartOfLocalTopology( "host.bbbb.bbbb.com"     , true, false ) == 0 );
         CPPUNIT_ASSERT( theRules.isPartOfLocalTopology( "some.host.bbbb.bbbb.com", true, false ) == 0 );
         CPPUNIT_ASSERT( theRules.isPartOfLocalTopology( "bbbb.bbbb.com"          , true, false ) == 0 );
      }

      void testGetMaxMediaRelaySessions()
      {
         NatTraversalRules theRules;
         // port range = 30000-31000
         UtlString     rulesFile1(TEST_DATA_DIR "/NatTraversalAgent/maxrelaysessiontest1.xml");
         CPPUNIT_ASSERT( theRules.loadRules( rulesFile1 ) == OS_SUCCESS );
         CPPUNIT_ASSERT( theRules.getMaxMediaRelaySessions() == 125 );
         CPPUNIT_ASSERT( theRules.isEnabled() == true );

         // port range = 30000-30016
         UtlString     rulesFile2(TEST_DATA_DIR "/NatTraversalAgent/maxrelaysessiontest2.xml");
         CPPUNIT_ASSERT( theRules.loadRules( rulesFile2 ) == OS_SUCCESS );
         CPPUNIT_ASSERT( theRules.getMaxMediaRelaySessions() == 2 );
         CPPUNIT_ASSERT( theRules.isEnabled() == true );

         // port range = 30000-29000 -> invalid range
         UtlString     rulesFile3(TEST_DATA_DIR "/NatTraversalAgent/maxrelaysessiontest3.xml");
         CPPUNIT_ASSERT( theRules.loadRules( rulesFile3 ) == OS_SUCCESS );
         CPPUNIT_ASSERT( theRules.isEnabled() == false );

         // port range = 30000-30004 -> not enough ports
         UtlString     rulesFile4(TEST_DATA_DIR "/NatTraversalAgent/maxrelaysessiontest4.xml");
         CPPUNIT_ASSERT( theRules.loadRules( rulesFile4 ) == OS_SUCCESS );
         CPPUNIT_ASSERT( theRules.isEnabled() == false );
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION(NatTraversalRulesTest);
