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
#include <net/HttpMessage.h>
#include <net/SdpBody.h>
#include <utl/UtlString.h>

// Extract the contents of an HttpBody, with the Content-Type header prepended,
// and the boundary string replaced with "[boundary]".
UtlString extract_contents(HttpBody* body);

/**
 * Unittest for HttpBody
 */
class HttpBodyTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(HttpBodyTest);
   CPPUNIT_TEST(testCreateMultipart);
   CPPUNIT_TEST_SUITE_END();

public:

   void testCreateMultipart()
      {
         #define BODY1 \
            "Now is the time for all good men to come to the aid of their party."
         #define HEADER \
            "Content-Type: multipart/related;" \
               "boundary=\"[boundary]\"\r\n"   \
               "\r\n"
         #define PART1 \
            "--[boundary]\r\n" \
               "Content-ID: <nXYxAE@example.com>\r\n" \
               "CONTENT-TYPE: text/plain\r\n" \
               "CONTENT-TRANSFER-ENCODING: binary\r\n" \
               "\r\n" \
               BODY1 \
               "\r\n"
         #define FORMAT2 \
            "--[boundary]\r\n" \
               "CONTENT-TYPE: application/octet-stream\r\n" \
               "CONTENT-TRANSFER-ENCODING: binary\r\n" \
               "\r\n" \
               "%s" \
               "\r\n"
         #define TRAILER \
            "--[boundary]--\r\n"

         HttpBodyMultipart body("multipart/related");
         ASSERT_STR_EQUAL_MESSAGE("Zero body parts",
                                  HEADER
                                  TRAILER,
                                  extract_contents(&body).data());

         HttpBody bodyPart1(BODY1, sizeof (BODY1) - 1, "text/plain");
         UtlDList parameters1;
         NameValuePair nvp("Content-ID", "<nXYxAE@example.com>");
         parameters1.append(&nvp);
         body.appendBodyPart(bodyPart1, parameters1);

         ASSERT_STR_EQUAL_MESSAGE("One body part",
                                  HEADER
                                  PART1
                                  TRAILER,
                                  extract_contents(&body).data());

         // Find the current boundary string and use it as a body,
         // forcing the HttpBody to change the boundary string.
         char boundary_string[10];
         strcpy(boundary_string, body.getMultipartBoundary());
         HttpBody bodyPart2(boundary_string, strlen(boundary_string),
                            "application/octet-stream");
         UtlDList parameters2;
         body.appendBodyPart(bodyPart2, parameters2);

         char part2[1024];
         sprintf(part2, HEADER PART1 FORMAT2 TRAILER, boundary_string);
         ASSERT_STR_EQUAL_MESSAGE("Two body parts",
                                  part2,
                                  extract_contents(&body).data());

         const char* new_boundary_string = body.getMultipartBoundary();
         CPPUNIT_ASSERT_MESSAGE("Changed boundary string",
                                strcmp(boundary_string, new_boundary_string));
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION(HttpBodyTest);

// Extract the contents of an HttpBody.
UtlString extract_contents(HttpBody* body)
{
   UtlString res;

   res.append("Content-Type: ");
   res.append(body->getContentType());
   res.append("\r\n\r\n");
   res.append(body->getBytes());

   // Replace the boundary string with "[boundary]".
   const char* boundary_string = body->getMultipartBoundary();
   ssize_t location;
   while ((location = res.index(boundary_string)) != UTL_NOT_FOUND)
   {
      res.replace(location, strlen(boundary_string),
                  "[boundary]");
   }

   return res;
}
