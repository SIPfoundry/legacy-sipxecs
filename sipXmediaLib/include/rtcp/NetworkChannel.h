//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _INC_NETWORKCHANNEL_H_
#define _INC_NETWORKCHANNEL_H_

#include "rtcp/RtcpConfig.h"

#ifdef PINGTEL_OSSOCKET
#include "os/OsSocket.h"
#endif

#if defined(_WIN32)
#include <winsock.h>
#elif defined(_VXWORKS)
#include <sockLib.h>
#include <ioLib.h>
typedef struct sockaddr_in SOCKADDR_IN;
#define SOCKET_ERROR    ERROR
#elif defined(__pingtel_on_posix__)
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
#define SOCKET_ERROR    (-1)
#else
#error Unsupported target platform.
#endif

//  Includes
#include "BaseClass.h"
#include "INetworkChannel.h"
#include "INetworkSource.h"
#include "INetworkRender.h"
#include "INetDispatch.h"



class CNetworkChannel : public CBaseClass,
                        public INetworkSource,
                        public INetworkRender
{


public:

#ifndef PINGTEL_OSSOCKET


/*|><|***********************************************************************
 Name:          CNetworkChannel constructor

 Description:   Performs initial construction on CNetworkChannel class in
                addition to setting internal properties.

 Returns:       None.
 ***********************************************************************|><|*/
    CNetworkChannel(INetDispatch *piNetDispatch=NULL, unsigned short usPort=0);

#else

/*|><|***********************************************************************
 Name:          CNetworkChannel constructor

 Description:   An alternate constructor defined to handle use of Pingtel's
                OSSocket object.  In this mode, this object acts as a simple
                wrapper that calls the Pingtel OSSocket object's Write method
                whenever Send() is called.  The OSSocket has already been
                initialized by external logic and is operational when passed.

 Returns:       None.
 ***********************************************************************|><|*/
    CNetworkChannel(OsSocket & rOsSocket);
#endif

/*|><|***********************************************************************
 Name:          CNetworkChannel destructor

 Description:   Performs final destruction on CNetworkChannel class.

 Returns:       None.
 ***********************************************************************|><|*/
    ~CNetworkChannel(void);


/*|><|***********************************************************************
 Name:          Initialize

 Description:   A polymorphic method that handles the creation and
                initialization of an inbound or outbound connectionless
                datagram channels.  This method shall create and open the
                datagram channel in addition to binding it to establish a
                local port ID.

 Returns:       bool.
 ***********************************************************************|><|*/
    virtual bool Initialize(void);


/*|><|***********************************************************************
 Name:          CNetworkChannel::Open

 Description:   A polymorphic method that manages the creation and
                of an inbound or outbound connectionless datagram channel.

 Returns:       bool.
 ***********************************************************************|><|*/
    virtual bool Open(int iAddressType, int iSocketType, int iProtocol);


/*|><|***********************************************************************
 Name:          Connect

 Description:   A polymorphic method that manages the connection of an inbound
                or outbound connectionless datagram channels over TCP/IP.
                This is a non-blocking call.

 Returns:       bool.
 ***********************************************************************|><|*/
    virtual bool  Connect(unsigned char *puchIPAddress=NULL,
                                                unsigned short usPortID=0);

/*|><|***********************************************************************
 Name:          Receive

 Description:   A polymorphic method that processes any receive events
                associated with a channel.  This version of receive shall
                handle inbound data on either a connectionless channel.

 Returns:       int.
 ***********************************************************************|><|*/
    virtual int  Receive(unsigned char * puchBuffer, unsigned long ulRxBytes);

/*|><|***********************************************************************
 Name:          Dispatch

 Description:   A method that takes a data transmission forwarded by the
                Receive() method and routes it to the appopriate receiver.

 Returns:       None.
 ***********************************************************************|><|*/
    virtual void  Dispatch(unsigned char * puchBuffer,
                                              unsigned long ulBytesRecvd);

/*|><|***********************************************************************
 Name:          Send

 Description: A private method that takes a data transmission delivered by the
              StreamClient's DataReceived() callback and sends it the
              associated channel socket for transmission to the respective FE.
              The Send() method also contains some conditionally compiled logic
              to collect statistics regarding packets being sent.

 Returns:       bool.
 ***********************************************************************|><|*/
    virtual int  Send(unsigned char * puchDataBuffer,
                                               unsigned long ulBytesToSend);

/*|><|***********************************************************************
 Name:          Close

 Description:   Amethod that closes a connectionless datagram channel.

 Returns:       bool.
 ***********************************************************************|><|*/
    virtual bool  Close(void);


/*|><|***********************************************************************
 Name:          GetSocket

 Description:   An IChannel interface method that retrieves the socket handle
                associated with a given channel.

 Returns:       int.
 ***********************************************************************|><|*/
    int  GetSocket(void)     { return m_hSocket; }

/*|><|***********************************************************************
 Name:          GetPort

 Description:   An IChannel interface method that retrieves the port ID
                associated with a given channel.

 Returns:       unsigned short.
 ***********************************************************************|><|*/
    unsigned short    GetPort(void)    { return m_ipLocal.sin_port; }


/*|><|***********************************************************************
 Name:          IsOpened

 Description:   An IChannel interface method that returns a flag identifying
                whether a channel has been opened.

 Returns:       None.
 ***********************************************************************|><|*/
    bool    IsOpened(void)      { return((bool)m_fFlags.bfOpen); }

/*|><|***********************************************************************
 Name:          IsConnected

 Description:   An IChannel interface method that returns a flag identifying
                whether a channel has been connected.

 Returns:       None.
 ***********************************************************************|><|*/
    bool    IsConnected(void)   { return((bool)m_fFlags.bfConnected); }

/**
 *
 * Macro Name:  DECLARE_IBASE_M
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: This implements the IBaseClass functions used and exposed by
 *              derived classes.
 *
 * Usage Notes:
 *
 *
 */
DECLARE_IBASE_M

private:    // Private Member Functions

/*|><|***********************************************************************
 Name:          Bind

 Description:   A polymorphic method that manages the creation and
                of an inbound or outbound connectionless datagram channels.

 Returns:       bool.
 ***********************************************************************|><|*/
    virtual bool  Bind(SOCKADDR_IN *lpAddr=NULL);


/*|><|***********************************************************************
 Name:          GetNewPort

 Description:   A private method that generates a new port ID from a pool
                reserved for communication.
                The pool starts at ID 5000 and continues to 7000.  When 7000
                is reached, it wraps back to 5000.

 Returns:       unsigned short.
 ***********************************************************************|><|*/
    static unsigned short  GetNewPort(void);


/*|><|***********************************************************************
 Name:          GetRemoteAddress

 Description:   A private method that returns  the SOCKADDR_IN structure
                provided by the caller with information identifying the
                connected FE.

 Returns:       None.
 ***********************************************************************|><|*/
    void  GetRemoteAddress( SOCKADDR_IN *lpAddr );

/*|><|***********************************************************************
 Name:          SetRemoteAddress

 Description:   A private method that loads the SOCKADDR_IN structure
                maintained as a channel property with the information provided
                by the caller.

 Returns:       None.
 ***********************************************************************|><|*/
    void  SetRemoteAddress( SOCKADDR_IN *lpAddr );


private:

    CRITICAL_SECTION m_csChannelSynchronized;
    INetDispatch    *m_piNetDispatch;
    int              m_hSocket;          // Socket

    SOCKADDR_IN      m_ipLocal;          // Local address:port
    SOCKADDR_IN      m_ipRemote;         // Remote address:port

    struct tagChannelFlags              // State/Status flags
    {
        unsigned short  bfOpen           : 1;   // TRUE if socket is open
        unsigned short  bfDirection      : 2;
                                     // 01=Rx, 10=Tx, 11=bidirectional
        unsigned short  bfReliable       : 1;
                                     // Is the channel a reliable channel
        unsigned short  bfBound          : 1;   // TRUE if socket is bound
        unsigned short  bfConnectPending : 1;   // Connection pending
        unsigned short  bfConnected      : 1;   // TRUE if socket is connected
        unsigned short  bfConnectFailed  : 1;
                                     // TRUE if the connection request failed
        unsigned short  bfListening      : 1;   // Socket is listening
        unsigned short  bfEnableRecv     : 1;   // Enable receiving of data
    } m_fFlags;

#ifdef PINGTEL_OSSOCKET
// Extension to allow the Network Channel object to be used as a wrapper
// for a Pingtel OS Socket object
    OsSocket & m_rOsSocket;
#endif

};




#endif
