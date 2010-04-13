//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsDefs.h>
#include <os/OsTimerTask.h>
#include <os/OsProcess.h>
#include <net/SipMessage.h>
#include <net/SipUserAgent.h>
#include <net/SipLineMgr.h>
#include <net/SipRefreshMgr.h>
#include <net/SipTcpServer.h>

#define SIP_SHUTDOWN_ITERATIONS 3

/**
 * Unittest for server shutdown testing
 */
class SipServerShutdownTest : public CppUnit::TestCase
{
      CPPUNIT_TEST_SUITE(SipServerShutdownTest);
      CPPUNIT_TEST(testTcpShutdown);
      CPPUNIT_TEST_SUITE_END();

public:

   void testTcpShutdown()
   {
      SipUserAgent sipUA( PORT_NONE
                         ,PORT_NONE
                         ,PORT_NONE
                         ,NULL     // default publicAddress
                         ,NULL     // default defaultUser
                         ,"127.0.0.1"     // default defaultSipAddress
                         ,NULL     // default sipProxyServers
                         ,NULL     // default sipDirectoryServers
                         ,NULL     // default sipRegistryServers
                         ,NULL     // default authenicateRealm
                         ,NULL     // default authenticateDb
                         ,NULL     // default authorizeUserIds
                         ,NULL     // default authorizePasswords
         );

      for (int i=0; i<SIP_SHUTDOWN_ITERATIONS; ++i)
      {
         SipTcpServer pSipTcpServer(5090, &sipUA, "SipTcpServer-%d", false);
         pSipTcpServer.startListener();

         OsTask::delay(1000);

         pSipTcpServer.shutdownListener();
      }

   };

};

CPPUNIT_TEST_SUITE_REGISTRATION(SipServerShutdownTest);
