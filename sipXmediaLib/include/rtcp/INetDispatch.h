//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

//  Border Guard
#ifndef _INetDispatch_h
#define _INetDispatch_h

#include "rtcp/RtcpConfig.h"

// Include
#include "IBaseClass.h"

/**
 *
 * Interface Name:  INetDispatch
 *
 * Inheritance:     None
 *
 *
 * Description:     The INetDispatch interface allows a Network Source object
 *                  to forward data packets to a recipient object as they are
 *                  received on the network interface.
 *
 * Notes:
 *
 */
interface INetDispatch : public IBaseClass
 {

//  Public Methods

public:


/**
 *
 * Method Name: ProcessPacket
 *
 *
 * Inputs:   unsigned char *puchDataBuffer
 *             Data Buffer received from Network Source
 *           unsigned long ulBufferLength
 *             Length of Data Buffer
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The packet received is processed and dispatched after having
 *              been received from the Network Source object
 *
 * Usage Notes:
 *
 *
 */
    virtual void ProcessPacket(unsigned char *puchDataBuffer,
                               unsigned long ulBufferLength, int vbose=0) = 0;


};

#endif
