//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _SipTlsServer_h_
#define _SipTlsServer_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <net/SipTcpServer.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class SipTlsServer : public SipTcpServer
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   SipTlsServer(int sipPort,
                SipUserAgent* userAgent,
                UtlBoolean bUseNextAvailablePort = FALSE,
                const char* szBindAddr = NULL);
     //:Default constructor

   virtual
   ~SipTlsServer();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    // Caller must hold mClientLock.
    UtlBoolean createServerSocket(const char* szBindAddr,
                                  int& port,
                                  const UtlBoolean& bUseNextAvailablePort);

    virtual OsSocket* buildClientSocket(int hostPort,
                                        const char* hostAddress,
                                        const char* localIp,
                                        bool& existingSocketReused);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    SipTlsServer(const SipTlsServer& rSipTlsServer);
    //: disable Copy constructor

    SipTlsServer& operator=(const SipTlsServer& rhs);
    //:disable Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipTlsServer_h_
