//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

//  Border Guard
#ifndef _ByeReport_h
#define _ByeReport_h

#include "rtcp/RtcpConfig.h"

//  Includes
#include "RTCPHeader.h"
#include "IByeReport.h"
#include "IGetByeInfo.h"


#define MAX_CSRCS   64

/**
 *
 * Class Name:  CByeReport
 *
 * Inheritance: CRTCPHeader      - RTCP Report Header Base Class
 *
 *
 * Interfaces:  IByeReport       - RTCP Bye Report Interface
 *
 * Description: The CByeReport Class coordinates the processing and generation
 *              of RTCP Bye reports associated with either an inbound or
 *              outbound RTP connection.
 *
 * Notes:       CByeReport is derived from CBaseClass which provides basic
 *              Initialization and reference counting support.
 *
 */
class CByeReport : public CBaseClass,  // Inherits the CBaseClass Impl
                   public CRTCPHeader, // Inherits the CRTCPHeader impl
                   public IByeReport,  // Bye Report Control Interface
                   public IGetByeInfo  // Interface for retrieving Bye Info

 {

//  Public Methods
public:

/**
 *
 * Method Name:  CByeReport() - Constructor
 *
 *
 * Inputs:       unsigned long ulSSRC     - The the Identifier for this source
 *
 * Outputs:      None
 *
 * Returns:      None
 *
 * Description:  Performs routine CByeReport object initialization.
 *
 * Usage Notes:  A CByeReport object shall be created by the CRTCPRender with
 *               this constructor.  A CByeReport object will also be created
 *               be the CRTCPSource upon receipt of an RTCP Bye Report from
 *               a particiapting FE.
 *
 */
    CByeReport(unsigned long ulSSRC=0);



/**
 *
 * Method Name: ~CByeReport() - Destructor
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
    ~CByeReport(void);


/**
 *
 * Method Name:  FormatByeReport
 *
 *
 * Inputs:   unsigned long ulBufferSize
 *           Optional size defining the length allocated for the buffer
 *
 * Outputs:  unsigned char *puchReportBuffer
 *           Character Buffer used to store the contents of the Sender Report
 *
 * Returns:  unsigned long
 *           Returns the number of octet written into the buffer.
 *
 * Description: Constructs a Bye report using the buffer passed in by the
 *              caller.
 *
 * Usage Notes: The header of the RTCP Report shall be formatted by delegating
 *              to the base class.
 *
 *
 */
    unsigned long FormatByeReport(unsigned char *puchReportBuffer,
        unsigned long ulBufferSize=0);


/**
 *
 * Method Name:  ParseByeReport
 *
 *
 * Inputs:      unsigned char *puchReportBuffer
 *              Character Buffer containing the contents of the Bye Report
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
    unsigned long ParseByeReport(unsigned char *puchReportBuffer);

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
    unsigned long GetSSRC(void);

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
 * Method Name: GetCSRC
 *
 *
 * Inputs:   bool bNBO
 *           Flag identifying whether data should be represented in NBO format

 *
 * Outputs:  unsigned long *paulCSRC
 *           Contributing Source Identifier(s) Array pointer
 *
 * Returns:  unsigned long
 *           Number of Contributing Source Identifier(s)
 *
 * Description: Returns the contributing source values associated with the
 *              RTP packet.
 *
 * Usage Notes:
 *
 *
 */
    unsigned long GetCSRC(unsigned long *paulCSRC, bool bNBO=FALSE);

/**
 *
 * Method Name: SetCSRC
 *
 *
 * Inputs:    unsigned long *paulCSRC
 *              - Contributing Source Identifier(s) Array pointer
 *            unsigned long ulCSRCCount
 *              - Number of Contributing Source Identifier(s)
 *            bool bNBO - TRUE indicates data is in NBO.
 *
 * Outputs:   None
 *
 * Returns:   void
 *
 * Description: Sets the contributing source values associated with the RTP
 *              packet.
 *
 * Usage Notes:
 *
 *
 */
    void SetCSRC(unsigned long *paulCSRC, unsigned long ulCSRCCount,
        bool bNBO=FALSE);


/**
 *
 * Method Name:  SetReason
 *
 *
 * Inputs:    char *puchName
 *              Reason Character String
 *            unsigned long ulLength
 *              Optional Length of Reason argument passed
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
    void SetReason(unsigned char *puchReason, unsigned long ulLength=0);


/**
 *
 * Method Name:  GetReason
 *
 *
 * Inputs:  None
 *
 * Outputs: char *puchReason
 *          Character buffer in which the Reason attribute shall be returned
 *
 * Returns: unsigned long
 *          Length of the item being returned in the unsigned character buffer
 *
 * Description: Retrieves the Reason attribute stored within the object and
 *              returns its associated length.
 *
 * Usage Notes: All unsigned character strings returned are limited to 255
 *              bytes in length. Any wide unsigned character support for
 *              internationalized display is a responsibility assumed by
 *              the caller.
 *
 *
 */
    unsigned long GetReason(unsigned char *puchReason);

/**
 *
 * Method Name:  GetByeInterface()
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     IByeReport *  - Pointer to the Bye Report Interface
 *
 * Description: Returns the Bye Report interface.
 *
 * Usage Notes:
 *
 */
    IByeReport * GetByeInterface(void);

/**
 *
 * Macro Name:  DECLARE_IBASE_M
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: This implements the IBaseClass functions used and exposed by
 *              derived classes.
 *
 * Usage Notes:
 *
 *
 */
DECLARE_IBASE_M

private:        // Private Member Functions

/**
 *
 * Method Name:  ExtractPadding
 *
 *
 * Inputs:      unsigned char *puchReportBuffer
 *                Character Buffer containing the contents of the Bye Report
 *
 * Outputs:     None
 *
 * Returns:     unsigned long - Number of octets processed
 *
 * Description: Extracts the padding that might be present at the end of a
 *              list of field data contained within an Bye report.
 *
 * Usage Notes:
 *
 *
 */
    unsigned long ExtractPadding(unsigned char *puchReportBuffer);

/**
 *
 * Method Name:  LoadPadding
 *
 *
 * Inputs:      unsigned char *puchReportBuffer
 *                Character Buffer containing the contents of the Bye Report
 *
 * Outputs:     bool &pbPadded
 *                Flag specifying whether padding was added
 *
 * Returns:     unsigned long - Number of octets processed
 *
 * Description: Pad out to a 4 byte boundary as needed.
 *
 * Usage Notes:
 *
 *
 */
    unsigned long LoadPadding(unsigned char *puchReportBuffer, bool *pbPadded);



private:        // Private Data Members


/**
 *
 * Attribute Name:  m_ulReasonLength
 *
 * Type:            unsigned long
 *
 * Description:     This member shall store the length of the textual reason
 *                  for RTP session termination.
 *
 */
      unsigned long m_ulReasonLength;


/**
 *
 * Attribute Name:  m_uchReason
 *
 * Type:            Character Array
 *
 * Description:     This member shall store the textual reason for an RTP
 *                  session termination.
 *
 */
      unsigned char m_uchReason[MAX_SOURCE_LENGTH];

/**
 *
 * Attribute Name:  m_ulCSRCCount
 *
 * Type:            unsigned long
 *
 * Description:     The number of CSRCs contained with an RTP report.
 *
 */
      unsigned long m_ulCSRCCount;

/**
 *
 * Attribute Name:  m_aulCSRC
 *
 * Type:            unsigned long
 *
 * Description:     The CSRCs contained with an RTP report.
 *
 */
      unsigned long m_aulCSRC[MAX_CSRCS];

};

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
inline unsigned long CByeReport::GetSSRC(void)
{

    return(CRTCPHeader::GetSSRC());
}

#endif
