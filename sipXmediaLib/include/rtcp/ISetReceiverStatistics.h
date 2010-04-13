//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


//  Border Guard
#ifndef _ISetReceiverStatistics_h
#define _ISetReceiverStatistics_h

#include "rtcp/RtcpConfig.h"

//  Includes
#include "IRTPHeader.h"
#include "IBaseClass.h"

/**
 *
 * Interface Name:  ISetReceiverStatistics
 *
 * Inheritance:     None
 *
 *
 * Description:     The ISetReceiverStatistics interface allows the RTP Source
 *                  and RTP inbound Sender Reports to provide information
 *                  necessary in calculating RTCP Receiver Report statistics
 *                  associated within an inbound RTP connection.
 *
 * Notes:
 *
 */
interface ISetReceiverStatistics  : public IBaseClass
 {

//  Public Methods

public:

/**
 *
 * Method Name:  SetRTPStatistics
 *
 *
 * Inputs:      CRTPHeader *poRTPHeader -
 *                              RTP Packet Header received from RTP Source
 *
 * Outputs:     None
 *
 * Returns:     void
 *
 * Description: Takes the CRTPHeader object passed by the RTP Source object and
 *              updates Receiver Report statistics based upon its contents.
 *
 * Usage Notes:
 *
 *
 *
 */
    virtual void SetRTPStatistics(IRTPHeader *piRTPHeader) = 0;


/**
 *
 * Method Name: SetLastRcvdSRTime
 *
 *
 * Inputs:      unsigned long aulNTPTimestamp[]
 *
 * Outputs:     None
 *
 * Returns:     void
 *
 * Description: Store the RTP timestamp from the last Sender Report received
 *              and store the time that this report was received on the system.
 *
 * Usage Notes:
 *
 *
 *
 */
    virtual void SetLastRcvdSRTime(unsigned long aulNTPTimestamp[]) = 0;

};


#endif
