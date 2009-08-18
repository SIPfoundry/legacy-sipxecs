//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


//  Border Guard
#ifndef _IRTCPSession_h
#define _IRTCPSession_h

#include "rtcp/RtcpConfig.h"

//  Includes
#include "IBaseClass.h"
#include "INetworkRender.h"
#include "IRTCPConnection.h"

//  Forward Declaration
interface IGetSrcDescription;
interface IGetByeInfo;

//  Enumerated Types
enum MIXER_MODE_ET {
    MIXER_DISABLED = 0,
    MIXER_ENABLED  = 1
};

/**
 *
 * Interface Name:  IRTCPSession
 *
 * Inheritance:     None
 *
 *
 * Description: The IRTCPSession interface shall allow a consumer to create
 *              and terminate RTCP Connections within an RTCP Session.  An
 *              RTCP Connection shall monitor and report on the identity and
 *              performance of an RTP Connection within an RTCP Session.
 *
 * Notes:
 *
 */


interface IRTCPSession : public IBaseClass
 {

//  Public Methods
public:


/**
 *
 * Method Name: CreateRTCPConnection
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  IRTCPConnection *
 *             Returns an interface for use in controlling an RTCP Connection
 *
 * Description: The CreateRTCPConnection shall manage the instantiation of
 *              RTCP Connection object used to report and track the identity
 *              and performance of active RTP connection.
 *
 *
 * Usage Notes: An RTCPConnection object shall be create per RTP Connection.
 *
 *
 */
    virtual IRTCPConnection *CreateRTCPConnection(void)=0;


/**
 *
 * Method Name: TerminateRTCPConnection
 *
 *
 * Inputs:   IRTCPConnection *piRTCPConnection
 *             The Connection Interface pointer returned on creation
 *
 * Outputs:  None
 *
 * Returns:  bool
 *
 * Description: The TerminateRTCPConnection() method shall manage the
 *              termination of and RTCP session.  This shall include the
 *              graceful release of all associated objects as well as the
 *              deallocation of all resources associated with each contained
 *              RTCP connection.
 *
 *
 * Usage Notes:
 *
 *
 */
    virtual bool TerminateRTCPConnection(IRTCPConnection *piRTCPConnection)=0;

/**
 *
 * Method Name: TerminateAllConnections
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  bool

 *
 * Description: The TerminateAllConnections() method shall manage the
 *              termination of all RTCP connections.  This shall include the
 *              transmission of a Bye report over each RTCP connection in
 *              addition to the graceful release of all associated objects and
 *              the deallocation of all resources associated with each
 *              contained RTCP connection.
 *
 *
 * Usage Notes:
 *
 *
 */
    virtual void TerminateAllConnections(void)=0;

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
    virtual unsigned long GetSessionID(void) = 0;

/**
 *
 * Method Name:  ReassignSSRC
 *
 *
 * Inputs:   unsigned long   ulSSRC      - Source ID
 *           unsigned char  *puchReason  - Optional Reason for Reassignment
 *
 * Outputs:  None
 *
 * Returns:  void
 *
 * Description: Reassigns the Source Identifier associated with an RTP session
 *              due to collision detection and resolution.  Calling of this
 *              method shall result in the resetting of the SSRC IDs of
 *              associated Sender, Receiver, and SDES Reports.
 *
 * Usage Notes:
 *
 *
 *
 */
    virtual void ReassignSSRC(unsigned long ulSSRC,
                              unsigned char *puchReason=NULL)=0;

/**
 *
 * Method Name: ForwardSDESReport
 *
 *
 * Inputs:   IGetSrcDescription *piGetSrcDescription
 *             Interface for getting SDES Report Statistics
 *           IRTCPConnection  *piRTCPConnection
 *             Interface to RTCP Connection originating SDES
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
    virtual void ForwardSDESReport(IGetSrcDescription *piGetSrcDescription,
                                   IRTCPConnection  *piRTCPConnection)=0;


/**
 *
 * Method Name: ForwardByeReport
 *
 *
 * Inputs:   IGetByeInfo     *piGetByeInfo
 *             Interface used to retrieve Bye Report information
 *           IRTCPConnection *piRTCPConnection
 *             Interface to RTCP Connection originating Bye
 *
 * Outputs:  None
 *
 * Returns:  None
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
    virtual void ForwardByeReport(IGetByeInfo     *piGetByeInfo,
                                  IRTCPConnection  *piRTCPConnection)=0;

/**
 *
 * Method Name: CheckLocalSSRCCollisions
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  void
 *
 * Description: Check that our local SSRC is not colliding with one fo the
 *              SSRCs of a participating site.
 *
 * Usage Notes:
 *
 *
 *
 */
    virtual void CheckLocalSSRCCollisions(void) = 0;

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
    virtual void
        CheckRemoteSSRCCollisions(IRTCPConnection *piRTCPConnection) = 0;

/**
 *
 * Method Name:  SetMixerMode()
 *
 *
 * Inputs:   MIXER_MODE_ET  etMixerMode
 *             Identifies whether the audio mixer is enabled or disabled
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
    virtual void SetMixerMode(MIXER_MODE_ET etMixerMode)=0;

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
    virtual MIXER_MODE_ET GetMixerMode(void)=0;

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
    virtual unsigned long GetSSRC(void)=0;

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
 *             Returns an interface for use in controlling an RTCP Connection
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
    virtual IRTCPConnection *GetFirstConnection(void)=0;

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
 *             Returns an interface for use in controlling an RTCP Connection
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
    virtual IRTCPConnection *CheckConnection(IRTCPConnection *)=0;


/**
 *
 * Method Name: GetNextConnection
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  IRTCPConnection *
 *             Returns an interface for use in controlling an RTCP Connection
 *
 *
 * Description: The GetNextConnection shall retrieve the next instantiation of
 *              an RTCP Connection interface on the RTCP Session's collection
 *              list.
 *
 *
 * Usage Notes:
 *
 *
 */
    virtual IRTCPConnection *GetNextConnection(void)=0;
};

#endif
