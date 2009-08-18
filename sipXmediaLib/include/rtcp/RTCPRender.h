//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

//  Border Guard
#ifndef _RTCPRender_h
#define _RTCPRender_h

#include "rtcp/RtcpConfig.h"

//  Includes
#include "SenderReport.h"
#include "ReceiverReport.h"
#include "SourceDescription.h"
#include "ByeReport.h"
#include "INetworkRender.h"
#include "IRTPDispatch.h"
#include "IRTCPRender.h"
#include "IRTCPStatistics.h"
#include "IRTCPNotify.h"




/**
 *
 * Class Name:  CRTCPRender
 *
 * Inheritance: CBaseClass           - Base Class Implementation
 *
 *
 * Interfaces:  IRTCPRender,         - RTCP Sender InterfaceIRTPDispatch,
 *              IRTPDispatch         - RTP Packet Input Interface
 *              IRTCPStatistics      - RTCP Statistics Retrieval Interface
 *
 * Description: The CRTCPRender Class coordinates the generation of RTCP
 *              reports corresponding to an outbound RTP connection to a call
 *              participant.
 *
 * Notes:       CRTCPRender is derived from CBaseClass which provides basic
 *              Initialization and reference counting support.
 *
 */
class CRTCPRender:
      public CBaseClass,     // Inherits CBaseClass implementation
      public IRTPDispatch,   // Inbound RTP Dispatch Interface
      public IRTCPRender,    // RTCP Sender Control Interface
      public IRTCPStatistics // RTCP Statistics Retrieval Interface

 {

//  Public Methods
public:

/**
 *
 * Method Name:  CRTCPRender() - Constructor
 *
 *
 * Inputs:  unsigned long   ulSSRC          - SSRC ID
 *          IRTCPNotify    *piRTCPNotify    - RTCP Event Notification Interface
 *          INetworkRender *piNetworkRender
 *                                - Pointer to the Network Render interface
 *          ISDESReport    *piSDESReport
 *                                - Pointer to the local Source Desc interface
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description:  Performs routine CRTCPRender object initialization.
 *
 * Usage Notes:  The interface pointers passed in the constructor list shall
 *               be stored as an attribute and used to send RTCP Reports out
 *               to the network.
 *
 *
 */
    CRTCPRender(unsigned long ulSSRC,
                IRTCPNotify *piRTCPNotify,
                ISDESReport *piSDESReport=NULL);


/**
 *
 * Method Name: ~CRTCPRender() - Destructor
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: Shall deallocated and/or release all resources which were
 *              acquired over the course of runtime.  In particular, the
 *              following shall occur:
 *                ==> The reference to the local Source Description object
 *                    shall be released.
 *                ==> The reference to the local Source Report object shall be
 *                    released
 *                ==> The reference to the local Receiver Report object shall
 *                    be released
 *                ==> The reference to the local Bye Report object shall be
 *                    released
 *                ==> The reference to the Network Render object shall be
 *                    released
 *
 * Usage Notes: This shall override the virtual destructor in the base class
 *              so that deallocation specific to the derived class will be
 *              done despite the destruction being performed in the base
 *              class as part of the release.
 *
 */
    virtual ~CRTCPRender(void);


/**
 *
 * Method Name:  Initialize
 *
 *
 * Inputs:       None
 *
 * Outputs:      None
 *
 * Returns:      bool
 *
 * Description:  Performs runtime initialization that may be failure prone and
 *               therefore inappropriate for execution within a constructor.
 *               In particular, this object shall create the CReceiveReport,
 *               CSourceReport, and CByeReport objects used to store RTCP
 *               reports received associated with an outbound RTP connection.
 *
 * Usage Notes:  This is an overload of the CBaseClass Initialize() method.
 *
 */
    bool Initialize(void);

/**
 *
 * Method Name:  GetReceiveStatInterface
 *
 *
 * Inputs:       ISetReceiverStatistics  **ppiSetReceiverStats
 *
 * Outputs:      None
 *
 * Returns:      void
 *
 * Description:  This method returns the  Receiver Interface used to set
 *               statistics in the Receiver Report Object.
 *
 * Usage Notes:
 *
 */
    void GetReceiveStatInterface(ISetReceiverStatistics **ppiSetReceiverStats);

/**
 *
 * Method Name:  GetSenderStatInterface
 *
 *
 * Inputs:       ISetSenderStatistics    **ppiSetSenderStats
 *
 * Outputs:      None
 *
 * Returns:      void
 *
 * Description:  This method returns the Sender Interface used to set
 *               statistics in the Sender Report object.
 *
 * Usage Notes:
 *
 */
    void GetSenderStatInterface(ISetSenderStatistics    **ppiSetSenderStats);

/**
 *
 * Method Name:  SetNetworkRender()
 *
 *
 * Inputs:      INetworkRender *piNetworkRender   - Network Render Interface
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: This method sets the Network Render interface for the RTCP
 *              Render object.
 *
 *
 * Usage Notes:
 *
 */
    void SetNetworkRender(INetworkRender *piNetworkRender);

/**
 *
 * Method Name:  ClearNetworkRender()
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: This method clears the Network Render interface used by the
 *              RTCP Render object.
 *
 *
 * Usage Notes:
 *
 */
    void ClearNetworkRender(void);

/**
 *
 * Method Name:  ForwardRTPHeader
 *
 *
 * Inputs:      CRTPHeader *poRTPHeader - RTP Packet Header received from RTP
 *              Source
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: Takes an instance of an RTP Header object received from the
 *              RTP Source and dispatches it to the CReceiverReport object for
 *              analysis and update of receiver report statistics associated
 *              with that RTP source.
 *
 * Usage Notes: For the time being, it has been decided to process the RTP
 *              Header on the thread of the Network interface rather than
 *              waiting for the RTC Manager to process it.  If found to affect
 *              network throughput, the RTP Headers shall be queued for batch
 *              processing by the RTC Manager.
 *
 *
 */
    void ForwardRTPHeader(IRTPHeader *piRTPHeader);


/**
 *
 * Method Name: ForwardSDESReport
 *
 *
 * Inputs:   ISDESReport *piSDESReport
 *                            - Interface for generating an SDES Report
 *
 * Outputs:  None
 *
 * Returns:  unsigned long
 *                     - Returns a mask of reports sent in the composite that
 *                       included the forwarded SDES Report.
 *
 * Description: The ForwardSDESReport() method shall enable the RTC Manager to
 *              pass interfaces to SDES Reports received from participating
 *              site while acting in the mode of a conference Mixer.  The
 *              Mixer's role in this situation is to transmit these reports
 *              unchanged to others participating within a conference.  The
 *              handoff of an SDES Report to the CRTCPRender will cause the
 *              report to be transmitted to a participating site using the
 *              associated Network Render object.
 *
 * Usage Notes: The interface for the local site's Source Description Report
 *              generator is passed as an argument at construction time.
 *
 *
 */
    unsigned long ForwardSDESReport(ISDESReport *piSDESReport);


/**
 *
 * Method Name: ForwardByeReport
 *
 *
 * Inputs:   IByeReport *piByeReport - Interface for generating an Bye Report
 *
 * Outputs:  None
 *
 * Returns:  unsigned long
 *                      - Returns a mask of reports sent in the composite that
 *                        included the forwarded SDES Report.
 *
 * Description: The ForwardByeReport() method shall enable the RTC Manager to
 *              pass interfaces to Bye Reports received from participating site
 *              while acting in the mode of a conference Mixer.  The Mixer's
 *              role in this situation is to transmit these reports unchanged
 *              to others participating within a conference.  The handoff of a
 *              Bye Report to the CRTCPRender will cause the report to be
 *              transmitted to a participating site using the associated
 *              Network Render object.
 *
 * Usage Notes:
 *
 *
 */
    unsigned long ForwardByeReport(IByeReport *piByeReport);

/**
 *
 * Method Name: GenerateRTCPReports
 *
 *
 * Inputs:   unsigned char *puchRTCPReport
 *                           - RTCP Report buffer to be appended to composite
 *           unsigned long  ulReportLength
 *                           - Size of RTCP Report to be appended
 *
 * Outputs:  None
 *
 * Returns:  unsigned long
 *
 * Description: The GenerateRTCPReports() method is called periodically (every
 *              5 seconds or less) to generate and deliver a new batch of RTCP
 *              reports to a participating site.  The RTCP reports generated
 *              shall included a Sender Report (if RTP data was sent during the
 *              period), a Receiver Report (either as part of the Sender Report
 *              or individually if no RTP data were sent during the period),
 *              and one or more SDES Reports.  A counter, starting at 0, will
 *              be incremented each reporting period.  This count shall be
 *              passed in to the SDES Report format call so that the report
 *              content can be varied as per standard.
 *
 * Usage Notes:
 *
 *
 */
    unsigned long GenerateRTCPReports(unsigned char *puchRTCPReport=NULL,
                                      unsigned long  ulReportLength=0);

/**
 *
 * Method Name: GenerateByeReport
 *
 *
 * Inputs:   unsigned long aulCSRC[]  - Array of Contributing SRCs
 *           unsigned long ulCSRCs    - Number of CSRCs contain in array
 *           unsigned char *puchReason
 *                          - Reason for the Bye Report being generated
 *
 * Outputs:  None
 *
 * Returns:  unsigned long
 *
 * Description: The GenerateByeReport() method is called by the RTC Manager
 *              when an outbound RTP connection associated with the local site
 *              is being terminated or is recovering from an SSRC collision.
 *
 * Usage Notes:
 *
 *
 */
    unsigned long GenerateByeReport(unsigned long aulCSRC[],
                                    unsigned long ulCSRCs,
                                    unsigned char *puchByeReason);


/**
 *
 * Method Name:  ReassignSSRC
 *
 *
 * Inputs:      unsigned long   ulSSRC   - Source ID
 *
 * Outputs:     None
 *
 * Returns:     void
 *
 * Description: Reassigns the Source Identifier associated with an RTP
 *              connection due to collision detection and resolution.
 *              Calling of this method shall result in the resetting of the
 *              SSRC IDs of associated Sender, Receiver, and SDES Reports.
 *
 * Usage Notes:
 *
 *
 *
 */
    void ReassignSSRC(unsigned long ulSSRC);



/**
 *
 * Method Name:  GetRemoteSSRC()
 *
 *
 * Inputs:       None
 *
 * Outputs:      None
 *
 * Returns:      unsigned long - FE SSRC associated with the connection
 *
 * Description:  Retrieves the FE SSRC associated with a connection.
 *
 * Usage Notes:
 *
 */
    unsigned long GetRemoteSSRC(void);

/**
 *
 * Method Name:  GetStatistics
 *
 *
 * Inputs:    None
 *
 * Outputs:   IGetSrcDescription     **piGetSrcDescription
 *                                - Source Description Interface Pointer
 *            IGetSenderStatistics   **piSenderStatistics
 *                                - Sender Statistics Interface Pointer
 *            IGetReceiverStatistics **piReceiverStatistics
 *                                - Receiver Statistics Interface Pointer
 *            IByeInfo               **piGetByeInfo
 *                                - Interface for retrieving Bye Report Info
 *
 *
 * Returns:   None
 *
 * Description:  Returns pointers to the Sender,Receiver and Bye statistics
 *               interfaces upon request.
 *
 * Usage Notes:  This would be used by the QOS object or the RTCP Statistics
 *               object if a polling method were supported.  These objects
 *               could alternatively be informed of statistic changes via
 *               notification with the interfaces contained within the callback.
 *
 *
 */
    void GetStatistics(IGetSrcDescription     **piGetSrcDescription,
                       IGetSenderStatistics   **piSenderStatistics,
                       IGetReceiverStatistics **piReceiverStatistics,
                       IGetByeInfo            **piGetByeInfo);

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


private:        // Private Data Members

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
 * Attribute Name:  m_piNetworkRender
 *
 * Type:            INetworkRender *
 *
 * Description:  This member shall store the interface to the network render.
 *               This interface shall be used to transmit RTCP reports onto
 *               the network.
 *
 */
      INetworkRender *m_piNetworkRender;

/**
 *
 * Attribute Name:  m_piSDESReport
 *
 * Type:            ISDESReport *
 *
 * Description:  This member shall store the control interface for the
 *               Source Description report that identifies the local site.
 *
 */
      ISDESReport *m_piSDESReport;


/**
 *
 * Attribute Name:  m_poSenderReport
 *
 * Type:            CSenderReport *
 *
 * Description:  This member shall contain a pointer to a Sender report object
 *               used to store and generate RTCP Sender Reports containing
 *               performance statistics associated with an outbound RTP
 *               connection.
 *
 */
      CSenderReport *m_poSenderReport;

/**
 *
 * Attribute Name:  m_poReceiverReport
 *
 * Type:            CReceiverReport *
 *
 * Description:  This member shall contain a pointer to a Receiver Report
 *               object used to store and generate RTCP Receiver Reports
 *               containing statistics regarding the quality of the
 *               associated inbound RTP connection.
 *
 */
      CReceiverReport *m_poReceiverReport;


/**
 *
 * Attribute Name:  m_poByeReport
 *
 * Type:            CByeReport *
 *
 * Description:  This member shall contain a pointer to a Bye Report object
 *               used to generate a RTCP Bye Report for delivery to a call
 *               participant.  This Bye Report shall provide notification of
 *               an outbound RTP connection's termination.
 *
 */
      CByeReport *m_poByeReport;


/**
 *
 * Attribute Name:  m_ulReportCount
 *
 * Type:            unsigned long
 *
 * Description:  This member shall be initialized to 0 and shall be
 *               incremented monatonically at each reporting period.
 *
 */
      unsigned long m_ulReportCount;

/**
 *
 * Attribute Name:  m_ulLocalSSRC
 *
 * Type:            unsigned long
 *
 * Description:  This member shall store the local SSRC ID of the associated
 *               RTP connection.
 *
 */
      unsigned long m_ulLocalSSRC;

/**
 *
 * Attribute Name:  m_ulRemoteSSRC
 *
 * Type:            unsigned long
 *
 * Description:  This member shall store the remote SSRC ID of the
 *               associated RTP connection.
 *
 */
      unsigned long m_ulRemoteSSRC;

/**
 *
 * Attribute Name:  m_iRemoteSSRCFound
 *
 * Type:            integer
 *
 * Description:  This member indicates how many times we have detected a
 *               change in the SSRC ID of the associated RTP connection.
 *
 */
      int m_iRemoteSSRCFound;

      int mPacketCount;

};

/**
 *
 * Method Name:  GetRemoteSSRC()
 *
 *
 * Inputs:       None
 *
 * Outputs:      None
 *
 * Returns:      unsigned long - Sender's SSRC associated with the connection
 *
 * Description:  Retrieves the Sender's SSRC associated with a connection.
 *
 * Usage Notes:
 *
 */
inline unsigned long CRTCPRender::GetRemoteSSRC(void)
{

    return(m_ulRemoteSSRC);
}

/**
 *
 * Method Name:  SetNetworkRender()
 *
 *
 * Inputs:      INetworkRender *piNetworkRender   - Network Render Interface
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: This method sets the Network Render interface for the RTCP
 *               Render object.
 *
 *
 * Usage Notes:
 *
 */
inline void CRTCPRender::SetNetworkRender(INetworkRender *piNetworkRender)
{

//  Store the Network Render interface
    m_piNetworkRender = piNetworkRender;
    m_piNetworkRender->AddRef();

}

/**
 *
 * Method Name:  ClearNetworkRender()
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: This method clears the Network Render interface used by the
 *               RTCP Render object.
 *
 *
 * Usage Notes:
 *
 */
inline void CRTCPRender::ClearNetworkRender(void)
{

//  Clear the Network Render interface
    if(m_piNetworkRender)
    {
        m_piNetworkRender->Release();
        m_piNetworkRender = NULL;
    }

}

#endif
