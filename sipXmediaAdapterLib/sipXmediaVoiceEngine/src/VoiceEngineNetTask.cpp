//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////


//#include "rtcp/RtcpConfig.h"

#define LOGEM
#undef LOGEM

// SYSTEM INCLUDES

#include "os/OsDefs.h"
#include <assert.h>
#include <string.h>
#include "os/OsTask.h"
#include "include/VoiceEngineDatagramSocket.h"

#ifdef WIN32 /* [ */
#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#endif /* WIN32 ] */

#if defined(__pingtel_on_posix__) /* [ */
#include <stdlib.h>
#include <sys/time.h>
#include <mp/MpMisc.h>
#endif /* __pingtel_on_posix__ ] */

// APPLICATION INCLUDES

#include "os/OsDefs.h"
#include "os/OsTask.h"
#include "os/OsServerSocket.h"
#include "os/OsConnectionSocket.h"
#include "os/OsEvent.h"
#include "include/VoiceEngineNetTask.h"

// EXTERNAL FUNCTIONS

// EXTERNAL VARIABLES

// CONSTANTS

#define NET_TASK_MAX_MSG_LEN sizeof(VoiceEngineNetTaskMsg)
#define NET_TASK_MAX_FD_PAIRS 100

#define MAX_RTP_BYTES 1500

struct __VoiceEngineNetTaskMsg
{
/*
   OsSocket* pRtpSocket;
   OsSocket* pRtcpSocket;
   MprFromNet* fwdTo;
*/
   OsProtectedEvent* notify;

   enum { ADD, REMOVE, SHUTDOWN } operation ;
   VoiceEngineDatagramSocket* pSocket ;

};

typedef struct __VoiceEngineNetTaskMsg VoiceEngineNetTaskMsg, *VoiceEngineNetTaskMsgPtr;

// STATIC VARIABLE INITIALIZATIONS

static  VoiceEngineNetTaskMsg pairs[NET_TASK_MAX_FD_PAIRS];
static  int numPairs;

VoiceEngineNetTask* VoiceEngineNetTask::spInstance = 0;
OsRWMutex     VoiceEngineNetTask::sLock(OsBSem::Q_PRIORITY);

const int VoiceEngineNetTask::DEF_NET_IN_TASK_PRIORITY  = 100; // default task priority
const int VoiceEngineNetTask::DEF_NET_IN_TASK_OPTIONS   = 0;   // default task options
#ifdef USING_NET_EQ /* [ */
const int VoiceEngineNetTask::DEF_NET_IN_TASK_STACKSIZE = 40960;//default task stacksize
#else /* USING_NET_EQ ] [ */
const int VoiceEngineNetTask::DEF_NET_IN_TASK_STACKSIZE = 4096;// default task stacksize
#endif /* USING_NET_EQ ] */

#define DEBUG
#undef  DEBUG


/************************************************************************/

/*
void VoiceEngineNetTask::openWriteFD()
{
    mpWriteSocket = new OsConnectionSocket(mCmdPort, NULL);
}
*/

/*
int VoiceEngineNetTask::getWriteFD()
{
    // connect to the socket
    sLock.acquireRead();
    if (NULL != mpWriteSocket)
    {
        sLock.releaseWrite();
        return mpWriteSocket->getSocketDescriptor();
    }

    if (OsTask::getCurrentTask() == VoiceEngineNetTask::spInstance)
    {
        OsEvent* pNotify;
        VoiceEngineNetTaskHelper* pHelper;

        // Start our helper thread to go open the socket
        pNotify = new OsEvent;
        pHelper = new VoiceEngineNetTaskHelper(this, pNotify);
        if (!pHelper->isStarted()) {
            pHelper->start();
        }
        pNotify->wait();
        delete pHelper;
        delete pNotify;
    }
    else
    {
        // we are in a different thread already, go do it ourselves.
        osPrintf("Not VoiceEngineNetTask: opening connection directly\n");
        OsSysLog::add(FAC_MP, PRI_DEBUG, "Not VoiceEngineNetTask: opening connection directly\n");
        openWriteFD();
    }
    sLock.releaseRead();
    return mpWriteSocket->getSocketDescriptor();
}
*/


OsConnectionSocket* VoiceEngineNetTask::getWriteSocket()
{
    if (NULL == mpWriteSocket)
    {
        OsConnectionSocket* pWriteSocket = NULL;

        int trying = 1000;
        while (trying > 0)
        {
            OsTask::delay(5);
            pWriteSocket = new OsConnectionSocket(mCmdPort, "127.0.0.1", TRUE, "127.0.0.1") ;
            if (pWriteSocket && pWriteSocket->isConnected())
            {
                break;
            }
            else
            {
                delete pWriteSocket ;
                pWriteSocket = NULL ;
            }
            trying--;
        }
        mpWriteSocket = pWriteSocket ;
    }
    return mpWriteSocket;
}


OsConnectionSocket* VoiceEngineNetTask::getReadSocket()
{
    return mpReadSocket;
}


OsStatus VoiceEngineNetTask::addInputSource(VoiceEngineDatagramSocket* pSocket)
{
    int wrote = 0;

    OsConnectionSocket* writeSocket = getWriteSocket() ;
    if (writeSocket)
    {
        VoiceEngineNetTaskMsg msg;
        memset((void*)&msg, 0, sizeof(VoiceEngineNetTaskMsg));
        msg.operation = VoiceEngineNetTaskMsg::ADD ;
        msg.pSocket = pSocket ;

        wrote = writeSocket->write((char *) &msg, NET_TASK_MAX_MSG_LEN);
        if (wrote != NET_TASK_MAX_MSG_LEN)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                    "addNetInputSources - writeSocket error: 0x%p,%d wrote %d",
                    writeSocket, writeSocket->getSocketDescriptor(), wrote);
        }
    }
    else
    {
        assert(FALSE) ;
    }

    return ((NET_TASK_MAX_MSG_LEN == wrote) ? OS_SUCCESS : OS_BUSY);
}


OsStatus VoiceEngineNetTask::removeInputSource(VoiceEngineDatagramSocket* pSocket, OsProtectedEvent* pEvent)
{
    int wrote = 0;

    OsConnectionSocket* writeSocket = getWriteSocket() ;
    if (writeSocket)
    {
        VoiceEngineNetTaskMsg msg;
        memset((void*)&msg, 0, sizeof(VoiceEngineNetTaskMsg));
        msg.operation = VoiceEngineNetTaskMsg::REMOVE ;
        msg.pSocket = pSocket ;
        msg.notify = pEvent;

        wrote = writeSocket->write((char *) &msg, NET_TASK_MAX_MSG_LEN);
        if (wrote != NET_TASK_MAX_MSG_LEN)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                    "addNetInputSources - writeSocket error: 0x%p,%d wrote %d",
                    writeSocket, writeSocket->getSocketDescriptor(), wrote);
        }
    }
    else
    {
        assert(FALSE) ;
    }

    return ((NET_TASK_MAX_MSG_LEN == wrote) ? OS_SUCCESS : OS_BUSY);
}




/************************************************************************/
/*
static OsStatus get1Msg(OsSocket* pRxpSkt, MprFromNet* fwdTo, int rtpOrRtcp,
    int ostc)
{
        MpBufPtr ib;
        char junk[MAX_RTP_BYTES];
        int ret, nRead;
        struct rtpHeader *rp;
        struct in_addr fromIP;
        int      fromPort;

static  int numFlushed = 0;
static  int flushedLimit = 125;

        rp = (struct rtpHeader *) &junk[0];
        if (MpBufferMsg::AUD_RTP_RECV == rtpOrRtcp) {
           ib = MpBuf_getBuf(MpMisc.RtpPool, 0, 0, MP_FMT_RTPPKT);
        } else {
           ib = MpBuf_getBuf(MpMisc.RtcpPool, 0, 0, MP_FMT_RTCPPKT);
        }
        if (numFlushed >= flushedLimit) {
            Zprintf("get1Msg: flushed %d packets! (after %d DMA frames).\n",
               numFlushed, showFrameCount(1), 0,0,0,0);
            if (flushedLimit<1000000) {
                flushedLimit = flushedLimit << 1;
            } else {
                numFlushed = 0;
                flushedLimit = 125;
            }
        }
        if (NULL != ib) {
            nRead = ret = pRxpSkt->read(junk, MAX_RTP_BYTES,
                                                      &fromIP, &fromPort);
            MpBuf_setOsTC(ib, ostc);
            if (ret > 0) {
                if (ret > MpBuf_getByteLen(ib)) {
                    ret = MpBuf_getByteLen(ib);
                    if (MpBufferMsg::AUD_RTP_RECV == rtpOrRtcp) {
                        junk[0] &= ~0x20;
                    }
                }
                memcpy((char *) MpBuf_getStorage(ib), junk, ret);
                MpBuf_setNumSamples(ib, ret);
                MpBuf_setContentLen(ib, ret);
                fwdTo->pushPacket(ib, rtpOrRtcp, &fromIP, fromPort);
            } else {
                Zprintf(" *** get1Msg: read(0x%p) returned %d, errno=%d=0x%X)\n",
                    pRxpSkt, nRead, errno, errno, 0,0);
                MpBuf_delRef(ib);
                return OS_NO_MORE_DATA;
            }
        } else {
            nRead = pRxpSkt->read(junk, sizeof(junk));
            if (numFlushed++ < 10) {
                Zprintf("get1Msg: flushing a packet! (%d, %d, 0x%p)"
                    " (after %d DMA frames).\n",
                    nRead, errno, pRxpSkt, showFrameCount(1), 0,0);
            }
            if (nRead < 1) {
                return OS_NO_MORE_DATA;
            }
        }
        return OS_SUCCESS;
}
*/

int XXisFdPoison(int fd)
{
        fd_set fdset;
        fd_set *fds;
        int     numReady;
        struct  timeval tv, *ptv;

        if (0 > fd) {
            return TRUE;
        }

        tv.tv_sec = tv.tv_usec = 0;
        ptv = &tv;
        fds = &fdset;
        FD_ZERO(fds);
        FD_SET((UINT) fd, fds);
        numReady = select(fd+1, fds, NULL, NULL, ptv);
        return (0 > numReady) ? TRUE : FALSE;
}

int XXfindPoisonFds(int pipeFD)
{
        int i;
        int n = 0;
        VoiceEngineNetTaskMsgPtr ppr;

        if (XXisFdPoison(pipeFD)) {
            OsSysLog::add(FAC_MP, PRI_ERR, " *** VoiceEngineNetTask: pipeFd socketDescriptor=%d busted!\n", pipeFD);
            return -1;
        }
        for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) {
            if (ppr->pSocket && // not NULL socket and
                XXisFdPoison(ppr->pSocket->getSocketDescriptor())) {
                OsSysLog::add(FAC_MP, PRI_ERR, " *** VoiceEngineNetTask: Removing fdRtp[%d], socket=0x%p, socketDescriptor=%d\n", ppr-pairs,ppr->pSocket, (int)ppr->pSocket->getSocketDescriptor());
                n++;
                ppr->pSocket = NULL;
            }
            ppr++;
        }
        return n;
}

int VoiceEngineNetTask::processControlSocket(int last)
{
    VoiceEngineNetTaskMsg    msg;
    VoiceEngineNetTaskMsgPtr ppr;
    int i ;
    int newFd ;

    memset((void*)&msg, 0, sizeof(VoiceEngineNetTaskMsg));
    if (NET_TASK_MAX_MSG_LEN != mpReadSocket->read((char *) &msg, NET_TASK_MAX_MSG_LEN))
    {
        OsSysLog::add(FAC_MP, PRI_ERR, "VoiceEngineNetTask::run: Invalid request!") ;
    }
    else if (-2 == (int) msg.pSocket)
    {
        // Request to exit

        OsSysLog::add(FAC_MP, PRI_ERR, " *** VoiceEngineNetTask: closing pipeFd (%d)\n",
            mpReadSocket->getSocketDescriptor());

        sLock.acquireWrite();
        if (mpReadSocket)
        {
            mpReadSocket->close();
            delete mpReadSocket;
            mpReadSocket = NULL;
        }
        sLock.releaseWrite();
    }
    else
    {
        switch (msg.operation)
        {
            case VoiceEngineNetTaskMsg::ADD:

                // Insert into the pairs list
                newFd  = (msg.pSocket) ? msg.pSocket->getSocketDescriptor() : -1;

                OsSysLog::add(FAC_MP, PRI_DEBUG, " *** VoiceEngineNetTask: Adding new sockets (socket: %p,%d)\n",
                          msg.pSocket, newFd);

                // Look for duplicate file descriptors; remove if found
                for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++)
                {
                    if (ppr->pSocket != NULL)
                    {
                        int existingFd  = (ppr->pSocket)  ? ppr->pSocket->getSocketDescriptor()  : -1;
                        UtlBoolean foundDupFd  = FALSE;

                        if ((existingFd >= 0) && (existingFd == newFd))
                        {
                            OsSysLog::add(FAC_MP, PRI_ERR,
                                    " *** VoiceEngineNetTask: Using a dup descriptor (New:%p,%d, Old:%p,%d)\n",
                                    msg.pSocket, newFd, ppr->pSocket, existingFd) ;
                            ppr->pSocket = NULL;
                            numPairs-- ;
                        }
                    }
                    ppr++;
                }

                // Add pair
                for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++)
                {
                    if (ppr->pSocket == NULL)
                    {
                        ppr->pSocket = msg.pSocket;
                        numPairs++;
                        OsSysLog::add(FAC_MP, PRI_DEBUG,
                                " *** VoiceEngineNetTask: Add socket Fds: (New=%p,%d)\n",
                                msg.pSocket, newFd);
                        break ;
                    }
                    ppr++;
                }

                // Update last
                last = max(last, newFd);

                break ;
            case VoiceEngineNetTaskMsg::REMOVE:
                for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++)
                {
                    if (msg.pSocket == ppr->pSocket)
                    {
                        OsSysLog::add(FAC_MP, PRI_DEBUG,
                                " *** VoiceEngineNetTask: Remove socket Fds: (Old=%p,%d)\n",
                                ppr->pSocket, ppr->pSocket->getSocketDescriptor()) ;
                        ppr->pSocket = NULL ;
                    }
                    last = OS_INVALID_SOCKET_DESCRIPTOR ;
                    numPairs-- ;
                    ppr++;
                }
                if (msg.notify->isInUse())
                {
                    msg.notify->signal(0);
                }
                else
                {
                    OsSysLog::add(FAC_MP, PRI_ERR,
                                " *** VoiceEngineNetTask: VoiceEngineNetTaskMsg::REMOVE: already signaled\n") ;
                }
                break ;
            case VoiceEngineNetTaskMsg::SHUTDOWN:
                break ;
            default:
                break ;
        }
    }

    return last ;
}


int VoiceEngineNetTask::run(void *pNotUsed)
{
    fd_set fdset;
    fd_set *fds;
    int     last;
    int     i;
    int     numReady;
    VoiceEngineNetTaskMsgPtr ppr;
    OsServerSocket* pBindSocket;

    // Setup control socket
    pBindSocket = new OsServerSocket(1, PORT_DEFAULT, "127.0.0.1") ;
    mCmdPort = pBindSocket->getLocalHostPort() ;
    assert(-1 != mCmdPort) ;
    mpReadSocket = pBindSocket->accept() ;
    pBindSocket->close();
    delete pBindSocket;

    // If we failed to setup the control socket, abort...
    if (NULL == mpReadSocket)
    {
        printf(" *** VoiceEngineNetTask: accept() failed!\n", 0,0,0,0,0,0);
        return 0;
    }

    // Clean up our control structures
    for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++)
    {
        ppr->pSocket =  NULL;
        ppr++;
    }
    numPairs = 0;


    fds = &fdset;
    last = OS_INVALID_SOCKET_DESCRIPTOR;

    // Only run while the control socket is stable
    while (mpReadSocket && mpReadSocket->isOk())
    {

        // Figure out the max socket desc number
        if (OS_INVALID_SOCKET_DESCRIPTOR == last)
        {
           last = mpReadSocket->getSocketDescriptor();
           for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++)
           {
                if (NULL != ppr->pSocket)
                {
                    last=max(last, ppr->pSocket->getSocketDescriptor());
                }
                ppr++;
            }
        }

        // Build the fdset
        FD_ZERO(fds);
        FD_SET((UINT) mpReadSocket->getSocketDescriptor(), fds);
        for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++)
        {
            if (NULL != ppr->pSocket)
            {
                FD_SET((UINT)(ppr->pSocket->getSocketDescriptor()), fds);
            }
            ppr++;
        }

        // Wait for some data
        errno = 0;
        numReady = select(last+1, fds, NULL, NULL, NULL);
        if (numReady < 0)
        {
            OsSysLog::add(FAC_MP, PRI_ERR, " *** VoiceEngineNetTask: select returned %d, errno=%d=0x%X\n",
                numReady, errno, errno);
            i = XXfindPoisonFds(mpReadSocket->getSocketDescriptor());
            if (i < 0)
            {
                OsSysLog::add(FAC_MP, PRI_ERR, " *** VoiceEngineNetTask: My comm socket failed! Quitting!\n");
                mpReadSocket->close();
            }
            else if (i > 0)
            {
                last = OS_INVALID_SOCKET_DESCRIPTOR;
            }
            continue;
        }


        // Process the control socket
        if (FD_ISSET(mpReadSocket->getSocketDescriptor(), fds))
        {
            numReady-- ;
            last = processControlSocket(last) ;
        }

        ppr=pairs;
        for (i=0; ((i<NET_TASK_MAX_FD_PAIRS)&&(numReady>0)); i++)
        {
            if ((ppr->pSocket != NULL) &&
                    (FD_ISSET(ppr->pSocket->getSocketDescriptor(), fds)))
            {
                ppr->pSocket->pushPacket() ;
            }
            ppr++;
        }
    }
    return 0;
}


VoiceEngineNetTask* VoiceEngineNetTask::getVoiceEngineNetTask()
{
   UtlBoolean isStarted;
   // OsStatus  stat;

   // If the task object already exists, and the corresponding low-level task
   // has been started, then use it
   if (spInstance != NULL && spInstance->isStarted())
      return spInstance;

   // If the task does not yet exist or hasn't been started, then acquire
   // the lock to ensure that only one instance of the task is started
   sLock.acquireRead();
   if (spInstance == NULL) {
       spInstance = new VoiceEngineNetTask();
   }
   isStarted = spInstance->isStarted();
   if (!isStarted)
   {
      isStarted = spInstance->start() ;
      spInstance->getWriteSocket();
      assert(isStarted);
   }
   sLock.releaseRead();
   return spInstance;
}

void VoiceEngineNetTask::shutdownSockets()
{
        getLockObj().acquireWrite();

        if (mpWriteSocket)
        {
            mpWriteSocket->close();
            delete mpWriteSocket;
            mpWriteSocket = NULL;
        }

        if (mpReadSocket)
        {
            mpReadSocket->close();
            delete mpReadSocket;
            mpReadSocket =  NULL;
        }
        getLockObj().releaseWrite();

}

// Default constructor (called only indirectly via getVoiceEngineNetTask())
VoiceEngineNetTask::VoiceEngineNetTask(int prio, int options, int stack)
:  OsTask("VoiceEngineNetTask", NULL, prio, options, stack),
   mpWriteSocket(NULL),
   mpReadSocket(NULL),
   mCmdPort(-1)
{
}

// Destructor
VoiceEngineNetTask::~VoiceEngineNetTask()
{
   spInstance = NULL;
}

// $$$ These messages need to contain OsSocket* instead of fds.

OsStatus shutdownVoiceEngineNetTask()
{
        VoiceEngineNetTask::getLockObj().acquireWrite();

        VoiceEngineNetTaskMsg msg;
        memset((void*)&msg, 0, sizeof(VoiceEngineNetTaskMsg));

        int wrote;
        VoiceEngineNetTask* pInst = VoiceEngineNetTask::getVoiceEngineNetTask();
        OsConnectionSocket* writeSocket;

        // pInst->getWriteFD();
        writeSocket = pInst->getWriteSocket();

        msg.operation = VoiceEngineNetTaskMsg::SHUTDOWN ;

        wrote = writeSocket->write((char *) &msg, NET_TASK_MAX_MSG_LEN);

        VoiceEngineNetTask::getLockObj().releaseWrite();
        pInst->shutdownSockets();

        VoiceEngineNetTask* pTask = VoiceEngineNetTask::getVoiceEngineNetTask();
        VoiceEngineNetTask::getLockObj().acquireWrite();
        pTask->requestShutdown();
        VoiceEngineNetTask::getLockObj().releaseWrite();
        return ((NET_TASK_MAX_MSG_LEN == wrote) ? OS_SUCCESS : OS_BUSY);
}


#define LOGEM
#undef LOGEM

/************************************************************************/

/* ============================ FUNCTIONS ================================= */
