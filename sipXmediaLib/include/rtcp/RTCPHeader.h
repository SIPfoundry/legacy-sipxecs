//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


//  Border Guard
#ifndef _RTCPHeader_h
#define _RTCPHeader_h

#include "rtcp/RtcpConfig.h"

//  Includes
#include "BaseClass.h"
#include "IRTCPHeader.h"


// Defines
#define MAX_SOURCE_LENGTH   256 // Max Length of a NULL terminated SDES element
#define HEADER_LENGTH         8 // Header Size

#define PAYLOAD_OFFSET        1
#define LENGTH_OFFSET         2
#define SSRC_OFFSET           4
#define COUNT_MASK         0x1F


/**
 *
 * Class Name:  CRTCPHeader
 *
 * Inheritance: CBaseClass   - Base Class Implementation
 *
 *
 * Interfaces:  IRTCPHeader  - Services for loading and extracting RTCP header
 *                             info.
 *
 * Description: The CRTCPHeader Class manages the basic header and structure
 *              information associated with an RTCP report.
 *
 * Notes:       The CRTCPHeader is inherited by all RTCP Report objects.
 *
 */
class CRTCPHeader
{

//  Public Methods
public:

/**
 *
 * Method Name:  CRTCPHeader() - Constructor
 *
 *
 * Inputs:    unsigned long ulSSRC
 *                          - The the Identifier for this source
 *            RTCP_REPORTS_ET etPayloadType
 *                          - The Payload type associated with this report
 *            unsigned long ulVersion
 *                          - Version of the RFC Standard being followed
 *
 *
 * Outputs:   None
 *
 * Returns:   None
 *
 * Description:  The CRTCPHeader is an abstract class that is initialized by
 *               a derived object at construction time.
 *
 * Usage Notes:
 *
 */
    CRTCPHeader(unsigned long ulSSRC,
                RTCP_REPORTS_ET etPayloadType,
                unsigned long ulVersion=2);



/**
 *
 * Method Name: ~CRTCPHeader() - Destructor
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: Shall deallocated and/or release all resources which was
 *              acquired over the course of runtime.
 *
 * Usage Notes:
 *
 *
 */
    virtual ~CRTCPHeader(void);

/**
 *
 * Method Name:  GetHeaderLength
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long  - Returns the size of the RTCP Header
 *
 * Description: Retrieves the size of the RTCP Header that preceeds
 *              the payload.
 *
 * Usage Notes:
 *
 *
 */
    virtual unsigned long GetHeaderLength(void);

/**
 *
 * Method Name:  GetVersion
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long  - Returns the Version
 *
 * Description: Retrieves the Version attribute stored within the object.
 *
 * Usage Notes:
 *
 *
 */
    virtual unsigned long GetVersion(void);

/**
 *
 * Method Name:  GetPadding
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long  - Returns the Padding Flag
 *
 * Description: Retrieves the Padding attribute stored within the object.
 *
 * Usage Notes:
 *
 *
 */
    virtual unsigned long GetPadding(void);

/**
 *
 * Method Name:  GetPayload
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     RTCP_REPORTS_ET   - Returns the Payload Type
 *
 * Description: Retrieves the payload type associated with this RTCP report.
 *
 * Usage Notes:
 *
 *
 */
    virtual RTCP_REPORTS_ET GetPayload(void);


/**
 *
 * Method Name:  GetReportCount
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long   - Returns the Report Count
 *
 * Description: Retrieves the report count associated with this RTCP report.
 *
 * Usage Notes:
 *
 *
 */
    virtual unsigned long GetReportCount(void);


/**
 *
 * Method Name:  GetReportlength
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long   - Returns the Report Length
 *
 * Description: Retrieves the report length associated with this RTCP report.
 *
 * Usage Notes:
 *
 *
 */
    virtual unsigned long GetReportLength(void);

/**
 *
 * Method Name:  GetSSRC
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long  - Return the SSRC IDt
 *
 * Description: Retrieves the SSRC attribute stored within the object.
 *
 * Usage Notes:
 *
 *
 */
    virtual unsigned long GetSSRC(void);

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
 * Usage Notes:
 *
 *
 *
 */
    virtual void SetSSRC(unsigned long ulSSRC);

/**
 *
 * Method Name:  IsOurSSRC
 *
 *
 * Inputs:      None
 *
 * Outputs:     unsigned long ulSSRC  - SSRC ID
 *
 * Returns:     boolean
 *
 * Description: Compares the SSRC ID passed to that stored as an attribute
 *              within this object instance.  Will return either True or
 *              False based on the match.
 *
 * Usage Notes:
 *
 *
 */
    virtual bool IsOurSSRC(unsigned long ulSSRC);

protected:   // Protected Methods


/**
 *
 * Method Name:  FormatRTCPHeader
 *
 *
 * Inputs:  unsigned long ulPadding       - Padding used
 *          unsigned long ulCount         - Report Count
 *          unsigned long ulPayloadLength - Payload Length (Excluding Header)
 *
 * Outputs: unsigned char *puchRTCPBuffer
 *                            - Buffer used to store the RTCP Report Header
 *
 * Returns: unsigned long
 *          - Returns the number of octets written into the buffer.
 *
 * Description: Constructs an RTCP Report report using information stored
 *              and passed by the caller.
 *
 * Usage Notes: A buffer of sufficient size should be allocated and passed
 *              to this formatting method.
 *
 *
 */
    unsigned long FormatRTCPHeader(unsigned char *puchRTCPBuffer,
                                   unsigned long ulPadding,
                                   unsigned long ulCount,
                                   unsigned long ulPayloadLength);


/**
 *
 * Method Name:  ParseRTCPHeader
 *
 *
 * Inputs:      unsigned char *puchRTCPBuffer
 *         - Character Buffer containing the contents of the RTCP Report
 *
 * Outputs:     None
 *
 * Returns:     bool
 *
 * Description: Extracts the header contents of an RTCP report using the
 *              buffer passed in by the caller.  The header will be validated
 *              to determine whether it has an appropriate version, payload
 *              type, and SSRC for this object.
 *
 * Usage Notes:
 *
 *
 */
    bool ParseRTCPHeader(unsigned char *puchRTCPBuffer);



protected:      // Protected Data Members



/**
 *
 * Attribute Name:  m_ulVersion
 *
 * Type:            unsigned long
 *
 * Description:     The protocol version of the RTCP Report.
 *
 */
      unsigned long m_ulVersion;

/**
 *
 * Attribute Name:  m_ulPadding
 *
 * Type:            unsigned long
 *
 * Description: A flag identifying the use of padding within an RTCP report.
 *
 */
      unsigned long m_ulPadding;

/**
 *
 * Attribute Name:  m_ulCount
 *
 * Type:            unsigned long
 *
 * Description: The number of composite records contained with an RTCP report.
 *
 */
      unsigned long m_ulCount;


/**
 *
 * Attribute Name:  m_etPayloadType
 *
 * Type:            RTCP_REPORTS_ET
 *
 * Description:     The RTCP Payload type.
 *
 */
      RTCP_REPORTS_ET m_etPayloadType;

/**
 *
 * Attribute Name:  m_ulLength
 *
 * Type:            unsigned long
 *
 * Description:     The RTCP Report Length.
 *
 */
      unsigned long m_ulLength;

/**
 *
 * Attribute Name:  m_ulSSRC
 *
 * Type:            unsigned long
 *
 * Description:     This member shall store the SSRC ID of the associated
 *                  RTP connection.
 *
 */
      unsigned long m_ulSSRC;


};


#endif
