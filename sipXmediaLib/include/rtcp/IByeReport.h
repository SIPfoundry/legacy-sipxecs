//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////



//  Border Guard
#ifndef _IByeReport_h
#define _IByeReport_h

#include "rtcp/RtcpConfig.h"

// Include
#include "IBaseClass.h"

/**
 *
 * Interface Name:  IByeReport
 *
 * Inheritance:     None
 *
 *
 * Description:     The IByeReport interface allows consumers to parse, format,
 *                  and otherwise control the interpretation and generation of
 *                  RTCP Bye Reports.
 *
 * Notes:
 *
 */
interface IByeReport : IBaseClass
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
 * Method Name: SetCSRC
 *
 *
 * Inputs:      None
 *
 * Outputs:     unsigned long *paulCSRC
 *                Contributing Source Identifier(s) Array pointer
 *              unsigned long ulCSRCCount
 *                Number of Contributing Source Identifier(s)
 *              bool bNBO
 *                Flag indicating whether data is in NBO
 *
 * Returns:     void
 *
 * Description: Sets the contributing source values associated with the
 *              RTP packet.
 *
 * Usage Notes:
 *
 *
 */
    virtual void SetCSRC(unsigned long *paulCSRC, unsigned long ulCSRCCount,
        bool bNBO)=0;

/**
 *
 * Method Name:  SetReason
 *
 *
 * Inputs:      unsigned char  *puchName
 *                Reason Character String
 *              unsigned long   ulLength
 *                Optional Length of Reason argument passed
 *
 * Outputs:     None
 *
 * Returns:     void
 *
 * Description: Stores the Reason field and length, either specified of
 *              derived, as attributed within the object.
 *
 * Usage Notes: The Reason argument MUST be passed as a NULL terminated
 *              string or must contain a valid length argument. All text
 *              strings passed shall be truncated beyond the length of
 *              255 unsigned characters.
 *
 */
    virtual void SetReason(unsigned char *puchReason, unsigned long ulLength)=0;

/**
 *
 * Method Name:  FormatByeReport
 *
 *
 * Inputs:  unsigned long ulBufferSize
 *            Optional size defining the length allocated for the buffer
 *
 * Outputs: unsigned char *puchReportBuffer
 *            Character Buffer used to store the contents of the Sender Report
 *
 * Returns: unsigned long
 *            Returns the number of octet written into the buffer.
 *
 * Description: Constructs a Bye report using the buffer passed in by the
 *              caller.
 *
 * Usage Notes: The header of the RTCP Report shall be formatted by
 *              delegating to the base class.
 *
 *
 */
    virtual unsigned long FormatByeReport(unsigned char *puchReportBuffer,
                                          unsigned long ulBufferSize)=0;


/**
 *
 * Method Name:  ParseByeReport
 *
 *
 * Inputs:      unsigned char *puchReportBuffer
 *                Character Buffer containing the contents of the Bye Report
 *
 * Outputs:     None
 *
 * Returns:     unsigned long
 *
 * Description: Extracts the contents of an Bye report using the buffer
 *              passed in by the caller.
 *
 * Usage Notes: The header of the RTCP Report shall be parsed by delegating
 *              to the base class.
 *
 *
 */
    virtual unsigned long ParseByeReport(unsigned char *puchReportBuffer)=0;


};

#endif
