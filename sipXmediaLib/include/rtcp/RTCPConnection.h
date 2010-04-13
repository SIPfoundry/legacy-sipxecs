//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

//  Border Guard
#ifndef _RTCPConnection_h
#define _RTCPConnection_h

#include "rtcp/RtcpConfig.h"

//  Includes
#include "NetworkChannel.h"
#include "RTCPSource.h"
#include "RTCPRender.h"
#include "RTCPTimer.h"
#include "IRTCPSession.h"


/**
 *
 * Class Name:  CRTCPConnection
 *
 * Inheritance: CBaseClass           - Base Class Implementation
 *
 *
 * Interfaces:  IRTCPConnection      - RTCP Session Control Interface
 *
 * Description: The CRTCPConnection Class shall manage the instantiation of
 *              RTCP objects used to report and track the identity and
 *              performance RTP connections.  At RTP connection creation time,
 *              the RTCPConnection will instantiate a CRTCPRender and
 *              CRTCPSource object and make the appropriate association
 *              between objects to:
 *                ==>  Allow the RTPRender object to dispatch RTP Headers, as
 *                     received, to the RTCPRender
 *                ==>  Allow the RTCPSource object to receive RTCP Reports
 *                     from the Network Source object through the INetDispatch
 *                     interface
 *                ==>  Allow the RTCPRender object to send RTCP Reports
 *                     through the INetSend interface exposed by an associated
 *                     Network Render object.
 *                ==>  Allow the SenderReport object associated with the
 *                     RTCPSource object to update Sender Report statistics
 *                     within ReceiverReport object associated with the
 *                     RTCPSource object.
 *
 *              In addition to RTCP setup, the RTCPConnection shall coordinate
 *              the generation of periodic RTCP Sender, Receiver, and SDES
 *              Reports as well as the aperiodic generation of RTCP BYE
 *              Reports.  The CRTCPConnection shall provide the the thread of
 *              execution for these activities and will use the resources of
 *              the various CRTCPRender objects to actually carry out report
 *              generation and transmission.  Additionally, the CRTCPConnection
 *              object shall be responsible for managing registered interests
 *              in RTCP events both in and outside of a call in addition to be
 *              an intermediate dispatcher of these events.
 *
 *
 * Notes:       CRTCPConnection is derived from CBaseClass which provides
 *              basic Initialization and reference counting support.
 *
 */
class CRTCPConnection : public CBaseClass,
                              // Inherits CBaseClass implementation
                        public CRTCPTimer,
                              // Inherit CTimer Class implementation
                        public IRTCPConnection,
                              // Interface exposed for RTCP Connection Control
                        public IRTCPNotify
                              // Interface exposed for RTCP Notifications
 {

//  Public Methods
public:

/**
 *
 * Method Name:  CRTCPConnection() - Constructor
 *
 *
 * Inputs:   unsigned long ulSSRC       - SSRC ID
 *           IRTCPNotify *piRTCPNotify  - RTCP Event Notification Interface
 *           ISDESReport *piSDESReport  - Local Source Description Interface
 *
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description:  Allocates the RTCP Source and Render components of an RTCP
 *               Connection.
 *
 * Usage Notes:  Allocation is not normally performed at construction time but
 *               a flag shall be set that will be used during the
 *               initialization phase to determine whether all went well.  This
 *               is being done so construction arguments do not have to be
 *               stored as attributes of the RTCP Session object.
 *
 */
    CRTCPConnection(unsigned long   ulSSRC, IRTCPNotify *piRTCPNotify,
                    ISDESReport *piSDESReport);

/**
 *
 * Method Name: ~CRTCPConnection() - Destructor
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: Shall deallocated and/or release all resources which was
 *               acquired over the course of runtime.  In particular, the
 *               following shall occur:
 *                ==> Shall release the reference to the RTCP Render object
 *                ==> Shall release the reference to the RTCP Source object
 *                ==> Shall release the reference to the RTCP Timer object
 *
 * Usage Notes:
 *
 *
 */
    ~CRTCPConnection(void);


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
 *               This includes the verification of construction time
 *               initialization as well as the instantiation of the RTCP Timer
 *               object.
 *
 * Usage Notes:  This is an overload of the CBaseClass Initialize() method.
 *
 */
    bool Initialize(void);


/**
 *
 * Method Name:  GetDispatchInterfaces()
 *
 *
 * Inputs:   None
 *
 * Outputs:  INetDispatch **ppiNetDispatch
 *                          -  Interface used to route Network packets
 *                             to the RTCP Source object
 *           IRTPDispatch **ppRTPDispatch
 *                          -  Interface used to route RTP Header packets to
 *                             the RTPRender object's instance of an RTCP
 *                             Receiver Report
 *           ISetSenderStatistics **piSenderStats
 8                          -  Statistical Dispatch Interface
 *
 * Returns:     void
 *
 * Description: This method allow a consumer to obtain the dispatch
 *              interfaces that are crucial to routing inbound RTCP and
 *              RTP network packets.
 *
 *
 * Usage Notes:
 *
 */
    void GetDispatchInterfaces(INetDispatch         **ppiNetDispatch,
                               IRTPDispatch         **ppiRTPDispatch,
                               ISetSenderStatistics **piSenderStats);

/**
 *
 * Method Name:  GetRemoteSSRC()
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long   - SSRC of remote particant
 *
 * Description: This method returns the  remote SSRC ID associated with
 *              an RTCP Connection.
 *
 *
 * Usage Notes:
 *
 */
    unsigned long GetRemoteSSRC(void);


/**
 *
 * Method Name:  GetRenderInterface()
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     IRTCPRender *   - RTCP Render Interface
 *
 * Description: This method allows a consumer to gain access to the RTCP
 *              Render interface associated with a specific RTCP Connection.
 *
 *
 * Usage Notes:
 *
 */
    IRTCPRender * GetRenderInterface(void);

#ifdef PINGTEL_OSSOCKET
/**
 *
 * Method Name:  StartRenderer()
 *
 *
 * Inputs:      OsSocket& rRtcpSocket   - Reference to an RTCP Socket object
 *
 * Outputs:     None
 *
 * Returns:     bool                    - Result of Starting the Renderer
 *
 * Description: This method starts the RTCP Renderer allowing it to generate
 *              RTCP reports on regular intervals.
 *
 *
 * Usage Notes:
 *
 */
    bool StartRenderer(OsSocket& rRtcpSocket);
#else
    bool StartRenderer(INetworkRender *piNetworkRender);
#endif

/**
 *
 * Method Name: GenerateRTCPReports
 *
 *
 * Inputs:   unsigned char *puchByeReason  - Reason for terminating Connection
 *           unsigned long aulCSRC[]       - Array of contributing sources
 *           unsigned long ulCSRCs         - Number of contributing sources
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The GenerateRTCPReports() method is called when the RTCP
 *              Report Timer has expired.  This method will obtain the
 *              interface of the CRTCPRender object and instruct it to generate
 *              reports.  The reports generate shall be returned and shall
 *              result in event delivery to the RTC Manager onto subscribing
 *              clients.
 *
 * Usage Notes:
 *
 *
 */
    void GenerateRTCPReports(unsigned char *puchByeReason = NULL,
                             unsigned long aulCSRC[] = NULL,
                             unsigned long ulCSRCs = 0);


/**
 *
 * Method Name:  StopRenderer
 *
 *
 * Inputs:       bool   bRelease
 *
 * Outputs:      None
 *
 * Returns:      bool
 *
 * Description:  Performs connection based shutdown related to the orderly
 *               deallocation of connection based filters.
 *
 * Usage Notes:
 *
 */
    bool StopRenderer(void);


/**
 *
 * Method Name:  Terminate
 *
 *
 * Inputs:       None
 *
 * Outputs:      None
 *
 * Returns:      bool
 *
 * Description:  Performs connection based shutdown related to the orderly
 *               deallocation of connection based filters.
 *
 * Usage Notes:  This method must be called in order for the Connection object
 *               and its constituents to be gracefully deallocated.
 *
 */
    bool Terminate(void);


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
 * Description: The GetEventInterest() event method shall allow the
 *              dispatcher of notifications to access the event interests
 *              of a subscriber and use these wishes to dispatch RTCP event
 *              notifications
 *
 * Usage Notes:
 *
 */
    unsigned long GetEventInterest(void);

/**
 *
 * Method Name:  NewSDES()
 *
 *
 * Inputs:   IGetSrcDescription *piGetSrcDescription
 *                               - Interface to the new Source Description info
 *           IRTCPConnection    *piRTCPConnection
 *                               - Interface to associated RTCP Connection
 *           IRTCPSession       *piRTCPSession
 *                               - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: Provides the corresponding RTCPConnection interface to the
 *              notification and passes it along to the subscribing Session
 *              Object.
 *
 *
 * Usage Notes:
 *
 */
    void NewSDES(IGetSrcDescription *piGetSrcDescription,
                 IRTCPConnection    *piRTCPConnection=NULL,
                 IRTCPSession       *piRTCPSession=NULL);

/**
 *
 * Method Name:  UpdatedSDES()
 *
 *
 * Inputs:   IGetSrcDescription *piGetSrcDescription
 *                        - Interface to the new Source Description info
 *           unsigned long       ulChangeMask
 *                        - The SDES fields that were subject to change
 *           IRTCPConnection    *piRTCPConnection
 *                        - Interface to associated RTCP Connection
 *           IRTCPSession       *piRTCPSession
 *                        - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: Provides the corresponding RTCPConnection interface to the
 *              notification and passes it along to the subscribing Session
 *              Object.
 *
 * Usage Notes:
 *
 */
    void UpdatedSDES(IGetSrcDescription *piGetSrcDescription,
                     unsigned long       ulChangeMask,
                     IRTCPConnection    *piRTCPConnection=NULL,
                     IRTCPSession       *piRTCPSession=NULL);


/**
 *
 * Method Name:  SenderReportReceived()
 *
 *
 * Inputs:   IGetSenderStatistics *piGetSenderStatistics
 *                                   - Interface to the Sender Statistics
 *           IRTCPConnection      *piRTCPConnection
 *                                   - Interface to associated RTCP Connection
 *           IRTCPSession         *piRTCPSession
 *                                   - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: Provides the corresponding RTCPConnection interface to the
 *              notification and passes it along to the subscribing Session
 *              Object.
 *
 * Usage Notes:
 *
 */
    void SenderReportReceived(IGetSenderStatistics *piGetSenderStatistics,
                              IRTCPConnection      *piRTCPConnection=NULL,
                              IRTCPSession         *piRTCPSession=NULL);


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
 * Description: Provides the corresponding RTCPConnection interface to the
 *              notification and passes it along to the subscribing Session
 *              Object.
 *
 * Usage Notes:
 *
 */
    void ReceiverReportReceived(IGetReceiverStatistics *piGetReceiverStatistics,
                                IRTCPConnection        *piRTCPConnection=NULL,
                                IRTCPSession           *piRTCPSession=NULL);


/**
 *
 * Method Name:  ByeReportReceived()
 *
 *
 * Inputs:   IGetByeInfo      *piGetByeInfo
 *                        - Interface used to retrieve Bye Report information
 *           IRTCPConnection  *piRTCPConnection
 *                        - Interface to associated RTCP Connection
 *           IRTCPSession     *piRTCPSession
 *                        - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: Provides the corresponding RTCPConnection interface to the
 *              notification and passes it along to the subscribing Session
 *              Object.
 *
 * Usage Notes:
 *
 */
    void ByeReportReceived(IGetByeInfo        *piGetByeInfo,
                           IRTCPConnection    *piRTCPConnection=NULL,
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
 * Description: Provides the corresponding RTCPConnection interface to the
 *              notification and passes it along to the subscribing Session
 *              Object.
 *
 * Usage Notes:
 *
 */
    void SDESReportSent(IGetSrcDescription *piGetSrcDescription,
                        IRTCPConnection    *piRTCPConnection=NULL,
                        IRTCPSession       *piRTCPSession=NULL) {};

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
 * Description: Provides the corresponding RTCPConnection interface to the
 *              notification and passes it along to the subscribing Session
 *              Object.
 *
 * Usage Notes:
 *
 */
    void SenderReportSent(IGetSenderStatistics *piGetSenderStatistics,
                          IRTCPConnection    *piRTCPConnection=NULL,
                          IRTCPSession       *piRTCPSession=NULL) {};


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
 * Description: Provides the corresponding RTCPConnection interface to the
 *              notification and passes it along to the subscribing Session
 *              Object.
 *
 * Usage Notes:
 *
 */
    void ReceiverReportSent(IGetReceiverStatistics *piGetReceiverStatistics,
                            IRTCPConnection        *piRTCPConnection=NULL,
                            IRTCPSession           *piRTCPSession=NULL) {};


/**
 *
 * Method Name:  ByeReportSent()
 *
 *
 * Inputs:   IGetByeInfo     *piGetByeInfo
 *                       - Interface used to retrieve Bye Report information
 *           IRTCPConnection *piRTCPConnection
 *                       - Interface to associated RTCP Connection
 *           IRTCPSession    *piRTCPSession
 *                       - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: Provides the corresponding RTCPConnection interface to the
 *              notification and passes it along to the subscribing Session
 *              Object.
 *
 * Usage Notes:
 *
 */
    void ByeReportSent(IGetByeInfo      *piGetByeInfo,
                       IRTCPConnection  *piRTCPConnection=NULL,
                       IRTCPSession     *piRTCPSession=NULL) {};

/**
 *
 * Method Name:  LocalSSRCCollision()
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
 * Description: The LocalSSRCCollision() event method shall inform the
 *              recipient of a collision between the local SSRC and one
 *              used by one of the remote participants.
 *
 * Usage Notes:
 *
 */
    void LocalSSRCCollision(IRTCPConnection    *piRTCPConnection=NULL,
                            IRTCPSession       *piRTCPSession=NULL) {};


/**
 *
 * Method Name:  RemoteSSRCCollision()
 *
 *
 * Inputs:   IRTCPConnection *piRTCPConnection
 *                              - Interface to associated RTCP Connection
 *           IRTCPSession    *piRTCPSession
 *                              - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The RemoteSSRCCollision() event method shall inform the
 *              recipient of a collision between two remote participants.
 *
 * Usage Notes:
 *
 */
    void RemoteSSRCCollision(IRTCPConnection    *piRTCPConnection=NULL,
                             IRTCPSession       *piRTCPSession=NULL) {};

/**
 *
 * Method Name: RTCPReportingAlarm
 *
 *
 * Inputs:   IRTCPConnection *piRTCPConnection
 *                              - Interface to associated RTCP Connection
 *           IRTCPSession    *piRTCPSession
 *                              - Interface to associated RTCP Session
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
    void RTCPReportingAlarm(IRTCPConnection *piRTCPConnection=NULL,
                            IRTCPSession *piRTCPSession=NULL);


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
 * Attribute Name:  m_ulSSRC
 *
 * Type:            unsigned long
 *
 * Description:     This member shall cache the SSRC associated with
 *                  this connection
 *
 */
      unsigned long m_ulSSRC;

/**
 *
 * Attribute Name:  m_piRTCPNetworkRender
 *
 * Type:            INetworkRender *
 *
 * Description:     This member shall store the interface to the
 *                  CNetworkChannel wrapper to the Pingtel OSSocket object.
 *
 */
    INetworkRender *m_piRTCPNetworkRender;

/**
 *
 * Attribute Name:  m_piRTCPNotify
 *
 * Type:            IRTCPNotify *
 *
 * Description:     This member shall contain the interface for delivering
 *                  notification to a registered consumer.
 *
 */
      IRTCPNotify * m_piRTCPNotify;

/**
 *
 * Attribute Name:  m_piSDESReport
 *
 * Type:            ISDESReport *
 *
 * Description:     This member shall cache the control interface for the
 *                  Source Description report that identifies the local site.
 *
 */
      ISDESReport *m_piSDESReport;

/**
 *
 * Attribute Name:  m_ulEventInterest
 *
 * Type:            unsigned long
 *
 * Description:     This member shall store the events of interest for this
 *                  Session object.
 *
 */
      unsigned long m_ulEventInterest;



/**
 *
 * Attribute Name:  m_poRTCPRender
 *
 * Type:            CRTCPRender *
 *
 * Description:     This member shall store an instance to the
 *                  RTCP Render object.
 *
 */
      CRTCPRender *m_poRTCPRender;


/**
 *
 * Attribute Name:  m_poRTCPSource
 *
 * Type:            CRTCPSource *
 *
 * Description: This member shall store an instance to the RTCP Source object.
 *
 */
      CRTCPSource *m_poRTCPSource;


};

/**
 *
 * Method Name:  GetRemoteSSRC()
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     unsigned long   - SSRC of remote particant
 *
 * Description: This method returns the  remote SSRC ID associated
 *              with an RTCP Connection.
 *
 *
 * Usage Notes:
 *
 */

inline unsigned long CRTCPConnection::GetRemoteSSRC(void)
{

    if(m_poRTCPRender)
        return(m_poRTCPRender->GetRemoteSSRC());

    return(0);

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
 * Description: The GetEventInterest() event method shall allow the
 *              dispatcher of notifications to access the event interests of
 *              a subscriber and use these wishes to dispatch RTCP event
 *              notifications
 *
 * Usage Notes:
 *
 */
inline unsigned long CRTCPConnection::GetEventInterest(void)
{

    return(m_ulEventInterest);
}

#endif
