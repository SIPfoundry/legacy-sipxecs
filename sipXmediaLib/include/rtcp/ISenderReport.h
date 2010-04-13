//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


//  Border Guard
#ifndef _ISenderReport_h
#define _ISenderReport_h

#include "rtcp/RtcpConfig.h"

//  Includes
#include "IBaseClass.h"

/**
 *
 * Interface Name:  ISenderReport
 *
 * Inheritance:     None
 *
 *
 * Description:     The ISenderReport interface allows consumers to parse,
 *                  format, and otherwise control the interpretation and
 *                  generation of RTCP Sender Reports.
 *
 * Notes:
 *
 */
interface ISenderReport  : public IBaseClass
 {

//  Public Methods

public:


/**
 *
 * Method Name:  SetSSRC
 *
 *
 * Inputs:      unsigned long   ulSSRC   - Source ID
 *
 * Outputs:     None
 *
 * Returns:     void
 *
 * Description: Stores the Source Identifier associated with an RTP connection.
 *
 * Usage Notes: This is an override of the base class method defined in
 *              CRTCPHeader.  This method shall additionally reset the octet
 *              and packet count accumulators as mandated by standard.
 *
 *
 *
 */
    virtual void SetSSRC(unsigned long ulSSRC) = 0;

/**
 *
 * Method Name:  WasMediaSent
 *
 *
 * Inputs:       None
 *
 * Outputs:      None
 *
 * Returns:      bool
 *
 * Description:  A method to determine whether media has been sent out since
 *               the last reporting period.  This will determine whether a
 *               Sender Report or Receiver Report is in order.
 *
 * Usage Notes:
 *
 */
    virtual bool WasMediaSent(void)=0;


/**
 *
 * Method Name:  FormatSenderReport
 *
 *
 * Inputs:      unsigned long ulBufferSize -
 *                 Optional size defining the length allocated for the buffer
 *
 * Outputs:     unsigned char *puchReportBuffer -
 *                 Buffer used to store the contents of the Sender Report
 *
 * Returns:     unsigned long -
 *                 Returns the number of octets written into the buffer.
 *
 * Description: Constructs a Sender report using the buffer passed in by the
 *              caller.  The Sender Report object shall keep track of the
 *              reporting periods that have passed an which information
 *              should be used to populate the report.
 *
 * Usage Notes: The header of the RTCP Report shall be formatted by delegating
 *              to the base class.
 *
 *
 */
    virtual unsigned long FormatSenderReport(unsigned char *puchReportBuffer,
                                             unsigned long ulBufferSize) = 0;


/**
 *
 * Method Name:  ParseSenderReport
 *
 *
 * Inputs:      unsigned char *puchReportBuffer -
 *                 Buffer containing the contents of the Sender Report
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: Extracts the contents of an Sender report using the buffer
 *              passed in by the caller.  The Sender Report object shall store
 *              the content and length of data fields extracted from the
 *              Sender Report.
 *
 * Usage Notes: The header of the RTCP Report shall be parsed by delegating to
 *              the base class.
 *
 *
 */
    virtual unsigned long ParseSenderReport(unsigned char *puchReportBuffer)=0;

};

#endif
