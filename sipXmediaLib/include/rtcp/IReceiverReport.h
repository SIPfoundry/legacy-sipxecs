//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


//  Border Guard
#ifndef _IReceiverReport_h
#define _IReceiverReport_h

#include "rtcp/RtcpConfig.h"

// Include
#include "IBaseClass.h"

/**
 *
 * Interface Name:  IReceiverReport
 *
 * Inheritance:     None
 *
 *
 * Description:   The IReceiverReport interface allows consumers to parse,
 *                format, and otherwise control the interpretation and
 *                generation of RTCP Receiver Reports.
 *
 * Notes:
 *
 */
interface IReceiverReport : public IBaseClass
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
 *              CRTCPHeader.  This method shall additionally reset the
 *              receiver statistics associated with the RTP connection as
 *              mandated per standard.
 *
 */
    virtual void SetSSRC(unsigned long ulSSRC) = 0;


/**
 *
 * Method Name:  FormatReceiverReport
 *
 *
 * Inputs:   bool           bSRPresent
 *             Specifies whether a Sender Report is prepended
 *           unsigned long   ulBufferSize
 *             Optional size defining the length allocated for the buffer
 *
 * Outputs:  unsigned char *puchReportBuffer
 *             Character Buffer used to store the Receiver Report
 *
 * Returns:  unsigned long
 *             Returns the number of octets written into the buffer.
 *
 * Description: Constructs a Receiver report using the buffer passed in by
 *              the caller.  A Receiver Report may be appended to the contents
 *              of a Sender Report or sent along in the case where no data was
 *              transmitted during the reporting period.  The header flag set
 *              to True shall cause the Receiver Report to be appended while
 *              False will cause a header to be prepended to the Report
 *              information.
 *
 *              A call to this method shall cause all period counts to be reset.
 *
 * Usage Notes: The header of the RTCP Report shall be formatted by delegating
 *              to the base class.
 *
 *
 *
 */
    virtual unsigned long FormatReceiverReport(bool          bSRPresent,
                                               unsigned char *puchReportBuffer,
                                               unsigned long ulBufferSize)=0;


/**
 *
 * Method Name:  ParseReceiverReport
 *
 *
 * Inputs:   boolean         bHeader
 *             Specifies whether a header is included
 *           unsigned char  *puchReportBuffer
 *             Character Buffer used to store the Receiver Report
 *
 * Outputs:
 *
 * Returns:  unsigned long
 *             Returns the number of octets processed
 *
 * Description: Processes a Receiver report using the buffer passed in by the
 *              caller.  The header flag shall identify whether the report is
 *              prepended with a header.
 *
 * Usage Notes: The header of the RTCP Report, if provided, shall be parsed
 *              by delegating to the base class.
 *
 *
 */
    virtual unsigned long  ParseReceiverReport(bool bHeader,
                                    unsigned char *puchReportBuffer) = 0;

};

#endif
