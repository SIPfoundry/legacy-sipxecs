//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

//  Border Guard
#ifndef _INetworkSource_h
#define _INetworkSource_h

#include "rtcp/RtcpConfig.h"

//  Includes
#include "IBaseClass.h"
#include "INetworkChannel.h"

/**
 *
 * Interface Name:  INetworkSource
 *
 * Inheritance:     INetworkChannel
 *
 *
 * Description:     The INetworkSource interface allows allows a consumer to
 *                  receive and dispatch PDUs that arrive on a Network channel.
 *
 * Notes:
 *
 */
interface INetworkSource : public INetworkChannel
 {

//  Public Methods

public:

/*|><|************************************************************************
 Name:          CNetworkChannel::Receive

 Description:   A polymorphic method of IChannel that processes any receive
                events associated with a channel.  This version of receive
                shall handle inbound data on either a connectionless or
                connection-oriented channel.  Arriving data shall be
                dispatched to a Processing routine while various errors or
                channel conditions shall be reported and returned as errors.

 Returns:       int.
 ***********************************************************************|><|*/
    virtual int  Receive(unsigned char *puchBuffer,
                         unsigned long ulBufferSize)=0;

/*|><|************************************************************************
 Name:          Dispatch

 Description:   A method that takes a data transmission forwarded by the
                Receive() method and routes it to the appopriate receiver.

 Returns:       void.
 ***********************************************************************|><|*/
    virtual void  Dispatch(unsigned char * puchBuffer,
                           unsigned long ulBytesRecvd)=0;


};


#endif
