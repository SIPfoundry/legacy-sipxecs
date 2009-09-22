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
#include <net/SmimeBody.h>
#include <net/SdpBody.h>
#include <net/HttpMessage.h>
#include <os/OsFS.h>
#include <nss.h>

/**
 * Unittest for SmimeBody
 */
class SmimeBodyTest : public CppUnit::TestCase
{
      CPPUNIT_TEST_SUITE(SmimeBodyTest);
      CPPUNIT_TEST(testLowLevelEncrypt);
      CPPUNIT_TEST(testSmimeBody);
      CPPUNIT_TEST_SUITE_END();

public:

    void testSmimeBody()
    {

    };

    void testLowLevelEncrypt()
    {
        NSS_NoDB_Init(".");
        OsFile file("public-test.pem");

        // create an HttpBody
        UtlString testString("So I was sitting in my cubicle today, and I realized, ever since I started working, every single day of my life has been worse than the day before it. So that means that every single day that you see me, that's on the worst day of my life.");
        SdpBody* pBody = new SdpBody(testString.data(), testString.length());

        // build a key
        const char* pemPublicKeyCert[1];
        const char* derPublicKeyCert[1];
        char szKey[1256];
        unsigned long actualRead = 0;

        file.open();
        file.read((void*)szKey, sizeof(szKey), actualRead);
        CPPUNIT_ASSERT(actualRead > 0);
        file.close();

        size_t certLength[1];
        pemPublicKeyCert[0] = szKey;
        UtlString pem = pemPublicKeyCert[0];
        UtlString der;

        // convert the key from pem to der
        SmimeBody::convertPemToDer(pem, der);

        derPublicKeyCert[0] = der.data();
        certLength[0] = der.length();

        SmimeBody newlyEncryptedBody(NULL, 0, NULL);

        CPPUNIT_ASSERT(true == newlyEncryptedBody.encrypt(pBody, 1, derPublicKeyCert, certLength));
    };
};

CPPUNIT_TEST_SUITE_REGISTRATION(SmimeBodyTest);
