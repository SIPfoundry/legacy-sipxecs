//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


//  Border Guard
#ifndef _ISetSenderStatistics_h
#define _ISetSenderStatistics_h

#include "rtcp/RtcpConfig.h"

//  Includes
#include "IBaseClass.h"


//  Defines
#define SAMPLES_PER_SEC     8000       // Default samples per second

/**
 *
 * Interface Name:  ISetSenderStatistics
 *
 * Inheritance:     None
 *
 *
 * Description:     The ISetSenderStatistics interface allows consumers to
 *                  increment the cumulative packet and octet count of either
 *                  an inbound or outbound RTP connection.
 *
 * Notes:
 *
 */
interface ISetSenderStatistics : public IBaseClass
 {

//  Public Methods

public:

/**
 *
 * Method Name:  IncrementCounts
 *
 *
 * Inputs:       unsigned long  ulOctetCount    -   RTP Octets Sent
 *
 *
 * Outputs:      None
 *
 * Returns:      void
 *
 * Description:  The IncrementCounts method shall add the number of octets
 *               passed to the cumulative octet count stored as an attribute
 *               to this object. Each call to IncrementCounts() shall also
 *               increment the packet count by 1.
 *
 * Usage Notes:
 *
 */
    virtual void IncrementCounts(unsigned long ulOctetCount) = 0;

/**
 *
 * Method Name:  SetRTPTimestamp
 *
 *
 * Inputs:       unsigned long ulRandomOffset -
 *                                  Random Offset for RTP Timestamp
 *               unsigned long ulSamplesPerSecond  -
 *                                  Number of sample per second
 *
 * Outputs:      None
 *
 * Returns:      void
 *
 * Description:  The SetRTPTimestamp method shall initialized values that are
 *               used to determine the RTP Timestamp value to be sent out in
 *               an SR Report.
 *
 * Usage Notes:
 *
 */
    virtual void SetRTPTimestamp(unsigned long ulRandomOffset,
                     unsigned long ulSamplesPerSecond = SAMPLES_PER_SEC) = 0;

};

#endif
