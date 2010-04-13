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

/// Create an XML-RPC dispatcher on an existing HttpServer.
XmlRpcDispatch::XmlRpcDispatch(HttpServer* httpServer, ///< existing HttpServer to use
                               const char* uriPath     ///< uri path
                               )
   : mpHttpServer(httpServer)
   , mLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
   , mManageHttpServer(false)
{
   if (mpHttpServer) // might be NULL in unit tests
   {
      // Add the XmlRpcDispatch to the HttpServer
      mpHttpServer->addHttpService(uriPath, (HttpService*)this);
   }
}


// Old Constructor
XmlRpcDispatch::XmlRpcDispatch(int httpServerPort,
                               bool isSecureServer,
                               const char* uriPath,
                               const char* bindIp)
   : mLock(OsBSem::Q_PRIORITY, OsBSem::FULL)
   , mManageHttpServer(true)
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
                                 NULL, // no valid ip address list
                                 true  // use persistent http connections
                                 );

   mpHttpServer->start();

   // Add the XmlRpcDispatch to the HttpServer
   mpHttpServer->addHttpService(uriPath, (HttpService*)this);
}


// Destructor
XmlRpcDispatch::~XmlRpcDispatch()
{
   // HTTP server shutdown
   if (mManageHttpServer && mpHttpServer)
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

   UtlString logString;
   if (bodyString.length() > XmlRpcBody::MAX_LOG)
   {
      logString.append(bodyString, 0, XmlRpcBody::MAX_LOG);
      logString.append("\n...");
   }
   else
   {
      logString = bodyString;
   }
   OsSysLog::add(FAC_XMLRPC, PRI_INFO,
                 "XmlRpcDispatch::processRequest requestBody = \n%s",
                 logString.data());

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
         XmlRpcBody::deallocateContainedValues(&params);

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

   logString.remove(0);
   if (bodyString.length() > XmlRpcBody::MAX_LOG)
   {
      logString.append(bodyString, 0, XmlRpcBody::MAX_LOG);
      logString.append("\n...");
   }
   else
   {
      logString = bodyString;
   }

   OsSysLog::add(FAC_XMLRPC, PRI_INFO,
                 "XmlRpcDispatch::processRequest method '%s' response status=%s\n%s",
                 methodName.data(),
                 XmlRpcMethod::ExecutionStatusString(status),
                 logString.data()
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


void XmlRpcDispatch::removeAllMethods()
{
   mLock.acquire();
   mMethods.removeAll();
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
                        UtlString parseErrorMsg;
                        UtlContainable* param = XmlRpcBody::parseValue(subNode, 0, parseErrorMsg);
                        if (param)
                        {
                           params.append(param);
                           index++;
                        }
                        else
                        {
                           char errorLoc[200];
                           sprintf(errorLoc," in param %d",
                                   index);
                           OsSysLog::add(FAC_XMLRPC, PRI_ERR,
                                         "XmlRpcDispatch::parseXmlRpcRequest"
                                         " invalid <value> contents %s of %s",
                                         errorLoc, requestContent.data());
                           parseErrorMsg.append(errorLoc);
                           response.setFault(EMPTY_PARAM_VALUE_FAULT_CODE,
                                             parseErrorMsg.data());
                           result=false;
                        }
                     }
                     else
                     {
                        char errorLoc[200];
                        sprintf(errorLoc,"no <value> element in param %d.",
                                index);
                        OsSysLog::add(FAC_XMLRPC, PRI_ERR,
                                      "XmlRpcDispatch::parseXmlRpcRequest %s of: %s",
                                      errorLoc, requestContent.data());
                        response.setFault(EMPTY_PARAM_VALUE_FAULT_CODE,
                                          errorLoc);
                        result=false;
                     }
                  }
               }
               else
               {
                  OsSysLog::add(FAC_XMLRPC, PRI_ERR,
                                "XmlRpcDispatch::parseXmlRpcRequest no <params> element found");
                  response.setMethod(methodCall);
                  response.setFault(ILL_FORMED_CONTENTS_FAULT_CODE, "no <params> element");
                  result = false;
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



/* ============================ FUNCTIONS ================================= */
