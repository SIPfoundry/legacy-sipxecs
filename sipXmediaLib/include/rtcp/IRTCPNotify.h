//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

//  Border Guard
#ifndef _IRTCPNotify_h
#define _IRTCPNotify_h

#include "rtcp/RtcpConfig.h"

// Include
#include "IBaseClass.h"
#include "IRTCPSession.h"

//  Defines
#define NO_EVENTS               0x0
#define RTCP_NEW_SDES           0x1
#define RTCP_SDES_UPDATE        0x2
#define RTCP_RR_RCVD            0x4
#define RTCP_SR_RCVD            0x8
#define RTCP_BYE_RCVD           0x10
#define RTCP_RR_SENT            0x20
#define RTCP_SR_SENT            0x40
#define RTCP_SDES_SENT          0x80
#define RTCP_BYE_SENT           0x100
#define LOCAL_SSRC_COLLISION    0x200
#define REMOTE_SSRC_COLLISION   0x400
#define REPORTING_ALARM         0x800
#define ALL_EVENTS              0xffff


/**
 *
 * Interface Name:  IRTCPNotify
 *
 * Inheritance:     None
 *
 *
 * Description:     The IRTCPNotify interface allows subscribing clients to
 *                  receive event notifications corresponding to a previously
 *                  registered interest.  The notification will identify the
 *                  type of event and will provide information or resources
 *                  for handling the event.
 *
 * Notes:           It is necessary that the subscribing client implement the
 *                  methods of this notification interface so that the events
 *                  can be processed when received.
 *
 */
interface IRTCPNotify : public IBaseClass
 {

//  Public Methods
public:

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
    virtual unsigned long GetEventInterest(void) { return(NO_EVENTS); };

/**
 *
 * Method Name:  NewSDES()
 *
 *
 * Inputs:   IGetSrcDescription *piGetSrcDescription
 *             Interface to the new Source Description info
 *           IRTCPConnection    *piRTCPConnection
 *             Interface to associated RTCP Connection
 *           IRTCPSession       *piRTCPSession
 *             Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The NewSDES() event method shall inform the recipient of a new
 *              Source Description and shall include the IGetSrcDescription
 *              interface for accessing the contents of this new Source
 *              Description.
 *
 * Usage Notes:
 *
 */
    virtual void NewSDES(IGetSrcDescription *piGetSrcDescription,
                         IRTCPConnection    *piRTCPConnection=NULL,
                         IRTCPSession       *piRTCPSession=NULL) { return; };

/**
 *
 * Method Name:  UpdatedSDES()
 *
 *
 * Inputs:   IGetSrcDescription *piGetSrcDescription
 *             Interface to the new Source Description info
 *           unsigned long       ulChangeMask
 *             Changes that occurred in the SDEs Report
 *           IRTCPConnection    *piRTCPConnection
 *             Interface to associated RTCP Connection
 *           IRTCPSession       *piRTCPSession
 *             Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The UpdatedSDES() event method shall inform the recipient of
 *              a change in Source Description and shall include the
 *              IGetSrcDescription interface for accessing the contents of
 *              this updated Source Description.
 *
 * Usage Notes:
 *
 */
    virtual void UpdatedSDES(IGetSrcDescription *piGetSrcDescription,
                             unsigned long       ulChangeMask,
                             IRTCPConnection    *piRTCPConnection=NULL,
                             IRTCPSession       *piRTCPSession=NULL)
    { return; };


/**
 *
 * Method Name:  SenderReportReceived()
 *
 *
 * Inputs:   IGetSenderStatistics *piGetSenderStatistics
 *             Interface to the Sender Statistics
 *           IRTCPConnection    *piRTCPConnection
 *             Interface to associated RTCP Connection
 *           IRTCPSession       *piRTCPSession
 *             Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The SenderReportReceived() event method shall inform the
 *              recipient of a change in Sender Statistics and shall include
 *              the IGetSenderStatistics interface for accessing the contents
 *              of this updated Sender Report.
 *
 * Usage Notes:
 *
 */
    virtual void
        SenderReportReceived(IGetSenderStatistics *piGetSenderStatistics,
                             IRTCPConnection      *piRTCPConnection=NULL,
                             IRTCPSession         *piRTCPSession=NULL)
        { return; };


/**
 *
 * Method Name:  ReceiverReportReceived()
 *
 *
 * Inputs:   IGetReceiverStatistics *piGetReceiverStatistics
 *             Interface to the Receiver Statistics
 *           IRTCPConnection    *piRTCPConnection
 *             Interface to associated RTCP Connection
 *           IRTCPSession       *piRTCPSession
 *             Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The ReceiverReportReceived() event method shall inform the
 *              recipient of a change in Receiver Statistics and shall include
 *              the IGetReceiverStatistics interface for accessing the
 *              contents of this updated Receiver Report.
 *
 * Usage Notes:
 *
 */
    virtual void
        ReceiverReportReceived(IGetReceiverStatistics *piGetReceiverStatistics,
                               IRTCPConnection        *piRTCPConnection=NULL,
                               IRTCPSession           *piRTCPSession=NULL)
        { return; };


/**
 *
 * Method Name:  ByeReportReceived()
 *
 *
 * Inputs:   IGetByeInfo        *piGetByeInfo
 *             Interface used to retrieve Bye Report information
 *           IRTCPConnection    *piRTCPConnection
 *             Interface to associated RTCP Connection
 *           IRTCPSession       *piRTCPSession
 *             Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The ByeReportReceived() event method shall inform the recipient
 *              of the discontinuation of an SSRC.  This may result from an
 *              SSRC collision or the termination of an associated RTP
 *              connection.
 *
 * Usage Notes:
 *
 */
    virtual void ByeReportReceived(IGetByeInfo      *piGetByeInfo,
                                   IRTCPConnection  *piRTCPConnection=NULL,
                                   IRTCPSession     *piRTCPSession=NULL)
        { return; };


/**
 *
 * Method Name:  SenderReportSent()
 *
 *
 * Inputs:   IGetSenderStatistics *piGetSenderStatistics
 *             Interface to the Sender Statistics
 *           IRTCPConnection      *piRTCPConnection
 *             Interface to associated RTCP Connection
 *           IRTCPSession         *piRTCPSession
 *             Interface to associated RTCP Session
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
    virtual void SenderReportSent(IGetSenderStatistics *piGetSenderStatistics,
                                  IRTCPConnection      *piRTCPConnection=NULL,
                                   IRTCPSession        *piRTCPSession=NULL)
        { return; };


/**
 *
 * Method Name:  ReceiverReportSent()
 *
 *
 * Inputs:  IGetReceiverStatistics *piGetReceiverStatistics
 *            Interface to the Receiver Statistics
 *          IRTCPConnection        *piRTCPConnection
 *            Interface to associated RTCP Connection
 *          IRTCPSession           *piRTCPSession
 *            Interface to associated RTCP Session
 *
 * Outputs: None
 *
 * Returns: None
 *
 * Description: The ReceiverReportSent() event method shall inform the
 *              recipient of a newly transmitted Receiver Report and shall
 *              include the IGetReceiverStatistics interface for accessing
 *              the contents of this transmitted Receiver Report.
 *
 * Usage Notes:
 *
 */
    virtual void
        ReceiverReportSent(IGetReceiverStatistics *piGetReceiverStatistics,
                           IRTCPConnection        *piRTCPConnection=NULL,
                           IRTCPSession           *piRTCPSession=NULL)
        { return; };


/**
 *
 * Method Name:  SDESReportSent()
 *
 *
 * Inputs:   IGetSrcDescription *piGetSrcDescription
 *             Interface to the local Source Description
 *           IRTCPConnection    *piRTCPConnection
 *             Interface to associated RTCP Connection
 *           IRTCPSession       *piRTCPSession
 *             Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The SDESReportSent() event method shall inform the recipient of
 *              a newly transmitted SDES Report and shall include the
 *              IGetSrcDescription interface for accessing the contents of
 *              this transmitted SDES Report.
 *
 * Usage Notes:
 *
 */
    virtual void SDESReportSent(IGetSrcDescription *piGetSrcDescription,
                                IRTCPConnection    *piRTCPConnection=NULL,
                                IRTCPSession       *piRTCPSession=NULL)
        { return; };


/**
 *
 * Method Name:  ByeReportSent()
 *
 *
 * Inputs:   IGetByeInfo     *piGetByeInfo
 *             Interface used to retrieve Bye Report information
 *           IRTCPConnection *piRTCPConnection
 *             Interface to associated RTCP Connection
 *           IRTCPSession    *piRTCPSession
 *             Interface to associated RTCP Session
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
    virtual void ByeReportSent(IGetByeInfo     *piGetByeInfo,
                               IRTCPConnection *piRTCPConnection=NULL,
                               IRTCPSession    *piRTCPSession=NULL)
        { return; };

/**
 *
 * Method Name:  LocalSSRCCollision()
 *
 *
 * Inputs:   IRTCPConnection    *piRTCPConnection
 *             Interface to associated RTCP Connection
 *           IRTCPSession       *piRTCPSession
 *             Interface to associated RTCP Session
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
    virtual void LocalSSRCCollision(IRTCPConnection  *piRTCPConnection=NULL,
                                    IRTCPSession     *piRTCPSession=NULL)
        { return; };


/**
 *
 * Method Name:  RemoteSSRCCollision()
 *
 *
 * Inputs:   IRTCPConnection    *piRTCPConnection
 *             Interface to associated RTCP Connection
 *           IRTCPSession       *piRTCPSession
 *             Interface to associated RTCP Session
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
    virtual void RemoteSSRCCollision(IRTCPConnection  *piRTCPConnection=NULL,
                                     IRTCPSession     *piRTCPSession=NULL)
        { return; };

/**
 *
 * Method Name:  RTCPReportingAlarm()
 *
 *
 * Inputs:   IRTCPConnection    *piRTCPConnection
 *             Interface to associated RTCP Connection
 *           IRTCPSession       *piRTCPSession
 *             Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The ReportAlarm() event method shall inform the recipient of
 *              the expiration of reporting period.  This event usually causes
 *              RTCP Reports to be sent out on the associated session.
 *
 * Usage Notes:
 *
 */
    virtual void RTCPReportingAlarm(IRTCPConnection  *piRTCPConnection=NULL,
                                    IRTCPSession     *piRTCPSession=NULL)
        { return; };

/**
 *
 * Method Name:  RTCPConnectionStopped()
 *
 *
 * Inputs:   IRTCPConnection    *piRTCPConnection
 *             Interface to associated RTCP Connection
 *           IRTCPSession       *piRTCPSession
 *             Interface to associated RTCP Session
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
 * Usage Notes:
 *
 */
    virtual void RTCPConnectionStopped(IRTCPConnection  *piRTCPConnection,
                                       IRTCPSession     *piRTCPSession=NULL)
        { return; };

/**
 *
 * Method Name:  RTCPConnectionStarted()
 *
 *
 * Inputs:   IRTCPConnection    *piRTCPConnection
 *             Interface to associated RTCP Connection
 *           IRTCPSession       *piRTCPSession
 *             Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The RTCPConnectionStarted() event method shall inform the
 *              recipient of the resumption of an RTCP outbound connection.
 *              This will allow whatever connection related operations to be
 *              resumed.
 *
 * Usage Notes:
 *
 */
    virtual void RTCPConnectionStarted(IRTCPConnection  *piRTCPConnection,
                                       IRTCPSession     *piRTCPSession=NULL)
        { return; };
};


#endif
