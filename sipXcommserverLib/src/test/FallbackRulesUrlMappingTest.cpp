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

#include "net/Url.h"
#include "digitmaps/FallbackRulesUrlMapping.h"
#include "sipdb/ResultSet.h"

#include "testlib/FileTestContext.h"
#include "net/SipMessage.h"

class FallbackRulesUrlMappingTest : public CppUnit::TestCase
{
      CPPUNIT_TEST_SUITE(FallbackRulesUrlMappingTest);
      CPPUNIT_TEST(testAllEmergencyPermutations);
      CPPUNIT_TEST(testAll800Permutations);
      CPPUNIT_TEST(testAllLongDistancePermutations);
      CPPUNIT_TEST(testAllInternationalPermutations);
      CPPUNIT_TEST(testRouteHeaderPermutations);
      CPPUNIT_TEST(testAllNoDefaultPermutations);
      CPPUNIT_TEST(testAllNonMatchingDialstrings);
      CPPUNIT_TEST_SUITE_END();


      public:
      void setUp()
      {
         mFileTestContext = new FileTestContext(TEST_DATA_DIR "/mapdata", TEST_WORK_DIR "/mapdata");
      }
            
      void tearDown()
      {
         delete mFileTestContext;
      }
   
      void testAllEmergencyPermutations()
      {
         FallbackRulesUrlMapping* urlmap;
         ResultSet registrations;
         UtlString actual;
         UtlString callTag = "UNK";

         CPPUNIT_ASSERT( urlmap = new FallbackRulesUrlMapping() );
         UtlString simpleXml;
         mFileTestContext->inputFilePath("fallbackrules.xml", simpleXml);
         CPPUNIT_ASSERT( urlmap->loadMappings(simpleXml.data() ) == OS_SUCCESS );

         // permutations for the 'boston' location
         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:sos@example.edu")
                                ,UtlString("boston"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:911@pstn-boston-tw.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:911@example.edu")
                                ,UtlString("boston"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:911@pstn-boston-tw.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:9911@example.edu")
                                ,UtlString("boston"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:911@pstn-boston-tw.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:sos@sipx.example.edu")
                                ,UtlString("boston"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:911@pstn-boston-tw.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:911@sipx.example.edu")
                                ,UtlString("boston"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:911@pstn-boston-tw.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:9911@sipx.example.edu")
                                ,UtlString("boston"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:911@pstn-boston-tw.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:sos@10.1.20.20")
                                ,UtlString("boston"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:911@pstn-boston-tw.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:911@10.1.20.20")
                                ,UtlString("boston"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:911@pstn-boston-tw.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:9911@10.1.20.20")
                                ,UtlString("boston"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:911@pstn-boston-tw.example.edu",actual);
         registrations.destroyAll();

         // permutation for the 'seattle' location
         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:sos@example.edu")
                                ,UtlString("seattle"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:911@pstn-seattle-tw.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:911@example.edu")
                                ,UtlString("seattle"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:911@pstn-seattle-tw.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:9911@example.edu")
                                ,UtlString("seattle"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:911@pstn-seattle-tw.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:sos@sipx.example.edu")
                                ,UtlString("seattle"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:911@pstn-seattle-tw.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:911@sipx.example.edu")
                                ,UtlString("seattle"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:911@pstn-seattle-tw.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:9911@sipx.example.edu")
                                ,UtlString("seattle"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:911@pstn-seattle-tw.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:sos@10.1.20.20")
                                ,UtlString("seattle"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:911@pstn-seattle-tw.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:911@10.1.20.20")
                                ,UtlString("seattle"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:911@pstn-seattle-tw.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:9911@10.1.20.20")
                                ,UtlString("seattle"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:911@pstn-seattle-tw.example.edu",actual);
         registrations.destroyAll();
         
         // permutation for the default location
         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:sos@example.edu")
                                ,UtlString("Kookamonga"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:911@pstn-gw.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:911@example.edu")
                                ,UtlString("Kookamonga"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:911@pstn-gw.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:9911@example.edu")
                                ,UtlString("Kookamonga"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:911@pstn-gw.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:sos@sipx.example.edu")
                                ,UtlString("Kookamonga"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:911@pstn-gw.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:911@sipx.example.edu")
                                ,UtlString("Kookamonga"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:911@pstn-gw.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:9911@sipx.example.edu")
                                ,UtlString("Kookamonga"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:911@pstn-gw.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:sos@10.1.20.20")
                                ,UtlString("Kookamonga"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:911@pstn-gw.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:911@10.1.20.20")
                                ,UtlString("Kookamonga"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:911@pstn-gw.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:9911@10.1.20.20")
                                ,UtlString("Kookamonga"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:911@pstn-gw.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:9911@10.1.20.20")
                                ,UtlString(""),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:911@pstn-gw.example.edu",actual);
         registrations.destroyAll();
         
         delete urlmap;
      }

      void testAll800Permutations()
      {
         FallbackRulesUrlMapping* urlmap;
         ResultSet registrations;
         UtlString actual;
         UtlString callTag = "UNK";

         CPPUNIT_ASSERT( urlmap = new FallbackRulesUrlMapping() );
         UtlString simpleXml;
         mFileTestContext->inputFilePath("fallbackrules.xml", simpleXml);
         CPPUNIT_ASSERT( urlmap->loadMappings(simpleXml.data() ) == OS_SUCCESS );

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:918005551212@example.edu")
                                ,UtlString("boston"),
                                registrations, callTag
                                ) == OS_SUCCESS );

         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:8005551212@pstn-gw2.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:18005551213@example.edu")
                                ,UtlString("regina"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:8005551213@pstn-gw2.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:8005551214@example.edu")
                                ,UtlString(""),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:8005551214@pstn-gw2.example.edu",actual);
         registrations.destroyAll();
         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:918005551212@sipx.example.edu")
                                ,UtlString("new-york"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:8005551212@pstn-gw2.example.edu",actual);
         registrations.destroyAll();
         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:18005551213@sipx.example.edu")
                                ,UtlString("DC"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:8005551213@pstn-gw2.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:8005551214@sipx.example.edu")
                                ,UtlString("Philly"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:8005551214@pstn-gw2.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:918005551212@10.1.20.20")
                                ,UtlString(""),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:8005551212@pstn-gw2.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:18005551213@10.1.20.20")
                                ,UtlString("miami"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:8005551213@pstn-gw2.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:8005551214@10.1.20.20")
                                ,UtlString("orlando"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:8005551214@pstn-gw2.example.edu",actual);
         registrations.destroyAll();

         delete urlmap;
      }
      
      void testAllLongDistancePermutations()
      {
         FallbackRulesUrlMapping* urlmap;
         ResultSet registrations;
         UtlString actual;
         UtlString callTag = "UNK";

         CPPUNIT_ASSERT( urlmap = new FallbackRulesUrlMapping() );
         UtlString simpleXml;
         mFileTestContext->inputFilePath("fallbackrules.xml", simpleXml);
         CPPUNIT_ASSERT( urlmap->loadMappings(simpleXml.data() ) == OS_SUCCESS );

         // permutations for the 'boston' location
         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:912335551212@example.edu")
                                ,UtlString("boston"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 2 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("<sip:2335551212@pstn-boston-tw.example.edu>;q=0.9",actual);
         getResult( registrations, 1, "contact", actual);
         ASSERT_STR_EQUAL("<sip:662335551212@pstn-gw.example.edu>;q=0.8",actual);
         registrations.destroyAll();
         
         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:12335551212@example.edu")
                                ,UtlString("boston"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 2 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("<sip:2335551212@pstn-boston-tw.example.edu>;q=0.9",actual);
         getResult( registrations, 1, "contact", actual);
         ASSERT_STR_EQUAL("<sip:662335551212@pstn-gw.example.edu>;q=0.8",actual);
         registrations.destroyAll();
         
         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:2335551212@example.edu")
                                ,UtlString("boston"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 2 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("<sip:2335551212@pstn-boston-tw.example.edu>;q=0.9",actual);
         getResult( registrations, 1, "contact", actual);
         ASSERT_STR_EQUAL("<sip:662335551212@pstn-gw.example.edu>;q=0.8",actual);
         registrations.destroyAll();

         // permutations for the 'seattle' location
         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:912335551212@example.edu")
                                ,UtlString("seattle"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 2 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("<sip:2335551212@pstn-seattle-tw.example.edu>;q=0.9",actual);
         getResult( registrations, 1, "contact", actual);
         ASSERT_STR_EQUAL("<sip:2335551212@pstn-gw.example.edu>;q=0.7",actual);
         registrations.destroyAll();
         
         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:12335551212@example.edu")
                                ,UtlString("seattle"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 2 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("<sip:2335551212@pstn-seattle-tw.example.edu>;q=0.9",actual);
         getResult( registrations, 1, "contact", actual);
         ASSERT_STR_EQUAL("<sip:2335551212@pstn-gw.example.edu>;q=0.7",actual);
         registrations.destroyAll();
         
         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:2335551212@example.edu")
                                ,UtlString("seattle"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 2 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("<sip:2335551212@pstn-seattle-tw.example.edu>;q=0.9",actual);
         getResult( registrations, 1, "contact", actual);
         ASSERT_STR_EQUAL("<sip:2335551212@pstn-gw.example.edu>;q=0.7",actual);
         registrations.destroyAll();

         // permutations for the other locations
         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:912335551212@example.edu")
                                ,UtlString("walla-walla"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:992335551212@pstn-gw2.example.edu",actual);
         registrations.destroyAll();
         
         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:12335551212@example.edu")
                                ,UtlString(""),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:992335551212@pstn-gw2.example.edu",actual);
         registrations.destroyAll();
         
         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:2335551212@example.edu")
                                ,UtlString("ogden"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:992335551212@pstn-gw2.example.edu",actual);
         registrations.destroyAll();

         delete urlmap;
      }
      
      void testAllInternationalPermutations()
      {
         FallbackRulesUrlMapping* urlmap;
         ResultSet registrations;
         UtlString actual;
         UtlString callTag = "UNK";

         CPPUNIT_ASSERT( urlmap = new FallbackRulesUrlMapping() );
         UtlString simpleXml;
         mFileTestContext->inputFilePath("fallbackrules.xml", simpleXml);
         CPPUNIT_ASSERT( urlmap->loadMappings(simpleXml.data() ) == OS_SUCCESS );

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:011336135551212@example.edu")
                                ,UtlString("Boston"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:011336135551212@pstn-gw3.example.edu",actual);
         registrations.destroyAll();
         
         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:01133613555121211@example.edu")
                                ,UtlString(""),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:01133613555121211@pstn-gw3.example.edu",actual);
         registrations.destroyAll();
         
         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:01133613555121212@example.edu")
                                ,UtlString("ogden"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:01133613555121212@pstn-gw3.example.edu",actual);
         registrations.destroyAll();
         
         delete urlmap;
      }
      
      void testRouteHeaderPermutations()
      {
         FallbackRulesUrlMapping* urlmap;
         ResultSet registrations;
         UtlString actual;
         UtlString callTag = "UNK";

         CPPUNIT_ASSERT( urlmap = new FallbackRulesUrlMapping() );
         UtlString simpleXml;
         mFileTestContext->inputFilePath("fallbackrules.xml", simpleXml);
         CPPUNIT_ASSERT( urlmap->loadMappings(simpleXml.data() ) == OS_SUCCESS );

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:75551212@example.org")
                                ,UtlString("ogden"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 1 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("<sip:75551212@example.org?route=sbc.example.edu>",actual);
         registrations.destroyAll();
         
         delete urlmap;
      }
      
      void testAllNoDefaultPermutations()
      {
         FallbackRulesUrlMapping* urlmap;
         ResultSet registrations;
         UtlString actual;
         UtlString callTag = "UNK";

         CPPUNIT_ASSERT( urlmap = new FallbackRulesUrlMapping() );
         UtlString simpleXml;
         mFileTestContext->inputFilePath("fallbackrules.xml", simpleXml);
         CPPUNIT_ASSERT( urlmap->loadMappings(simpleXml.data() ) == OS_SUCCESS );

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:99991234@example.org")
                                ,UtlString("boston"),
                                registrations, callTag
                                ) == OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 2 ,  registrations.getSize() );
         getResult( registrations, 0, "contact", actual);
         ASSERT_STR_EQUAL("sip:1234@pstn-boston-tw2.example.edu",actual);
         getResult( registrations, 1, "contact", actual);
         ASSERT_STR_EQUAL("sip:661234@pstn-gw4.example.edu",actual);
         registrations.destroyAll();

         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:99991234@example.org")
                                ,UtlString("salem"),
                                registrations, callTag
                                ) != OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 0 ,  registrations.getSize() );
         
         delete urlmap;
      }
      
      void testAllNonMatchingDialstrings()
      {
         FallbackRulesUrlMapping* urlmap;
         ResultSet registrations;
         UtlString actual;
         UtlString callTag = "UNK";

         CPPUNIT_ASSERT( urlmap = new FallbackRulesUrlMapping() );
         UtlString simpleXml;
         mFileTestContext->inputFilePath("fallbackrules.xml", simpleXml);
         CPPUNIT_ASSERT( urlmap->loadMappings(simpleXml.data() ) == OS_SUCCESS );

         // use non-matching host name
         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:99991234@example.net")
                                ,UtlString("boston"),
                                registrations, callTag
                                ) != OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 0 ,  registrations.getSize() );
         registrations.destroyAll();

         // use non-matching user name
         CPPUNIT_ASSERT( urlmap->getContactList( Url("sip:71234@example.com")
                                ,UtlString("boston"),
                                registrations, callTag
                                ) != OS_SUCCESS );
         CPPUNIT_ASSERT_EQUAL( 0 ,  registrations.getSize() );
         registrations.destroyAll();

         delete urlmap;         
      }
      protected:

      void getResult( ResultSet& resultSet
                     ,int         index
                     ,const char* key
                     ,UtlString&  result
                     )
      {
         UtlHashMap hash;
         resultSet.getIndex( index, hash );
         UtlString theKey(key);
         result = *((UtlString*)hash.findValue(&theKey));
      }

      bool loadUrlMap( FallbackRulesUrlMapping& urlmap, const char* mapfile )
      {
         return (   urlmap.loadMappings(mapfile )
                 == OS_SUCCESS
                 );
      };

   FileTestContext* mFileTestContext;
   
};

CPPUNIT_TEST_SUITE_REGISTRATION(FallbackRulesUrlMappingTest);

