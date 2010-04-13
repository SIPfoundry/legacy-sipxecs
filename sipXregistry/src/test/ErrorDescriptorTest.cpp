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

using namespace std;

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

class ErrorDescriptorTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(ErrorDescriptorTest);
   CPPUNIT_TEST( SetAndGetStatusLineDataTest );
   CPPUNIT_TEST( SetAndGetWarningDataTest );
   CPPUNIT_TEST( SetAndGetAppendRequestTest );
   CPPUNIT_TEST( SetAndGetOptionalFieldsTest );
   CPPUNIT_TEST_SUITE_END();

public:
   void setUp()
   {
   }

   void SetAndGetStatusLineDataTest()
   {
      ErrorDescriptor errorDescriptor;
      UtlString text;
      int      code;
      
      // test default values
      errorDescriptor.getStatusLineData( code, text );
      CPPUNIT_ASSERT( code == SIP_FORBIDDEN_CODE );
      ASSERT_STR_EQUAL( SIP_FORBIDDEN_TEXT, text.data() );
      
      // check status code boundary conditions
      CPPUNIT_ASSERT( !errorDescriptor.setStatusLineData( 399, "bogus text" ) );
      CPPUNIT_ASSERT(  errorDescriptor.setStatusLineData( 400, "bogus text" ) );
      CPPUNIT_ASSERT(  errorDescriptor.setStatusLineData( 699, "bogus text" ) );
      CPPUNIT_ASSERT( !errorDescriptor.setStatusLineData( 700, "bogus text" ) );

      // write/read-back test
      CPPUNIT_ASSERT(  errorDescriptor.setStatusLineData( 650, "there goes some error!" ) );
      errorDescriptor.getStatusLineData( code, text );
      CPPUNIT_ASSERT( code == 650 );
      ASSERT_STR_EQUAL( "there goes some error!", text.data() );
   }
   
   void SetAndGetWarningDataTest()
   {
      ErrorDescriptor errorDescriptor;
      UtlString text;
      int      code;
      
      // test default values
      CPPUNIT_ASSERT( !errorDescriptor.getWarningData( code, text ) );
      CPPUNIT_ASSERT( !errorDescriptor.isWarningDataSet() );
      
      // check status code boundary conditions
      CPPUNIT_ASSERT( !errorDescriptor.setWarningData( 299, "bogus text" ) );
      CPPUNIT_ASSERT(  errorDescriptor.setWarningData( 300, "bogus text" ) );
      CPPUNIT_ASSERT(  errorDescriptor.setWarningData( 399, "bogus text" ) );
      CPPUNIT_ASSERT( !errorDescriptor.setWarningData( 400, "bogus text" ) );

      // write/read-back test
      CPPUNIT_ASSERT(  errorDescriptor.setWarningData( 350, "you have been warned!" ) );
      CPPUNIT_ASSERT( errorDescriptor.getWarningData( code, text ) );
      CPPUNIT_ASSERT( code == 350 );
      ASSERT_STR_EQUAL( "you have been warned!", text.data() );
      CPPUNIT_ASSERT( errorDescriptor.isWarningDataSet() );      
   }

   void SetAndGetAppendRequestTest()
   {
      ErrorDescriptor errorDescriptor;
      
      // test default values
      CPPUNIT_ASSERT( errorDescriptor.shouldRequestBeAppendedToResponse() == false ); 
      
      // write/read-back test
      errorDescriptor.appendRequestToResponse();
      CPPUNIT_ASSERT( errorDescriptor.shouldRequestBeAppendedToResponse() == true ); 
      errorDescriptor.dontAppendRequestToResponse();
      CPPUNIT_ASSERT( errorDescriptor.shouldRequestBeAppendedToResponse() == false ); 
      errorDescriptor.appendRequestToResponse();
      CPPUNIT_ASSERT( errorDescriptor.shouldRequestBeAppendedToResponse() == true ); 
   }
   
   void SetAndGetOptionalFieldsTest()
   {
      ErrorDescriptor errorDescriptor;
      UtlString fieldValue;
      
      // test default values
      CPPUNIT_ASSERT( errorDescriptor.getAcceptFieldValue( fieldValue )         == false ); 
      CPPUNIT_ASSERT( errorDescriptor.getAcceptEncodingFieldValue( fieldValue ) == false ); 
      CPPUNIT_ASSERT( errorDescriptor.getAcceptLanguageFieldValue( fieldValue ) == false ); 
      CPPUNIT_ASSERT( errorDescriptor.getAllowFieldValue( fieldValue )          == false ); 
      CPPUNIT_ASSERT( errorDescriptor.getRequireFieldValue( fieldValue )        == false ); 
      CPPUNIT_ASSERT( errorDescriptor.getRetryAfterFieldValue( fieldValue )     == false ); 
      CPPUNIT_ASSERT( errorDescriptor.getUnsupportedFieldValue( fieldValue )    == false ); 
      
      // write/read-back test
      errorDescriptor.setAcceptFieldValue( "truth" );
      CPPUNIT_ASSERT( errorDescriptor.getAcceptFieldValue( fieldValue )         == true ); 
      ASSERT_STR_EQUAL( "truth", fieldValue.data() );
      CPPUNIT_ASSERT( errorDescriptor.getAcceptEncodingFieldValue( fieldValue ) == false ); 
      CPPUNIT_ASSERT( errorDescriptor.getAcceptLanguageFieldValue( fieldValue ) == false ); 
      CPPUNIT_ASSERT( errorDescriptor.getAllowFieldValue( fieldValue )          == false ); 
      CPPUNIT_ASSERT( errorDescriptor.getRequireFieldValue( fieldValue )        == false ); 
      CPPUNIT_ASSERT( errorDescriptor.getRetryAfterFieldValue( fieldValue )     == false ); 
      CPPUNIT_ASSERT( errorDescriptor.getUnsupportedFieldValue( fieldValue )    == false ); 
      
      errorDescriptor.setAcceptEncodingFieldValue( "scrambled eggs");
      CPPUNIT_ASSERT( errorDescriptor.getAcceptFieldValue( fieldValue )         == true ); 
      ASSERT_STR_EQUAL( "truth", fieldValue.data() );
      CPPUNIT_ASSERT( errorDescriptor.getAcceptEncodingFieldValue( fieldValue ) == true ); 
      ASSERT_STR_EQUAL( "scrambled eggs", fieldValue.data() );
      CPPUNIT_ASSERT( errorDescriptor.getAcceptLanguageFieldValue( fieldValue ) == false ); 
      CPPUNIT_ASSERT( errorDescriptor.getAllowFieldValue( fieldValue )          == false ); 
      CPPUNIT_ASSERT( errorDescriptor.getRequireFieldValue( fieldValue )        == false ); 
      CPPUNIT_ASSERT( errorDescriptor.getRetryAfterFieldValue( fieldValue )     == false ); 
      CPPUNIT_ASSERT( errorDescriptor.getUnsupportedFieldValue( fieldValue )    == false ); 
      
      errorDescriptor.setAcceptLanguageFieldValue( "French" );
      CPPUNIT_ASSERT( errorDescriptor.getAcceptFieldValue( fieldValue )         == true ); 
      ASSERT_STR_EQUAL( "truth", fieldValue.data() );
      CPPUNIT_ASSERT( errorDescriptor.getAcceptEncodingFieldValue( fieldValue ) == true ); 
      ASSERT_STR_EQUAL( "scrambled eggs", fieldValue.data() );
      CPPUNIT_ASSERT( errorDescriptor.getAcceptLanguageFieldValue( fieldValue ) == true ); 
      ASSERT_STR_EQUAL( "French", fieldValue.data() );
      CPPUNIT_ASSERT( errorDescriptor.getAllowFieldValue( fieldValue )          == false ); 
      CPPUNIT_ASSERT( errorDescriptor.getRequireFieldValue( fieldValue )        == false ); 
      CPPUNIT_ASSERT( errorDescriptor.getRetryAfterFieldValue( fieldValue )     == false ); 
      CPPUNIT_ASSERT( errorDescriptor.getUnsupportedFieldValue( fieldValue )    == false ); 
      
      errorDescriptor.setAllowFieldValue( "me" );
      CPPUNIT_ASSERT( errorDescriptor.getAcceptFieldValue( fieldValue )         == true ); 
      ASSERT_STR_EQUAL( "truth", fieldValue.data() );
      CPPUNIT_ASSERT( errorDescriptor.getAcceptEncodingFieldValue( fieldValue ) == true ); 
      ASSERT_STR_EQUAL( "scrambled eggs", fieldValue.data() );
      CPPUNIT_ASSERT( errorDescriptor.getAcceptLanguageFieldValue( fieldValue ) == true ); 
      ASSERT_STR_EQUAL( "French", fieldValue.data() );
      CPPUNIT_ASSERT( errorDescriptor.getAllowFieldValue( fieldValue )          == true ); 
      ASSERT_STR_EQUAL( "me", fieldValue.data() );
      CPPUNIT_ASSERT( errorDescriptor.getRequireFieldValue( fieldValue )        == false ); 
      CPPUNIT_ASSERT( errorDescriptor.getRetryAfterFieldValue( fieldValue )     == false ); 
      CPPUNIT_ASSERT( errorDescriptor.getUnsupportedFieldValue( fieldValue )    == false ); 
      
      errorDescriptor.setRequireFieldValue( "precision" );
      CPPUNIT_ASSERT( errorDescriptor.getAcceptFieldValue( fieldValue )         == true ); 
      ASSERT_STR_EQUAL( "truth", fieldValue.data() );
      CPPUNIT_ASSERT( errorDescriptor.getAcceptEncodingFieldValue( fieldValue ) == true ); 
      ASSERT_STR_EQUAL( "scrambled eggs", fieldValue.data() );
      CPPUNIT_ASSERT( errorDescriptor.getAcceptLanguageFieldValue( fieldValue ) == true ); 
      ASSERT_STR_EQUAL( "French", fieldValue.data() );
      CPPUNIT_ASSERT( errorDescriptor.getAllowFieldValue( fieldValue )          == true ); 
      ASSERT_STR_EQUAL( "me", fieldValue.data() );
      CPPUNIT_ASSERT( errorDescriptor.getRequireFieldValue( fieldValue )        == true ); 
      ASSERT_STR_EQUAL( "precision", fieldValue.data() );
      CPPUNIT_ASSERT( errorDescriptor.getRetryAfterFieldValue( fieldValue )     == false ); 
      CPPUNIT_ASSERT( errorDescriptor.getUnsupportedFieldValue( fieldValue )    == false ); 
      
      errorDescriptor.setRetryAfterFieldValue( "tomorrow" );
      CPPUNIT_ASSERT( errorDescriptor.getAcceptFieldValue( fieldValue )         == true ); 
      ASSERT_STR_EQUAL( "truth", fieldValue.data() );
      CPPUNIT_ASSERT( errorDescriptor.getAcceptEncodingFieldValue( fieldValue ) == true ); 
      ASSERT_STR_EQUAL( "scrambled eggs", fieldValue.data() );
      CPPUNIT_ASSERT( errorDescriptor.getAcceptLanguageFieldValue( fieldValue ) == true ); 
      ASSERT_STR_EQUAL( "French", fieldValue.data() );
      CPPUNIT_ASSERT( errorDescriptor.getAllowFieldValue( fieldValue )          == true ); 
      ASSERT_STR_EQUAL( "me", fieldValue.data() );
      CPPUNIT_ASSERT( errorDescriptor.getRequireFieldValue( fieldValue )        == true ); 
      ASSERT_STR_EQUAL( "precision", fieldValue.data() );
      CPPUNIT_ASSERT( errorDescriptor.getRetryAfterFieldValue( fieldValue )     == true ); 
      ASSERT_STR_EQUAL( "tomorrow", fieldValue.data() );
      CPPUNIT_ASSERT( errorDescriptor.getUnsupportedFieldValue( fieldValue )    == false ); 
      
      errorDescriptor.setUnsupportedFieldValue( "feature" );
      CPPUNIT_ASSERT( errorDescriptor.getAcceptFieldValue( fieldValue )         == true ); 
      ASSERT_STR_EQUAL( "truth", fieldValue.data() );
      CPPUNIT_ASSERT( errorDescriptor.getAcceptEncodingFieldValue( fieldValue ) == true ); 
      ASSERT_STR_EQUAL( "scrambled eggs", fieldValue.data() );
      CPPUNIT_ASSERT( errorDescriptor.getAcceptLanguageFieldValue( fieldValue ) == true ); 
      ASSERT_STR_EQUAL( "French", fieldValue.data() );
      CPPUNIT_ASSERT( errorDescriptor.getAllowFieldValue( fieldValue )          == true ); 
      ASSERT_STR_EQUAL( "me", fieldValue.data() );
      CPPUNIT_ASSERT( errorDescriptor.getRequireFieldValue( fieldValue )        == true ); 
      ASSERT_STR_EQUAL( "precision", fieldValue.data() );
      CPPUNIT_ASSERT( errorDescriptor.getRetryAfterFieldValue( fieldValue )     == true ); 
      ASSERT_STR_EQUAL( "tomorrow", fieldValue.data() );
      CPPUNIT_ASSERT( errorDescriptor.getUnsupportedFieldValue( fieldValue )    == true ); 
      ASSERT_STR_EQUAL( "feature", fieldValue.data() );
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(ErrorDescriptorTest);
