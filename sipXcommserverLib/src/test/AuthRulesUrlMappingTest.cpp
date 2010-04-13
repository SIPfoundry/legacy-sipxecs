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
#include "digitmaps/AuthRulesUrlMapping.h"
#include "sipdb/ResultSet.h"

#include "testlib/FileTestContext.h"
#include "net/SipMessage.h"

#define VM "VoIcEmAiL"
#define MS "MeDiAsErVeR"
#define LH "LoCaLhOsT"

class AuthRulesUrlMappingTest : public CppUnit::TestCase
{
      CPPUNIT_TEST_SUITE(AuthRulesUrlMappingTest);
      CPPUNIT_TEST(testAuthRules);
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
   
      void testAuthRules()
      {
         AuthRulesUrlMapping* urlmap;
         ResultSet permissions;
         UtlString actual;

         CPPUNIT_ASSERT( urlmap = new AuthRulesUrlMapping() );
         UtlString simpleXml;
         mFileTestContext->inputFilePath("authrules.xml", simpleXml);
         CPPUNIT_ASSERT( urlmap->loadMappings(simpleXml.data(),
                                              MS, VM, LH
                                              )
                        == OS_SUCCESS
                        );

         // the interface says getContactList returns an OsStatus,
         // but it is not set so don't test it

         urlmap->getPermissionRequired( Url("sip:THISUSER@THISHOST.THISDOMAIN")
                                , permissions
                                );
         CPPUNIT_ASSERT_EQUAL( 4 ,  permissions.getSize() );
         getResult( permissions, 0, "permission", actual);
         ASSERT_STR_EQUAL("steal",actual);
         getResult( permissions, 1, "permission", actual);
         ASSERT_STR_EQUAL("lie",actual);
         getResult( permissions, 2, "permission", actual);
         ASSERT_STR_EQUAL("cheat",actual);
         getResult( permissions, 3, "permission", actual);
         ASSERT_STR_EQUAL("eat like a pig",actual);
         permissions.destroyAll();
         
         urlmap->getPermissionRequired( Url("sip:THISUSER@THISDOMAIN")
                                ,permissions
                                );
         CPPUNIT_ASSERT_EQUAL( 4 ,  permissions.getSize() );
         getResult( permissions, 0, "permission", actual);
         ASSERT_STR_EQUAL("steal",actual);
         getResult( permissions, 1, "permission", actual);
         ASSERT_STR_EQUAL("lie",actual);
         getResult( permissions, 2, "permission", actual);
         ASSERT_STR_EQUAL("cheat",actual);
         getResult( permissions, 3, "permission", actual);
         ASSERT_STR_EQUAL("eat like a pig",actual);
         permissions.destroyAll();

         urlmap->getPermissionRequired( Url("sip:THATUSER@THISHOST.THISDOMAIN")
                                , permissions
                                );
         CPPUNIT_ASSERT_EQUAL( 4 ,  permissions.getSize() );
         getResult( permissions, 0, "permission", actual);
         ASSERT_STR_EQUAL("steal",actual);
         getResult( permissions, 1, "permission", actual);
         ASSERT_STR_EQUAL("lie",actual);
         getResult( permissions, 2, "permission", actual);
         ASSERT_STR_EQUAL("cheat",actual);
         getResult( permissions, 3, "permission", actual);
         ASSERT_STR_EQUAL("eat like a pig",actual);
         permissions.destroyAll();

         urlmap->getPermissionRequired( Url("sip:THATUSER@THISDOMAIN")
                                , permissions
                                );
         CPPUNIT_ASSERT_EQUAL( 4 ,  permissions.getSize() );
         getResult( permissions, 0, "permission", actual);
         ASSERT_STR_EQUAL("steal",actual);
         getResult( permissions, 1, "permission", actual);
         ASSERT_STR_EQUAL("lie",actual);
         getResult( permissions, 2, "permission", actual);
         ASSERT_STR_EQUAL("cheat",actual);
         getResult( permissions, 3, "permission", actual);
         ASSERT_STR_EQUAL("eat like a pig",actual);
         permissions.destroyAll();

         urlmap->getPermissionRequired( Url("sip:OTHERUSER@THISHOST.THIDOMAIN")
                                , permissions
                                );
         CPPUNIT_ASSERT_EQUAL( 0 , permissions.getSize() );

         urlmap->getPermissionRequired( Url("sip:OTHERUSER@THISDOMAIN")
                                , permissions
                                );
         CPPUNIT_ASSERT_EQUAL( 0 , permissions.getSize() );

         urlmap->getPermissionRequired( Url("sip:THISUSER@OTHERHOST.THIDOMAIN")
                                , permissions
                                );
         CPPUNIT_ASSERT_EQUAL( 0 , permissions.getSize() );

         urlmap->getPermissionRequired( Url("sip:THISUSER@OTHERDOMAIN")
                                , permissions
                                );
         CPPUNIT_ASSERT_EQUAL( 0 , permissions.getSize() );

         urlmap->getPermissionRequired( Url("sip:THISUSER@UserChgDOMAIN")
                                , permissions
                                );
         CPPUNIT_ASSERT_EQUAL( 0 , permissions.getSize() );

         urlmap->getPermissionRequired( Url("sip:THATUSER@UserChgDOMAIN")
                                , permissions
                                );
         CPPUNIT_ASSERT_EQUAL( 0 , permissions.getSize() );

         urlmap->getPermissionRequired( Url("sip:OTHERUSER@UserChgDOMAIN")
                                , permissions
                                );
         CPPUNIT_ASSERT_EQUAL( 0 , permissions.getSize() );

         urlmap->getPermissionRequired( Url("sip:THISUSER@HostChgDOMAIN")
                                , permissions
                                );
         CPPUNIT_ASSERT_EQUAL( 0 , permissions.getSize() );

         urlmap->getPermissionRequired( Url("sip:THATUSER@HostChgDOMAIN")
                                , permissions
                                );
         CPPUNIT_ASSERT_EQUAL( 0 , permissions.getSize() );

         // do the domain transformtion again and check that the transport is removed [XRR-114]
         urlmap->getPermissionRequired( Url("<sip:THATUSER@HostChgDOMAIN;transport=xyz>")
                                , permissions
                                );
         CPPUNIT_ASSERT_EQUAL( 0, permissions.getSize() );

         urlmap->getPermissionRequired( Url("sip:OTHERUSER@UserChgDOMAIN")
                                , permissions
                                );
         CPPUNIT_ASSERT_EQUAL( 0 ,  permissions.getSize() );

         // Check the hostPattern format='url'
         urlmap->getPermissionRequired( Url("sip:PortUser@example.com:4242")
                                , permissions
                                );
         CPPUNIT_ASSERT_EQUAL( 1 ,  permissions.getSize() );
         getResult( permissions, 0, "permission", actual);
         ASSERT_STR_EQUAL( "NoAccess", actual );
         permissions.destroyAll();

         // Check the hostPattern format='DnsWildcard'
         urlmap->getPermissionRequired( Url("sip:DnsUser@a.b.c.d.e.f.example.com")
                                , permissions
                                );
         CPPUNIT_ASSERT_EQUAL( 1 ,  permissions.getSize() );
         getResult( permissions, 0, "permission", actual);
         ASSERT_STR_EQUAL( "NoAccess", actual );
         permissions.destroyAll();

         // Check the hostPattern format='IPv4subnet'
         urlmap->getPermissionRequired( Url("sip:SubnetUser@192.168.1.1")
                                , permissions
                                );
         CPPUNIT_ASSERT_EQUAL( 1 ,  permissions.getSize() );
         getResult( permissions, 0, "permission", actual);
         ASSERT_STR_EQUAL( "NoAccess", actual );
         permissions.destroyAll();

         // Check the hostPattern format='IPv4subnet' (nomatch)
         urlmap->getPermissionRequired( Url("sip:SubnetUser@192.169.1.1")
                                , permissions
                                );
         CPPUNIT_ASSERT_EQUAL( 0 ,  permissions.getSize() );

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

      bool loadUrlMap( AuthRulesUrlMapping& urlmap, const char* mapfile )
      {
         return (   urlmap.loadMappings(mapfile, MS, VM, LH )
                 == OS_SUCCESS
                 );
      };

   FileTestContext* mFileTestContext;
   
};

CPPUNIT_TEST_SUITE_REGISTRATION(AuthRulesUrlMappingTest);

