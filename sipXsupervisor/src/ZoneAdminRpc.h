//
// Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _ZONEADMINRPC_H_
#define _ZONEADMINRPC_H_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "net/XmlRpcMethod.h"
#include "os/OsFS.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipxRpc;

/**
 The base class for all Software Admin XML-RPC Methods.
 */
class ZoneAdminRpcMethod : public XmlRpcMethod
{
public:

   /// The XmlRpcMethod::Get registered with the dispatcher for this XML-RPC Method.
   static XmlRpcMethod* get();

   /// Destructor
   virtual ~ZoneAdminRpcMethod() {}

   /// Get the name of the XML-RPC method.
   virtual const char* name() = 0;

   /// Register this method with the XmlRpcDispatch object so it can be called.
   static void registerSelf(SipxRpc & sipxRpcImpl);

   typedef enum
   {
      UnconfiguredPeer = 300, ///< caller is not a configured peer of this server
      InvalidParameter,       ///< missing parameter, extra parameter(s), or invalid type
      FailureToLaunch,        ///< failed to launch command.
      FailureToEncode,        ///< failed to encode file.
      DuplicateInstance       ///< duplicate instance of the process running.
   } FaultCode;

protected:

   /// The name of the XML-RPC method.
   static const char* METHOD_NAME;

   /// The name of the XML-RPC 'callingHostname' parameter.
   static const char* PARAM_NAME_CALLING_HOST;
   static const char* PARAM_NAME_COMMAND;

   /// constructor
   ZoneAdminRpcMethod();

   /// Common method for registering with the XML-RPC dispatcher.
   static void registerMethod(const char*       methodName,
                              XmlRpcMethod::Get getMethod,
                              SipxRpc&          sipxRpcImpl
                              );

   /// The execute method called by XmlRpcDispatch.
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList&                 params,         ///< request param list
                        void*                     userData,       ///< user data
                        XmlRpcResponse&           response,       ///< request response
                        ExecutionStatus&          status          ///< XML-RPC method execution status
                        ) = 0;

   /// Common method to do caller peer validation.
   bool validCaller(const HttpRequestContext& requestContext, ///< request context
                    const UtlString&          peerName,       ///< name of the calling host
                    XmlRpcResponse&           response,       ///< response to put fault in
                    const SipxRpc&            sipxRpcImpl,    ///< the sipXsupervisor
                    const char*               callingMethod   ///< calling xml rpc method name
                    );

   /// Common method to do check for duplicate instance of a process.
   bool duplicateProcess(const char*     command,
                         XmlRpcResponse& response,
                         ExecutionStatus& status
                        );

   /// Common method to check the activity of a process instance.
   bool isProcessActive(const char* command);

   /// Common method to check validity of subcommand and build output filenames.
   bool buildOutputFiles(const UtlString&     command,
                         UtlString&      stdoutfn,
                         UtlString&      stderrfn
                        );

   /**<
    * Ensures that:
    *  - the peerName is one of the configured allowed hosts; and
    *  - the HttpRequestContext indicates that the connection is indeed from that host.
    *
    * If the caller is not a valid, then the XMLRPC 'response' is populated with an
    * appropriate message.
    *
    * \return True is the caller is a valid (allowed) peer, false otherwise.
    */

   /// Handle missing parameters for the execute method.
   void handleMissingExecuteParam(const char* methodName,  ///< name of the called XmlRpc method
                                  const char* paramName,   ///< name of problematic parameter
                                  XmlRpcResponse& response,///< response (fault is set on this)
                                  ExecutionStatus& status  ///< set to FAILED
                                  );

   /// Handle extra parameters for the execute method.
   void handleExtraExecuteParam(const char* methodName,  ///< name of the called XmlRpc method
                                XmlRpcResponse& response,///< response (fault is set on this)
                                ExecutionStatus& status  ///< set to FAILED
                                );

private:
   /// no copy constructor
   ZoneAdminRpcMethod(const ZoneAdminRpcMethod& nocopy);

   /// no assignment operator
   ZoneAdminRpcMethod& operator=(const ZoneAdminRpcMethod& noassignment);
};


/**
 Generates the DNS zone file for the calling host.

 \par
 <b>Method Name: ZoneAdmin.generateDns</b>

 \par
 <b>Input:</b>
 <table border="1">
    <tr>
       <td><b>Data type</b></td>
       <td><b>Name</b></td>
       <td><b>Description</b></td>
    </tr>
    <tr>
       <td>string</td>
       <td>callingHostname</td>
       <td>The FQDN of the calling host to be checked as an SSL trusted peer <b>and</b>
           against an explicit list of hosts allowed to make requests.</td>
    </tr>
    <tr>
       <td>string</td>
       <td>subCommand</td>
       <td>The parameters for the zone generation command.</td>
    </tr>
 </table>

 \par
 <b>Return Value:</b>
 <table border="1">
    <tr>
       <td><b>Data type</b></td>
       <td><b>Description</b></td>
    </tr>
    <tr>
       <td>void</td>
       <td>No return code.</td>
    </tr>
 </table>
*/

class ZoneAdminRpcExec :  public ZoneAdminRpcMethod
{
public:

   /// The XmlRpcMethod::Get registered with the dispatcher for this XML-RPC Method.
   static XmlRpcMethod* get();

   /// Destructor.
   virtual ~ZoneAdminRpcExec() {};

   /// Get the name of the XML-RPC method.
   virtual const char* name();

   /// Register this method handler with the XML-RPC dispatcher.
   static void registerSelf(SipxRpc & sipxRpcImpl);

protected:

   /// The name of the XML-RPC method.
   static const char* METHOD_NAME;

   /// Constructor.
   ZoneAdminRpcExec();

   /// The execution of this XML-RPC Method.
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status                   ///< XML-RPC method execution status
                        );

};

#endif // _ZONEADMINRPC_H_
