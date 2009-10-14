//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _IMDBRPC_H_
#define _IMDBRPC_H_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "net/XmlRpcMethod.h"
#include "sipdb/ResultSet.h"

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
 The base class for all data IMDB XML-RPC Methods.
 */
class ImdbRpcMethod : public XmlRpcMethod
{
public:

   /// The XmlRpcMethod::Get registered with the dispatcher for this XML-RPC Method.
   static XmlRpcMethod* get();

   /// Destructor
   virtual ~ImdbRpcMethod() {}

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

   /// The name of the XML-RPC 'IMDBTable' parameter.
   static const char* PARAM_NAME_IMDB_TABLE;

   /// The name of the XML-RPC 'IMDBTableData' parameter.
   static const char* PARAM_NAME_IMDB_TABLE_DATA;

   /// constructor
   ImdbRpcMethod();

   /// Common method for registering with the XML-RPC dispatcher.
   static void registerMethod(const char*       methodName,
                              XmlRpcMethod::Get getMethod,
                              SipxRpc&          sipxRpcImpl
                              );

   /// The execute method called by XmlRpcDispatch.
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status                   ///< XML-RPC method execution status
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


   /// Store all records from an IMDB Table to an XML file for reloading.
   void storeTable( const UtlString& tableName);

   /// Add a record to an IMDB Table.
   UtlBoolean insertTableRecord(UtlString& tableName, const UtlHashMap& tableRecord);

private:
   /// no copy constructor
   ImdbRpcMethod(const ImdbRpcMethod& nocopy);

   /// no assignment operator
   ImdbRpcMethod& operator=(const ImdbRpcMethod& noassignment);
};


/**
 Replaces a specified table in the IMDB with the records/rows supplied.

 \par
 <b>Method Name: ImdbTable.replace</b>

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
       <td>IMDBTableName</td>
       <td>The name of the IMDB table to replace with the records supplied.  Current
           tables supported are "credential", "alias", "caller-alias", "permission", "extension".</td>
    </tr>
    <tr>
       <td>array</td>
       <td>IMDBTableRecords</td>
       <td>Array of table records to be inserted (may be empty, which clears the table).
           Records are represented using XML-RPC structs</td>
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
       <td>True if the specified IMDB table was successfully cleared and populated with
           the new records.  Returns false otherwise.</td>
    </tr>
 </table>
*/

class ImdbRpcReplaceTable :  public ImdbRpcMethod
{
public:

   /// The XmlRpcMethod::Get registered with the dispatcher for this XML-RPC Method.
   static XmlRpcMethod* get();

   /// Destructor.
   virtual ~ImdbRpcReplaceTable() {};

   /// Get the name of the XML-RPC method.
   virtual const char* name();

   /// Register this method handler with the XML-RPC dispatcher.
   static void registerSelf(SipxRpc & sipxRpcImpl);

protected:

   /// The name of the XML-RPC method.
   static const char* METHOD_NAME;

   /// Constructor.
   ImdbRpcReplaceTable();

   /// The execution of this XML-RPC Method.
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status                   ///< XML-RPC method execution status
                        );

private:

   /// Clear out all records from an IMDB Table.
   void clearTable( UtlString& tableName);

};



/**
 Retrieve/Read a specified table in the IMDB.

 \par
 <b>Method Name: ImdbTable.read</b>

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
       <td>IMDBTableName</td>
       <td>The name of the IMDB table to replace with the records supplied.  Current
           tables supported are "credential", "alias", "caller-alias", "permission", "extension".</td>
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
       <td>array</td>
       <td>Array of table records retrieved.  Records are represented using XML-RPC structs (i.e. HashMap )</td>
    </tr>
 </table>
*/

class ImdbRpcRetrieveTable :  public ImdbRpcMethod
{
public:

   /// The XmlRpcMethod::Get registered with the dispatcher for this XML-RPC Method.
   static XmlRpcMethod* get();

   /// Destructor.
   virtual ~ImdbRpcRetrieveTable() {};

   /// Get the name of the XML-RPC method.
   virtual const char* name();

   /// Register this method handler with the XML-RPC dispatcher.
   static void registerSelf(SipxRpc & sipxRpcImpl);

protected:

   /// The name of the XML-RPC method.
   static const char* METHOD_NAME;

   /// Constructor.
   ImdbRpcRetrieveTable();

   /// The execution of this XML-RPC Method.
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status                   ///< XML-RPC method execution status
                        );

private:

   /// Read all records from an IMDB Table.
   void readTable( UtlString& tableName, ResultSet* imdb_table);

};



/**
 Adds records to a specified table in the IMDB.

 \par
 <b>Method Name: ImdbTable.insertRows</b>

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
       <td>IMDBTableName</td>
       <td>The name of the IMDB table to which to add the records.  Current
           tables supported are "credential", "alias", "caller-alias", "permission", "extension".</td>
    </tr>
    <tr>
       <td>array</td>
       <td>IMDBTableRecords</td>
       <td>Array of table records to be added.  Records are represented using XML-RPC structs</td>
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
       <td>True if the records were successfully added to the specified IMDB table.
           Returns false otherwise.</td>
    </tr>
 </table>
*/

class ImdbRpcAddTableRecords :  public ImdbRpcMethod
{
public:

   /// The XmlRpcMethod::Get registered with the dispatcher for this XML-RPC Method.
   static XmlRpcMethod* get();

   /// Destructor.
   virtual ~ImdbRpcAddTableRecords() {};

   /// Get the name of the XML-RPC method.
   virtual const char* name();

   /// Register this method handler with the XML-RPC dispatcher.
   static void registerSelf(SipxRpc & sipxRpcImpl);

protected:

   /// The name of the XML-RPC method.
   static const char* METHOD_NAME;

   /// Constructor.
   ImdbRpcAddTableRecords();

   /// The execution of this XML-RPC Method.
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status                   ///< XML-RPC method execution status
                        );

private:

};



/**
 Deletes records from a specified table in the IMDB.

 \par
 <b>Method Name: ImdbTable.deleteRows</b>

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
       <td>IMDBTableName</td>
       <td>The name of the IMDB table from which to delete the records.  Current
           tables supported are "credential", "alias", "caller-alias", "permission", "extension".</td>
    </tr>
    <tr>
       <td>array</td>
       <td>IMDBTableRecordKey</td>
       <td>Array of row keys to be used for deletion of IMDB Table records. Records are represented using XML-RPC structs</td>
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
       <td>True if the records were successfully deleted from the specified IMDB table.
           Returns false otherwise.</td>
    </tr>
 </table>
*/

class ImdbRpcDeleteTableRecords :  public ImdbRpcMethod
{
public:

   /// The XmlRpcMethod::Get registered with the dispatcher for this XML-RPC Method.
   static XmlRpcMethod* get();

   /// Destructor.
   virtual ~ImdbRpcDeleteTableRecords() {};

   /// Get the name of the XML-RPC method.
   virtual const char* name();

   /// Register this method handler with the XML-RPC dispatcher.
   static void registerSelf(SipxRpc & sipxRpcImpl);

protected:

   /// The name of the XML-RPC method.
   static const char* METHOD_NAME;

   /// Constructor.
   ImdbRpcDeleteTableRecords();

   /// The execution of this XML-RPC Method.

   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status                   ///< XML-RPC method execution status
                        );

private:

   /// Delete a record from the IMDB Table.
   UtlBoolean deleteTableRecord( UtlString& tableName, const UtlHashMap& tableRecordKeys);

};

#endif // _IMDBRPC_H_
