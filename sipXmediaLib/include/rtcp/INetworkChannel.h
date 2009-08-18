//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


//  Border Guard
#ifndef _INetworkChannel_h
#define _INetworkChannel_h

#include "rtcp/RtcpConfig.h"

// Include
#include "IBaseClass.h"


/**
 *
 * Interface Name:  INetworkChannel
 *
 * Inheritance:     IBaseClass
 *
 *
 * Description:     The INetworkChannel interface allows allows a consumer to
 *                  to create, control, and terminate a unidirectional
 *                  network connection for sending or receiving data.
 *
 * Notes:
 *
 */
interface INetworkChannel : public IBaseClass
{

//  Public Methods

public:

/*|><|************************************************************************
 Name:          CNetworkChannel::Initialize

 Description:   A polymorphic method of INetworkChannel that handles the
                creation and initialization of an inbound or outbound
                connectionless datagram channels.  The method shall create
                and open the datagram channel in addition to binding it to
                establish a local port ID.

 Returns:       bool.
 ***********************************************************************|><|*/
    virtual bool  Initialize(void)=0;


/*|><|************************************************************************
 Name:          Connect

 Description:   A polymorphic method that manages the connection of an inbound
                or outbound connectionless datagram channels over TCP/IP.
                This is a non-blocking call.

 Returns:       bool.
 ***********************************************************************|><|*/
    virtual bool  Connect(unsigned char *puchIPAddress,
                          unsigned short usPortID)=0;



/*|><|************************************************************************
 Name:          Close

 Description:   A method that closes a connectionless datagram channel.

 Returns:       bool.
 ***********************************************************************|><|*/
    virtual bool  Close(void)=0;



/*|><|************************************************************************
 Name:          GetSocket

 Description:   An IChannel interface method that retrieves the socket handle
                associated with a given channel.

 Returns:       int.
 ***********************************************************************|><|*/
    virtual int  GetSocket(void)=0;

/*|><|************************************************************************
 Name:          GetPort

 Description:   An IChannel interface method that retrieves the port ID
                associated with a given channel.

 Returns:       unsigned short.
 ***********************************************************************|><|*/
    virtual unsigned short    GetPort(void)=0;


/*|><|************************************************************************
 Name:          IsOpened

 Description:   An IChannel interface method that returns a flag identifying
                whether a channel has been opened.

 Returns:       None.
 ***********************************************************************|><|*/
    virtual bool    IsOpened(void)=0;

/*|><|************************************************************************
 Name:          IsConnected

 Description:   An IChannel interface method that returns a flag identifying
                whether a channel has been connected.

 Returns:       None.
 ***********************************************************************|><|*/
    virtual bool    IsConnected(void)=0;


};


#endif
