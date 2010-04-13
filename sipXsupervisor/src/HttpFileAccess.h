//
// Copyright (C) 2008 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _HTTPFILEACCESS_H_
#define _HTTPFILEACCESS_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "net/HttpService.h"
#include "SipxRpc.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class HttpServer;

/// Provide access to files as allowed by process definitions.
/**
 * This class is registered a an HttpService with the HttpServer provided by the sipXsupervisor.
 * It allows read access to any resource or log file defined by any process definition.
 */
class HttpFileAccess : public HttpService
{
  public:

   /// Construct the service object and register it with the HttpServer
   HttpFileAccess(HttpServer* httpServer,
                  SipxRpc*    rpcService
                  );

   /// destructor
   virtual ~HttpFileAccess();

   /// Provide access to files as allowed by process definitions.
   virtual void processRequest(const HttpRequestContext& requestContext,
                               const HttpMessage& request,
                               HttpMessage*& response
                               );

  protected:

   static const size_t MAX_FILE_CHUNK_SIZE;
   SipxRpc* mSipxRpc;  ///< RPC service handles checking for peers.

  private:

   void sendFile(const UtlString& path,
                 const UtlString& peerName,
                 const HttpRequestContext& requestContext,
                 const HttpMessage& request,
                 HttpMessage*& response
                 );

   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   HttpFileAccess(const HttpFileAccess& nocopyconstructor);

   /// There is no assignment operator.
   HttpFileAccess& operator=(const HttpFileAccess& noassignmentoperator);
   // @endcond
};

#endif // _HTTPFILEACCESS_H_
