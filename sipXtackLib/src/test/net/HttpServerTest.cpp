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
#include "sipxunit/TestUtilities.h"

#include "os/OsDefs.h"
#include "net/HttpServer.h"
#include "net/HttpService.h"

class TestHttpService : public HttpService
{
   void processRequest(const HttpRequestContext& requestContext,
                       const HttpMessage& request,
                       HttpMessage*& response
                       )
      {
         // dummy process method - never called
      }
};


/**
 * Unit tests for HttpServer methods
 */
class HttpServerTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(HttpServerTest);
   CPPUNIT_TEST(testMapUriRoot);
   CPPUNIT_TEST(testMapUriNoRoot);
   CPPUNIT_TEST(testMapUriPostRoot);
   CPPUNIT_TEST(testMapUriLongest);
   CPPUNIT_TEST(testMapUriEndSep);
   CPPUNIT_TEST(testMapUriToNull);
   CPPUNIT_TEST(testFindHttpService);
   
   CPPUNIT_TEST_SUITE_END();

public:

   void testMapUriRoot()
      {
         UtlString mapped;

         HttpServer server(NULL);
         server.addUriMap("/", "/rootpath");

         CPPUNIT_ASSERT(HttpServer::mapUri(server.mUriMaps, "/", mapped));
         ASSERT_STR_EQUAL("/rootpath", mapped.data());

         CPPUNIT_ASSERT(HttpServer::mapUri(server.mUriMaps, "/uripath", mapped));
         ASSERT_STR_EQUAL("/rootpath/uripath", mapped.data());
      }

   void testMapUriNoRoot()
      {
         UtlString mapped;

         HttpServer server(NULL);
         server.addUriMap("/notrooturi", "/notrootpath");
         
         CPPUNIT_ASSERT(!HttpServer::mapUri(server.mUriMaps, "/uripath", mapped));
         ASSERT_STR_EQUAL("/uripath", mapped.data());

         CPPUNIT_ASSERT(!HttpServer::mapUri(server.mUriMaps, "/", mapped));
         ASSERT_STR_EQUAL("/", mapped.data());
      }

   void testMapUriPostRoot()
      {
         UtlString mapped;

         HttpServer server(NULL);
         server.addUriMap("/a/b", "/x/y");

         CPPUNIT_ASSERT(HttpServer::mapUri(server.mUriMaps, "/a/b/c", mapped));
         ASSERT_STR_EQUAL("/x/y/c", mapped.data());

         CPPUNIT_ASSERT(!HttpServer::mapUri(server.mUriMaps, "/", mapped));
         ASSERT_STR_EQUAL("/", mapped.data());

         CPPUNIT_ASSERT(!HttpServer::mapUri(server.mUriMaps, "/x/a/b/c", mapped));
         ASSERT_STR_EQUAL("/x/a/b/c", mapped.data());
      }

   void testMapUriLongest()
      {
         UtlString mapped;

         HttpServer server(NULL);
         server.addUriMap("/a/b/c", "/longpath");
         server.addUriMap("/a/b",   "/shortpath");

         CPPUNIT_ASSERT(HttpServer::mapUri(server.mUriMaps, "/a/b/c/d", mapped));
         ASSERT_STR_EQUAL("/longpath/d", mapped.data());
      }

   void testMapUriEndSep() 
      {
         UtlString mapped;

         HttpServer server(NULL);
         server.addUriMap("/a/b/", "/root1"); // @TODO bogus that this doesn't work

         CPPUNIT_ASSERT(!HttpServer::mapUri(server.mUriMaps, "/a/b/c", mapped));
         ASSERT_STR_EQUAL("/a/b/c", mapped.data());

         server.addUriMap("/a/b",  "/root2");

         CPPUNIT_ASSERT(HttpServer::mapUri(server.mUriMaps, "/a/b/c", mapped));
         ASSERT_STR_EQUAL("/root2/c", mapped.data());

         CPPUNIT_ASSERT(!HttpServer::mapUri(server.mUriMaps, "/a/bc", mapped));
         ASSERT_STR_EQUAL("/a/bc", mapped.data());
      }

   void testMapUriToNull()
      {
         UtlString mapped;

         HttpServer server(NULL);
         server.addUriMap("/a/b", "/");

         CPPUNIT_ASSERT(HttpServer::mapUri(server.mUriMaps, "/a/b/c/d", mapped));
//         KNOWN_BUG("TODO", "mapping prefix to root or null does not work");
         ASSERT_STR_EQUAL("/c/d", mapped.data());
      }

   void testFindHttpService()
      {
         HttpServer      httpServer(NULL);
         
         HttpService* foundService;
         
         CPPUNIT_ASSERT(!httpServer.findHttpService("/", foundService));
         CPPUNIT_ASSERT(NULL == foundService);

         TestHttpService testServiceOne;
         httpServer.addHttpService("/one", &testServiceOne);

         CPPUNIT_ASSERT(!httpServer.findHttpService("/", foundService));
         CPPUNIT_ASSERT(NULL == foundService);

         CPPUNIT_ASSERT(httpServer.findHttpService("/one", foundService));
         CPPUNIT_ASSERT(&testServiceOne == foundService);

         CPPUNIT_ASSERT(httpServer.findHttpService("/one/two", foundService));
         CPPUNIT_ASSERT(&testServiceOne == foundService);

         CPPUNIT_ASSERT(!httpServer.findHttpService("/one_extra", foundService));
         CPPUNIT_ASSERT(NULL == foundService);

         TestHttpService testServiceTwo;
         httpServer.addHttpService("/one/two", &testServiceTwo);

         CPPUNIT_ASSERT(!httpServer.findHttpService("/", foundService));
         CPPUNIT_ASSERT(NULL == foundService);

         CPPUNIT_ASSERT(httpServer.findHttpService("/one", foundService));
         CPPUNIT_ASSERT(&testServiceOne == foundService);

         CPPUNIT_ASSERT(httpServer.findHttpService("/one/two", foundService));
         CPPUNIT_ASSERT(&testServiceTwo == foundService);

         CPPUNIT_ASSERT(httpServer.findHttpService("/one/two/three", foundService));
         CPPUNIT_ASSERT(&testServiceTwo == foundService);

         CPPUNIT_ASSERT(!httpServer.findHttpService("/one_extra", foundService));
         CPPUNIT_ASSERT(NULL == foundService);

         CPPUNIT_ASSERT(!httpServer.findHttpService("/one_extra/two", foundService));
         CPPUNIT_ASSERT(NULL == foundService);

         CPPUNIT_ASSERT(httpServer.findHttpService("/one/two_extra", foundService));
         CPPUNIT_ASSERT(&testServiceOne == foundService);

         TestHttpService testRootService;
         httpServer.addHttpService("/", &testRootService);

         CPPUNIT_ASSERT(httpServer.findHttpService("/", foundService));
         CPPUNIT_ASSERT(&testRootService == foundService);

         CPPUNIT_ASSERT(httpServer.findHttpService("/one", foundService));
         CPPUNIT_ASSERT(&testServiceOne == foundService);

         CPPUNIT_ASSERT(httpServer.findHttpService("/one/two", foundService));
         CPPUNIT_ASSERT(&testServiceTwo == foundService);

         CPPUNIT_ASSERT(httpServer.findHttpService("/one/two/three", foundService));
         CPPUNIT_ASSERT(&testServiceTwo == foundService);

         CPPUNIT_ASSERT(httpServer.findHttpService("/one_extra", foundService));
         CPPUNIT_ASSERT(&testRootService == foundService);

         CPPUNIT_ASSERT(httpServer.findHttpService("/one_extra/two", foundService));
         CPPUNIT_ASSERT(&testRootService == foundService);

         CPPUNIT_ASSERT(httpServer.findHttpService("/one/two_extra", foundService));
         CPPUNIT_ASSERT(&testServiceOne == foundService);
      } 
};


CPPUNIT_TEST_SUITE_REGISTRATION(HttpServerTest);

