//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _INCLUDED_NETINTASK_H /* [ */
#define _INCLUDED_NETINTASK_H

#include "rtcp/RtcpConfig.h"

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsTask.h"
#include "os/OsLock.h"
#include "os/OsSocket.h"
#include "os/OsMutex.h"
#include "mp/MpTypes.h"
#include "mp/MpBuf.h"
#include "mp/MpMisc.h"

class OsNotification;

// DEFINES
#define CODEC_TYPE_PCMU 0
#define CODEC_TYPE_PCMA 8
#define CODEC_TYPE_L16  11

#define RTP_DIR_IN  1
#define RTP_DIR_OUT 2
#define RTP_DIR_NEW 4

#define RTCP_DIR_IN  1
#define RTCP_DIR_OUT 2
#define RTCP_DIR_NEW 4

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS

class OsConnectionSocket;
class OsServerSocket;
class OsSocket;

struct rtpHeader {
        UCHAR vpxcc;
        UCHAR mpt;
        USHORT seq;      /* Big Endian! */
        UINT timestamp;  /* Big Endian! */
        UINT ssrc;       /* Big Endian, but random */
};

struct rtpSession {
        UCHAR vpxcc; /* Usually: ((2<<6) | (0<<5) | (0<<4) | 0) */
        UCHAR mpt;   /* Usually: ((0<<7) | 0) */
        USHORT seq;
        UINT timestamp;
        UINT ssrc;
        OsSocket* socket;
        int dir;
        UINT packets;
        UINT octets;
        USHORT cycles;
};

#ifndef INCLUDE_RTCP /* [ */
struct __MprRtcpStats {
   UINT ssrc;
   short seqNumCycles;
   USHORT highSeqNum;
};

// TYPEDEFS

typedef struct __MprRtcpStats  MprRtcpStats;
typedef struct __MprRtcpStats* MprRtcpStatsPtr;
#endif /* INCLUDE_RTCP ] */

typedef struct rtpSession *rtpHandle;
typedef struct rtcpSession *rtcpHandle;

// FORWARD DECLARATIONS
class MprFromNet;

extern UINT rand_timer32(void);
extern rtpHandle StartRtpSession(OsSocket* socket, int direction, char type);
extern OsStatus setRtpType(rtpHandle h, int codecType);
extern OsStatus setRtpSocket(rtpHandle h, OsSocket* socket);
extern OsSocket* getRtpSocket(rtpHandle h);
extern void FinishRtpSession(rtpHandle h);
extern rtcpHandle StartRtcpSession(int direction);
extern OsStatus setRtcpSocket(rtcpHandle h, OsSocket* socket);
extern OsSocket* getRtcpSocket(rtcpHandle h);
extern void FinishRtcpSession(rtcpHandle h);

extern OsStatus startNetInTask();
extern OsStatus shutdownNetInTask();
extern OsStatus addNetInputSources(OsSocket* pRtpSocket,
            OsSocket* pRtcpSocket, MprFromNet* fwdTo, OsNotification* note);
extern OsStatus removeNetInputSources(MprFromNet* fwdTo, OsNotification* note);

class NetInTask : public OsTask
{

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   static const int DEF_NET_IN_TASK_PRIORITY;      // default task priority
   static const int DEF_NET_IN_TASK_OPTIONS;       // default task options
   static const int DEF_NET_IN_TASK_STACKSIZE;     // default task stacksize

/* ============================ CREATORS ================================== */

   static NetInTask* getNetInTask();
     //:Return a pointer to the NetIn task, creating it if necessary

   virtual
   ~NetInTask();
     //:Destructor

   int getWriteFD();


   void shutdownSockets();

/* ============================ MANIPULATORS ============================== */
         virtual int run(void* pArg);
/* ============================ ACCESSORS ================================= */

   OsConnectionSocket* getWriteSocket(void);
   OsConnectionSocket* getReadSocket(void);
   void openWriteFD(void);
   static OsMutex& getLockObj() { return sLock; }

/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   NetInTask(
      int prio    = DEF_NET_IN_TASK_PRIORITY,      // default task priority
      int options = DEF_NET_IN_TASK_OPTIONS,       // default task options
      int stack   = DEF_NET_IN_TASK_STACKSIZE);    // default task stacksize
     //:Default constructor

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   // Static data members used to enforce Singleton behavior
   static NetInTask* spInstance;    // pointer to the single instance of
                                    //  the MpNetInTask class
   static OsMutex     sLock;         // semaphore used to ensure that there
                                    //  is only one instance of this class

   OsConnectionSocket* mpWriteSocket;
   OsConnectionSocket* mpReadSocket;
   int               mCmdPort;      // internal socket port number

   NetInTask(const NetInTask& rNetInTask);
     //:Copy constructor (not implemented for this task)

   NetInTask& operator=(const NetInTask& rhs);
     //:Assignment operator (not implemented for this task)

};

#endif /* _INCLUDED_NETINTASKH ] */
