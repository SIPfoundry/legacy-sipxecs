// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsFS.h>
#include <os/OsSysLog.h>
#include <utl/UtlVoidPtr.h>
#include <utl/UtlInt.h>
#include <utl/UtlLongLongInt.h>
#include <utl/UtlBool.h>
#include <utl/UtlDateTime.h>
#include <utl/UtlHashMapIterator.h>
#include <utl/UtlSListIterator.h>
#include <os/OsServerSocket.h>
#include <os/OsSSLServerSocket.h>
#include <net/HttpServer.h>
#include <net/HttpRequestContext.h>
#include <net/HttpMessage.h>
#include "net/XmlRpcDispatch.h"

// STATIC VARIABLE DEFINITIONS
const UtlContainableType XmlRpcMethodContainer::TYPE = "XmlRpcMethod";


#undef TEST_HTTP /* turn on to log raw http messages */

XmlRpcMethodContainer::XmlRpcMethodContainer(const char* name)
   :mMethodName(name)
   ,mpUserData(NULL)
   ,mpMethod(NULL)     
{
}

XmlRpcMethodContainer::~XmlRpcMethodContainer()
{
}


int XmlRpcMethodContainer::compareTo(const UtlContainable *b) const
{
   return ((mpUserData == ((XmlRpcMethodContainer *)b)->mpUserData) &&
           (mpMethod == ((XmlRpcMethodContainer *)b)->mpMethod));
}


unsigned int XmlRpcMethodContainer::hash() const
{
   return hashPtr(mpUserData);
}


const UtlContainableType XmlRpcMethodContainer::getContainableType() const
{
    return TYPE;
}

void XmlRpcMethodContainer::setData(XmlRpcMethod::Get* method, void* userData)
{
   mpMethod = method;
   mpUserData = userData;
}
   
void XmlRpcMethodContainer::getData(XmlRpcMethod::Get*& method, void*& userData)
{
   method = mpMethod;
   userData = mpUserData;
}

void XmlRpcMethodContainer::getName(UtlString& methodName)
{
   methodName = mMethodName;
}


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const char* XmlRpcDispatch::DEFAULT_URL_PATH = "/RPC2";


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
XmlRpcDispatch::XmlRpcDispatch(int httpServerPort,
                               bool isSecureServer,
                               const char* uriPath,
                               const char* bindIp)
   : mLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
{
    UtlString osBaseUriDirectory ;

    OsPath workingDirectory;
    OsPath path;
    OsFileSystem::getWorkingDirectory(path);
    path.getNativePath(workingDirectory);
    osBaseUriDirectory =  workingDirectory + OsPathBase::separator;

   // Create a HTTPS Server
   OsServerSocket* pServerSocket = NULL;
   if (isSecureServer)
   {
      pServerSocket = new OsSSLServerSocket(50, httpServerPort, bindIp);
   }
   else
   {
      pServerSocket = new OsServerSocket(50, httpServerPort, bindIp);
   }
      
   mpHttpServer = new HttpServer(pServerSocket,
                                 NULL, // no password database
                                 NULL, // no http authentication realm
                                 NULL, // no valid ip address list
                                 true  // use persistent http connections 
                                 );
   
   // Set the http server root to the current directory
   mpHttpServer->allowFileAccess(false);
   mpHttpServer->addUriMap("/", osBaseUriDirectory.data());
   mpHttpServer->start();
   
   // Add the XmlRpcDispatch to the HttpServer
   mpHttpServer->addHttpService(uriPath, (HttpService*)this);
}


// Destructor
XmlRpcDispatch::~XmlRpcDispatch()
{
   // HTTP server shutdown
   if (mpHttpServer)
   {
      mpHttpServer->requestShutdown();
      delete mpHttpServer;
      mpHttpServer = NULL;
   }
}


/* ============================ MANIPULATORS ============================== */


void XmlRpcDispatch::processRequest(const HttpRequestContext& requestContext,
                                    const HttpMessage& request,
                                    HttpMessage*& response )
{
#   ifdef TEST_HTTP
    ssize_t len;
    UtlString httpString;

    request.getBytes(&httpString , &len);
    OsSysLog::add(FAC_XMLRPC, PRI_DEBUG,
                  "XmlRpcDispatch::processRequest HttpEvent = \n%s",
                  httpString.data());
#  endif
    
   // Create a response
   response = new HttpMessage();
   response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION_1_1,
                                        HTTP_OK_CODE,
                                        HTTP_OK_TEXT);

   UtlString bodyString;
   ssize_t bodyLength;
   const HttpBody* requestBody = request.getBody();
   requestBody->getBytes(&bodyString, &bodyLength);
   
   XmlRpcMethod::ExecutionStatus status;
   UtlString methodName("<unparsed>");
   XmlRpcResponse responseBody;
   XmlRpcMethodContainer* methodContainer = NULL;
   UtlSList params;

   if (parseXmlRpcRequest(bodyString, methodContainer, params, responseBody))
   {
      if (methodContainer)
      {
         methodContainer->getName(methodName);
         responseBody.setMethod(methodName);
      
         XmlRpcMethod::Get* methodGet;
         void* userData;
         methodContainer->getData(methodGet, userData);
         XmlRpcMethod* method = methodGet();
         OsSysLog::add(FAC_XMLRPC, PRI_DEBUG,
                       "XmlRpcDispatch::processRequest calling method '%s'",
                       methodName.data()
                       );
         method->execute(requestContext,
                         params, 
                         userData,
                         responseBody,
                         status
                         );
      
         // Delete the instance of the method                
         delete method;

         // Clean up the memory allocated in params
         cleanUp(&params);

         // if the method wants authentication, build the standard response
         // for anything else, it's already built one.
         if (XmlRpcMethod::REQUIRE_AUTHENTICATION == status)
         {
            // Create an authentication challenge response
            responseBody.setFault(AUTHENTICATION_REQUIRED_FAULT_CODE,
                                  AUTHENTICATION_REQUIRED_FAULT_STRING);
         }
      }
      else
      {
         // could not find a registered method - logged and response built in parseXmlRpcRequest
         status = XmlRpcMethod::FAILED;         
      }
   }
   else
   {
      // Parsing the request failed - it will have logged a specific error a
      // and created an appropriate response body
      status = XmlRpcMethod::FAILED;
   }

   // Send the response back
   responseBody.getBody()->getBytes(&bodyString, &bodyLength);

   OsSysLog::add(FAC_XMLRPC, PRI_INFO,
                 "XmlRpcDispatch::processRequest method '%s' response status=%s\n%s",
                 methodName.data(),
                 XmlRpcMethod::ExecutionStatusString(status),
                 bodyString.data()
                 );
   
   response->setBody(new HttpBody(bodyString.data(), bodyLength));
   response->setContentType(CONTENT_TYPE_TEXT_XML);
   response->setContentLength(bodyLength);
}

/* ============================ ACCESSORS ================================= */

void XmlRpcDispatch::addMethod(const char* methodName, XmlRpcMethod::Get* method, void* userData)
{
   mLock.acquire();
   UtlString name(methodName);
   if (mMethods.findValue(&name) == NULL)
   {
      XmlRpcMethodContainer *methodContainer = new XmlRpcMethodContainer(methodName);
      methodContainer->setData(method, userData);
      mMethods.insertKeyAndValue(new UtlString(methodName), methodContainer);
   }
   mLock.release();
}


void XmlRpcDispatch::removeMethod(const char* methodName)
{
   mLock.acquire();
   UtlString key = methodName;
   mMethods.remove(&key);
   mLock.release();
}


/// Return the HTTP server that services RPC requests
HttpServer* XmlRpcDispatch::getHttpServer()
{
   return mpHttpServer;
}

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

bool XmlRpcDispatch::parseXmlRpcRequest(const UtlString& requestContent,
                                        XmlRpcMethodContainer*& methodContainer,
                                        UtlSList& params,
                                        XmlRpcResponse& response)
{
   bool result = false;
   OsSysLog::add(FAC_XMLRPC, PRI_DEBUG,
                 "XmlRpcDispatch::parseXmlRpcRequest requestBody = \n%s",
                 requestContent.data());

   // Parse the XML-RPC response
   TiXmlDocument doc("XmlRpcRequest.xml");
   
   doc.Parse(requestContent);   
   if (!doc.Error())
   {
      TiXmlNode* rootNode = doc.FirstChild ("methodCall");
      
      if (rootNode != NULL)
      {
         // Positive response example
         // 
         // <methodCall>
         //   <methodName>examples.getStateName</methodName>
         //   <params>
         //     <param>
         //       <value><i4>41</i4></value>
         //     </param>
         //   </params>
         // </methodCall>
                  
         TiXmlNode* methodNode = rootNode->FirstChild("methodName");
         
         if (methodNode)
         {
            // Check whether the method exists or not. If not, send back a fault response
            UtlString methodCall = methodNode->FirstChild()->Value();
            methodContainer = (XmlRpcMethodContainer*) mMethods.findValue(&methodCall);
            if (methodContainer)
            {
               /*
                * Since params are optional,
                * assume all is well until proven otherwise now
                */
               result = true; 
               TiXmlNode* paramsNode = rootNode->FirstChild("params");
               
               if (paramsNode)
               {
                  int index = 0;
                  for (TiXmlNode* paramNode = paramsNode->FirstChild("param");
                       result /* stop if any error */ && paramNode /* or no more param nodes */; 
                       paramNode = paramNode->NextSibling("param"))
                  {
                     TiXmlNode* subNode = paramNode->FirstChild("value");
                     
                     if (subNode)
                     {
                        result = parseValue(subNode, index, params);
                        if (!result)
                        {
                           OsSysLog::add(FAC_XMLRPC, PRI_ERR,
                                         "XmlRpcDispatch::parseXmlRpcRequest"
                                         " ill-formed XML contents in %s.",
                                          requestContent.data());
                           response.setFault(EMPTY_PARAM_VALUE_FAULT_CODE,
                                             EMPTY_PARAM_VALUE_FAULT_STRING);
                        }
                        index++;
                     }                     
                  }
               }               
            }
            else
            {
               OsSysLog::add(FAC_XMLRPC, PRI_ERR,
                             "XmlRpcDispatch::parseXmlRpcRequest no method '%s' registered",
                             methodCall.data());
               response.setMethod(methodCall);
               response.setFault(UNREGISTERED_METHOD_FAULT_CODE, UNREGISTERED_METHOD_FAULT_STRING);
               result = false;
            }
         }
         else
         {
            UtlString faultMsg(INVALID_ELEMENT_FAULT_STRING);
            faultMsg.append("methodName not found");
            OsSysLog::add(FAC_XMLRPC, PRI_ERR,
                          "XmlRpcDispatch::parseXmlRpcRequest %s", faultMsg.data());
            response.setFault(INVALID_ELEMENT, faultMsg.data());
            result = false;
         }
      }
      else
      {
         UtlString faultMsg(INVALID_ELEMENT_FAULT_STRING);
         faultMsg.append("methodCall not found");
         OsSysLog::add(FAC_XMLRPC, PRI_ERR,
                       "XmlRpcDispatch::parseXmlRpcRequest %s", faultMsg.data());
         response.setFault(INVALID_ELEMENT, faultMsg.data());
         result = false;
      }
   }
   else
   {
      OsSysLog::add(FAC_XMLRPC, PRI_ERR,
                    "XmlRpcDispatch::parseXmlRpcRequest"
                    " ill-formed XML contents in %s. Parsing error = %s",
                     requestContent.data(), doc.ErrorDesc());
      response.setFault(ILL_FORMED_CONTENTS_FAULT_CODE, ILL_FORMED_CONTENTS_FAULT_STRING);
      result = false;
   }
   
   return result;   
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

bool XmlRpcDispatch::parseValue(TiXmlNode* subNode,
                                int index,
                                UtlSList& params)
{
   bool result = false;
   UtlString paramValue;
                        
   // four-byte signed integer
   TiXmlNode* valueNode = subNode->FirstChild("i4");                  
   if (valueNode)
   {
      if (valueNode->FirstChild())
      {
         paramValue = valueNode->FirstChild()->Value();
         params.insertAt(index, new UtlInt(atoi(paramValue)));
         result = true;
      }
      else
      {
         result = false;
      }
   }
   else
   {         
      valueNode = subNode->FirstChild("int");
      if (valueNode)
      {
         if (valueNode->FirstChild())
         {
            paramValue = valueNode->FirstChild()->Value();
            params.insertAt(index, new UtlInt(atol(paramValue)));
            result = true;
         }
         else
         {
            result = false;
         }
      }
      else
      {         
         valueNode = subNode->FirstChild("i8");
         if (valueNode)
         {
            if (valueNode->FirstChild())
            {
               paramValue = valueNode->FirstChild()->Value();
               
               // We could use "atoll" here but it is obsolete, "strtoll" is the recommended function
               // See http://www.delorie.com/gnu/docs/glibc/libc_423.html .
               params.insertAt(index, new UtlLongLongInt(strtoll(paramValue, 0, 0)));
               result = true;
            }
            else
            {
               result = false;
            }
         }
         else
         {
            valueNode = subNode->FirstChild("boolean");
            if (valueNode)
            {
               if (valueNode->FirstChild())
               {
                  paramValue = valueNode->FirstChild()->Value();
                  params.insertAt(index, new UtlBool((atoi(paramValue)==1)));
                  result = true;
               }
               else
               {
                  result = false;
               }
            }
            else
            {
               // string
               // Note: In the string case, we allow a null string
               valueNode = subNode->FirstChild("string");            
               if (valueNode)
               {
                  if (valueNode->FirstChild())
                  {
                     paramValue = valueNode->FirstChild()->Value();
                     params.insertAt(index, new UtlString(paramValue));
                  }
                  else
                  {
                     params.insertAt(index, new UtlString());
                  }
                  result = true;
               }
               else
               {
                  // dateTime.iso8601
                  valueNode = subNode->FirstChild("dateTime.iso8601");            
                  if (valueNode)
                  {
                     if (valueNode->FirstChild())
                     {
                        paramValue = valueNode->FirstChild()->Value(); // need to change to UtlDateTime
                        params.insertAt(index, new UtlString(paramValue));
                        result = true;
                     }
                     else
                     {
                        result = false;
                     }
                  }
                  else
                  {
                     // struct
                     valueNode = subNode->FirstChild("struct");            
                     if (valueNode)
                     {
                        UtlHashMap* members = NULL;
                        if (parseStruct(valueNode, members))
                        {
                           params.insertAt(index, members);
                           result = true;
                        }
                     }
                     else
                     {
                        // array
                        valueNode = subNode->FirstChild("array");            
                        if (valueNode)
                        {
                           UtlSList* array = NULL;
                           if (parseArray(valueNode, array))
                           {
                              params.insertAt(index, array);
                              result = true;
                           }
                        }
                        else
                        {
                           // Default case for string
                           if (subNode->FirstChild())
                           {
                              paramValue = subNode->FirstChild()->Value();
                              params.insertAt(index, new UtlString(paramValue));
                           }
                           else
                           {
                              params.insertAt(index, new UtlString());
                           }
                           
                           result = true;
                        }
                     }                     
                  }               
               }            
            }
         }
      }
   }
      
   return result;
}


bool XmlRpcDispatch::parseStruct(TiXmlNode* subNode, UtlHashMap*& members)
{
   bool result = false;

   // struct
   UtlString name;
   UtlString paramValue;
   TiXmlNode* memberValue;
   UtlHashMap* pMembers = new UtlHashMap();
   for (TiXmlNode* memberNode = subNode->FirstChild("member");
        memberNode; 
        memberNode = memberNode->NextSibling("member"))
   {
      TiXmlNode* memberName = memberNode->FirstChild("name");
      if (memberName)
      {
         if (memberName->FirstChild())
         {
            name = memberName->FirstChild()->Value();
         }
         else
         {
            result = false;
            break;
         }
         
         memberValue = memberNode->FirstChild("value");        
         if (memberValue)
         {
            // four-byte signed integer                         
            TiXmlNode* valueElement = memberValue->FirstChild("i4");
            if (valueElement)
            {
               if (valueElement->FirstChild())
               {
                  paramValue = valueElement->FirstChild()->Value();
                  pMembers->insertKeyAndValue(new UtlString(name), new UtlInt(atoi(paramValue)));
                  result = true;
               }
               else
               {
                  result = false;
                  break;
               }
            }
            else
            {
               valueElement = memberValue->FirstChild("int");
               if (valueElement)
               {
                  if (valueElement->FirstChild())
                  {
                     paramValue = valueElement->FirstChild()->Value();
                     pMembers->insertKeyAndValue(new UtlString(name), new UtlInt(atol(paramValue)));
                     result = true;
                  }
                  else
                  {
                     result = false;
                     break;
                  }
               }
               else
               {
                  valueElement = memberValue->FirstChild("i8");
                  if (valueElement)
                  {
                     if (valueElement->FirstChild())
                     {
                        paramValue = valueElement->FirstChild()->Value();
                        pMembers->insertKeyAndValue(new UtlString(name),
                                                    new UtlLongLongInt(strtoll(paramValue, 0, 0)));
                        result = true;
                     }
                     else
                     {
                        result = false;
                        break;
                     }
                  }
                  else
                  {
                     valueElement = memberValue->FirstChild("boolean");
                     if (valueElement)
                     {
                        if (valueElement->FirstChild())
                        {
                           paramValue = valueElement->FirstChild()->Value();
                           pMembers->insertKeyAndValue(new UtlString(name),
                                                       new UtlBool((atoi(paramValue)==1)));
                           result = true;
                        }
                        else
                        {
                           result = false;
                           break;
                        }
                     }
                     else
                     {              
                        valueElement = memberValue->FirstChild("string");
                        if (valueElement)
                        {
                           if (valueElement->FirstChild())
                           {
                              paramValue = valueElement->FirstChild()->Value();
                              pMembers->insertKeyAndValue(new UtlString(name),
                                                          new UtlString(paramValue));
                           }
                           else
                           {
                              pMembers->insertKeyAndValue(new UtlString(name), new UtlString());
                           }
                           result = true;
                        }
                        else
                        {
                           valueElement = memberValue->FirstChild("dateTime.iso8601");
                           if (valueElement)
                           {
                              if (valueElement->FirstChild())
                              {
                                 paramValue = valueElement->FirstChild()->Value();
                                 pMembers->insertKeyAndValue(new UtlString(name),
                                                             new UtlString(paramValue));
                                 result = true;
                              }
                              else
                              {
                                 result = false;
                                 break;
                              }
                           }
                           else
                           {
                              valueElement = memberValue->FirstChild("struct");
                              if (valueElement)
                              {
                                 UtlHashMap* members;
                                 if (parseStruct(valueElement, members))
                                 {
                                    pMembers->insertKeyAndValue(new UtlString(name), members);
                                    result = true;
                                 }
                              }
                              else
                              {
                                 valueElement = memberValue->FirstChild("array");
                                 if (valueElement)
                                 {
                                    UtlSList* subArray;
                                    if (parseArray(valueElement, subArray))
                                    {
                                       pMembers->insertKeyAndValue(new UtlString(name), subArray);
                                       result = true;
                                    }
                                 }
                                 else
                                 {
                                    // default for string
                                    if (memberValue->FirstChild())
                                    {
                                       paramValue = memberValue->FirstChild()->Value();
                                       pMembers->insertKeyAndValue(new UtlString(name),
                                                                   new UtlString(paramValue));
                                    }
                                    else
                                    {
                                       pMembers->insertKeyAndValue(new UtlString(name),
                                                                   new UtlString());
                                    }
                                    
                                    result = true;
                                 }
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
      }
   }
   
   members = pMembers;
   return result;   
}

bool XmlRpcDispatch::parseArray(TiXmlNode* subNode, UtlSList*& array)
{
   bool arrayIsOk = true;
   
   // array
   UtlString paramValue;
   TiXmlNode* dataNode = subNode->FirstChild("data");
   if (dataNode)
   {
      UtlSList* pList = new UtlSList();
      TiXmlNode* valueNode;
      for ((valueNode = dataNode->FirstChild("value"), arrayIsOk = true);
           valueNode && arrayIsOk; 
           valueNode = valueNode->NextSibling("value"))
      {
         
         // four-byte signed integer                         
         TiXmlNode* arrayElement = valueNode->FirstChild("i4");
         if (arrayElement)
         {
            if (arrayElement->FirstChild())
            {
               paramValue = arrayElement->FirstChild()->Value();
               pList->insert(new UtlInt(atoi(paramValue)));
            }
            else
            {
               arrayIsOk = false;
            }
         }
         else
         {
            arrayElement = valueNode->FirstChild("int");
            if (arrayElement)
            {
               if (arrayElement->FirstChild())
               {
                  paramValue = arrayElement->FirstChild()->Value();
                  pList->insert(new UtlInt(atol(paramValue)));
               }
               else
               {
                  arrayIsOk = false;
               }
            }
            else
            {
               arrayElement = valueNode->FirstChild("i8");
               if (arrayElement)
               {
                  if (arrayElement->FirstChild())
                  {
                     paramValue = arrayElement->FirstChild()->Value();
                     pList->insert(new UtlLongLongInt(strtoll(paramValue, 0, 0)));
                  }
                  else
                  {
                     arrayIsOk = false;
                  }
               }
               else
               {
                  arrayElement = valueNode->FirstChild("boolean");
                  if (arrayElement)
                  {
                     if (arrayElement->FirstChild())
                     {
                        paramValue = arrayElement->FirstChild()->Value();
                        pList->insert(new UtlBool((atoi(paramValue)==1)));
                     }
                     else
                     {
                        arrayIsOk = false;
                     }
                  }
                  else
                  {              
                     arrayElement = valueNode->FirstChild("string");
                     if (arrayElement)
                     {
                        if (arrayElement->FirstChild())
                        {
                           paramValue = arrayElement->FirstChild()->Value();
                           pList->insert(new UtlString(paramValue));
                        }
                        else
                        {
                           pList->insert(new UtlString());
                        }
                     }
                     else
                     {
                        arrayElement = valueNode->FirstChild("dateTime.iso8601");
                        if (arrayElement)
                        {
                           if (arrayElement->FirstChild())
                           {
                              paramValue = arrayElement->FirstChild()->Value();
                              pList->insert(new UtlString(paramValue));
                           }
                           else
                           {
                              arrayIsOk = false;
                           }
                        }
                        else
                        {
                           arrayElement = valueNode->FirstChild("struct");
                           if (arrayElement)
                           {
                              UtlHashMap* members;
                              if ((arrayIsOk=parseStruct(arrayElement, members)))
                              {
                                 pList->insert(members);
                              }
                           }
                           else
                           {
                              arrayElement = valueNode->FirstChild("array");
                              if (arrayElement)
                              {
                                 UtlSList* subArray;
                                 if ((arrayIsOk=parseArray(arrayElement, subArray)))
                                 {
                                    pList->insert(subArray);
                                 }
                              }
                              else
                              {
                                 // default for string
                                 if (valueNode->FirstChild())
                                 {
                                    paramValue = valueNode->FirstChild()->Value();
                                    pList->insert(new UtlString(paramValue));
                                 }
                                 else
                                 {
                                    // assume an empty string element
                                    pList->insert(new UtlString());
                                 }
                              }
                           }
                        }
                     }
                  }
               }
            }
         }
      }

      if (arrayIsOk)
      {
         array = pList;
      }
      else
      {
         cleanUp(pList);
         delete pList;
      }

   }
   else
   {
      arrayIsOk=false;
   }
   
   
   return arrayIsOk;
}

void XmlRpcDispatch::cleanUp(UtlHashMap* map)
{
   UtlHashMapIterator iterator(*map);
   UtlString* pName;
   UtlContainable *key;
   UtlContainable *value;
   while ((pName = (UtlString *) iterator()))
   {
      key = map->removeKeyAndValue(pName, value);
      UtlString paramType(value->getContainableType());
      if (paramType.compareTo("UtlHashMap") == 0)
      {
         UtlHashMap* pMap = (UtlHashMap *) value;
         cleanUp(pMap);
         delete pMap;
      }
      else
      {
         if (paramType.compareTo("UtlSList") == 0)
         {
            UtlSList* pList = (UtlSList *) value;
            cleanUp(pList);
            delete pList;
         }
         else
         {
            delete value;
         }
      }
      
      delete pName;  
   }
}

void XmlRpcDispatch::cleanUp(UtlSList* array)
{
   UtlSListIterator iterator(*array);
   UtlContainable *value;
   while ((value = iterator()))
   {
      value = array->remove(value);
      UtlString paramType(value->getContainableType());
      if (paramType.compareTo("UtlHashMap") == 0)
      {
         UtlHashMap* pMap = (UtlHashMap *) value;
         cleanUp(pMap);
         delete pMap;
      }
      else
      {
         if (paramType.compareTo("UtlSList") == 0)
         {
            UtlSList* pList = (UtlSList *) value;
            cleanUp(pList);
            delete pList;
         }
         else
         {
            delete value;
         }
      }
   }
}

/* ============================ FUNCTIONS ================================= */

