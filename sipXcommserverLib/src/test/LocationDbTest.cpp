// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include "utl/UtlSList.h"
#include "utl/UtlSListIterator.h"
#include "net/Url.h"
#include "os/OsDateTime.h"

#include "sipdb/LocationDB.h"
#include "sipdb/ResultSet.h"
#include "sipdb/SIPDBManager.h"
#include "testlib/SipDbTestContext.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

class LocationDbTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(LocationDbTest);
   CPPUNIT_TEST(testInsertAndStoreDb);
   CPPUNIT_TEST(getOneRowByNameTest);
   CPPUNIT_TEST(getOneRowByLocationCodeTest);
   CPPUNIT_TEST(getRowByIpAddressTest);
   CPPUNIT_TEST(getRowsTest);
   CPPUNIT_TEST_SUITE_END();

public:

   void setUp()
   {
      
   }
   void tearDown()
   {
      LocationDB::getInstance()->releaseInstance();
   }

   void testInsertAndStoreDb()
   {
      
      SipDbTestContext sipDbTestContext(TEST_DATA_DIR "/locationdata",
                                        TEST_WORK_DIR "/locationdata"
                                                 );
      LocationDB* pLocDb = LocationDB::getInstance("dummy_loc_db");
      CPPUNIT_ASSERT( pLocDb->insertRow( "Ottawa", "One crazy location", "613", "172.30.0.0/16,22.22.22.22/32") );
      CPPUNIT_ASSERT( pLocDb->insertRow( "Quebec", "Happy 400th anniversary", "418", "") );
      CPPUNIT_ASSERT( pLocDb->insertRow( "Montreal", "Go Habs Go!", "514", "10.10.10.0/24") );
      CPPUNIT_ASSERT( pLocDb->insertRow( "Springfield", "It's a hell of a town", "KL5", "47.0.0.0/8") );
      CPPUNIT_ASSERT(pLocDb->store() == OS_SUCCESS );

   }
   
   void getOneRowByNameTest()
   {
      SipDbTestContext sipDbTestContext(TEST_DATA_DIR "/locationdata",
                                        TEST_WORK_DIR "/locationdata"
                                                 );
      sipDbTestContext.inputFile("dummy_loc_db.xml" );
      
      LocationDB* pLocDb = LocationDB::getInstance("dummy_loc_db");
      UtlHashMap hashmap;
      CPPUNIT_ASSERT( !pLocDb->getRowByName( "Washington", hashmap ) );
      CPPUNIT_ASSERT( pLocDb->getRowByName( "Ottawa", hashmap ) );
      CPPUNIT_ASSERT( hashmap.entries() == 4 );

      UtlString* pTempString;
      UtlString key;
      key = "name";
      pTempString = dynamic_cast<UtlString*>(hashmap.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "Ottawa", pTempString->data() );
      
      key = "description";
      pTempString = dynamic_cast<UtlString*>(hashmap.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "One crazy location", pTempString->data() );
      
      key = "locationcode";
      pTempString = dynamic_cast<UtlString*>(hashmap.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "613", pTempString->data() );

      key = "subnets";
      pTempString = dynamic_cast<UtlString*>(hashmap.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "172.30.0.0/16,22.22.22.22/32", pTempString->data() );

      // get a second location row
      CPPUNIT_ASSERT( pLocDb->getRowByName( "Quebec", hashmap ) );
      CPPUNIT_ASSERT( hashmap.entries() == 4 );

      key = "name";
      pTempString = dynamic_cast<UtlString*>(hashmap.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "Quebec", pTempString->data() );
      
      key = "description";
      pTempString = dynamic_cast<UtlString*>(hashmap.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "Happy 400th anniversary", pTempString->data() );
      
      key = "locationcode";
      pTempString = dynamic_cast<UtlString*>(hashmap.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "418", pTempString->data() );

      key = "subnets";
      pTempString = dynamic_cast<UtlString*>(hashmap.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( SPECIAL_IMDB_NULL_VALUE, pTempString->data() );
   }

   void getOneRowByLocationCodeTest()
   {
      SipDbTestContext sipDbTestContext(TEST_DATA_DIR "/locationdata",
                                        TEST_WORK_DIR "/locationdata"
                                                 );
      sipDbTestContext.inputFile("dummy_loc_db.xml" );
      
      LocationDB* pLocDb = LocationDB::getInstance("dummy_loc_db");
      UtlHashMap hashmap;
      CPPUNIT_ASSERT( !pLocDb->getRowByLocationCode( "5144", hashmap ) );
      CPPUNIT_ASSERT( pLocDb->getRowByLocationCode( "514", hashmap ) );
      CPPUNIT_ASSERT( hashmap.entries() == 4 );

      UtlString* pTempString;
      UtlString key;
      key = "name";
      pTempString = dynamic_cast<UtlString*>(hashmap.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "Montreal", pTempString->data() );
      
      key = "description";
      pTempString = dynamic_cast<UtlString*>(hashmap.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "Go Habs Go!", pTempString->data() );
      
      key = "locationcode";
      pTempString = dynamic_cast<UtlString*>(hashmap.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "514", pTempString->data() );

      key = "subnets";
      pTempString = dynamic_cast<UtlString*>(hashmap.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "10.10.10.0/24,11.11.11.0/24,12.12.12.0/24", pTempString->data() );
   }
   
   void getRowByIpAddressTest()
   {
      SipDbTestContext sipDbTestContext(TEST_DATA_DIR "/locationdata",
                                        TEST_WORK_DIR "/locationdata"
                                                 );
      sipDbTestContext.inputFile("dummy_loc_db.xml" );
      
      LocationDB* pLocDb = LocationDB::getInstance("dummy_loc_db");
      UtlHashMap hashmap;
      UtlString key;
      UtlString* pTempString;
      key = "name";
      CPPUNIT_ASSERT( !pLocDb->getRowByIpAddress( "172.29.255.255", hashmap ) );
      CPPUNIT_ASSERT( !pLocDb->getRowByIpAddress( "172.31.0.0", hashmap ) );
      CPPUNIT_ASSERT( !pLocDb->getRowByIpAddress( "22.22.22.21", hashmap ) );
      CPPUNIT_ASSERT( !pLocDb->getRowByIpAddress( "22.22.22.23", hashmap ) );
      CPPUNIT_ASSERT( !pLocDb->getRowByIpAddress( "10.10.9.255", hashmap ) );
      CPPUNIT_ASSERT( !pLocDb->getRowByIpAddress( "10.10.11.0", hashmap ) );
      CPPUNIT_ASSERT( !pLocDb->getRowByIpAddress( "11.11.10.255", hashmap ) );
      CPPUNIT_ASSERT( !pLocDb->getRowByIpAddress( "11.11.12.0", hashmap ) );
      CPPUNIT_ASSERT( !pLocDb->getRowByIpAddress( "12.12.11.255", hashmap ) );
      CPPUNIT_ASSERT( !pLocDb->getRowByIpAddress( "12.12.13.0", hashmap ) );
      CPPUNIT_ASSERT( !pLocDb->getRowByIpAddress( "46.255.255.255", hashmap ) );
      CPPUNIT_ASSERT( !pLocDb->getRowByIpAddress( "48.0.0.0", hashmap ) );

      CPPUNIT_ASSERT( pLocDb->getRowByIpAddress( "172.30.0.0", hashmap ) );
      pTempString = dynamic_cast<UtlString*>( hashmap.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "Ottawa", pTempString->data() );

      CPPUNIT_ASSERT( pLocDb->getRowByIpAddress( "172.30.255.255", hashmap ) );
      pTempString = dynamic_cast<UtlString*>( hashmap.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "Ottawa", pTempString->data() );
/*
      CPPUNIT_ASSERT( pLocDb->getRowByIpAddress( "22.22.22.22", hashmap ) );
      pTempString = dynamic_cast<UtlString*>( hashmap.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "Ottawa", pTempString->data() );
*/
      CPPUNIT_ASSERT( pLocDb->getRowByIpAddress( "10.10.10.0", hashmap ) );
      pTempString = dynamic_cast<UtlString*>( hashmap.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "Montreal", pTempString->data() );
      
      CPPUNIT_ASSERT( pLocDb->getRowByIpAddress( "10.10.10.255", hashmap ) );
      pTempString = dynamic_cast<UtlString*>( hashmap.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "Montreal", pTempString->data() );
      
      CPPUNIT_ASSERT( pLocDb->getRowByIpAddress( "11.11.11.0", hashmap ) );
      pTempString = dynamic_cast<UtlString*>( hashmap.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "Montreal", pTempString->data() );
      
      CPPUNIT_ASSERT( pLocDb->getRowByIpAddress( "11.11.11.255", hashmap ) );
      pTempString = dynamic_cast<UtlString*>( hashmap.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "Montreal", pTempString->data() );
      
      CPPUNIT_ASSERT( pLocDb->getRowByIpAddress( "12.12.12.0", hashmap ) );
      pTempString = dynamic_cast<UtlString*>( hashmap.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "Montreal", pTempString->data() );
      
      CPPUNIT_ASSERT( pLocDb->getRowByIpAddress( "12.12.12.255", hashmap ) );
      pTempString = dynamic_cast<UtlString*>( hashmap.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "Montreal", pTempString->data() );
      
      CPPUNIT_ASSERT( pLocDb->getRowByIpAddress( "47.0.0.0", hashmap ) );
      pTempString = dynamic_cast<UtlString*>( hashmap.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "Springfield", pTempString->data() );
      
      CPPUNIT_ASSERT( pLocDb->getRowByIpAddress( "47.255.255.255", hashmap ) );
      pTempString = dynamic_cast<UtlString*>( hashmap.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "Springfield", pTempString->data() );
      
      key = "description";
      pTempString = dynamic_cast<UtlString*>(hashmap.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "It's a hell of a town", pTempString->data() );
      
      key = "locationcode";
      pTempString = dynamic_cast<UtlString*>(hashmap.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "KL5", pTempString->data() );

      key = "subnets";
      pTempString = dynamic_cast<UtlString*>(hashmap.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "47.0.0.0/8", pTempString->data() );      
   }
   
   void getRowsTest()
   {
      SipDbTestContext sipDbTestContext(TEST_DATA_DIR "/locationdata",
                                        TEST_WORK_DIR "/locationdata"
                                                 );
      sipDbTestContext.inputFile("dummy_loc_db.xml" );
      
      LocationDB* pLocDb = LocationDB::getInstance("dummy_loc_db");
      ResultSet resultSet;
      UtlHashMap hashmap1;
      UtlHashMap hashmap2;
      UtlHashMap hashmap3;
      UtlHashMap hashmap4;
      UtlString* pTempString;
      UtlString key;

      pLocDb->getAllRows( resultSet );
      CPPUNIT_ASSERT( resultSet.getSize() == 4 );

      CPPUNIT_ASSERT( resultSet.getIndex( 0, hashmap1 ) == OS_SUCCESS );
      key = "name";
      pTempString = dynamic_cast<UtlString*>(hashmap1.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "Ottawa", pTempString->data() );
      
      key = "description";
      pTempString = dynamic_cast<UtlString*>(hashmap1.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "One crazy location", pTempString->data() );
      
      key = "locationcode";
      pTempString = dynamic_cast<UtlString*>(hashmap1.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "613", pTempString->data() );

      key = "subnets";
      pTempString = dynamic_cast<UtlString*>(hashmap1.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "172.30.0.0/16,22.22.22.22/32", pTempString->data() );

      CPPUNIT_ASSERT( resultSet.getIndex( 1, hashmap2 ) == OS_SUCCESS );
      key = "name";
      pTempString = dynamic_cast<UtlString*>(hashmap2.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "Quebec", pTempString->data() );
      
      key = "description";
      pTempString = dynamic_cast<UtlString*>(hashmap2.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "Happy 400th anniversary", pTempString->data() );
      
      key = "locationcode";
      pTempString = dynamic_cast<UtlString*>(hashmap2.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "418", pTempString->data() );

      key = "subnets";
      pTempString = dynamic_cast<UtlString*>(hashmap2.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( SPECIAL_IMDB_NULL_VALUE, pTempString->data() );

      CPPUNIT_ASSERT( resultSet.getIndex( 2, hashmap3 ) == OS_SUCCESS );
      key = "name";
      pTempString = dynamic_cast<UtlString*>(hashmap3.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "Montreal", pTempString->data() );
      
      key = "description";
      pTempString = dynamic_cast<UtlString*>(hashmap3.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "Go Habs Go!", pTempString->data() );
      
      key = "locationcode";
      pTempString = dynamic_cast<UtlString*>(hashmap3.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "514", pTempString->data() );

      key = "subnets";
      pTempString = dynamic_cast<UtlString*>(hashmap3.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "10.10.10.0/24,11.11.11.0/24,12.12.12.0/24", pTempString->data() );

      CPPUNIT_ASSERT( resultSet.getIndex( 3, hashmap4 ) == OS_SUCCESS );
      key = "name";
      pTempString = dynamic_cast<UtlString*>(hashmap4.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "Springfield", pTempString->data() );
      
      key = "description";
      pTempString = dynamic_cast<UtlString*>(hashmap4.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "It's a hell of a town", pTempString->data() );
      
      key = "locationcode";
      pTempString = dynamic_cast<UtlString*>(hashmap4.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "KL5", pTempString->data() );

      key = "subnets";
      pTempString = dynamic_cast<UtlString*>(hashmap4.findValue( &key ) );
      CPPUNIT_ASSERT( pTempString );
      ASSERT_STR_EQUAL( "47.0.0.0/8", pTempString->data() );
   }
   
};

CPPUNIT_TEST_SUITE_REGISTRATION(LocationDbTest);
