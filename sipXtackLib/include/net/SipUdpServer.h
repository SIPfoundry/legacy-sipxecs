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
class OsStunDatagramSocket ;
class OsNotification ;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class SipUdpServer : public SipProtocolServerBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   SipUdpServer(int sipPort = SIP_PORT,
       SipUserAgent* userAgent = NULL,
       const char* natPingUrl = "",
       int natPingFrequency = 0,
       const char* natPingMethod = "PING",
       int udpReadBufferSize = -1,
       UtlBoolean bUseNextAvailablePort = FALSE,
       const char* szBoundIp = NULL);
     //:Default constructor


   virtual
   ~SipUdpServer();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

    int run(void* pArg);

    void shutdownListener();

    void enableStun(const char* szStunServer, 
                    const char* szLocalIp, 
                    int refreshPeriodInSecs, 
                    int stunOptions,
                    OsNotification* pNotification) ;
      //:Enable stun lookups for UDP signaling
      // Use a NULL szStunServer to disable

    UtlBoolean sendTo(const SipMessage& message,
                     const char* address,
                     int port,
                     const char* szLocalSipIp = NULL);

/* ============================ ACCESSORS ================================= */

    void printStatus();

    int getServerPort(const char* szLocalIp = NULL) ;

    UtlBoolean getStunAddress(UtlString* pIpAddress,
                              int* pPort,
                              const char* szLocalIp = NULL) ;

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    OsSocket* buildClientSocket(int hostPort,
                                const char* hostAddress,
                                const char* localIp);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    UtlString mNatPingUrl;
    int mNatPingFrequencySeconds;
    UtlString mNatPingMethod;
    UtlString mStunServer ;
    int mStunRefreshSecs ;
    int mStunOptions ;
    OsStatus createServerSocket(const char* localIp,
                                 int& localPort,
                                 const UtlBoolean& bUseNextAvailablePort,
                                 int udpReadBufferSize);

    SipUdpServer(const SipUdpServer& rSipUdpServer);
    //: disable Copy constructor

    SipUdpServer& operator=(const SipUdpServer& rhs);
     //:disable Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipUdpServer_h_
