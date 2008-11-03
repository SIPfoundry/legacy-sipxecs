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
#include "registry/RedirectPlugin.h"
#include "net/Url.h"
#include "DummyPlugin.h"

using namespace std;

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

class ContactListTest : public CppUnit::TestCase, public DummyPlugin
{
   CPPUNIT_TEST_SUITE(ContactListTest);
   CPPUNIT_TEST( GetAddStringTest );
   CPPUNIT_TEST( GetAddUrlTest );
   CPPUNIT_TEST( GetSetStringTest );
   CPPUNIT_TEST( GetSetUrlTest );
   CPPUNIT_TEST( RemoveTest );
   CPPUNIT_TEST( RemoveAllTest );
   CPPUNIT_TEST( ModificationTrackingTest );
   CPPUNIT_TEST_SUITE_END();

public:
   ContactListTest() : DummyPlugin("Dummy PlugIn"){}
   void setUp()
   {
   }

   void GetAddStringTest()
   {
      UtlString tmpString;
      
      ContactList contactList("sip:dontdowhat@jimmydont.does");

      CPPUNIT_ASSERT( contactList.entries() == 0 );
      CPPUNIT_ASSERT( contactList.get( 0, tmpString ) == false ); 
      CPPUNIT_ASSERT( contactList.get( 1, tmpString ) == false ); 
      CPPUNIT_ASSERT( contactList.wasListModified() == false );
      
      CPPUNIT_ASSERT( contactList.add( UtlString("sip:401@192.168.0.102:5060"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.wasListModified() == true );
      CPPUNIT_ASSERT( contactList.get( 0, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:401@192.168.0.102:5060", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 1, tmpString ) == false ); 
      ASSERT_STR_EQUAL( "sip:401@192.168.0.102:5060", tmpString.data() );
      CPPUNIT_ASSERT( contactList.entries() == 1 );
      
      CPPUNIT_ASSERT( contactList.add( UtlString("sip:402@192.168.0.103:5061"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.get( 0, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:401@192.168.0.102:5060", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 1, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:402@192.168.0.103:5061", tmpString.data() );
      CPPUNIT_ASSERT( contactList.entries() == 2 );
   }

   void GetAddUrlTest()
   {
      UtlString tmpString;
      
      ContactList contactList("sip:dontdowhat@jimmydont.does");

      CPPUNIT_ASSERT( contactList.entries() == 0 );
      CPPUNIT_ASSERT( contactList.get( 0, tmpString ) == false ); 
      CPPUNIT_ASSERT( contactList.get( 1, tmpString ) == false ); 
      CPPUNIT_ASSERT( contactList.wasListModified() == false );

      CPPUNIT_ASSERT( contactList.add( Url("sip:401@192.168.0.102:5060"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.wasListModified() == true );
      CPPUNIT_ASSERT( contactList.get( 0, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:401@192.168.0.102:5060", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 1, tmpString ) == false ); 
      ASSERT_STR_EQUAL( "sip:401@192.168.0.102:5060", tmpString.data() );
      CPPUNIT_ASSERT( contactList.entries() == 1 );
      
      CPPUNIT_ASSERT( contactList.add( Url("sip:402@192.168.0.103:5061"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.get( 0, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:401@192.168.0.102:5060", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 1, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:402@192.168.0.103:5061", tmpString.data() );
      CPPUNIT_ASSERT( contactList.entries() == 2 );
   }

   void GetSetStringTest()
   {
      UtlString tmpString;
      
      ContactList contactList("sip:dontdowhat@jimmydont.does");

      CPPUNIT_ASSERT( contactList.entries() == 0 );
      CPPUNIT_ASSERT( contactList.add( Url("sip:401@192.168.0.102:5060"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:402@192.168.0.102:5061"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:403@192.168.0.102:5062"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:404@192.168.0.102:5063"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:405@192.168.0.102:5064"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:406@192.168.0.102:5065"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:407@192.168.0.102:5066"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:408@192.168.0.102:5067"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:409@192.168.0.102:5068"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:410@192.168.0.102:5069"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.entries() == 10);

      CPPUNIT_ASSERT( contactList.set( 10, UtlString("sip:mod401@192.168.0.102:5060"), *this ) == false ); 

      CPPUNIT_ASSERT( contactList.get( 0, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:401@192.168.0.102:5060", tmpString.data() );
      CPPUNIT_ASSERT( contactList.set( 0, UtlString("sip:mod401@192.168.0.102:5060"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.get( 0, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:mod401@192.168.0.102:5060", tmpString.data() );
      CPPUNIT_ASSERT( contactList.entries() == 10);

      CPPUNIT_ASSERT( contactList.get( 1, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:402@192.168.0.102:5061", tmpString.data() );
      CPPUNIT_ASSERT( contactList.set( 1, UtlString("sip:mod402@192.168.0.102:5061"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.get( 1, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:mod402@192.168.0.102:5061", tmpString.data() );
      CPPUNIT_ASSERT( contactList.entries() == 10);

      CPPUNIT_ASSERT( contactList.get( 2, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:403@192.168.0.102:5062", tmpString.data() );
      CPPUNIT_ASSERT( contactList.set( 2, UtlString("sip:mod403@192.168.0.102:5062"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.get( 2, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:mod403@192.168.0.102:5062", tmpString.data() );
      CPPUNIT_ASSERT( contactList.entries() == 10);

      CPPUNIT_ASSERT( contactList.get( 3, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:404@192.168.0.102:5063", tmpString.data() );
      CPPUNIT_ASSERT( contactList.set( 3, UtlString("sip:mod404@192.168.0.102:5063"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.get( 3, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:mod404@192.168.0.102:5063", tmpString.data() );
      CPPUNIT_ASSERT( contactList.entries() == 10);

      CPPUNIT_ASSERT( contactList.get( 4, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:405@192.168.0.102:5064", tmpString.data() );
      CPPUNIT_ASSERT( contactList.set( 4, UtlString("sip:mod405@192.168.0.102:5064"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.get( 4, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:mod405@192.168.0.102:5064", tmpString.data() );
      CPPUNIT_ASSERT( contactList.entries() == 10);

      CPPUNIT_ASSERT( contactList.get( 5, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:406@192.168.0.102:5065", tmpString.data() );
      CPPUNIT_ASSERT( contactList.set( 5, UtlString("sip:mod406@192.168.0.102:5065"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.get( 5, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:mod406@192.168.0.102:5065", tmpString.data() );
      CPPUNIT_ASSERT( contactList.entries() == 10);

      CPPUNIT_ASSERT( contactList.get( 6, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:407@192.168.0.102:5066", tmpString.data() );
      CPPUNIT_ASSERT( contactList.set( 6, UtlString("sip:mod407@192.168.0.102:5066"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.get( 6, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:mod407@192.168.0.102:5066", tmpString.data() );
      CPPUNIT_ASSERT( contactList.entries() == 10);

      CPPUNIT_ASSERT( contactList.get( 7, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:408@192.168.0.102:5067", tmpString.data() );
      CPPUNIT_ASSERT( contactList.set( 7, UtlString("sip:mod408@192.168.0.102:5067"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.get( 7, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:mod408@192.168.0.102:5067", tmpString.data() );
      CPPUNIT_ASSERT( contactList.entries() == 10);

      CPPUNIT_ASSERT( contactList.get( 8, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:409@192.168.0.102:5068", tmpString.data() );
      CPPUNIT_ASSERT( contactList.set( 8, UtlString("sip:mod409@192.168.0.102:5068"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.get( 8, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:mod409@192.168.0.102:5068", tmpString.data() );
      CPPUNIT_ASSERT( contactList.entries() == 10);

      CPPUNIT_ASSERT( contactList.get( 9, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:410@192.168.0.102:5069", tmpString.data() );
      CPPUNIT_ASSERT( contactList.set( 9, UtlString("sip:mod410@192.168.0.102:5069"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.get( 9, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:mod410@192.168.0.102:5069", tmpString.data() );
      CPPUNIT_ASSERT( contactList.entries() == 10);
   }

   void GetSetUrlTest()
   {
      UtlString tmpString;
      
      ContactList contactList("sip:dontdowhat@jimmydont.does");

      CPPUNIT_ASSERT( contactList.entries() == 0 );
      CPPUNIT_ASSERT( contactList.add( Url("sip:401@192.168.0.102:5060"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:402@192.168.0.102:5061"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:403@192.168.0.102:5062"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:404@192.168.0.102:5063"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:405@192.168.0.102:5064"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:406@192.168.0.102:5065"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:407@192.168.0.102:5066"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:408@192.168.0.102:5067"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:409@192.168.0.102:5068"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:410@192.168.0.102:5069"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.entries() == 10);

      CPPUNIT_ASSERT( contactList.set( 10, Url("sip:mod401@192.168.0.102:5060"), *this ) == false ); 

      CPPUNIT_ASSERT( contactList.get( 0, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:401@192.168.0.102:5060", tmpString.data() );
      CPPUNIT_ASSERT( contactList.set( 0, Url("sip:mod401@192.168.0.102:5060"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.get( 0, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:mod401@192.168.0.102:5060", tmpString.data() );
      CPPUNIT_ASSERT( contactList.entries() == 10);

      CPPUNIT_ASSERT( contactList.get( 1, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:402@192.168.0.102:5061", tmpString.data() );
      CPPUNIT_ASSERT( contactList.set( 1, Url("sip:mod402@192.168.0.102:5061"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.get( 1, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:mod402@192.168.0.102:5061", tmpString.data() );
      CPPUNIT_ASSERT( contactList.entries() == 10);

      CPPUNIT_ASSERT( contactList.get( 2, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:403@192.168.0.102:5062", tmpString.data() );
      CPPUNIT_ASSERT( contactList.set( 2, Url("sip:mod403@192.168.0.102:5062"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.get( 2, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:mod403@192.168.0.102:5062", tmpString.data() );
      CPPUNIT_ASSERT( contactList.entries() == 10);

      CPPUNIT_ASSERT( contactList.get( 3, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:404@192.168.0.102:5063", tmpString.data() );
      CPPUNIT_ASSERT( contactList.set( 3, Url("sip:mod404@192.168.0.102:5063"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.get( 3, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:mod404@192.168.0.102:5063", tmpString.data() );
      CPPUNIT_ASSERT( contactList.entries() == 10);

      CPPUNIT_ASSERT( contactList.get( 4, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:405@192.168.0.102:5064", tmpString.data() );
      CPPUNIT_ASSERT( contactList.set( 4, Url("sip:mod405@192.168.0.102:5064"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.get( 4, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:mod405@192.168.0.102:5064", tmpString.data() );
      CPPUNIT_ASSERT( contactList.entries() == 10);

      CPPUNIT_ASSERT( contactList.get( 5, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:406@192.168.0.102:5065", tmpString.data() );
      CPPUNIT_ASSERT( contactList.set( 5, Url("sip:mod406@192.168.0.102:5065"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.get( 5, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:mod406@192.168.0.102:5065", tmpString.data() );
      CPPUNIT_ASSERT( contactList.entries() == 10);

      CPPUNIT_ASSERT( contactList.get( 6, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:407@192.168.0.102:5066", tmpString.data() );
      CPPUNIT_ASSERT( contactList.set( 6, UtlString("sip:mod407@192.168.0.102:5066"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.get( 6, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:mod407@192.168.0.102:5066", tmpString.data() );
      CPPUNIT_ASSERT( contactList.entries() == 10);

      CPPUNIT_ASSERT( contactList.get( 7, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:408@192.168.0.102:5067", tmpString.data() );
      CPPUNIT_ASSERT( contactList.set( 7, Url("sip:mod408@192.168.0.102:5067"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.get( 7, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:mod408@192.168.0.102:5067", tmpString.data() );
      CPPUNIT_ASSERT( contactList.entries() == 10);

      CPPUNIT_ASSERT( contactList.get( 8, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:409@192.168.0.102:5068", tmpString.data() );
      CPPUNIT_ASSERT( contactList.set( 8, Url("sip:mod409@192.168.0.102:5068"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.get( 8, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:mod409@192.168.0.102:5068", tmpString.data() );
      CPPUNIT_ASSERT( contactList.entries() == 10);

      CPPUNIT_ASSERT( contactList.get( 9, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:410@192.168.0.102:5069", tmpString.data() );
      CPPUNIT_ASSERT( contactList.set( 9, Url("sip:mod410@192.168.0.102:5069"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.get( 9, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:mod410@192.168.0.102:5069", tmpString.data() );
      CPPUNIT_ASSERT( contactList.entries() == 10);
   }

   void RemoveTest()
   {
      UtlString tmpString;
      
      ContactList contactList("sip:dontdowhat@jimmydont.does");

      CPPUNIT_ASSERT( contactList.entries() == 0 );
      CPPUNIT_ASSERT( contactList.add( Url("sip:401@192.168.0.102:5060"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:402@192.168.0.102:5061"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:403@192.168.0.102:5062"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:404@192.168.0.102:5063"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:405@192.168.0.102:5064"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:406@192.168.0.102:5065"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:407@192.168.0.102:5066"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:408@192.168.0.102:5067"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:409@192.168.0.102:5068"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:410@192.168.0.102:5069"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.entries() == 10);

      CPPUNIT_ASSERT( contactList.remove( 10, *this ) == false ); 

      CPPUNIT_ASSERT( contactList.remove( 5, *this ) == true ); 
      CPPUNIT_ASSERT( contactList.entries() == 9 );
      CPPUNIT_ASSERT( contactList.get( 0, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:401@192.168.0.102:5060", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 1, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:402@192.168.0.102:5061", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 2, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:403@192.168.0.102:5062", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 3, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:404@192.168.0.102:5063", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 4, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:405@192.168.0.102:5064", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 5, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:407@192.168.0.102:5066", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 6, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:408@192.168.0.102:5067", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 7, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:409@192.168.0.102:5068", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 8, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:410@192.168.0.102:5069", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 9, tmpString ) == false ); 

      CPPUNIT_ASSERT( contactList.remove( 2, *this ) == true ); 
      CPPUNIT_ASSERT( contactList.entries() == 8 );
      CPPUNIT_ASSERT( contactList.get( 0, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:401@192.168.0.102:5060", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 1, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:402@192.168.0.102:5061", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 2, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:404@192.168.0.102:5063", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 3, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:405@192.168.0.102:5064", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 4, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:407@192.168.0.102:5066", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 5, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:408@192.168.0.102:5067", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 6, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:409@192.168.0.102:5068", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 7, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:410@192.168.0.102:5069", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 8, tmpString ) == false ); 

      CPPUNIT_ASSERT( contactList.remove( 0, *this ) == true ); 
      CPPUNIT_ASSERT( contactList.entries() == 7 );
      CPPUNIT_ASSERT( contactList.get( 0, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:402@192.168.0.102:5061", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 1, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:404@192.168.0.102:5063", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 2, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:405@192.168.0.102:5064", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 3, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:407@192.168.0.102:5066", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 4, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:408@192.168.0.102:5067", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 5, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:409@192.168.0.102:5068", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 6, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:410@192.168.0.102:5069", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 7, tmpString ) == false ); 
      
      CPPUNIT_ASSERT( contactList.remove( 6, *this ) == true ); 
      CPPUNIT_ASSERT( contactList.entries() == 6 );
      CPPUNIT_ASSERT( contactList.get( 0, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:402@192.168.0.102:5061", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 1, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:404@192.168.0.102:5063", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 2, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:405@192.168.0.102:5064", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 3, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:407@192.168.0.102:5066", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 4, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:408@192.168.0.102:5067", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 5, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:409@192.168.0.102:5068", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 6, tmpString ) == false ); 
      
      CPPUNIT_ASSERT( contactList.remove( 2, *this ) == true ); 
      CPPUNIT_ASSERT( contactList.entries() == 5 );
      CPPUNIT_ASSERT( contactList.get( 0, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:402@192.168.0.102:5061", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 1, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:404@192.168.0.102:5063", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 2, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:407@192.168.0.102:5066", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 3, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:408@192.168.0.102:5067", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 4, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:409@192.168.0.102:5068", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 5, tmpString ) == false ); 
      
      CPPUNIT_ASSERT( contactList.remove( 4, *this ) == true ); 
      CPPUNIT_ASSERT( contactList.entries() == 4 );
      CPPUNIT_ASSERT( contactList.get( 0, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:402@192.168.0.102:5061", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 1, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:404@192.168.0.102:5063", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 2, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:407@192.168.0.102:5066", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 3, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:408@192.168.0.102:5067", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 4, tmpString ) == false ); 

      
      CPPUNIT_ASSERT( contactList.remove( 3, *this ) == true ); 
      CPPUNIT_ASSERT( contactList.entries() == 3 );
      CPPUNIT_ASSERT( contactList.get( 0, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:402@192.168.0.102:5061", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 1, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:404@192.168.0.102:5063", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 2, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:407@192.168.0.102:5066", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 3, tmpString ) == false ); 
      
      CPPUNIT_ASSERT( contactList.remove( 1, *this ) == true ); 
      CPPUNIT_ASSERT( contactList.entries() == 2 );
      CPPUNIT_ASSERT( contactList.get( 0, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:402@192.168.0.102:5061", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 1, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:407@192.168.0.102:5066", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 2, tmpString ) == false ); 

      CPPUNIT_ASSERT( contactList.remove( 0, *this ) == true ); 
      CPPUNIT_ASSERT( contactList.entries() == 1 );
      CPPUNIT_ASSERT( contactList.get( 0, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:407@192.168.0.102:5066", tmpString.data() );
      CPPUNIT_ASSERT( contactList.get( 1, tmpString ) == false ); 
      
      CPPUNIT_ASSERT( contactList.remove( 0, *this ) == true ); 
      CPPUNIT_ASSERT( contactList.entries() == 0 );
      CPPUNIT_ASSERT( contactList.get( 0, tmpString ) == false ); 

      CPPUNIT_ASSERT( contactList.add( Url("sip:401@192.168.0.102:5060"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.entries() == 1 );
      CPPUNIT_ASSERT( contactList.get( 0, tmpString ) == true ); 
      ASSERT_STR_EQUAL( "sip:401@192.168.0.102:5060", tmpString.data() );
   }

   void RemoveAllTest()
   {
      UtlString tmpString;
      
      ContactList contactList("sip:dontdowhat@jimmydont.does");

      CPPUNIT_ASSERT( contactList.removeAll( *this ) == true ); 
      CPPUNIT_ASSERT( contactList.entries() == 0 );

      CPPUNIT_ASSERT( contactList.add( Url("sip:401@192.168.0.102:5060"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:402@192.168.0.102:5061"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:403@192.168.0.102:5062"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:404@192.168.0.102:5063"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:405@192.168.0.102:5064"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:406@192.168.0.102:5065"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:407@192.168.0.102:5066"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:408@192.168.0.102:5067"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:409@192.168.0.102:5068"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.add( Url("sip:410@192.168.0.102:5069"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.entries() == 10);

      CPPUNIT_ASSERT( contactList.removeAll( *this ) == true ); 
      CPPUNIT_ASSERT( contactList.entries() == 0 );
   }
   
   void ModificationTrackingTest()
   {
      UtlString tmpString;
      
      ContactList contactList("sip:dontdowhat@jimmydont.does");

      //remove all on empty list sets the 'modified' flag
      CPPUNIT_ASSERT( contactList.wasListModified() == false );
      CPPUNIT_ASSERT( contactList.removeAll( *this ) == true ); 
      CPPUNIT_ASSERT( contactList.wasListModified() == true );

      // adding an entry sets the 'modified' flag
      contactList.resetWasModifiedFlag();
      CPPUNIT_ASSERT( contactList.wasListModified() == false );
      CPPUNIT_ASSERT( contactList.add( Url("sip:401@192.168.0.102:5060"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.wasListModified() == true );
      
      //remove all on list with content sets the 'modified' flag
      contactList.resetWasModifiedFlag();
      CPPUNIT_ASSERT( contactList.wasListModified() == false );
      CPPUNIT_ASSERT( contactList.removeAll( *this ) == true ); 
      CPPUNIT_ASSERT( contactList.wasListModified() == true );
      CPPUNIT_ASSERT( contactList.entries() == 0 );

      // removing invalid entries does not change the flag
      contactList.resetWasModifiedFlag();
      CPPUNIT_ASSERT( contactList.wasListModified() == false );
      CPPUNIT_ASSERT( contactList.remove( 3, *this ) == false ); 
      CPPUNIT_ASSERT( contactList.wasListModified() == false );
      CPPUNIT_ASSERT( contactList.add( Url("sip:401@192.168.0.102:5060"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.wasListModified() == true );
      CPPUNIT_ASSERT( contactList.remove( 3, *this ) == false ); 
      CPPUNIT_ASSERT( contactList.wasListModified() == true );
      
      // removing value sets the 'modified' flag
      contactList.resetWasModifiedFlag();
      CPPUNIT_ASSERT( contactList.wasListModified() == false );
      CPPUNIT_ASSERT( contactList.remove( 0, *this ) == true ); 
      CPPUNIT_ASSERT( contactList.wasListModified() == true );
      CPPUNIT_ASSERT( contactList.entries() == 0 );
      
      // setting invalid entry does not change the flag
      contactList.resetWasModifiedFlag();
      CPPUNIT_ASSERT( contactList.set( 6, UtlString("sip:mod407@192.168.0.102:5066"), *this ) == false ); 
      CPPUNIT_ASSERT( contactList.wasListModified() == false );
      CPPUNIT_ASSERT( contactList.add( Url("sip:401@192.168.0.102:5060"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.wasListModified() == true );
      CPPUNIT_ASSERT( contactList.set( 6, UtlString("sip:mod407@192.168.0.102:5066"), *this ) == false ); 
      CPPUNIT_ASSERT( contactList.wasListModified() == true );

      // setting value  sets the 'modified' flag
      contactList.resetWasModifiedFlag();
      CPPUNIT_ASSERT( contactList.wasListModified() == false );
      CPPUNIT_ASSERT( contactList.set( 0, UtlString("sip:mod407@192.168.0.102:5066"), *this ) == true ); 
      CPPUNIT_ASSERT( contactList.wasListModified() == true );
      CPPUNIT_ASSERT( contactList.entries() == 1 );
      
      // touching the list sets the 'modified' flag
      contactList.resetWasModifiedFlag();
      CPPUNIT_ASSERT( contactList.wasListModified() == false );
      contactList.touch( *this );
      CPPUNIT_ASSERT( contactList.wasListModified() == true );
      contactList.resetWasModifiedFlag();
      CPPUNIT_ASSERT( contactList.wasListModified() == false );
   }
   //removal of invalid does not set flag
};

CPPUNIT_TEST_SUITE_REGISTRATION(ContactListTest);
