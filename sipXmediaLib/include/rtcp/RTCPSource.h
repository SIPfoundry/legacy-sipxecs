//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


//  Border Guard
#ifndef _RTCPSource_h
#define _RTCPSource_h

#include "rtcp/RtcpConfig.h"

//  Includes
#include "TLinkedList.h"
#include "RTCPHeader.h"
#include "ByeReport.h"
#include "SenderReport.h"
#include "ReceiverReport.h"
#include "SourceDescription.h"
#include "IRTCPStatistics.h"
#include "INetDispatch.h"
#include "IRTCPNotify.h"

#ifdef __pingtel_on_posix__
#include <netinet/in.h>
#endif

/**
 *
 * Class Name:  CRTCPSource
 *
 * Inheritance: CBaseClass           - Base Class Implementation
 *
 *
 * Interfaces:  INetDispatch,        - RTCP Report Input Interface
 *              IRTCPStatistics      - RTCP Statistics Retrieval Interface
 *
 * Description: The CRTCPSource Class coordinates the processing and generation
 *              of RTCP reports corresponding to an inbound RTP connection from
 *              a call participant.
 *
 * Notes:       CRTCPSource is derived from CBaseClass which provides basic
 *              Initialization and reference counting support.
 *
 */
class CRTCPSource:
         public CBaseClass,        // Inherits CBaseClass implementation
         public INetDispatch,      // RTCP Report Input Interface
         public IRTCPStatistics    // RTCP Statistics Retrieval Interface
 {

//  Public Methods
public:

/**
 *
 * Method Name:  CRTCPSource() - Constructor
 *
 *
 * Inputs:   unsigned long          ulSSRC
 *                                    - The the Identifier for this source
 *           IRTCPNotify            *piRTCPNotify
 *                                    - RTCP Event Notification Interface
 *           ISetReceiverStatistics *piSetStatistics
 *                                    - Interface for setting receiver stats
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description:  Performs routine CRTCPSource object initialization include
 *               assignment of constructor arguments to internal attributes.
 *
 * Usage Notes:  It is assumed the the pointer obtained during CRTCPSource
 *               object construction shall be used in the construction or
 *               initialization of the Network Source, RTP Source, and RTCP
 *               Sender objects associated with an inbound RTP connection.
 *
 */
    CRTCPSource(unsigned long ulSSRC, IRTCPNotify *piRTCPNotify,
                ISetReceiverStatistics *piSetStatistics);


/**
 *
 * Method Name: ~CRTCPSource() - Destructor
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: Shall deallocated and/or release all resources which was
 *              acquired over the course of runtime.  In particular, the
 *              following shall occur:
 *                ==> The queue containing remote RTCP Source Description
 *                    objects shall be drained with the reference to each
 *                    object released.
 *                ==> The reference to the remote Source Report object shall
 *                    be released
 *                ==> The reference to the remote Receiver Report object shall
 *                    be released
 *
 * Usage Notes: This shall override the virtual destructor in the base class so
 *              that deallocation specific to the derived class will be done
 *              despite the destruction being performed in the base class as
 *              part of the release.
 *
 */
    virtual ~CRTCPSource(void);

/**
 *
 * Method Name: ProcessPacket
 *
 *
 * Inputs:   unsigned char *puchDataBuffer
 *                            - Data Buffer received from Network Source
 *           unsigned long ulBufferLength
 *                            - Length of Data Buffer
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The ProcessDataPacket() sequentially processes data packets
 *              received by looking for for header information that describes
 *              the single or composite RTCP report sent.  The RTCP Payload
 *              type of each header found is examined.  A packet or a composite
 *              portion is discarded if it the header not contain a valid RTCP
 *              Payload Type (Types 200 - 204).  The packet or composite
 *              portion is then routed by Payload type to the associated
 *              Sender, Receiver Source Description, or Bye method for further
 *              report processing. This iterative process continues until a
 *              packet or composite has been full processed.
 *
 *
 * Usage Notes: The intention is for this method to be processed on the
 *              Execution thread of the Network Dispatcher.  This may be
 *              modified if RTCP processing is interferring with the
 *              Dispatcher's ability to receive and route high volume RTP
 *              packets.
 *
 *
 */
    void ProcessPacket(unsigned char *puchDataBuffer,
                       unsigned long ulBufferLength, int verbose=0);

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

private:        // Private Methods


/**
 *
 * Method Name: SendRTCPEvent
 *
 *
 * Inputs:   ulong ulEventType    - Event Type
 *           void  *pvInterface   - Opaque pointer to an associated interface
 *           ulong ulChangeMask   - Optional mask for SDES Changes
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: A centralized processing method that will dispatch events to
 *              all subscribing parties based on their registered interest.
 *
 *
 * Usage Notes: This centralized event generation mechanism is useful in
 *              dispatching events to multiple subscribers.  This avoid the
 *              duplication of code in each method that might cause an event
 *              to be generated.
 *
 *
 */
    void SendRTCPEvent(unsigned long ulEventType,
                       void  *pvInterface,
                       unsigned long ulChangeMask=0);

/**
 *
 * Method Name: ProcessSenderReport
 *
 *
 * Inputs:   unsigned char *puchRTCPReport - A pointer to an RTCP Sender Report
 *
 * Outputs:  None
 *
 * Returns:  unsigned long
 *
 * Description: Takes the RTCP Sender Report passed and calls the CSenderReport
 *              object's ISenderReport interface to parse the Sender packet and
 *              extract the sender statistics contained therein.  This method
 *              shall also check for the presence of receiver reports within
 *              the packet and call the CReceiverReport object's
 *              IReceiverReport interface to parse and extract its contents.
 *              Although no more than one receiver report is expected under the
 *              current Pingtel call model, it is possible that multiple
 *              receiver reports (one per PingTel source) may be sent.  In this
 *              case,  a new CReceiverReport object shall be created and queued
 *              if not already existing on the Receiver Report list.
 *
 *
 * Usage Notes: Notifications shall be generated to all subscribing parties to
 *              inform them of the receipt of a new Source Report.
 *              The notification shall contain the event type and a pointer to
 *              the Source Report's IGetSenderStatistics interface.
 *
 *
 */
    unsigned long ProcessSenderReport(unsigned char *puchRTCPReport);


/**
 *
 * Method Name: ProcessReceiverReport
 *
 *
 * Inputs:   unsigned char *puchRTCPReport
 *                                       - A pointer to an RTCP Receiver Report
 *           unsigned long ulReportCount - An optional report count
 *
 * Outputs:  None
 *
 * Returns:  unsigned long
 *
 * Description: Takes the RTCP Receiver Report passed and calls the
 *              CReceiverReport object's IReceiverReport interface to parse the
 *              Receiver packet and extract the receiver statistics contained
 *              therein.  Although no more than one receiver report is expected
 *              under the current Pingtel call model, it is possible that
 *              multiple receiver reports (one per PingTel source) may be sent.
 *              In this case, a new CReceiverReport object shall be created and
 *              queued if not already existing on the Receiver Report list.
 *
 *
 * Usage Notes: Notifications shall be generated to all subscribing parties to
 *              inform them of the receipt of a new Receiver Report.
 *              The notification shall contain the event type and a pointer to
 *              the Receiver Report's IGetReceiverStatistics interface.
 *
 *
 */
    unsigned long ProcessReceiverReport(unsigned char *puchRTCPReport,
                                        unsigned long ulReportCount = 0);


/**
 *
 * Method Name: ProcessSDESReport
 *
 *
 * Inputs:   unsigned char *puchRTCPReport
 *                            - A pointer to an RTCP Source Description Report
 *
 * Outputs:  None
 *
 * Returns:  unsigned long
 *
 * Description: Takes the RTCP SDES Report passed and calls the
 *              CSourceDescription object's ISDESReport interface to parse the
 *              Source Description packet and extract the identification
 *              information contained therein.
 *
 *
 * Usage Notes: A call connection to a Mixer would cause all SDES Reports to be
 *              forward as multiple SDES Reports. In this case, a new
 *              CSourceDescription object shall be created and queued if not
 *              already existing on the SrcDescription list.  The SSRC ID will
 *              be used to determine uniqueness among reports.
 *
 *              Notifications shall be generated to all subscribing parties to
 *              inform them of the new Source Descriptions or changes in
 *              previously existing Source Descriptions.  The notification
 *              shall contain the event type and a pointer to the new or
 *              modified Source Description interface (IGetSrcDescription).
 */
    unsigned long ProcessSDESReport(unsigned char *puchRTCPReport);

/**
 *
 * Method Name: ProcessByeReport
 *
 *
 * Inputs:   unsigned char *puchRTCPReport - A pointer to an RTCP Bye Report
 *
 * Outputs:  None
 *
 * Returns:  unsigned long
 *
 * Description: Takes the RTCP Bye Report and extracts the associated SSRC ID.
 *              A notification shall be generated to all subscribing parties
 *              to inform them of the receipt of the BYE along with the
 *              associated SSRC ID.
 *
 *
 * Usage Notes:
 *
 *
 */
    unsigned long ProcessByeReport(unsigned char *puchRTCPReport);


/**
 *
 * Method Name: ProcessAppReport
 *
 *
 * Inputs:   unsigned char *puchRTCPReport
 *                               - A pointer to an RTCP Application Report
 *
 * Outputs:  None
 *
 * Returns:  unsigned long
 *
 * Description: No processing is performed on an Application Report but the
 *              length is extracted from the header and returned so that other
 *              composite reports might still be processed.
 *
 *
 * Usage Notes:
 *
 *
 */
    unsigned long ProcessAppReport(unsigned char *puchRTCPReport);


/**
 *
 * Method Name:  GetStatistics
 *
 *
 * Inputs:   None
 *
 * Outputs:  IGetSrcDescription     **piGetSrcDescription
 *                                      - Source Description Interface Pointer
 *           IGetSenderStatistics   **piSenderStatistics
 *                                      - Sender Statistics Interface Pointer
 *           IGetReceiverStatistics **piReceiverStatistics
 *                                      - Receiver Statistics Interface Pointer
 *           IByeInfo               **piGetByeInfo -
 *                                     Interface for retrieving Bye Report Info
 *
 *
 * Returns:  None
 *
 * Description:  Returns pointers to the Sender,Receiver and Bye statistics
 *              interfaces upon request.
 *
 * Usage Notes:  This would be used by the QOS object or the RTCP Statistics
 *              object if a polling method were supported.  These objects could
 *              alternatively be informed of statistic changes via notification
 *              with the interfaces contained within the callback.
 *
 *
 */
    void GetStatistics(IGetSrcDescription     **piGetSrcDescription,
                       IGetSenderStatistics   **piSenderStatistics,
                       IGetReceiverStatistics **piReceiverStatistics,
                       IGetByeInfo            **piGetByeInfo);

/**
 *
 * Method Name: GetPayloadType
 *
 *
 * Inputs:      unsigned char *puchDataBuffer   - RTCP Buffer
 *
 * Outputs:     None
 *
 * Returns:     RTCP_REPORTS_ET                 - Returns Payload Type
 *
 * Description: Returns the payload type value associated with the RTCP packet.
 *
 * Usage Notes:
 *
 *
 */
    RTCP_REPORTS_ET GetPayloadType(unsigned char *puchRTCPBuffer);


/**
 *
 * Method Name: GetSenderSSRC
 *
 *
 * Inputs:   bool bHeader        - TRUE implies presence of an RTCP Header
 *           unsigned char *puchRTCPBuffer  - RTCP Buffer
 *
 * Outputs:  None
 *
 * Returns:  unsigned long                   - Returns Sender's SSRC
 *
 * Description: Returns the Sender's SSRC.
 *
 * Usage Notes:
 *
 *
 */
    unsigned long GetSenderSSRC(bool bHeader, unsigned char *puchRTCPBuffer);


/**
 *
 * Method Name: GetReceiverSSRC
 *
 *
 * Inputs:   bool BRTCPHeader    - TRUE implies presence of an RTCP Header
 *           unsigned char *puchRTCPBuffer   - RTCP Buffer
 *
 * Outputs:  None
 *
 * Returns:  unsigned long       - Returns Receiver's SSRC
 *
 * Description: Returns the Receiver's SSRC.
 *
 * Usage Notes:
 *
 *
 */
    unsigned long GetReceiverSSRC(bool bRTCPHeader,
                                  unsigned char *puchRTCPBuffer);

/**
 *
 * Method Name: GetReportCount
 *
 *
 * Inputs:      unsigned char *puchRTCPBuffer   - RTCP Buffer
 *
 * Outputs:     None
 *
 * Returns:     unsigned long                   - Returns Report Count
 *
 * Description: Returns the RTCP Report Count.
 *
 * Usage Notes:
 *
 *
 */
    unsigned long GetReportCount(unsigned char *puchRTCPBuffer);


/**
 *
 * Method Name: GetReportLength
 *
 *
 * Inputs:      unsigned char *puchRTCPBuffer   - RTCP Buffer
 *
 * Outputs:     None
 *
 * Returns:     unsigned long                   - Returns Report Length
 *
 * Description: Returns the RTCP Report Length.
 *
 * Usage Notes:
 *
 *
 */
    unsigned long GetReportLength(unsigned char *puchRTCPBuffer);


private:        // Private Data Members

/**
 *
 * Attribute Name:  m_ulSSRC
 *
 * Type:            unsigned long
 *
 * Description:  This member shall store the SSRC ID of the associated RTP
 *               connection.
 *
 */
      unsigned long m_ulSSRC;


/**
 *
 * Attribute Name:  m_piRTCPNotify
 *
 * Type:            IRTCPNotify *
 *
 * Description:  This member shall contain the interface for delivering
 *               notification to a registered consumer.
 *
 */
      IRTCPNotify * m_piRTCPNotify;


/**
 *
 * Attribute Name:  m_piSetReceiverStatistics
 *
 * Type:            ISetReceiverStatistics
 *
 * Description:  Interface used by the Sender Report of an associated inbound
 *               RTP connection to update SR timestamp information within the
 *               corresponding Receiver Report.
 *
 */
      ISetReceiverStatistics *m_piSetReceiverStatistics;

/**
 *
 * Attribute Name:  m_tSrcDescriptorList
 *
 * Type:            CTLinkedList
 *
 * Description:  This member shall store the one or more SrcDescription objects
 *               representing unique RTCP SDES reports received.  More than one
 *               SDES report may result when connected to an audio mixer.
 *
 */
      CTLinkedList<CSourceDescription *> m_tSrcDescriptorList;


/**
 *
 * Attribute Name:  m_tReceiverReportList
 *
 * Type:            CTLinkedList
 *
 * Description:  This member shall store the one or more Receiver Report
 *               objects representing unique RTP sources associated with
 *               this call site.  Each object will store/extract RTCP
 *               Receiver Report statistics associated with that source (SSRC).
 *
 */
      CTLinkedList<CReceiverReport *> m_tReceiverReportList;

/**
 *
 * Attribute Name:  m_poSenderReport
 *
 * Type:            CSenderReport *
 *
 * Description:  This member shall contain a pointer to a Sender report object
 *               used to store/extract elements of a call participant's RTCP
 *               Sender Report.
 *
 */
      CSenderReport *m_poSenderReport;

/**
 *
 * Attribute Name:  m_poByeReport
 *
 * Type:            CByeReport *
 *
 * Description:  This member shall contain a pointer to a Bye report object
 *               used to store/extract elements of a call participant's RTCP
 *               Bye Report.
 *
 */
      CByeReport *m_poByeReport;

};


/**
 *
 * Method Name: GetPayloadType
 *
 *
 * Inputs:      unsigned char *puchDataBuffer   - RTCP Buffer
 *
 * Outputs:     None
 *
 * Returns:     RTCP_REPORTS_ET                 - Returns Payload Type
 *
 * Description: Returns the payload type value associated with the RTCP packet.
 *
 * Usage Notes:
 *
 *
 */
inline RTCP_REPORTS_ET
            CRTCPSource::GetPayloadType(unsigned char *puchRTCPBuffer)
{

//  Snoop into the packet header and return the payload type
    return((RTCP_REPORTS_ET)*(puchRTCPBuffer + PAYLOAD_OFFSET));

}


/**
 *
 * Method Name: GetSenderSSRC
 *
 *
 * Inputs:      unsigned char *puchRTCPBuffer   - RTCP Buffer
 *
 * Outputs:     None
 *
 * Returns:     unsigned long                   - Returns Sender's SSRC
 *
 * Description: Returns the Sender's SSRC.
 *
 * Usage Notes:
 *
 *
 */
inline unsigned long
    CRTCPSource::GetSenderSSRC(bool bRTCPHeader, unsigned char *puchRTCPBuffer)
{

//  Advance over RTCP Header if one exists
    if(bRTCPHeader)
        puchRTCPBuffer += SSRC_OFFSET;

//  Return Sender SSRC
    return(ntohl(*((unsigned long *)puchRTCPBuffer)));

}


/**
 *
 * Method Name: GetReceiverSSRC
 *
 *
 * Inputs:   bool BRTCPHeader      - TRUE implies presence of RTCP Header
 *           unsigned char *puchRTCPBuffer   - RTCP Buffer
 *
 * Outputs:  None
 *
 * Returns:  unsigned long         - Returns Receiver's SSRC
 *
 * Description: Returns the Receiver's SSRC.
 *
 * Usage Notes:
 *
 *
 */
inline unsigned long
    CRTCPSource::GetReceiverSSRC(bool bRTCPHeader,
                                 unsigned char *puchRTCPBuffer)
{

//  Advance over RTCP Header if one exists
    if(bRTCPHeader)
        puchRTCPBuffer += HEADER_LENGTH;

//  Return Receiver SSRC
    return(ntohl(*((unsigned long *)puchRTCPBuffer)));

}

/**
 *
 * Method Name: GetReportCount
 *
 *
 * Inputs:      unsigned char *puchRTCPBuffer   - RTCP Buffer
 *
 * Outputs:     None
 *
 * Returns:     unsigned long                   - Returns Report Count
 *
 * Description: Returns the RTCP Report Count.
 *
 * Usage Notes:
 *
 *
 */
inline unsigned long CRTCPSource::GetReportCount(unsigned char *puchRTCPBuffer)
{

//  Extract Report Count
    return(*puchRTCPBuffer & COUNT_MASK);

}

/**
 *
 * Method Name: GetReportLength
 *
 *
 * Inputs:      unsigned char *puchRTCPBuffer   - RTCP Buffer
 *
 * Outputs:     None
 *
 * Returns:     unsigned long                   - Returns Report Length
 *
 * Description: Returns the RTCP Report Length.
 *
 * Usage Notes:
 *
 *
 */
inline unsigned long CRTCPSource::GetReportLength(unsigned char *puchRTCPBuffer)
{

//  Extract Report Length
    unsigned long ulReportLength =
               ntohs(*((unsigned short *)(puchRTCPBuffer + LENGTH_OFFSET))) + 1;

    return(ulReportLength * sizeof(long));

}
#endif
