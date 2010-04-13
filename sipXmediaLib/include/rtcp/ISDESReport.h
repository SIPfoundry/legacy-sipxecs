//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


//  Border Guard
#ifndef _ISDESReport_h
#define _ISDESReport_h

#include "rtcp/RtcpConfig.h"

//  Includes
#include "IBaseClass.h"

//  Defines
#define SDES_CHANGES    -1

//  Forward Declarations
interface IGetSrcDescription;

/**
 *
 * Interface Name:  ISDESReport
 *
 * Inheritance:     None
 *
 *
 * Description: The ISDESReport interface allows consumers to format and parse
 *              SDES reports according to the template defined for the RTCP
 *              SDES payload format. Report elements supported include Name,
 *              Email, Phone, Location, Application Name, Notes, and Private
 *              fields.
 *
 * Notes:
 *
 */
interface ISDESReport : public IBaseClass
 {

//  Public Methods

public:

/**
 *
 * Method Name:  FormatSDESReport
 *
 *
 * Inputs:      boolean        bHeader -
 *                 TRUE indicates a header should be included
 *              long           lContentMask - Content Mask
 *              unsigned long  ulBufferSize -
 *                 Optional size defining the length allocated for the buffer
 *
 * Outputs:     unsigned char *puchReportBuffer -
 *                 Buffer used to store the contents of the SDES Report
 *
 * Returns:     unsigned long  -
 *                 Returns the number of octets written into the buffer.
 *
 * Description: Constructs an SDES report using the buffer passed in by the
 *              caller.  The Source Description object shall use the period
 *              count passed to determine which information should be used to
 *              populate an SDES report.
 *
 * Usage Notes: The header of the RTCP Report shall be formatted by delegating
 *              to the base class.
 *
 *
 */
    virtual unsigned long FormatSDESReport(bool bHeader, long lContentMask,
                                           unsigned char *puchReportBuffer,
                                           unsigned long ulBufferSize)=0;


/**
 *
 * Method Name:  ParseSDESReport
 *
 *
 * Inputs:      bool bHeader  -
 *                      TRUE indicates an RTCP Header preceeds the report
 *              unsigned char *puchReportBuffer -
 *                      Buffer containing the contents of the SDES Report
 *
 * Outputs:     None
 *
 * Returns:     unsigned long - Number of octet processed
 *
 * Description: Extracts the contents of an SDES report using the buffer passed
 *              in by the caller.  The Source Description object shall store
 *              the content and length of data fields extracted from the SDES
 *              Report.
 *
 * Usage Notes: The header of the RTCP Report shall be parsed by delegating to
 *              the base class.
 *
 *
 */
    virtual unsigned long ParseSDESReport(bool bHeader,
                                       unsigned char *puchReportBuffer) = 0;

/**
 *
 * Method Name:  SetSSRC
 *
 *
 * Inputs:      unsigned long  ulSSRC   - Source ID
 *
 * Outputs:     None
 *
 * Returns:     void
 *
 * Description: Stores the Source Identifier associated with an RTP connection.
 *
 * Usage Notes: This is an override of the base class method defined in
 *              CRTCPHeader.
 *
 *
 *
 */
    virtual void SetSSRC(unsigned long ulSSRC) = 0;

/**
 *
 * Method Name:  GetAccessInterface()
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     IGetSrcDescription * -
 *                       Pointer to the Get Src Description Interface
 *
 * Description: Returns a pointer to the IGetSrcDescription interface used to
 *              view the contents of an SDES Report.
 *
 * Usage Notes:
 *
 */
    virtual IGetSrcDescription * GetAccessInterface(void) = 0;

};

#endif
