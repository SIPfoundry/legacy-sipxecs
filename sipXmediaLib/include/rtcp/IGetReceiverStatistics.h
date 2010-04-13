//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


//  Border Guard
#ifndef _IGetReceiverStatistics_h
#define _IGetReceiverStatistics_h

#include "rtcp/RtcpConfig.h"

// Include
#include "IBaseClass.h"

/**
 *
 * Interface Name:  IGetReceiverStatistics
 *
 * Inheritance:     None
 *
 *
 * Description:     The IGetReceiverStatistics interface allows consumers to
 *                  retrieve the performance and continuity statistics of an
 *                  RTCP Receiver Report associated with an inbound or
 *                  outbound RTP connection.
 *
 * Notes:
 *
 */
interface IGetReceiverStatistics : public IBaseClass
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
 * Method Name:  GetReceiverStatistics
 *
 *
 * Inputs:   None
 *
 * Outputs:  unsigned long  *pulFractionalLoss
 *             Fractional Packet Loss
 *           unsigned long  *pulCumulativeLoss
 *             Cumulative Packet Loss
 *           unsigned long  *pulHighestSequenceNo
 *             Highest Sequence Number Received
 *           unsigned long  *pulInterarrivalJitter
 *             Interarrival Packet Variance
 *           unsigned long  *pulSRTimestamp
 *             Timestamp of last Sender Report received
 *           unsigned long  *pulPacketDelay
 *             Delay between last Sender Report Received and sending this
 *             report
 *
 * Returns:     void
 *
 * Description: Returns a number of receiver report statistics associated
 *              with an inbound or outbound RTP connection.
 *
 * Usage Notes:
 *
 *
 *
 */
virtual void GetReceiverStatistics(unsigned long   *pulFractionalLoss,
                                   unsigned long   *pulCumulativeLoss,
                                   unsigned long   *pulHighestSequenceNo,
                                   unsigned long   *pulInterarrivalJitter,
                                   unsigned long   *pulSRTimestamp,
                                   unsigned long   *pulPacketDelay) = 0;



};

#endif
