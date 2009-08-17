//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _VoiceEngineDatagramSocket_h_
#define _VoiceEngineDatagramSocket_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsStunDatagramSocket.h"
#ifdef _WIN32
#include "GipsVoiceEngineLib.h"
#else
#include "GipsVoiceEngineLibLinux.h"
#endif
#include "os/OsMsgQ.h"
#include "os/OsTimer.h"

// DEFINES
#define TYPE_AUDIO_RTP    0
#define TYPE_AUDIO_RTCP   1
#define TYPE_VIDEO_RTP    2
#define TYPE_VIDEO_RTCP   3

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class GipsVoiceEngineLib ;
class GipsVideoEngineWindows ;

//: VoiceEngineDatagramSocket creates a OsDatagramSocket and automatically initiates
//: stun lookups and keep-alives.
class VoiceEngineDatagramSocket : public OsStunDatagramSocket
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
    VoiceEngineDatagramSocket(GipsVoiceEngineLib* pVoiceEngine,
                              GipsVideoEngineWindows* pVideoEngine,
                              int audioChannel,
                              int videoChannel,
                              int type,
                              int remoteHostPort,
                              const char* remoteHostName,
                              int localHostPort = 0,
                              const char* localHostName = NULL,
                              bool bEnable = TRUE,
                              const char* szStunServer = "larry.gloo.net",
                              int iRefreshPeriodInSec = 28,
                              int stunOptions = STUN_OPTION_NORMAL) ;

      //: Constructor accepting remote host port, name and optional local
      //: host name and port, and stun attributes.

    virtual ~VoiceEngineDatagramSocket();
       //:Destructor

/* ============================ MANIPULATORS ============================== */

    virtual void pushPacket() ;

    void setVideoChannel(int channelId) { miVideoEngineChannel = channelId; }

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    GipsVoiceEngineLib* mpVoiceEngine ;
    GipsVideoEngineWindows *mpVideoEngine ;
    int miVoiceEngineChannel ;
    int miVideoEngineChannel ;
    int miType ;
};

/* ============================ INLINE METHODS ============================ */


class VoiceEngineSocketAdapter : public GIPS_transport
{
public:
    VoiceEngineSocketAdapter(VoiceEngineDatagramSocket* pRtpSocket,
                             VoiceEngineDatagramSocket* pRtcpSocket)
    {
        mpRtpSocket = pRtpSocket ;
        mpRtcpSocket = pRtcpSocket ;
    }

    ~VoiceEngineSocketAdapter()
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
    VoiceEngineDatagramSocket* mpRtpSocket ;
    VoiceEngineDatagramSocket* mpRtcpSocket ;

} ;


#endif  // _VoiceEngineDatagramSocket_h_
