//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


    // Includes
#include "rtcp/RTPHeader.h"

#ifdef __pingtel_on_posix__
#include <netinet/in.h>
#endif

#ifdef INCLUDE_RTCP /* [ */

    // Constants
const int   COUNT_MASK          = 0xF;
const int   EXTENSION_MASK      = 0x10;
const int   PAD_MASK            = 0x20;
const int   VERSION_MASK        = 0xC0;
const int   PAYLOAD_MASK        = 0x7F;
const int   MARKER_MASK         = 0x80;

const int   EXTENSION_SHIFT     = 4;
const int   PAD_SHIFT           = 5;
const int   VERSION_SHIFT       = 6;
const int   MARKER_SHIFT        = 7;

const int   COUNT_SHIFT         = 0x4;

const int   SSRC_OFFSET         = 8;
const int   CSRC_OFFSET         = 12;

const int   HEADER_LENGTH       = 12;

/**
 *
 * Method Name:  CRTPHeader() - Constructor
 *
 *
 * Inputs:   unsigned char *puchHeaderData
 *                             - Option Header content used to load the object
 *           unsigned long ulPacketLength - length of buffer content passed
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description:  Performs routine CRTPHeader object initialization.
 *
 * Usage Notes:  A CRTPHeader object shall be created by the CRTPSource with
 *               this constructor if allocated on the fly.
 *
 *
 */
CRTPHeader::CRTPHeader(unsigned long ulSSRC,
          unsigned char *puchHeaderData, unsigned long ulPacketLength) :
        m_ulVersion(2),
        m_ulPadding(0),
        m_ulMarker(0),
        m_ulExtension(0),
        m_ulPayload(0),
        m_ulSequenceNo(0),
        m_ulRTPTimestamp(0),
        m_ulRecvTimestamp(0),
        m_ulCSRCCount(0)

{

    // Save the SSRC ID
    m_ulSSRC = ulSSRC;

    // The user has created this RTP Header object on the fly.
    // Let's check whether the content of an RTP packet has been passed as an
    //  initialization parameter
    if(puchHeaderData)
    {
        // Header Data has been provided.
        // Let's load its contents into individual RTP header attributes.
        ParseRTPHeader(puchHeaderData, ulPacketLength);
    }

}


/**
 *
 * Method Name: ~CRTPHeader() - Destructor
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: Shall deallocate and/or release all resources that were
 *              acquired over the course of runtime.
 *
 * Usage Notes:
 *
 *
 */
CRTPHeader::~CRTPHeader(void)
{
// Our reference count must have gone to 0 to get here.  We have not allocated
//  any memory so we shall now go quietly into that good night!
}


/**
 *
 * Method Name:  ParseRTPHeader
 *
 *
 * Inputs:   unsigned char *puchRTPBuffer  - Buffer containing RTP Packet
 *           unsigned long  ulPacketLength - length of buffer content passed
 *
 * Outputs:  None
 *
 * Returns:  unsigned long
 *
 * Description: Parse an RTP network packet into an Report header.  Some basic
 *              check will be performed to determine whether the packet is
 *              valid.
 *
 * Usage Notes: A buffer of sufficient size should be allocated and passed to
 *              this parsing method.
 *
 *
 */
unsigned long CRTPHeader::ParseRTPHeader(unsigned char *puchRTPBuffer,
                                         unsigned long ulPacketLength)
{

    // The buffer is composed of several constituents pieces of data.
    // Let's store this data into individual attributes.
    unsigned char *puchRTPHeader = puchRTPBuffer;

    // Extract CSRC Count
    m_ulCSRCCount = *puchRTPHeader & COUNT_MASK;

    // Extract Extension
    m_ulExtension = ((*puchRTPHeader & EXTENSION_MASK) >> EXTENSION_SHIFT);

    // Extract Padding
    m_ulPadding   = ((*puchRTPHeader & PAD_MASK) >> PAD_SHIFT);

    // Check for valid Version #
    if(((*puchRTPHeader++ & VERSION_MASK) >> VERSION_SHIFT) !=
                                                   (unsigned char)m_ulVersion)
    {
        osPrintf("**** FAILURE **** CRTPHeader::ParseRTPHeader()"
                                                      " - Invalid Version\n");
        return(0);
    }

    // Check for valid Payload Type
    m_ulPayload   = *puchRTPHeader & PAYLOAD_MASK;

    // Extract Marker
    m_ulMarker    = ((*puchRTPHeader++ & MARKER_MASK) >> MARKER_SHIFT);

    // Extract RTP Sequence #
    m_ulSequenceNo = ntohs(*((unsigned short *)puchRTPHeader));
    puchRTPHeader += sizeof(short);

    // Extract RTP Timestamp
    m_ulRTPTimestamp = ntohl(*((unsigned long *)puchRTPHeader));
    puchRTPHeader += sizeof(long);


    // Check for valid SSRC if set
    unsigned long ulSSRC = ntohl(*((unsigned long *)puchRTPHeader));
    if(m_ulSSRC && ulSSRC != m_ulSSRC)
    {
        osPrintf("**** FAILURE **** CRTPHeader::ParseRTPHeader()"
                                                         " - Invalid SSRC\n");
        return(0);
    }

    // Load SSRC
    m_ulSSRC = ulSSRC;
    puchRTPHeader += sizeof(long);

    // Load CSRC
    unsigned long *paulCSRCs = ((unsigned long *)puchRTPHeader);
    for(unsigned long ulCount = 0;
        ulCount < MAX_CSRCS && ulCount < m_ulCSRCCount;
        ulCount++)
    {
        m_aulCSRC[ulCount] = ntohl(*paulCSRCs);
        paulCSRCs++;
    }

    return(puchRTPHeader - puchRTPBuffer);

}

/**
 *
 * Method Name:  FormatRTPHeader
 *
 *
 * Inputs:   unsigned char *puchRTPBuffer   - Buffer for storing RTP Packet
 *           unsigned long  ulPacketLength  - length of buffer content passed
 *
 * Outputs:  None
 *
 * Returns:  unsigned long
 *
 * Description: Format an RTP packet for transmission over the network.
 *
 * Usage Notes: A buffer of sufficient size should be allocated and passed to
 *              this formatting method.
 *
 *
 */
unsigned long CRTPHeader::FormatRTPHeader(unsigned char *puchRTPBuffer,
                                          unsigned long ulPacketLength)
{

    // The buffer is composed of several constituents pieces of data.
    // Let's build this buffer using information contained with the object.
    unsigned char *puchRTPHeader = puchRTPBuffer;

    // Load CSRC Count
    *puchRTPHeader = (unsigned char)m_ulCSRCCount;

    // Load Extension bit
    *puchRTPHeader |=
         (unsigned char)((m_ulExtension << EXTENSION_SHIFT) & EXTENSION_MASK);

    // Load Padding bit
    *puchRTPHeader |= (unsigned char)((m_ulPadding << PAD_SHIFT) & PAD_MASK);

    // Load Version #
    *puchRTPHeader++ |=
               (unsigned char)((m_ulVersion << VERSION_SHIFT) & VERSION_MASK);

    // Load Payload Type
    *puchRTPHeader    = (unsigned char)m_ulPayload;

    // Load Marker Bit
    *puchRTPHeader++ |=
                  (unsigned char)((m_ulMarker << MARKER_SHIFT) & MARKER_MASK);

    // Load Sequence #
    *((unsigned short *)puchRTPHeader) =
                                      htons((unsigned short)m_ulSequenceNo++);
    puchRTPHeader    += sizeof(unsigned short);

    // Load RTP Timestamp
    *((unsigned long *)puchRTPHeader) = htonl(m_ulRTPTimestamp);
    puchRTPHeader    += sizeof(unsigned long);

    // Load SSRC
    *((unsigned long *)puchRTPHeader) = htonl(m_ulSSRC);
    puchRTPHeader    += sizeof(unsigned long);

    // Load CSRC
    // unsigned long *paulCSRCs = (unsigned long *)puchRTPHeader;
      // Did Dave plan to use this at $$$$?
    for(unsigned long ulCount = 0;
                    ulCount < MAX_CSRCS && ulCount < m_ulCSRCCount; ulCount++)
    {
        *((unsigned long *)puchRTPHeader) = htonl(m_aulCSRC[ulCount]); // $$$$
        puchRTPHeader += sizeof(unsigned long);
    }

    return(puchRTPHeader - puchRTPBuffer);

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
 * Returns:     unsigned long  - Return the size of the RTP Header
 *
 * Description: Returns the size of the RTP Header that preceeds the payload.
 *
 * Usage Notes:
 *
 *
 */
unsigned long CRTPHeader::GetHeaderLength(void)
{

    // Return the header length
    return(HEADER_LENGTH + (m_ulCSRCCount * sizeof(long)));

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
 * Returns:     unsigned long - Protocol Version #
 *
 * Description: Returns the protocol version number from the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
unsigned long CRTPHeader::GetVersion(void)
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
 * Returns:     unsigned long - Padding Flag
 *
 * Description: Returns the padding flag value from the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
unsigned long CRTPHeader::GetPadding(void)
{

    // Return Padding
    return(m_ulPadding);

}



/**
 *
 * Method Name: GetExtension
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long - Header Extension Flag
 *
 * Description: Returns the header extension flag from the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
unsigned long  CRTPHeader::GetExtension(void)
{

    // Return Extension
    return(m_ulExtension);

}


/**
 *
 * Method Name: GetMarker
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long - Marker Flag
 *
 * Description: Returns the marker flag value from the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
unsigned long  CRTPHeader::GetMarker(void)
{

    // Return Marker
    return(m_ulMarker);

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
 * Returns:     unsigned long - Payload Type
 *
 * Description: Returns the payload type value from the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
unsigned long CRTPHeader::GetPayload(void)
{

    // Return Payload Type
    return(m_ulPayload);

}


/**
 *
 * Method Name: GetSequenceNo
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long - Packet Sequence Number
 *
 * Description: Returns the sequence number value from the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
unsigned long CRTPHeader::GetSequenceNo(void)
{

    // Return Sequence No
    return(m_ulSequenceNo);

}

/**
 *
 * Method Name: GetRTPTimestamp
 *
 *
 * Inputs:      None
 *
 * Outputs:     unsigned long *pulTimestamp    - RTP Packet Timestamp
 *
 * Returns:     void
 *
 * Description: Returns the timestamp value from the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
void CRTPHeader::GetRTPTimestamp(unsigned long *pulTimestamp)
{

    // Return RTP Timestamp
    *pulTimestamp = m_ulRTPTimestamp;

}

/**
 *
 * Method Name: GetRecvTimestamp
 *
 *
 * Inputs:      None
 *
 * Outputs:     unsigned long *pulTimestamp    - Packet Receive Timestamp
 *
 * Returns:     void
 *
 * Description: Returns the receive timestamp value from the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
void CRTPHeader::GetRecvTimestamp(unsigned long *pulRecvTimestamp)
{

    // Return RTP Receive Timestamp
    *pulRecvTimestamp = m_ulRecvTimestamp;

}



/**
 *
 * Method Name: GetSSRC
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long - Packet Source Identifier
 *
 * Description: Returns the SSRC value from the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
unsigned long CRTPHeader::GetSSRC(void)
{

    // Return SSRC
    return(m_ulSSRC);

}


/**
 *
 * Method Name: GetCSRC
 *
 *
 * Inputs:   None
 *
 * Outputs:  unsigned long *paulCSRC
 *                          - Contributing Source Identifier(s) Array pointer
 *           bool bNBO      - TRUE => data should be represented in NBO format
 *
 * Returns:  unsigned long - Number of elements loaded
 *
 * Description: Returns the contributing source values from the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
unsigned long CRTPHeader::GetCSRC(unsigned long *paulCSRC, bool bNBO)
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
    return(bNBO ? htonl(m_ulCSRCCount) : m_ulCSRCCount);


}

/**
 *
 * Method Name: SetPayload
 *
 *
 * Inputs:      unsigned long - Payload Type
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: Sets the payload type value associated with the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
void CRTPHeader::SetPayload(unsigned long ulPayload)
{

    // Set Payload Type
    m_ulPayload = ulPayload;

}


/**
 *
 * Method Name: SetSequenceNo
 *
 *
 * Inputs:      unsigned long - Packet Sequence Number
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: Sets the sequence number value associated with the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
void CRTPHeader::SetSequenceNo(unsigned long ulSequenceNo)
{

    // Return Sequence No
    m_ulSequenceNo = ulSequenceNo;

}

/**
 *
 * Method Name: SetRTPTimestamp
 *
 *
 * Inputs:      unsigned long ulRTPTimestamp - Packet RTP Timestamp
 *
 * Outputs:     None
 *
 * Returns:     void
 *
 * Description: Sets the sRTP timestamp value associated with the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
void CRTPHeader::SetRTPTimestamp(unsigned long ulRTPTimestamp)
{

    // Set RTP Timestamp
    m_ulRTPTimestamp = ulRTPTimestamp;

}

/**
 *
 * Method Name: SetRecvTimestamp
 *
 *
 * Inputs:      unsigned long ulRecvTimestamp - Packet Receive Timestamp
 *
 * Outputs:     None
 *
 * Returns:     void
 *
 * Description: Sets the receive timestamp value associated with
 *              the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
void CRTPHeader::SetRecvTimestamp(unsigned long ulRecvTimestamp)
{

    // Set Receive Timestamp
    m_ulRecvTimestamp = ulRecvTimestamp;

}

/**
 *
 * Method Name: ResetContents
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     void
 *
 * Description: Resets the content of the RTP header object except for the
 *              version number and SSRC.  This method allows a user to re-use
 *              RTP Header objects.
 *
 * Usage Notes: This method allows a user to re-use RTP Header objects.
 *
 *
 */
void CRTPHeader::ResetContents(void)
{

    // Restore all RTP Header attributes to their initial values
    m_ulPadding             = 0;
    m_ulMarker              = 0;
    m_ulExtension           = 0;
    m_ulPayload             = 0;
    m_ulSequenceNo          = 0;
    m_ulRecvTimestamp       = 0;
    m_ulRTPTimestamp        = 0;
    m_ulCSRCCount           = 0;

}
#endif /* INCLUDE_RTCP ] */
