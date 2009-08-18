//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


//  Border Guard
#ifndef _RTCManager_h
#define _RTCManager_h

#include "rtcp/RtcpConfig.h"

//  Includes
#include "RTCPSession.h"
#include "ByeReport.h"
#include "SenderReport.h"
#include "ReceiverReport.h"
#include "SourceDescription.h"
#include "TLinkedList.h"
#include "MsgQueue.h"
#include "IRTCPRegister.h"
#include "IRTCPControl.h"
#include "IRTCPNotify.h"


/**
 *
 * Class Name:  CRTCManager
 *
 * Inheritance: CBaseClass           - Base Class Implementation
 *
 *
 * Interfaces:  IRTCPRegister        - Event Registration Interface
 *
 * Description: The CRTCManager Class shall manage the instantiation of RTCP
 *              session objects used to report and track the identity and
 *              performance of RTP connections.  At RTP connection creation
 *              time, the RTCManager will be instructed to create a
 *              corresponding RTCP Session by the Call Control Manager.  The
 *              RTCP Session object shall, in turn, instantiate a CRTCPRender
 *              and CRTCPSource object and make the appropriate association
 *              between objects to:
 *                ==>  Allow the RTPRender object to dispatch RTP Headers, as
 *                     received, to the RTCPRender
 *                ==>  Allow the RTCPSource object to receive RTCP Reports from
 *                     the Network Source object through the INetDispatch
 *                     interface
 *                ==>  Allow the RTCPRender object to send RTCP Reports through
 *                     the INetSend interface exposed by an associated Network
 *                     Render object.
 *                ==>  Allow the SenderReport object associated with the
 *                     RTCPSource object to update Sender Report statistics
 *                     within ReceiverReport object associated with the
 *                     RTCPSource object.
 *
 *              In addition to RTCP setup, the RTCManager shall coordinate the
 *              generation of periodic RTCP Sender, Receiver, and SDES Reports
 *              as well as the aperiodic generation of RTCP BYE Reports.  The
 *              CRTCManager shall provide the the thread of execution for these
 *              activities and will use the resources of the various
 *              CRTCPRender objects to actually carry out report generation and
 *              transmission.  Additionally, the CRTCManager object shall be
 *              responsible for managing registered interests in RTCP events
 *              both in and outside of a call in addition to be an intermediate
 *              dispatcher of these events.
 *
 *              In the future, the CRTCManager's role may be expanded to
 *              pushing statistical data out to the RTCPStatistics and QOS
 *              objects.
 *
 * Notes:       CRTCManager is derived from CBaseClass which provides basic
 *              Initialization and reference counting support.
 *
 */
class CRTCManager : public CBaseClass,
                            // Inherits CBaseClass implementation
                    public CMsgQueue,
                            // Inherit Messaging Services
                    public IRTCPControl,
                            // Interface exposed for controlling RTCP
                    public IRTCPNotify
                            // Notification Interface for receiving RTCP events
 {

//  Public Methods
public:

/**
 *
 * Method Name:  CRTCManager() - Constructor
 *
 *
 * Inputs:       ISDESReporrt *piSDESReport
 *                  - Control interface for the local source description object
 *
 *
 * Outputs:      None
 *
 * Returns:      None
 *
 * Description:  Performs routine CRTCManager object initialization including
 *               the caching of the Source Description Control interface.
 *
 * Usage Notes:
 *
 */
    CRTCManager(ISDESReport *piSDESReport=NULL);


/**
 *
 * Method Name: ~CRTCManager() - Destructor
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
 *                ==> The list of registered notification interfaces shall be
 *                    drained with the reference to each interface released.
 *                ==> The list containing RTCP Session objects shall be drained
 *                    with the reference to each object released.
 *
 * Usage Notes:
 *
 *
 */
    ~CRTCManager(void);

/**
 *
 * Method Name: Initialize
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     Boolean True/False
 *
 * Description: Create a processing thread that shall react to abd process
 *              events generated by active RTCP Session and Connections.
 *
 * Usage Notes:
 *
 *
 */
    bool Initialize(void);



/**
 *
 * Method Name:  getRTCPControl()
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  IRTCPControl *piRTCPControl - Pointer to IRTCPControl Interface
 *
 * Description:  A static member function used ot obtain an RTCPControl
 *               interface.
 *
 * Usage Notes:  This method shall cause the RTCManager Singleton object ot be
 *               created if it has not already been instantiated.
 *
 */
    static IRTCPControl *getRTCPControl(void);

/**
 *
 * Method Name:  Advise()
 *
 *
 * Inputs:      IRTCPNotify *piRTCPNotify       - RTCP Event Notify Interface
 *
 * Outputs:     None
 *
 * Returns:     bool
 *
 * Description: The Advise() method shall be used by a caller to register
 *              interest in receiving events in addition to identifying an
 *              interface on which to receive notifications when these event
 *              criteria are met.
 *
 * Usage Notes: The Advise method can be used to register a new interest or
 *              modify and existing interest.  The interface pointer is used
 *              as the primary key for storing the registered interest.
 *
 */
    bool Advise(IRTCPNotify *piRTCPNotify);

/**
 *
 * Method Name:  Unadvise()
 *
 *
 * Inputs:      iRTCPNotify *piRTCPNotify       - RTCP Event Notify Interface
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: The Unadvise() method allows a caller to remove a previously
 *              registered interest in receiving event.
 *
 * Usage Notes:
 *
 *
 */
    bool Unadvise(IRTCPNotify *piRTCPNotify);


/**
 *
 * Method Name: CreateSession
 *
 *
 * Inputs:      unsigned long ulSSRC - SSRC Identifying the local source
 *
 * Outputs:     None
 *
 * Returns:     IRTCPSession *
 *           - Returns an interface for use in controlling an RTCP Session
 *
 *
 * Description: The CreateSession shall manage the instantiation of RTCP
 *              Session objects used to report and track the identity and
 *              performance of an RTP connections active within a call.
 *
 *
 * Usage Notes: An RTCPSession object shall be create per call.
 *
 *
 */
    IRTCPSession *CreateSession(unsigned long ulSSRC);


/**
 *
 * Method Name: GetFirstSession
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     IRTCPSession *
 *           - Returns an interface for use in controlling an RTCP Session
 *
 *
 * Description: The GetFirstSession shall retrieve the first instantiation of
 *              an RTCP Session interface on the RTC Manager's collection list.
 *
 *
 * Usage Notes:
 *
 *
 */
    IRTCPSession *GetFirstSession(void);

/**
 *
 * Method Name: GetNextSession
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     IRTCPSession *
 *           - Returns an interface for use in controlling an RTCP Session
 *
 *
 * Description: The GetNextSession shall retrieve the next instantiation of an
 *              RTCP Session interface on the RTC Manager's collection list.
 *
 *
 * Usage Notes:
 *
 *
 */
    IRTCPSession *GetNextSession(void);

/**
 *
 * Method Name: TerminateSession
 *
 *
 * Inputs:      IRTCPSession *piRTCPSession
 *                         - Session Interface used as unique handle
 *
 * Outputs:     None
 *
 * Returns:     boolean

 *
 * Description: The TerminateSession() method shall manage the termination of
 *              and RTCP session.  This shall include the graceful release of
 *              all associated objects as well as the deallocation of all
 *              resources associated with each contained RTCP connection.
 *
 *
 * Usage Notes:
 *
 *
 */
    bool TerminateSession(IRTCPSession *piRTCPSession);

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
    unsigned long GetEventInterest(void);


/**
 *
 * Method Name:  NewSDES()
 *
 *
 * Inputs:      IGetSrcDescription *piGetSrcDescription
 *                             - Interface to the new Source Description info
 *              IRTCPConnection    *piRTCPConnection
 *                             - Interface to associated RTCP Connection
 *              IRTCPSession       *piRTCPSession
 *                             - Interface to associated RTCP Session
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: The NewSDES() event method shall inform the RTC Manager of a
 *              new Source Description and shall include the SSRC ID, Call
 *              Handle and the IGetSrcDescription interface for accessing the
 *              contents of this new Source Description.  The RTC Manager shall
 *              play the role of central dispatcher; forwarding the event to
 *              other subscribed and interested parties.
 *
 * Usage Notes:
 *
 */
    void NewSDES(IGetSrcDescription *piGetSrcDescription,
                 IRTCPConnection    *piRTCPConnection,
                 IRTCPSession       *piRTCPSession);

/**
 *
 * Method Name:  UpdatedSDES()
 *
 *
 * Inputs:   IGetSrcDescription *piGetSrcDescription
 *                              - Interface to the new Source Description info
 *           unsigned long      ulChangeMask
 *                              - The SDES fields that were subject to change
 *           IRTCPConnection    *piRTCPConnection
 *                              - Interface to associated RTCP Connection
 *           IRTCPSession       *piRTCPSession
 *                              - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The UpdatedSDES() event method shall inform the RTC Manager of
 *              a change in Source Description and shall include the
 *              IGetSrcDescription interface for accessing the contents of this
 *              updated Source Description.  The RTC Manager shall  play the
 *              role of central dispatcher; forwarding the event to other
 *              subscribed and interested parties.
 *
 * Usage Notes:
 *
 */
    void UpdatedSDES(IGetSrcDescription *piGetSrcDescription,
                     unsigned long       ulChangeMask,
                     IRTCPConnection    *piRTCPConnection,
                     IRTCPSession       *piRTCPSession);


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
 * Description: The SenderReportReceived() event method shall inform the
 *              recipient of a change in Sender Statistics and shall include
 *              the IGetSenderStatistics interface for accessing the contents
 *              of this updated Sender Report. The RTC Manager shall  play the
 *              role of central dispatcher; forwarding the event to other
 *              subscribed and interested parties.
 *
 * Usage Notes:
 *
 */
    void SenderReportReceived(IGetSenderStatistics *piGetSenderStatistics,
                              IRTCPConnection    *piRTCPConnection,
                              IRTCPSession       *piRTCPSession);

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
 *              recipient of a change in Receiver Statistics and shall include
 *              the IGetReceiverStatistics interface for accessing the contents
 *              of this updated Receiver Report. The RTC Manager shall play the
 *              role of central dispatcher; forwarding the event to other
 *              subscribed and interested parties.
 *
 * Usage Notes:
 *
 */
    void ReceiverReportReceived(IGetReceiverStatistics *piGetReceiverStatistics,
                                IRTCPConnection        *piRTCPConnection,
                                IRTCPSession           *piRTCPSession);

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
 * Description: The ByeReportReceived() event method shall inform the recipient
 *              of the discontinuation of an SSRC.  This may result from an
 *              SSRC collision or the termination of an associated RTP
 *              connection.  The RTC Manager shall also play the role of
 *              central dispatcher; forwarding the event to other subscribed
 *              and interested parties.
 *
 * Usage Notes:
 *
 */
    void ByeReportReceived(IGetByeInfo        *piGetByeInfo,
                           IRTCPConnection    *piRTCPConnection,
                           IRTCPSession       *piRTCPSession);

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
                        IRTCPSession       *piRTCPSession);

/**
 *
 * Method Name:  SenderReportSent()
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
 * Description: The SenderReportSent() event method shall inform the recipient
 *              of a newly transmitted Sender Report and shall include the
 *              IGetSenderStatistics interface for accessing the contents of
 *              this transmitted Sender Report. The RTC Manager shall play
 *              the role of central dispatcher; forwarding the event to other
 *              subscribed and interested parties.
 *
 * Usage Notes:
 *
 */
    void SenderReportSent(IGetSenderStatistics *piGetSenderStatistics,
                          IRTCPConnection      *piRTCPConnection,
                          IRTCPSession         *piRTCPSession);


/**
 *
 * Method Name:  ReceiverReportSent()
 *
 *
 * Inputs:   IGetReceiverStatistics *piGetReceiverStatistics
 *                                   - Interface to the Receiver Statistics
 *           IRTCPConnection        *piRTCPConnection
 *                                   - Interface to associated RTCP Connection
 *           IRTCPSession           *piRTCPSession
 *                                   - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The ReceiverReportSent() event method shall inform the
 *              recipient of a newly transmitted Receiver Report and shall
 *              include the IGetReceiverStatistics interface for accessing the
 *              contents of this transmitted Receiver Report.  The RTC Manager
 *              shall  play the role of central dispatcher; forwarding the
 *              event to other subscribed and interested parties.
 *
 * Usage Notes:
 *
 */
    void ReceiverReportSent(IGetReceiverStatistics *piGetReceiverStatistics,
                            IRTCPConnection        *piRTCPConnection,
                            IRTCPSession           *piRTCPSession);


/**
 *
 * Method Name:  ByeReportSent()
 *
 *
 * Inputs:   IGetByeInfo     *piGetByeInfo
 *                  - Interface used to retrieve Bye Report information
 *           IRTCPConnection *piRTCPConnection
 *                  - Interface to associated RTCP Connection
 *           IRTCPSession    *piRTCPSession
 *                  - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The ByeReportSent() event method shall inform the recipient of
 *              a newly transmitted BYE Report and shall include the SSRC ID
 *              and reason to identify the connection and the cause of
 *              termination.  The RTC Manager shall  play the role of central
 *              dispatcher; forwarding the event to other subscribed and
 *              interested parties.
 *
 * Usage Notes:
 *
 */
    void ByeReportSent(IGetByeInfo      *piGetByeInfo,
                       IRTCPConnection  *piRTCPConnection,
                       IRTCPSession     *piRTCPSession);

/**
 *
 * Method Name: RTCPReportingAlarm
 *
 *
 * Inputs:   IRTCPConnection *piRTCPConnection
 *                                - Interface to associated RTCP Connection
 *           IRTCPSession    *piRTCPSession
 *                                - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The RTCPReportingAlarm() event method shall inform the
 *              recipient of the expiration of the RTCP Reporting Timer.  This
 *              signals the generation of the next round of RTCP Reports.
 *
 * Usage Notes:
 *
 *
 */
    void RTCPReportingAlarm(IRTCPConnection *piRTCPConnection,
                            IRTCPSession *piRTCPSession);


/**
 *
 * Method Name:  LocalSSRCCollision()
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
 * Description: The LocalSSRCCollision() event method shall inform the
 *              recipient of a collision between the local SSRC and one used
 *              by one of the remote participants.
 *
 * Usage Notes:
 *
 */
    void LocalSSRCCollision(IRTCPConnection    *piRTCPConnection,
                            IRTCPSession       *piRTCPSession);


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
    void RemoteSSRCCollision(IRTCPConnection    *piRTCPConnection,
                             IRTCPSession       *piRTCPSession);

/**
 *
 * Method Name:  ProcessMessage
 *
 *
 * Inputs:       CMessage *
 *
 * Outputs:      None
 *
 * Returns:      bool
 *
 * Description: This is a pure virtual function provide by CMsgQueue as
 *              a means for dispatching messages removed from the queue
 *              for processing.  This method is overidden by this method to
 *              form a dispatcher for event messages removed from the message
 *              queue.
 *
 * Usage Notes:
 *
 *
 */
    bool ProcessMessage(CMessage *poMessage);





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
 * Attribute Name:  m_spoRTCManager
 *
 * Type:            static CRTCManager *
 *
 * Description:     This static member shall store a pointer to the RTC
 *                  Manager Singleton object.
 *
 */
      static CRTCManager * m_spoRTCManager;


/**
 *
 * Attribute Name:  m_tRegistrationList
 *
 * Type:            CTLinkedList
 *
 * Description:     This member shall maintain a list of elements that
 *                  represent the callback interface and event interest of
 *                  each registered subscriber.
 *
 */
      CTLinkedList<IRTCPNotify *> m_tRegistrationList;

/**
 *
 * Attribute Name:  m_tSessionList
 *
 * Type:            CTLinkedList
 *
 * Description:     This member shall maintain a list of elements that
 *                  represent the active sessions within a system.
 *
 */
      CTLinkedList<CRTCPSession *> m_tSessionList;

/**
 *
 * Attribute Name:  m_piSDESReport
 *
 * Type:            ISDESReport *
 *
 * Description:     This member shall store a reference to the Source
 *                  description object that contains identifying information
 *                  for the local site.
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

};


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
 *                  of notifications to access the event interests of a
 *                  subscriber and use these wishes to dispatch RTCP event
 *                  notifications
 *
 * Usage Notes:
 *
 */
inline unsigned long CRTCManager::GetEventInterest(void)
{

    return(m_ulEventInterest);
}

/**
 *
 * Method Name: GetFirstSession
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     IRTCPSession *
 *                 - the interface for use in controlling an RTCP Session
 *
 *
 * Description: The GetFirstSession shall retrieve the first instantiation of
 *              an RTCP Session interface on the RTC Manager's collection list.
 *
 *
 * Usage Notes:
 *
 *
 */
inline IRTCPSession *CRTCManager::GetFirstSession(void)
{

    return(m_tSessionList.GetFirstEntry());

}

/**
 *
 * Method Name: GetNextSession
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     IRTCPSession *
 *                 - the interface for use in controlling an RTCP Session
 *
 *
 * Description: The GetNextSession shall retrieve the next instantiation of an
 *              RTCP Session interface on the RTC Manager's collection list.
 *
 *
 * Usage Notes:
 *
 *
 */
inline IRTCPSession *CRTCManager::GetNextSession(void)
{

    return(m_tSessionList.GetNextEntry());

}

#endif
