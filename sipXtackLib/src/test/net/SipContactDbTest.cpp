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
#include <os/OsSocket.h>
#include <net/SipContactDb.h>

/**
 * Unittest for SipContactDb
 */
class SipContactDbTest : public CppUnit::TestCase
{
      CPPUNIT_TEST_SUITE(SipContactDbTest);
      CPPUNIT_TEST(testSipContactDb);
      CPPUNIT_TEST_SUITE_END();

public:

    void testSipContactDb()
    {
        // first, create a new contact Db
        SipContactDb pDb;

        // test the inserting of records
        ContactAddress contact1;
        memset((void*)&contact1, 0, sizeof(ContactAddress));
        strcpy(contact1.cInterface, "eth0");
        strcpy(contact1.cIpAddress, "9.9.9.1");
        contact1.eContactType = ContactAddress::NAT_MAPPED;
        contact1.iPort = 9991;
        CPPUNIT_ASSERT(pDb.addContact(contact1));
        CPPUNIT_ASSERT(contact1.id == 1);

        // test the addition of a duplicate (same IP and port)
        // (should fail)
        ContactAddress contact2;
        memset((void*)&contact2, 0, sizeof(ContactAddress));
        strcpy(contact2.cInterface, "eth0");
        strcpy(contact2.cIpAddress, "9.9.9.1");
        contact2.eContactType = ContactAddress::LOCAL;
        contact2.iPort = 9991;
        CPPUNIT_ASSERT(pDb.addContact(contact2) == false);
        CPPUNIT_ASSERT(contact2.id == 1);

        // test the addition of same IP, different port
        // (should succeed)
        ContactAddress contact3;
        memset((void*)&contact3, 0, sizeof(ContactAddress));
        strcpy(contact3.cInterface, "eth0");
        strcpy(contact3.cIpAddress, "9.9.9.1");
        contact3.eContactType = ContactAddress::LOCAL;
        contact3.iPort = 9992;
        CPPUNIT_ASSERT(pDb.addContact(contact3) == true);
        CPPUNIT_ASSERT(contact3.id == 2);

        // test the addition of differnt IP
        // same adapter
        // (should succeed)
        ContactAddress contact4;
        memset((void*)&contact4, 0, sizeof(ContactAddress));
        strcpy(contact4.cInterface, "eth0");
        strcpy(contact4.cIpAddress, "9.9.9.2");
        contact4.eContactType = ContactAddress::RELAY;
        contact4.iPort = 9993;
        CPPUNIT_ASSERT(pDb.addContact(contact4) == true);
        CPPUNIT_ASSERT(contact4.id == 3);

        // test the addition of differnt IP
        // same adapter
        // (should succeed)
        ContactAddress contact5;
        memset((void*)&contact5, 0, sizeof(ContactAddress));
        strcpy(contact5.cInterface, "eth1");
        strcpy(contact5.cIpAddress, "10.10.10.5");
        contact5.eContactType = ContactAddress::LOCAL;
        contact5.iPort = 9991;
        CPPUNIT_ASSERT(pDb.addContact(contact5) == true);
        CPPUNIT_ASSERT(contact5.id == 4);

        // now test the finding of the records
        ContactAddress* pFound = NULL;
        // search by ID - positive
        pFound = pDb.find(4);
        CPPUNIT_ASSERT(pFound != NULL);
        CPPUNIT_ASSERT(pFound->id == 4);
        CPPUNIT_ASSERT(strcmp(pFound->cInterface, "eth1") == 0);
        CPPUNIT_ASSERT(strcmp(pFound->cIpAddress, "10.10.10.5") == 0);
        CPPUNIT_ASSERT(pFound->iPort == 9991);

        // search by ID - negative
        pFound = pDb.find(0);
        CPPUNIT_ASSERT(pFound == NULL);

        // search by IP and port - positive
        pFound = pDb.find("9.9.9.1", 9991);
        CPPUNIT_ASSERT(pFound != NULL);
        CPPUNIT_ASSERT(pFound->id == 1);

        // search by IP and port - negative
        // bad IP
        pFound = pDb.find("zaphod", 9991);
        CPPUNIT_ASSERT(pFound == NULL);

        // search by IP and port - negative
        // bad port
        pFound = pDb.find("9.9.9.1", 42);
        CPPUNIT_ASSERT(pFound == NULL);

        // get All records
        ContactAddress* addresses[MAX_IP_ADDRESSES];
        int num = 0;
        pDb.getAll(addresses, num);
        CPPUNIT_ASSERT(4 == num);

        for (int i = 0; i < num; i++)
        {
            delete addresses[i];
        }

        // remove records
        CPPUNIT_ASSERT(pDb.deleteContact(1) == true);
        CPPUNIT_ASSERT(pDb.deleteContact(2) == true);
        CPPUNIT_ASSERT(pDb.deleteContact(3) == true);
        CPPUNIT_ASSERT(pDb.deleteContact(4) == true);
    };
};

CPPUNIT_TEST_SUITE_REGISTRATION(SipContactDbTest);
