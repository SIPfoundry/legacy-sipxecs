//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _SYNCRPC_H_
#define _SYNCRPC_H_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "net/XmlRpcMethod.h"
#include "RegistrarPeer.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipRegistrar;
class XmlRpcDispatch;

/// Base class for all registerSync XML-RPC Methods
class SyncRpcMethod : public XmlRpcMethod
{
public:
   static XmlRpcMethod* get();

   /// destructor
   virtual ~SyncRpcMethod()
      {
      }

   /// Get the name of the XML-RPC method.
   virtual const char* name() = 0;

   /// Register this method with the XmlRpcDispatch object so it can be called.
   static void registerSelf(SipRegistrar&   registrar);

   typedef enum
   {
      UnconfiguredPeer = 300, ///< caller is not a configured peer of this server
      UnreachablePeer,        ///< caller has been declared UnReachable
      InvalidParameter,       ///< missing parameter or invalid type
      AuthenticationFailure,  ///< connection not authenticated by SSL
      UpdateFailed,           ///< error in applying updates to the registration DB
      UpdateOutOfOrder,       ///< received an update ahead of prior updates
      MixedUpdateNumbers,     ///< received a pushed update with multiple update numbers
      IncompatiblePeer        ///< peer previously marked Incompatible - must restart to clear
   } FaultCode;

protected:
   // Method name
   static const char* METHOD_NAME;

   /// constructor
   SyncRpcMethod();

   /// Common method for registration with the XML-RPC dispatcher
   static void registerMethod(const char*        methodName,
                              XmlRpcMethod::Get  getMethod,
                              SipRegistrar&      registrar
                              );

   /// The execute method called by XmlRpcDispatch
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status
                        ) = 0;

   /// Common method to do peer validation
   RegistrarPeer* validCaller(
      const HttpRequestContext& requestContext, ///< request context
      const UtlString&          peerName,       ///< name of the peer who is calling
      XmlRpcResponse&           response,       ///< response to put fault in
      SipRegistrar&             registrar,      ///< registrar
      const char*               callingMethod   ///< calling xml rpc method name
                                      );
   /**<
    * @returns the pointer to the RegistrarPeer if all of the following are true:
    * - the peerName is configured
    * - the HttpRequestContext indicates that the connection is from that peer
    * - the peer state is not Incompatible
    *
    * On any failure, the fault response is set appropriately and a NULL is returned.
    */

   /// Handle missing parameters for the execute method
   void handleMissingExecuteParam(const char* methodName,  ///< name of the called XmlRpc method
                                  const char* paramName,   ///< name of problematic parameter
                                  XmlRpcResponse& response,///< response (fault is set on this)
                                  ExecutionStatus& status, ///< set to FAILED
                                  RegistrarPeer* peer = NULL ///< if passed, is set to Incompatible
                                  );

private:
   /// no copy constructor
   SyncRpcMethod(const SyncRpcMethod& nocopy);

   /// no assignment operator
   SyncRpcMethod& operator=(const SyncRpcMethod& noassignment);
};


/// the registerSync.reset XML-RPC method.
/**
 * This method conveys the PeerReceivedDbUpdateNumber in both directions
 * between the client and the server, and indicates that the client is
 * ready to receive registrarSync.pushUpdates calls.
 *
 * Inputs:
 *
 * string  callingRegistrar          Calling registrar name
 * i8      updateNumber
 *
 * The updateNumber input is the client's PeerReceivedDbUpdateNumber for
 * that server.  PeerReceivedDbUpdateNumber is the highest update number
 * in the client's database owned by the server, or zero if there are no
 * such updates.  This value becomes the PeerSentDbUpdateNumber in the
 * server for the callingRegistrar client.  Note that this value may be
 * less than the current value for PeerSentDbUpdateNumber, indicating
 * that some previously sent updates were lost.
 *
 * Outputs:
 *
 * i8   updateNumber
 *
 * The returned updateNumber is the highest update number in the
 * server's database owned by the client: the PeerReceivedDbUpdateNumber
 * in the server for the client.  The client sets PeerSentDbUpdateNumber
 * for the server to this value.  A successful return indicates that the
 * server is prepared to receive registrarSync.pushUpdates calls.
 *
 * If no fault is returned, the client and server each mark the other as
 * Reachable, and call the RegistrarSync::sendUpdates C++ method to
 * begin pushing updates to the peer.  It is possible that there are no
 * updates to be sent, but determining this is the responsibility of the
 * RegistrarSync (Section 5.7.3.2) thread.
 */
class SyncRpcReset : public SyncRpcMethod
{
  public:

   static XmlRpcMethod* get();

   /// destructor
   virtual ~SyncRpcReset() {};

   /// Get the name of the XML-RPC method.
   virtual const char* name();

   /// Register this method handler with the XML-RPC dispatcher.
   static void registerSelf(SipRegistrar&   registrar);

   /// Reset the SynchronizationState and update numbers with respect to some peer.
   static RegistrarPeer::SynchronizationState
      invoke( const char*    myName   ///< primary name of this registrar
             ,RegistrarPeer& peer     ///< the peer to invoke reset on
             );
   /**<
    * On success,
    *   - updates the sent update number for the peer
    *   - marks the peer as Reachable
    *
    * On any failure, the peer is marked UnReachable.
    */

  protected:
   /// constructor
   SyncRpcReset();

   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status
                        );

   static const char* METHOD_NAME;

  private:

   /// no copy constructor
   SyncRpcReset(const SyncRpcReset& nocopy);

   /// no assignment operator
   SyncRpcReset& operator=(const SyncRpcReset& noassignment);
};



/// the registerSync.pullUpdates XML-RPC method.
/**
 * Retrieve all updates for primaryRegistrar whose update number is greater than updateNumber.
 *
 * Parameters  Type          Name                    Description
 *  Inputs:
 *             string        callingRegistrar        Calling registrar name
 *             string        primaryRegistrar        Primary registrar name
 *             Int64         updateNumber
 *  Outputs:
 *             struct
 *               int         numUpdates
 *               array       updates
 *                 struct    row
 *                   string  uri
 *                   string  callid
 *                   int     cseq
 *                   string  contact
 *                   int     expires
 *                   string  qvalue
 *                   string  instanceId
 *                   string  gruu
 *                   string  path
 *                   string  primary
 *                   Int64   updateNumber
 */
class SyncRpcPullUpdates : public SyncRpcMethod
{
public:

   static XmlRpcMethod* get();

   /// destructor
   virtual ~SyncRpcPullUpdates() {};

   /// Get the name of the XML-RPC method.
   virtual const char* name();

   /// Register this method handler with the XML-RPC dispatcher.
   static void registerSelf(SipRegistrar&   registrar);

   /// pull all missing updates for a given primary from a peer
   static RegistrarPeer::SynchronizationState
      invoke(RegistrarPeer* source,       ///< peer to pull from
             const char*    myName,       ///< name of this registrar
             const char*    primaryName,  ///< name of registrar whose updates we want
             Int64          updateNumber, ///< pull updates starting after this number
             UtlSList*      bindings      ///< list of RegistrationBinding
             );
   /**<
    * On success, the bindings are returned.
    *
    * On any failure, the source is marked UnReachable.
    */

protected:
   static const char* METHOD_NAME;
   static const UtlString NUM_UPDATES;
   static const UtlString UPDATES;

   /// constructor
   SyncRpcPullUpdates();

   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status
                        );

private:
   /// no copy constructor
   SyncRpcPullUpdates(const SyncRpcPullUpdates& nocopy);

   /// no assignment operator
   SyncRpcPullUpdates& operator=(const SyncRpcPullUpdates& noassignment);
};




/// the registerSync.pushUpdates XML-RPC method.
/**
 * Send registry updates to a peer registrar
 *
 * Parameters  Type        Name                    Description
 *  Inputs:
 *             string      callingRegistrar        Calling registrar name
 *             i8          lastSentUpdateNumber    Number of last update sent
 *             array       updates
 *               struct
 *                 string  uri
 *                 string  callid
 *                 int     cseq
 *                 string  contact
 *                 int     expires
 *                 string  qvalue
 *                 string  instanceId
 *                 string  gruu
 *                 string  path
 *                 string  primary
 *                 Int64   updateNumber
 *  Outputs:
 *             Int64       updateNumber
 */
class SyncRpcPushUpdates : public SyncRpcMethod
{
public:
   static XmlRpcMethod* get();

   /// destructor
   virtual ~SyncRpcPushUpdates() {};

   /// Get the name of the XML-RPC method.
   virtual const char* name();

   /// Register this method handler with the XML-RPC dispatcher.
   static void registerSelf(SipRegistrar&   registrar);

   /// Push one update to a given peer
   static RegistrarPeer::SynchronizationState
      invoke(RegistrarPeer* replicated, ///< peer to push to
             const char*    myName,     ///< primary name of this registrar
             UtlSList*      bindings    ///< list of RegistrationBinding
             );
   /**<
    * On success, this updates the sent update number for the peer.
    *
    * On any failure, the peer is marked UnReachable.
    */

protected:
   /// constructor
   SyncRpcPushUpdates();

   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params,                         ///< request param list
                        void* userData,                           ///< user data
                        XmlRpcResponse& response,                 ///< request response
                        ExecutionStatus& status
                        );

   /// Core method for handling pushUpdates.  Return true on success, false on failure.
   bool applyPushedUpdates(UtlSList&        updateMaps,
                           XmlRpcResponse&  response,
                           ExecutionStatus& status,
                           RegistrarPeer&   peer,
                           SipRegistrar&    registrar);

   static const char* METHOD_NAME;

private:
   /// Check lastSentUpdateNumber <= PeerReceivedDbUpdateNumber, otherwise updates are missing
   void checkLastSentUpdateNumber(Int64 lastSentUpdateNumber,
                                  RegistrarPeer& peer,
                                  XmlRpcResponse& response,
                                  ExecutionStatus& status);
   /**<
    * If everything is OK, then set status to XmlRpcMethod::OK.
    * Otherwise fill in the response with a fault and set status to XmlRpcMethod::FAILED.
    */

   // Compare the binding's updateNumber with the expected number.
   // Return true if they match and false if they don't.
   // If there is a mismatch, then set up fault info in the RPC reponse.
   bool checkUpdateNumber(const RegistrationBinding& reg,
                          Int64                      updateNumber,
                          RegistrarPeer&             peer,
                          XmlRpcResponse&            response,
                          ExecutionStatus&           status);

   /// no copy constructor
   SyncRpcPushUpdates(const SyncRpcPushUpdates& nocopy);

   /// no assignment operator
   SyncRpcPushUpdates& operator=(const SyncRpcPushUpdates& noassignment);
};



#endif // _SYNCRPC_H_
