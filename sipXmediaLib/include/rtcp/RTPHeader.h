//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////



//  Border Guard
#ifndef _RTPHeader_h
#define _RTPHeader_h

#include "rtcp/RtcpConfig.h"

//  Includes
#include "BaseClass.h"
#include "IRTPHeader.h"

//  Defines
#define MAX_CSRCS   64


/**
 *
 * Class Name:  CRTPHeader
 *
 * Inheritance: CBaseClass       - Generic Base Class
 *
 *
 * Interfaces:  IRTPHeader       - RTP Header Interface
 *
 * Description: The CRTPHeader Class coordinates the creation of RTP Headers
 *              given a well formed RTP packet received from the network.
 *              It also provides services for the extraction of information
 *              contained within an RTP packet header.
 *
 * Notes:       CRTPHeader is derived from CBaseClass which provides basic
 *              Initialization and reference counting support.
 *
 */
class CRTPHeader: public CBaseClass, // Inherits the CBaseClass implementation
                  public IRTPHeader  // RTP Header Interface
 {

//  Public Methods
public:

/**
 *
 * Method Name:  CRTPHeader() - Constructor
 *
 *
 * Inputs:   unsigned long ulSSRC
 *           unsigned char* puchHeaderData
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
    CRTPHeader(unsigned long ulSSRC = 0,
               unsigned char *puchHeaderData=NULL,
               unsigned long ulPacketLength=0);



/**
 *
 * Method Name: ~CRTPHeader() - Destructor
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: Shall deallocated and/or release all resources which was
 *              acquired over the course of runtime.
 *
 * Usage Notes:
 *
 *
 */
    ~CRTPHeader(void);



/**
 *
 * Method Name:  ParseRTPHeader
 *
 *
 * Inputs:   unsigned char  *puchRTPBuffer  - Buffer containing RTP Packet
 *           unsigned long   ulPacketLength - length of buffer content passed
 *
 * Outputs:  None
 *
 * Returns:  unsigned long
 *
 * Description: Parse an RTP network packet into an Report header.
 *              Some basic check will be performed to determine whether the
 *              packet is valid.
 *
 * Usage Notes: A buffer of sufficient size should be allocated and passed to
 *              this parsing method.
 *
 *
 */
    unsigned long ParseRTPHeader(unsigned char *puchRTPBuffer,
                                 unsigned long ulPacketLength=0);

/**
 *
 * Method Name:  FormatRTPHeader
 *
 *
 * Inputs:   unsigned char  *puchRTPBuffer  - Buffer containing RTP Packet
 *           unsigned long  ulPacketLength  - length of buffer content passed
 *
 * Outputs:  None
 *
 * Returns:  unsigned long
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
    unsigned long FormatRTPHeader(unsigned char *puchRTPBuffer,
                                  unsigned long ulPacketLength=0);

/**
 *
 * Method Name:  GetHeaderLength
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  unsigned long  - Returns the size of the RTP Header
 *
 * Description: Retrieves the size of the RTP Header that preceeds the payload.
 *
 * Usage Notes:
 *
 *
 */
    unsigned long GetHeaderLength(void);


/**
 *
 * Method Name:  GetVersion
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  unsigned long  - Returns the Version
 *
 * Description: Retrieves the Version attribute stored within the object.
 *
 * Usage Notes:
 *
 *
 */
    unsigned long GetVersion(void);


/**
 *
 * Method Name: GetPadding
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  unsigned long - Padding Flag
 *
 * Description: Returns the padding flag value from the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
    unsigned long GetPadding(void);

/**
 *
 * Method Name: GetExtension
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  unsigned long - - Header Extension Flag
 *
 * Description: Returns the header extension flag from the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
    unsigned long GetExtension(void);


/**
 *
 * Method Name: GetMarker
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  unsigned long  - Marker Flag
 *
 * Description: Returns the marker flag value associated with the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
    unsigned long GetMarker(void);


/**
 *
 * Method Name:  GetPayload
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  unsigned long   - Returns the Payload Type
 *
 * Description: Retrieves the payload type from this RTP report.
 *
 * Usage Notes:
 *
 *
 */
    unsigned long GetPayload(void);


/**
 *
 * Method Name: GetSequenceNo
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  unsigned long - Packet Sequence Number
 *
 * Description: Returns the sequence number value from the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
    unsigned long GetSequenceNo(void);

/**
 *
 * Method Name: GetRTPTimestamp
 *
 *
 * Inputs:   None
 *
 * Outputs:  unsigned long *pulTimestamp    - RTP Packet Timestamp
 *
 * Returns:  void
 *
 * Description: Returns the timestamp value associated with the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
    void GetRTPTimestamp(unsigned long *pulTimestamp);

/**
 *
 * Method Name: GetRecvTimestamp
 *
 *
 * Inputs:   None
 *
 * Outputs:  unsigned long *pulTimestamp    - Packet Receive Timestamp
 *
 * Returns:  void
 *
 * Description: Returns the timestamp value associated with the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
    void GetRecvTimestamp(unsigned long *pulTimestamp);


/**
 *
 * Method Name: GetSSRC
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  unsigned long - Packet Source Identifier
 *
 * Description: Returns the SSRC value associated with the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
    unsigned long GetSSRC(void);


/**
 *
 * Method Name:  GetCSRC
 *
 *
 * Inputs:   bool bNBO - TRUE implies data should be represented in NBO format
 *
 * Outputs:  unsigned long *paulCSRC
 *                           - Contributing Source Identifier(s) Array pointer
 *
 * Returns:  unsigned long - Number of Contributing Source Identifier(s)
 *
 * Description: Returns the contributing source ID values from the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
    unsigned long GetCSRC(unsigned long *paulCSRC, bool bNBO=FALSE);

/**
 *
 * Method Name: SetPayload
 *
 *
 * Inputs:   unsigned long - Payload Type
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: Sets the payload type value associated with the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
    void SetPayload(unsigned long ulPayload);


/**
 *
 * Method Name: SetSequenceNo
 *
 *
 * Inputs:   unsigned long - Packet Sequence Number
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: Sets the sequence number value associated with the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
    void SetSequenceNo(unsigned long ulSequenceNo);

/**
 *
 * Method Name: SetRTPTimestamp
 *
 *
 * Inputs:   unsigned long ulRTPTimestamp - Packet RTP Timestamp
 *
 * Outputs:  None
 *
 * Returns:  void
 *
 * Description: Sets the sRTP timestamp value associated with the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
    void SetRTPTimestamp(unsigned long ulRTPTimestamp);

/**
 *
 * Method Name: SetRecvTimestamp
 *
 *
 * Inputs:   unsigned long ulTimestamp - Packet Receive Timestamp
 *
 * Outputs:  None
 *
 * Returns:  void
 *
 * Description: Sets the receive timestamp values for the RTP packet.
 *
 * Usage Notes:
 *
 *
 */
    void SetRecvTimestamp(unsigned long ulTimestamp);

/**
 *
 * Macro Name:  DECLARE_IBASE_M
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: This implements the IBaseClass functions used and exposed by
 *              derived classes.
 *
 * Usage Notes:
 *
 *
 */
DECLARE_IBASE_M


private:   // Private Methods


/**
 *
 * Method Name: ResetContents
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  void
 *
 * Description: Resets the contetn of the RTP header object except for the
 *              version number and SSRC.  This method allows a user to
 *              re-use RTP Header objects.
 *
 * Usage Notes: This method allows a user to re-use RTP Header objects.
 *
 *
 */
    void ResetContents(void);


private:        // Protected Data Members



/**
 *
 * Attribute Name:  m_ulVersion
 *
 * Type:            unsigned long
 *
 * Description:     The protocol version of the RTP Report.
 *
 */
      unsigned long m_ulVersion;

/**
 *
 * Attribute Name:  m_ulPadding
 *
 * Type:            unsigned long
 *
 * Description:  A flag identifying the use of padding within an RTP report.
 *
 */
      unsigned long m_ulPadding;


/**
 *
 * Attribute Name:  m_ulPadding
 *
 * Type:            unsigned long
 *
 * Description:     A flag identifying the use of the marker bit within
 *                  an RTP report.
 *
 */
      unsigned long m_ulMarker;

/**
 *
 * Attribute Name:  m_ulExtension
 *
 * Type:            unsigned long
 *
 * Description:     A flag identifying the use of header extensions within
 *                  an RTP report.
 *
 */
      unsigned long m_ulExtension;


/**
 *
 * Attribute Name:  m_ulPayload
 *
 * Type:            unsigned long
 *
 * Description:     The RTP Payload type.
 *
 */
      unsigned long m_ulPayload;


/**
 *
 * Attribute Name:  m_ulSequenceNo
 *
 * Type:            unsigned long
 *
 * Description:     The RTP Sequence Number.
 *
 */
      unsigned long m_ulSequenceNo;


/**
 *
 * Attribute Name:  m_ulRTPTimestamp
 *
 * Type:            unsigned long
 *
 * Description:     The RTP Timestamp; aka, the timestamp when the FE sent
 *                  the packet
 *
 */
      unsigned long m_ulRTPTimestamp;

/**
 *
 * Attribute Name:  m_ulRecvTimestamp
 *
 * Type:            unsigned long
 *
 * Description:     The RTP Recv Timestamp; aka, when the RTP pakcet was
 *                  actually received on our system
 *
 */
      unsigned long m_ulRecvTimestamp;


/**
 *
 * Attribute Name:  m_ulSSRC
 *
 * Type:            unsigned long
 *
 * Description:     This member shall store the SSRC ID of the
 *                  associated RTP connection.
 *
 */
      unsigned long m_ulSSRC;

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


#endif
