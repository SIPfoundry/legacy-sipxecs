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

#include "sipdb/UserLocationDB.h"
#include "sipdb/ResultSet.h"
#include "sipdb/SIPDBManager.h"
#include "testlib/SipDbTestContext.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

class UserLocationDbTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(UserLocationDbTest);
   CPPUNIT_TEST(testInsertAndStoreDb);
   CPPUNIT_TEST(getLocationsTest);
   CPPUNIT_TEST(getIdentitiesTest);
   CPPUNIT_TEST(hasLocationTest);
   CPPUNIT_TEST(nonExistentFileHandlingTest);
   CPPUNIT_TEST_SUITE_END();

public:

   void setUp()
   {
      
   }
   void tearDown()
   {
      UserLocationDB::getInstance()->releaseInstance();
   }

   void testInsertAndStoreDb()
   {
      
      SipDbTestContext sipDbTestContext(TEST_DATA_DIR "/locationdata",
                                        TEST_WORK_DIR "/locationdata"
                                                 );
      UserLocationDB* pLocDb = UserLocationDB::getInstance("dummy_userlocation_db");
      CPPUNIT_ASSERT( pLocDb->insertRow( Url( "201@example.com" ), "Ottawa"         ) );
      CPPUNIT_ASSERT( pLocDb->insertRow( Url( "202@example.com" ), "Montreal"       ) );
      CPPUNIT_ASSERT( pLocDb->insertRow( Url( "203@example.com" ), "Venise-Sur-Mer" ) );
      CPPUNIT_ASSERT( pLocDb->insertRow( Url( "204@example.com" ), "Drummondville"  ) );
      CPPUNIT_ASSERT( pLocDb->insertRow( Url( "205@example.com" ), "Ottawa"         ) );
      CPPUNIT_ASSERT( pLocDb->insertRow( Url( "206@example.com" ), "Ottawa"         ) );
      CPPUNIT_ASSERT(pLocDb->store() == OS_SUCCESS );
   }
   
   void getLocationsTest()
   {
      UtlString callerLocation, callerLocation2;
      SipDbTestContext sipDbTestContext(TEST_DATA_DIR "/locationdata",
                                        TEST_WORK_DIR "/locationdata"
                                                 );
      sipDbTestContext.inputFile("dummy_userlocation_db.xml" );
      
      UserLocationDB* pLocDb = UserLocationDB::getInstance("dummy_userlocation_db");
      ResultSet userLocationsResult;
      UtlString locationKey("location");

      pLocDb->getLocations( "201@example.com", userLocationsResult );
      CPPUNIT_ASSERT( userLocationsResult.getSize() == 2 );
      callerLocation.remove( 0 );
      UtlHashMap record1;
      CPPUNIT_ASSERT( userLocationsResult.getIndex( 0, record1 ) == OS_SUCCESS );
      CPPUNIT_ASSERT( (callerLocation = *((UtlString*)record1.findValue( &locationKey ) ) ) );
      UtlHashMap record2;
      CPPUNIT_ASSERT( userLocationsResult.getIndex( 1, record2 ) == OS_SUCCESS );
      CPPUNIT_ASSERT( (callerLocation2 = *((UtlString*)record2.findValue( &locationKey ) ) ) );
      // test below does not assume any ordering of the records within result set
		CPPUNIT_ASSERT( ( callerLocation.compareTo("Quebec") == 0 && callerLocation2.compareTo("Ottawa") == 0 ) ||
					       ( callerLocation2.compareTo("Quebec") == 0 && callerLocation.compareTo("Ottawa") == 0 ) );

      pLocDb->getLocations( "202@example.com", userLocationsResult );
      CPPUNIT_ASSERT( userLocationsResult.getSize() == 1 );
      callerLocation.remove( 0 );
      UtlHashMap record3;
      CPPUNIT_ASSERT( userLocationsResult.getIndex( 0, record3 ) == OS_SUCCESS );
      CPPUNIT_ASSERT( (callerLocation = *((UtlString*)record3.findValue( &locationKey ) ) ) );
      ASSERT_STR_EQUAL( "Montreal", callerLocation.data() );
      
      pLocDb->getLocations( "203@example.com", userLocationsResult );
      CPPUNIT_ASSERT( userLocationsResult.getSize() == 1 );
      callerLocation.remove( 0 );
      UtlHashMap record4;
      CPPUNIT_ASSERT( userLocationsResult.getIndex( 0, record4 ) == OS_SUCCESS );
      CPPUNIT_ASSERT( (callerLocation = *((UtlString*)record4.findValue( &locationKey ) ) ) );
      ASSERT_STR_EQUAL( "Venise-Sur-Mer", callerLocation.data() );
      
      pLocDb->getLocations( "203@example.com", userLocationsResult );
      CPPUNIT_ASSERT( userLocationsResult.getSize() == 1 );
      callerLocation.remove( 0 );
      UtlHashMap record5;
      CPPUNIT_ASSERT( userLocationsResult.getIndex( 0, record5 ) == OS_SUCCESS );
      CPPUNIT_ASSERT( (callerLocation = *((UtlString*)record5.findValue( &locationKey ) ) ) );
      ASSERT_STR_EQUAL( "Venise-Sur-Mer", callerLocation.data() );
      
      pLocDb->getLocations( "204@example.com", userLocationsResult );
      CPPUNIT_ASSERT( userLocationsResult.getSize() == 1 );
      callerLocation.remove( 0 );
      UtlHashMap record7;
      CPPUNIT_ASSERT( userLocationsResult.getIndex( 0, record7 ) == OS_SUCCESS );
      CPPUNIT_ASSERT( (callerLocation = *((UtlString*)record7.findValue( &locationKey ) ) ) );
      ASSERT_STR_EQUAL( "Drummondville", callerLocation.data() );
      
      pLocDb->getLocations( "205@example.com", userLocationsResult );
      CPPUNIT_ASSERT( userLocationsResult.getSize() == 1 );
      callerLocation.remove( 0 );
      UtlHashMap record8;
      CPPUNIT_ASSERT( userLocationsResult.getIndex( 0, record8 ) == OS_SUCCESS );
      CPPUNIT_ASSERT( (callerLocation = *((UtlString*)record8.findValue( &locationKey ) ) ) );
      ASSERT_STR_EQUAL( "Ottawa", callerLocation.data() );
      
      pLocDb->getLocations( "206@example.com", userLocationsResult );
      CPPUNIT_ASSERT( userLocationsResult.getSize() == 1 );
      callerLocation.remove( 0 );
      UtlHashMap record9;
      CPPUNIT_ASSERT( userLocationsResult.getIndex( 0, record9 ) == OS_SUCCESS );
      CPPUNIT_ASSERT( (callerLocation = *((UtlString*)record9.findValue( &locationKey ) ) ) );
      ASSERT_STR_EQUAL( "Ottawa", callerLocation.data() );

      pLocDb->getLocations( "207@example.com", userLocationsResult );
      CPPUNIT_ASSERT( userLocationsResult.getSize() == 0 );
      
      pLocDb->getLocations( "206@example2.com", userLocationsResult );
      CPPUNIT_ASSERT( userLocationsResult.getSize() == 0 );
   }
   
   void getIdentitiesTest()
   {
      UtlString id1, id2, id3;
      SipDbTestContext sipDbTestContext(TEST_DATA_DIR "/locationdata",
                                        TEST_WORK_DIR "/locationdata"
                                                 );
      sipDbTestContext.inputFile("dummy_userlocation_db.xml" );
      
      UserLocationDB* pLocDb = UserLocationDB::getInstance("dummy_userlocation_db");
      ResultSet identitiesResult;
      UtlString key("identity");

      pLocDb->getIdentities( "Ottawa", identitiesResult );
      CPPUNIT_ASSERT( identitiesResult.getSize() == 3 );
      UtlHashMap record1;
      CPPUNIT_ASSERT( identitiesResult.getIndex( 0, record1 ) == OS_SUCCESS );
      CPPUNIT_ASSERT( (id1 = *((UtlString*)record1.findValue( &key ) ) ) );
      UtlHashMap record2;
      CPPUNIT_ASSERT( identitiesResult.getIndex( 1, record2 ) == OS_SUCCESS );
      CPPUNIT_ASSERT( (id2 = *((UtlString*)record2.findValue( &key ) ) ) );
      UtlHashMap record3;
      CPPUNIT_ASSERT( identitiesResult.getIndex( 2, record3 ) == OS_SUCCESS );
      CPPUNIT_ASSERT( (id3 = *((UtlString*)record3.findValue( &key ) ) ) );
      CPPUNIT_ASSERT( id1.compareTo("201@example.com") == 0 || 
                      id2.compareTo("201@example.com") == 0 || 
                      id3.compareTo("201@example.com") == 0 );
      CPPUNIT_ASSERT( id1.compareTo("205@example.com") == 0 || 
                      id2.compareTo("205@example.com") == 0 || 
                      id3.compareTo("205@example.com") == 0 );
      CPPUNIT_ASSERT( id1.compareTo("206@example.com") == 0 || 
                      id2.compareTo("206@example.com") == 0 || 
                      id3.compareTo("206@example.com") == 0 );

      pLocDb->getIdentities( "Chicoutimi", identitiesResult );
      CPPUNIT_ASSERT( identitiesResult.getSize() == 0 );

      pLocDb->getIdentities( "Montreal", identitiesResult );
      CPPUNIT_ASSERT( identitiesResult.getSize() == 1 );
      UtlHashMap record4;
      CPPUNIT_ASSERT( identitiesResult.getIndex( 0, record4 ) == OS_SUCCESS );
      CPPUNIT_ASSERT( (id1 = *((UtlString*)record4.findValue( &key ) ) ) );
      ASSERT_STR_EQUAL( "202@example.com", id1.data() );      

      pLocDb->getIdentities( "Quebec", identitiesResult );
      CPPUNIT_ASSERT( identitiesResult.getSize() == 1 );
      UtlHashMap record5;
      CPPUNIT_ASSERT( identitiesResult.getIndex( 0, record5 ) == OS_SUCCESS );
      CPPUNIT_ASSERT( (id1 = *((UtlString*)record5.findValue( &key ) ) ) );
      ASSERT_STR_EQUAL( "201@example.com", id1.data() );      
	}   
	
   void hasLocationTest()
   {
      SipDbTestContext sipDbTestContext(TEST_DATA_DIR "/locationdata",
                                        TEST_WORK_DIR "/locationdata"
                                                 );
      sipDbTestContext.inputFile("dummy_userlocation_db.xml" );
      
      UserLocationDB* pLocDb = UserLocationDB::getInstance("dummy_userlocation_db");

      CPPUNIT_ASSERT( pLocDb->hasLocation( "201@example.com" ) );
      CPPUNIT_ASSERT( pLocDb->hasLocation( "202@example.com" ) );
      CPPUNIT_ASSERT( pLocDb->hasLocation( "203@example.com" ) );
      CPPUNIT_ASSERT( pLocDb->hasLocation( "204@example.com" ) );
      CPPUNIT_ASSERT( pLocDb->hasLocation( "205@example.com" ) );
      CPPUNIT_ASSERT( pLocDb->hasLocation( "206@example.com" ) );
      CPPUNIT_ASSERT( !pLocDb->hasLocation( "207@example.com" ) );
   }
   
   void nonExistentFileHandlingTest()
   {
      ResultSet userLocationsResult;

      SipDbTestContext sipDbTestContext(TEST_DATA_DIR "/locationdata",
                                        TEST_WORK_DIR "/locationdata"
                                                 );
      UserLocationDB* pLocDb = UserLocationDB::getInstance("this_file_does_not_exist");

      CPPUNIT_ASSERT( !pLocDb->hasLocation( "201@example.com" ) );
      CPPUNIT_ASSERT( !pLocDb->hasLocation( "202@example.com" ) );
      CPPUNIT_ASSERT( !pLocDb->hasLocation( "203@example.com" ) );
      CPPUNIT_ASSERT( !pLocDb->hasLocation( "204@example.com" ) );
      CPPUNIT_ASSERT( !pLocDb->hasLocation( "205@example.com" ) );
      CPPUNIT_ASSERT( !pLocDb->hasLocation( "206@example.com" ) );
      CPPUNIT_ASSERT( !pLocDb->hasLocation( "207@example.com" ) );
      
      pLocDb->getLocations( "207@example.com", userLocationsResult );
      CPPUNIT_ASSERT( userLocationsResult.getSize() == 0 );
   }
	
};

CPPUNIT_TEST_SUITE_REGISTRATION(UserLocationDbTest);
