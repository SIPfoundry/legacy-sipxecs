//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _SipTcpServer_h_
#define _SipTcpServer_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsServerSocket.h>
#include <net/SipProtocolServerBase.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipUserAgent;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class SipTcpServer : public SipProtocolServerBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   SipTcpServer(int sipPort,
                ///< Set to PORT_NONE when using as base for TLS server.
                SipUserAgent* userAgent,
                const char* taskName  = "SipTcpServer-%d",
                UtlBoolean bUseNextAvailablePort = FALSE,
                const char* szBindAddr = NULL,
                const char* protocolString = SIP_TRANSPORT_TCP
                ///< SIP_TRANSPORT_TLS when using as base for TLS server.
      );
     //:Default constructor

   virtual
   ~SipTcpServer();
     //:Destructor

/* ============================ MANIPULATORS ============================== */
    virtual UtlBoolean startListener();

    void shutdownListener();

    int run(void* pArg);

/* ============================ ACCESSORS ================================= */

    int getServerPort() const ;
    //: The the local server port for this server

    virtual int isOk();

/* ============================ INQUIRY =================================== */


/* ============================ Enumerations ============================== */
    enum EventSubTypes
    {
        SIP_SERVER_BROKER_NOTIFY = 1
    };

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    // Caller must hold mClientLock.
    UtlBoolean createServerSocket(const char* szBindAddr,
                                  int& port,
                                  const UtlBoolean& bUseNextAvailablePort);

    class SipServerBrokerListener : public OsServerTask
    {
        public:
            SipServerBrokerListener(SipTcpServer* pOwner) :
               OsServerTask("SipTcpServerBrokerListener-%d", (void*)pOwner),
               mpOwner(pOwner)
            {
               start();
            }
            virtual ~SipServerBrokerListener()
            {
                waitUntilShutDown();
            }
            virtual UtlBoolean handleMessage(OsMsg& rMsg);
        private:
            SipTcpServer* mpOwner;
    };

    friend class SipServerBrokerListener;
    SipServerBrokerListener* mpServerBrokerListener;

    virtual OsSocket* buildClientSocket(int hostPort,
                                        const char* hostAddress,
                                        const char* localIp,
                                        bool& existingSocketReused);

    int mServerPort;

    // Map from the local IP addresses (as UtlString's) to SipServerBroker's
    // that listen on them.
    UtlHashMap mServerBrokers;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    SipTcpServer(const SipTcpServer& rSipTcpServer);
        //: disable Copy constructor

    SipTcpServer& operator=(const SipTcpServer& rhs);
     //:disable Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipTcpServer_h_
