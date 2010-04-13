//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _INCLUDED_VoiceEngineNetTask_H /* [ */
#define _INCLUDED_VoiceEngineNetTask_H

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/qsTypes.h"
#include "os/OsTask.h"
#include "os/OsLock.h"
#include "os/OsSocket.h"
#include "os/OsRWMutex.h"
#include "os/OsProtectEvent.h"
#if defined(__pingtel_on_posix__)
#include <mp/MpMisc.h>
#endif

class OsNotification;
class VoiceEngineNetTask ;

// DEFINES

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
class VoiceEngineDatagramSocket ;

#ifdef foo
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
#endif

typedef struct rtpSession *rtpHandle;
typedef struct rtcpSession *rtcpHandle;

// FORWARD DECLARATIONS
class MprFromNet;

extern UINT rand_timer32(void);

extern OsStatus startVoiceEngineNetTask();
extern OsStatus shutdownVoiceEngineNetTask();
extern OsStatus addNetInputSources(OsSocket* pRtpSocket,
            OsSocket* pRtcpSocket, MprFromNet* fwdTo, OsNotification* note);
extern OsStatus removeNetInputSources(MprFromNet* fwdTo, OsNotification* note);

class VoiceEngineNetTask : public OsTask
{

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   static const int DEF_NET_IN_TASK_PRIORITY;      // default task priority
   static const int DEF_NET_IN_TASK_OPTIONS;       // default task options
   static const int DEF_NET_IN_TASK_STACKSIZE;     // default task stacksize

/* ============================ CREATORS ================================== */

   static VoiceEngineNetTask* getVoiceEngineNetTask();
     //:Return a pointer to the NetIn task, creating it if necessary

   virtual
   ~VoiceEngineNetTask();
     //:Destructor

   void shutdownSockets();

/* ============================ MANIPULATORS ============================== */
    virtual int run(void* pArg);

    virtual OsStatus addInputSource(VoiceEngineDatagramSocket* pSocket) ;
    virtual OsStatus removeInputSource(VoiceEngineDatagramSocket* pSocket, OsProtectedEvent* pEvent = NULL) ;

/* ============================ ACCESSORS ================================= */

   OsConnectionSocket* getWriteSocket(void);
   OsConnectionSocket* getReadSocket(void);

   // void openWriteFD(void);

   static OsRWMutex& getLockObj() { return sLock; }

/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   VoiceEngineNetTask(
      int prio    = DEF_NET_IN_TASK_PRIORITY,      // default task priority
      int options = DEF_NET_IN_TASK_OPTIONS,       // default task options
      int stack   = DEF_NET_IN_TASK_STACKSIZE);    // default task stacksize
     //:Default constructor

   int processControlSocket(int last) ;


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   // Static data members used to enforce Singleton behavior
   static VoiceEngineNetTask* spInstance;    // pointer to the single instance of
                                    //  the MpVoiceEngineNetTask class
   static OsRWMutex     sLock;         // semaphore used to ensure that there
                                    //  is only one instance of this class

   OsConnectionSocket* mpWriteSocket;
   OsConnectionSocket* mpReadSocket;

   int               mCmdPort;      // internal socket port number

   VoiceEngineNetTask(const VoiceEngineNetTask& rVoiceEngineNetTask);
     //:Copy constructor (not implemented for this task)

   VoiceEngineNetTask& operator=(const VoiceEngineNetTask& rhs);
     //:Assignment operator (not implemented for this task)
};

#endif /* _INCLUDED_VoiceEngineNetTaskH ] */
