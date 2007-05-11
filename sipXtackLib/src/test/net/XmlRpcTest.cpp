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

#include <os/OsDefs.h>
#include <utl/UtlInt.h>
#include <utl/UtlLongLongInt.h>
#include <utl/UtlBool.h>
#include <utl/UtlDateTime.h>
#include <utl/UtlSListIterator.h>
#include <utl/UtlHashMapIterator.h>
#include <net/Url.h>
#include <net/XmlRpcRequest.h>
#include <net/XmlRpcResponse.h>
#include <net/XmlRpcDispatch.h>

//#define PRINT_OUT 1

class AddExtension : public XmlRpcMethod
{
public:

   /// Get the instance of this method.
   static AddExtension* get()
      {
         return (new AddExtension());
      };

   /// Destructor
   virtual ~AddExtension()
      {
      };

   bool execute(const HttpRequestContext& context, UtlSList& params, void* userData, XmlRpcResponse& response, XmlRpcMethod::ExecutionStatus& status)
      {
#ifdef PRINT_OUT
         for (unsigned int i = 0; i < params.entries(); i++)
         {
            printf("index = %d\n", i);
            
            UtlContainable *value = params.at(i);
            UtlString paramType(value->getContainableType());
            if (paramType.compareTo("UtlInt") == 0)
            {
               UtlInt* paramValue = (UtlInt *)value;
               printf("value = %d\n", paramValue->getValue());
            }
            
            if (paramType.compareTo("UtlString") == 0)
            {
               UtlString* paramValue = (UtlString *)value;
               printf("value = %s\n", paramValue->data());
            }
   
            if (paramType.compareTo("UtlSList") == 0)
            {
               UtlSList* list = (UtlSList *)value;
               UtlSListIterator iterator(*list);
               UtlContainable* pObject;
               while((pObject = iterator()))
               {
                  UtlString elementType(pObject->getContainableType());
                  if (elementType.compareTo("UtlInt") == 0)
                  {
                     UtlInt* paramValue = (UtlInt *)pObject;
                     printf("value = %d\n", paramValue->getValue());
                  }
                  
                  if (elementType.compareTo("UtlString") == 0)
                  {
                     UtlString* paramValue = (UtlString *)pObject;
                     printf("value = %s\n", paramValue->data());
                  }
               }
            }
   
            if (paramType.compareTo("UtlHashMap") == 0)
            {
               UtlHashMap* map = (UtlHashMap *)value;
               UtlHashMapIterator iterator(*map);
               UtlString* pName;
               while((pName = (UtlString *)iterator()))
               {
                  printf("name = %s\n", pName->data());
                  
                  UtlContainable* pObject = map->findValue(pName);
                  UtlString elementType(pObject->getContainableType());
                  if (elementType.compareTo("UtlInt") == 0)
                  {
                     UtlInt* paramValue = (UtlInt *)pObject;
                     printf("value = %d\n", paramValue->getValue());
                  }
                  
                  if (elementType.compareTo("UtlString") == 0)
                  {
                     UtlString* paramValue = (UtlString *)pObject;
                     printf("value = %s\n", paramValue->data());
                  }
                  
                  if (elementType.compareTo("UtlSList") == 0)
                  {
                     UtlSList* list = (UtlSList *)pObject;
                     UtlSListIterator iterator(*list);
                     UtlContainable* pList;
                     while((pList = iterator()))
                     {
                        UtlString elementType(pList->getContainableType());
                        if (elementType.compareTo("UtlInt") == 0)
                        {
                           UtlInt* paramValue = (UtlInt *)pList;
                           printf("value = %d\n", paramValue->getValue());
                        }
                        
                        if (elementType.compareTo("UtlString") == 0)
                        {
                           UtlString* paramValue = (UtlString *)pList;
                           printf("value = %s\n", paramValue->data());
                        }
                     }
                  }
               }
            }
         }
#endif
         status = XmlRpcMethod::OK;
         UtlString responseText("method call \"");
         responseText.append(UtlString((char*)userData));
         responseText.append("\" successful");
         response.setResponse(&responseText);
         return true;
      };

private:
   AddExtension() {};

};

/**
 * Unit test for XmlRpc
 */
class XmlRpcTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(XmlRpcTest);
   CPPUNIT_TEST(testXmlRpcRequestCreation);
   CPPUNIT_TEST(testXmlRpcRequestParse);
   CPPUNIT_TEST(testXmlRpcResponseParse);
   CPPUNIT_TEST(testXmlRpcResponseSetting);
   CPPUNIT_TEST(testIllFormattedXmlRpcRequest);   
   CPPUNIT_TEST_SUITE_END();

public:

   void testXmlRpcRequestCreation()
      {
         const char *ref =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<methodCall>\n"
            "<methodName>addExtension</methodName>\n"
            "<params>\n"
            "<param>\n"
            "<value><string>&quot;ACD&quot; &lt;acd@pingtel.com&gt;</string></value>\n"
            "</param>\n"
            "<param>\n"
            "<value><int>162</int></value>\n"
            "</param>\n"
            "<param>\n"
            "<value><i8>0x00000000027972</i8></value>\n"
            "</param>\n"
            "<param>\n"
            "<value><array><data>\n"
            "<value><string>160@pingtel.com</string></value>\n"
            "<value><string>167@pingtel.com</string></value>\n"
            "<value><int>1000</int></value>\n"
            "<value><boolean>1</boolean></value>\n"
            "</data></array></value>\n"
            "</param>\n"
            "<param>\n"
            "<value><struct>\n"
            "<member>\n"
            "<name>acd@pingtel.com</name><value><array><data>\n"
            "<value><string>160@pingtel.com</string></value>\n"
            "<value><string>167@pingtel.com</string></value>\n"
            "<value><int>1000</int></value>\n"
            "<value><boolean>1</boolean></value>\n"
            "</data></array></value>\n"
            "</member>\n"
            "</struct></value>\n"
            "</param>\n"
            "</params>\n"
            "</methodCall>\n"
            ;

         Url url;
         XmlRpcRequest request(url, "addExtension");

         // Use quotes, "<" and ">" to test that XML special chars are escaped properly
         UtlString stringToEscapeValue("\"ACD\" <acd@pingtel.com>");
         request.addParam(&stringToEscapeValue);

         UtlInt intValue(162);
         request.addParam(&intValue);

         UtlLongLongInt llintValue(162162);
         request.addParam(&llintValue);
         
         UtlSList list;
         UtlString array1("160@pingtel.com");
         list.insert(&array1);
         UtlString array2("167@pingtel.com");
         list.insert(&array2);
         UtlInt array3(1000);
         list.insert(&array3);
         UtlBool array4(true);
         list.insert(&array4);
         request.addParam(&list);
         
         UtlHashMap members;
         UtlString stringValue("acd@pingtel.com");
         members.insertKeyAndValue(&stringValue, &list);
         request.addParam(&members);         

         XmlRpcResponse response;
         request.execute(response);

         UtlString requestBody;
         int length;
         request.mpRequestBody->getBytes(&requestBody, &length);
         
         ASSERT_STR_EQUAL(ref, requestBody.data());
      }


   void testXmlRpcRequestParse()
      {
         const char *ref =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<methodCall>\n"
            "<methodName>addExtension</methodName>\n"
            "<params>\n"
            "<param>\n"
            "<value>&quot;ACD&quot; &lt;acd@pingtel.com&gt;</value>\n"
            "</param>\n"
            "<param>\n"
            "<value><int>162</int></value>\n"
            "</param>\n"
            "<param>\n"
            "<value><i8>0x00000000027972</i8></value>\n"
            "</param>\n"
            "<param>\n"
            "<value><array><data>\n"
            "<value><string>160@pingtel.com</string></value>\n"
            "<value><string>167@pingtel.com</string></value>\n"
            "<value><int>1000</int></value>\n"
            "<value><boolean>1</boolean></value>\n"
            "</data></array></value>\n"
            "</param>\n"
            "<param>\n"
            "<value><struct>\n"
            "<member>\n"
            "<name>acd@pingtel.com</name><value><array><data>\n"
            "<value><string>160@pingtel.com</string></value>\n"
            "<value><string>167@pingtel.com</string></value>\n"
            "<value><int>1000</int></value>\n"
            "<value><boolean>1</boolean></value>\n"
            "</data></array></value>\n"
            "</member>\n"
            "</struct></value>\n"
            "</param>\n"
            "<param>\n"
            "<value><struct>\n"
            "<member>\n"
            "<name>tcp-port</name><value><int>5150</int></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>rtp-port</name><value><int>9100</int></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>upd-port</name><value><int>5150</int></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>server-name</name><value>sipxacd</value>\n"
            "</member>\n"
            "<member>\n"
            "<name>object-class</name><value>acd-server</value>\n"
            "</member>\n"
            "<member>\n"
            "<name>agent-state-server-port</name><value><int>8101</int></value>\n"
            "</member>\n"
            "</struct></value>\n"
            "</param>\n"
            "</params>\n"
            "</methodCall>\n"
            ;
            
         const char *faultResponse =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<methodResponse>\n"
            "<fault>\n"
            "<value><struct>\n"
            "<member>\n"
            "<name>faultCode</name><value><int>-3</int></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>faultString</name><value><string>Method has not been registered</string></value>\n"
            "</member>\n"
            "</struct></value>\n"
            "</fault>\n"
            "</methodResponse>\n"
            ;

         const char *successResponse =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<methodResponse>\n"
            "<params>\n"
            "<param>\n"
            "<value><string>method call &quot;AddExtension&quot; successful</string></value>\n"
            "</param>\n"
            "</params>\n"
            "</methodResponse>\n"
            ;

         XmlRpcDispatch dispatch(8200, false, "/RPC2");

         UtlString requestContent(ref);
         XmlRpcResponse response;
         XmlRpcMethodContainer* method;
         UtlSList params;

         bool result = dispatch.parseXmlRpcRequest(requestContent, method, params, response);
         CPPUNIT_ASSERT(result == false);

         XmlRpcBody *responseBody = response.getBody();

         UtlString body;
         int length;
         responseBody->getBytes(&body, &length);
         
         ASSERT_STR_EQUAL(faultResponse, body.data());

         XmlRpcResponse newResponse;
         char userData[] = "AddExtension"; 
         dispatch.addMethod("addExtension", (XmlRpcMethod::Get *)AddExtension::get, (void*)userData);
         result = dispatch.parseXmlRpcRequest(requestContent, method, params, newResponse);
         CPPUNIT_ASSERT(result == true);
         
         HttpRequestContext context;
         XmlRpcMethod::ExecutionStatus status = XmlRpcMethod::OK;
         XmlRpcMethod::Get* methodGet;
         void* user;
         method->getData(methodGet, user);
         XmlRpcMethod* addEx = methodGet();
         addEx->execute(context, params, userData, newResponse, status);
         dispatch.cleanUp(&params);

         responseBody = newResponse.getBody();

         responseBody->getBytes(&body, &length);

         ASSERT_STR_EQUAL(successResponse, body.data());
      }


   void testXmlRpcResponseParse()
      {
         const char *faultResponse =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<methodResponse>\n"
            "<fault>\n"
            "<value><struct>\n"
            "<member>\n"
            "<name>faultCode</name><value><int>-3</int></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>faultString</name><value><string>Method has not been registered</string></value>\n"
            "</member>\n"
            "</struct></value>\n"
            "</fault>\n"
            "</methodResponse>\n"
            ;

         const char *successResponse1 =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<methodResponse>\n"
            "<params>\n"
            "<param>\n"
            "<value><string>method call \"AddExtension\" successful</string></value>\n"
            "</param>\n"
            "</params>\n"
            "</methodResponse>\n"
            ;

         const char *successResponse2 =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<methodResponse>\n"
            "<params>\n"
            "<param>\n"
            "<value><struct>\n"
            "<member>\n"
            "<name>acd@pingtel.com</name><value><array>\n"
            "<data><value><string>160@pingtel.com</string></value>\n"
            "<value><string>167@pingtel.com</string></value>\n"
            "<value><int>1000</int></value>\n"
            "<value><boolean>1</boolean></value>\n"
            "</data></array></value>\n"
            "</member>\n"
            "</struct></value>\n"
            "</param>\n"
            "</params>\n"
            "</methodResponse>\n"
            ;

         UtlString faultContent(faultResponse);
         XmlRpcResponse response;

         bool result = response.parseXmlRpcResponse(faultContent);
         CPPUNIT_ASSERT(result == false);

         int faultCode;
         UtlString faultString;
         response.getFault(&faultCode, faultString);
         CPPUNIT_ASSERT_EQUAL_MESSAGE("faultCode is not the same",
                                      faultCode, -3);

         ASSERT_STR_EQUAL("Method has not been registered", faultString.data());

         UtlString successContent1(successResponse1);

         result = response.parseXmlRpcResponse(successContent1);
         CPPUNIT_ASSERT(result == true);

         UtlContainable *containable;
         response.getResponse(containable);
         UtlString* responseString = (UtlString *)containable;

         ASSERT_STR_EQUAL("method call \"AddExtension\" successful", responseString->data());

         UtlString successContent2(successResponse2);

         result = response.parseXmlRpcResponse(successContent2);
         CPPUNIT_ASSERT(result == true);
      }


   void testXmlRpcResponseSetting()
      {
         const char *ref =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<methodResponse>\n"
            "<params>\n"
            "<param>\n"
            "<value><struct>\n"
            "<member>\n"
            "<name>acd@pingtel.com</name><value><array><data>\n"
            "<value><string>160@pingtel.com</string></value>\n"
            "<value><string>167@pingtel.com</string></value>\n"
            "<value><int>1000</int></value>\n"
            "<value><boolean>1</boolean></value>\n"
            "</data></array></value>\n"
            "</member>\n"
            "</struct></value>\n"
            "</param>\n"
            "</params>\n"
            "</methodResponse>\n"
            ;

         Url url;
         XmlRpcResponse response;
         
         UtlString stringValue("acd@pingtel.com");

         UtlSList list;
         UtlString array1("160@pingtel.com");
         list.insert(&array1);
         UtlString array2("167@pingtel.com");
         list.insert(&array2);
         UtlInt array3(1000);
         list.insert(&array3);
         UtlBool array4(true);
         list.insert(&array4);
         
         UtlHashMap members;
         members.insertKeyAndValue(&stringValue, &list);
         response.setResponse(&members);         

         UtlString responseBody;
         int length;
         response.getBody()->getBytes(&responseBody, &length);

         ASSERT_STR_EQUAL(ref, responseBody.data());
      }

   void testIllFormattedXmlRpcRequest()
      {
         const char *ref1 =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<methodCall>\n"
            "<methodName>addExtension</methodName>\n"
            "<params>\n"
            "<param>\n"
            "<value></value>\n"
            "</param>\n"
            "<param>\n"
            "<value><int></int></value>\n"
            "</param>\n"
            "</params>\n"
            "</methodCall>\n"
            ;

         const char *ref2 =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<methodCall>\n"
            "<methodName>addExtension</methodName>\n"
            "<params>\n"
            "<param>\n"
            "<value><array><data>\n"
            "<value><string>160@pingtel.com</string></value>\n"
            "<value><string>167@pingtel.com</string></value>\n"
            "<value><int></int></value>\n"
            "<value><boolean>1</boolean></value>\n"
            "</data></array></value>\n"
            "</param>\n"
            "</params>\n"
            "</methodCall>\n"
            ;

         const char *ref3 =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<methodCall>\n"
            "<methodName>addExtension</methodName>\n"
            "<params>\n"
            "<param>\n"
            "<value><struct>\n"
            "<member>\n"
            "<name>tcp-port</name><value><int>5150</int></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>rtp-port</name><value><int></int></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>upd-port</name><value><int>5150</int></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>server-name</name><value></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>object-class</name><value></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>agent-state-server-port</name><value><int>8101</int></value>\n"
            "</member>\n"
            "</struct></value>\n"
            "</param>\n"
            "</params>\n"
            "</methodCall>\n"
            ;
            
         const char *faultResponse =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<methodResponse>\n"
            "<fault>\n"
            "<value><struct>\n"
            "<member>\n"
            "<name>faultCode</name><value><int>-5</int></value>\n"
            "</member>\n"
            "<member>\n"
            "<name>faultString</name><value><string>Empty param value</string></value>\n"
            "</member>\n"
            "</struct></value>\n"
            "</fault>\n"
            "</methodResponse>\n"
            ;

         XmlRpcDispatch dispatch(8200, false, "/RPC2");

         UtlString requestContent1(ref1);
         XmlRpcResponse response1;
         XmlRpcMethodContainer* method;
         UtlSList params;

         const char* userData = "AddExtension"; 
         dispatch.addMethod("addExtension", (XmlRpcMethod::Get *)AddExtension::get, (void*)userData);
         bool result = dispatch.parseXmlRpcRequest(requestContent1, method, params, response1);
         CPPUNIT_ASSERT(result == false);
         dispatch.cleanUp(&params);

         XmlRpcBody *responseBody = response1.getBody();

         UtlString body;
         int length;
         responseBody->getBytes(&body, &length);

         ASSERT_STR_EQUAL(faultResponse, body.data());

         UtlString requestContent2(ref2);
         XmlRpcResponse response2;
         result = dispatch.parseXmlRpcRequest(requestContent2, method, params, response2);
         CPPUNIT_ASSERT(result == false);
         dispatch.cleanUp(&params);

         responseBody = response2.getBody();
         responseBody->getBytes(&body, &length);

         ASSERT_STR_EQUAL(faultResponse, body.data());

         UtlString requestContent3(ref3);
         XmlRpcResponse response3;
         result = dispatch.parseXmlRpcRequest(requestContent3, method, params, response3);
         CPPUNIT_ASSERT(result == false);
         dispatch.cleanUp(&params);

         responseBody = response3.getBody();
         responseBody->getBytes(&body, &length);

         ASSERT_STR_EQUAL(faultResponse, body.data());
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION(XmlRpcTest);
