//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _SipRegistrar_h_
#define _SipRegistrar_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "net/SipUserAgent.h"
#include "os/OsDefs.h"
#include "os/OsBSem.h"
#include "os/OsRWMutex.h"
#include "os/OsServerTask.h"
#include "utl/UtlHashMap.h"
#include "utl/PluginHooks.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsConfigDb;
class SipUserAgent;
class SipMessage;
class HttpServer;
class XmlRpcDispatch;
class RegistrarPeer;
class RegistrarTest;
class RegistrarSync;
class RegistrarPersist;
class RegistrarInitialSync;
class RegisterEventServer;
class SipRedirectServer;
class SipRegistrarServer;
class UtlSListIterator;

/// Top Level sipXregistry thread
/**
 * This is the top level thread in the service; it spawns
 * all other threads and controls which are started at which time.
 */
class SipRegistrar : public OsServerTask
{
   friend class ApplyUpdatesTest;

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   //:Default constructor
   SipRegistrar( OsConfigDb* configDb
                );

   virtual
   ~SipRegistrar();
     //:Destructor


    static SipRegistrar* getInstance(OsConfigDb* configDb = NULL);

    SipRegistrarServer& getRegistrarServer();

    /// top level task
    virtual int run(void* pArg);

    /// Signal the task that it is time to shut down.
    virtual void requestShutdown();

    /// Receive SIP or other OS messages.
    virtual UtlBoolean handleMessage(OsMsg& eventMessage);
    /**< Messages are dispatched to either the SipRegistrarServer or SipRedirectServer thread */

    /// Launch all Startup Phase threads and wait until synchronization state is known
    void startupPhase();
    /**<
     * Begin the RegistrarInitialSync thread and wait for it to finish
     */

    /// Launch all Operational Phase threads.
    UtlBoolean operationalPhase();
    /**<
     * Begins operation of the SipRegistrarServer and SipRedirectServer.
     *
     * Returns false on fatal error, otherwise true
     */

    /// Create and start the RegistrarPersist thread.
    void createAndStartPersist();
    /**<
     * Create the RegistrarPersist sub-task and start it.
     * (This is a separate method so that it can be called by testing
     * code.)
     */




    /// Get the RegistrarPersist thread object
    RegistrarPersist* getRegistrarPersist();


    /// Get the RegisterEventServer thread object
    //  Returns NULL if the server is not active yet.
    RegisterEventServer* getRegisterEventServer();



    /// Get the config DB
    OsConfigDb* getConfigDB();

    // Is this registrar authoritative for the domain in this URL?
    bool isValidDomain(const Url& uri) const;
    /**<
     * @return
     * - true if this registrar is authoritative for this URL
     * - false if not
     */

    /// Get the default domain name for this registrar
    const char* defaultDomain() const;

    /// Get the proxy port for the domain
    int domainProxyPort() const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    /// Configure a domain as valid for registration at this server.
    void addValidDomain(const UtlString& host, ///<the host part of a registration url
                        int port = PORT_NONE   ///<the port number portion of a registration url
                        );

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   // Constants

   static const int SIP_REGISTRAR_DEFAULT_XMLRPC_PORT;

   // Singleton globals
   static SipRegistrar* spInstance;
   static OsBSem sLock;

   // order is important - these two must be before everything below
   // so that they are initialized first.
   OsConfigDb* mConfigDb; ///< this is owned by the main routine - do not delete


   SipUserAgent* mSipUserAgent;

   SipRedirectServer* mRedirectServer;
   OsMsgQ* mRedirectMsgQ;

   SipRegistrarServer* mRegistrarServer;
   OsMsgQ* mRegistrarMsgQ;


   RegisterEventServer* mRegisterEventServer;
   RegistrarPersist* mRegistrarPersist;

   UtlString mDefaultDomain;
   int mDefaultDomainPort;
   UtlString mDefaultDomainHost;
   UtlHashMap mValidDomains;
   // A port number, which if found on an AOR to register,
   // will be removed, or PORT_NONE
   int mProxyNormalPort;
   UtlString mBindIp;    /// Local Ip address to bind on

   /* ============================ REGISTRAR =================================== */
   void startRegistrarServer();
   void sendToRegistrarServer(OsMsg& eventMessage);

   /* ============================ REDIRECT ==================================== */
   void startRedirectServer();
   void sendToRedirectServer(OsMsg& eventMessage);


   /* ============================ REG EVENT SERVER ============================ */
   /// Create and start the server that handles "reg" events.
   void startEventServer();
};

#endif  // _SipRegistrar_h_
