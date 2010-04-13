//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////


#define LOGEM
#undef LOGEM

// SYSTEM INCLUDES

#include <assert.h>
#include <string.h>

#ifdef WIN32 /* [ */
#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#endif /* WIN32 ] */

#ifdef __pingtel_on_posix__ /* [ */
#include <stdlib.h>
#include <sys/time.h>
#endif /* __pingtel_on_posix__ ] */

// APPLICATION INCLUDES
#include "include/ConferenceEngineDatagramSocket.h"
#include "include/ConferenceEngineNetTask.h"
#include <os/OsDefs.h>
#include <os/OsTask.h>
#include <os/OsServerSocket.h>
#include <os/OsConnectionSocket.h>
#include <os/OsEvent.h>
#if defined(__pingtel_on_posix__)
#include <mp/MpMisc.h>
#endif

// EXTERNAL FUNCTIONS

// EXTERNAL VARIABLES

// CONSTANTS

#define NET_TASK_MAX_MSG_LEN sizeof(ConferenceEngineNetTaskMsg)
#define NET_TASK_MAX_FD_PAIRS 100

#define MAX_RTP_BYTES 1500

struct __ConferenceEngineNetTaskMsg
{
   OsProtectedEvent* notify;

   enum { ADD, REMOVE, SHUTDOWN } operation ;
   ConferenceEngineDatagramSocket* pSocket ;

};

typedef struct __ConferenceEngineNetTaskMsg ConferenceEngineNetTaskMsg, *ConferenceEngineNetTaskMsgPtr;

// STATIC VARIABLE INITIALIZATIONS

static  ConferenceEngineNetTaskMsg pairs[NET_TASK_MAX_FD_PAIRS];
static  int numPairs;

ConferenceEngineNetTask* ConferenceEngineNetTask::spInstance = 0;
OsRWMutex     ConferenceEngineNetTask::sLock(OsBSem::Q_PRIORITY);

const int ConferenceEngineNetTask::DEF_NET_IN_TASK_PRIORITY  = 100; // default task priority
const int ConferenceEngineNetTask::DEF_NET_IN_TASK_OPTIONS   = 0;   // default task options
#ifdef USING_NET_EQ /* [ */
const int ConferenceEngineNetTask::DEF_NET_IN_TASK_STACKSIZE = 40960;//default task stacksize
#else /* USING_NET_EQ ] [ */
const int ConferenceEngineNetTask::DEF_NET_IN_TASK_STACKSIZE = 4096;// default task stacksize
#endif /* USING_NET_EQ ] */

#define DEBUG
#undef  DEBUG


/************************************************************************/


OsConnectionSocket* ConferenceEngineNetTask::getWriteSocket()
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


OsConnectionSocket* ConferenceEngineNetTask::getReadSocket()
{
    return mpReadSocket;
}


OsStatus ConferenceEngineNetTask::addInputSource(ConferenceEngineDatagramSocket* pSocket)
{
    int wrote = 0;

    OsConnectionSocket* writeSocket = getWriteSocket() ;
    if (writeSocket)
    {
        ConferenceEngineNetTaskMsg msg;
        memset((void*)&msg, 0, sizeof(ConferenceEngineNetTaskMsg));
        msg.operation = ConferenceEngineNetTaskMsg::ADD ;
        msg.pSocket = pSocket ;

        wrote = writeSocket->write((char *) &msg, NET_TASK_MAX_MSG_LEN);
        if (wrote != NET_TASK_MAX_MSG_LEN)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                    "addNetInputSources - writeSocket error: 0x%08x,%d wrote %d",
                    (int)writeSocket, writeSocket->getSocketDescriptor(), wrote);
        }
    }
    else
    {
        assert(FALSE) ;
    }

    return ((NET_TASK_MAX_MSG_LEN == wrote) ? OS_SUCCESS : OS_BUSY);
}


OsStatus ConferenceEngineNetTask::removeInputSource(ConferenceEngineDatagramSocket* pSocket, OsProtectedEvent* pEvent)
{
    int wrote = 0;

    OsConnectionSocket* writeSocket = getWriteSocket() ;
    if (writeSocket)
    {
        ConferenceEngineNetTaskMsg msg;
        memset((void*)&msg, 0, sizeof(ConferenceEngineNetTaskMsg));
        msg.operation = ConferenceEngineNetTaskMsg::REMOVE ;
        msg.pSocket = pSocket ;
        msg.notify = pEvent;

        wrote = writeSocket->write((char *) &msg, NET_TASK_MAX_MSG_LEN);
        if (wrote != NET_TASK_MAX_MSG_LEN)
        {
            OsSysLog::add(FAC_MP, PRI_ERR,
                    "addNetInputSources - writeSocket error: 0x%08x,%d wrote %d",
                    (int)writeSocket, writeSocket->getSocketDescriptor(), wrote);
        }
    }
    else
    {
        assert(FALSE) ;
    }

    return ((NET_TASK_MAX_MSG_LEN == wrote) ? OS_SUCCESS : OS_BUSY);
}


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
        ConferenceEngineNetTaskMsgPtr ppr;

        if (XXisFdPoison(pipeFD)) {
            OsSysLog::add(FAC_MP, PRI_ERR, " *** ConferenceEngineNetTask: pipeFd socketDescriptor=%d busted!\n", pipeFD);
            return -1;
        }
        for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++) {
            if (ppr->pSocket && // not NULL socket and
                XXisFdPoison(ppr->pSocket->getSocketDescriptor())) {
                OsSysLog::add(FAC_MP, PRI_ERR, " *** ConferenceEngineNetTask: Removing fdRtp[%d], socket=0x%08x, socketDescriptor=%d\n", ppr-pairs,(int)ppr->pSocket, (int)ppr->pSocket->getSocketDescriptor());
                n++;
                ppr->pSocket = NULL;
            }
            ppr++;
        }
        return n;
}

int ConferenceEngineNetTask::processControlSocket(int last)
{
    ConferenceEngineNetTaskMsg    msg;
    ConferenceEngineNetTaskMsgPtr ppr;
    int i ;
    int newFd ;

    memset((void*)&msg, 0, sizeof(ConferenceEngineNetTaskMsg));
    if (NET_TASK_MAX_MSG_LEN != mpReadSocket->read((char *) &msg, NET_TASK_MAX_MSG_LEN))
    {
        OsSysLog::add(FAC_MP, PRI_ERR, "ConferenceEngineNetTask::run: Invalid request!") ;
    }
    else if (-2 == (int) msg.pSocket)
    {
        // Request to exit

        OsSysLog::add(FAC_MP, PRI_ERR, " *** ConferenceEngineNetTask: closing pipeFd (%d)\n",
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
            case ConferenceEngineNetTaskMsg::ADD:

                // Insert into the pairs list
                newFd  = (msg.pSocket) ? msg.pSocket->getSocketDescriptor() : -1;

                OsSysLog::add(FAC_MP, PRI_DEBUG, " *** ConferenceEngineNetTask: Adding new sockets (socket: %p,%d)\n",
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
                                    " *** ConferenceEngineNetTask: Using a dup descriptor (New:%p,%d, Old:%p,%d)\n",
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
                                " *** ConferenceEngineNetTask: Add socket Fds: (New=%p,%d)\n",
                                msg.pSocket, newFd);
                        break ;
                    }
                    ppr++;
                }

                // Update last
                last = max(last, newFd);

                break ;
            case ConferenceEngineNetTaskMsg::REMOVE:
                for (i=0, ppr=pairs; i<NET_TASK_MAX_FD_PAIRS; i++)
                {
                    if (msg.pSocket == ppr->pSocket)
                    {
                        OsSysLog::add(FAC_MP, PRI_DEBUG,
                                " *** ConferenceEngineNetTask: Remove socket Fds: (Old=%p,%d)\n",
                                ppr->pSocket, ppr->pSocket->getSocketDescriptor()) ;
                        ppr->pSocket = NULL ;
                    }
                    last = OS_INVALID_SOCKET_DESCRIPTOR ;
                    numPairs-- ;
                    ppr++;
                }
                msg.notify->signal(0);
                break ;
            case ConferenceEngineNetTaskMsg::SHUTDOWN:
                break ;
            default:
                break ;
        }
    }

    return last ;
}


int ConferenceEngineNetTask::run(void *pNotUsed)
{
    fd_set fdset;
    fd_set *fds;
    int     last;
    int     i;
    int     numReady;
    ConferenceEngineNetTaskMsgPtr ppr;
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
        printf(" *** ConferenceEngineNetTask: accept() failed!\n", 0,0,0,0,0,0);
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
            OsSysLog::add(FAC_MP, PRI_ERR, " *** ConferenceEngineNetTask: select returned %d, errno=%d=0x%X\n",
                numReady, errno, errno);
            i = XXfindPoisonFds(mpReadSocket->getSocketDescriptor());
            if (i < 0)
            {
                OsSysLog::add(FAC_MP, PRI_ERR, " *** ConferenceEngineNetTask: My comm socket failed! Quitting!\n");
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


ConferenceEngineNetTask* ConferenceEngineNetTask::getConferenceEngineNetTask()
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
       spInstance = new ConferenceEngineNetTask();
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

void ConferenceEngineNetTask::shutdownSockets()
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

// Default constructor (called only indirectly via getConferenceEngineNetTask())
ConferenceEngineNetTask::ConferenceEngineNetTask(int prio, int options, int stack)
:  OsTask("ConferenceEngineNetTask", NULL, prio, options, stack),
   mpWriteSocket(NULL),
   mpReadSocket(NULL),
   mCmdPort(-1)
{
}

// Destructor
ConferenceEngineNetTask::~ConferenceEngineNetTask()
{
   spInstance = NULL;
}

// $$$ These messages need to contain OsSocket* instead of fds.

OsStatus shutdownConferenceEngineNetTask()
{
        ConferenceEngineNetTask::getLockObj().acquireWrite();

        ConferenceEngineNetTaskMsg msg;
        memset((void*)&msg, 0, sizeof(ConferenceEngineNetTaskMsg));

        int wrote;
        ConferenceEngineNetTask* pInst = ConferenceEngineNetTask::getConferenceEngineNetTask();
        OsConnectionSocket* writeSocket;

        // pInst->getWriteFD();
        writeSocket = pInst->getWriteSocket();

        msg.operation = ConferenceEngineNetTaskMsg::SHUTDOWN ;

        wrote = writeSocket->write((char *) &msg, NET_TASK_MAX_MSG_LEN);

        ConferenceEngineNetTask::getLockObj().releaseWrite();
        pInst->shutdownSockets();

        ConferenceEngineNetTask* pTask = ConferenceEngineNetTask::getConferenceEngineNetTask();
        ConferenceEngineNetTask::getLockObj().acquireWrite();
        pTask->requestShutdown();
        ConferenceEngineNetTask::getLockObj().releaseWrite();
        return ((NET_TASK_MAX_MSG_LEN == wrote) ? OS_SUCCESS : OS_BUSY);
}


#define LOGEM
#undef LOGEM

/************************************************************************/

/* ============================ FUNCTIONS ================================= */
