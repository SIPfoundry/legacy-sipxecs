//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////



    // Includes
#include "rtcp/RTCPRender.h"
#ifdef INCLUDE_RTCP /* [ */


    // Constants
const int MAX_BUFFER_SIZE = 1500;

/**
 *
 * Method Name:  CRTCPRender() - Constructor
 *
 *
 * Inputs:    unsigned long   ulSSRC       - SSRC ID
 *            IRTCPNotify    *piRTCPNotify - RTCP Event Notification Interface
 *            ISDESReport    *piSDESReport - Pointer to the local Source
 *                                            Description interface
 *
 * Outputs:   None
 *
 * Returns:   None
 *
 * Description:  Performs routine CRTCPRender object initialization.
 *
 * Usage Notes:  The interface pointers passed in the constructor list shall
 *               be stored as an attribute and used to send RTCP Reports out
 *               to to the network.
 *
 *
 */
CRTCPRender::CRTCPRender(unsigned long ulSSRC,
          IRTCPNotify *piRTCPNotify, ISDESReport *piSDESReport):
          m_piNetworkRender(NULL),
          m_ulReportCount(0),
          m_ulRemoteSSRC(0),
          m_iRemoteSSRCFound(0),
          mPacketCount(0)
{

    // Store the arguments passed in the constructor as internal data members
    m_ulLocalSSRC       = ulSSRC;
    m_piRTCPNotify      = piRTCPNotify;
    m_piSDESReport      = piSDESReport;

    // Increment reference counts to interfaces passed
    if(piSDESReport)
        piSDESReport->AddRef();
    if(piRTCPNotify)
        piRTCPNotify->AddRef();
}


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
 *  ==> The reference to the local Source Description object shall be released.
 *  ==> The reference to the local Source Report object shall be released
 *  ==> The reference to the local Receiver Report object shall be released
 *  ==> The reference to the local Bye Report object shall be released
 *  ==> The reference to the Network Render object shall be released
 *
 * Usage Notes: This shall override the virtual destructor in the base class
 *              so that deallocation specific to the derived class will be
 *              done despite the destruction being performed in the base class
 *              as part of the release.
 *
 *
 */
CRTCPRender::~CRTCPRender(void)
{

    // Clear Network Render object if it has been set
    ClearNetworkRender();

    // Release object references and terminate
    if(m_piSDESReport)
        m_piSDESReport->Release();
    if(m_piRTCPNotify)
        m_piRTCPNotify->Release();

    ((ISenderReport *)m_poSenderReport)->Release();
    ((IReceiverReport *)m_poReceiverReport)->Release();
    ((IByeReport *)m_poByeReport)->Release();

}


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
 *              therefore inappropriate for execution within a constructor.
 *              In particular, this object shall create the CReceiveReport,
 *              CSourceReport, and CByeReport objects used to store RTCP
 *              reports received associated with an outbound RTP connection.
 *
 * Usage Notes:  This is an overload of the CBaseClass Initialize() method.
 *
 */
bool CRTCPRender::Initialize(void)
{

    // SDES Report already exists.  Let's simply set its SSRC for this session
    m_piSDESReport->SetSSRC(m_ulLocalSSRC);

    // Create The Receiver Report class
    if((m_poReceiverReport = new CReceiverReport(m_ulLocalSSRC)) == NULL)
    {
        osPrintf("**** FAILURE **** CRTCPRender::Initialize() -"
                                " Unable to create CReceiverReport object\n");
    }

    // Initialize Receiver Report Class
    else if(!m_poReceiverReport->Initialize())
    {
        osPrintf("**** FAILURE **** CRTCPRender::Initialize() -"
                            " Unable to initialize CReceiverReport object\n");
    }

    // Create The Sender Report class
    else if((m_poSenderReport = new CSenderReport(m_ulLocalSSRC,
                       (ISetReceiverStatistics *)m_poReceiverReport)) == NULL)
    {
        osPrintf("**** FAILURE **** CRTCPRender::Initialize() -"
                                  " Unable to create CSenderReport object\n");
    }

    //  Initialize Sender Report Class
    else if(!m_poSenderReport->Initialize())
    {
        osPrintf("**** FAILURE **** CRTCPRender::Initialize() -"
                              " Unable to Initialize CSenderReport object\n");
    }

    //  Create The Bye Report class
    else if((m_poByeReport = new CByeReport(m_ulLocalSSRC)) == NULL)
    {
        osPrintf("**** FAILURE **** CRTCPRender::Initialize() -"
                                     " Unable to Create CByeReport object\n");
    }

    // Initialize Bye Report Class
    else if(!m_poByeReport->Initialize())
    {
        osPrintf("**** FAILURE **** CRTCPRender::Initialize() -"
                                 " Unable to Initialize CByeReport object\n");
    }
    else
        return(TRUE);

    return(FALSE);

}



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
void CRTCPRender::GetReceiveStatInterface(
                                 ISetReceiverStatistics **ppiSetReceiverStats)
{

    // Assign the Receiver Report's statistical interface to the argument
    //  pointer passed
    *ppiSetReceiverStats = (ISetReceiverStatistics *)m_poReceiverReport;

    (*ppiSetReceiverStats)->AddRef();
}


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
 *               statistics in the Sedner Report object.
 *
 * Usage Notes:
 *
 */
void CRTCPRender::GetSenderStatInterface(
                                     ISetSenderStatistics **ppiSetSenderStats)
{

    // Assign the Sender Report's statistical interface to the argument
    //  pointers passed
    *ppiSetSenderStats   = (ISetSenderStatistics *)m_poSenderReport;

    (*ppiSetSenderStats)->AddRef();

}

/**
 *
 * Method Name:  ForwardRTPHeader
 *
 * Inputs:   CRTPHeader *poRTPHeader
 *                                - RTP Packet Header received from RTP Source
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: Takes an instance of an RTP Header object received from the RTP
 *              Source and dispatches it to the CReceiverReport object for
 *              analysis and update of receiver report statistics associated
 *              with that RTP source.
 *
 * Usage Notes: For the time being, it has been decided to process the RTP
 *              Header on the thread of the Network interface rather than
 *              waiting for the RTC Manager to process it.  If found to
 *              affect network throughput, the RTP Headers shall be queued
 *              for batch processing by the RTC Manager.
 *
 *
 */
void CRTCPRender::ForwardRTPHeader(IRTPHeader *piRTPHeader)
{

    unsigned long ulRemoteSSRC = piRTPHeader->GetSSRC();

    // Bump refernece count of RTP Header interface
    piRTPHeader->AddRef();

    // Check to see whether this is the first packet establishing our FE SSRC
    if(m_iRemoteSSRCFound == 0)
    {
/*
        osPrintf(">> CRTCPRdr::FwdRTPHdr():"
                            " New SSRC 0x%lX detected\n", ulRemoteSSRC);
*/
        m_ulRemoteSSRC = ulRemoteSSRC;
        m_iRemoteSSRCFound++;
    }

    else if(m_ulRemoteSSRC != ulRemoteSSRC)
    {
        // SSRC ID has changed
        if (m_iRemoteSSRCFound < 20)
        {
/*
            osPrintf(">> CRTCPRdr::FwdRTPHdr(): SSRC Changed"
               " from 0x%lX to 0x%lX\n", m_ulRemoteSSRC, ulRemoteSSRC);
*/
        }
        m_ulRemoteSSRC = ulRemoteSSRC;
        m_iRemoteSSRCFound++;
    }

    mPacketCount++;
    if (0 == (mPacketCount & ((1<<11)-1))) {
        m_iRemoteSSRCFound = (m_iRemoteSSRCFound ? 1 : 0);
    }

    // Let's simply hand this off to the CReceiverReport object so that it
    //  may perform statistics calculations regarding the performance of
    //  the RTP channel.
    m_poReceiverReport->SetRTPStatistics(piRTPHeader);

    // Release the reference to RTP Header interface
    piRTPHeader->Release();

}


/**
 *
 * Method Name: ForwardSDESReport
 *
 *
 * Inputs:   ISDESReport *piSDESReport
                                     - Interface for generating an SDES Report
 *
 * Outputs:  None
 *
 * Returns:  unsigned long - Returns a mask of reports sent in the composite
 *                           that included the forwarded SDES Report.
 *
 * Description: The ForwardSDESReport() method shall enable the RTC Manager
 *              to pass interfaces to SDES Reports received from participating
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
unsigned long CRTCPRender::ForwardSDESReport(ISDESReport *piSDESReport)
{
    // Let's format this report into an octet string that so that it may
    //  retransmitted as an RTCP PDU.
    unsigned char uchSDESReport[MAX_BUFFER_SIZE];
    unsigned long ulReportLength;
    unsigned long ulReportMask;

    // Bump reference count of SDES Report interface
    piSDESReport->AddRef();

    // Format the SDES Report with the changes that were received
    ulReportLength = piSDESReport->FormatSDESReport(TRUE,
                                                    SDES_CHANGES,
                                                    uchSDESReport,
                                                    MAX_BUFFER_SIZE);

    // Let's concatentate the SDES Report onto the end of one final
    //  composite report sent
    ulReportMask = GenerateRTCPReports(uchSDESReport, ulReportLength);


    // Release the reference to SDES Report interface
    piSDESReport->Release();

    return(ulReportMask);
}


/**
 *
 * Method Name: ForwardByeReport
 *
 *
 * Inputs:   IByeReport *piByeReport - Interface for generating an Bye Report
 *
 * Outputs:  None
 *
 * Returns:  unsigned long - Returns a mask of reports sent in the composite
 *                           that included the forwarded Bye Report.
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
unsigned long CRTCPRender::ForwardByeReport(IByeReport *piByeReport)
{

    // Let's format this report into an octet string that so that it may
    //  retransmitted as an RTCP PDU.
    unsigned char uchByeReport[MAX_BUFFER_SIZE];
    unsigned long ulReportLength;
    unsigned long ulReportMask;

    // Bump refernece count of Bye Report interface
    piByeReport->AddRef();

    // Format the Bye Report
    ulReportLength =
                  piByeReport->FormatByeReport(uchByeReport, MAX_BUFFER_SIZE);

    // Let's concatentate the Bye Report onto the end of one final
    //  composite report sent
    ulReportMask = GenerateRTCPReports(uchByeReport, ulReportLength);

    // Release the reference to Bye Report interface
    piByeReport->Release();

    return(ulReportMask);

}



/**
 *
 * Method Name: GenerateRTCPReports
 *
 *
 * Inputs:   unsigned char *puchAppendReport
 *                            - RTCP Report buffer to be appended to composite
 *           unsigned long  ulAppendLength
 *                            - Size of RTCP Report to be appended
 *
 * Outputs:  None
 *
 * Returns:  unsigned long
 *
 * Description: The GenerateRTCPReports() method is called periodically (every
 *              5 seconds or less) to generate and deliver a new batch of RTCP
 *              reports to a participating site.  The RTCP reports generated
 *              shall included a Sender Report (if RTP data was sent during
 *              the period), a Receiver Report (either as part of the Sender
 *              Report or individually if no RTP data were sent during the
 *              period), and one or more SDES Reports.  A counter, starting at
 *              0, will be incremented each reporting period.  This count
 *              shall be passed in to the SDES Report format call so that the
 *              report content can be varied as per standard.
 *
 * Usage Notes:
 *
 *
 */
unsigned long CRTCPRender::GenerateRTCPReports(unsigned char *puchAppendReport,
                                               unsigned long ulAppendLength)
{

    // During this reporting cycle, we shall generate RTCP Sender (if
    //  necessary), Receiver, and SDES Report.
    unsigned long ulReportMask   = 0;
    unsigned long ulReportLength = 0;
    unsigned char uchRTCPReport[MAX_BUFFER_SIZE];


    // Generate Sender Report
    // Let's check to see whether any media was sent since the last reporting
    //  period.
    if(m_poSenderReport->WasMediaSent())
    {
        // Create a Sender Report with the statistics accumulated over the
        //  last reporting period
        ulReportLength = m_poSenderReport->FormatSenderReport(uchRTCPReport,
                                                             MAX_BUFFER_SIZE);
        ulReportMask |= RTCP_SR_SENT;
    }

    // Now we will prepare to formulate a reception report.  This report will
    //  either be appended to a sender report or sent as its own report.
    ulReportLength += m_poReceiverReport->FormatReceiverReport(
                   ulReportLength ? TRUE : FALSE, // Composite Report Size
                   uchRTCPReport+ulReportLength,  // Offset into Report Buffer
                   MAX_BUFFER_SIZE);              // Buffer available
    ulReportMask |= RTCP_RR_SENT;


    // Generate SDES Report
    // Let's increment the report count.  This increases monatonically over
    //  each reporting period and is used to determine the content of
    //  SDES reports.
    m_ulReportCount++;

    // Format the SDES Report with the changes that were received
    ulReportLength += m_piSDESReport->FormatSDESReport(TRUE, m_ulReportCount,
                             uchRTCPReport + ulReportLength, MAX_BUFFER_SIZE);
    ulReportMask |= RTCP_SDES_SENT;

    // Check and see whether there was a Report that was passed in that should
    //  be appended to this outgoing report.
    if(puchAppendReport)
    {
        memcpy(uchRTCPReport + ulReportLength,
                                            puchAppendReport, ulAppendLength);
        ulReportLength += ulAppendLength;
    }


    // Let's take the formatted octet string and transmit it to a deserving
    // recipient through use of the Network Render object's service interface.
    if(m_piNetworkRender &&
                      !m_piNetworkRender->Send(uchRTCPReport, ulReportLength))
    {
        // Log a meaningful error
        return(0);
    }

    return(ulReportMask);
}



/**
 *
 * Method Name: GenerateByeReport
 *
 *
 * Inputs:   unsigned long aulCSRC[]    - Array of Contributing SRCs
 *           unsigned long ulCSRCs      - Number of CSRCs contain in array
 *           unsigned char *puchReason
 *                               - A Reason for the Bye Report being generated
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The GenerateByeReport() method is called by the RTC Manager
 *              when an outbound RTP connection associated with the local site
 *              is being terminate or is recovering from an SSRC collision.
 *
 * Usage Notes:
 *
 *
 */
unsigned long CRTCPRender::GenerateByeReport(unsigned long aulCSRC[],
                          unsigned long ulCSRCs, unsigned char *puchByeReason)

{

    // Let's format this report into an octet string that so that it may
    //  retransmitted as an RTCP PDU.
    unsigned char uchByeReport[MAX_BUFFER_SIZE];
    unsigned long ulReportLength;
    unsigned long ulReportMask;

    // Set the reason for RTP connection termination.
    m_poByeReport->SetReason(puchByeReason);

    // Load the CSRC list into the Bye Report
    m_poByeReport->SetCSRC(aulCSRC, ulCSRCs);

    // Format the Bye Report
    ulReportLength =
                m_poByeReport->FormatByeReport(uchByeReport, MAX_BUFFER_SIZE);

    // Let's concatentate the Bye Report onto the end of one final composite
    //  report sent
    if((ulReportMask = GenerateRTCPReports(uchByeReport, ulReportLength)) != 0)
        ulReportMask |= RTCP_BYE_SENT;

    return(ulReportMask);

}

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
 *              connection due to collision detection and resolution.  Calling
 *              of this method shall result in the resetting of the SSRC IDs
 *              of associated Sender, Receiver, and SDES Reports.
 *
 * Usage Notes:
 *
 *
 *
 */
void CRTCPRender::ReassignSSRC(unsigned long ulSSRC)
{

    // Reset the locally stored SSRC ID
    m_ulLocalSSRC            = ulSSRC;

    // Reset the SSRC's of those object which the CRTCPRender controls; namely
    //  the Sender, Receiver, and Bye objects.  The Source Description object
    //  shall be reset by the RTC Manager since it owns responsibility for this
    //  across many RTP connections.
    m_piSDESReport->SetSSRC(ulSSRC);
    m_poSenderReport->SetSSRC(ulSSRC);
    m_poReceiverReport->SetSSRC(ulSSRC);
    m_poByeReport->SetSSRC(ulSSRC);

}


/**
 *
 * Method Name:  GetStatistics
 *
 *
 * Inputs:   None
 *
 * Outputs:  IGetSrcDescription     **piGetSrcDescription
 *                                    - Source Description Interface Pointer
 *           IGetSenderStatistics   **piSenderStatistics
 *                                    - Sender Statistics Interface Pointer
 *           IGetReceiverStatistics **piReceiverStatistics
 *                                    - Receiver Statistics Interface Pointer
 *           IByeInfo               **piGetByeInfo
 *                                   - Interface for retrieving Bye Report Info
 *
 *
 * Returns:  None
 *
 * Description:  Returns pointers to the Sender,Receiver and Bye statistics
 *               interfaces upon request.
 *
 * Usage Notes:  This would be used by the QOS object or the RTCP Statistics
 *               object if a polling method were supported.  These objects
 *               could alternatively be informed of statistic changes via
 *               notification with the interfaces contained within the
 *               callback.
 *
 *
 */
void CRTCPRender::GetStatistics(IGetSrcDescription     **piGetSrcDescription,
                                IGetSenderStatistics   **piSenderStatistics,
                                IGetReceiverStatistics **piReceiverStatistics,
                                IGetByeInfo            **piGetByeInfo)
{


    // Load the pointers to the corresponding statistics retrieval interfaces
    if(piGetSrcDescription != NULL)
    {
        *piGetSrcDescription =
                   (IGetSrcDescription *)m_piSDESReport->GetAccessInterface();
    }

    if(piSenderStatistics != NULL)
    {
        *piSenderStatistics = (IGetSenderStatistics *)m_poSenderReport;
        (*piSenderStatistics)->AddRef();
    }

    if(piReceiverStatistics != NULL)
    {
        *piReceiverStatistics = (IGetReceiverStatistics *)m_poReceiverReport;
        (*piReceiverStatistics)->AddRef();
    }

    if(piGetByeInfo != NULL)
    {
        *piGetByeInfo = (IGetByeInfo *)m_poByeReport;
        (*piGetByeInfo)->AddRef();
    }

}
#endif /* INCLUDE_RTCP ] */
