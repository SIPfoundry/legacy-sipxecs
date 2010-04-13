//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////




//  Border Guard
#ifndef _IRTPHeader_h
#define _IRTPHeader_h

#include "rtcp/RtcpConfig.h"

//  Includes
#include "IBaseClass.h"

/**
 *
 * Interface Name:  IRTPHeader
 *
 * Inheritance:     None
 *
 *
 * Description:     The IRTPHeader interface allows a user to create an RTP
 *                  Header given an RTP packet received from the network.  It
 *                  also provides the user with services to extract
 *                  information from this header once it has been formed.
 *
 * Notes:
 *
 */
interface IRTPHeader : public IBaseClass
 {

//  Public Methods
public:

/**
 *
 * Method Name:  ParseRTPHeader
 *
 *
 * Inputs:      unsigned char  *puchRTPBuffer   -
 *                                 Character Buffer containing RTP Packet
 *              unsigned long   ulPacketLength  -
 *                               Optional length of buffer content passed
 *
 * Outputs:     None
 *
 * Returns:     unsigned long
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
    virtual unsigned long ParseRTPHeader(unsigned char *puchRTPBuffer,
                                         unsigned long ulPacketLength)=0;

/**
 *
 * Method Name:  FormatRTPHeader
 *
 *
 * Inputs:      unsigned char  *puchRTPBuffer   -
 *                                    Character Buffer containing RTP Packet
 *              unsigned long   ulPacketLength  -
 *                                    Optional length of buffer content passed
 *
 * Outputs:     None
 *
 * Returns:     unsigned long
 *
 * Description: Constructs an RTP Report header using information passed in
 *              the RTP character buffer.  Some basic check will be performed
 *              to determine whether the packet is valid.
 *
 * Usage Notes: A buffer of sufficient size should be allocated and passed to
 *              this formatting method.
 *
 *
 */
    virtual unsigned long FormatRTPHeader(unsigned char *puchRTPBuffer,
                                          unsigned long ulPacketLength)=0;

/**
 *
 * Method Name:  GetHeaderLength
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long  - Returns the size of the RTP Header
 *
 * Description: Returns the size of the RTP Header that preceeds the payload.
 *
 * Usage Notes:
 *
 *
 */
    virtual unsigned long GetHeaderLength(void)=0;


/**
 *
 * Method Name:  GetVersion
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long               - Returns the Version
 *
 * Description: Retrieves the Version attribute stored within the object.
 *
 * Usage Notes:
 *
 *
 */
    virtual unsigned long GetVersion(void)=0;


/**
 *
 * Method Name: GetPadding
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long               - Padding Flag
 *
 * Description: Returns the padding flag value associated with the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
    virtual unsigned long GetPadding(void)=0;

/**
 *
 * Method Name: GetExtension
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long               - Header Extension Flag
 *
 * Description: Returns the header extension flag from the RTP packet header.
 *
 * Usage Notes:
 *
 *
 */
    virtual unsigned long GetExtension(void)=0;


/**
 *
 * Method Name: GetMarker
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long               - Marker Flag
 *
 * Description: Returns the marker flag value associated with the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
    virtual unsigned long GetMarker(void)=0;


/**
 *
 * Method Name:  GetPayload
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long               - Returns the Payload Type
 *
 * Description: Retrieves the payload type associated with this RTP report.
 *
 * Usage Notes:
 *
 *
 */
    virtual unsigned long GetPayload(void)=0;


/**
 *
 * Method Name: GetSequenceNo
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long           - Packet Sequence Number
 *
 * Description: Returns the sequence number from the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
    virtual unsigned long GetSequenceNo(void)=0;

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
 * Description: Returns the timestamp value associated with the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
    virtual void GetRTPTimestamp(unsigned long *pulTimestamp)=0;


/**
 *
 * Method Name: GetRecvTimestamp
 *
 *
 * Inputs:      None
 *
 * Outputs:     unsigned long *pulTimestamp    - Packet TReceive Timestamp
 *
 * Returns:     void
 *
 * Description: Returns the timestamp value associated with the RTP packet.
 *              The Received Timestamp is the time of reception of the RTP
 *              Packet.
 *
 * Usage Notes:
 *
 *
 */
    virtual void GetRecvTimestamp(unsigned long *pulTimestamp)=0;


/**
 *
 * Method Name: GetSSRC
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long               - Packet Source Identifier
 *
 * Description: Returns the SSRC value associated with the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
    virtual unsigned long GetSSRC(void)=0;



/**
 *
 * Method Name:  GetCSRC
 *
 *
 * Inputs:      bool bNBO -
 *                        Flag specifying whether data should be represented
 *                        in Network Byte order format
 *
 * Outputs:     unsigned long *paulCSRC  -
 *                        Contributing Source Identifier(s) Array pointer
 *
 * Returns:     unsigned long -
 *                        Number of Contributing Source Identifier(s)
 *
 * Description: Returns the contributing source values from the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
    virtual unsigned long GetCSRC(unsigned long *paulCSRC, bool bNBO)=0;


};


#endif
