//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


//  Border Guard
#ifndef _IGetSenderStatistics_h
#define _IGetSenderStatistics_h

#include "rtcp/RtcpConfig.h"

// Include
#include "IBaseClass.h"

/**
 *
 * Interface Name:  IGetSenderStatistics
 *
 * Inheritance:     None
 *
 *
 * Description:     The IGetSenderStatistics interface allows consumers to
 *                  retrieve the cumulative packet and octet count of either
 *                  an inbound or outbound RTP connection.
 *
 * Notes:
 *
 */
interface IGetSenderStatistics : public IBaseClass
 {

//  Public Methods

public:

/**
 *
 * Method Name:  GetSSRC
 *
 *
 * Inputs:       None
 *
 *
 * Outputs:      None
 *
 * Returns:     unsigned long - The SSRC of the Bye Report
 *
 * Description: Returns the SSRC Associated with the Bye Report.
 *
 * Usage Notes:
 *
 *
 */
    virtual unsigned long GetSSRC(void)=0;

/**
 *
 * Method Name:  GetSenderStatistics
 *
 *
 * Inputs:      None
 *
 * Outputs:     unsigned long   *ulPacketCount   - Sender Packet Count
 *              unsigned long   *ulOctetCount    - Sender Octet Count
 *
 * Returns:     void
 *
 * Description: Returns the packet and octet counts stored as attributes.
 *
 * Usage Notes:
 *
 *
 *
 */
    virtual void GetSenderStatistics(unsigned long *ulPacketCount,
        unsigned long *ulOctetCount) = 0;

};

#endif
