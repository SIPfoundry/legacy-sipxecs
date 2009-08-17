//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _INCLUDED_ConferenceEngineNetTask_H /* [ */
#define _INCLUDED_ConferenceEngineNetTask_H

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/qsTypes.h>
#include <os/OsTask.h>
#include <os/OsLock.h>
#include <os/OsSocket.h>
#include <os/OsRWMutex.h>
#include <os/OsProtectEvent.h>

class OsNotification;
class ConferenceEngineNetTask ;

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
class ConferenceEngineDatagramSocket ;

typedef struct rtpSession *rtpHandle;
typedef struct rtcpSession *rtcpHandle;

// FORWARD DECLARATIONS
class MprFromNet;

extern UINT rand_timer32(void);

extern OsStatus startConferenceEngineNetTask();
extern OsStatus shutdownConferenceEngineNetTask();
extern OsStatus addNetInputSources(OsSocket* pRtpSocket,
            OsSocket* pRtcpSocket, MprFromNet* fwdTo, OsNotification* note);
extern OsStatus removeNetInputSources(MprFromNet* fwdTo, OsNotification* note);

class ConferenceEngineNetTask : public OsTask
{

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    static const int DEF_NET_IN_TASK_PRIORITY;      // default task priority
    static const int DEF_NET_IN_TASK_OPTIONS;       // default task options
    static const int DEF_NET_IN_TASK_STACKSIZE;     // default task stacksize

/* ============================ CREATORS ================================== */

    static ConferenceEngineNetTask* getConferenceEngineNetTask();
     //:Return a pointer to the NetIn task, creating it if necessary

    virtual
    ~ConferenceEngineNetTask();
     //:Destructor

    void shutdownSockets();

/* ============================ MANIPULATORS ============================== */
    virtual int run(void* pArg);

    virtual OsStatus addInputSource(ConferenceEngineDatagramSocket* pSocket) ;
    virtual OsStatus removeInputSource(ConferenceEngineDatagramSocket* pSocket, OsProtectedEvent* pEvent = NULL) ;

/* ============================ ACCESSORS ================================= */

    OsConnectionSocket* getWriteSocket(void);
    OsConnectionSocket* getReadSocket(void);

    // void openWriteFD(void);

    static OsRWMutex& getLockObj() { return sLock; }

/* ============================ INQUIRY =================================== */
/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

    ConferenceEngineNetTask(
      int prio    = DEF_NET_IN_TASK_PRIORITY,      // default task priority
      int options = DEF_NET_IN_TASK_OPTIONS,       // default task options
      int stack   = DEF_NET_IN_TASK_STACKSIZE);    // default task stacksize
     //:Default constructor

    int processControlSocket(int last) ;


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    // Static data members used to enforce Singleton behavior
    static ConferenceEngineNetTask* spInstance;    // pointer to the single instance of
                                    //  the MpConferenceEngineNetTask class
    static OsRWMutex     sLock;         // semaphore used to ensure that there
                                    //  is only one instance of this class

    OsConnectionSocket* mpWriteSocket;
    OsConnectionSocket* mpReadSocket;

    int               mCmdPort;      // internal socket port number

    ConferenceEngineNetTask(const ConferenceEngineNetTask& rConferenceEngineNetTask);
     //:Copy constructor (not implemented for this task)

    ConferenceEngineNetTask& operator=(const ConferenceEngineNetTask& rhs);
     //:Assignment operator (not implemented for this task)
};

#endif /* _INCLUDED_ConferenceEngineNetTaskH ] */
