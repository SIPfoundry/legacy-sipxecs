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
#include <net/SignedUrl.h>

/**
 * Unittest for SignedUrl
 */
class SignedUrlTest : public CppUnit::TestCase
{
      CPPUNIT_TEST_SUITE(SignedUrlTest);
      CPPUNIT_TEST(testSignAndValidateUrl);
      CPPUNIT_TEST(testSignAndValidateUrlWithBrackets);
      CPPUNIT_TEST(testSignAndValidateUrlNoUserInfo);
      CPPUNIT_TEST(testValidateBadlySignedUrl);
      CPPUNIT_TEST(testValidateUnsignedUrl);
      CPPUNIT_TEST(testValidateTamperedUrl);
      CPPUNIT_TEST(testSignatureUniqueness);
      CPPUNIT_TEST(testEquivalentUrlSignatures);
      CPPUNIT_TEST_SUITE_END();

      public:

   void testSignAndValidateUrl()
   {
      SignedUrl::setSecret("1234567890ABCDEF");
      Url testUrl( "bob@bobnet.com:5060" );
      SignedUrl::sign(testUrl );
      CPPUNIT_ASSERT( SignedUrl::isUrlSigned( testUrl ) );
   }
   
   void testSignAndValidateUrlWithBrackets()
   {
      SignedUrl::setSecret("1234567890ABCDEF");
      Url testUrl( "<bob@bobnet.com:5060>" );
      SignedUrl::sign(testUrl );
      CPPUNIT_ASSERT( SignedUrl::isUrlSigned( testUrl ) );
   }
   
   void testSignAndValidateUrlNoUserInfo()
   {
      SignedUrl::setSecret("1234567890ABCDEF");
      Url testUrl( "bobnet.com:5060" );
      SignedUrl::sign(testUrl );
      CPPUNIT_ASSERT( SignedUrl::isUrlSigned( testUrl ) );
   }
   

   void testValidateUnsignedUrl()
   {
      Url testUrl( "bobnet.com:5060" );
      CPPUNIT_ASSERT( SignedUrl::isUrlSigned( testUrl ) == FALSE );
   }
   
   void testValidateBadlySignedUrl()
   {
      SignedUrl::setSecret("1234567890ABCDEF");
      Url testUrl( "bobnet.com:5060" );
      SignedUrl::sign(testUrl );
      SignedUrl::setSecret("0000000000000000");
      CPPUNIT_ASSERT( SignedUrl::isUrlSigned( testUrl ) == FALSE );
      
   }
   
   void testValidateTamperedUrl()
   {
      SignedUrl::setSecret("1234567890ABCDEF");
      Url testUrl( "bobnet.com:5060" );
      SignedUrl::sign(testUrl );

      // tamper with URI before verifying sigfnature
      testUrl.setHostPort(5061);      
      CPPUNIT_ASSERT( SignedUrl::isUrlSigned( testUrl ) == FALSE );
   }
   
   void testSignatureUniqueness()
   {
      unsigned int index, subIndex;
      // verify thatURLs with slight variations have different signatures
      SignedUrl::setSecret("1234567890ABCDEF");

      
      Url testUrlArray[] = { "bobnet.com:5060",
                             "babnet.com:5060",
                             "bobnet.com:5061",
                             "bob@bobnet.com:5060",
                             "bab@bobnet.com:5060",
                             "bob@babnet.com:5060",
                             "bob@babnet.com:5061" };
      
      // sign URLs
      for( index = 0; index < sizeof(testUrlArray)/sizeof(testUrlArray[0]); index++ )
      {
         SignedUrl::sign( testUrlArray[index] );
      }

      // extract signatures
      UtlString signatureArray[7];
      for( index = 0; index < sizeof(testUrlArray)/sizeof(testUrlArray[0]); index++ )
      {
         testUrlArray[index].getUrlParameter( "sipX-sig", signatureArray[index] );         
      }
      
      // verify that each signature is different
      for( index = 0; index < sizeof(testUrlArray)/sizeof(testUrlArray[0]); index++ )
      {
         for( subIndex = 0; subIndex < sizeof(testUrlArray)/sizeof(testUrlArray[0]); subIndex++ )
         {
            if( index != subIndex ) // do not compare against self
            {
               CPPUNIT_ASSERT( signatureArray[index].compareTo( signatureArray[subIndex] ) != 0 );
            }
         }
      }
   }
   
   void testEquivalentUrlSignatures()
   {
      unsigned int index;
      // verify thatURLs with slight variations have different signatures
      SignedUrl::setSecret("1234567890ABCDEF");

      Url testUrlArray[] = { "bobnet.com:5060",
                             "sip:bobnet.com:5060",
                             "<bobnet.com:5060>",
                             "<sip:bobnet.com:5060>",
                             "<bobnet.com:5060;lr>",
                             "<bobnet.com:5060;lr;a;b;c;d;e;f;g=10>",
                             "\"bob\" <bobnet.com:5060>",
                             "<bobnet.com:5060;lr;a;b;c;d;e;f;g=10>",
                             "<bobnet.com:5060;lr;a;b;c;d;e;f;g=10>",
                             "<bobnet.com:5060>;somefieldparam=TRUE" };            
      
      // sign URLs
      for( index = 0; index < sizeof(testUrlArray)/sizeof(testUrlArray[0]); index++ )
      {
         SignedUrl::sign( testUrlArray[index] );
      }
      
      //make sure all signatures are equivalent
      UtlString referenceSignature;
      testUrlArray[0].getUrlParameter( "sipX-sig", referenceSignature );
      
            
      // extract signatures
      for( index = 0; index < sizeof(testUrlArray)/sizeof(testUrlArray[0]); index++ )
      {
         UtlString otherSignature;
         testUrlArray[index].getUrlParameter( "sipX-sig", otherSignature );
         CPPUNIT_ASSERT( otherSignature.compareTo( referenceSignature ) == 0 );
      }
   }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SignedUrlTest);
