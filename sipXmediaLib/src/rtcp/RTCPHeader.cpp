//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


    // Includes
#include "rtcp/RTCPHeader.h"
#ifdef INCLUDE_RTCP /* [ */

#ifdef __pingtel_on_posix__
#include <netinet/in.h>
#endif

    // Constants
const int   PAD_MASK            = 0x20;
const int   VERSION_MASK        = 0xC0;

const int   PAD_SHIFT           = 5;
const int   VERSION_SHIFT       = 6;

/**
 *
 * Method Name:  CRTCPHeader() - Constructor
 *
 *
 * Inputs:    unsigned long ulSSRC
 *                              - The the IDentifier for this source
 *            unsigned long ulVersion
 *                              - Version of the RFC Standard being followed
 *            unsigned long ulPayload
 *                              - The Payload type associated with this report
 *
 * Outputs:   None
 *
 * Returns:   None
 *
 * Description:  The CRTCPHeader is an abstract class that is initialized by a
 *               derived object at construction time.
 *
 * Usage Notes:
 *
 */
CRTCPHeader::CRTCPHeader(unsigned long ulSSRC, RTCP_REPORTS_ET etPayloadType,
                         unsigned long ulVersion)
            : m_ulPadding(FALSE), m_ulCount(0), m_ulLength(0)
{

    // Assign initial values to attributes
    m_ulSSRC        = ulSSRC;
    m_ulVersion     = ulVersion;
    m_etPayloadType = etPayloadType;

}

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
 *               acquired over the course of runtime.
 *
 * Usage Notes:
 *
 *
 */
CRTCPHeader::~CRTCPHeader(void)
{
// Our reference count must have gone to 0 to get here.  We have not allocated
// any memory so we shall now go quietly into that good night!
}



/**
 *
 * Method Name:  GetHeaderLength
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long  - the size of the RTCP Header
 *
 * Description: Returns the size of the RTCP Header that preceeds the payload.
 *
 * Usage Notes:
 *
 *
 */
unsigned long CRTCPHeader::GetHeaderLength(void)
{

    // Load the argument passed with the header length
    return(HEADER_LENGTH);

}

/**
 *
 * Method Name:  GetSSRC
 *
 *
 * Inputs:      None
 *
 * Outputs:     Bobe
 *
 * Returns:     unsigned long  - Return the SSRC ID
 *
 * Description: Retrieves the SSRC attribute stored within the object.
 *
 * Usage Notes:
 *
 *
 */
unsigned long CRTCPHeader::GetSSRC(void)
{

    // Return the SSRC in the argument passed
    return(m_ulSSRC);

}


/**
 *
 * Method Name: GetVersion
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long  - Protocol Version #
 *
 * Description: Returns the protocol version number from the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
unsigned long CRTCPHeader::GetVersion(void)
{

    // Return Version Number
    return(m_ulVersion);

}

/**
 *
 * Method Name: GetPadding
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long  - Padding Flag
 *
 * Description: Returns the padding flag from the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
unsigned long CRTCPHeader::GetPadding(void)
{

    // Return Padding Flag
    return(m_ulPadding);

}


/**
 *
 * Method Name:  GetReportCount
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long - Returns Report Count
 *
 * Description: Retrieves the report count associated with this RTCP report.
 *
 * Usage Notes:
 *
 *
 */
unsigned long CRTCPHeader::GetReportCount(void)
{

    // Return Report Count
    return(m_ulCount);

}


/**
 *
 * Method Name:  GetReportlength
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long - Returns Report Length
 *
 * Description: Retrieves the report length associated with this RTCP report.
 *
 * Usage Notes:
 *
 *
 */
unsigned long CRTCPHeader::GetReportLength(void)
{

    // Return Report Length
    return(m_ulLength ? m_ulLength + sizeof(long) : m_ulLength);

}


/**
 *
 * Method Name: GetPayload
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     RTCP_REPORTS_ET - Returns Payload Type
 *
 * Description: Returns the payload type value from the RTCP packet.
 *
 * Usage Notes:
 *
 *
 */
RTCP_REPORTS_ET CRTCPHeader::GetPayload(void)
{

    // Return Payload Type
    return(m_etPayloadType);

}

/**
 *
 * Method Name:  IsOurSSRC
 *
 *
 * Inputs:      None
 *
 * Outputs:     unsigned long ulSSRC  - SSRC ID
 *
 * Returns:     boolean - TRUE => match
 *
 * Description: Compares the SSRC ID passed to that stored as an attribute
 *              within this object instance.  Will return either True or False
 *              based on the match.
 *
 * Usage Notes:
 *
 *
 */
bool CRTCPHeader::IsOurSSRC(unsigned long ulSSRC)
{

    // Compare the SSRC passed to the one that we have stored.
    return(ulSSRC == m_ulSSRC);

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
void CRTCPHeader::SetSSRC(unsigned long ulSSRC)
{

    // Store the modified SSRC as an internal attribute
    m_ulSSRC    = ulSSRC;

}

/**
 *
 * Method Name:  FormatRTCPHeader
 *
 *
 * Inputs:      unsigned long ulPadding         - Padding used
 *              unsigned long ulCount           - Report Count
 *              unsigned long ulReportLength    - Report Length
 *
 * Outputs:     unsigned char *puchRTCPBuffer
 *                               - Buffer used to store the RTCP Report Header
 *
 * Returns:     unsigned long  - number of octets written into the buffer.
 *
 * Description: Constructs an RTCP Report report using information stored and
 *              passed by the caller.
 *
 * Usage Notes: A buffer of sufficient size should be allocated and passed to
 *              this formatting method.
 *
 *
 */
unsigned long CRTCPHeader::FormatRTCPHeader(unsigned char *puchRTCPBuffer,
                                            unsigned long ulPadding,
                                            unsigned long ulCount,
                                            unsigned long ulReportLength)
{

    unsigned char *puchRTCPHeader = puchRTCPBuffer;

    // Report Count goes into the bits 4 - 8 of the first octet
    m_ulCount = ulCount;
    *puchRTCPHeader  = (unsigned char)ulCount;

    // Padding flag goes into the third bit of the first octet
    m_ulPadding = ulPadding;
    *puchRTCPHeader |= (unsigned char)((ulPadding << PAD_SHIFT) & PAD_MASK);

    // Version # goes into the first 2 bits of the first octet
    *puchRTCPHeader++ |=
               (unsigned char)((m_ulVersion << VERSION_SHIFT) & VERSION_MASK);

    // Payload Type goes into the second octet
    *puchRTCPHeader++ = (unsigned char)m_etPayloadType;

    // RTCP Report length goes into the third and fourth octet.  This length
    //  is expressed in long words.
    m_ulLength = ulReportLength;
    *((unsigned short *)puchRTCPHeader) =
     htons((((unsigned short)ulReportLength) / sizeof(long)) - 1);
    puchRTCPHeader += sizeof(short);

    // SSRC goes into the next 4 octet
    *((unsigned long *)puchRTCPHeader) = htonl(m_ulSSRC);

    return(puchRTCPBuffer - puchRTCPBuffer);

}


/**
 *
 * Method Name:  ParseRTCPHeader
 *
 *
 * Inputs:   unsigned char *puchRTCPBuffer - Buffer containing the RTCP Report
 *
 * Outputs:  None
 *
 * Returns:  bool
 *
 * Description:  Extracts the header contents of an RTCP report using the
 *               buffer passed in by the caller.  The header will be validated
 *               to determine whether it has an appropriate version, payload
 *               type, and SSRC for this object.
 *
 * Usage Notes:
 *
 *
 */
bool CRTCPHeader::ParseRTCPHeader(unsigned char *puchRTCPBuffer)
{

    unsigned char *puchRTCPHeader = puchRTCPBuffer;

    // Extract Report Count
    m_ulCount = *puchRTCPHeader & COUNT_MASK;

    // Extract Padding
    m_ulPadding = ((*puchRTCPHeader & PAD_MASK) >> PAD_SHIFT);

    // Check for valid Version #
    if (((*puchRTCPHeader++ & VERSION_MASK) >> VERSION_SHIFT) !=
                                                            (char)m_ulVersion)
    {
        osPrintf("**** FAILURE **** CRTCPHeader::ParseRTCPHeader() -"
                                                        " Invalid Version\n");
        return(FALSE);
    }

    // Check for valid Payload Type
    if(*puchRTCPHeader++ != (unsigned char)m_etPayloadType)
    {
        osPrintf("**** FAILURE **** CRTCPHeader::ParseRTCPHeader() -"
                                                   " Invalid Payload Type\n");
        return(FALSE);
    }

    // Extract RTCP Report length and convert from word count to byte count
    m_ulLength = ntohs(*((unsigned short *)puchRTCPHeader)) + 1;
    m_ulLength *= sizeof(long);
    puchRTCPHeader += sizeof(short);

    // Assign SSRC if one hadn't previously existed
    if(m_ulSSRC == 0)
        m_ulSSRC = ntohl(*((unsigned long *)puchRTCPHeader));

    // Check SSRC to be sure that the one received corresponds with the one
    //  previously established.
    else if(ntohl(*((unsigned long *)puchRTCPHeader)) != m_ulSSRC)
    {
#if RTCP_DEBUG /* [ */
        if(bPingtelDebug)
        {
            osPrintf(">>>>> CRTCPHeader::ParseRTCPHeader() -"
                                                 " SSRC has Changed <<<<<\n");
        }
#endif /* RTCP_DEBUG ] */
        m_ulSSRC = ntohl(*((unsigned long *)puchRTCPHeader));

    }
    puchRTCPHeader += sizeof(long);

    return(TRUE);

}

#endif /* INCLUDE_RTCP ] */
