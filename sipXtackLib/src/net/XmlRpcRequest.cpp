//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsSysLog.h>
#include "net/XmlRpcRequest.h"
#include "config.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const int XML_RPC_TIMEOUT = (3*1000);

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
XmlRpcRequest::XmlRpcRequest(Url& uri, const char* methodName)
   : mUrl(uri)
   , mpHttpRequest(new HttpMessage())
   , mpRequestBody(new XmlRpcBody())
{
   UtlString path;
   if( uri.getPath( path ) == FALSE )
   {
      path = "/RPC2";
   }
   // Start to contruct the HTTP message
   mpHttpRequest->setFirstHeaderLine(HTTP_POST_METHOD, path, HTTP_PROTOCOL_VERSION_1_1);
   mpHttpRequest->addHeaderField("Accept", "text/xml");
   mpHttpRequest->setUserAgentField(PACKAGE_NAME "_xmlrpc/" PACKAGE_VERSION);

   // Start to construct the XML-RPC body
   mpRequestBody->append(BEGIN_METHOD_CALL BEGIN_METHOD_NAME);
   mpRequestBody->append(methodName);
   mpRequestBody->append(END_METHOD_NAME BEGIN_PARAMS);
}

// Destructor
XmlRpcRequest::~XmlRpcRequest()
{
   if (mpHttpRequest)
   {
      delete mpHttpRequest;
   }
   if (mpRequestBody) // should not happen, because it should have been passed to HttpRequest
   {
      delete mpRequestBody;
   }
}


bool XmlRpcRequest::execute(XmlRpcResponse& response)
{
   bool result = false;

   // End of constructing the XML-RPC body
   mpRequestBody->append(END_PARAMS END_METHOD_CALL);

   if (OsSysLog::willLog(FAC_XMLRPC, PRI_INFO))
   {
      UtlString logString;
      ssize_t   logLength;
      mpRequestBody->getBytes(&logString, &logLength);
      if (logString.length() > XmlRpcBody::MAX_LOG)
      {
         logString.remove(XmlRpcBody::MAX_LOG);
         logString.append("\n...");
      }
      UtlString urlString;
      mUrl.toString(urlString);
      OsSysLog::add(FAC_XMLRPC, PRI_INFO,
                    "XmlRpcRequest::execute XML-RPC to '%s' request =\n%s",
                    urlString.data(),
                    logString.data());
   }

   mpHttpRequest->setContentLength(mpRequestBody->getLength());
   mpHttpRequest->setBody(mpRequestBody);
   mpRequestBody = NULL; // the HttpMessage now owns the request body

   // Create an empty response object and sent the built up request
   // to the XML-RPC server
   HttpMessage httpResponse;

   int statusCode = httpResponse.get(mUrl,*mpHttpRequest,XML_RPC_TIMEOUT,true /* persist conn */ );
   if (statusCode/100 == 2)
   {
      UtlString bodyString;
      ssize_t   bodyLength;

      httpResponse.getBody()->getBytes(&bodyString, &bodyLength);

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

      if (response.parseXmlRpcResponse(bodyString))
      {
         result = true;
         OsSysLog::add(FAC_XMLRPC, PRI_INFO,
                       "XmlRpcRequest::execute XML-RPC received valid response = \n%s",
                       logString.data());
      }
      else
      {
         OsSysLog::add(FAC_XMLRPC, PRI_ERR,
                       "XmlRpcRequest::execute XML-RPC received fault response = \n%s",
                       logString.data());
      }
   }
   else if (statusCode == -1)
   {
      response.setFault(XmlRpcResponse::ConnectionFailure, CONNECTION_FAILURE_FAULT_STRING);

      OsSysLog::add(FAC_XMLRPC, PRI_ERR,
                    "XmlRpcRequest::execute http connection failed");
   }
   else // some non-2xx HTTP response
   {
      UtlString statusText;

      httpResponse.getResponseStatusText(&statusText);
      response.setFault(XmlRpcResponse::HttpFailure, statusText.data());

      OsSysLog::add(FAC_XMLRPC, PRI_INFO,
                    "XmlRpcRequest::execute http request failed; status = %d %s",
                    statusCode, statusText.data());
   }

   return result;
}

/* ============================ MANIPULATORS ============================== */


/* ============================ ACCESSORS ================================= */

bool XmlRpcRequest::addParam(const UtlContainable* value)
{
   bool result = false;
   mpRequestBody->append(BEGIN_PARAM);

   result = mpRequestBody->addValue(value);

   mpRequestBody->append(END_PARAM);

   return result;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */


/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */
