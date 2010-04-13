//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include "os/OsConfigDb.h"
#include "net/NetMd5Codec.h"
#include "sipXecsService/SipXecsService.h"
#include "sipXecsService/SharedSecret.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

class SharedSecretTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SharedSecretTest);

   CPPUNIT_TEST(testReadConfig);

   CPPUNIT_TEST_SUITE_END();

public:

   void setUp()
      {
         setenv(SipXecsService::ConfigurationDirType, TEST_DATA_DIR "/sharedsecret", true );
      }

   void tearDown()
      {
         unsetenv(SipXecsService::ConfigurationDirType);
      }

   void testReadConfig()
      {
         // read the domain configuration
         OsConfigDb domainConfiguration;
         domainConfiguration.loadFromFile(SipXecsService::domainConfigPath());

         // get the shared secret for generating signatures
         SharedSecret signingSecret(domainConfiguration);

         ASSERT_STR_EQUAL("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
                          signingSecret.data());

         UtlString value("value");

         // generate a hash that signs the (existing) value
         NetMd5Codec signature;

         signature.hash(signingSecret);
         signature.hash(value);

         UtlString signedValue(value);
         signature.appendHashValue(signedValue);

         ASSERT_STR_EQUAL("value52deaa6aa21db3721bc97eb11e243bab",
                          signedValue.data());

      }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SharedSecretTest);
