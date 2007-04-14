//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include "processcgi/processXMLCommon.h"
#include "utl/UtlString.h"
#include "xmlparser/tinyxml.h"

int gSubDoc = 0;

OsStatus subDocCounter(TiXmlDocument &root, TiXmlDocument &subdoc)
{
    gSubDoc++;
    return OS_SUCCESS;
}

/**
 * Unittest for OsTime
 */
class WatchdogProcessXmlTest : public CppUnit::TestCase
{

    CPPUNIT_TEST_SUITE(WatchdogProcessXmlTest);
    CPPUNIT_TEST(testFindProcessSubDocs);
    CPPUNIT_TEST(testAddWatchDogSubDoc);
    CPPUNIT_TEST(testAddProcessDefSubDoc);
    CPPUNIT_TEST_SUITE_END();


public:
    void testFindProcessSubDocs()
    {
        OsPath processFile = OsPath(TEST_SRC_DIR) + OsPath::separator + "process.xml";
        TiXmlDocument processDoc;
	bool success = processDoc.LoadFile(processFile);
	CPPUNIT_ASSERT(success);

        OsPath processDir = OsPath(TEST_SRC_DIR) + OsPath::separator + "process.d";

        gSubDoc = 0;
        OsStatus status = findSubDocs(processDir, processDoc, &subDocCounter);
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, status);
        CPPUNIT_ASSERT_EQUAL(1, gSubDoc);
    }

    void testAddWatchDogSubDoc()
    {
        OsPath watchDogFile = OsPath(TEST_SRC_DIR) + OsPath::separator + "watchdog.xml";
        TiXmlDocument watchDogDoc;
        bool success = watchDogDoc.LoadFile(watchDogFile);
        CPPUNIT_ASSERT(success);

        OsPath subdocFile = OsPath(TEST_SRC_DIR) + OsPath::separator + "process.d" + 
            OsPath::separator +  "app.process.xml";
        TiXmlDocument subdoc;
        success = subdoc.LoadFile(subdocFile);
        CPPUNIT_ASSERT(success);        

        OsStatus status = addWatchDogSubDoc(watchDogDoc, subdoc);
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, status);

        TiXmlElement *root = watchDogDoc.RootElement();
        TiXmlNode *monitor = root->FirstChild("monitor");
        int count = countNodes(monitor, "monitor-process");
        CPPUNIT_ASSERT_EQUAL(1, count);
    }

    void testAddProcessDefSubDoc()
    {
        OsPath processDefFile = OsPath(TEST_SRC_DIR) + OsPath::separator + "process.xml";
        TiXmlDocument processDefDoc;
        bool success = processDefDoc.LoadFile(processDefFile);
        CPPUNIT_ASSERT(success);

        OsPath subdocFile = OsPath(TEST_SRC_DIR) + OsPath::separator + "process.d" + 
            OsPath::separator +  "app.process.xml";
        TiXmlDocument subdoc;
        success = subdoc.LoadFile(subdocFile);
        CPPUNIT_ASSERT(success);        

        OsStatus status = addProcessDefSubDoc(processDefDoc, subdoc);
        CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, status);

        TiXmlElement *root = processDefDoc.RootElement();
        int count = countNodes(root, "group");
        CPPUNIT_ASSERT_EQUAL(1, count);
    }

    int countNodes(TiXmlNode *node, const char *childName) 
    {
        int count = 0;
        for (TiXmlNode *child = node->FirstChild(childName);
             child != NULL;
             child = child->NextSibling(childName)) 
        {
            count++;
        }

        return count;
    }
        
};


CPPUNIT_TEST_SUITE_REGISTRATION(WatchdogProcessXmlTest);

