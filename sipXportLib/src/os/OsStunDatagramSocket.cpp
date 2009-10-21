//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>
#include <stdio.h>
#ifndef _WIN32
#include <netinet/in.h>
#endif

// APPLICATION INCLUDES
#include "os/OsStunDatagramSocket.h"
#include "os/OsStunAgentTask.h"
#include "os/OsStunQueryAgent.h"
#include "os/OsLock.h"
#include "os/OsSysLog.h"
#include "os/OsEvent.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS

// FORWARD DECLARATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsStunDatagramSocket::OsStunDatagramSocket(int remoteHostPortNum,
                                           const char* remoteHost,
                                           int localHostPortNum,
                                           const char* localHost,
                                           bool bEnableStun,
                                           const char* szStunServer,
                                           int iRefreshPeriodInSec,
                                           int iStunOptions,
                                           OsNotification *pNotification)
        : OsDatagramSocket(remoteHostPortNum, remoteHost,
                           localHostPortNum, localHost)
        , mKeepAlivePeriod(0)
        , mCurrentKeepAlivePeriod(0)
        , mStunServer(szStunServer ? szStunServer : "") // szStunServer can be NULL
        , mStunOptions(iStunOptions)
        , mStunPort(PORT_NONE)
        , mpTimer(new OsTimer(pStunAgent->getMessageQueue(), this))
        , mbEnabled(bEnableStun)
        , mStunRefreshErrors(0)
        , pStunAgent(OsStunAgentTask::getInstance(this))
        , mbTransparentStunRead(FALSE)
        , mDestAddress(mRemoteIpAddress)
        , miDestPort(remoteHostPort)
        , mcDestPriority(0)
        , mpNotification(pNotification)
{

    // If enabled, kick off first stun request
    if (mbEnabled)
    {
        refreshStunBinding(true) ;
    }

    // If valid refresh period, set timer
    if (iRefreshPeriodInSec > 0)
    {
        mKeepAlivePeriod = iRefreshPeriodInSec ;
        setKeepAlivePeriod(iRefreshPeriodInSec) ;
    }
}


// Destructor
OsStunDatagramSocket::~OsStunDatagramSocket()
{
    enableStun(FALSE) ;

    mpTimer->stop();
    // Invoking synchronize will wait until any active timer/refresh activies
    // are completed.
    pStunAgent->synchronize() ;
    delete mpTimer ;

    pStunAgent->removeSocket(this) ;
 }

/* ============================ MANIPULATORS ============================== */


int OsStunDatagramSocket::read(char* buffer, int bufferLength)
{
    bool bStunPacket ;
    int iRC ;
    UtlString receivedIp ;
    int iReceivedPort ;

    do
    {
        iRC = OsSocket::read(buffer, bufferLength, &receivedIp, &iReceivedPort) ;

        // Look for stun packet
        bStunPacket = FALSE ;
        if ((iRC > 0) && StunMessage::isStunMessage(buffer, iRC))
        {
            bStunPacket = TRUE ;

            // Make copy and queue it.
            char* szCopy = (char*) malloc(iRC) ;
            if (szCopy)
            {
                memcpy(szCopy, buffer, iRC) ;
                StunMsg msg(szCopy, iRC, this, receivedIp, iReceivedPort);
                pStunAgent->postMessage(msg) ;
            }

            // If not configured for transparent reads, exit with
            // zero bytes read
            if (!mbTransparentStunRead)
            {
                iRC = 0 ;
                bStunPacket = FALSE ;
            }
        }
    } while ((iRC >= 0) && bStunPacket) ;

    return iRC ;
}

int OsStunDatagramSocket::read(char* buffer, int bufferLength,
       UtlString* ipAddress, int* port)
{
    bool bStunPacket ;
    int iRC ;
    UtlString receivedIp ;
    int iReceivedPort ;

    do
    {
        iRC = OsSocket::read(buffer, bufferLength, &receivedIp, &iReceivedPort) ;

        // Look for stun packet
        bStunPacket = FALSE ;
        if ((iRC > 0) && StunMessage::isStunMessage(buffer, iRC))
        {
            bStunPacket = TRUE ;

            // Make copy and queue it.
            char* szCopy = (char*) malloc(iRC) ;
            if (szCopy)
            {
                memcpy(szCopy, buffer, iRC) ;
                StunMsg msg(szCopy, iRC, this, receivedIp, iReceivedPort);
                pStunAgent->postMessage(msg) ;
            }

            // If not configured for transparent reads, exit with
            // zero bytes read
            if (!mbTransparentStunRead)
            {
                iRC = 0 ;
                bStunPacket = FALSE ;
            }
        }

    } while ((iRC >= 0) && bStunPacket) ;

    if (ipAddress)
    {
        *ipAddress = receivedIp ;
    }

    if (port)
    {
        *port = iReceivedPort ;
    }

    return iRC ;
}

int OsStunDatagramSocket::read(char* buffer, int bufferLength,
       struct in_addr* ipAddress, int* port)
{
    bool bStunPacket ;
    int iRC ;
    struct in_addr fromSockAddress;
    int iReceivedPort ;

    do
    {
        iRC = OsSocket::read(buffer, bufferLength, &fromSockAddress, &iReceivedPort) ;

        // Look for stun packet
        bStunPacket = FALSE ;
        if ((iRC > 0) && StunMessage::isStunMessage(buffer, iRC))
        {
            bStunPacket = TRUE ;

            // Make copy and queue it.
            char* szCopy = (char*) malloc(iRC) ;
            if (szCopy)
            {
                UtlString receivedIp ;
                inet_ntoa_pt(fromSockAddress, receivedIp);

                memcpy(szCopy, buffer, iRC) ;
                StunMsg msg(szCopy, iRC, this, receivedIp, iReceivedPort) ;
                pStunAgent->postMessage(msg) ;
            }

            // If not configured for transparent reads, exit with
            // zero bytes read
            if (!mbTransparentStunRead)
            {
                iRC = 0 ;
                bStunPacket = FALSE ;
            }
        }

    } while ((iRC >= 0) && bStunPacket) ;

    if (ipAddress != NULL)
    {
        memcpy(ipAddress, &fromSockAddress, sizeof(struct in_addr)) ;
    }

    if (port != NULL)
    {
        *port = iReceivedPort ;
    }

    return iRC ;
}

int OsStunDatagramSocket::read(char* buffer, int bufferLength, long waitMilliseconds)
{
    assert(FALSE) ;
    return -1 ;
/*
    bool bStunPacket = FALSE ;
    int iRC ;
    UtlString receivedIp ;
    int iReceivedPort ;

    do
    {
        iRC = OsSocket::read(buffer, bufferLength, waitMilliseconds) ;

        // Look for stun packet
        bStunPacket = FALSE ;
        if ((iRC > 0) && StunMessage::isStunMessage(buffer, iRC))
        {
            bStunPacket = TRUE ;

            // Make copy and queue it.
            char* szCopy = (char*) malloc(iRC) ;
            if (szCopy)
            {
                memcpy(szCopy, buffer, iRC) ;
                StunMsg msg(szCopy, iRC, this);
                pStunAgent->postMessage(msg) ;
            }

            // If not configured for transparent reads, exit with
            // zero bytes read
            if (!mbTransparentStunRead)
            {
                iRC = 0 ;
                bStunPacket = FALSE ;
            }
        }

    } while ((iRC >= 0) && bStunPacket) ;


    return iRC ;
    */
}


void OsStunDatagramSocket::setKeepAlivePeriod(int secs)
{
    mCurrentKeepAlivePeriod = secs ;
    mpTimer->stop() ;

    assert(secs >= 0) ;
    if ((mbEnabled) && secs > 0)
    {
        assert(mpTimer != NULL) ;
        if (mpTimer)
        {
            mpTimer->periodicEvery(OsTime(secs, 0), OsTime(secs, 0)) ;
        }
    }
}


void OsStunDatagramSocket::setStunServer(const char* szHostname)
{
    mStunServer = szHostname ;
}

void OsStunDatagramSocket::setStunOptions(int stunOptions)
{
    mStunOptions = stunOptions ;
}


void OsStunDatagramSocket::enableStun(bool bEnable)
{
    if (mbEnabled != bEnable)
    {
        mbEnabled = bEnable ;
        if (mbEnabled)
        {
            refreshStunBinding(FALSE) ;

            // Use the current keep alive period -- callign refreshStunBinding may
            // report an error and speed up the refreshes.
            setKeepAlivePeriod(mCurrentKeepAlivePeriod) ;
        }
        else
        {
            // Calling setKeepAlivePeriod will disable the timer when mbEnabled
            // is false
            setKeepAlivePeriod(mKeepAlivePeriod) ;

            // Verify that all timer activity is stopped
            pStunAgent->synchronize() ;

            // Clear the STUN values
            mStunAddress.remove(0) ;
            mStunPort = PORT_NONE ;
        }
    }
}


void OsStunDatagramSocket::enableTransparentStunReads(bool bEnable)
{
    mbTransparentStunRead = bEnable ;
}


void OsStunDatagramSocket::refreshStunBinding(UtlBoolean bFromReadSocket)
{
    OsTime timeout(0, STUN_TIMEOUT_RESPONSE_MS * OsTime::USECS_PER_MSEC) ;
    bool bSuccess = FALSE ;

    if (bFromReadSocket)
    {
        OsStunQueryAgent agent;
        if (agent.setServer(mStunServer))
        {
            // We must touch the socket and look for the next stun packet.
            bSuccess = agent.getMappedAddress(this, mStunAddress, mStunPort, mStunOptions, timeout) ;
        }

        // Report status
        if (bSuccess)
        {
            markStunSuccess() ;
        }
        else
        {
            markStunFailure() ;
        }
    }
    else
    {
        pStunAgent->sendStunDiscoveryRequest(this, mStunServer, STUN_PORT, mStunOptions) ;
    }
}


/* ============================ ACCESSORS ================================= */

// Return the external IP address for this socket.
UtlBoolean OsStunDatagramSocket::getExternalIp(UtlString* ip, int* port)
{
    UtlBoolean bSuccess = false ;

    if (mStunAddress.length() && mbEnabled)
    {
        if (ip)
        {
            *ip = mStunAddress ;
        }

        if (port)
        {
            *port = mStunPort ;
        }

        // Success if we were able to set either the ip or port
        if (ip || port)
        {
            bSuccess = true ;
        }
    }

    return bSuccess ;
}

/*
int OsStunDatagramSocket::getStunPacket(char* buffer, int bufferLength, const OsTime& rTimeout)
{
    int iRead = 0 ;
    OsMsg* msg;

    assert(buffer != NULL) ;
    assert(bufferLength > 0) ;

    if (buffer && (bufferLength > 0))
    {
        if (mStunMsgQ.receive(msg, rTimeout) == OS_SUCCESS)
        {
            StunMsg* pStunMsg = (StunMsg*) msg ;
            assert(pStunMsg != NULL) ;
            if (pStunMsg)
            {
                char* pStunBuffer = pStunMsg->getBuffer() ;
                int iStunLength = pStunMsg->getLength() ;

                assert(pStunBuffer) ;
                assert(iStunLength > 0) ;
                if (pStunBuffer && iStunLength > 0)
                {
                    if (iStunLength > bufferLength)
                    {
                        iStunLength = bufferLength ;
                    }

                    memcpy(buffer, pStunBuffer, iStunLength) ;
                    StunMsgQFlushHookPtr(*pStunMsg) ;
                    pStunMsg->releaseMsg() ;

                    iRead = iStunLength ;
                }
            }
        }
    }

    return iRead ;
}
*/

int OsStunDatagramSocket::readStunPacket(char* buffer, int bufferLength, const OsTime& rTimeout)
{
    bool bStunPacket = FALSE;
    int iRC = 0 ;

    assert(buffer != NULL) ;
    assert(bufferLength > 0) ;

    if (buffer && (bufferLength > 0))
    {
        OsTime abortAfter ;
        OsDateTime::getCurTime(abortAfter) ;
        abortAfter += rTimeout ;

        do
        {
            if (isReadyToRead(rTimeout.cvtToMsecs()))
            {
                iRC = OsDatagramSocket::read(buffer, bufferLength) ;

                // Look for stun packet
                bStunPacket = FALSE ;
                if ((iRC > 0) && StunMessage::isStunMessage(buffer, iRC))
                {
                    bStunPacket = TRUE ;
                }
                else if (!rTimeout.isInfinite())
                {
                    if (rTimeout.isNoWait())
                    {
                        iRC = -1 ; // Force kickout
                    }
                    else
                    {
                        OsTime now ;
                        OsDateTime::getCurTime(now) ;
                        if (now > abortAfter)
                        {
                            iRC = -1 ;
                        }
                    }
                }
            }
        } while ((iRC >= 0) && !bStunPacket) ;
    }

    return iRC ;
}


void OsStunDatagramSocket::addAlternateDestination(const char* szAddress, int iPort, unsigned char cPriority)
{
    if (pStunAgent)
    {
        pStunAgent->sendStunConnectivityRequest(this, szAddress, iPort, cPriority) ;
    }
}


void OsStunDatagramSocket::setNotifier(OsNotification* pNotification)
{
    mpNotification = pNotification ;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

void OsStunDatagramSocket::setStunAddress(const UtlString& address,
                                          const int iPort)
{
    mStunAddress = address ;
    mStunPort = iPort ;
}


void OsStunDatagramSocket::markStunFailure()
{
    // Speed up refreshes on first error case
    if ((mCurrentKeepAlivePeriod != STUN_FAILURE_REFRESH_PERIOD_SECS) &&
            mStunRefreshErrors == 0)
    {
        setKeepAlivePeriod(STUN_FAILURE_REFRESH_PERIOD_SECS) ;
    }

    mStunRefreshErrors++ ;

    if ((mStunRefreshErrors == STUN_INITIAL_REFRESH_REPORT_THRESHOLD) ||
            (mStunRefreshErrors % STUN_REFRESH_REPORT_THRESHOLD) == 0)
    {
        OsSysLog::add(FAC_NET, PRI_WARNING,
                "STUN failed to obtain binding from %s (attempt=%d)\n",
                mStunServer.data(), mStunRefreshErrors) ;

        // Signal external identities interested in the STUN outcome.
        if (mpNotification)
        {
            mpNotification->signal(0) ;
            mpNotification = NULL ;
        }

        // At this point it looks like we have some problems, slow down
        // and use the normal refresh period.
        if (mCurrentKeepAlivePeriod != mKeepAlivePeriod)
        {
            setKeepAlivePeriod(mKeepAlivePeriod) ;
        }

    }

    if (mStunRefreshErrors >= STUN_ABORT_THRESHOLD)
    {
        // Shutdown if we never received a valid address
        if (mStunServer.length() > 0)
        {
            OsSysLog::add(FAC_NET, PRI_ERR,
                "STUN Aborted; Failed to obtain stun binding from %s (attempt=%d)\n",
                mStunServer.data(), mStunRefreshErrors) ;
            enableStun(FALSE) ;
        }
    }
}

void OsStunDatagramSocket::markStunSuccess()
{
    mStunRefreshErrors = 0 ;

    // Reset keep alive to normal value if we had previously accelerated it
    // to due error
    if (mCurrentKeepAlivePeriod != mKeepAlivePeriod)
    {
        setKeepAlivePeriod(mKeepAlivePeriod);
    }

    // Signal external identities interested in the STUN outcome.
    if (mpNotification)
    {
        char szAdapterName[256];
        memset((void*)szAdapterName, 0, sizeof(szAdapterName));

        getContactAdapterName(szAdapterName, mLocalIp.data());

        ContactAddress* pContact = new ContactAddress();

        strcpy(pContact->cIpAddress, mStunAddress);
        strcpy(pContact->cInterface, szAdapterName);
        pContact->eContactType = ContactAddress::NAT_MAPPED;
        pContact->iPort = mStunPort;

        mpNotification->signal((intptr_t) pContact);
        mpNotification = NULL;
    }
}


void OsStunDatagramSocket::setDestinationAddress(const UtlString& address,
                                                 int iPort,
                                                 unsigned char cPriority)
{
    if ((address.compareTo(mDestAddress, UtlString::ignoreCase) != 0) &&
            (iPort != cPriority))
    {
        if (cPriority > mcDestPriority)
        {
            mcDestPriority = cPriority ;
            mDestAddress = address ;
            miDestPort = iPort ;

            // Change the destination address
            doConnect(miDestPort, mDestAddress, FALSE) ;

            // ::TODO:: bob: I suspect that we need to lock reads/writes/close
            // during an unlock.

            // ::TODO:: Socket may have changed -- need to reset with NetInTask?
        }
    }
    else if (cPriority > mcDestPriority)
    {
        // No change in host/port, just store updated priority.
        mcDestPriority = cPriority ;
    }
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

/* ///////////////////////// HELPER CLASSES /////////////////////////////// */

StunMsg::StunMsg(char*                 szBuffer,
                 int                   nLength,
                 OsStunDatagramSocket* pSocket,
                 UtlString             receivedIp,
                 int                   iReceivedPort)
    : OsMsg(STUN_MSG_TYPE, 0)
{
    mBuffer = szBuffer ;    // Shallow copy
    mLength = nLength ;
    mpSocket = pSocket ;
    mReceivedIp = receivedIp ;
    miReceivedPort = iReceivedPort ;
}


StunMsg::StunMsg(const StunMsg& rStunMsg)
    : OsMsg(STUN_MSG_TYPE, 0)
{
    mBuffer = rStunMsg.mBuffer ;
    mLength  = rStunMsg.mLength ;
    mpSocket = rStunMsg.mpSocket ;
    mReceivedIp = rStunMsg.mReceivedIp ;
    miReceivedPort = rStunMsg.miReceivedPort ;
}


OsMsg* StunMsg::createCopy(void) const
{
    return new StunMsg(*this);
}


StunMsg::~StunMsg()
{

}


StunMsg& StunMsg::operator=(const StunMsg& rhs)
{
    if (this != &rhs)            // handle the assignment to self case
    {
        mBuffer = rhs.mBuffer ;
        mLength = rhs.mLength ;
        mpSocket = rhs.mpSocket ;
        mReceivedIp = rhs.mReceivedIp ;
        miReceivedPort = rhs.miReceivedPort ;
    }

    return *this ;
}


char* StunMsg::getBuffer() const
{
    return mBuffer ;
}


int StunMsg::getLength() const
{
    return mLength ;
}


OsStunDatagramSocket* StunMsg::getSocket() const
{
    return mpSocket ;
}


UtlString StunMsg::getReceivedIp() const
{
    return mReceivedIp ;
}

int StunMsg::getReceivedPort() const
{
    return miReceivedPort ;
}
