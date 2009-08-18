//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#include "rtcp/RtcpConfig.h"

#define LOGEM
#undef LOGEM

// SYSTEM INCLUDES

#include "os/OsDefs.h"
#include <assert.h>
#include <string.h>
#include "os/OsTask.h"
#ifdef _VXWORKS /* [ */
#include <selectLib.h>
#include <iosLib.h>
#include <inetlib.h>
#endif /* _VXWORKS ] */

#ifdef WIN32 /* [ */
#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#endif /* WIN32 ] */

#ifdef __pingtel_on_posix__ /* [ */
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/time.h>
#endif /* __pingtel_on_posix__ ] */

// APPLICATION INCLUDES

#include "os/OsDefs.h"
#include "os/OsTask.h"
#include "os/OsServerSocket.h"
#include "os/OsConnectionSocket.h"
#include "os/OsEvent.h"
#include "mp/NetInTask.h"
#include "mp/MprFromNet.h"
#include "mp/MpBufferMsg.h"
#include "mp/dmaTask.h"
#ifdef _VXWORKS /* [ */
#ifdef CPU_XSCALE /* [ */
#include "mp/pxa255.h"
#define OSTIMER_COUNTER_POINTER ((int*) PXA250_OSTIMER_OSCR)
#else /* CPU_XSCALE ] [ */
#include "mp/sa1100.h"
#define OSTIMER_COUNTER_POINTER ((int*) SA1100_OSTIMER_COUNTER)
#endif /* CPU_XSCALE ] */
#else /* _VXWORKS ][ */
#define OSTIMER_COUNTER_POINTER (&dummy0)
static int dummy0 = 0;
#endif /* _VXWORKS ] */

// EXTERNAL FUNCTIONS

// EXTERNAL VARIABLES

// CONSTANTS

#define NET_TASK_PIPE_NAME "/pipe/tcas1NetInTask"
#define NET_TASK_MAX_MSGS 10
#define NET_TASK_MAX_MSG_LEN sizeof(netInTaskMsg)
#define NET_TASK_MAX_FD_PAIRS 100

#define MAX_RTP_BYTES 1500

struct __netInTaskMsg {
   OsSocket* pRtpSocket;
   OsSocket* pRtcpSocket;
   MprFromNet* fwdTo;
   OsNotification* notify;
};

typedef struct __netInTaskMsg netInTaskMsg, *netInTaskMsgPtr;

// STATIC VARIABLE INITIALIZATIONS
volatile int* pOsTC = OSTIMER_COUNTER_POINTER;

static  netInTaskMsg pairs[NET_TASK_MAX_FD_PAIRS];
static  int numPairs;

NetInTask* NetInTask::spInstance = 0;
OsMutex    NetInTask::sLock(OsBSem::Q_PRIORITY);

const int NetInTask::DEF_NET_IN_TASK_PRIORITY  = 100; // default task priority
const int NetInTask::DEF_NET_IN_TASK_OPTIONS   = 0;   // default task options
#ifdef USING_NET_EQ /* [ */
const int NetInTask::DEF_NET_IN_TASK_STACKSIZE = 40960;//default task stacksize
#else /* USING_NET_EQ ] [ */
const int NetInTask::DEF_NET_IN_TASK_STACKSIZE = 4096;// default task stacksize
#endif /* USING_NET_EQ ] */

#define DEBUG
#undef  DEBUG

/************************************************************************/

// This task is launched when getWriteFD() called from the same task as
// the one that is about to call accept.

class NetInTaskHelper : public OsTask
{
public:
    NetInTaskHelper(NetInTask* task, OsNotification* notify);
    ~NetInTaskHelper();
    virtual int run(void* pArg);
private:
    int port;
    OsNotification* mpNotify;
};


NetInTaskHelper::NetInTaskHelper(NetInTask* pNIT, OsNotification* pNotify)
:  OsTask("NetInTaskHelper-%d", (void*)pNIT, 25, 0, 2000)
{
    mpNotify = pNotify;
}

NetInTaskHelper::~NetInTaskHelper()
{
    waitUntilShutDown();
}

int NetInTaskHelper::run(void* pInst)
{
    // osPrintf("NetInTaskHelper::run: Start\n");
    NetInTask* pNIT = (NetInTask*) pInst;
    int trying = 1000;
    OsConnectionSocket* pWriteSocket = NULL;
    while (trying > 0)
    {
       OsTask::delay(1);
       pNIT->openWriteFD();
       pWriteSocket = pNIT->getWriteSocket();
       if (pWriteSocket && pWriteSocket->isConnected()) break;
       trying--;
    }
    mpNotify->signal(0);
    OsSysLog::add(FAC_MP, PRI_INFO,
       "NetInTaskHelper::run()... returning 0, after %d tries\n", (1001 - trying));

    return 0;
}

/************************************************************************/
void NetInTask::openWriteFD()
{
    mpWriteSocket = new OsConnectionSocket(mCmdPort, NULL);
}

int NetInTask::getWriteFD()
{
    if (NULL != mpWriteSocket) {
        return mpWriteSocket->getSocketDescriptor();
    }

    // connect to the socket
    sLock.acquire();
    if (NULL != mpWriteSocket) {
        sLock.release();
        return mpWriteSocket->getSocketDescriptor();
    }

    if (OsTask::getCurrentTask() == NetInTask::spInstance) {
        OsEvent* pNotify;
        NetInTaskHelper* pHelper;

        // Start our helper thread to go open the socket
        pNotify = new OsEvent;
        pHelper = new NetInTaskHelper(this, pNotify);
        if (!pHelper->isStarted()) {
            pHelper->start();
        }
        pNotify->wait();
        delete pHelper;
        delete pNotify;
    } else {
        // we are in a different thread already, go do it ourselves.
        osPrintf("Not NetInTask: opening connection directly\n");
        OsSysLog::add(FAC_MP, PRI_DEBUG, "Not NetInTask: opening connection directly\n");
        openWriteFD();
    }
    sLock.release();
    return mpWriteSocket->getSocketDescriptor();
}

OsConnectionSocket* NetInTask::getWriteSocket()
{
    if (NULL == mpWriteSocket) {
        getWriteFD();
    }
    return mpWriteSocket;
}

OsConnectionSocket* NetInTask::getReadSocket()
{
    int i;

    for (i=0; ((i<10) && (NULL == mpReadSocket)); i++) {
        getWriteFD();
        OsTask::delay(100);
    }
    return mpReadSocket;
}

/************************************************************************/

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
#if 0
            nRead = ret = pRxpSkt->read(junk, MAX_RTP_BYTES, &fromIP, &fromPort);
            MpBuf_setOsTC(ib, ostc);
            if (ret > 0)
            {
                if (ret > MpBuf_getByteLen(ib)) {
                    ret = MpBuf_getByteLen(ib);
                    if (MpBufferMsg::AUD_RTP_RECV == rtpOrRtcp) {
                        junk[0] &= ~0x20; /* must turn off Pad flag */
                    }
                }
                memcpy((char *) MpBuf_getStorage(ib), junk, ret);
                MpBuf_setNumSamples(ib, ret);
                MpBuf_setContentLen(ib, ret);
                fwdTo->pushPacket(ib, rtpOrRtcp, &fromIP, fromPort);
            }
#else
            // Read directly into the MpBuf, so as to elimiate a memcpy
            MpBuf_setOsTC(ib, ostc);
            char *buffer = (char *)MpBuf_getStorage(ib) ;
            int len = MpBuf_getByteLen(ib) ;

            nRead = ret = pRxpSkt->read(buffer, len, &fromIP, &fromPort);
            if (ret > 0)
            {
                /*
                 * I really want to do a recvfrom with flag == MSG_TRUNC so
                 * we know how much was really available.  But OsSocket doesn't
                 * support that right now.  So instead just assume if we get
                 * exactly what we asked for, there was really more than we
                 * asked for available.  So the rest got truncated and lost.
                 * Look for
                 *    ret == len
                 * instead of the more natural
                 *    ret > len
                 */
                if (ret == len) { // packet truncated
                    if (MpBufferMsg::AUD_RTP_RECV == rtpOrRtcp) {
                        junk[0] &= ~0x20; /* must turn off Pad flag */
                    }
                }
                MpBuf_setNumSamples(ib, ret);
                MpBuf_setContentLen(ib, ret);
                fwdTo->pushPacket(ib, rtpOrRtcp, &fromIP, fromPort);
            }
#endif
            else
            {
                MpBuf_delRef(ib);
                if (!pRxpSkt->isOk())
                {
                    Zprintf(" *** get1Msg: read(%p) returned %d, errno=%d=0x%X)\n",
                        pRxpSkt, nRead, errno, errno, 0,0);
                    return OS_NO_MORE_DATA;
                }
            }
        } else {
            nRead = pRxpSkt->read(junk, sizeof(junk));
            if (numFlushed++ < 10) {
                Zprintf("get1Msg: flushing a packet! (%d, %d, %p)"
                    " (after %d DMA frames).\n",
                    nRead, errno, pRxpSkt, showFrameCount(1), 0,0);
            }
            if ((nRead < 1) && !pRxpSkt->isOk())
            {
                return OS_NO_MORE_DATA;
            }
        }
        return OS_SUCCESS;
}

static int selectErrors;

int isFdPoison(int fd)
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

int findPoisonFds(int pipeFD)
{
        int i;
        int n = 0;
        netInTaskMsgPtr ppr;

        if (isFdPoison(pipeFD)) {
            OsSysLog::add(FAC_MP, PRI_ERR, " *** NetInTask: pipeFd socketDescriptor=%d busted!\n", pipeFD);
            return -1;
        }
        for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) {
            if (ppr->pRtpSocket && // not NULL socket and
                isFdPoison(ppr->pRtpSocket->getSocketDescriptor())) {
                OsSysLog::add(FAC_MP, PRI_ERR, " *** NetInTask: Removing fdRtp[%ld], socket=0x%p, socketDescriptor=%d\n", (long)(ppr-pairs), ppr->pRtpSocket, ppr->pRtpSocket->getSocketDescriptor());
                n++;
                ppr->pRtpSocket = NULL;
                if (NULL == ppr->pRtcpSocket) ppr->fwdTo = NULL;
            }
            if (ppr->pRtcpSocket && // not NULL socket and
                isFdPoison(ppr->pRtcpSocket->getSocketDescriptor())) {
                OsSysLog::add(FAC_MP, PRI_ERR, " *** NetInTask: Removing fdRtcp[%ld], socket=0x%p, socketDescriptor=%d\n", (long)(ppr-pairs), ppr->pRtcpSocket, (int)ppr->pRtcpSocket->getSocketDescriptor());
                n++;
                ppr->pRtcpSocket = NULL;
                if (NULL == ppr->pRtpSocket) ppr->fwdTo = NULL;
            }
            ppr++;
        }
        return n;
}

static volatile int selectCounter = 0;
int whereSelectCounter()
{
    int save;
    save = selectCounter;
    selectCounter = 0;
    return save;
}

int showNetInTable() {
   int     last = 1234567;
   int     pipeFd;
   int     i;
   netInTaskMsgPtr ppr;
   NetInTask* pInst = NetInTask::getNetInTask();
   OsConnectionSocket* readSocket;

   // pInst->getWriteFD();
   readSocket = pInst->getReadSocket();

   pipeFd = readSocket->getSocketDescriptor();

   for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) {
      if (NULL != ppr->fwdTo) {
         if (NULL != ppr->pRtpSocket)
            last=max(last, ppr->pRtpSocket->getSocketDescriptor());
         if (NULL != ppr->pRtcpSocket)
            last=max(last, ppr->pRtcpSocket->getSocketDescriptor());
      }
      ppr++;
   }
   Zprintf("pipeFd = %d (last = %d)\n", pipeFd, last, 0,0,0,0);
   for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) {
      if (NULL != ppr->fwdTo) {
         Zprintf(" %2d: MprFromNet=0x%p, pRtpSocket: 0x%p, pRtcpSocket: 0x%p\n",
            i, ppr->fwdTo, ppr->pRtpSocket, ppr->pRtcpSocket, 0,0);
      }
      ppr++;
   }
   return last;
}

volatile int NetInWait = 0;

int NetInHalt() {
   NetInWait = 1;
   return 0;
}

int NetInResume() {
   NetInWait = 0;
   return 0;
}

int NetInTask::run(void *pNotUsed)
{
        fd_set fdset;
        fd_set *fds;
        int     last;
        int     i;
        OsStatus  stat;
        int     numReady;
        netInTaskMsg    msg;
        netInTaskMsgPtr ppr;
        OsServerSocket* pBindSocket;
        int     ostc;

        while (NetInWait) {
         ;
        }
        pBindSocket = new OsServerSocket(1);
        mCmdPort = pBindSocket->getLocalHostPort();
        // osPrintf("\n NetInTask: local comm port is %d\n\n", mCmdPort);
        assert(-1 != mCmdPort);
        // osPrintf("NetInTask: getting WriteFD\n");
        getWriteFD();
        // osPrintf("NetInTask: calling accept\n");
        mpReadSocket = pBindSocket->accept();
        // osPrintf("NetInTask: accept returned, closing server socket\n");
        pBindSocket->close();
        delete pBindSocket;
        if (NULL == mpReadSocket) {
            Zprintf(" *** NetInTask: accept() failed!\n", 0,0,0,0,0,0);
            return 0;
        }

        for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) {
            ppr->pRtpSocket =  NULL;
            ppr->pRtcpSocket = NULL;
            ppr->fwdTo = NULL;
            ppr++;
        }
        numPairs = 0;

        selectErrors = 0;

        fds = &fdset;
        last = OS_INVALID_SOCKET_DESCRIPTOR;
        Zprintf(" *** NetInTask: pipeFd is %d\n",
                       mpReadSocket->getSocketDescriptor(), 0,0,0,0,0);

        while (mpReadSocket && mpReadSocket->isOk()) {
            if (OS_INVALID_SOCKET_DESCRIPTOR == last) {
               last = mpReadSocket->getSocketDescriptor();
               for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) {
                  if (NULL != ppr->fwdTo) {
                    if (NULL != ppr->pRtpSocket)
                       last=max(last, ppr->pRtpSocket->getSocketDescriptor());
                    if (NULL != ppr->pRtcpSocket)
                       last=max(last, ppr->pRtcpSocket->getSocketDescriptor());
                  }
                  ppr++;
               }
            }
            FD_ZERO(fds);
            FD_SET((UINT) mpReadSocket->getSocketDescriptor(), fds);
            for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) {
              if (NULL != ppr->fwdTo) {
                if (NULL != ppr->pRtpSocket)
                {
                  int fd = ppr->pRtpSocket->getSocketDescriptor();
                  if (fd > 0)
                    FD_SET(fd, fds);
                }
                if (NULL != ppr->pRtcpSocket)
                {
                  int fd = ppr->pRtcpSocket->getSocketDescriptor();
                  if (fd > 0)
                    FD_SET(fd, fds);
                }
              }
              ppr++;
            }
            errno = 0;
            numReady = select(last+1, fds, NULL, NULL, NULL);
            ostc = *pOsTC;
            selectCounter++;
            if (0 > numReady) {
                OsSysLog::add(FAC_MP, PRI_ERR,
                              " *** NetInTask: select returned %d, errno=%d '%s'",
                              numReady, errno, strerror(errno));
                i = findPoisonFds(mpReadSocket->getSocketDescriptor());
                if (i < 0) {
                    OsSysLog::add(FAC_MP, PRI_ERR, " *** NetInTask: My comm socket failed! Quitting!\n");
                    mpReadSocket->close();
                } else if (0 < i) {
                    last = OS_INVALID_SOCKET_DESCRIPTOR;
                }
/*
                if (selectErrors++ > 10) {
                    OsSysLog::add(FAC_MP, PRI_ERR, " *** NetInTask: Quitting!\n");
                    mpReadSocket->close();
                }
*/
                continue;
            }

#ifdef DEBUG_BY_SUSPEND /* [ */
            if (0 == numReady) {
                Zprintf(" !!! select() returned 0!!!  errno=%d\n",
                    errno, 0,0,0,0,0);
                taskSuspend(0);
                continue;
            }
#endif /* DEBUG_BY_SUSPEND ] */

            /* is it a request to modify the set of file descriptors? */
            if (FD_ISSET(mpReadSocket->getSocketDescriptor(), fds)) {
                numReady--;
                if (NET_TASK_MAX_MSG_LEN !=
                     mpReadSocket->read((char *) &msg, NET_TASK_MAX_MSG_LEN)) {
                    osPrintf("NetInTask::run: Invalid request!\n");
                } else if (-2 == (intptr_t) msg.pRtpSocket) {
                    /* request to exit... */
                    Nprintf(" *** NetInTask: closing pipeFd (%d)\n",
                        mpReadSocket->getSocketDescriptor(), 0,0,0,0,0);
                    OsSysLog::add(FAC_MP, PRI_ERR, " *** NetInTask: closing pipeFd (%d)\n",
                        mpReadSocket->getSocketDescriptor());
                    sLock.acquire();
                    if (mpReadSocket)
                    {
                        mpReadSocket->close();
                       delete mpReadSocket;
                        mpReadSocket = NULL;
                    }
                    sLock.release();
                } else if (NULL != msg.fwdTo) {
                    if ((NULL != msg.pRtpSocket) || (NULL != msg.pRtcpSocket)) {
                        /* add a new pair of file descriptors */
                        last = max(last,msg.pRtpSocket->getSocketDescriptor());
                        last = max(last,msg.pRtcpSocket->getSocketDescriptor());
#define CHECK_FOR_DUP_DESCRIPTORS
#ifdef CHECK_FOR_DUP_DESCRIPTORS
                        int newRtpFd  = (msg.pRtpSocket)  ? msg.pRtpSocket->getSocketDescriptor()  : -1;
                        int newRtcpFd = (msg.pRtcpSocket) ? msg.pRtcpSocket->getSocketDescriptor() : -1;

                        OsSysLog::add(FAC_MP, PRI_DEBUG, " *** NetInTask: Adding new RTP/RTCP sockets (RTP:%p,%d, RTCP:%p,%d)\n",
                                      msg.pRtpSocket, newRtpFd, msg.pRtcpSocket, newRtcpFd);

                        for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) {
                            if (NULL != ppr->fwdTo) {
                                int existingRtpFd  = (ppr->pRtpSocket)  ? ppr->pRtpSocket->getSocketDescriptor()  : -1;
                                int existingRtcpFd = (ppr->pRtcpSocket) ? ppr->pRtcpSocket->getSocketDescriptor() : -1;
                                UtlBoolean foundDupRtpFd  = FALSE;
                                UtlBoolean foundDupRtcpFd = FALSE;

                                if (existingRtpFd >= 0 &&
                                    (existingRtpFd == newRtpFd || existingRtpFd == newRtcpFd))
                                {
                                    foundDupRtpFd = TRUE;
                                }

                                if (existingRtcpFd >= 0 &&
                                    (existingRtcpFd == newRtpFd || existingRtcpFd == newRtcpFd))
                                {
                                    foundDupRtcpFd = TRUE;
                                }

                                if (foundDupRtpFd || foundDupRtcpFd)
                                {
                                    OsSysLog::add(FAC_MP, PRI_ERR, " *** NetInTask: Using a dup descriptor (New RTP:%p,%d, New RTCP:%p,%d, Old RTP:%p,%d, Old RTCP:%p,%d)\n",
                                                  msg.pRtpSocket, newRtpFd, msg.pRtcpSocket, newRtcpFd, ppr->pRtpSocket, existingRtpFd, ppr->pRtcpSocket, existingRtcpFd);

                                    if (foundDupRtpFd)
                                        ppr->pRtpSocket = NULL;
                                    if (foundDupRtcpFd)
                                        ppr->pRtcpSocket = NULL;
                                    if (ppr->pRtpSocket == NULL && ppr->pRtcpSocket == NULL)
                                        ppr->fwdTo = NULL;
                                }
                            }
                            ppr++;
                        }
#endif
                        for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) {
                            if (NULL == ppr->fwdTo) {
                                // Enable non-blocking mode on these sockets.
                                // This works around a Linux Kernel bug that
                                // allows packets with incorrect UDP checksums
                                // to trip select() but then block in read().
                                // By making the sockets non-blocking, the
                                // read() just returns with EAGAIN.  This issue
                                // has been fixed in Linux: 2.6.10-rc3, but
                                // not in RH as of RHES4-U1
                                // See http://kerneltrap.org/node/4351
                                //   "[UDP]: Select handling of bad checksums."
                                msg.pRtpSocket->makeNonblocking() ;
                                msg.pRtcpSocket->makeNonblocking() ;

                                ppr->pRtpSocket  = msg.pRtpSocket;
                                ppr->pRtcpSocket = msg.pRtcpSocket;
                                ppr->fwdTo   = msg.fwdTo;
                                i = NET_TASK_MAX_FD_PAIRS;
                                numPairs++;
                                OsSysLog::add(FAC_MP, PRI_DEBUG, " *** NetInTask: Add socket Fds: RTP=%p, RTCP=%p, Q=%p\n",
                                              msg.pRtpSocket, msg.pRtcpSocket, msg.fwdTo);
                            }
                            ppr++;
                        }
                        if (NULL != msg.notify) {
                            msg.notify->signal(0);
                        }
                    } else {
                        /* remove a pair of file descriptors */
                        for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) {
                            if (msg.fwdTo == ppr->fwdTo) {
                                OsSysLog::add(FAC_MP, PRI_DEBUG, " *** NetInTask: Remove socket Fds: RTP=%p, RTCP=%p, Q=%p\n",
                                              ppr->pRtpSocket, ppr->pRtcpSocket, ppr->fwdTo);
                                ppr->pRtpSocket = NULL;
                                ppr->pRtcpSocket = NULL;
                                ppr->fwdTo = NULL;
                                numPairs--;
                                last = -1;
                            }
                            ppr++;
                        }
                        if (NULL != msg.notify) {
                            msg.notify->signal(0);
                        }
                    }
                }
                else // NULL FromNet, not good
                {
                    osPrintf("NetInTask::run msg with NULL FromNet\n");
                }
            }
            ppr=pairs;
            for (i=0; ((i<NET_TASK_MAX_FD_PAIRS)&&(numReady>0)); i++) {
                if ((NULL != ppr->pRtpSocket) &&
                  (FD_ISSET(ppr->pRtpSocket->getSocketDescriptor(), fds))) {
                    stat = get1Msg(ppr->pRtpSocket, ppr->fwdTo,
                       MpBufferMsg::AUD_RTP_RECV, ostc);
                    if (OS_SUCCESS != stat) {
                        Zprintf(" *** NetInTask: removing RTP#%ld pSkt=0x%p due"
                            " to read error.\n", ppr-pairs,
                            ppr->pRtpSocket, 0,0,0,0);
                        if (last == ppr->pRtpSocket->getSocketDescriptor())
                           last = OS_INVALID_SOCKET_DESCRIPTOR;
                        ppr->pRtpSocket = NULL;
                        if (NULL == ppr->pRtcpSocket) ppr->fwdTo = NULL;
                    }
                    numReady--;
                }
                if ((NULL != ppr->pRtcpSocket) &&
                  (FD_ISSET(ppr->pRtcpSocket->getSocketDescriptor(), fds))) {
                    stat = get1Msg(ppr->pRtcpSocket, ppr->fwdTo,
                       MpBufferMsg::AUD_RTCP_RECV, ostc);
                    if (OS_SUCCESS != stat) {
                        Zprintf(" *** NetInTask: removing RTCP#%ld pSkt=0x%p due"
                            " to read error.\n", ppr-pairs,
                            ppr->pRtcpSocket, 0,0,0,0);
                        if (last == ppr->pRtcpSocket->getSocketDescriptor())
                           last = OS_INVALID_SOCKET_DESCRIPTOR;
                        ppr->pRtcpSocket = NULL;
                        if (NULL == ppr->pRtpSocket) ppr->fwdTo = NULL;
                    }
                    numReady--;
                }
                ppr++;
            }
        }
        return 0;
}

/* possible args are: buffer pool to take from, max message size */

OsStatus startNetInTask()
{
        NetInTask *pTask;

        pTask = NetInTask::getNetInTask();
        return (NULL != pTask) ? OS_SUCCESS : OS_TASK_NOT_STARTED;
}

NetInTask* NetInTask::getNetInTask()
{
   // Lock to ensure that only one instance of the task is started
   sLock.acquire();

   // If the task object already exists, then use it, else create and start it
   if (NULL == spInstance)
   {
      spInstance = new NetInTask();
      UtlBoolean isStarted = spInstance->start();
      assert(isStarted);
   }

   sLock.release();

   return spInstance;
}

void NetInTask::shutdownSockets()
{
        getLockObj().acquire();

        if (mpWriteSocket)
        {
            mpWriteSocket->close();
            delete mpWriteSocket;
            mpWriteSocket = NULL;
        }

        /*if (mpReadSocket)
        {
            mpReadSocket->close();
            delete mpReadSocket;
            mpReadSocket =  NULL;
        }*/
        getLockObj().release();

}
// Default constructor (called only indirectly via getNetInTask())
NetInTask::NetInTask(int prio, int options, int stack)
:  OsTask("NetInTask", NULL, prio, options, stack),
   mpWriteSocket(NULL),
   mpReadSocket(NULL)
{
}

// Destructor
NetInTask::~NetInTask()
{
   spInstance = NULL;
}

// $$$ These messages need to contain OsSocket* instead of fds.

OsStatus shutdownNetInTask()
{
        NetInTask::getLockObj().acquire();

        netInTaskMsg msg;
        int wrote;
        NetInTask* pInst = NetInTask::getNetInTask();
        OsConnectionSocket* writeSocket;

        // pInst->getWriteFD();
        writeSocket = pInst->getWriteSocket();

        msg.pRtpSocket = (OsSocket*) -2;
        msg.pRtcpSocket = (OsSocket*) -1;
        msg.fwdTo = NULL;

        wrote = writeSocket->write((char *) &msg, NET_TASK_MAX_MSG_LEN);

        pInst->shutdownSockets();

        NetInTask* pTask = NetInTask::getNetInTask();
        pTask->requestShutdown();
        NetInTask::getLockObj().release();
        return ((NET_TASK_MAX_MSG_LEN == wrote) ? OS_SUCCESS : OS_BUSY);
}

// $$$ This is quite Unix-centric; on Win/NT these are handles...
/* neither fd can be < -1, at least one must be > -1, and fwdTo non-NULL */

OsStatus addNetInputSources(OsSocket* pRtpSocket, OsSocket* pRtcpSocket,
      MprFromNet* fwdTo, OsNotification* notify)
{
        netInTaskMsg msg;
        int wrote = 0;
        NetInTask* pInst = NetInTask::getNetInTask();
        OsConnectionSocket* writeSocket;

        // pInst->getWriteFD();
        writeSocket = pInst->getWriteSocket();

        if (NULL != fwdTo) {
            msg.pRtpSocket = pRtpSocket;
            msg.pRtcpSocket = pRtcpSocket;
            msg.fwdTo = fwdTo;
            msg.notify = notify;

            wrote = writeSocket->write((char *) &msg, NET_TASK_MAX_MSG_LEN);
            if (wrote != NET_TASK_MAX_MSG_LEN)
            {
                OsSysLog::add(FAC_MP, PRI_ERR,
                    "addNetInputSources - writeSocket error: 0x%p,%d wrote %d",
                writeSocket, writeSocket->getSocketDescriptor(), wrote);
            }
        }
        return ((NET_TASK_MAX_MSG_LEN == wrote) ? OS_SUCCESS : OS_BUSY);
}

OsStatus removeNetInputSources(MprFromNet* fwdTo, OsNotification* notify)
{
        netInTaskMsg msg;
        int wrote = NET_TASK_MAX_MSG_LEN;
        NetInTask* pInst = NetInTask::getNetInTask();
        OsConnectionSocket* writeSocket;

        // pInst->getWriteFD();
        writeSocket = pInst->getWriteSocket();

        if (NULL != fwdTo) {
            msg.pRtpSocket = NULL;
            msg.pRtcpSocket = NULL;
            msg.fwdTo = fwdTo;
            msg.notify = notify;
            wrote = writeSocket->write((char *) &msg, NET_TASK_MAX_MSG_LEN);
            if (wrote != NET_TASK_MAX_MSG_LEN)
            {
                OsSysLog::add(FAC_MP, PRI_ERR,
                    "removeNetInputSources - writeSocket error: 0x%p,%d wrote %d",
                    writeSocket, writeSocket->getSocketDescriptor(), wrote);
            }
        }

#ifdef WIN32 /* [ */
        // Reduce the delay, not sure this is still needed
      Sleep(100);
#endif /* WIN32 ] */

        return ((NET_TASK_MAX_MSG_LEN == wrote) ? OS_SUCCESS : OS_BUSY);
}

#define LOGEM
#undef LOGEM

/************************************************************************/

// return something random (32 bits)
UINT rand_timer32()
{
#ifdef _VXWORKS /* [ */
// On VxWorks, this is based on reading the 3.686400 MHz counter
        UINT x;
static  UINT last_timer = 0x12345678;

        x = *pOsTC;
        if (x == last_timer) {
                SEM_ID s;
                s = semBCreate(SEM_Q_FIFO, SEM_EMPTY);
                semTake(s, 0);
                semDelete(s);
                x = *pOsTC;
        }
        last_timer = x;
        /* Rotate left 8 */
        return (((x&0xFF)<<8) | ((x&0xFFFF00)<<16) | ((x&0xFF000000)>>24));
#endif /* _VXWORKS ] */

#if defined(_WIN32) || defined(__pingtel_on_posix__) /* [ */
// Otherwise, call rand() 3 times, using 12 or 8 bits from each call (15 max)
        static int firstTime = 1;
        unsigned int x, y, z;

        if (firstTime) {
            assert(RAND_MAX > 0xfff);
            srand((unsigned)time(NULL));
            firstTime = 0;
        }
        x = rand();
        y = rand();
        z = rand();
        return ((x&0xFFF) | ((y<<12)&0xFFF000) | ((z<<24)&0xFF000000));
#endif /* _WIN32 || __pingtel_on_posix__ ] */
}

struct rtcpSession {
        int dir;
        OsSocket* socket;
};

#define RTP_DIR_NEW 4

rtpHandle StartRtpSession(OsSocket* socket, int direction, char type)
{
        struct rtpSession *ret;
        USHORT rseq;

        rseq = 0xFFFF & rand_timer32();

        ret = (struct rtpSession *) malloc(sizeof(struct rtpSession));
        if (ret) {
                ret->vpxcc = ((2<<6) | (0<<5) | (0<<4) | 0);
                ret->mpt = ((0<<7) | (type & 0x7f));
                ret->seq = rseq;
                /* ret->timestamp = rand_timer32(); */
#ifdef INCLUDE_RTCP /* [ */
                ret->ssrc = 0;   // Changed by DMG.  SSRC now generated in MpFlowGraph
#else /* INCLUDE_RTCP ] [ */
                ret->ssrc = rand_timer32();
#endif /* INCLUDE_RTCP ] */
                ret->dir = direction | RTP_DIR_NEW;
                ret->socket = socket;
                ret->packets = 0;
                ret->octets = 0;
                ret->cycles = 0;
        }
        return ret;
}

rtcpHandle StartRtcpSession(int direction)
{
        struct rtcpSession *ret;
        USHORT rseq;

        rseq = 0xFFFF & rand_timer32();

        ret = (struct rtcpSession *) malloc(sizeof(struct rtcpSession));
        if (ret) {
                ret->dir = direction | RTP_DIR_NEW;
                ret->socket = NULL;
        }
        return ret;
}

OsStatus setRtpType(rtpHandle h, int type)
{
        h->mpt = ((0<<7) | (type & 0x7f));
        return OS_SUCCESS;
}

OsStatus setRtpSocket(rtpHandle h, OsSocket* socket)
{
        h->socket = socket;
        return OS_SUCCESS;
}

OsSocket* getRtpSocket(rtpHandle h)
{
        return h->socket;
}

OsStatus setRtcpSocket(rtcpHandle h, OsSocket* socket)
{
        h->socket = socket;
        return OS_SUCCESS;
}

OsSocket* getRtcpSocket(rtcpHandle h)
{
        return h->socket;
}

void FinishRtpSession(rtpHandle h)
{
        if (NULL != h) {
            h->socket = NULL;
            free(h);
        }
}

void FinishRtcpSession(rtcpHandle h)
{
        if (NULL != h) {
            h->socket = NULL;
            free(h);
        }
}

/* ============================ FUNCTIONS ================================= */
