//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


//  Includes
#include "rtcp/ByeReport.h"
#ifdef __pingtel_on_posix__
#include <netinet/in.h>
#endif

#ifdef INCLUDE_RTCP /* [ */

/**
 *
 * Method Name:  CByeReport() - Constructor
 *
 *
 * Inputs:       unsigned long ulSSRC - The Identifier for this source
 *
 * Outputs:      None
 *
 * Returns:      None
 *
 * Description:  Performs routine CByeReport object initialization.
 *
 * Usage Notes:  A CByeReport object shall be created by the CRTCPRender with
 *               this constructor.  A CByeReport object will also be created
 *               be the CRTCPSource upon receipt of an RTCP Bye Report from a
 *               participating FE.
 *
 */
CByeReport::CByeReport(unsigned long ulSSRC)
           :CRTCPHeader(ulSSRC, etByeReport),  // Base class construction
            m_ulReasonLength(0), m_ulCSRCCount(0)
{

//  Nothing to do

}


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
 * Description: Shall deallocate and/or release all resources which were
 *              acquired over the course of runtime.
 *
 * Usage Notes:
 *
 *
 */
CByeReport::~CByeReport(void)
{

//  Our reference count must have gone to 0 to get here.  We have not
//  allocated any memory so we shall now go quietly into that good night!

}


/**
 *
 * Method Name:  FormatByeReport
 *
 *
 * Inputs:      unsigned long  ulBufferSize - length allocated for the buffer
 *
 * Outputs:     unsigned char *puchReportBuffer
 *                       - Buffer to receive the contents of the Sender Report
 *
 * Returns:     unsigned long  - number of octets written into the buffer.
 *
 * Description: Constructs a Bye report using the buffer passed in by the
 *              caller.
 *
 * Usage Notes: The header of the RTCP Report shall be formatted by
 *              delegating to the base class.
 *
 *
 */
unsigned long CByeReport::FormatByeReport(unsigned char *puchReportBuffer,
                                          unsigned long ulBufferSize)
{
    unsigned long  ulReportLength, ulCSRCCount = 0;
    unsigned char *puchPayloadBuffer;

    // Let's offset into the Formatting buffer enough to
    // start depositing payload
    puchPayloadBuffer = puchReportBuffer + GetHeaderLength();

    // Let's load the field information based upon the period.
    // Conversion to NBO done in GetCSRC().
    ulCSRCCount = GetCSRC((unsigned long *)puchPayloadBuffer, TRUE);
    puchPayloadBuffer += (ulCSRCCount * sizeof(long));

    // Let's load the field information based upon the period
    unsigned long ulReasonLength = GetReason(puchPayloadBuffer+1);
    bool bPadded = FALSE;
    if(ulReasonLength > 0)
    {
        // Adjust the count and payload pointer
        *puchPayloadBuffer++ = (unsigned char)ulReasonLength;
        puchPayloadBuffer += ulReasonLength;

        // Let's load padding onto the end of the packet to
        // ensure 4 byte alignment
        puchPayloadBuffer += LoadPadding(puchPayloadBuffer, &bPadded);
    }

    // Set the report length
    ulReportLength = puchPayloadBuffer - puchReportBuffer;

    // Let's call the RTCP Header base class's formatter so we can prepend
    // a header to this Bye Payload
    FormatRTCPHeader(puchReportBuffer,      // RTCP Report Buffer
                     bPadded,               // Padding Flag
                     ulCSRCCount+1,         // SSRC/CSRC Count
                     ulReportLength);       // Report Length

    return(ulReportLength);
}


/**
 *
 * Method Name:  ParseByeReport
 *
 *
 * Inputs:   unsigned char *puchReportBuffer - Buffer containing the Bye Report
 *
 * Outputs:  None
 *
 * Returns:  unsigned long
 *
 * Description: Extracts the contents of an Bye report using the buffer
 *              passed in by the caller.
 *
 * Usage Notes: The header of the RTCP Report shall be parsed by delegating
 *              to the base class.
 *
 *
 */
unsigned long CByeReport::ParseByeReport(unsigned char *puchReportBuffer)
{

    unsigned char    *puchPayloadBuffer = puchReportBuffer;

    // Check whether the RTCP Header has been correctly
    //   formed (Version, etc...).
    if(!ParseRTCPHeader(puchReportBuffer))
        return(GetReportLength());

    // Good header.  Let's bump the payload pointer and continue.
    puchPayloadBuffer += GetHeaderLength();


    // Let's store the CSRCs from the Bye Report
    unsigned long ulCSRCCount = GetReportCount() ? GetReportCount() - 1 : 0;

    SetCSRC((unsigned long *)puchPayloadBuffer, ulCSRCCount, TRUE);
    puchPayloadBuffer += (sizeof(unsigned long) * ulCSRCCount);

    // Let's determine whether there is an optional Reason field associated
    // with this Bye Report.  We can surmise this through comparing the
    // packet length to what we've already processed

    if(puchPayloadBuffer - puchReportBuffer < (long)GetReportLength())
    {
        unsigned long ulReasonLength = (unsigned long)*puchPayloadBuffer++;
        SetReason(puchPayloadBuffer, ulReasonLength);
        puchPayloadBuffer += ulReasonLength;

    }

    // Let's process any padding that might be present to align the
    // payload on a 32 bit boundary.
    if(GetPadding())
        puchPayloadBuffer += ExtractPadding(puchPayloadBuffer);

    return(puchPayloadBuffer - puchReportBuffer);

}



/**
 *
 * Method Name:  SetReason
 *
 *
 * Inputs:      unsigned char  *puchName   - Reason Character String
 *              unsigned long   ulLength   - Length of Reason argument passed
 *
 * Outputs:     None
 *
 * Returns:     void
 *
 * Description: Stores the Reason field and length, either specified of
 *              derived, as attributed within the object.
 *
 * Usage Notes: The Reason argument MUST be passed as a NULL terminated string
 *              or must contain a valid length argument.  All text strings
 *              passed shall be truncated beyond the length of 255 unsigned
 *              characters.
 *
 *
 *
 */
void CByeReport::SetReason(unsigned char *puchReason, unsigned long ulLength)
{

    // Check whether a valid length was passed
    if(ulLength)
    {
        // Make sure length is less than MAX_SOURCE_LENGTH
        m_ulReasonLength = (ulLength >= MAX_SOURCE_LENGTH) ?
                                            MAX_SOURCE_LENGTH - 1 : ulLength;
        strncpy((char *)m_uchReason, (char *)puchReason, m_ulReasonLength);
        m_uchReason[m_ulReasonLength] = 0; //NULL
    }
    else if(puchReason != NULL)
    {
        // Assume NULL termination and do a straight string copy
        strcpy((char *)m_uchReason, (char *)puchReason);
        m_ulReasonLength = strlen((char *)puchReason);
    }

}


/**
 *
 * Method Name:  GetReason
 *
 *
 * Inputs:   None
 *
 * Outputs:  unsigned char *puchReason - buffer to receive the Reason attribute
 *
 * Returns:  unsigned long - Length of the item returned in the buffer
 *
 * Description: Retrieves the Reason attribute stored within the object and
 *              returns its length.
 *
 * Usage Notes: All unsigned character strings returned are limited to 255
 *              bytes in length.  Any wide unsigned character support for
 *              internationalized display is a responsibility assumed by the
 *              caller.
 *
 *
 */
unsigned long CByeReport::GetReason(unsigned char *puchReason)
{

    // Copy the attribute contents into the output buffer passed
    strcpy((char *)puchReason, (char *)m_uchReason);

    return(m_ulReasonLength);

}


/**
 *
 * Method Name: GetCSRC
 *
 *
 * Inputs:   bool bNBO
 *                   - TRUE indicates data should be represented in NBO format
 *
 * Outputs:  unsigned long *paulCSRC
 *                           - Contributing Source Identifier(s) Array pointer
 *
 * Returns:  unsigned long - Number of elements loaded
 *
 * Description: Returns the contributing source values associated
 *              with the Bye packet.
 *
 * Usage Notes:
 *
 *
 */
unsigned long CByeReport::GetCSRC(unsigned long *paulCSRC, bool bNBO)
{

    // Loop through the list of CSRCs
    for(unsigned long ulCount = 0; ulCount < m_ulCSRCCount; ulCount++)
    {
        if(bNBO)
            *(paulCSRC + ulCount) = htonl(m_aulCSRC[ulCount]);
        else
            *(paulCSRC + ulCount) = m_aulCSRC[ulCount];

    }

    // Set Count
    return(m_ulCSRCCount);
}

/**
 *
 * Method Name: SetCSRC
 *
 *
 * Inputs:   unsigned long *paulCSRC
 *                           - Contributing Source Identifier(s) Array pointer
 *           unsigned long ulCSRCCount
 *                           - Number of Contributing Source Identifier(s)
 *           bool bNBO - TRUE indicates data is in NBO format
 *
 * Outputs:  None
 *
 * Returns:  void
 *
 * Description: Sets the contributing source values associated
 *              with the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
void CByeReport::SetCSRC(unsigned long *paulCSRC,
                         unsigned long ulCSRCCount, bool bNBO)
{

    // Loop through the list of CSRCs
    for(unsigned long ulCount = 0; ulCount < ulCSRCCount; ulCount++)
    {
        if(bNBO)
            m_aulCSRC[ulCount] = ntohl(*(paulCSRC + ulCount));
        else
            m_aulCSRC[ulCount] = *(paulCSRC + ulCount);
    }

    // Set Count
    m_ulCSRCCount = ulCSRCCount;

}

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
IByeReport * CByeReport::GetByeInterface(void)
{

    ((IByeReport *)this)->AddRef();
    return((IByeReport *)this);

}

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
void CByeReport::SetSSRC(unsigned long ulSSRC)
{

    // Store the modified SSRC as an internal attribute
    CRTCPHeader::SetSSRC(ulSSRC);

}


/**
 *
 * Method Name:  ExtractPadding
 *
 *
 * Inputs:   unsigned char *puchReportBuffer - Buffer containing the Bye Report
 *
 * Outputs:  None
 *
 * Returns:  unsigned long - Number of octets processed
 *
 * Description: Extracts the padding that might be present at the end of a
 *              list of field data contained within an Bye report.
 *
 * Usage Notes:
 *
 *
 */
unsigned long CByeReport::ExtractPadding(unsigned char *puchReportBuffer)
{
    unsigned char *puchPayloadBuffer = puchReportBuffer;

    // The last entry at the end of the list will be padded out to
    //  a 4 byte boundary
    while(((unsigned long)puchPayloadBuffer) % 4)
        puchPayloadBuffer++;

    return(puchPayloadBuffer - puchReportBuffer);

}

/**
 *
 * Method Name:  LoadPadding
 *
 *
 * Inputs:   unsigned char *puchReportBuffer - Buffer containing the Bye Report
 *
 * Outputs:  bool &pbPadded                - TRUE indicates padding was added
 *
 * Returns:  unsigned long - Number of octets processed
 *
 * Description: Pad out to a 4 byte boundary as needed.
 *
 * Usage Notes:
 *
 *
 */
unsigned long CByeReport::LoadPadding(unsigned char *puchReportBuffer,
                                      bool *pbPadded)
{
    unsigned char *puchPayloadBuffer = puchReportBuffer;

    // Initialize the Padding Flag
    *pbPadded = FALSE;

    // Add padding as needed to get us aligned on a 4 byte boundary
    while(((unsigned long)puchPayloadBuffer) % 4)
    {
        *puchPayloadBuffer++ = 0; //NULL
        *pbPadded = TRUE;
    }

    return(puchPayloadBuffer - puchReportBuffer);

}


#endif /* INCLUDE_RTCP ] */
