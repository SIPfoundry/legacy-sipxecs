//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


//  Border Guard
#ifndef _IRTCPStatistics_h
#define _IRTCPStatistics_h

#include "rtcp/RtcpConfig.h"

//  Includes
#include "IBaseClass.h"
#include "IGetReceiverStatistics.h"
#include "IGetByeInfo.h"

/**
 *
 * Interface Name:  IRTCPStatistics
 *
 * Inheritance:     None
 *
 *
 * Description:     The IRTCPStatistics interface allows consumers to access
 *                  RTCP Sender and Receiver statistics associated with either
 *                  an outbound or inbound RTP connection.
 *
 * Notes:
 *
 */


interface IRTCPStatistics : public IBaseClass
 {

//  Public Methods
public:

/**
 *
 * Method Name:  GetStatistics
 *
 *
 * Inputs:       None
 *
 * Outputs:      IGetSrcDescription     **piGetSrcDescription
 *                  Source Description Interface Pointer
 *               IGetSenderStatistics   **piSenderStatistics
 *                  Sender Statistics Interface Pointer
 *               IGetReceiverStatistics **piReceiverStatistics
 *                  Receiver Statistics Interface Pointer
 *               IByeInfo               **piGetByeInfo
 *                  Interface for retrieving Bye Report Info
 *
 *
 * Returns:      None
 *
 * Description:  Returns pointers to the Sender,Receiver and Bye statistics
 *               interfaces upon request.
 *
 * Usage Notes:  This would be used by the QOS object or the RTCP Statistics
 *               object if a polling method were supported.  These objects
 *               could alternatively be informed of statistic changes via
 *               notification with the interfaces contained within the
 *               callback.
 *
 */
    void GetStatistics(IGetSrcDescription     **piGetSrcDescription,
                       IGetSenderStatistics   **piSenderStatistics,
                       IGetReceiverStatistics **piReceiverStatistics,
                       IGetByeInfo            **piGetByeInfo);



};

#endif
