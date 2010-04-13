//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


//  Border Guard
#ifndef _IRTCPRender_h
#define _IRTCPRender_h

#include "rtcp/RtcpConfig.h"

//  Includes
#include "ISDESReport.h"
#include "IByeReport.h"

/**
 *
 * Interface Name:  IRTCPRender
 *
 * Inheritance:     None
 *
 *
 * Description:     The IRTCPRender interface allows the RTC Manager to
 *                  control the content and generation of outbound RTCP
 *                  Sender, Receiver, SDES, and BYE Reports.
 *
 * Notes:
 *
 */


interface IRTCPRender : public IBaseClass
{

//  Public Methods
public:


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
    virtual void
      GetReceiveStatInterface(ISetReceiverStatistics **ppiSetReceiverStats)=0;

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
    virtual void
        GetSenderStatInterface(ISetSenderStatistics **ppiSetSenderStats)=0;


/**
 *
 * Method Name: ForwardSDESReport
 *
 *
 * Inputs:   ISDESReport *piSDESReport - Interface for generating an SDES Report
 *
 * Outputs:  None
 *
 * Returns:  unsigned long
 *             Returns a mask of reports sent in the composite that included
 *             the forwarded SDES Report.
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
    virtual unsigned long ForwardSDESReport(ISDESReport *piSDESReport)=0;


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
 *             Returns a mask of reports sent in the composite that included
 *             the forwarded SDES Report.
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
    virtual unsigned long ForwardByeReport(IByeReport *piByeReport)=0;

/**
 *
 * Method Name: GenerateRTCPReports
 *
 *
 * Inputs:   unsigned char *puchRTCPReport
 *             RTCP Report buffer to be appended to composite
 *           unsigned long  ulReportLength
 *             Size of RTCP Report to be appended
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
 * Usage Notes: Returns a mask indicating the types of reports generated.
 *
 */
    virtual unsigned long
        GenerateRTCPReports(unsigned char *puchRTCPReport=NULL,
                            unsigned long  ulReportLength=0) = 0;


/**
 *
 * Method Name: GenerateByeReport
 *
 *
 * Inputs:   unsigned long aulCSRC[]    - Array of Contributing SRCs
 *           unsigned long ulCSRCs      - Number of CSRCs contain in array
 *           unsigned char *puchReason  -
 *                             A Reason for the Bye Report being generated
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The GenerateByeReport() method is called by the RTC Manager
 *              when an outbound RTP connection associated with the local
 *              site is being terminate or is recovering from an SSRC
 *              collision.
 *
 * Usage Notes:
 *
 *
 */
    virtual unsigned long
        GenerateByeReport(unsigned long aulCSRC[], unsigned long ulCSRCs,
                          unsigned char *puchByeReason) = 0;

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
    virtual unsigned long GetRemoteSSRC(void)=0;

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
    virtual void ReassignSSRC(unsigned long ulSSRC) = 0;


};

#endif
