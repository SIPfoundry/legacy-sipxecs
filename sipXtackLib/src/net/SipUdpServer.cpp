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
#include <net/CallId.h>
#include <net/SipUdpServer.h>
#include <net/SipUserAgent.h>
#include <net/Url.h>
#include <os/OsDateTime.h>
#include <os/OsEventMsg.h>
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
   SipProtocolServerBase(userAgent,
                         SIP_TRANSPORT_UDP,
                         "SipUdpServer-%d"),
   mNatPingUrl(natPingUrl),
   mNatPingFrequencySeconds(natPingFrequencySeconds),
   mNatPingMethod(natPingMethod && *natPingMethod ? natPingMethod : "PING"),
   mStunRefreshSecs(28), 
   mStunOptions(0)
{
    OsSysLog::add(FAC_SIP, PRI_DEBUG,
                  "SipUdpServer[%s]::_ port = %d, bUseNextAvailablePort = %d, szBoundIp = '%s'",
                  getName().data(), port, bUseNextAvailablePort, szBoundIp);

    if (szBoundIp && 0 != strcmp(szBoundIp, "0.0.0.0"))
    {
       // If an address was supplied, only open a socket on that address.
       mDefaultIp = szBoundIp;
       int serverSocketPort = port;
       createServerSocket(szBoundIp, serverSocketPort, bUseNextAvailablePort,
                          udpReadBufferSize);
    }
    else
    {
       // If no address was supplied, get all local addresses and open
       // sockets on all of them.
       int numAddresses = 0;
       const HostAdapterAddress* adapterAddresses[MAX_IP_ADDRESSES];
       getAllLocalHostIps(adapterAddresses, numAddresses);

       if (numAddresses >= 1)
       {
          // Use the first IP address in the array for the 'default IP address'.
          mDefaultIp = adapterAddresses[0]->mAddress.data();
       }
       for (int i = 0; i < numAddresses; i++)
       {
          int serverSocketPort = port;
            
          createServerSocket(adapterAddresses[i]->mAddress.data(),
                             serverSocketPort,
                             bUseNextAvailablePort,
                             udpReadBufferSize);
          delete adapterAddresses[i];   
       }
    }
}

// Destructor
SipUdpServer::~SipUdpServer()
{
}

/* ============================ MANIPULATORS ============================== */

void SipUdpServer::createServerSocket(const char* szBindAddr,
                                      int& port,
                                      const UtlBoolean& bUseNextAvailablePort, 
                                      int udpReadBufferSize)
{
   // Create the socket.
   OsStunDatagramSocket* pSocket =
      new OsStunDatagramSocket(0, NULL, port, szBindAddr, FALSE);
   
   // If the socket is busy or unbindable and the user requested using the
   // next available port, try the next SIP_MAX_PORT_RANGE ports.
   if (bUseNextAvailablePort)
   {
      for (int i=1; !pSocket->isOk() && i<=SIP_MAX_PORT_RANGE; i++)
      {
         delete pSocket;
         pSocket = new OsStunDatagramSocket(0, NULL, port+i, szBindAddr, FALSE);
      }
   }
   
   // If we opened the socket.
   if (pSocket->isOk())
   {     
      // Inform the SipUserAgent of the contact address.
      if (mSipUserAgent)
      {
         port = pSocket->getLocalHostPort();
         CONTACT_ADDRESS contact;
         strcpy(contact.cIpAddress, szBindAddr);
         contact.iPort = port;
         contact.eContactType = LOCAL;
         char szAdapterName[16];
         memset((void*)szAdapterName, 0, sizeof(szAdapterName)); // null out the string

         getContactAdapterName(szAdapterName, contact.cIpAddress);

         strcpy(contact.cInterface, szAdapterName);
         mSipUserAgent->addContactAddress(contact);
      }

      // Add address and port to the maps.
      UtlString* address = new UtlString(szBindAddr);
      mServerSocketMap.insertKeyAndValue(address, pSocket);
      port = pSocket->getLocalHostPort();
      mServerPortMap.insertKeyAndValue(address,
                                       new UtlInt(port));

      // Get the UDP buffer size from the kernel.
      int sockbufsize = 0;
      // The type of 'size' depends on the platform.
#if defined(__pingtel_on_posix__)
      socklen_t
#else
         int
#endif
         size = sizeof (sockbufsize);
      getsockopt(pSocket->getSocketDescriptor(),
                 SOL_SOCKET,
                 SO_RCVBUF,
                 (char*) &sockbufsize,
                 &size);
#ifdef LOG_SIZE
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipUdpServer[%s]::SipUdpServer UDP buffer size: %d size: %d",
                    getName().data(), sockbufsize, size);
#endif /* LOG_SIZE */

      // If the user specified a UDP buffer size, attempt to set it.
      if (udpReadBufferSize > 0)
      {
         setsockopt(pSocket->getSocketDescriptor(),
                    SOL_SOCKET,
                    SO_RCVBUF,
                    (char*) &udpReadBufferSize,
                    sizeof (udpReadBufferSize));

         getsockopt(pSocket->getSocketDescriptor(),
                    SOL_SOCKET,
                    SO_RCVBUF,
                    (char*) &sockbufsize,
                    &size);
#ifdef LOG_SIZE
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipUdpServer[%s]::SipUdpServer reset UDP buffer size: %d size: %d",
                       getName().data(), sockbufsize, size);
#endif /* LOG_SIZE */
      }
   }
}

int SipUdpServer::run(void* runArg)
{
    mPingCseq = 1;
    // Timer to trigger keepalive messages.
    OsTimer pingTimer(getMessageQueue(), 0);

    if (mSipUserAgent)
    {
        mSipUserAgent->getContactUri(&mPingContact);

        // Add a tag to the contact and build the From field
        mPingFrom = mPingContact;
        {
           UtlString tag;
           CallId::getNewTag("", tag);

           mPingFrom.append(";tag=");
           mPingFrom.append(tag);
        }

        UtlString rawAddress;
        Url pingUrl(mNatPingUrl);

        // Create a cannonized version of the ping URL in case
        // it does not specify "sip:", etc.
        UtlString mPingCanonizedUrl = pingUrl.toString();

        // Get the address and port in the ping URL so that
        // we can look up the DNS stuff if needed
        mPingPort = pingUrl.getHostPort();
        pingUrl.getHostAddress(rawAddress);

        // Resolve the raw address from a DNS SRV, A record
        // to an IP address
        server_t* dnsSrvRecords =
            SipSrvLookup::servers(rawAddress.data(),
                                  "sip",
                                  OsSocket::UDP,
                                  mPingPort);

        // Do a DNS SRV or A record lookup
        // If we started with an IP address, we will still get an IP
        // address in the result
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
            dnsSrvRecords[0].getIpAddressFromServerT(mPingAddress);
            mPingPort = dnsSrvRecords[0].getPortFromServerT();

            // If the ping URL or DNS SRV did not specify a port
            // bind it to the default port.
            if (!portIsValid(mPingPort))
            {
               mPingPort = SIP_PORT;
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
                    "SipUdpServer::run DNS lookup failed for ping host '%s' derived from URI '%s'",
                    rawAddress.data(), mNatPingUrl.data());
            }
            // Else no ping address, this means we are not supposed to
            // do a ping
        }
        // Free the list of server addresses.
        delete[] dnsSrvRecords;

        // Get the address to be used in the callId scoping
        int dummyPort;
        
        if (mSipUserAgent)
        {
            mSipUserAgent->getViaInfo(OsSocket::UDP, mPingCallId, dummyPort);
        }

        // Make up a call Id
        long epochTime = OsDateTime::getSecsSinceEpoch();
        int randNum = rand();
        char callIdPrefix[80];
        sprintf(callIdPrefix, "%ld%d-ping@", epochTime, randNum);
        mPingCallId.insert(0,callIdPrefix);

        // If keepalives are configured, start the timer to trigger them.
        if (mNatPingFrequencySeconds > 0 &&
            !mNatPingUrl.isNull() &&
            !mNatPingMethod.isNull() &&
            !mPingAddress.isNull())
        {
           OsTime period(mNatPingFrequencySeconds, 0);
           pingTimer.periodicEvery(OsTime::NO_WAIT, period);
        }
    }

    // Now that we have done the special set-up work, execute messages
    // like a normal OsServerTask.
    OsServerTask::run(runArg);

    // Turn off the timer.
    pingTimer.stop();

    return(mNatPingFrequencySeconds);
}

// Handles an incoming message (from the message queue).
UtlBoolean SipUdpServer::handleMessage(OsMsg& eventMessage)
{
   UtlBoolean messageProcessed = FALSE;

   int msgType = eventMessage.getMsgType();
   int msgSubType = eventMessage.getMsgSubType();

   if(msgType == OsMsg::OS_EVENT &&
             msgSubType == OsEventMsg::NOTIFY)
   {
      // The timer has signaled it is time to send keepalives.
      sendKeepalives();
      messageProcessed = TRUE;
   }
   else
   {
      // Continue with the generic processing.
      SipProtocolServerBase::handleMessage(eventMessage);
   }

   return (messageProcessed);
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
        UtlString key(szIpToStun);
        
        OsStunDatagramSocket* pSocket =
           dynamic_cast <OsStunDatagramSocket*> (mServerSocketMap.findValue(&key)); 
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
    UtlHashMapIterator iterator(mServers);
    UtlString* pKey = NULL;
    
    while ((pKey = dynamic_cast <UtlString*> (iterator())))
    {
       SipClient* pServer = dynamic_cast <SipClient*> (iterator.value());
       pServer->requestShutdown();
    }
}


UtlBoolean SipUdpServer::sendTo(SipMessage& message,
                               const char* address,
                               int port,
                               const char* szLocalSipIp)
{
    UtlBoolean sendOk;
    SipClient* pServer = NULL;
    
    if (szLocalSipIp)
    {
        UtlString localKey(szLocalSipIp);
        pServer = dynamic_cast <SipClient*> (mServers.findValue(&localKey));
    }
    else
    {
        // no local sip IP specified, so, use the default one
        pServer = dynamic_cast <SipClient*> (mServers.findValue(&mDefaultIp));
    }
    
    if (pServer)
    {
        sendOk = pServer->sendTo(message, address, port);
    }
    else
    {
        sendOk = false;
    }
    return (sendOk);
}

/* ============================ ACCESSORS ================================= */

void SipUdpServer::printStatus()
{
    UtlHashMapIterator iterator(mServers);
    UtlString* pKey = NULL;
    
    while ((pKey = dynamic_cast <UtlString*> (iterator())))
    {
       SipClient* pServer = dynamic_cast <SipClient*> (iterator.value());
       UtlString clientNames;
       long clientTouchedTime = pServer->getLastTouchedTime();
       UtlBoolean clientOk = pServer->isOk();
       pServer->getClientNames(clientNames);
       osPrintf("UDP server %p last used: %ld ok: %d names: \n%s \n",
                this, clientTouchedTime, clientOk, clientNames.data());

       SipProtocolServerBase::printStatus();
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
    pUtlPort = dynamic_cast <UtlInt*> (mServerPortMap.findValue(&localIpKey));
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

    if (szLocalIp)
    {
        UtlString localIpKey(szLocalIp);
       
        pSocket =
           dynamic_cast <OsStunDatagramSocket*> (mServerSocketMap.findValue(&localIpKey));
    }
    else
    {
        // just use the default Socket in our collection
        UtlString defaultIpKey(mDefaultIp);
       
        pSocket =
           dynamic_cast <OsStunDatagramSocket*> (mServerSocketMap.findValue(&defaultIpKey));
    }
    
    if (pSocket)
    {
        bRet =  pSocket->getExternalIp(pIpAddress, pPort);
    }
    return bRet;
}


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

OsSocket* SipUdpServer::buildClientSocket(int hostPort,
                                          const char* hostAddress,
                                          const char* localIp,
                                          bool& existingSocketReused)
{
   OsStunDatagramSocket* pSocket;

   if (mSipUserAgent &&
       (mSipUserAgent->getUseRport() ||
        mSipUserAgent->isSymmetricSignalingImposed()))
   {
      // If there is a SipUserAgent and if it is set to use rport (why?),
      // look up the server socket for this local IP.
      assert(localIp != NULL);
      UtlString localKey(localIp);

      pSocket =
         dynamic_cast <OsStunDatagramSocket*> (mServerSocketMap.findValue(&localKey));
      assert(pSocket);

      existingSocketReused = true;
   }
   else
   {
      // Otherwise, construct a new socket.
      pSocket = new OsStunDatagramSocket(hostPort, hostAddress,
                                         0, localIp, 
                                         !mStunServer.isNull(),
                                         mStunServer.data(),
                                         mStunRefreshSecs,
                                         mStunOptions);

      existingSocketReused = false;
   }
   return pSocket;
}

// Send the keepalives.
void SipUdpServer::sendKeepalives()
{
   // Send a no-op SIP message to the
   // server to keep a port open through a NAT
   // based firewall
   SipMessage pingMessage;
   pingMessage.setRequestData(mNatPingMethod, mPingCanonizedUrl.data(),
                              mPingFrom.data(), mNatPingUrl.data(),
                              mPingCallId, mPingCseq, mPingContact.data());

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
   pingMessage.setTopViaTag("", "rport");
#           ifdef TEST_PRINT            
   osPrintf("Sending ping to %s %d, From: %s\n",
            mPingAddress.data(), mPingPort, mPingContact.data());
#           endif
            
   // Send from the same UDP port that we receive from
   if (mSipUserAgent)
   {
      mSipUserAgent->sendSymmetricUdp(pingMessage, mPingAddress.data(), mPingPort);
   }

   mPingCseq++;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
