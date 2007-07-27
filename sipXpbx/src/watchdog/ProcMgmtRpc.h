// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////
#ifndef _PROCMGMTRPC_H_
#define _PROCMGMTRPC_H_

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
class WatchDog;

/**
 The base class for all Process Management XML-RPC Methods.
 */
class ProcMgmtRpcMethod : public XmlRpcMethod
{
public:
   
   /// The XmlRpcMethod::Get registered with the dispatcher for this XML-RPC Method.  
   static XmlRpcMethod* get();

   /// Destructor
   virtual ~ProcMgmtRpcMethod() {}
   
   /// Get the name of the XML-RPC method.
   virtual const char* name() = 0;

   /// Register this method with the XmlRpcDispatch object so it can be called.
   static void registerSelf(WatchDog & watchdog);

   typedef enum
   {
      UnconfiguredPeer = 300, ///< caller is not a configured peer of this server
      InvalidParameter,       ///< missing parameter, extra parameter(s), or invalid type
   } FaultCode;

protected:

   /// The maximum amount of time to block waiting for a single process to undergo a state change.
   static int SINGLE_BLOCK_MAX;

   /// The maximum amount of time to block waiting for a list of processes to undergo a state change.
   static int LIST_BLOCK_MAX;

   /// The name of the XML-RPC method.
   static const char* METHOD_NAME;
   
   /// The name of the XML-RPC 'callingHostname' parameter.
   static const char* PARAM_NAME_CALLING_HOST;

   /// The name of the XML-RPC 'alias' parameter.
   static const char* PARAM_NAME_ALIAS;

   /// The name of the XML-RPC 'blockForStateChange' parameter.
   static const char* PARAM_NAME_BLOCK;

   /// The name of the XML-RPC 'pid' parameter.
   static const char* PARAM_NAME_PID;

   /// constructor 
   ProcMgmtRpcMethod();

   /// Common method for registering with the XML-RPC dispatcher.
   static void registerMethod(const char*       methodName,
                              XmlRpcMethod::Get getMethod,
                              WatchDog&         watchdog
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
                    const WatchDog&           watchdog,       ///< the watchdog
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

   /// Sets the user requested state of a single process to the specified state, and blocks for the result.
   bool executeSetUserRequestState(const HttpRequestContext& requestContext, ///< request context
                                   UtlSList& params,                         ///< request param list
                                   void* userData,                           ///< user data
                                   XmlRpcResponse& response,                 ///< request response
                                   ExecutionStatus& status,                  ///< XML-RPC method execution status
                                   const int request_state                   ///< the state to set
                                   );
   /**<
     Returns true on success, false otherwise.
    */
   
   /// Calls blockForProcessStateMatchList(), but the list to block for contains only the single specified process.
   bool blockForProcessStateMatch(WatchDog* pWatchDog, 
                                  const UtlString& alias, 
                                  const int request_state, 
                                  const int max_secs = SINGLE_BLOCK_MAX);
   
   /// Calls blockForProcessRestartList(), but the list to block for contains only the single specified process.
   bool blockForProcessRestart(WatchDog* pWatchDog, 
                               const UtlString& alias, 
                               const PID original_pid, 
                               const int max_secs = SINGLE_BLOCK_MAX);
   
   /// Sets the user requested state of all processes to the specified state, and blocks for the result. 
   bool executeSetUserRequestStateAll(const HttpRequestContext& requestContext, ///< request context
                                      UtlSList& params,                         ///< request param list
                                      void* userData,                           ///< user data
                                      XmlRpcResponse& response,                 ///< request response
                                      ExecutionStatus& status,                  ///< XML-RPC method execution status
                                      const int request_state                   ///< the state to set
                                      );
   /**<
     Returns true on success, false otherwise.
    */

   /// Blocks for the update of the process's current state to the specified requested state.
   void blockForProcessStateMatchList(WatchDog* pWatchDog, 
                                      UtlHashMap& process_results, 
                                      const int request_state, 
                                      const int max_secs = LIST_BLOCK_MAX);
   /**<
     Only does this for the aliases in 'process_results' with a true result.  Any that are not 
     updated before timeout will have their 'process_results' entry set to a false result.
    */
   
   /// Blocks for the restart of the process's.
   void blockForProcessRestartList(WatchDog* pWatchDog, 
                                   UtlHashMap& process_results, 
                                   const UtlHashMap& original_pids, 
                                   const int max_secs = LIST_BLOCK_MAX);
  /**<
     Only does this for the aliases in 'process_results' with a true result.  Any that are not
     restarted  before timeout will have their 'process_results' entry set to a false result.
    */
   
private:
   /// no copy constructor
   ProcMgmtRpcMethod(const ProcMgmtRpcMethod& nocopy);

   /// no assignment operator
   ProcMgmtRpcMethod& operator=(const ProcMgmtRpcMethod& noassignment);
};

/**
 Returns the current state of all processes being monitored by the watchdog.

 \par
 <b>Method Name: ProcMgmtRpc.getStateAll</b>

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
 </table>
 
 \par
 <b>Return Value:</b>
 <table border="1">
    <tr>
       <td><b>Data type</b></td>
       <td><b>Description</b></td>
    </tr>
    <tr>
       <td>string struct</td>
       <td>The current status of each process, indexed by process alias.  (The alias 
           is the "name" attribute of the "monitor-process" element in the process's 
           SIPX_CONFDIR/process.d/___.process.xml configuration file.)</td>
    </tr>
 </table>
 */
class ProcMgmtRpcGetStateAll : public ProcMgmtRpcMethod
{
public:

   /// The XmlRpcMethod::Get registered with the dispatcher for this XML-RPC Method.  
   static XmlRpcMethod* get();

   /// Destructor.
   virtual ~ProcMgmtRpcGetStateAll() {};

   /// Get the name of the XML-RPC method.
   virtual const char* name();

   /// Register this method handler with the XML-RPC dispatcher.
   static void registerSelf(WatchDog & watchdog);

protected:

   /// The name of the XML-RPC method.
   static const char* METHOD_NAME;

   /// Constructor.
   ProcMgmtRpcGetStateAll();

   /// The execution of this XML-RPC Method.
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status                   ///< XML-RPC method execution status
                        );
};

/**
 Attempts to set the "user requested state" of the specified process to "start".  If successful, 
 it then optionally blocks (up to SINGLE_BLOCK_MAX seconds) for the current state to be equal 
 to the requested state.

 \par
 A process monitored by the watchdog may be configured as not user start-able.  Calling this 
 method for such a process will fail, even if the process is already started.  

 \par
 <b>Method Name: ProcMgmtRpc.start</b>

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
       <td>alias</td>
       <td>The alias of the process whose state is to be changed.  (The alias 
           is the "name" attribute of the "monitor-process" element in the process's 
           SIPX_CONFDIR/process.d/___.process.xml configuration file.)</td>
    </tr>
    <tr>
       <td>boolean</td>
       <td>blockForStateChange</td>
       <td>Whether or not to block for the state change to occur.</td>
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
       <td>True if a matching alias was found, its "user requested state" was changed,
           and the current state is equal to the requested state.  Returns false 
           otherwise.</td>
    </tr>
 </table>
 */
class ProcMgmtRpcStart : public ProcMgmtRpcMethod
{
public:

   /// The XmlRpcMethod::Get registered with the dispatcher for this XML-RPC Method.  
   static XmlRpcMethod* get();

   /// Destructor.
   virtual ~ProcMgmtRpcStart() {};

   /// Get the name of the XML-RPC method.
   virtual const char* name();

   /// Register this method handler with the XML-RPC dispatcher.
   static void registerSelf(WatchDog & watchdog);

protected:

   /// The name of the XML-RPC method.
   static const char* METHOD_NAME;

   /// Constructor.
   ProcMgmtRpcStart();

   /// The execution of this XML-RPC Method.
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status                   ///< XML-RPC method execution status
                        );
};

/**
 Attempts to set the "user requested state" of all processes being monitored by the watchdog  
 to "start".  It then blocks (up to LIST_BLOCK_MAX seconds) for the current states of each
 successfully updated process to be equal to the requested state.

 \par
 One or more processes monitored by the watchdog may be configured as not user start-able.  
 Calling this will result in a failure result for each such process, even if it is 
 already started.  

\par
 <b>Method Name: ProcMgmtRpc.startAll</b>

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
 </table>
 
 \par
 <b>Return Value:</b>
 <table border="1">
    <tr>
       <td><b>Data type</b></td>
       <td><b>Description</b></td>
    </tr>
    <tr>
       <td>boolean struct</td>
       <td>Whether or not each monitored process "user requested state" was changed, and 
           the current state is equal to the requested state.  Indexed by process alias.  
           (The alias is the "name" attribute of the "monitor-process" element in the 
           process's SIPX_CONFDIR/process.d/___.process.xml configuration file.)</td>
    </tr>
 </table>
 */
class ProcMgmtRpcStartAll : public ProcMgmtRpcMethod
{
public:

   /// The XmlRpcMethod::Get registered with the dispatcher for this XML-RPC Method.  
   static XmlRpcMethod* get();

   /// Destructor.
   virtual ~ProcMgmtRpcStartAll() {};

   /// Get the name of the XML-RPC method.
   virtual const char* name();

   /// Register this method handler with the XML-RPC dispatcher.
   static void registerSelf(WatchDog & watchdog);

protected:

   /// The name of the XML-RPC method.
   static const char* METHOD_NAME;

   /// Constructor.
   ProcMgmtRpcStartAll();

   /// The execution of this XML-RPC Method.
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status                   ///< XML-RPC method execution status
                        );
};

/**
 Attempts to set the "user requested state" of the specified process to "stop".  If successful, 
 it then optionally blocks (up to SINGLE_BLOCK_MAX seconds) for the current state to be equal 
 to the requested state. 

 \par
 A process monitored by the watchdog may be configured as not user stop-able.  (i.e. The
 "KeepAlive" proces.)  Calling this method for such a process will fail, even if the 
 process is already stopped.  

 \par
 <b>Method Name: ProcMgmtRpc.stop</b>

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
       <td>alias</td>
       <td>The alias of the process.  (The alias is the "name" attribute of the 
           "monitor-process" element in the process's 
           SIPX_CONFDIR/process.d/___.process.xml configuration file.)</td>
    </tr>
    <tr>
       <td>boolean</td>
       <td>blockForStateChange</td>
       <td>Whether or not to block for the state change to occur.</td>
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
       <td>True if a matching alias was found, its "user requested state" was changed,
           and the current state is equal to the requested state.  Returns false 
           otherwise.</td>
    </tr>
 </table>

 */
class ProcMgmtRpcStop : public ProcMgmtRpcMethod
{
public:

   /// The XmlRpcMethod::Get registered with the dispatcher for this XML-RPC Method.  
   static XmlRpcMethod* get();

   /// Destructor.
   virtual ~ProcMgmtRpcStop() {};

   /// Get the name of the XML-RPC method.
   virtual const char* name();

   /// Register this method handler with the XML-RPC dispatcher.
   static void registerSelf(WatchDog & watchdog);

protected:

   /// The name of the XML-RPC method.
   static const char* METHOD_NAME;

   /// Constructor.
   ProcMgmtRpcStop();

   /// The execution of this XML-RPC Method.
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status                   ///< XML-RPC method execution status
                        );
};

/**
 Attempts to set the "user requested state" of all processes being monitored by the watchdog 
 to "stop".  It then blocks (up to LIST_BLOCK_MAX seconds) for the current states of each
 successfully updated process to be equal to the requested state.

 \par
 One or more processes monitored by the watchdog may be configured as not user stop-able.  
 (i.e. The "KeepAlive" proces.)  Calling this will result in a failure result for each such
 process, even if it is already stopped.  

 \par
 <b>Method Name: ProcMgmtRpc.stopAll</b>

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
 </table>
 
 \par
 <b>Return Value:</b>
 <table border="1">
    <tr>
       <td><b>Data type</b></td>
       <td><b>Description</b></td>
    </tr>
    <tr>
       <td>boolean struct</td>
       <td>Whether or not each monitored process "user requested state" was changed, and 
           the current state is equal to the requested state.  Indexed by process alias.  
           (The alias is the "name" attribute of the "monitor-process" element in the 
           process's SIPX_CONFDIR/process.d/___.process.xml configuration file.)</td>
    </tr>
 </table>
 */
class ProcMgmtRpcStopAll : public ProcMgmtRpcMethod
{
public:

   /// The XmlRpcMethod::Get registered with the dispatcher for this XML-RPC Method.  
   static XmlRpcMethod* get();


   /// Destructor.
   virtual ~ProcMgmtRpcStopAll() {};

   /// Get the name of the XML-RPC method.
   virtual const char* name();

   /// Register this method handler with the XML-RPC dispatcher.
   static void registerSelf(WatchDog & watchdog);

protected:

   /// The name of the XML-RPC method.
   static const char* METHOD_NAME;

   /// Constructor.
   ProcMgmtRpcStopAll();

   /// The execution of this XML-RPC Method.
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status                   ///< XML-RPC method execution status
                        );
};

/**
 Attempts to set the "user requested state" of the specified process to "restart".  If successful, 
 it then optionally blocks (up to SINGLE_BLOCK_MAX seconds) for the process to be running with a 
 new PID.

 \par
 A process monitored by the watchdog may be configured as not user restart-able.  (i.e. The 
 "KeepAlive" proces.)  Calling this method for such a process will fail.  

 \par
 <b>Method Name: ProcMgmtRpc.restart</b>

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
       <td>alias</td>
       <td>The alias of the process.  (The alias is the "name" attribute of the 
           "monitor-process" element in the process's 
           SIPX_CONFDIR/process.d/___.process.xml configuration file.)</td>
    </tr>
    <tr>
       <td>boolean</td>
       <td>blockForStateChange</td>
       <td>Whether or not to block for the state change to occur.</td>
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
       <td>True if a matching alias was found, its "user requested state" was changed,
           and the process is now running with a new PID.  Returns false otherwise.</td>
    </tr>
 </table>
 */
class ProcMgmtRpcRestart : public ProcMgmtRpcMethod
{
public:

   /// The XmlRpcMethod::Get registered with the dispatcher for this XML-RPC Method.  
   static XmlRpcMethod* get();

   /// Destructor.
   virtual ~ProcMgmtRpcRestart() {};

   /// Get the name of the XML-RPC method.
   virtual const char* name();

   /// Register this method handler with the XML-RPC dispatcher.
   static void registerSelf(WatchDog & watchdog);

protected:

   /// The name of the XML-RPC method.
   static const char* METHOD_NAME;

   /// Constructor.
   ProcMgmtRpcRestart();

   /// The execution of this XML-RPC Method.
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status                   ///< XML-RPC method execution status
                        );
};

/**
 Attempts to set the "user requested state" of all processes being monitored by the watchdog to 
 "restart".  It then blocks (up to LIST_BLOCK_MAX seconds) for the current states of each
 successfully updated process to be running with a new PID.

 \par
 One or more processes monitored by the watchdog may be configured as not user restart-able.  
 (i.e. The "KeepAlive" proces.)  Calling this will result in a failure result for each such
 process.  

 \par
 <b>Method Name: ProcMgmtRpc.restartAll</b>

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
 </table>
 
 \par
 <b>Return Value:</b>
 <table border="1">
    <tr>
       <td><b>Data type</b></td>
       <td><b>Description</b></td>
    </tr>
    <tr>
       <td>boolean struct</td>
       <td>Whether or not each monitored process "user requested state" was changed, and 
           the process is now running with a new PID.  Indexed by process alias.  
           (The alias is the "name" attribute of the "monitor-process" element in the 
           process's SIPX_CONFDIR/process.d/___.process.xml configuration file.)</td>
    </tr>
 </table>
 */
class ProcMgmtRpcRestartAll : public ProcMgmtRpcMethod
{
public:

   /// The XmlRpcMethod::Get registered with the dispatcher for this XML-RPC Method.  
   static XmlRpcMethod* get();

   /// Destructor.
   virtual ~ProcMgmtRpcRestartAll() {};

   /// Get the name of the XML-RPC method.
   virtual const char* name();

   /// Register this method handler with the XML-RPC dispatcher.
   static void registerSelf(WatchDog & watchdog);

protected:

   /// The name of the XML-RPC method.
   static const char* METHOD_NAME;

   /// Constructor.
   ProcMgmtRpcRestartAll();

   /// The execution of this XML-RPC Method.
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status                   ///< XML-RPC method execution status
                        );
};

/**
 Returns the alias of the monitored processes with the specified PID.  (The 
 alias is the "name" attribute of the "monitor-process" element in  
 the process's SIPX_CONFDIR/process.d/___.process.xml configuration file.)

 \par
 <b>Method Name: ProcMgmtRpc.getAliasByPID</b>

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
       <td>integer</td>
       <td>pid</td>
       <td>The PID of the monitored processes whose alias is to be returned.</td>
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
       <td>string</td>
       <td>The alias of the monitored processes with the specified PID.  A blank string
           indicates that no matching monitored processes was found.</td>
    </tr>
 </table>
 */
class ProcMgmtRpcGetAliasByPID : public ProcMgmtRpcMethod
{
public:

   /// The XmlRpcMethod::Get registered with the dispatcher for this XML-RPC Method.  
   static XmlRpcMethod* get();

   /// Destructor.
   virtual ~ProcMgmtRpcGetAliasByPID() {};

   /// Get the name of the XML-RPC method.
   virtual const char* name();

   /// Register this method handler with the XML-RPC dispatcher.
   static void registerSelf(WatchDog & watchdog);

protected:

   /// The name of the XML-RPC method.
   static const char* METHOD_NAME;

   /// Constructor.
   ProcMgmtRpcGetAliasByPID();

   /// The execution of this XML-RPC Method.
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status                   ///< XML-RPC method execution status
                        );
};

#endif // _PROCMGMTRPC_H_


// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _PROCMGMTRPC_H_
#define _PROCMGMTRPC_H_

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
class WatchDog;

/**
 The base class for all Process Management XML-RPC Methods.
 */
class ProcMgmtRpcMethod : public XmlRpcMethod
{
public:
   
   /// The XmlRpcMethod::Get registered with the dispatcher for this XML-RPC Method.  
   static XmlRpcMethod* get();

   /// Destructor
   virtual ~ProcMgmtRpcMethod() {}
   
   /// Get the name of the XML-RPC method.
   virtual const char* name() = 0;

   /// Register this method with the XmlRpcDispatch object so it can be called.
   static void registerSelf(WatchDog & watchdog);

   typedef enum
   {
      UnconfiguredPeer = 300, ///< caller is not a configured peer of this server
      InvalidParameter,       ///< missing parameter, extra parameter(s), or invalid type
   } FaultCode;

protected:

   /// The maximum amount of time to block waiting for a single process to undergo a state change.
   static int SINGLE_BLOCK_MAX;

   /// The maximum amount of time to block waiting for a list of processes to undergo a state change.
   static int LIST_BLOCK_MAX;

   /// The name of the XML-RPC method.
   static const char* METHOD_NAME;
   
   /// The name of the XML-RPC 'callingHostname' parameter.
   static const char* PARAM_NAME_CALLING_HOST;

   /// The name of the XML-RPC 'alias' parameter.
   static const char* PARAM_NAME_ALIAS;

   /// The name of the XML-RPC 'blockForStateChange' parameter.
   static const char* PARAM_NAME_BLOCK;

   /// The name of the XML-RPC 'pid' parameter.
   static const char* PARAM_NAME_PID;

   /// constructor 
   ProcMgmtRpcMethod();

   /// Common method for registering with the XML-RPC dispatcher.
   static void registerMethod(const char*       methodName,
                              XmlRpcMethod::Get getMethod,
                              WatchDog&         watchdog
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
                    const WatchDog&           watchdog,       ///< the watchdog
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

   /// Sets the user requested state of a single process to the specified state, and blocks for the result.
   bool executeSetUserRequestState(const HttpRequestContext& requestContext, ///< request context
                                   UtlSList& params,                         ///< request param list
                                   void* userData,                           ///< user data
                                   XmlRpcResponse& response,                 ///< request response
                                   ExecutionStatus& status,                  ///< XML-RPC method execution status
                                   const int request_state                   ///< the state to set
                                   );
   /**<
     Returns true on success, false otherwise.
    */
   
   /// Calls blockForProcessStateMatchList(), but the list to block for contains only the single specified process.
   bool blockForProcessStateMatch(WatchDog* pWatchDog, 
                                  const UtlString& alias, 
                                  const int request_state, 
                                  const int max_secs = SINGLE_BLOCK_MAX);
   
   /// Calls blockForProcessRestartList(), but the list to block for contains only the single specified process.
   bool blockForProcessRestart(WatchDog* pWatchDog, 
                               const UtlString& alias, 
                               const PID original_pid, 
                               const int max_secs = SINGLE_BLOCK_MAX);
   
   /// Sets the user requested state of all processes to the specified state, and blocks for the result. 
   bool executeSetUserRequestStateAll(const HttpRequestContext& requestContext, ///< request context
                                      UtlSList& params,                         ///< request param list
                                      void* userData,                           ///< user data
                                      XmlRpcResponse& response,                 ///< request response
                                      ExecutionStatus& status,                  ///< XML-RPC method execution status
                                      const int request_state                   ///< the state to set
                                      );
   /**<
     Returns true on success, false otherwise.
    */

   /// Blocks for the update of the process's current state to the specified requested state.
   void blockForProcessStateMatchList(WatchDog* pWatchDog, 
                                      UtlHashMap& process_results, 
                                      const int request_state, 
                                      const int max_secs = LIST_BLOCK_MAX);
   /**<
     Only does this for the aliases in 'process_results' with a true result.  Any that are not 
     updated before timeout will have their 'process_results' entry set to a false result.
    */
   
   /// Blocks for the restart of the process's.
   void blockForProcessRestartList(WatchDog* pWatchDog, 
                                   UtlHashMap& process_results, 
                                   const UtlHashMap& original_pids, 
                                   const int max_secs = LIST_BLOCK_MAX);
  /**<
     Only does this for the aliases in 'process_results' with a true result.  Any that are not
     restarted  before timeout will have their 'process_results' entry set to a false result.
    */
   
private:
   /// no copy constructor
   ProcMgmtRpcMethod(const ProcMgmtRpcMethod& nocopy);

   /// no assignment operator
   ProcMgmtRpcMethod& operator=(const ProcMgmtRpcMethod& noassignment);
};

/**
 Returns the current state of all processes being monitored by the watchdog.

 \par
 <b>Method Name: ProcMgmtRpc.getStateAll</b>

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
 </table>
 
 \par
 <b>Return Value:</b>
 <table border="1">
    <tr>
       <td><b>Data type</b></td>
       <td><b>Description</b></td>
    </tr>
    <tr>
       <td>string struct</td>
       <td>The current status of each process, indexed by process alias.  (The alias 
           is the "name" attribute of the "monitor-process" element in the process's 
           SIPX_CONFDIR/process.d/___.process.xml configuration file.)</td>
    </tr>
 </table>
 */
class ProcMgmtRpcGetStateAll : public ProcMgmtRpcMethod
{
public:

   /// The XmlRpcMethod::Get registered with the dispatcher for this XML-RPC Method.  
   static XmlRpcMethod* get();

   /// Destructor.
   virtual ~ProcMgmtRpcGetStateAll() {};

   /// Get the name of the XML-RPC method.
   virtual const char* name();

   /// Register this method handler with the XML-RPC dispatcher.
   static void registerSelf(WatchDog & watchdog);

protected:

   /// The name of the XML-RPC method.
   static const char* METHOD_NAME;

   /// Constructor.
   ProcMgmtRpcGetStateAll();

   /// The execution of this XML-RPC Method.
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status                   ///< XML-RPC method execution status
                        );
};

/**
 Attempts to set the "user requested state" of the specified process to "start".  If successful, 
 it then optionally blocks (up to SINGLE_BLOCK_MAX seconds) for the current state to be equal 
 to the requested state.

 \par
 A process monitored by the watchdog may be configured as not user start-able.  Calling this 
 method for such a process will fail, even if the process is already started.  

 \par
 <b>Method Name: ProcMgmtRpc.start</b>

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
       <td>alias</td>
       <td>The alias of the process whose state is to be changed.  (The alias 
           is the "name" attribute of the "monitor-process" element in the process's 
           SIPX_CONFDIR/process.d/___.process.xml configuration file.)</td>
    </tr>
    <tr>
       <td>boolean</td>
       <td>blockForStateChange</td>
       <td>Whether or not to block for the state change to occur.</td>
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
       <td>True if a matching alias was found, its "user requested state" was changed,
           and the current state is equal to the requested state.  Returns false 
           otherwise.</td>
    </tr>
 </table>
 */
class ProcMgmtRpcStart : public ProcMgmtRpcMethod
{
public:

   /// The XmlRpcMethod::Get registered with the dispatcher for this XML-RPC Method.  
   static XmlRpcMethod* get();

   /// Destructor.
   virtual ~ProcMgmtRpcStart() {};

   /// Get the name of the XML-RPC method.
   virtual const char* name();

   /// Register this method handler with the XML-RPC dispatcher.
   static void registerSelf(WatchDog & watchdog);

protected:

   /// The name of the XML-RPC method.
   static const char* METHOD_NAME;

   /// Constructor.
   ProcMgmtRpcStart();

   /// The execution of this XML-RPC Method.
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status                   ///< XML-RPC method execution status
                        );
};

/**
 Attempts to set the "user requested state" of all processes being monitored by the watchdog  
 to "start".  It then blocks (up to LIST_BLOCK_MAX seconds) for the current states of each
 successfully updated process to be equal to the requested state.

 \par
 One or more processes monitored by the watchdog may be configured as not user start-able.  
 Calling this will result in a failure result for each such process, even if it is 
 already started.  

\par
 <b>Method Name: ProcMgmtRpc.startAll</b>

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
 </table>
 
 \par
 <b>Return Value:</b>
 <table border="1">
    <tr>
       <td><b>Data type</b></td>
       <td><b>Description</b></td>
    </tr>
    <tr>
       <td>boolean struct</td>
       <td>Whether or not each monitored process "user requested state" was changed, and 
           the current state is equal to the requested state.  Indexed by process alias.  
           (The alias is the "name" attribute of the "monitor-process" element in the 
           process's SIPX_CONFDIR/process.d/___.process.xml configuration file.)</td>
    </tr>
 </table>
 */
class ProcMgmtRpcStartAll : public ProcMgmtRpcMethod
{
public:

   /// The XmlRpcMethod::Get registered with the dispatcher for this XML-RPC Method.  
   static XmlRpcMethod* get();

   /// Destructor.
   virtual ~ProcMgmtRpcStartAll() {};

   /// Get the name of the XML-RPC method.
   virtual const char* name();

   /// Register this method handler with the XML-RPC dispatcher.
   static void registerSelf(WatchDog & watchdog);

protected:

   /// The name of the XML-RPC method.
   static const char* METHOD_NAME;

   /// Constructor.
   ProcMgmtRpcStartAll();

   /// The execution of this XML-RPC Method.
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status                   ///< XML-RPC method execution status
                        );
};

/**
 Attempts to set the "user requested state" of the specified process to "stop".  If successful, 
 it then optionally blocks (up to SINGLE_BLOCK_MAX seconds) for the current state to be equal 
 to the requested state. 

 \par
 A process monitored by the watchdog may be configured as not user stop-able.  (i.e. The
 "KeepAlive" proces.)  Calling this method for such a process will fail, even if the 
 process is already stopped.  

 \par
 <b>Method Name: ProcMgmtRpc.stop</b>

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
       <td>alias</td>
       <td>The alias of the process.  (The alias is the "name" attribute of the 
           "monitor-process" element in the process's 
           SIPX_CONFDIR/process.d/___.process.xml configuration file.)</td>
    </tr>
    <tr>
       <td>boolean</td>
       <td>blockForStateChange</td>
       <td>Whether or not to block for the state change to occur.</td>
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
       <td>True if a matching alias was found, its "user requested state" was changed,
           and the current state is equal to the requested state.  Returns false 
           otherwise.</td>
    </tr>
 </table>

 */
class ProcMgmtRpcStop : public ProcMgmtRpcMethod
{
public:

   /// The XmlRpcMethod::Get registered with the dispatcher for this XML-RPC Method.  
   static XmlRpcMethod* get();

   /// Destructor.
   virtual ~ProcMgmtRpcStop() {};

   /// Get the name of the XML-RPC method.
   virtual const char* name();

   /// Register this method handler with the XML-RPC dispatcher.
   static void registerSelf(WatchDog & watchdog);

protected:

   /// The name of the XML-RPC method.
   static const char* METHOD_NAME;

   /// Constructor.
   ProcMgmtRpcStop();

   /// The execution of this XML-RPC Method.
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status                   ///< XML-RPC method execution status
                        );
};

/**
 Attempts to set the "user requested state" of all processes being monitored by the watchdog 
 to "stop".  It then blocks (up to LIST_BLOCK_MAX seconds) for the current states of each
 successfully updated process to be equal to the requested state.

 \par
 One or more processes monitored by the watchdog may be configured as not user stop-able.  
 (i.e. The "KeepAlive" proces.)  Calling this will result in a failure result for each such
 process, even if it is already stopped.  

 \par
 <b>Method Name: ProcMgmtRpc.stopAll</b>

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
 </table>
 
 \par
 <b>Return Value:</b>
 <table border="1">
    <tr>
       <td><b>Data type</b></td>
       <td><b>Description</b></td>
    </tr>
    <tr>
       <td>boolean struct</td>
       <td>Whether or not each monitored process "user requested state" was changed, and 
           the current state is equal to the requested state.  Indexed by process alias.  
           (The alias is the "name" attribute of the "monitor-process" element in the 
           process's SIPX_CONFDIR/process.d/___.process.xml configuration file.)</td>
    </tr>
 </table>
 */
class ProcMgmtRpcStopAll : public ProcMgmtRpcMethod
{
public:

   /// The XmlRpcMethod::Get registered with the dispatcher for this XML-RPC Method.  
   static XmlRpcMethod* get();


   /// Destructor.
   virtual ~ProcMgmtRpcStopAll() {};

   /// Get the name of the XML-RPC method.
   virtual const char* name();

   /// Register this method handler with the XML-RPC dispatcher.
   static void registerSelf(WatchDog & watchdog);

protected:

   /// The name of the XML-RPC method.
   static const char* METHOD_NAME;

   /// Constructor.
   ProcMgmtRpcStopAll();

   /// The execution of this XML-RPC Method.
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status                   ///< XML-RPC method execution status
                        );
};

/**
 Attempts to set the "user requested state" of the specified process to "restart".  If successful, 
 it then optionally blocks (up to SINGLE_BLOCK_MAX seconds) for the process to be running with a 
 new PID.

 \par
 A process monitored by the watchdog may be configured as not user restart-able.  (i.e. The 
 "KeepAlive" proces.)  Calling this method for such a process will fail.  

 \par
 <b>Method Name: ProcMgmtRpc.restart</b>

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
       <td>alias</td>
       <td>The alias of the process.  (The alias is the "name" attribute of the 
           "monitor-process" element in the process's 
           SIPX_CONFDIR/process.d/___.process.xml configuration file.)</td>
    </tr>
    <tr>
       <td>boolean</td>
       <td>blockForStateChange</td>
       <td>Whether or not to block for the state change to occur.</td>
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
       <td>True if a matching alias was found, its "user requested state" was changed,
           and the process is now running with a new PID.  Returns false otherwise.</td>
    </tr>
 </table>
 */
class ProcMgmtRpcRestart : public ProcMgmtRpcMethod
{
public:

   /// The XmlRpcMethod::Get registered with the dispatcher for this XML-RPC Method.  
   static XmlRpcMethod* get();

   /// Destructor.
   virtual ~ProcMgmtRpcRestart() {};

   /// Get the name of the XML-RPC method.
   virtual const char* name();

   /// Register this method handler with the XML-RPC dispatcher.
   static void registerSelf(WatchDog & watchdog);

protected:

   /// The name of the XML-RPC method.
   static const char* METHOD_NAME;

   /// Constructor.
   ProcMgmtRpcRestart();

   /// The execution of this XML-RPC Method.
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status                   ///< XML-RPC method execution status
                        );
};

/**
 Attempts to set the "user requested state" of all processes being monitored by the watchdog to 
 "restart".  It then blocks (up to LIST_BLOCK_MAX seconds) for the current states of each
 successfully updated process to be running with a new PID.

 \par
 One or more processes monitored by the watchdog may be configured as not user restart-able.  
 (i.e. The "KeepAlive" proces.)  Calling this will result in a failure result for each such
 process.  

 \par
 <b>Method Name: ProcMgmtRpc.restartAll</b>

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
 </table>
 
 \par
 <b>Return Value:</b>
 <table border="1">
    <tr>
       <td><b>Data type</b></td>
       <td><b>Description</b></td>
    </tr>
    <tr>
       <td>boolean struct</td>
       <td>Whether or not each monitored process "user requested state" was changed, and 
           the process is now running with a new PID.  Indexed by process alias.  
           (The alias is the "name" attribute of the "monitor-process" element in the 
           process's SIPX_CONFDIR/process.d/___.process.xml configuration file.)</td>
    </tr>
 </table>
 */
class ProcMgmtRpcRestartAll : public ProcMgmtRpcMethod
{
public:

   /// The XmlRpcMethod::Get registered with the dispatcher for this XML-RPC Method.  
   static XmlRpcMethod* get();

   /// Destructor.
   virtual ~ProcMgmtRpcRestartAll() {};

   /// Get the name of the XML-RPC method.
   virtual const char* name();

   /// Register this method handler with the XML-RPC dispatcher.
   static void registerSelf(WatchDog & watchdog);

protected:

   /// The name of the XML-RPC method.
   static const char* METHOD_NAME;

   /// Constructor.
   ProcMgmtRpcRestartAll();

   /// The execution of this XML-RPC Method.
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status                   ///< XML-RPC method execution status
                        );
};

/**
 Returns the alias of the monitored processes with the specified PID.  (The 
 alias is the "name" attribute of the "monitor-process" element in  
 the process's SIPX_CONFDIR/process.d/___.process.xml configuration file.)

 \par
 <b>Method Name: ProcMgmtRpc.getAliasByPID</b>

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
       <td>integer</td>
       <td>pid</td>
       <td>The PID of the monitored processes whose alias is to be returned.</td>
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
       <td>string</td>
       <td>The alias of the monitored processes with the specified PID.  A blank string
           indicates that no matching monitored processes was found.</td>
    </tr>
 </table>
 */
class ProcMgmtRpcGetAliasByPID : public ProcMgmtRpcMethod
{
public:

   /// The XmlRpcMethod::Get registered with the dispatcher for this XML-RPC Method.  
   static XmlRpcMethod* get();

   /// Destructor.
   virtual ~ProcMgmtRpcGetAliasByPID() {};

   /// Get the name of the XML-RPC method.
   virtual const char* name();

   /// Register this method handler with the XML-RPC dispatcher.
   static void registerSelf(WatchDog & watchdog);

protected:

   /// The name of the XML-RPC method.
   static const char* METHOD_NAME;

   /// Constructor.
   ProcMgmtRpcGetAliasByPID();

   /// The execution of this XML-RPC Method.
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status                   ///< XML-RPC method execution status
                        );
};

#endif // _PROCMGMTRPC_H_


