//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

// Include
#include "rtcp/MsgQueue.h"

#ifdef _VXWORKS /* [ */
#include <taskLib.h>
#elif defined(_WIN32)
#include <process.h>
#elif defined(__pingtel_on_posix__)
#include "os/OsMsgQ.h"
#include <pthread.h> /* OS-SPECIFIC!!! (Well, POSIX anyway) Eventually to be
            fully OSAL-compliant this will need to be an OsTask subclass... */
#endif /* ] */

#define MAX_NOTIFY_MESSAGES_DEFINE 100

#ifdef INCLUDE_RTCP /* [ */


/*|><|************************************************************************
*
* Method Name:  CMsgQueue - Constructor
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      None
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
CMsgQueue::CMsgQueue()
#ifdef WIN32 /* [ */
          : CTLinkedList<CMessage *>(), // Template Contructor Initialization
            m_hMessageEvent(NULL),
            m_hThreadEvent(NULL),
            m_hMessageThread(NULL)
#endif /* ] */
{

}


/*|><|************************************************************************
*
* Method Name:  CMsgQueue - Destructor
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      None
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
CMsgQueue::~CMsgQueue()
{

}


/*|><|************************************************************************
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
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
bool CMsgQueue::Initialize()
{

#ifdef WIN32 /* [ */
    // Create the Event Object which shall be used to inform us of new
    // incoming messages.
    if(!CreateMessageEvent())
        return(FALSE);

    // Create the Message thread.  The Message thread will use the event
    // object to signal us when it has complete the initialization phase.
    if(!CreateMessageThread())
    {
        DestroyMessageEvent();
        return(FALSE);
    }

#elif defined(_VXWORKS) /* ] [ */
    // Create VxWorks Message Processing Thread
    if(!CreateMessageTask())
        return(FALSE);
#elif defined(__pingtel_on_posix__)
    pthread_t thread;

    if(pthread_create(&thread, NULL, InitMessageThread, this))
        return(FALSE);
    pthread_detach(thread);
#endif /* ] */

    return(TRUE);


}


/*|><|************************************************************************
*
* Method Name:  InitMessageThread
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      None
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/

#ifdef WIN32 /* [ */
unsigned int __stdcall CMsgQueue::InitMessageThread(LPVOID argument1)
#elif defined(_VXWORKS) /* ] [ */
int CMsgQueue::InitMessageThread(int argument1, int arg2, int arg3, int arg4,
                           int arg5, int arg6, int arg7, int arg8, int arg9,
                           int arg10)
#elif defined(__pingtel_on_posix__) /* ] [ */
void * CMsgQueue::InitMessageThread(void * argument1)
#endif /* ] */
{

//  MessageLoop is a static method requiring the 'this' object to be
//  passed as an argument to gain access to internal data members.
    CMsgQueue *poMsgQueue = (CMsgQueue *)argument1;

#ifdef WIN32 /* [ */
//  Create the thread terminate Event object.  The primary thread
//  will set it when it wants this thread to cease operation.
    if(!poMsgQueue->CreateThreadEvents())
    {
//      If Event creation fails, this is sufficient grounds to
//      fail initialization.  Things haven't gone as planned.
//      Let's signal an Event and leave.

        SetEvent (poMsgQueue->m_hMessageEvent);
        return(0);
    }


//  Notify the primary thread that we are up
    SetEvent (poMsgQueue->m_hMessageEvent);

#elif defined(_VXWORKS) /* ] [ */

    const int MAX_NOTIFY_MESSAGES = MAX_NOTIFY_MESSAGES_DEFINE;

//  Create a vxWorks Fifo message queue
    poMsgQueue->m_ulMsgQID =
        msgQCreate(MAX_NOTIFY_MESSAGES,    // Max Messages that can be queued
                   sizeof(CMessage *),     // Max bytes in a message
                   MSG_Q_FIFO );           // message queue options

    if(poMsgQueue->m_ulMsgQID == NULL)
        return(0);
#elif defined(__pingtel_on_posix__)

    /* set up the OsMsgQ */
    poMsgQueue->m_pMsgQ = new OsMsgQ("CMsgQueue::InitMessageThread::poMsgQueue->m_pMsgQ",
                                     MAX_NOTIFY_MESSAGES_DEFINE,
                                     OsMsgQ::DEF_MAX_MSG_LEN,
                                     OsMsgQ::Q_PRIORITY);
    if(!poMsgQueue->m_pMsgQ)
        return(0);

#endif /* ] */

//  Enter Message Loop
    poMsgQueue->MessageLoop();

    return(0);
}



/*|><|************************************************************************
*
* Method Name:  Post
*
*
* Inputs:       CMessage *
*
* Outputs:      None
*
* Returns:      bool
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
bool CMsgQueue::Post(CMessage *poMessage)
{

#ifdef WIN32 /* [ */
//  A client has requested us to post this message on the queue.
//  Let's check whether the MessageEvent is still valid. It may become
//  invalid at the initial stages of startup or at termination.
    if(m_hMessageEvent == NULL)
        return(FALSE);

//  Presumably all is well.  Let's add the message to the list
    if(!AddEntry(poMessage))
        return(FALSE);

//  The Entry has been successfully added.  Let's set the message event
//  to actuate its being processed.
    SetEvent(m_hMessageEvent);

#elif defined(_VXWORKS) /* ] [ */
//  Check whether the Message Queue was successfully initialized
    if(m_ulMsgQID == NULL)
        return(FALSE);

//  Use the vxWorks Message Posting routine to place the message on the FIFO
    if(msgQSend(m_ulMsgQID,
               (char *)(&poMessage),
                sizeof(CMessage *),
                NO_WAIT,
                MSG_PRI_NORMAL) == ERROR)
    {
        return(FALSE);
    }
#elif defined(__pingtel_on_posix__) /* ] [ */
    if(m_pMsgQ == NULL)
        return(FALSE);
#ifdef RTCP_LINUX_DEBUG
    osPrintf("DEBUG: RTCP Message queue queueing message! (type %X)\n", poMessage->GetMsgType());
#endif
    if(m_pMsgQ->send(*(OsMsg *)poMessage, OsTime::NO_WAIT))
        return(FALSE);
    poMessage->releaseMsg();
#endif /* ] */

    return(TRUE);

}

/*|><|************************************************************************
*
* Method Name:  Shutdown
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      bool
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
bool CMsgQueue::Shutdown()
{

    // Call the message termination method of the base class.  By default,
    // the Terminate Message does not flush the message queue as part
    // of termination.  It will process each remianing message and then
    // terminate. To flush the message queue, pass 'FLUSH' as an
    // argument to TerminateProcessing().

    TerminateProcessing();

    return(TRUE);

}

/*|><|************************************************************************
*
* Method Name:  TerminateProcessing
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      bool
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
void CMsgQueue::TerminateProcessing(unsigned long dwMode)
{

#ifdef WIN32 /* [ */
// Check Whether Messaging has been terminated.  If the Message event is still
// valid, then it hasn't. In this case, we will terminate messaging ourselves.
    if(m_hMessageEvent)
        TerminateMessaging(dwMode);


//  Now let's wait for the Thread to notify us that its has terminated
    WaitForSingleObject (m_hTerminateEvent, INFINITE);

//  Close the Terminate Event Handle
    CloseHandle(m_hTerminateEvent);

//  Clear Terminate Event Handle
    m_hTerminateEvent = NULL;
#elif defined(_VXWORKS) /* ] [ */

//  Check whether the Message Queue was successfully created.
    if(m_ulMsgQID)
    {
//      Remove all message which may be left on the message list
        FlushMessages();

//      Delete the Message Queue
        msgQDelete(m_ulMsgQID);
        m_ulMsgQID = NULL;
    }
#elif defined(__pingtel_on_posix__) /* ] [ */
    if(m_pMsgQ)
        delete m_pMsgQ;
    m_pMsgQ = NULL;
#endif /* ] */

    return;

}

/*|><|************************************************************************
*
* Method Name:  FlushMessages
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      bool
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
void CMsgQueue::FlushMessages(void)
{
#ifdef WIN32 /* [ */
    CMessage *poMessage;

    // Let's iterate through the list and remove all messages it contains.
    poMessage = RemoveFirstEntry();
    while(poMessage)
    {
        // Delete the memory associated with the message.  This of course
        // assumes that the memory was dynamically allocated.  This might
        // be a dangerous assumption.
        delete poMessage;

        // Remove the next message from the list
        poMessage = RemoveNextEntry();
    }
#elif defined(_VXWORKS) /* ] [ */
    CMessage *poMessage;

    // Block Waiting for a new message to arrive on the FIFO
    while(ERROR != msgQReceive(m_ulMsgQID,
        (char *)&poMessage, sizeof(CMessage *), NO_WAIT))
            delete poMessage;
#elif defined(__pingtel_on_posix__)
    m_pMsgQ->flush();
#endif /* ] */

}


#ifdef WIN32 /* [ */


/*|><|************************************************************************
*
* Method Name:  CreateMessageEvent
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      bool
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
bool CMsgQueue::CreateMessageEvent()
{

    // Create an event object that will be set when
    // there is a control signal state change
    m_hMessageEvent = CreateEvent (NULL,  // No Special Security attributes
                                   TRUE,  // Manually Resetable
                                   FALSE, // Not signalled at creation
                                   NULL); // No name

    // If event creation fails, then we should return with an error
    if (m_hMessageEvent == NULL)
    {
        return (FALSE);
    }

    return(TRUE);

}


/*|><|************************************************************************
*
* Method Name:  CreateThreadEvents
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      bool
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
bool CMsgQueue::CreateThreadEvents()
{

    // Create an event object that will be set when
    // there is a thread state change
    m_hThreadEvent = CreateEvent (NULL,  // No Special Security attributes
                                   FALSE,  // Manually Resetable
                                   FALSE, // Not signalled at creation
                                   NULL); // No name

    // If event creation fails, then we should return with an error
    if (m_hThreadEvent == NULL)
    {
        return (FALSE);
    }

    // Create an event object that will be set when the thread has terminated
    m_hTerminateEvent = CreateEvent (NULL,  // No Special Security attributes
                                   FALSE,    // Manually Resetable
                                   FALSE,   // Not signalled at creation
                                   NULL);   // No name

    // If event creation fails, then we should return with an error
    if (m_hTerminateEvent == NULL)
    {
        CloseHandle(m_hThreadEvent);
        m_hThreadEvent = NULL;
        return (FALSE);
    }

    return(TRUE);

}

/*|><|************************************************************************
*
* Method Name:  CreateMessageThread
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      bool
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
bool CMsgQueue::CreateMessageThread()
{
    pthread_t iMessageThreadID;

//  We need to create a separate thread for managing the message queue
    m_hMessageThread = (HANDLE)_beginthreadex(
                NULL,                // No Special Security Attributes
                0,                   // Default Stack Size
                InitMessageThread,   // Thread Function
                this,                // Argument to the thread function
                0 ,                  // Run immediately
                &iMessageThreadID);  // Thread identifier returned


    // Return an appropriate Error to the
    // caller if Message thread creation fails.
    if (m_hMessageThread == NULL)
    {
        return (FALSE);
    }


    // Now let's wait for the Thread thread to notify
    // us that its initialization is complete.
    WaitForSingleObject (m_hMessageEvent, INFINITE);

    // If the message did initialize successfully, then the thread event
    // shall have a non-NULL value.  This event is created by the thread
    // so that message processing may be terminated gracefully when the
    // time comes.
    if (m_hThreadEvent == NULL)
        return(FALSE);


    // Reset message event for future message delivery signalling
    ResetEvent(m_hMessageEvent);

    return(TRUE);
}


/*|><|************************************************************************
*
* Method Name:  MessageLoop
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      None
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
void CMsgQueue::MessageLoop()
{

    // Establish an array of event on which we are to wait.  Among these
    // events are the message Arrival event and the Thread Termination event.
    HANDLE hEventList[] = {m_hThreadEvent, m_hMessageEvent};
    unsigned long dwEvents = 2;
    BOOL bTerminate = FALSE;

//  Sleep briefly to allow the parent thread to run and pickup the thread
//  initialization signal.
    Sleep(ONE_SECOND);

    while (!bTerminate)
    {

        // Wait for an Event to occur or for the timeout to expire
        unsigned long dwRetValue = WaitForMultipleObjects (
                dwEvents,           // Number of event objects passed in list
                hEventList,         // List of Event objects
                FALSE,              // Do not wait for all to be signalled
                ONE_SECOND);        // Wait timer duration - One Second


        // An Event was Detected.  Let's determine the event type.
        if (dwRetValue  != WAIT_TIMEOUT)
        {
            // Determine which object has signalled us by taking the return
            // value and subtracting the base value of WAIT_OBJECT_0 from it
            unsigned long dwIndex = dwRetValue - WAIT_OBJECT_0;

            // If the object that signalled us is Index 0, it is the terminate
            // thread object.  Let's break out of the while loop and tend to
            // gracefully shutting down the Message processing thread.
            if (dwIndex == 0 || dwIndex > dwEvents)
            {
                // Terminate message processing
                bTerminate = TRUE;
            }
            else
            {
                // Reset Index and Event
                dwIndex = 0;
                ResetEvent(m_hMessageEvent);
            }
        }


        // A message has been received or a timeout has occurred.  We check
        // the queue in the latter case to insure against loosing messages
        // which were pending the completion of a device connection (aka
        // Network connection).  Let's read off all messages which may be
        // on the list and dispatch them for processing.
        CMessage *poMessage = GetFirstEntry();
        while(poMessage)
        {
            // Dispatch the message to the user defined processing method
            if(ProcessMessage(poMessage))
            {
                RemoveEntry(poMessage);
                delete poMessage;
                poMessage = NULL;

            }

            // Remove the next message from the list
            poMessage = GetNextEntry();
        }

    }

    // Terminate the Message Thread
    TerminateMessageThread();

}


/*|><|************************************************************************
*
* Method Name:  TerminateMessageThread
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      None
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
void CMsgQueue::TerminateMessageThread()
{


//  Remove all message which may be left on the message list
    FlushMessages();

//  Destroy the thread event used to signal our untimely demise
    DestroyThreadEvents();

    return;
}

/*|><|************************************************************************
*
* Method Name:  TerminateMessageProcessing
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      bool
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
void CMsgQueue::TerminateMessaging(unsigned long dwMode)
{

//  Flush the message queue if the mode instructs us to do so
    if(dwMode == FLUSH)
        FlushMessages();

//  Processing shall be terminated by setting the thread event.
//  This shall inform the MessageLoop() of our desire to terminate
    SetEvent(m_hThreadEvent);

//  Suspend further message processing by destroying the Message Event
    DestroyMessageEvent();

}

/*|><|************************************************************************
*
* Method Name:  DestroyMessageEvent
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      None
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
void CMsgQueue::DestroyMessageEvent()
{

//  Close the Event Handle
    CloseHandle(m_hMessageEvent);

//  Clear Event Handle
    m_hMessageEvent = NULL;

}



/*|><|************************************************************************
*
* Method Name:  DestroyThreadEvents
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      None
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
void CMsgQueue::DestroyThreadEvents()
{

//  Close the Thread Event Handle
    CloseHandle(m_hThreadEvent);

//  Clear Thread Event Handle
    m_hThreadEvent = NULL;

//  Set the Terminate Event Handle
    SetEvent(m_hTerminateEvent);

}

#elif defined(_VXWORKS) /* ] [ */



/**
 *
 * Method Name:  CreateMessageTask()
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     bool
 *
 * Description:
 *
 * Usage Notes:
 *
 */
bool CMsgQueue::CreateMessageTask()
{

    // Create Message Task for processing incoming messages
    // off the ISR thread of execution
    m_iMsgTaskID = taskSpawn("RTCMessageTask",
                    90,                         // Priority
                    0,                          // Flag Mask
                    1024*10,                    // Stack Size
                    (FUNCPTR)InitMessageThread, // Task Entry Point
                    this,                       // Argument to Entry Point
                    0, 0, 0, 0, 0, 0, 0, 0, 0); // 9 unused arguments

//  Exit with a failure if task creation failed
    if(m_iMsgTaskID == ERROR)
    {
        return(FALSE);
    }

    return(TRUE);
}


/*|><|************************************************************************
*
* Method Name:  MessageLoop
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      None
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
void CMsgQueue::MessageLoop()
{

    // Set the terminate flag to False
    BOOL bTerminate = FALSE;
    int iRetVal;
    CMessage *poMessage;

    // Loop until termination is signalled
    while (!bTerminate)
    {

        // Block Waiting for a new message to arrive on the FIFO
        iRetVal = msgQReceive(m_ulMsgQID, (char *)(&poMessage),
       sizeof(CMessage *), WAIT_FOREVER);

        // Check for Message Queue Deletion.
        // We shall use this return as a signal for use to exit the task.
        if(iRetVal == ERROR)
        {
            if(errno == S_objLib_OBJ_DELETED)
                bTerminate = TRUE;
        }

        // A message has been received. Let's read off all messages which may
        // be on the list and dispatch them for processing.
        else
        {
            if(ProcessMessage(poMessage)){}
            delete poMessage;
        }

    }

    // Exit the vxWorks Task Gracefully
    exit(0);

}

#elif defined(__pingtel_on_posix__) /* ] [ */
    OsMsgQ * m_pMsgQ = NULL;

void CMsgQueue::MessageLoop()
{
    // Set the terminate flag to False
    bool bTerminate = FALSE;
    OsStatus iRetVal;
    OsMsg * pOsMsg;
    CMessage * poMessage;

    // Loop until termination is signalled
    while (!bTerminate)
    {

        // Block Waiting for a new message to arrive on the FIFO
        if(m_pMsgQ)
            iRetVal = m_pMsgQ->receive(pOsMsg, OsTime::OS_INFINITY);
        else
            iRetVal = OS_DELETED;

        if(iRetVal != OS_SUCCESS)
        {
            bTerminate = TRUE;
        }

        // A message has been received. Let's read off all messages which may
        // be on the list and dispatch them for processing.
        else
        {
            poMessage = (CMessage *)pOsMsg;
#ifdef RTCP_LINUX_DEBUG
            osPrintf("DEBUG: RTCP Message queue got message! (type 0x%X)\n", poMessage->GetMsgType());
#endif
            ProcessMessage(poMessage);
            delete poMessage;
            poMessage = NULL;
        }

    }

    return;
}
#endif /* ] */

#endif /* INCLUDE_RTCP ] */
