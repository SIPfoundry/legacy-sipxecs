//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


//  Border Guard
#ifndef _RTCPSession_h
#define _RTCPSession_h

#include "rtcp/RtcpConfig.h"

//  Includes
#include "TLinkedList.h"
#include "RTCPConnection.h"
#include "IRTCPRender.h"
#include "INetworkRender.h"
#include "IRTCPSession.h"
#include "IRTCPNotify.h"


/**
 *
 * Class Name:  CRTCPSession
 *
 * Inheritance: CBaseClass           - Base Class Implementation
 *
 *
 * Interfaces:  IRTCPSession         - RTCP Session Control Interface
 *
 * Description: The CRTCPSession Class shall create, manage, and terminate
 *              instances of RTCP Connection objects used to monitor and report
 *              upon a corresponding number of RTP Connections composing an RTP
 *              Session.
 *
 *
 * Notes:       CRTCPSession is derived from CBaseClass which provides basic
 *              Initialization and reference counting support.
 *
 */
class CRTCPSession:
           public CBaseClass,   // Inherits CBaseClass implementation
           public CTLinkedList<CRTCPConnection *>,
                                // Inherit Connection Linked List Services
           public IRTCPSession, // Interface exposed for RTCP Session Control
           public IRTCPNotify   // Interface exposed for RTCP Notifications
 {

//  Public Methods
public:

/**
 *
 * Method Name:  CRTCPSession() - Constructor
 *
 *
 * Inputs:   unsigned long ulSSRC      - SSRC ID of the RTCP Session
 *           IRTCPNotify *piRTCPNotify - RTCP Event Notification Interface
 *           ISDESReport *piSDESReport
 *                             - Local Source Description Report Interface
 *
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description:  Performs routine assignment of constructor arguments to
 *               internal attributes.
 *
 * Usage Notes:
 *
 */
    CRTCPSession(unsigned long ulSSRC,
                 IRTCPNotify *piRTCPNotify,
                 ISDESReport *piSDESReport);

/**
 *
 * Method Name: ~CRTCPSession() - Destructor
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
 *                ==> The list containing RTCP Connections objects shall be
 *                    drained with the reference to each object released.
 *
 * Usage Notes:
 *
 *
 */
    ~CRTCPSession(void);



/**
 *
 * Method Name:  CreateRTCPConnection()
 *
 *
 * Inputs:       None
 *
 * Outputs:      None
 *
 * Returns:      bool
 *
 * Description:  Creates an RTCP Connection used to monitor and report upon
 *               the identify and performance of an RTP Connection.
 *
 * Usage Notes:
 *
 */
    IRTCPConnection *CreateRTCPConnection(void);


/**
 *
 * Method Name:  TerminateRTCPConnection()
 *
 *
 * Inputs:   IRTCPConnection *piRTCPConnection
 *                  - The Connection Interface pointer returned on creation
 *           unsigned char *puchReason
 *                  - Reason for Termination
 *
 * Outputs:  None
 *
 * Returns:  bool
 *
 * Description:  Terminates an RTCP Connection previously used to monitor and
 *               report upon the identify and performance of an RTP Connection.
 *
 * Usage Notes:
 *
 */
    bool TerminateRTCPConnection(IRTCPConnection *piRTCPConnection);

/**
 *
 * Method Name: TerminateAllConnections
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     bool

 *
 * Description:  The TerminateAllConnections() method shall manage the
 *               termination of all RTCP connections.  This shall include the
 *               graceful release of all associated objects as well as the
 *               deallocation of all resources associated with each contained
 *               RTCP connection.
 *
 *
 * Usage Notes:
 *
 *
 */
    void TerminateAllConnections(void);

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
 * Description:  Reassigns the Source Identifier associated with an RTP session
 *               due to collision detection and resolution.  Calling of this
 *               method shall result in the resetting of the SSRC IDs of
 *               associated Sender, Receiver, and SDES Reports.
 *
 * Usage Notes:
 *
 *
 *
 */
    void ReassignSSRC(unsigned long ulSSRC, unsigned char *puchReason=NULL);

/**
 *
 * Method Name: ForwardSDESReport
 *
 *
 * Inputs:   IGetSrcDescription *piGetSrcDescription
 *                          - Interface for getting SDES Report Statistics
 *           IRTCPConnection  *piRTCPConnection
 *                          - Interface to RTCP Connection originating SDES
 *
 * Outputs:  None
 *
 * Returns:  None
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
    void ForwardSDESReport(IGetSrcDescription *piGetSrcDescription,
                           IRTCPConnection  *piRTCPConnection);


/**
 *
 * Method Name: ForwardByeReport
 *
 *
 * Inputs:      IGetByeInfo     *piGetByeInfo
 *                      - Interface used to retrieve Bye Report information
 *              IRTCPConnection *piRTCPConnection
 *                      - Interface to RTCP Connection originating Bye
 *
 * Outputs:     None
 *
 * Returns:     None
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
    void ForwardByeReport(IGetByeInfo      *piGetByeInfo,
                          IRTCPConnection  *piRTCPConnection);

/**
 *
 * Method Name: CheckLocalSSRCCollisions
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     void
 *
 * Description: Check that our local SSRC is not colliding with one fo the
 *              SSRCs of a participating site.
 *
 * Usage Notes:
 *
 *
 *
 */
    void CheckLocalSSRCCollisions(void);

/**
 *
 * Method Name: CheckRemoteSSRCCollisions
 *
 *
 * Inputs:   IRTCPConnection *piRTCPConnection  - Connection Interface
 *
 * Outputs:  None
 *
 * Returns:  void
 *
 * Description: Check that other remote SSRCs aren't colliding with each other.
 *
 * Usage Notes:
 *
 *
 *
 */
    void CheckRemoteSSRCCollisions(IRTCPConnection *piRTCPConnection);

/**
 *
 * Method Name:  SetMixerMode()
 *
 *
 * Inputs:   MIXER_MODE_ET etMixerMode
 *              - Identifies whether the audio mixer is enabled or disabled
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description:  Sets the audio mixer mode for a session.
 *
 * Usage Notes:
 *
 */
    void SetMixerMode(MIXER_MODE_ET etMixerMode);

/**
 *
 * Method Name:  GetMixerMode()
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  MIXER_MODE_ET - The audio mixer mode supported by the session
 *
 * Description:  Retrieves the audio mixer mode supported by the session.
 *
 * Usage Notes:
 *
 */
    MIXER_MODE_ET GetMixerMode(void);

/**
 *
 * Method Name:  GetSessionID()
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  unsigned long - The ID for this Sesson
 *
 * Description:  Retrieves the Session ID associated with a session.
 *
 * Usage Notes:
 *
 */
    unsigned long GetSessionID(void);

/**
 *
 * Method Name:  GetSSRC()
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  unsigned long - Local SSRC associated with the session
 *
 * Description:  Retrieves the SSRC associated with a session.
 *
 * Usage Notes:
 *
 */
    unsigned long GetSSRC(void);

/**
 *
 * Method Name:  GetEventInterest()
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  unsigned long - Mask of Event Interests
 *
 * Description: The GetEventInterest() event method shall allow the dispatcher
 *              of notifications to access the event interests of a subscriber
 *              and use these wishes to dispatch RTCP event notifications
 *
 * Usage Notes:
 *
 */
    unsigned long GetEventInterest(void);


/**
 *
 * Method Name: GetFirstConnection
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  IRTCPSession *
 *             - Returns an interface for use in controlling an RTCP Session
 *
 *
 * Description: The GetFirstConnection shall retrieve the first instantiation
 *              of an RTCP Session interface on the RTC Manager's collection
 *              list.
 *
 *
 * Usage Notes:
 *
 *
 */
    IRTCPConnection *GetFirstConnection(void);

/**
 *
 * Method Name: GetNextConnection
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  IRTCPSession *
 *             - Returns an interface for use in controlling an RTCP Session
 *
 *
 * Description: The GetNextConnection shall retrieve the next instantiation of
 *              an RTCP Conection interface on an RTCP Session's collection
 *              list.
 *
 *
 * Usage Notes:
 *
 *
 */
    IRTCPConnection *GetNextConnection(void);


/**
 *
 * Method Name: CheckConnection
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  IRTCPConnection *
 *             - Returns an interface for use in controlling an RTCP Connection
 *
 *
 * Description: The CheckConnection shall check an instance of an RTCP
 *              Connection interface on the RTCP Session's collection list.
 *
 *
 * Usage Notes:
 *
 *
 */
    IRTCPConnection *CheckConnection(IRTCPConnection *);


/**
 *
 * Method Name:  NewSDES()
 *
 *
 * Inputs:   IGetSrcDescription *piGetSrcDescription
 *                         - Interface to the new Source Description info
 *           IRTCPConnection        *piRTCPConnection
 *                         - Interface to associated RTCP Connection
 *           IRTCPSession           *piRTCPSession
 *                         - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The NewSDES() event method shall inform the RTC Manager of a
 *              new Source Description and shall include the IGetSrcDescription
 *              interface for accessing the contents of this new Source
 *              Description.  The RTCP Session shall react by adding this new
 *              SDES interface to the Source DescriptionList of associated
 *              CRTCPRenders so that the new SDES information may be forwarded
 *              to other call participants (i.e. Mixer scenario).
 *
 *
 * Usage Notes:
 *
 */
    void NewSDES(IGetSrcDescription *piGetSrcDescription,
                 IRTCPConnection    *piRTCPConnection,
                 IRTCPSession       *piRTCPSession=NULL);

/**
 *
 * Method Name:  UpdatedSDES()
 *
 *
 * Inputs:   IGetSrcDescription *piGetSrcDescription
 *                           - Interface to the new Source Description info
 *           unsigned long       ulChangeMask
 *                           - The SDES fields that were subject to change
 *           IRTCPConnection    *piRTCPConnection
 *                           - Interface to associated RTCP Connection
 *           IRTCPSession       *piRTCPSession
 *                           - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The UpdatedSDES() event method shall inform the RTCP Session
 *              of a change in Source Description and shall include the
 *              IGetSrcDescription interface for accessing the contents of
 *              this updated Source Description.
 *
 * Usage Notes:
 *
 */
    void UpdatedSDES(IGetSrcDescription *piGetSrcDescription,
                     unsigned long       ulChangeMask,
                     IRTCPConnection    *piRTCPConnection,
                     IRTCPSession       *piRTCPSession=NULL);


/**
 *
 * Method Name:  SenderReportReceived()
 *
 *
 * Inputs:   IGetSenderStatistics *piGetSenderStatistics
 *                                  - Interface to the Sender Statistics
 *           IRTCPConnection      *piRTCPConnection
 *                                  - Interface to associated RTCP Connection
 *           IRTCPSession         *piRTCPSession
 *                                  - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The SenderReportReceived() event method shall inform the
 *              recipient of a change in Sender Statistics and shall include
 *              the IGetSenderStatistics interface for accessing the
 *              contents of this updated Sender Report.
 *
 * Usage Notes:
 *
 */
    void SenderReportReceived(IGetSenderStatistics *piGetSenderStatistics,
                              IRTCPConnection    *piRTCPConnection,
                              IRTCPSession       *piRTCPSession=NULL);


/**
 *
 * Method Name:  ReceiverReportReceived()
 *
 *
 * Inputs:   IGetReceiverStatistics *piGetReceiverStatistics
 *                                    - Interface to the Receiver Statistics
 *           IRTCPConnection        *piRTCPConnection
 *                                    - Interface to associated RTCP Connection
 *           IRTCPSession           *piRTCPSession
 *                                    - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The ReceiverReportReceived() event method shall inform the
 *              recipient of a change in Receiver Statistics and shall
 *              include the IGetReceiverStatistics interface for accessing the
 *              contents of this updated Receiver Report.
 *
 * Usage Notes:
 *
 */
  void ReceiverReportReceived(IGetReceiverStatistics *piGetReceiverStatistics,
                                IRTCPConnection        *piRTCPConnection,
                                IRTCPSession           *piRTCPSession=NULL);


/**
 *
 * Method Name:  ByeReportReceived()
 *
 *
 * Inputs:   IGetByeInfo      *piGetByeInfo
 *                   - Interface used to retrieve Bye Report information
 *           IRTCPConnection  *piRTCPConnection
 *                   - Interface to associated RTCP Connection
 *           IRTCPSession     *piRTCPSession
 *                   - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The ByeReportReceived() event method shall inform the recipient
 *              of the discontinuation of an SSRC.  This may result from an
 *              SSRC collision or the termination of an associated RTP
 *              connection.  The RTCP Session shall react by forwarding this
 *              Bye Report to other connections within the session whne in a
 *              audio conference call (i.e. Mixer scenario).
 *
 * Usage Notes:
 *
 */
    void ByeReportReceived(IGetByeInfo *piGetByeInfo,
                           IRTCPConnection    *piRTCPConnection,
                           IRTCPSession       *piRTCPSession=NULL);

/**
 *
 * Method Name:  SDESReportSent()
 *
 *
 * Inputs:   IGetSrcDescription *piGetSrcDescription
 *                                - Interface to the local Source Description
 *           IRTCPConnection    *piRTCPConnection
 *                                - Interface to associated RTCP Connection
 *           IRTCPSession       *piRTCPSession
 *                                - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The SDESReportSent() event method shall inform the recipient
 *              of a newly transmitted SDES Report and shall include the
 *              IGetSrcDescription interface for accessing the contents of
 *              this transmitted SDES Report.
 *
 * Usage Notes:
 *
 */
    void SDESReportSent(IGetSrcDescription *piGetSrcDescription,
                        IRTCPConnection    *piRTCPConnection,
                        IRTCPSession       *piRTCPSession=NULL);

/**
 *
 * Method Name:  SenderReportSent()
 *
 *
 * Inputs:   IGetSenderStatistics *piGetSenderStatistics
 *                                  - Interface to the Sender Statistics
 *           IRTCPConnection      *piRTCPConnection
 *                                  - Interface to associated RTCP Connection
 *           IRTCPSession         *piRTCPSession
 *                                  - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The SenderReportSent() event method shall inform the recipient
 *              of a newly transmitted Sender Report and shall include the
 *              IGetSenderStatistics interface for accessing the contents of
 *              this transmitted Sender Report.
 *
 * Usage Notes:
 *
 */
    void SenderReportSent(IGetSenderStatistics *piGetSenderStatistics,
                          IRTCPConnection    *piRTCPConnection,
                          IRTCPSession       *piRTCPSession=NULL);


/**
 *
 * Method Name:  ReceiverReportSent()
 *
 *
 * Inputs:   IGetReceiverStatistics *piGetReceiverStatistics
 *                                    - Interface to the Receiver Statistics
 *           IRTCPConnection        *piRTCPConnection
 *                                    - Interface to associated RTCP Connection
 *           IRTCPSession           *piRTCPSession
 *                                    - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The ReceiverReportSent() event method shall inform the
 *              recipient of a newly transmitted Receiver Report and shall
 *              include the IGetReceiverStatistics interface for accessing
 *              the contents of this transmitted Receiver Report.
 *
 * Usage Notes:
 *
 */
    void ReceiverReportSent(IGetReceiverStatistics *piGetReceiverStatistics,
                            IRTCPConnection        *piRTCPConnection,
                            IRTCPSession           *piRTCPSession=NULL);


/**
 *
 * Method Name:  ByeReportSent()
 *
 *
 * Inputs:   IGetByeInfo     *piGetByeInfo
 *                                - Interface used to retrieve Bye Report
 *                                  information
 *           IRTCPConnection *piRTCPConnection
 *                                - Interface to associated RTCP Connection
 *           IRTCPSession    *piRTCPSession
 *                                - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The ByeReportSent() event method shall inform the recipient of
 *              a newly transmitted BYE Report and shall include the SSRC ID
 *              and reason to identify the connection and the cause of
 *              termination.
 *
 * Usage Notes:
 *
 */
    void ByeReportSent(IGetByeInfo      *piGetByeInfo,
                       IRTCPConnection  *piRTCPConnection,
                       IRTCPSession     *piRTCPSession=NULL);

/**
 *
 * Method Name: RTCPReportingAlarm
 *
 *
 * Inputs:   IRTCPConnection *piRTCPConnection
 *                             - Interface to associated RTCP Connection
 *           IRTCPSession    *piRTCPSession
 *                             - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The RTCPReportingAlarm() event method shall inform the
 *              recipient of the expiration of the RTCP Reporting Timer.
 *              This signals the generation of the next round of RTCP Reports.
 *
 * Usage Notes:
 *
 *
 */
    void RTCPReportingAlarm(IRTCPConnection *piRTCPConnection,
                            IRTCPSession *piRTCPSession=NULL);


/**
 *
 * Method Name:  RTCPConnectionStopped()
 *
 *
 * Inputs:   IRTCPConnection *piRTCPConnection
 *                             - Interface to associated RTCP Connection
 *           IRTCPSession    *piRTCPSession
 *                             - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The RTCPConnectionStopped() event method shall inform the
 *              recipient of the imminent suspension of an RTCP outbound
 *              connection.  This will allow whatever connection related
 *              operations to be suspended until it again resumes.
 *
 *
 * Usage Notes:
 *
 */
    void RTCPConnectionStopped(IRTCPConnection     *piRTCPConnection,
                                 IRTCPSession        *piRTCPSession=NULL);



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
 * Method Name: ResetAllConnections
 *
 *
 * Inputs:      unsigned char *puchReason     - Reason for Termination
 *
 * Outputs:     None
 *
 * Returns:     void

 *
 * Description: The ResetAllConnections() method shall manage the termination
 *              of all RTCP connections.  This shall include the graceful
 *              release of all associated objects as well as the deallocation
 *              of all resources associated with each contained RTCP connection.
 *
 *
 * Usage Notes:
 *
 *
 */
    void ResetAllConnections(unsigned char *puchReason);


private:        // Private Data Members


/**
 *
 * Attribute Name:  m_ulSSRC
 *
 * Type:            unsigned long
 *
 * Description:  This member shall store the SSRC associated with this session
 *
 */
      unsigned long m_ulSSRC;

/**
 *
 * Attribute Name:  m_ulSessionID
 *
 * Type:            unsigned long
 *
 * Description:  This member shall store a reference to the session number.
 *               This is only for identification purposes from debug tracking
 *
 */
      unsigned long m_ulSessionID;

/**
 *
 * Attribute Name:  m_ulEventInterest
 *
 * Type:            unsigned long
 *
 * Description:  This member shall store the events of interest for this
 *               Session object.
 *
 */
      unsigned long m_ulEventInterest;

/**
 *
 * Attribute Name:  m_etMixerMode
 *
 * Type:            MIXER_MODE_ET
 *
 * Description:  This member shall store the mixer mode of this Session object.
 *
 */
      MIXER_MODE_ET m_etMixerMode;

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
 * Attribute Name:  m_piSDESReport
 *
 * Type:            ISDESReport *
 *
 * Description:  This member shall cache the control interface for the Source
 *               Description report that identifies the local site.
 *
 */
      ISDESReport *m_piSDESReport;

};


/**
 *
 * Method Name:  GetSSRC()
 *
 *
 * Inputs:       None
 *
 * Outputs:      None
 *
 * Returns:      unsigned long - Local SSRC associated with the session
 *
 * Description:  Retrieves the SSRC associated with a session.
 *
 * Usage Notes:
 *
 */
inline unsigned long CRTCPSession::GetSSRC(void)
{
    return(m_ulSSRC);
}


/**
 *
 * Method Name:  GetEventInterest()
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long - Mask of Event Interests
 *
 * Description: The GetEventInterest() event method shall allow the dispatcher
 *              of notifications to access the event interests of a subscriber
 *              and use these wishes to dispatch RTCP event notifications
 *
 * Usage Notes:
 *
 */
inline unsigned long CRTCPSession::GetEventInterest(void)
{

    return(m_ulEventInterest);
}

/**
 *
 * Method Name: GetFirstConnection
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  IRTCPConnection *
 *            - Returns an interface for use in controlling an RTCP Connection
 *
 *
 * Description: The GetFirstConnection shall retrieve the first instantiation
 *              of an RTCP Connection interface on the RTCP Session's
 *              collection list.
 *
 *
 * Usage Notes:
 *
 *
 */
inline IRTCPConnection *CRTCPSession::GetFirstConnection(void)
{

    return(GetFirstEntry());

}

/**
 *
 * Method Name: GetNextConnection
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     IRTCPConnection *
 *            - Returns an interface for use in controlling an RTCP Connection
 *
 *
 * Description: The GetNextConnection shall retrieve the next instantiation
 *              of an RTCP Conection interface on an RTCP Session's
 *              collection list.
 *
 *
 * Usage Notes:
 *
 *
 */
inline IRTCPConnection *CRTCPSession::GetNextConnection(void)
{

    return(GetNextEntry());

}

/**
 *
 * Method Name: CheckConnection
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     IRTCPConnection *
 *            - Returns an interface for use in controlling an RTCP Connection
 *
 *
 * Description: The CheckConnection shall check an instance of an RTCP
 *              Connection interface on the RTCP Session's collection list.
 *
 *
 * Usage Notes:
 *
 *
 */
inline IRTCPConnection
             *CRTCPSession::CheckConnection(IRTCPConnection *piRTCPConnection)
{
    // Attempt to retrieve the connection from the collection
    CRTCPConnection *poRTCPConnection =
                           GetEntry((CRTCPConnection *)piRTCPConnection);

    // See whether the Render object is still active
    if(poRTCPConnection)
    {
        IRTCPRender *piRtcpRender = poRTCPConnection->GetRenderInterface();
        if(piRtcpRender)
        {
            piRtcpRender->Release();
            return((IRTCPConnection *)poRTCPConnection);
        }
    }

    return(NULL);

}


/**
 *
 * Method Name:  SetMixerMode()
 *
 *
 * Inputs:   MIXER_MODE_ET  etMixerMode
 *                - Identifies whether the audio mixer is enabled or disabled
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description:  Sets the audio mixer mode for a session.
 *
 * Usage Notes:
 *
 */
inline void CRTCPSession::SetMixerMode(MIXER_MODE_ET etMixerMode)
{

//  Set the mixer attribute for this session
    m_etMixerMode = etMixerMode;

}

/**
 *
 * Method Name:  GetMixerMode()
 *
 *
 * Inputs:       None
 *
 * Outputs:      None
 *
 * Returns:      MIXER_MODE_ET - The audio mixer mode supported by the session
 *
 * Description:  Retrieves the audio mixer mode supported by the session.
 *
 * Usage Notes:
 *
 */
inline MIXER_MODE_ET CRTCPSession::GetMixerMode(void)
{

//  return the mixer attribute for this session
    return(m_etMixerMode);
}

/**
 *
 * Method Name:  GetSessionID()
 *
 *
 * Inputs:       None
 *
 * Outputs:      None
 *
 * Returns:      unsigned long - The ID for this Sesson
 *
 * Description:  Retrieves the Session ID associated with a session.
 *
 * Usage Notes:
 *
 */
inline unsigned long CRTCPSession::GetSessionID(void)
{

    return(m_ulSessionID);

}

#endif
