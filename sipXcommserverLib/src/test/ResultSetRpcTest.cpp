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
#include "net/XmlRpcRequest.h"
#include "xmlparser/tinyxml.h"
#include "sipdb/ResultSet.h"

typedef struct
{
   const char* uri;
   const char* callid;
   const char* contact;
   const char* expires;
   const char* cseq;
   const char* qvalue;
   const char* instance_id;
   const char* gruu;
   const char* path;
} RegistrationRow;

RegistrationRow regdata[] =
{
   {
      "sip:user1@example.com",
      "6745637808245563@TmVhbC1sYXB0b3Ay",
      "sip:181@192.168.0.2:6012",
      "1133218054",
      "3",
      "",
      "1111",
      "sip:181@example.com;gr",
      "<sip:visitme.com>,<sip:overhere.com>,<sip:comemyway.com>"
   },
   {
      "sip:user@example.com",
      "8d2d9c70405f4e66@TmVhbC1sYXB0b3Ay",
      "sip:181@66.30.139.170:24907",
      "1133221655",
      "2",
      "0.8",
      "2222",
      "sip:182@example.com;gr",
      "<sip:visitme.com>,<sip:overhere.com>,<sip:comemyway.com>"
   },
   {
      "sip:user3@example.com",
      "fa294244984e0c3f@TmVhbC1sYXB0b3Ay",
      "sip:181@192.168.0.2:6000",
      "1133221680",
      "1",
      "0.2",
      "3333",
      "sip:183@example.com;gr",
      "<sip:visitme.com>,<sip:overhere.com>,<sip:comemyway.com>"
   }
};

class ResultSetRpcTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(ResultSetRpcTest);
   CPPUNIT_TEST(testRegistrationSet);
   CPPUNIT_TEST_SUITE_END();

public:

   void testRegistrationSet()
      {
         ResultSet registrations;

         size_t row;
         for (row = 0; row < sizeof(regdata)/sizeof(RegistrationRow); row++)
         {
            UtlHashMap regRow;

            UtlString* uriKey = new UtlString("uri");
            UtlString* uriValue = new UtlString(regdata[row].uri);
            regRow.insertKeyAndValue(uriKey, uriValue);

            UtlString* callidKey = new UtlString("callid");
            UtlString* callidValue = new UtlString(regdata[row].callid);
            regRow.insertKeyAndValue(callidKey, callidValue);

            UtlString* contactKey = new UtlString("contact");
            UtlString* contactValue = new UtlString(regdata[row].contact);
            regRow.insertKeyAndValue(contactKey, contactValue);

            UtlString* expiresKey = new UtlString("expires");
            UtlString* expiresValue = new UtlString(regdata[row].expires);
            regRow.insertKeyAndValue(expiresKey, expiresValue);

            UtlString* cseqKey = new UtlString("cseq");
            UtlString* cseqValue = new UtlString(regdata[row].cseq);
            regRow.insertKeyAndValue(cseqKey, cseqValue);

            UtlString* qvalueKey = new UtlString("qvalue");
            UtlString* qvalueValue = new UtlString(regdata[row].qvalue);
            regRow.insertKeyAndValue(qvalueKey, qvalueValue);

            UtlString* instanceIdKey = new UtlString("instance_id");
            UtlString* instanceIdValue = new UtlString(regdata[row].instance_id);
            regRow.insertKeyAndValue(instanceIdKey, instanceIdValue);

            UtlString* gruuKey = new UtlString("gruu");
            UtlString* gruuValue = new UtlString(regdata[row].gruu);
            regRow.insertKeyAndValue(gruuKey, gruuValue);

            UtlString* pathKey = new UtlString("path");
            UtlString* pathValue = new UtlString(regdata[row].path);
            regRow.insertKeyAndValue(pathKey, pathValue);

            registrations.addValue(regRow);
         }

         Url target("http://server.exmple.com");

         XmlRpcRequest request(target,"RPC.METHOD");

         request.addParam(&registrations);

         UtlString requestBody;
         ssize_t bodyLength;
         request.mpRequestBody->getBytes(&requestBody, &bodyLength);

         const char* correctRequestBody =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<methodCall>\n"
            "<methodName>RPC.METHOD</methodName>\n"
            "<params>\n"
            "<param>\n"
            "<value><array><data>\n"
            "<value><struct>\n"
            "<member>\n"
            "<name>gruu</name><value><string>sip:181@example.com;gr</string></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>path</name><value><string>&lt;sip:visitme.com&gt;,&lt;sip:overhere.com&gt;,&lt;sip:comemyway.com&gt;</string></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>uri</name><value><string>sip:user1@example.com</string></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>contact</name><value><string>sip:181@192.168.0.2:6012</string></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>qvalue</name><value><string></string></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>cseq</name><value><string>3</string></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>expires</name><value><string>1133218054</string></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>instance_id</name><value><string>1111</string></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>callid</name><value><string>6745637808245563@TmVhbC1sYXB0b3Ay</string></value>\n"
            "</member>\n"
            "</struct></value>\n"
            "<value><struct>\n"
            "<member>\n"
            "<name>gruu</name><value><string>sip:182@example.com;gr</string></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>path</name><value><string>&lt;sip:visitme.com&gt;,&lt;sip:overhere.com&gt;,&lt;sip:comemyway.com&gt;</string></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>uri</name><value><string>sip:user@example.com</string></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>contact</name><value><string>sip:181@66.30.139.170:24907</string></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>qvalue</name><value><string>0.8</string></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>cseq</name><value><string>2</string></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>expires</name><value><string>1133221655</string></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>instance_id</name><value><string>2222</string></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>callid</name><value><string>8d2d9c70405f4e66@TmVhbC1sYXB0b3Ay</string></value>\n"
            "</member>\n"
            "</struct></value>\n"
            "<value><struct>\n"
            "<member>\n"
            "<name>gruu</name><value><string>sip:183@example.com;gr</string></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>path</name><value><string>&lt;sip:visitme.com&gt;,&lt;sip:overhere.com&gt;,&lt;sip:comemyway.com&gt;</string></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>uri</name><value><string>sip:user3@example.com</string></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>contact</name><value><string>sip:181@192.168.0.2:6000</string></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>qvalue</name><value><string>0.2</string></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>cseq</name><value><string>1</string></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>expires</name><value><string>1133221680</string></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>instance_id</name><value><string>3333</string></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>callid</name><value><string>fa294244984e0c3f@TmVhbC1sYXB0b3Ay</string></value>\n"
            "</member>\n"
            "</struct></value>\n"
            "</data></array></value>\n"
            "</param>\n"
            ;

         ASSERT_STR_EQUAL(correctRequestBody, requestBody.data());
         CPPUNIT_ASSERT_EQUAL(strlen(correctRequestBody), requestBody.length());

      };
};

CPPUNIT_TEST_SUITE_REGISTRATION(ResultSetRpcTest);
