//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

// APPLICATION INCLUDES
#include "os/OsConfigDb.h"
#include "testlib/RegistrationDbTestContext.h"
#include "utl/UtlSListIterator.h"
#include "registry/SipRegistrar.h"
#include "RegistrarPeer.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

class RegistrarPeerTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(RegistrarPeerTest);
   CPPUNIT_TEST(testConfigureNone);
   CPPUNIT_TEST(testConfigureOne);
   CPPUNIT_TEST_SUITE_END();


public:
   void setUp()
      {
         RegistrationDbTestContext testDbContext(TEST_DATA_DIR "/regdbdata",
                                                 TEST_WORK_DIR "/regdbdata"
                                                 );
         testDbContext.inputFile("updatesToPull.xml");
      }

   void testConfigureNone()
      {
         OsConfigDb configuration;

         SipRegistrar registrar(&configuration);

         CPPUNIT_ASSERT(registrar.primaryName().isNull());

         UtlSListIterator* peerIterator = registrar.getPeers();
         CPPUNIT_ASSERT(NULL == peerIterator);

         RegistrarPeer* badPeer;
         badPeer = registrar.getPeer("notapeer.example.com");
         CPPUNIT_ASSERT(NULL == badPeer);

         delete peerIterator;
      }


   void testConfigureOne()
      {
         OsConfigDb configuration;

         configuration.set("SIP_REGISTRAR_XMLRPC_PORT", "8000");
         configuration.set("SIP_REGISTRAR_NAME", "myself.example.com");
         configuration.set("SIP_REGISTRAR_SYNC_WITH", "other1.example.com, myself.example.com");

         SipRegistrar registrar(&configuration);

         ASSERT_STR_EQUAL("myself.example.com", registrar.primaryName());

         UtlSListIterator* peerIterator = registrar.getPeers();

         RegistrarPeer* peer;
         peer = dynamic_cast<RegistrarPeer*>((*peerIterator)());
         CPPUNIT_ASSERT(peer);
         ASSERT_STR_EQUAL("other1.example.com", peer->data());

         Url peerUrl;
         peer->rpcURL(peerUrl);
         UtlString peerUrlString;
         peerUrl.toString(peerUrlString);
         ASSERT_STR_EQUAL("https://other1.example.com:8000/RPC2", peerUrlString.data());

         void* end;
         end = (*peerIterator)();
         CPPUNIT_ASSERT(NULL == end);

         delete peerIterator;

         RegistrarPeer* peer2;
         peer2 = registrar.getPeer("other1.example.com");
         CPPUNIT_ASSERT(peer2);
         ASSERT_STR_EQUAL("other1.example.com", peer2->data());

         Url peer2Url;
         peer->rpcURL(peer2Url);
         UtlString peer2UrlString;
         peer2Url.toString(peer2UrlString);
         ASSERT_STR_EQUAL("https://other1.example.com:8000/RPC2", peer2UrlString.data());

         RegistrarPeer* badPeer;
         badPeer = registrar.getPeer("notapeer.example.com");
         CPPUNIT_ASSERT(NULL == badPeer);
      }


protected:

};

CPPUNIT_TEST_SUITE_REGISTRATION(RegistrarPeerTest);
