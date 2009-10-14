//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _FILERPC_H_
#define _FILERPC_H_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "net/XmlRpcMethod.h"

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
 The base class for all File XML-RPC Methods.
 */
class FileRpcMethod : public XmlRpcMethod
{
public:

   /// The XmlRpcMethod::Get registered with the dispatcher for this XML-RPC Method.
   static XmlRpcMethod* get();

   /// Destructor
   virtual ~FileRpcMethod() {}

   /// Get the name of the XML-RPC method.
   virtual const char* name() = 0;

   /// Register this method with the XmlRpcDispatch object so it can be called.
   static void registerSelf(SipxRpc & sipxRpcImpl);

   typedef enum
   {
      UnconfiguredPeer = 300, ///< caller is not a configured peer of this server
      InvalidParameter,       ///< missing parameter, extra parameter(s), or invalid type
   } FaultCode;

protected:

   /// The name of the XML-RPC method.
   static const char* METHOD_NAME;

   /// The name of the XML-RPC 'callingHostname' parameter.
   static const char* PARAM_NAME_CALLING_HOST;

   /// The name of the XML-RPC 'fileName' parameter.
   static const char* PARAM_NAME_FILE_NAME;

   /// The name of the XML-RPC 'filePermissions' parameter.
   static const char* PARAM_NAME_FILE_PERMISSIONS;

   /// The name of the XML-RPC 'fileData' parameter.
   static const char* PARAM_NAME_FILE_DATA;

   /// constructor
   FileRpcMethod();

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
   FileRpcMethod(const FileRpcMethod& nocopy);

   /// no assignment operator
   FileRpcMethod& operator=(const FileRpcMethod& noassignment);
};


/**
 Replaces/Create a specified file (path and filename) on the system.

 \par
 <b>Method Name: File.replace</b>

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
       <td>fileName</td>
       <td>The full path and filename of the file to add or replace.</td>
    </tr>
    <tr>
       <td>int</td>
       <td>filePermissions</td>
       <td>The permissions to place on the file for user, group and other.
           This is typically specified as an Octet but we are restricted
           to the XML-RPC types.  The Octet value must be converted to an integer
           value and passed in.</td>
    </tr>
    <tr>
       <td>string</td>
       <td>filedata</td>
       <td>The actual contents of the file in base 64 encoding.</td>
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
       <td>boolean</td>
       <td>True if the specified file was successfully replaced or created.  Returns
           false otherwise.</td>
    </tr>
 </table>
*/

class FileRpcReplaceFile :  public FileRpcMethod
{
public:

   /// The XmlRpcMethod::Get registered with the dispatcher for this XML-RPC Method.
   static XmlRpcMethod* get();

   /// Destructor.
   virtual ~FileRpcReplaceFile() {};

   /// Get the name of the XML-RPC method.
   virtual const char* name();

   /// Register this method handler with the XML-RPC dispatcher.
   static void registerSelf(SipxRpc & sipxRpcImpl);

protected:

   /// The name of the XML-RPC method.
   static const char* METHOD_NAME;

   /// Constructor.
   FileRpcReplaceFile();

   /// The execution of this XML-RPC Method.
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status                   ///< XML-RPC method execution status
                        );
private:

   /// Create/Replace the file.
   bool replicateFile(UtlString& path_and_name,
                      UtlInt& file_permissions,
                      UtlString& file_content,  ///< base64 of file contents
                      UtlString& errorMsg       ///< set iff return == false
                      );

};

#endif // _FILERPC_H_
