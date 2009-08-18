//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#include "rtcp/NetworkChannel.h"
#ifdef INCLUDE_RTCP /* [ */

#ifndef PINGTEL_OSSOCKET
/*|><|************************************************************************
 Name:          CNetworkChannel constructor

 Description:   Performs initial construction on CNetworkChannel class in
                addition to setting internal properties.

 Returns:       None.
 ***********************************************************************|><|*/
CNetworkChannel::CNetworkChannel(INetDispatch *piNetDispatch,
                                 unsigned short usPort)
{

    // Store Constructor argument as internal properties
    m_piNetDispatch             = piNetDispatch;
    if(m_piNetDispatch)
        m_piNetDispatch->AddRef();

    // Initialize Local IP Address
    m_ipLocal.sin_family        = AF_INET;
	m_ipLocal.sin_addr.s_addr   = OsSocket::getDefaultBindAddress();
//    m_ipLocal.sin_addr.s_addr   = INADDR_ANY;
    m_ipLocal.sin_port          = htons(usPort);

    m_ipRemote.sin_family       = AF_INET;
    m_ipRemote.sin_addr.s_addr  = OsSocket::getDefaultBindAddress();;
//    m_ipRemote.sin_addr.s_addr  = INADDR_ANY;
    m_ipRemote.sin_port         = 0;

    // Initialize channel status flags
    m_fFlags.bfReliable         = FALSE;
    m_fFlags.bfOpen             = FALSE;
    m_fFlags.bfBound            = FALSE;
    m_fFlags.bfConnected        = FALSE;
    m_fFlags.bfConnectPending   = FALSE;
    m_fFlags.bfConnectFailed    = FALSE;
    m_fFlags.bfListening        = FALSE;

    // Initialize internal state info
    m_hSocket                   = OS_INVALID_SOCKET_DESCRIPTOR;

    // Create a Critical Section
    InitializeCriticalSection(&m_csChannelSynchronized);

}
#else

/*|><|************************************************************************
 Name:          CNetworkChannel constructor

 Description:   An alternate constructor defined to handle use of Pingtel's
                OSSocket object.  In this mode, this object acts as a simple
                wrapper that calls the Pingtel OSSocket object's Write method
                whenever Send() is called.  The OSSocket has already been
                initialized by external logic and is operational when passed.

 Returns:       None.
 ***********************************************************************|><|*/
CNetworkChannel::CNetworkChannel(OsSocket & rOsSocket)
                : m_piNetDispatch(NULL), m_rOsSocket(rOsSocket)
{

    // Initialize Local IP Address
    m_ipLocal.sin_family        = AF_INET;
    m_ipLocal.sin_addr.s_addr   = OsSocket::getDefaultBindAddress();
    m_ipLocal.sin_port          = 0;

    m_ipRemote.sin_family       = AF_INET;
    m_ipRemote.sin_addr.s_addr  = OsSocket::getDefaultBindAddress();
    m_ipRemote.sin_port         = 0;

    // Initialize channel status flags
    m_fFlags.bfReliable         = FALSE;
    m_fFlags.bfConnected        = FALSE;
    m_fFlags.bfConnectPending   = FALSE;
    m_fFlags.bfConnectFailed    = FALSE;
    m_fFlags.bfListening        = FALSE;

    // This is a fake out to accomodate the use of the Send() method as
    // a wrapper to the OsSocket object.
    m_fFlags.bfOpen             = TRUE;
    m_fFlags.bfBound            = TRUE;

    // Initialize internal state info
    m_hSocket                   = OS_INVALID_SOCKET_DESCRIPTOR;

}

#endif

/*|><|************************************************************************
 Name:          CNetworkChannel destructor

 Description:   Performs final destruction on CNetworkChannel class.

 Returns:       None.
 ***********************************************************************|><|*/
CNetworkChannel :: ~CNetworkChannel(void)
{

    // Release reference to store interfaces
    if(m_piNetDispatch)
        m_piNetDispatch->Release();
}


/*|><|************************************************************************
 Name:          CNetworkChannel::Initialize

 Description:   A polymorphic method of INetworkChannel that handles the
                creation and initialization of an inbound or outbound
                connectionless datagram channels.  The method shall create
                and open the datagram channel in addition to binding it to
                establish a local port ID.

 Returns:       bool.
 ***********************************************************************|><|*/
bool  CNetworkChannel::Initialize(void)
{


    // Open channel
    if(!Open(AF_INET,
            (m_fFlags.bfReliable ? SOCK_STREAM : SOCK_DGRAM),
            IPPROTO_IP))
        return(FALSE);


    // Check whether a valid port ID has been assigned as part of
    // Construction.  If not, we shall assign our own PortId with
    // the GetNewPort() method.  [or not... HZM]
    // if(!m_ipLocal.sin_port)
        // m_ipLocal.sin_port = GetNewPort();

    // The to the particular port ID to the opened socket
    if(!Bind(&m_ipLocal))
    {
/*
        osPrintf(
           "**** FAILURE **** CNetworkChannel::Initialize() - Bind Failed\n");
*/
        return(FALSE);
    }

    return(TRUE);

}


/*|><|************************************************************************
 Name:          CNetworkChannel::Open

 Description:   A polymorphic method of IChannel that manages the creation and
                of an inbound or outbound connectionless datagram or
                connection-oriented channels over TCP/IP.

 Returns:       bool.
 ***********************************************************************|><|*/
bool  CNetworkChannel::Open(int iAddressType, int iSocketType, int iProtocol)
{

    // New (unassociated) socket
    if (OS_INVALID_SOCKET_DESCRIPTOR ==
        (m_hSocket = socket(iAddressType, iSocketType, iProtocol)))
    {
        // Socket creation failed - find out why
/*
        osPrintf("**** FAILURE **** CNetworkChannel::Open()"
                                        " - Error Opening Socket\n");
*/
        return(FALSE);
    }

    // Flag the channel as open
    m_fFlags.bfOpen = TRUE;

    return(TRUE);
}


/*|><|************************************************************************
 Name:          CNetworkChannel::Bind

 Description:   A polymorphic method of IChannel that manages the creation and
                of an inbound or outbound connectionless datagram or
                connection-oriented channels over TCP/IP.

 Returns:       bool.
 ***********************************************************************|><|*/
bool  CNetworkChannel::Bind(SOCKADDR_IN *lpAddr)
{

    // Channel must be open before it can be bound
    if (!m_fFlags.bfOpen)
    {
/*
        osPrintf("**** FAILURE **** CNetworkChannel::Bind() - Bind Failed\n");
*/
        return(FALSE);
    }

    // If it's already bound, just return OK
    if (m_fFlags.bfBound)
    {
/*
        osPrintf("**** FAILURE **** CNetworkChannel::Bind()"
                                                   " - Already Bound\n");
*/
        return(TRUE);
    }

    // If caller passes a null address
    if (lpAddr == NULL)
    {
        // Set to default local interface and get a new port number
        lpAddr = &m_ipLocal;
        lpAddr->sin_port = GetNewPort();
    }


 // Try to bind the socket
    while (SOCKET_ERROR ==
              bind(m_hSocket, (SOCKADDR*) lpAddr, sizeof(SOCKADDR)))
    {
        // Bind failed - why?
#ifdef WIN32
        if (WSAGetLastError() == WSAEADDRINUSE)
        {
            // Get new port number if the previous one is in use
            lpAddr->sin_port = GetNewPort();
        }
        else
#endif
        {
/*
            osPrintf("**** FAILURE **** CNetworkChannel::Bind() -"
                       " Unable to Bind using the specified Socket:PortID\n");
*/
            return(FALSE);
        }
    }

    // Fetch & store address:port that the socket is bound to
    int iLength = sizeof(SOCKADDR);

    if (SOCKET_ERROR ==
#ifndef __pingtel_on_posix__
             getsockname(m_hSocket, (SOCKADDR *) &m_ipLocal, &iLength))
#else
             getsockname(m_hSocket, (SOCKADDR *) &m_ipLocal, (socklen_t *)&iLength))
#endif
    {
/*
        osPrintf("**** FAILURE **** CNetworkChannel::Bind() -"
                               " Unable to retrieve Name of Bound Socket\n");
*/
        return(FALSE);
    }
    else
    {
        // Socket is bound; update flag
        m_fFlags.bfBound = TRUE;
    }

    return(TRUE);
}

/*|><|************************************************************************
 Name:          CNetworkChannel::Connect

 Description:   A polymorphic method of INetworkChannel that manages the
                connection of an inbound or outbound connectionless datagram
                or connection-oriented channels over TCP/IP.
                This is a non-blocking call.

 Returns:       bool.
 ***********************************************************************|><|*/
bool CNetworkChannel::Connect(unsigned char *puchIPAddress,
                              unsigned short usPortID)
{

    // Initialize Remote IP Address
    if(puchIPAddress)
    {
        m_ipRemote.sin_family       = AF_INET;
        m_ipRemote.sin_addr.s_addr  = inet_addr((char *)puchIPAddress);
        m_ipRemote.sin_port         = htons(usPortID);
    }

    return(TRUE);
}

/*|><|************************************************************************
 Name:          CNetworkChannel::Send

 Description:   A polymorphic method of INetworkChannel that sends data
                over an outbound connection connectionless datagram or
                connection-oriented channel.  This is a non-blocking call.

 Returns:       bool.
 ***********************************************************************|><|*/
int  CNetworkChannel::Send(unsigned char *puchDataBuffer,
                           unsigned long ulBytesToSend)
{

    // The port must be open before sending
    if (!m_fFlags.bfOpen)
    {
/*
        osPrintf("**** FAILURE **** CNetworkChannel::Send() -"
                                " Unable to Send %lu bytes\n", ulBytesToSend);
*/
        return(0);
    }

    // The port must be bound to a local interface before sending
    if (!m_fFlags.bfBound)
    {
/*
        osPrintf("**** FAILURE **** CNetworkChannel::Send() -"
                                                  " Channel is not Bound\n");
*/
        return(0);
    }

    int iSent;

#ifdef PINGTEL_OSSOCKET

    // Call the corresponding OS Socket call to perform the Network Write
    iSent = m_rOsSocket.write((char *)puchDataBuffer, (int)ulBytesToSend);

#else
    // This is a Datagram to be sent over a Network connection
    if (!m_fFlags.bfConnected)
    {
        // If an unconnected socket, specify the address to send the data
        iSent = sendto(m_hSocket, (char *)puchDataBuffer, ulBytesToSend, 0,
                      (SOCKADDR *) &m_ipRemote, sizeof(SOCKADDR));
    }

    // This is a stream based connection
    else
    {
        // Connected socket; send the data
        iSent = send(m_hSocket, (char *)puchDataBuffer, ulBytesToSend, 0);
    }
#endif

    // Transmission Error.  Log it and update statistics
    if (iSent != (int)ulBytesToSend)
    {
        // Send failed. Report why.
/*
        osPrintf("**** FAILURE **** CNetworkChannel::Send() - Send Failed:\n"
           "   iSent=%d, BytesToSend = %lu, errno=%d '%s'\n",
           iSent, ulBytesToSend, errno, strerror(errno));
*/
    }

    return(iSent);
}


/*|><|************************************************************************
 Name:          CNetworkChannel::Receive

 Description:   A polymorphic method of IChannel that processes any receive
                events associated with a channel.  This version of receive
                shall handle inbound data on either a connectionless or
                connection-oriented channel.  Arriving data shall be
                dispatched to a Processing routine while various errors or
                channel conditions shall be reported and returned as errors.

 Returns:       int
 ***********************************************************************|><|*/
int  CNetworkChannel::Receive(unsigned char *puchBuffer,
                              unsigned long ulBufferSize)
{

    // The port must be open before connecting
    if (!m_fFlags.bfOpen)
    {
/*
        osPrintf("**** FAILURE **** CNetworkChannel::Receive() -"
                                              " Channel No longer opened\n");
*/
        return(0);
    }

    // Initialize Receive byte count
    int iRxBytes = 0;

    // Read the pending data if the Channel is already connected
    if (m_fFlags.bfConnected)
    {
        // Connected socket - fetch the received data
        iRxBytes = recv(m_hSocket, (char *)puchBuffer, ulBufferSize, 0);
    }

    else if (!m_fFlags.bfReliable)
    {
        socklen_t iLength = sizeof(SOCKADDR_IN);

        // If an unconnected Network channel, must use recvfrom()
        // After receiving the data, we'll connect the channel.
        // Connecting it will filter out renegade datagrams.

        // Fetch the received data and the address of the sender
        iRxBytes = recvfrom(m_hSocket, (char *)puchBuffer,  ulBufferSize, 0,
#ifndef __pingtel_on_posix__
                            (SOCKADDR *) &m_ipRemote, &iLength);
#else
/* matching ( */            (SOCKADDR *) &m_ipRemote, &iLength);
#endif

        // If data read, connect the datagram socket to the peer to enable
        // filtering of incoming data. If the connect() works - it works.
        // If not, who cares.
        if (iRxBytes > 0 && !m_fFlags.bfConnected)
        {
            if (SOCKET_ERROR ==
                    ::connect(m_hSocket, (SOCKADDR *) &m_ipRemote, iLength))
            {
/*
                osPrintf(">>>>> CNetworkChannel::Receive() -"
                              " Connected to Remote Network Client <<<<<<\n");
*/
                m_fFlags.bfConnected = TRUE;
            }
            else
            {
/*
                osPrintf("**** FAILURE **** CNetworkChannel::Receive() -"
                                " Connect to Remote Network Client Failed\n");
*/
            }
        }
    }

    // An illegal read was requested for a stream connection that
    // was not connected.
    else
    {
/*
        osPrintf("**** FAILURE **** CNetworkChannel::Receive() -"
                         " Invalid Receive Request on an Unconnect Socket\n");
*/
    }


    // While trying to read the channel, we received an error
    if (iRxBytes == SOCKET_ERROR)
    {
#ifdef WIN32
        // Recv[from]() failed - get error code
        if (WSAGetLastError() == WSAEMSGSIZE)
        {
/*
            osPrintf("**** FAILURE **** CNetworkChannel::Receive() -"
                                                    " RX Buffer too small\n");
*/
        }
        else
#endif
        {
/*
            osPrintf("**** FAILURE **** CNetworkChannel::Receive() -"
                                                             " Recv Error\n");
*/
        }

    }

    return(iRxBytes);
}


/*|><|************************************************************************
 Name:          CNetworkChannel::Dispatch

 Description:   A private method that takes a data transmission forwarded
                by the Receive() method and sends it the associated stream
                client for transmission to the VCP.  The Process() method
                also contains some conditionally compiled logic to collect
                statistics regarding packets received and out-of-sequence
                packets.

 Returns:       None.
 ***********************************************************************|><|*/
void  CNetworkChannel::Dispatch(unsigned char *puchBuffer,
                                unsigned long ulBytesRecvd)
{

    // Dispatch the message to the registered object
    m_piNetDispatch->ProcessPacket(puchBuffer, ulBytesRecvd);

    return;
}



/*|><|************************************************************************
 Name:          CNetworkChannel::Close

 Description:   An IChannel interface method that closes either a
                connectionless or connection-oriented channel.
                Connection-oriented channels shall be instructed to
                shutdown prior to the actual closure of the channel.

 Returns:       bool.
 ***********************************************************************|><|*/
bool  CNetworkChannel::Close(void)
{

    // Enter the Critical Section
    EnterCriticalSection(&m_csChannelSynchronized);


    // Return False if the channel isn't opened
    if(!m_fFlags.bfOpen)
    {
/*
        osPrintf("**** FAILURE **** CNetworkChannel::Close() -"
                                                     " Channel Not Opened\n");
*/
    }

    // Shut a connected socket down - stop all sends and receives
    else
    {

        // Reset flags appropriately
        m_fFlags.bfOpen = FALSE;
        m_fFlags.bfConnected = FALSE;
        m_fFlags.bfBound     = FALSE;
        m_fFlags.bfListening = FALSE;
        m_ipLocal.sin_port = 0;
/*
        osPrintf("**** FAILURE **** CNetworkChannel::Close() -"
                                                         " Closing socket\n");
*/

        // Close the socket
#ifdef WIN32
        if (closesocket(m_hSocket) == SOCKET_ERROR)
#else
        if (close(m_hSocket) == SOCKET_ERROR)
#endif
        {
/*
            // Continue on despite error
            osPrintf("**** FAILURE **** CNetworkChannel::Close() -"
                                                     " CloseSocket Failed\n");
*/
        }

        m_hSocket = OS_INVALID_SOCKET_DESCRIPTOR;
    }

    // Leave the Critical Section
    LeaveCriticalSection(&m_csChannelSynchronized);

    return(TRUE);
}


/*|><|************************************************************************
 Name:          CNetworkChannel::GetRemoteAddress

 Description:   A private method that returns loads the SOCKADDR_IN
                structure provided by the caller with information
                identifying the connected FE.

 Returns:       None.
 ***********************************************************************|><|*/
void  CNetworkChannel :: GetRemoteAddress( SOCKADDR_IN *lpAddr )
{
    memcpy((void*)lpAddr, (void*)&m_ipRemote, sizeof(m_ipRemote));
}


/*|><|************************************************************************
 Name:          CNetworkChannel::GetRemoteAddress

 Description:   A private method that loads the SOCKADDR_IN structure
                maintained as a channel property with the information
                provided by the caller.

 Returns:       None.
 ***********************************************************************|><|*/
void  CNetworkChannel :: SetRemoteAddress( SOCKADDR_IN *lpAddr )
{
    memcpy((void*)&m_ipRemote, (void*)lpAddr, sizeof(m_ipRemote));
}



/*|><|************************************************************************
 Name:          CNetworkChannel::GetNewPort

 Description:   A private method that generates a new port ID from a pool
                reserved for communication.  The pool starts ar ID 5000
                and continues to 7000.  When 7000 is reached, it wraps
                back to 5000.

 Returns:       unsigned short.
 ***********************************************************************|><|*/
unsigned short  CNetworkChannel::GetNewPort(void)
{
    static unsigned short   wMasterPort = 5000;
    unsigned short          wPort;

    if (!(wMasterPort % 7000))
    {
        wMasterPort = 5000;
    }

    wPort = wMasterPort += 2;

    return (ntohs(wPort));
}
#endif /* INCLUDE_RTCP ] */
