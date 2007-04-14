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
   CPPUNIT_TEST(testPullUpdates);
   CPPUNIT_TEST_SUITE_END();

public:
   SipRegistrarServerTest() :
      mRegistrar(NULL)
      {}

   void testPullUpdates()
      {
         RegistrationDbTestContext testDbContext(TEST_DATA_DIR "/regdbdata",
                                                 TEST_WORK_DIR "/regdbdata"
                                                 );
         testDbContext.inputFile("updatesToPull.xml");

         // Pull all updates with primary = R1 and updateNumber > 1.
         UtlSList updates;
         int numUpdates = getRegistrarServer().pullUpdates(
            "R1",        // registrarName
            1,           // updateNumber -- pull only the updates with larger numbers
            updates);    // updates are returned in this list

         // Verify that the right updates got pulled.
         CPPUNIT_ASSERT_EQUAL(1, numUpdates);
      }

   // Create the registrar for testing, without starting the associated thread
   void setUp()
      {
         // Create and initialize the registrar, but don't start it.
         // For unit testing, we just need the registrar object, not the thread.
         // This arrangement is wacky and we'll try to improve it in the future.
         OsConfigDb configDb;        // empty configuration DB is OK
         mRegistrar = SipRegistrar::getInstance(&configDb);

         // The config was empty so replication is not configured
         CPPUNIT_ASSERT(!mRegistrar->isReplicationConfigured());
      }

   void tearDown()
      {
         delete mRegistrar;
      }
   
private:
   SipRegistrar*      mRegistrar;

   SipRegistrarServer& getRegistrarServer()
      {
         return mRegistrar->getRegistrarServer();
      }

};

CPPUNIT_TEST_SUITE_REGISTRATION(SipRegistrarServerTest);
