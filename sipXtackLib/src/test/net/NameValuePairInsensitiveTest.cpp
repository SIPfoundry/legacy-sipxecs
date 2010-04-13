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

#include <net/NameValuePairInsensitive.h>

class NameValuePairInsensitiveTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(NameValuePairInsensitiveTest);
    CPPUNIT_TEST(testAccessors);
    CPPUNIT_TEST(testHash);
    CPPUNIT_TEST_SUITE_END();


public:
    void testAccessors()
    {
        const char* name = "xxx";
        const char* name_different_case = "XXX";
        const char* value = "xxx-value";
        const char* valueRef;

        NameValuePairInsensitive* nv = new NameValuePairInsensitive(name);
        NameValuePairInsensitive* nv_different_case =
           new NameValuePairInsensitive(name_different_case);

        valueRef = nv->getValue();
        CPPUNIT_ASSERT_MESSAGE("value should be null", NULL == valueRef);

        nv->setValue(value);
        valueRef = nv->getValue();
        ASSERT_STR_EQUAL_MESSAGE("incorrect value retrieved", value, valueRef);

        nv->setValue(name);
        valueRef = nv->getValue();
        ASSERT_STR_EQUAL_MESSAGE("incorrect value retrieved", valueRef, name);

        CPPUNIT_ASSERT_MESSAGE("nv should be == nv", nv->compareTo(nv) == 0);
        CPPUNIT_ASSERT_MESSAGE("nv should be == nv_different_case",
                               nv->compareTo(nv_different_case) == 0);

        delete nv;
    }

   void testHash()
      {
         const char* name = "eventtype";
         const char* name_different_case = "eventType";

         NameValuePairInsensitive* nv = new NameValuePairInsensitive(name);
         NameValuePairInsensitive* nv_different_case =
            new NameValuePairInsensitive(name_different_case);

         CPPUNIT_ASSERT_MESSAGE("values do not compare equal",
                                nv->isEqual(nv_different_case)
                                );
         CPPUNIT_ASSERT_MESSAGE("hash values do not match",
                                nv->hash() == nv_different_case->hash()
                                );
      }

};

CPPUNIT_TEST_SUITE_REGISTRATION(NameValuePairInsensitiveTest);
