//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _SipUdpServer_h_
#define _SipUdpServer_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
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
class OsStunDatagramSocket;
class OsNotification;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class SipUdpServer : public SipProtocolServerBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   SipUdpServer(int sipPort,
                SipUserAgent* userAgent,
                int udpReadBufferSize = -1,
                UtlBoolean bUseNextAvailablePort = FALSE,
                const char* szBoundIp = NULL);
     //:Default constructor

   virtual
   ~SipUdpServer();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

    void shutdownListener();

    int run(void* pArg);

    // Handles an incoming message (from the message queue).
    UtlBoolean handleMessage(OsMsg& eventMessage);

    UtlBoolean sendTo(SipMessage& message,
                     const char* address,
                     int port,
                     const char* szLocalSipIp = NULL);

    void enableStun(const char* szStunServer,
                    const char* szLocalIp,
                    int refreshPeriodInSecs,
                    int stunOptions,
                    OsNotification* pNotification) ;
      //:Enable stun lookups for UDP signaling
      // Use a NULL szStunServer to disable

/* ============================ ACCESSORS ================================= */

    int getServerPort(const char* szLocalIp = NULL);

    UtlBoolean getStunAddress(UtlString* pIpAddress,
                              int* pPort,
                              const char* szLocalIp = NULL);

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    OsSocket* buildClientSocket(int hostPort,
                                const char* hostAddress,
                                const char* localIp,
                                bool& existingSocketReused);

    // Caller must hold mClientLock.
    void printStatus();

    UtlString mStunServer;
    int mStunRefreshSecs;
    int mStunOptions;

    /** Create a listening socket and record it in mServerSocketMap and
     *  mServerPortMap.
     *  Manipulates the server list, and so should be called only by the Sip*Server
     *  constructors/destructors and the threads that call them.
     */
    // Caller must hold mClientLock.
    void createServerSocket(const char* localIp,
                            ///< Local address to listen on
                            int& localPort,
                            ///< Local port to listen on
                            const UtlBoolean& bUseNextAvailablePort,
                            ///< True if should search for an available port
                            int udpReadBufferSize
                            /**< If >0, attempt to set the kernel UDP read
                             *   buffer size to this value. */
       );

    // Send the keepalives.
    void sendKeepalives();

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    SipUdpServer(const SipUdpServer& rSipUdpServer);
    //: disable Copy constructor

    SipUdpServer& operator=(const SipUdpServer& rhs);
     //:disable Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipUdpServer_h_
