//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _XMLRPCDISPATCH_H_
#define _XMLRPCDISPATCH_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsBSem.h>
#include <xmlparser/tinyxml.h>
#include "net/HttpService.h"
#include "net/HttpServer.h"
#include "net/XmlRpcMethod.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS

// Private class to contain XmlRpcMethod and user data for each methodName
class XmlRpcMethodContainer : public UtlContainable
{
public:
   XmlRpcMethodContainer(const char* methodName);

   virtual ~XmlRpcMethodContainer();

   virtual UtlContainableType getContainableType() const;

   static const UtlContainableType TYPE;

   virtual unsigned int hash() const;

   int compareTo(const UtlContainable *b) const;

   void setData(XmlRpcMethod::Get* method, void* userData);

   void getData(XmlRpcMethod::Get*& method, void*& userData);

   void getName(UtlString& methodName);

private:

   UtlString          mMethodName; ///< record to be put in responses
   void*              mpUserData;  ///< unique instance data
   XmlRpcMethod::Get* mpMethod;    ///< factory to construct method instance

   //! DISALLOWED accidental copying
   XmlRpcMethodContainer(const XmlRpcMethodContainer& rXmlRpcMethodContainer);
   XmlRpcMethodContainer& operator=(const XmlRpcMethodContainer& rhs);
};

/**
 * A XmlRpcDispatch is a object that monitors the incoming
 * XML-RPC requests, parses XmlRpcRequest messages, invokes the correct
 * XmlRpcMethod calls, and sends back the corresponding XmlRpcResponse responses.
 * If the correspnding method does not exit, it will send back a 404 response.
 * Otherwise, it will always send back a 200 OK response with XmlRpcResponse
 * content.
 *
 * For each XML-RPC server, it needs to instantiate a XmlRpcDispatch object first,
 * and then register each service method using addMethod() or remove the method
 * using removeMethod().
 */

class XmlRpcDispatch : public HttpService
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   static const char* DEFAULT_URL_PATH;

/* ============================ CREATORS ================================== */

   /// Create an XML-RPC dispatcher on an existing HttpServer.
   XmlRpcDispatch(HttpServer* httpServer,       ///< existing HttpServer this XML RPC should use
                  const char* uriPath = DEFAULT_URL_PATH ///< uri path
                  );
   /**<
    * This is the preferred constructor.
    */

   /// Create an XML-RPC dispatcher that starts and manages its own HttpServer.
   XmlRpcDispatch(int httpServerPort,           ///< port number for HttpServer
                  bool isSecureServer,          ///< option for HTTP or HTTPS
                  const char* uriPath = DEFAULT_URL_PATH,         ///< uri path
                  const char* bindIp = NULL     /// Default bind IP
                  );
   /**<
    * @deprecated Use the preferred constructor above for any new usages.
    */

   /// Destructor.
   virtual ~XmlRpcDispatch();

/* ============================ MANIPULATORS ============================== */

   /// Handler for XML-RPC requests
   void processRequest(const HttpRequestContext& requestContext,
                       const HttpMessage& request,
                       HttpMessage*& response );

/* ============================ ACCESSORS ================================= */

   /// Add a method to the RPC dispatch
   void addMethod(const char* methodName, XmlRpcMethod::Get* method, void* userData = NULL);

   /// Remove a method from the RPC dispatch by name
   void removeMethod(const char* methodName);

   /// Remove all method from the RPC dispatch
   void removeAllMethods();

   /// Return the HTTP server that services RPC requests
   HttpServer* getHttpServer();

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   friend class XmlRpcTest;

   /// Parse the XML-RPC request
   bool parseXmlRpcRequest(const UtlString& requestContent, ///< HttpBody of received request
                           XmlRpcMethodContainer*& method,  ///< output method data
                           UtlSList& params,                ///< output parameter items
                           XmlRpcResponse& response         /**< response
                                                             * if return is false, this is a
                                                             * fault message ready to send.
                                                             */
                           );
   /**< @returns false if request did not parse cleanly */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   /// Http server for handling the HTTP POST request
   HttpServer* mpHttpServer;

   UtlHashMap  mMethods; /**< Map of the registered XML-RPC methods
                          *   Key is a UtlString of the method name,
                          *   Value is an XmlRpcMethodContainer
                          */

   /// Synchronizes
   OsBSem mLock;

   /// whether or not the HttpServer should be shut down and deleted in the destructor.
   bool mManageHttpServer;

   /// Disabled copy constructor
   XmlRpcDispatch(const XmlRpcDispatch& rXmlRpcDispatch);

   /// Disabled assignment operator
   XmlRpcDispatch& operator=(const XmlRpcDispatch& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _XMLRPCDISPATCH_H_
