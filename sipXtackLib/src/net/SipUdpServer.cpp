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

// APPLICATION INCLUDES
#include <net/SipUdpServer.h>
#include <net/SipUserAgent.h>
#include <net/Url.h>
#include <os/OsDateTime.h>
#include <os/OsStunDatagramSocket.h>
#include <os/HostAdapterAddress.h>
#include <utl/UtlHashMapIterator.h>

#if defined(_VXWORKS)
#   include <socket.h>
#   include <resolvLib.h>
//#   include <netinet/ip.h>
#elif defined(__pingtel_on_posix__)
//#   include <netinet/tcp.h>
#   include <sys/types.h>
#   include <sys/socket.h>
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
//#define TEST_PRINT
//#define LOG_SIZE
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipUdpServer::SipUdpServer(int port,
                           SipUserAgent* userAgent,
                           const char* natPingUrl,
                           int natPingFrequencySeconds,
                           const char* natPingMethod,
                           int udpReadBufferSize,
                           UtlBoolean bUseNextAvailablePort,
                           const char* szBoundIp) :
   SipProtocolServerBase(userAgent, "UDP", "SipUdpServer-%d"),
   mStunRefreshSecs(28), 
   mStunOptions(0)  
{
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipUdpServer::_ port = %d, bUseNextAvailablePort = %d, szBoundIp = '%s'",
                  port, bUseNextAvailablePort, szBoundIp);

    if (szBoundIp && 0 != strcmp(szBoundIp, "0.0.0.0"))
    {
        mDefaultIp = szBoundIp;
        int serverSocketPort = port;
        createServerSocket(szBoundIp, serverSocketPort, bUseNextAvailablePort, udpReadBufferSize);
    }
    else
    {
        int numAddresses = 0;
        const HostAdapterAddress* adapterAddresses[MAX_IP_ADDRESSES];
        getAllLocalHostIps(adapterAddresses, numAddresses);

        for (int i = 0; i < numAddresses; i++)
        {
            int serverSocketPort = port;
            
            createServerSocket(adapterAddresses[i]->mAddress.data(),
                               serverSocketPort,
                               bUseNextAvailablePort,
                               udpReadBufferSize);
            if (0 == i)
            {
                // use the first IP address in the array
                // for the 'default ip'
                mDefaultIp = adapterAddresses[i]->mAddress.data();
            }
            delete adapterAddresses[i];   
        }
    }

    if(natPingUrl && *natPingUrl)
        mNatPingUrl = natPingUrl;
    if(natPingMethod && *natPingMethod)
        mNatPingMethod = natPingMethod;
    else
        mNatPingMethod = "PING";

    mNatPingFrequencySeconds = natPingFrequencySeconds;
}

// Copy constructor
SipUdpServer::SipUdpServer(const SipUdpServer& rSipUdpServer) :
        SipProtocolServerBase(NULL, "UDP", "SipUdpServer-%d")
{
}

// Destructor
SipUdpServer::~SipUdpServer()
{
    waitUntilShutDown();
    
    SipClient* pServer = NULL;
    UtlHashMapIterator iterator(mServers);
    UtlVoidPtr* pServerContainer = NULL;
    UtlString* pKey = NULL;
    
    while ((pKey = (UtlString*)iterator()))
    {
        pServerContainer = (UtlVoidPtr*)iterator.value();
        if (pServerContainer)
        {
            pServer = (SipClient*)pServerContainer->getValue();
            pServer->requestShutdown();
            delete pServer;
        }
    }
    mServers.destroyAll();

    mServerPortMap.destroyAll();    

    mServerSocketMap.destroyAll();
    
}

/* ============================ MANIPULATORS ============================== */

OsStatus SipUdpServer::createServerSocket(const char* szBoundIp,
                                          int& port,
                                          const UtlBoolean& bUseNextAvailablePort, 
                                          int udpReadBufferSize)
{
    OsStatus rc = OS_FAILED;
    OsStunDatagramSocket* pSocket =
      new OsStunDatagramSocket(0, NULL, port, szBoundIp, FALSE);
   
    if (pSocket)
    {
        // If the socket is busy or unbindable and the user requested using the
        // next available port, try the next SIP_MAX_PORT_RANGE ports.
        if (bUseNextAvailablePort & portIsValid(port) && pSocket && !pSocket->isOk())
        {
            for (int i=1; i<=SIP_MAX_PORT_RANGE; i++)
            {
                delete pSocket ;
                pSocket = new OsStunDatagramSocket(0, NULL, port+i, szBoundIp, FALSE);
                if (pSocket->isOk())
                {
                    break ;
                }
            }
        }
    }
    
    if (pSocket)
    {     
        port = pSocket->getLocalHostPort();
        CONTACT_ADDRESS contact;
        strcpy(contact.cIpAddress, szBoundIp);
        contact.iPort = port;
        contact.eContactType = LOCAL;
        char szAdapterName[16];
        memset((void*)szAdapterName, 0, sizeof(szAdapterName)); // null out the string
        
        getContactAdapterName(szAdapterName, contact.cIpAddress);

        strcpy(contact.cInterface, szAdapterName);
        mSipUserAgent->addContactAddress(contact);
   
        // add address and port to the maps
        mServerSocketMap.insertKeyAndValue(new UtlString(szBoundIp),
                                            new UtlVoidPtr((void*)pSocket));
        port = pSocket->getLocalHostPort() ;
        mServerPortMap.insertKeyAndValue(new UtlString(szBoundIp), new UtlInt(port));


        int sockbufsize = 0;
        int size = sizeof(int);
            getsockopt(pSocket->getSocketDescriptor(),
                    SOL_SOCKET,
                    SO_RCVBUF,
                    (char*)&sockbufsize,
        #if defined(__pingtel_on_posix__)
                    (socklen_t*) // caste
        #endif
                    &size);
        #ifdef LOG_SIZE
        OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipUdpServer::SipUdpServer UDP buffer size: %d size: %d\n",
                    sockbufsize, size);
        #endif /* LOG_SIZE */

        if(udpReadBufferSize > 0)
        {
            setsockopt(pSocket->getSocketDescriptor(),
            SOL_SOCKET,
            SO_RCVBUF,
            (char*)&udpReadBufferSize,
            sizeof(int));

            getsockopt(pSocket->getSocketDescriptor(),
                SOL_SOCKET,
                SO_RCVBUF,
                (char*)&sockbufsize,
        #if defined(__pingtel_on_posix__)
                (socklen_t*) // caste
        #endif
                &size);
        #ifdef LOG_SIZE
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipUdpServer::SipUdpServer reset UDP buffer size: %d size: %d\n",
                        sockbufsize, size);
        #endif /* LOG_SIZE */
        }
    }
    return rc;
}

int SipUdpServer::run(void* runArg)
{
    int cseq = 1;
    if(mSipUserAgent)
    {
        UtlString contact;
        mSipUserAgent->getContactUri(&contact);

        // Add a tag to the contact and build the from field
        UtlString from(contact);
        int tagRand1 = rand();
        int tagRand2 = rand();
        char fromTag[80];
        sprintf(fromTag, ";tag=%d%d", tagRand1, tagRand2);
        from.append(fromTag);

        UtlString rawAddress;
        int port;
        Url pingUrl(mNatPingUrl);

        // Create a cannonized version of the ping URL in case
        // it does not specify "sip:", etc.
        UtlString cannonizedPingUrl = pingUrl.toString();

        // Get the address and port in the png URL so that
        // we can look up the DNS stuff if needed
        port = pingUrl.getHostPort();
        pingUrl.getHostAddress(rawAddress);

        // Resolve the raw address from a DNS SRV, A record
        // to an IP address
        server_t* dnsSrvRecords =
            SipSrvLookup::servers(rawAddress.data(),
                                  "sip",
                                  OsSocket::UDP,
                                  port);

        // Do a DNS SRV or A record lookup
        // If we started with an IP address, we will still get an IP
        // address in the result
        UtlString address;
        if(dnsSrvRecords[0].isValidServerT())
        {
            // Get the highest priority address and port from the
            // list with randomization of those according to the
            // weights.
            // Note: we are not doing any failover here as that is
            // a little tricky with the NAT stuff.  We cannot change
            // addresses with every transaction as we may get different
            // ports and addresses every time we send a ping.  For now
            // we do one DNS SRV lookup at the begining of time and
            // stick to that result.
            dnsSrvRecords[0].getIpAddressFromServerT(address);
            port = dnsSrvRecords[0].getPortFromServerT();

            // If the ping URL or DNS SRV did not specify a port
            // bind it to the default port.
            if (!portIsValid(port))
            {
               port = SIP_PORT;
            }
        }

        // Did not get a valid response from the DNS lookup
        else
        {
            // Configured with a bad DNS name that did not resolve.
            // Or the DNS server did not respond.
            if(!rawAddress.isNull())
            {
                OsSysLog::add(FAC_SIP, PRI_INFO,
                    "SipUdpServer::run DNS lookup failed for ping host: %s in URI: %s",
                    rawAddress.data(), mNatPingUrl.data());
            }
            // Else no ping address, this means we are not supposed to
            // do a ping
        }
        // Free the list of server addresses.
        delete[] dnsSrvRecords;

        // Get the address to be used in the callId scoping
        int dummyPort;
        UtlString callId;
        
        if (mSipUserAgent)
        {
            mSipUserAgent->getViaInfo(OsSocket::UDP, callId, dummyPort);
        }

        // Make up a call Id
        long epochTime = OsDateTime::getSecsSinceEpoch();
        int randNum = rand();
        char callIdPrefix[80];
        sprintf(callIdPrefix, "%ld%d-ping@", epochTime, randNum);
        callId.insert(0,callIdPrefix);

        while(mNatPingFrequencySeconds > 0 &&
            !mNatPingUrl.isNull() &&
            !mNatPingMethod.isNull() &&
            !address.isNull())
        {
            // Send a no-op SIP message to the
            // server to keep a port open through a NAT
            // based firewall
            SipMessage pingMessage;
            pingMessage.setRequestData(mNatPingMethod, cannonizedPingUrl.data(),
                from.data(), mNatPingUrl.data(), callId, cseq, contact.data());

            // Get the UDP via info from the SipUserAgent
            UtlString viaAddress;
            int viaPort;
            
            if (mSipUserAgent)
            {
                mSipUserAgent->getViaInfo(OsSocket::UDP, viaAddress, viaPort);
            }
            pingMessage.addVia(viaAddress.data(), viaPort, SIP_TRANSPORT_UDP);

            // Mark the via so the receiver knows we support and want the
            // received port to be set
            pingMessage.setLastViaTag("", "rport");
#           ifdef TEST_PRINT            
            osPrintf("Sending ping to %s %d, From: %s\n",
                address.data(), port, contact.data());
#           endif
            
            // Send from the same UDP port that we receive from
            if (mSipUserAgent)
            {
                mSipUserAgent->sendSymmetricUdp(pingMessage, address.data(), port);
            }

            cseq++;

            // Wait until it is time to send another ping
            delay(mNatPingFrequencySeconds * 1000);
        }
    }

    return(mNatPingFrequencySeconds);
}


void SipUdpServer::enableStun(const char* szStunServer,
                              const char* szLocalIp, 
                              int refreshPeriodInSecs, 
                              int stunOptions,
                              OsNotification* pNotification) 
{
    // Store settings
    mStunOptions = stunOptions ;
    mStunRefreshSecs = refreshPeriodInSecs ;   
    if (szStunServer)
    {
        mStunServer = szStunServer ;
    }
    else
    {
        mStunServer.remove(0) ;
    }
    
    UtlHashMapIterator iterator(mServerSocketMap);
    UtlString* pKey = NULL;

    char szIpToStun[256];
    memset((void*)szIpToStun, 0, sizeof(szIpToStun));
    
    if (szLocalIp)
    {
        strcpy(szIpToStun, szLocalIp);
    }
    bool bStunAll = false;
    if (0 == strcmp(szIpToStun, "") || 0 == strcmp(szIpToStun, "0.0.0.0"))
    {
        bStunAll = true;
        // if no ip specified, start on the first one
        pKey = (UtlString*) iterator();
        if (pKey)
        {
            strcpy(szIpToStun, pKey->data());
        }
    }
    
    while (0 != strcmp(szIpToStun, ""))
    {
        UtlVoidPtr* pSocketContainer;
        UtlString key(szIpToStun);
        
        pSocketContainer = (UtlVoidPtr*)this->mServerSocketMap.findValue(&key);
        OsStunDatagramSocket* pSocket = NULL;
        
        if (pSocketContainer)
        {
            pSocket = (OsStunDatagramSocket*)pSocketContainer->getValue();
        }                                                                  
        if (pSocket)
        {
            pSocket->enableStun(false) ;
            
            // Update server client
            if (pSocket && mStunServer.length()) 
            {
                pSocket->setStunServer(mStunServer) ;
                pSocket->setKeepAlivePeriod(refreshPeriodInSecs) ;
                pSocket->setNotifier(pNotification) ;
                pSocket->setStunOptions(mStunOptions) ;
                pSocket->enableStun(true) ;
            }  
        }
        if (bStunAll)
        {
            // get the next address to stun
            pKey = (UtlString*) iterator();
            if (pKey)
            {
                strcpy(szIpToStun, pKey->data());
            }
            else
            {
                strcpy(szIpToStun, "");
            }
        }
        else
        {
            break;
        }
    } // end while  
}

void SipUdpServer::shutdownListener()
{
    SipClient* pServer = NULL;
    UtlHashMapIterator iterator(mServers);
    UtlVoidPtr* pServerContainer = NULL;
    UtlString* pKey = NULL;
    
    while ((pKey = (UtlString*)iterator()))
    {
        pServerContainer = (UtlVoidPtr*) iterator.value();
        pServer = (SipClient*)pServerContainer->getValue();
        if (pServer)
        {
            pServer->requestShutdown();
        }
    }
}


UtlBoolean SipUdpServer::sendTo(const SipMessage& message,
                               const char* address,
                               int port,
                               const char* szLocalSipIp)
{
    UtlBoolean sendOk;
    UtlVoidPtr* pServerContainer = NULL;
    SipClient* pServer = NULL;
    
    if (szLocalSipIp)
    {
        UtlString localKey(szLocalSipIp);
        pServerContainer = (UtlVoidPtr*)this->mServers.findValue(&localKey);
        if (pServerContainer)
        {
            pServer = (SipClient*) pServerContainer->getValue();
        }
    }
    else
    {
        // no local sip IP specified, so, use the default one
        UtlString defaultKey(mDefaultIp);
       
        pServerContainer = (UtlVoidPtr*) mServers.findValue(&defaultKey);
        if (pServerContainer)
        {
            pServer = (SipClient*) pServerContainer->getValue();
        }
    }
    
    if (pServer)
    {
        sendOk = pServer->sendTo(message, address, port);
    }
    else
    {
        sendOk = false;
    }
    return(sendOk);
}


OsSocket* SipUdpServer::buildClientSocket(int hostPort, const char* hostAddress, const char* localIp)
{
    if (mSipUserAgent && mSipUserAgent->getUseRport())
    {
        UtlVoidPtr* pSocketContainer = NULL;
        OsStunDatagramSocket* pSocket = NULL;

        assert(localIp != NULL);
        UtlString localKey(localIp);
        
        pSocketContainer = (UtlVoidPtr*)mServerSocketMap.findValue(&localKey);
        assert(pSocketContainer);
        
        pSocket = (OsStunDatagramSocket*)pSocketContainer->getValue();
        assert(pSocket);
        
        return pSocket ;
    }
    else
    {
        return(new OsStunDatagramSocket(hostPort, hostAddress, 0, localIp, 
            (mStunServer.length() != 0), mStunServer.data(), mStunRefreshSecs, mStunOptions));
    }
}


// Assignment operator
SipUdpServer&
SipUdpServer::operator=(const SipUdpServer& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/* ============================ ACCESSORS ================================= */

void SipUdpServer::printStatus()
{
    SipClient* pServer = NULL;
    UtlHashMapIterator iterator(mServers);
    UtlVoidPtr* pServerContainer = NULL;
    UtlString* pKey = NULL;
    
    while ((pKey = (UtlString*)iterator()))
    {
        pServerContainer = (UtlVoidPtr*) iterator.value();
        if (pServerContainer)
        {
            pServer = (SipClient*)pServerContainer->getValue();
        }
        if (pServer)
        {
            UtlString clientNames;
            long clientTouchedTime = pServer->getLastTouchedTime();
            UtlBoolean clientOk = pServer->isOk();
            pServer->getClientNames(clientNames);
            osPrintf("UDP server %p last used: %ld ok: %d names: \n%s \n",
                this, clientTouchedTime, clientOk, clientNames.data());

            SipProtocolServerBase::printStatus();
        }
    }
}

int SipUdpServer::getServerPort(const char* szLocalIp) 
{
    int port = PORT_NONE;

    char szLocalIpForPortLookup[256];
    memset((void*)szLocalIpForPortLookup, 0, sizeof(szLocalIpForPortLookup));
    
    if (NULL == szLocalIp)
    {
        strcpy(szLocalIpForPortLookup, mDefaultIp);
    }
    else
    {
        strcpy(szLocalIpForPortLookup, szLocalIp);
    }
    
    UtlString localIpKey(szLocalIpForPortLookup);
    
    UtlInt* pUtlPort;
    pUtlPort = (UtlInt*)this->mServerPortMap.findValue(&localIpKey);
    if (pUtlPort)
    {
       port = pUtlPort->getValue();
    }
    
    return port ;
}


UtlBoolean SipUdpServer::getStunAddress(UtlString* pIpAddress, int* pPort,
                                        const char* szLocalIp) 
{
    UtlBoolean bRet = false;
    OsStunDatagramSocket* pSocket = NULL;
    UtlVoidPtr* pSocketContainer = NULL;

    if (szLocalIp)
    {
        UtlString localIpKey(szLocalIp);
       
        pSocketContainer = (UtlVoidPtr*)this->mServerSocketMap.findValue(&localIpKey);
        if (pSocketContainer)
        {
            pSocket = (OsStunDatagramSocket*)pSocketContainer->getValue();
        }
    }
    else
    {
        // just use the default Socket in our collection
        UtlString defaultIpKey(mDefaultIp);
       
        pSocketContainer = (UtlVoidPtr*)mServerSocketMap.findValue(&defaultIpKey);
        if (pSocketContainer != NULL )
        {
            pSocket = (OsStunDatagramSocket*)pSocketContainer->getValue();
        }
    }
    
    if (pSocket)
    {
        bRet =  pSocket->getExternalIp(pIpAddress, pPort) ;
    }
    return bRet;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
