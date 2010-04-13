//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////



   // Includes
#include "rtcp/RTCManager.h"
#include "os/OsSysLog.h"


#ifdef INCLUDE_RTCP /* [ */

   // Static Variable Initialization
CRTCManager *CRTCManager::m_spoRTCManager = NULL;
#if RTCP_DEBUG /* [ */
bool        bPingtelDebug = FALSE;
#endif /* INCLUDE_RTCP ] */


#if RTCP_DEBUG /* [ */
    // Global Function used to turn RTCP debugging ON or OFF during runtime
int SetRTCPDebug(int iFlag)
{
    if(iFlag)
        bPingtelDebug = TRUE;
    else
        bPingtelDebug = FALSE;

    return(bPingtelDebug);
}
#endif /* INCLUDE_RTCP ] */

/**
 *
 * Method Name:  GetRTCPControl()
 *
 * Inputs:    None
 *
 * Outputs:   None
 *
 * Returns:   IRTCPControl *piRTCPControl  - Pointer to IRTCPControl Interface
 *
 * Description:  A static member function used ot obtain an RTCPControl
 *               interface.
 *
 * Usage Notes:  This method shall cause the RTCManager Singleton object to
 *               be created if it has not already been instantiated.
 *
 */
IRTCPControl *CRTCManager::getRTCPControl(void)
{

    // If the RTCManager does not yet exist or hasn't been started, then
    // acquire the lock to ensure that only one instance of the task is started
     // sLock.acquire();
    if (m_spoRTCManager == NULL)
    {
        ISDESReport *piSDESReport =
                            (ISDESReport *)CSourceDescription::GetLocalSDES();

        if((m_spoRTCManager = new CRTCManager(piSDESReport)) == NULL)
        {
            osPrintf("**** FAILURE **** CRTCManager::getRTCPControl() -"
                                      " RTCManager Object Creation Failed\n");
            piSDESReport->Release();
            return(NULL);
        }

        // Release Reference to SDES Report
        piSDESReport->Release();
    }

    // Check whether the RTCManager object has been initialized.
    // Initialize it if it has not yet been done.
    if(!m_spoRTCManager->IsInitialized())
    {
       if(!m_spoRTCManager->Initialize())
       {
           osPrintf("**** FAILURE **** CRTCManager::getRTCPControl() -"
                               " Unable to Initialize RTCManager object\n");
           m_spoRTCManager->Release();
           m_spoRTCManager = NULL;
           return(NULL);
       }

       return(m_spoRTCManager);
    }

     // sLock.release();
    // Bump the reference count to this object
    m_spoRTCManager->AddRef();

    return((IRTCPControl *)m_spoRTCManager);

}


/**
 *
 * Method Name: CRTCManager() - Constructor
 *
 *
 * Inputs:   CSourceDescription *poSourceDescription
 *                                        - Local Source Description object
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description:  Performs routine CRTCManager object initialization include
 *               assignment of event notification information.
 *
 * Usage Notes:  It is assumed the the pointer obtained during CRTCManager
 *               object construction shall perform apriori event registration
 *               on behalf of Call Control at system Startup.  Call Control
 *               could also opt to register events during runtime.
 *
 */
CRTCManager::CRTCManager(ISDESReport *piSDESReport)
            :m_ulEventInterest(ALL_EVENTS)

{

    // Stores the constructor arguments as attributes of this object
    if(piSDESReport)
    {
        m_piSDESReport = piSDESReport;
        m_piSDESReport->AddRef();
    }

}

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
 *              acquired over the course of runtime.  In particular, the
 *              following shall occur:
 *                ==> The list containing RTCP Sender objects shall be drained
 *                    with the reference to each object released.
 *                ==> The list containing RTCP Receiver objects shall be
 *                    drained with the reference to each object released.
 *
 * Usage Notes:
 *
 *
 */
CRTCManager::~CRTCManager(void)
{

    // Declarations
    IRTCPSession    *piRTCPSession;
    IRTCPNotify     *piRTCPNotify;

    // Perform a shutdown on the Message Processing Thread
    CMsgQueue::Shutdown();

    // Release Source Description object reference
    if(m_piSDESReport)
        m_piSDESReport->Release();

    // Iterate through the EventRegistration List and release all references
    // to objects contained therein
    piRTCPNotify = m_tRegistrationList.RemoveFirstEntry();
    while (piRTCPNotify != NULL)
    {
        // Release Reference
        piRTCPNotify->Release();

        // Get Next Entry
        piRTCPNotify = m_tRegistrationList.RemoveNextEntry();
    }

    // Iterate through the RTCP Session object List and release all references
    // to those objects
    piRTCPSession = m_tSessionList.RemoveFirstEntry();
    while (piRTCPSession != NULL)
    {
        // Terminate All RTCP Connections
        piRTCPSession->TerminateAllConnections();

        piRTCPSession->Release();

        // Get Next Entry
        piRTCPSession = m_tSessionList.RemoveNextEntry();
    }

    // Clear RTC Manager pointer
    m_spoRTCManager = NULL;
}

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
 * Description: Create a processing thread that shall react to and process
 *              events generated by active RTCP Session and Connections.
 *
 * Usage Notes:
 *
 *
 */
bool CRTCManager::Initialize(void)
{

    // Call the Initialization Method of the Message Processing Class
    if(!CMsgQueue::Initialize())
    {
        return(FALSE);
    }

    // Let's create a Local Source Description object and populate it with
    // default site information
    if(!m_piSDESReport)
    {
        m_piSDESReport = (ISDESReport *)CSourceDescription::GetLocalSDES();
        if (!(m_piSDESReport))
            return(FALSE);
    }

    m_bInitialized = TRUE;
    return(TRUE);

}


/**
 *
 * Method Name:  Advise()
 *
 *
 * Inputs:      IRTCPNotify *piRTCPNotify       - RTCP Event Notify Interface
 *
 * Outputs:     None
 *
 * Returns:     None
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
bool CRTCManager::Advise(IRTCPNotify *piRTCPNotify)
{

    // Place the new Event Notification object on
    // the registration collection list
    if(piRTCPNotify)
    {
        piRTCPNotify->AddRef();
        return(m_tRegistrationList.AddEntry(piRTCPNotify));
    }

    return(FALSE);

}

/**
 *
 * Method Name:  Unadvise()
 *
 *
 * Inputs:      IRTCPNotify *piRTCPNotify       - RTCP Event Notify Interface
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
bool CRTCManager::Unadvise(IRTCPNotify *piRTCPNotify)
{

    // Remove the  Event Notification object from
    // the registration collection list
    if(m_tRegistrationList.RemoveEntry(piRTCPNotify) != NULL)
    {
        piRTCPNotify->Release();
    }

    return(TRUE);
}



/**
 *
 * Method Name: CreateRTCPSession
 *
 *
 * Inputs:      unsigned long ulSSRC - SSRC Identifying the local source
 *
 * Outputs:     None
 *
 * Returns:     IRTCPSession * - interface for controlling an RTCP Session
 *
 * Description: The CreateRTCPSession shall manage the instantiation of RTCP
 *              Session objects used to report and track the identity and
 *              performance of an RTP connections active within a call.
 *
 *
 * Usage Notes: An RTCPSession object shall be create per call.
 *
 *
 */
IRTCPSession * CRTCManager::CreateSession(unsigned long ulSSRC)
{

    CRTCPSession *poRTCPSession;

    // Create The RTCP Session object
    poRTCPSession =
                new CRTCPSession(ulSSRC, (IRTCPNotify *)this, m_piSDESReport);
    if (poRTCPSession == NULL)
    {
        osPrintf("**** FAILURE **** CRTCManager::CreateSession() -"
                                   " Unable to create CRTCPSession object\n");
        return(NULL);
    }

    // Initialize RTCP Session object
    else if(!poRTCPSession->Initialize())
    {
        // Release the RTCP Session reference.  This should cause the object
        // to be destroyed
        osPrintf("**** FAILURE **** CRTCManager::CreateSession() -"
                                " Unable to Intialize CRTCPSession object\n");
        ((IRTCPSession *)poRTCPSession)->Release();
        return(NULL);
    }

    // Place the new RTCP Session object on the collection list
    else if(!m_tSessionList.AddEntry(poRTCPSession))
    {
        // Release the RTCP Session reference.  This should cause the object
        // to be destroyed
        osPrintf("**** FAILURE **** CRTCManager::CreateSession() -"
                        " Unable to add CRTCPSession object to Collection\n");
        ((IRTCPSession *)poRTCPSession)->Release();
        return(NULL);
    }

#if RTCP_DEBUG /* [ */
    if(bPingtelDebug)
    {
        osPrintf("*** RTCP SESSION CREATED ****\n");
        osPrintf("\t  SESSION ==> %d\n", poRTCPSession->GetSessionID());
    }
#endif /* INCLUDE_RTCP ] */

    ((IRTCPSession *)poRTCPSession)->AddRef();
    return((IRTCPSession *)poRTCPSession);
}


/**
 *
 * Method Name: TerminateSession
 *
 *
 * Inputs:      IRTCPSession *piSession
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
bool CRTCManager::TerminateSession(IRTCPSession *piSession)
{
    CRTCPSession *poRTCPSession;

    // Remove the RTCP Session object from the collection list
    poRTCPSession = m_tSessionList.RemoveEntry((CRTCPSession *)piSession);
    if (poRTCPSession != NULL)
    {

#if RTCP_DEBUG /* [ */
        if(bPingtelDebug)
        {
            osPrintf("*** RTCP SESSION TERMINATED ****\n");
            osPrintf("\t ON SESSION ==> %d\n", poRTCPSession->GetSessionID());
        }
#endif /* INCLUDE_RTCP ] */

        // Terminate All RTCP Connections
        ((IRTCPSession *)poRTCPSession)->TerminateAllConnections();

        // Release reference twice.  Once for its removal from the collection
        // and once on behalf of the client since this method serves to
        // terminate the session and release the client's reference.
        ((IRTCPSession *)poRTCPSession)->Release();
        ((IRTCPSession *)poRTCPSession)->Release();
        return(TRUE);
    }

    return(FALSE);
}

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
 * Description:  This is a pure virtual function provide by CMsgQueue as a
 *               means for dispatching messages removed from the queue for
 *               processing.  This method is overidden by this method to
 *               form a dispatcher for event messages removed from the
 *               message queue.
 *
 * Usage Notes:
 *
 *
 */
bool CRTCManager::ProcessMessage(CMessage *poMessage)
{

    // Determine the type of message taken from the queue
    unsigned long   ulMsgType     = poMessage->GetMsgType();
    IRTCPSession    *piSession    =
                            (IRTCPSession *)poMessage->GetThirdArgument();
    IRTCPConnection *piConnection =
                            (IRTCPConnection *)poMessage->GetSecondArgument();
    IBaseClass      *piBaseClass  =
                            (IBaseClass *)poMessage->GetFirstArgument();

    // Determine whether specific session or connection base processing
    // should occur as the result of this event

    // Must forward SDES Reports if acting as a Session Mixer
    if(ulMsgType == RTCP_NEW_SDES || ulMsgType == RTCP_SDES_UPDATE)
    {
        // Retrieve the Session Interface associated with this event.
        piSession = (ulMsgType == RTCP_NEW_SDES) ? piSession :
                               (IRTCPSession *)poMessage->GetFourthArgument();

        // Retrieve the Connection Interface associated with this event.
        piConnection = (ulMsgType == RTCP_NEW_SDES) ? piConnection :
                             (IRTCPConnection *)poMessage->GetThirdArgument();

        // Check whether the Session is acting as an audio mixer.
        // If so, we are obligated by standard to forward the SDES Report
        // to other parties within the session.
        if(piSession->GetMixerMode() == MIXER_ENABLED)
        {
            // Forward the SDES report under the assistance of the
            // Session object
            if (piSession->CheckConnection(piConnection))
                piSession->ForwardSDESReport(
                         (IGetSrcDescription *)poMessage->GetFirstArgument(),
                         piConnection);
        }
    }

    // Must forward BYE Reports if acting as a Session Mixer
    else if(ulMsgType == RTCP_BYE_RCVD)
    {
        // Check whether the Session is acting as an audio mixer.
        // If so, we are obligated by standard to forward the Bye Report
        // received to other parties within the session.
        if(piSession->GetMixerMode() == MIXER_ENABLED)
        {
          // Forward the Bye report under the assistance of the Session object
            if (piSession->CheckConnection(piConnection))
                piSession->ForwardByeReport(
                                (IGetByeInfo *)poMessage->GetFirstArgument(),
                                piConnection);
        }
    }

    // A Report Alarm has gone off on a particular connection.
    // Let's generate the requisite RTCP Reports
    else if(ulMsgType == REPORTING_ALARM)
    {
        // Retrieve the Session Interface associated with this event.
        piSession = (IRTCPSession *)poMessage->GetSecondArgument();

        // Retrieve the Connection Interface associated with this event.
        piConnection = (IRTCPConnection *)poMessage->GetFirstArgument();

        // Set the Base Class pointer to NULL
        piBaseClass = NULL;

        // Call the method that generates the RTCP Reports if the
        // connection is still active
        if(piSession->CheckConnection(piConnection))
            piConnection->GenerateRTCPReports();
    }

    // An RTCP Receiver Report was received.  Let's use this event as a
    // trigger for checking remote SSRC Collisions
    else if(ulMsgType == RTCP_RR_RCVD)
    {
        // Call the Session method to check for Remote Collisions
        if(piSession->CheckConnection(piConnection))
            piSession->CheckRemoteSSRCCollisions(piConnection);
    }

    // An RTCP Receiver Report was sent.  Let's use this event as a
    // trigger for checking local SSRC Collisions
    else if(ulMsgType == RTCP_RR_SENT)
    {
        // Call the Session method to check for Remote Collisions
        if(piSession->CheckConnection(piConnection))
            piSession->CheckLocalSSRCCollisions();
    }

    // We will do no processing of these event other than to assign the
    // interface pointers to variables used for reference counting
    else if ((ulMsgType == LOCAL_SSRC_COLLISION) ||
             (ulMsgType == REMOTE_SSRC_COLLISION))
    {
        // Retrieve the Session Interface associated with this event.
        piSession = (IRTCPSession *)poMessage->GetSecondArgument();

        // Retrieve the Connection Interface associated with this event.
        piConnection = (IRTCPConnection *)poMessage->GetFirstArgument();

        // Set the Base Class pointer to NULL
        piBaseClass = NULL;

    }

    // Check the event registration list for those subscribers that have a
    // matching interest.  For each matching interest, use the recipients
    // IRTCPNotify interface to deliver the message.
    IRTCPNotify *piRTCPNotify = m_tRegistrationList.GetFirstEntry();

    // Iterate through the list until all subscribers have been checked for
    // an interest match
    while(piRTCPNotify != NULL)
    {
        piRTCPNotify->AddRef();

        // Retrieve the registered interest of this client and see if it
        // specifies this event
        if(piRTCPNotify->GetEventInterest() & ulMsgType)
        {
            // Bump reference counts
            piConnection->AddRef();
            piSession->AddRef();
            if(piBaseClass)
                piBaseClass->AddRef();

            // Interest match.  Send the event with the corresponding info
            // as determined by the message type.
            switch(ulMsgType)
            {

                case RTCP_NEW_SDES:
                    piRTCPNotify->NewSDES(
                        (IGetSrcDescription *)poMessage->GetFirstArgument(),
                         piConnection, piSession);
                    break;

                case RTCP_SDES_UPDATE:
                    // Check whether any information has changed in an
                    // existing SDES Report
                    if((unsigned long)poMessage->GetSecondArgument())
                    {
                        piRTCPNotify->UpdatedSDES(
                           (IGetSrcDescription *)poMessage->GetFirstArgument(),
                           (unsigned long)poMessage->GetSecondArgument(),
                           piConnection, piSession);
                    }
                    break;

                case RTCP_RR_RCVD:
                    piRTCPNotify->ReceiverReportReceived(
                       (IGetReceiverStatistics *)poMessage->GetFirstArgument(),
                       piConnection, piSession);
                    break;

                case RTCP_SR_RCVD:
                    piRTCPNotify->SenderReportReceived(
                        (IGetSenderStatistics *)poMessage->GetFirstArgument(),
                         piConnection, piSession);
                    break;

                case RTCP_BYE_RCVD:
                    piRTCPNotify->ByeReportReceived(
                        (IGetByeInfo *)poMessage->GetFirstArgument(),
                         piConnection, piSession);
                    break;

                case RTCP_RR_SENT:
                    piRTCPNotify->ReceiverReportSent(
                       (IGetReceiverStatistics *)poMessage->GetFirstArgument(),
                        piConnection, piSession);
                    break;

                case RTCP_SR_SENT:
                    piRTCPNotify->SenderReportSent(
                       (IGetSenderStatistics *)poMessage->GetFirstArgument(),
                        piConnection, piSession);
                    break;

                case RTCP_SDES_SENT:
                    piRTCPNotify->SDESReportSent(
                       (IGetSrcDescription *)poMessage->GetFirstArgument(),
                        piConnection, piSession);
                    break;

                case RTCP_BYE_SENT:
                    piRTCPNotify->ByeReportSent(
                       (IGetByeInfo *)poMessage->GetFirstArgument(),
                        piConnection, piSession);
                    break;

                case LOCAL_SSRC_COLLISION:
                    piRTCPNotify->LocalSSRCCollision(piConnection, piSession);
                    break;

                case REMOTE_SSRC_COLLISION:
                    piRTCPNotify->RemoteSSRCCollision(piConnection, piSession);
                    break;

                case REPORTING_ALARM:
                    piRTCPNotify->RTCPReportingAlarm(piConnection, piSession);
                    break;

                default:
                    // This is an Error Condition
                    osPrintf("**** FAILURE **** CRTCManager::ProcessMessage()"
                         " - An invalid message type was encountered - %lu\n",
                         ulMsgType);
                    break;

            }

        }

        // Release RTCP Notify Interface
        piRTCPNotify->Release();

        // Get the next interface from the registration list
        // and perform the same checks
        piRTCPNotify = m_tRegistrationList.GetNextEntry();
    }

    // Decrement reference counts to reflect their removal from
    // the RTCManager's Msg Queue
    piConnection->Release();
    piSession->Release();
    if(piBaseClass)
        piBaseClass->Release();

    return(TRUE);
}


/**
 *
 * Method Name:  NewSDES()
 *
 *
 * Inputs:   IGetSrcDescription *piGetSrcDescription
 *                              - Interface to the new Source Description info
 *           IRTCPConnection    *piRTCPConnection
 *                                 - Interface to associated RTCP Connection
 *           IRTCPSession       *piRTCPSession
 *                                 - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The NewSDES() event method shall inform the RTC Manager of a
 *              new Source Description and shall include the SSRC ID, Call
 *              Handle and the IGetSrcDescription interface for accessing
 *              the contents of this new Source Description.  The RTC Manager
 *              shall play the role of central dispatcher; forwarding the
 *              event to other subscribed and interested parties.
 *
 * Usage Notes:
 *
 */
void CRTCManager::NewSDES(IGetSrcDescription *piGetSrcDescription,
                          IRTCPConnection    *piRTCPConnection,
                          IRTCPSession       *piRTCPSession)
{

    // Create a message structure populated with the contents
    // of this event notification
    CMessage *poMessage = new CMessage(RTCP_NEW_SDES,
                                      (void *)piGetSrcDescription,
                                      (void *)piRTCPConnection,
                                      (void *)piRTCPSession);

    // Post the newly created event message to the RTC Manager's message queue
    // for processing by the Message Queue thread/task.
#ifdef TEST_PRINT
    OsSysLog::add(FAC_MP, PRI_DEBUG, "RTCP - CRTCManager::NewSDES new message 0x%08x \n", (int)poMessage);
#endif
    if(poMessage)
    {
        Post(poMessage);
#if defined(__pingtel_on_posix__)
	delete poMessage;
	poMessage = NULL;
#endif
    }
}

/**
 *
 * Method Name:  UpdatedSDES()
 *
 *
 * Inputs:   IGetSrcDescription *piGetSrcDescription
 *                              - Interface to the new Source Description info
 *           unsigned long       ulChangeMask
 *                              - The SDES fields that were subject to change
 *           IRTCPConnection    *piRTCPConnection
 *                                 - Interface to associated RTCP Connection
 *           IRTCPSession       *piRTCPSession
 *                                 - Interface to associated RTCP Session
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
void CRTCManager::UpdatedSDES(IGetSrcDescription *piGetSrcDescription,
                              unsigned long       ulChangeMask,
                              IRTCPConnection    *piRTCPConnection,
                              IRTCPSession       *piRTCPSession)
{

    // Create a message structure populated with the contents
    // of this event notification
    CMessage *poMessage = new CMessage(RTCP_SDES_UPDATE,
                                      (void *)piGetSrcDescription,
                                      (void *)ulChangeMask,
                                      (void *)piRTCPConnection,
                                      (void *)piRTCPSession);

    // Post the newly created event message to the RTC Manager's message queue
    // for processing by the Message Queue thread/task.
#ifdef TEST_PRINT
    OsSysLog::add(FAC_MP, PRI_DEBUG, "RTCP - CRTCManager::UpdatedSDES new message 0x%08x \n", (int)poMessage);
#endif
    if(poMessage)
    {
        Post(poMessage);
#if defined(__pingtel_on_posix__)
        delete poMessage;
        poMessage = NULL;
#endif
    }
}


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
void CRTCManager::SenderReportReceived(
                        IGetSenderStatistics *piGetSenderStatistics,
                        IRTCPConnection      *piRTCPConnection,
                        IRTCPSession         *piRTCPSession)
{

    // Create a message structure populated with the contents
    // of this event notification
    CMessage *poMessage = new CMessage(RTCP_SR_RCVD,
                                      (void *)piGetSenderStatistics,
                                      (void *)piRTCPConnection,
                                      (void *)piRTCPSession);

    // Post the newly created event message to the RTC Manager's message queue
    // for processing by the Message Queue thread/task.
#ifdef TEST_PRINT
    OsSysLog::add(FAC_MP, PRI_DEBUG, "RTCP - CRTCManager::SenderReportReceived new message 0x%08x \n", (int)poMessage);
#endif
    if(poMessage)
    {
        Post(poMessage);
#if defined(__pingtel_on_posix__)
        delete poMessage;
        poMessage = NULL;
#endif
    }
}


/**
 *
 * Method Name:  ReceiverReportReceived()
 *
 *
 * Inputs:   IGetReceiverStatistics *piGetReceiverStatistics
 *                                      - Interface to the Receiver Statistics
 *           IRTCPConnection        *piRTCPConnection
 *                                   - Interface to associated RTCP Connection
 *           IRTCPSession           *piRTCPSession
 *                                      - Interface to associated RTCP Session
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
void CRTCManager::ReceiverReportReceived(
                              IGetReceiverStatistics *piGetReceiverStatistics,
                              IRTCPConnection        *piRTCPConnection,
                              IRTCPSession           *piRTCPSession)
{

    // Create a message structure populated with the contents
    // of this event notification
    CMessage *poMessage = new CMessage(RTCP_RR_RCVD,
                                      (void *)piGetReceiverStatistics,
                                      (void *)piRTCPConnection,
                                      (void *)piRTCPSession);

    // Post the newly created event message to the RTC Manager's message queue
    // for processing by the Message Queue thread/task.
#ifdef TEST_PRINT
    OsSysLog::add(FAC_MP, PRI_DEBUG, "RTCP - CRTCManager::ReceiverReportReceived new message 0x%08x \n", (int)poMessage);
#endif
    if(poMessage)
    {
        Post(poMessage);
#if defined(__pingtel_on_posix__)
        delete poMessage;
        poMessage = NULL;
#endif
    }
}


/**
 *
 * Method Name:  ByeReportReceived()
 *
 *
 * Inputs:   IGetByeInfo      *piGetByeInfo
 *                         - Interface used to retrieve Bye Report information
 *           IRTCPConnection  *piRTCPConnection
 *                                   - Interface to associated RTCP Connection
 *           IRTCPSession     *piRTCPSession
 *                                      - Interface to associated RTCP Session
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
void CRTCManager::ByeReportReceived(IGetByeInfo        *piGetByeInfo,
                                    IRTCPConnection    *piRTCPConnection,
                                    IRTCPSession       *piRTCPSession)
{

    // Create a message structure populated with the contents
    // of this event notification
    CMessage *poMessage = new CMessage(RTCP_BYE_RCVD,
                                      (void *)piGetByeInfo,
                                      (void *)piRTCPConnection,
                                      (void *)piRTCPSession);

    // Post the newly created event message to the RTC Manager's message queue
    // for processing by the Message Queue thread/task.
#ifdef TEST_PRINT
    OsSysLog::add(FAC_MP, PRI_DEBUG, "RTCP - CRTCManager::ByeReportReceived new message 0x%08x \n", (int)poMessage);
#endif
    if(poMessage)
    {
        Post(poMessage);
#if defined(__pingtel_on_posix__)
        delete poMessage;
        poMessage = NULL;
#endif
    }
}

/**
 *
 * Method Name:  SDESReportSent()
 *
 *
 * Inputs:   IGetSrcDescription *piGetSrcDescription
 *                                 - Interface to the local Source Description
 *           IRTCPConnection    *piRTCPConnection
 *                                   - Interface to associated RTCP Connection
 *           IRTCPSession       *piRTCPSession
 *                                      - Interface to associated RTCP Session
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
void CRTCManager::SDESReportSent(IGetSrcDescription *piGetSrcDescription,
                                 IRTCPConnection    *piRTCPConnection,
                                 IRTCPSession       *piRTCPSession)
{

    // Create a message structure populated with the contents
    // of this event notification
    CMessage *poMessage = new CMessage(RTCP_SDES_SENT,
                                      (void *)piGetSrcDescription,
                                      (void *)piRTCPConnection,
                                      (void *)piRTCPSession);

    // Post the newly created event message to the RTC Manager's message queue
    // for processing by the Message Queue thread/task.
#ifdef TEST_PRINT
    OsSysLog::add(FAC_MP, PRI_DEBUG, "RTCP - CRTCManager::SDESReportSent new message 0x%08x \n", (int)poMessage);
#endif
    if(poMessage)
    {
        Post(poMessage);
#if defined(__pingtel_on_posix__)
        delete poMessage;
        poMessage = NULL;
#endif
    }
}

/**
 *
 * Method Name:  SenderReportSent()
 *
 *
 * Inputs:   IGetSenderStatistics *piGetSenderStatistics
 *                                        - Interface to the Sender Statistics
 *           IRTCPConnection      *piRTCPConnection
 *                                   - Interface to associated RTCP Connection
 *           IRTCPSession         *piRTCPSession
 *                                      - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The SenderReportSent() event method shall inform the recipient
 *              of a newly transmitted Sender Report and shall include the
 *              IGetSenderStatistics interface for accessing the contents of
 *              this transmitted Sender Report. The RTC Manager shall  play
 *              the role of central dispatcher; forwarding the event to other
 *              subscribed and interested parties.
 *
 * Usage Notes:
 *
 */
void CRTCManager::SenderReportSent(IGetSenderStatistics *piGetSenderStatistics,
                                   IRTCPConnection      *piRTCPConnection,
                                   IRTCPSession         *piRTCPSession)
{

    // Create a message structure populated with the contents
    // of this event notification
    CMessage *poMessage = new CMessage(RTCP_SR_SENT,
                                      (void *)piGetSenderStatistics,
                                      (void *)piRTCPConnection,
                                      (void *)piRTCPSession);

    // Post the newly created event message to the RTC Manager's message queue
    // for processing by the Message Queue thread/task.
#ifdef TEST_PRINT
    OsSysLog::add(FAC_MP, PRI_DEBUG, "RTCP - CRTCManager::SenderReportSent new message 0x%08x \n", (int)poMessage);
#endif
    if(poMessage)
    {
        Post(poMessage);
#if defined(__pingtel_on_posix__)
        delete poMessage;
        poMessage = NULL;
#endif
    }
}


/**
 *
 * Method Name:  ReceiverReportSent()
 *
 *
 * Inputs:   IGetReceiverStatistics *piGetReceiverStatistics
 *                                      - Interface to the Receiver Statistics
 *           IRTCPConnection        *piRTCPConnection
 *                                   - Interface to associated RTCP Connection
 *           IRTCPSession           *piRTCPSession
 *                                      - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The ReceiverReportSent() event method shall inform the
 *              recipient of a newly transmitted Receiver Report and shall
 *              include the IGetReceiverStatistics interface for accessing
 *              the contents of this transmitted Receiver Report.  The RTC
 *              Manager shall  play the role of central dispatcher; forwarding
 *              the event to other subscribed and interested parties.
 *
 * Usage Notes:
 *
 */
void CRTCManager::ReceiverReportSent(
                              IGetReceiverStatistics *piGetReceiverStatistics,
                              IRTCPConnection        *piRTCPConnection,
                              IRTCPSession           *piRTCPSession)
{

    // Create a message structure populated with the contents
    // of this event notification
    CMessage *poMessage = new CMessage(RTCP_RR_SENT,
                                      (void *)piGetReceiverStatistics,
                                      (void *)piRTCPConnection,
                                      (void *)piRTCPSession);

    // Post the newly created event message to the RTC Manager's message queue
    // for processing by the Message Queue thread/task.
#ifdef TEST_PRINT
    OsSysLog::add(FAC_MP, PRI_DEBUG, "RTCP - CRTCManager::ReceiverReportSent new message 0x%08x \n", (int)poMessage);
#endif
    if(poMessage)
    {
        Post(poMessage);
#if defined(__pingtel_on_posix__)
        delete poMessage;
        poMessage = NULL;
#endif
    }
}


/**
 *
 * Method Name:  ByeReportSent()
 *
 *
 * Inputs:   IGetByeInfo        *piGetByeInfo
 *                         - Interface used to retrieve Bye Report information
 *           IRTCPConnection    *piRTCPConnection
 *                                   - Interface to associated RTCP Connection
 *           IRTCPSession       *piRTCPSession
 *                                      - Interface to associated RTCP Session
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
void CRTCManager::ByeReportSent(IGetByeInfo      *piGetByeInfo,
                                IRTCPConnection  *piRTCPConnection,
                                IRTCPSession     *piRTCPSession)
{

    // Create a message structure populated with the contents
    // of this event notification
    CMessage *poMessage = new CMessage(RTCP_BYE_SENT,
                                      (void *)piGetByeInfo,
                                      (void *)piRTCPConnection,
                                      (void *)piRTCPSession);

    // Post the newly created event message to the RTC Manager's message queue
    // for processing by the Message Queue thread/task.
#ifdef TEST_PRINT
    OsSysLog::add(FAC_MP, PRI_DEBUG, "RTCP - CRTCManager::ByeReportSent new message 0x%08x \n", (int)poMessage);
#endif
    if(poMessage)
    {
        Post(poMessage);
#if defined(__pingtel_on_posix__)
        delete poMessage;
        poMessage = NULL;
#endif
    }
}


/**
 *
 * Method Name:  LocalSSRCCollision()
 *
 *
 * Inputs:   IRTCPConnection    *piRTCPConnection
 *                                   - Interface to associated RTCP Connection
 *           IRTCPSession       *piRTCPSession
 *                                      - Interface to associated RTCP Session
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
void CRTCManager::LocalSSRCCollision(IRTCPConnection    *piRTCPConnection,
                                     IRTCPSession       *piRTCPSession)
{

#if RTCP_DEBUG /* [ */
    if(bPingtelDebug)
    {
         osPrintf("*** LOCAL SSRC COLLISION DETECTED ****\n");
         osPrintf("\t ON SESSION ==> %d\n", piRTCPSession->GetSessionID());
         osPrintf("\t WITH SSRC  ==> %d\n", piRTCPSession->GetSSRC());
    }
#endif /* INCLUDE_RTCP ] */

    // Create a message structure populated with the contents
    // of this event notification
    CMessage *poMessage = new CMessage(LOCAL_SSRC_COLLISION,
                                      (void *)piRTCPConnection,
                                      (void *)piRTCPSession);

    // Post the newly created event message to the RTC Manager's message queue
    // for processing by the Message Queue thread/task.
#ifdef TEST_PRINT
    OsSysLog::add(FAC_MP, PRI_DEBUG, "RTCP - CRTCManager::LocalSSRCCollision new message 0x%08x \n", (int)poMessage);
#endif
    if(poMessage)
    {
        Post(poMessage);
#if defined(__pingtel_on_posix__)
        delete poMessage;
        poMessage = NULL;
#endif
    }
}


/**
 *
 * Method Name:  RemoteSSRCCollision()
 *
 *
 * Inputs:   IRTCPConnection    *piRTCPConnection
 *                                   - Interface to associated RTCP Connection
 *           IRTCPSession       *piRTCPSession
 *                                      - Interface to associated RTCP Session
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
void CRTCManager::RemoteSSRCCollision(IRTCPConnection    *piRTCPConnection,
                                     IRTCPSession        *piRTCPSession)
{
#if RTCP_DEBUG /* [ */
    if(bPingtelDebug)
    {
         osPrintf("*** REMOTE SSRC COLLISION DETECTED ****\n");
         osPrintf("\t ON SESSION ==> %d\n", piRTCPSession->GetSessionID());
         osPrintf("\t WITH SSRC  ==> %d\n", piRTCPConnection->GetRemoteSSRC());
    }
#endif /* INCLUDE_RTCP ] */

    // Create a message structure populated with the contents
    // of this event notification
    CMessage *poMessage = new CMessage(REMOTE_SSRC_COLLISION,
                                      (void *)piRTCPConnection,
                                      (void *)piRTCPSession);

    // Post the newly created event message to the RTC Manager's message queue
    // for processing by the Message Queue thread/task.
#ifdef TEST_PRINT
    OsSysLog::add(FAC_MP, PRI_DEBUG, "RTCP - CRTCManager::RemoteSSRCCollision new message 0x%08x \n", (int)poMessage);
#endif
    if(poMessage)
    {
        Post(poMessage);
#if defined(__pingtel_on_posix__)
        delete poMessage;
        poMessage = NULL;
#endif
    }
}

/**
 *
 * Method Name:  RTCPReportingAlarm()
 *
 *
 * Inputs:   IRTCPConnection    *piRTCPConnection
 *                                   - Interface to associated RTCP Connection
 *           IRTCPSession       *piRTCPSession
 *                                      - Interface to associated RTCP Session
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
void CRTCManager::RTCPReportingAlarm(IRTCPConnection     *piRTCPConnection,
                                     IRTCPSession        *piRTCPSession)
{


    // Create a message structure populated with the contents
    // of this event notification
    CMessage *poMessage = new CMessage(REPORTING_ALARM,
                                      (void *)piRTCPConnection,
                                      (void *)piRTCPSession);

    // Post the newly created event message to the RTC Manager's message queue
    // for processing by the Message Queue thread/task.
#ifdef TEST_PRINT
    OsSysLog::add(FAC_MP, PRI_DEBUG, "RTCP - CRTCManager::RTCPReportingAlarm new message 0x%08x \n", (int)poMessage);
#endif
    if(poMessage)
    {
        Post(poMessage);
#if defined(__pingtel_on_posix__)
        delete poMessage;
        poMessage = NULL;
#endif
    }
}
#endif /* INCLUDE_RTCP ] */
