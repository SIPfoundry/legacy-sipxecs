//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


//  Border Guard
#ifndef _IRTCPConnection_h
#define _IRTCPConnection_h

#include "rtcp/RtcpConfig.h"

//  Includes
#include "IBaseClass.h"
#include "INetworkRender.h"
#include "IRTPDispatch.h"
#include "INetDispatch.h"
#include "ISetSenderStatistics.h"

// Pingtel extension
#ifdef PINGTEL_OSSOCKET
#include <os/OsSocket.h>
#endif

/**
 *
 * Interface Name:  IRTCPConnection
 *
 * Inheritance:     None
 *
 *
 * Description:     The IRTCPConnection interface shall provide services used
 *                  to access and control the generation and content of RTCP
 *                  reports carrying identification and performance information
 *                  regarding an associated RTP connection.
 *
 * Notes:
 *
 */


interface IRTCPConnection : public IBaseClass
 {

//  Public Methods
public:


/**
 *
 * Method Name:  GetDispatchInterfaces()
 *
 *
 * Inputs:   None
 *
 * Outputs:  INetDispatch         **ppiNetDispatch
 *             Interface used to route Network packets to the RTCP Source object
 *           IRTPDispatch         **ppRTPDispatch
 *             Interface used to route RTP Header packets to the RTPRender
 *             object's instance of an RTCP Receiver Report
 *           ISetSenderStatistics **piSenderStats
 *             Statistical Dispatch Interface
 *
 * Returns:  void
 *
 * Description: This method allow a consumer to obtain the dispatch interfaces
 *              that are crucial to routing inbound RTCP and RTP network
 *              packets.
 *
 *
 * Usage Notes:
 *
 */
    virtual void GetDispatchInterfaces(INetDispatch         **ppiNetDispatch,
                                       IRTPDispatch         **ppiRTPDispatch,
                                       ISetSenderStatistics **piSenderStats)=0;


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
 * Description: This method returns the  remote SSRC ID associated with an
 *              RTCP Connection.
 *
 *
 * Usage Notes:
 *
 */
    virtual unsigned long GetRemoteSSRC(void)=0;


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
    virtual bool StartRenderer(OsSocket& rRtcpSocket) = 0;
#else
    virtual bool StartRenderer(INetworkRender *piNetworkRender) = 0;
#endif

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
    virtual bool StopRenderer(void) = 0;

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
 * Description: The GenerateRTCPReports() method is called when the RTCP Report
 *              Timer has expired.  This method will obtain the interface of
 *              the CRTCPRender object and instruct it to generate reports.
 *              The reports generate shall be returned and shall result in
 *              event delivery to the RTC Manager onto subscribing clients.
 *
 * Usage Notes:
 *
 *
 */
    virtual void GenerateRTCPReports(unsigned char *puchByeReason = NULL,
                                     unsigned long aulCSRC[] = NULL,
                                     unsigned long ulCSRCs = 0)=0;

};

#endif
