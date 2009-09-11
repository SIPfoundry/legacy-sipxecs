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
#include <iostream>
#include <memory>
#include <sipxunit/TestUtilities.h>
#include <stdlib.h>

// APPLICATION INCLUDES
#include "net/Url.h"
#include "os/OsDateTime.h"
#include "os/OsFS.h"
#include "os/OsProcess.h"
#include "testlib/RegistrationDbTestContext.h"
#include "registry/SipRegistrar.h"
#include "SipRegistrarServer.h"

using namespace std;

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

class SipRegistrarServerTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SipRegistrarServerTest);
   // Commented out to prevent semaphore leakage, which causes serious problem
   // on the build machines.  See XECS-546.
#if 0
   CPPUNIT_TEST(testPullUpdates);
#endif
   CPPUNIT_TEST_SUITE_END();

public:
   void setUp()
      {
         RegistrationDbTestContext testDbContext(TEST_DATA_DIR "/regdbdata",
                                                 TEST_WORK_DIR "/regdbdata"
                                                 );
         testDbContext.inputFile("updatesToPull.xml");
      }

   void testPullUpdates()
      {
         OsConfigDb configuration;

         SipRegistrar registrar(&configuration);

         // Pull all updates with primary = R1 and updateNumber > 1.
         UtlSList updates;
         int numUpdates = registrar.getRegistrarServer().pullUpdates(
            "R1",        // registrarName
            1,           // updateNumber -- pull only the updates with larger numbers
            updates);    // updates are returned in this list

         // Verify that the right updates got pulled.
         CPPUNIT_ASSERT_EQUAL(1, numUpdates);
      }

};

CPPUNIT_TEST_SUITE_REGISTRATION(SipRegistrarServerTest);
