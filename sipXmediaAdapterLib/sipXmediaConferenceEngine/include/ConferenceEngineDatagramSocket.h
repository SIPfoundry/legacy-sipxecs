//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ConferenceEngineDatagramSocket_h_
#define _ConferenceEngineDatagramSocket_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsStunDatagramSocket.h"
#include "ConferenceEngine.h"
#include "os/OsMsgQ.h"
#include "os/OsTimer.h"

// DEFINES
#define TYPE_RTP    0
#define TYPE_RTCP   1

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class ConferenceEngine ;

//: ConferenceEngineDatagramSocket creates a OsDatagramSocket and automatically initiates
//: stun lookups and keep-alives.
class ConferenceEngineDatagramSocket : public OsStunDatagramSocket
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
    ConferenceEngineDatagramSocket(ConferenceEngine* pConferenceEngine,
                                   int channel,
                                   int type,
                                   int remoteHostPort,
                                   const char* remoteHostName,
                                   int localHostPort = 0,
                                   const char* localHostName = NULL,
                                   bool bEnable = TRUE,
                                   const char* szStunServer = "larry.gloo.net",
                                   int iRefreshPeriodInSec = 28) ;

      //: Constructor accepting remote host port, name and optional local
      //: host name and port, and stun attributes.

    virtual ~ConferenceEngineDatagramSocket();
       //:Destructor

/* ============================ MANIPULATORS ============================== */

    virtual void pushPacket() ;

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    ConferenceEngine* mpConferenceEngine ;
    int miConferenceEngineChannel ;
    int miType ;
};

/* ============================ INLINE METHODS ============================ */


class ConferenceEngineSocketAdapter : public GIPS_transport
{
public:
    ConferenceEngineSocketAdapter(ConferenceEngineDatagramSocket* pRtpSocket,
                                  ConferenceEngineDatagramSocket* pRtcpSocket)
    {
        mpRtpSocket = pRtpSocket ;
        mpRtcpSocket = pRtcpSocket ;
    }

    virtual ~ConferenceEngineSocketAdapter()
    {

    }

    virtual void SendPacket(int channel, const void *data, int len)
    {
        mpRtpSocket->write((char*) data, len) ;
    }

    virtual void SendRTCPPacket(int channel, const void *data, int len)
    {
        mpRtcpSocket->write((char*) data, len) ;
    }

private:

    ConferenceEngineDatagramSocket* mpRtpSocket ;
    ConferenceEngineDatagramSocket* mpRtcpSocket ;
} ;

#endif  // _ConferenceEngineDatagramSocket_h_
