//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


    // Includes
#ifdef _WIN32
#include <process.h>
#endif

#include "rtcp/RTCPTimer.h"
#ifdef INCLUDE_RTCP /* [ */

/**
 *
 * Method Name:  CRTCPTimer() - Constructor
 *
 *
 * Inputs:   unsigned long ulTimerPeriod - Amount of time before alarming
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description:  Performs routine CRTCPTimer object initialization.
 *
 * Usage Notes:  The argument passed in the constructor list shall be
 *               stored as an attribute and used establish the alarm period.
 *
 *
 */
CRTCPTimer::CRTCPTimer(unsigned long ulTimerPeriod)
#ifdef WIN32
           : m_hTerminateEvent(NULL), m_hTimerThread(NULL)
#elif defined(_VXWORKS)
           : m_tTimer(NULL)
#elif defined(__pingtel_on_posix__)
           : m_pTimeout(NULL), m_pCallback(NULL), m_pTimer(NULL)
#endif
{

    // Store the arguments passed in the constructor as internal data members
    SetReportTimer(ulTimerPeriod);

}


/**
 *
 * Method Name: ~CRTCPTimer() - Destructor
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: Shall deallocate and/or release all resources which were
 * acquired over the course of runtime.
 *
 * Usage Notes: This shall override the virtual destructor in the base class
 *   so that deallocation specific to the derived class will be done despite
 *   the destruction being performed in the base class as part of the release.
 *
 *
 */
CRTCPTimer::~CRTCPTimer(void)
{

    // Shutdown the processing thread and deallocate all related resources
    Shutdown();

}

/**
 *
 * Method Name: CRTCPTimer::Initialize
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     Boolean True/False
 *
 * Description: Create a timer thread that shall wakeup periodically and
 *             perform some operation and go back to sleep.
 *
 * Usage Notes: This overrides the virtual method in the base class.
 *
 */
bool CRTCPTimer::Initialize(void)
{

#ifdef WIN32 /* [ */
    // Create Timer Thread
    if(!CreateTimerThread())
    {
        osPrintf("**** FAILURE **** CRTCPTimer::Initialize() -"
                                   " Unable to Create Timer Thread\n");
        return(FALSE);
    }

#elif defined(_VXWORKS) /* [ */
    // Create VxWorks Timer
    if(timer_create(CLOCK_REALTIME, NULL, &m_tTimer) == ERROR)
    {
        osPrintf("**** FAILURE **** CRTCPTimer::Initialize() -"
                                 " Unable to Establish VxWorks Timer\n");
        return(FALSE);
    }

    // Associate a callback with an established vxWorks Timer
    if(timer_connect(m_tTimer, (VOIDFUNCPTR) ReportingAlarm, this) == ERROR)
    {
        // Failure.  Let's deallocate all vxWorks timer related resources
        osPrintf("**** FAILURE **** CRTCPTimer::Initialize() -"
                        " Unable to Establish VxWorks Timer Callback\n");
        return(FALSE);
    }

    // Install RTCP Alarm clock to fire after the given timer period
    if (ERROR == timer_settime(m_tTimer, CLOCK_REALTIME, &m_stTimeout, NULL))
    {
        osPrintf("RTCPTimer::Initialize: timer_settime() returned"
            " ERROR, errno = %d = 0x%X\n", errno);
        return(FALSE);
    }
#elif defined(__pingtel_on_posix__)
    /* use OSAL like all well-behaved Pingtel code should */
    if(m_pTimeout != NULL)
        delete m_pTimeout;
    // m_ulTimerPeriod is in milliseconds, whereas the second argument to OsTime()
    // is in microseconds.
    m_pTimeout = new OsTime(m_ulTimerPeriod / 1000, (m_ulTimerPeriod % 1000) * 1000);

    if(m_pCallback != NULL)
        delete m_pCallback;
    m_pCallback = new OsCallback(this, ReportingAlarm);

    if(m_pTimer != NULL)
        delete m_pTimer;
    m_pTimer = new OsTimer(*m_pCallback);

    m_pTimer->periodicEvery(*m_pTimeout, *m_pTimeout);
#endif /* ] */

    return(TRUE);

}


/**
 *
 * Method Name: CRTCPTimer::Shutdown
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     Boolean True/False
 *
 * Description: Perform RTCPTimer shutdown operations in preparation for
 *              terminating the network interface.
 *
 * Usage Notes:
 *
 *
 */
bool CRTCPTimer::Shutdown( void )
{

#ifdef WIN32
    // Send a signal to instruct the Net Thread to Terminate
    if(m_hTerminateEvent && m_hTimerThread)
    {
        SetEvent(m_hTerminateEvent);

        // Wait for thread to terminate
        WaitForSingleObject(m_hTimerThread, INFINITE);
    }

    // Close Events
    if(m_hTerminateEvent)
    {
        CloseHandle(m_hTerminateEvent);
        m_hTerminateEvent = NULL;
    }

    // Close RTP Thread Handle
    if(m_hTimerThread)
    {
        CloseHandle(m_hTimerThread);
        m_hTimerThread = NULL;
    }

#elif defined(_VXWORKS)
    // Delete a previously installed vxWorks timer.  The assumption is
    // that deleting the timer will, be default, cancel any established
    // alarm.
    if(m_tTimer)
    {
        timer_delete(m_tTimer);
        m_tTimer = NULL;
    }
#elif defined(__pingtel_on_posix__)
    if(m_pTimer)
    {
        m_pTimer->stop();
        delete m_pTimer;
        delete m_pCallback;
        delete m_pTimeout;
        m_pTimer = NULL;
        m_pCallback = NULL;
        m_pTimeout = NULL;
    }
#endif

    return (TRUE);
}


#ifdef WIN32
/**
 *
 * Method Name: CRTCPTimer::CreateTimerThread
 *
 *
 * Inputs:      None
 *
 * Outputs:     None
 *
 * Returns:     Boolean True/False
 *
 * Description: Creates the Timer Processing Thread.  This thread shall be
 *              responsible for generating a periodic event to signal the
 *              commencement of a new reporting period.
 *
 * Usage Notes:
 *
 *
 */
bool CRTCPTimer::CreateTimerThread(void)
{

    pthread_t iThreadID;


    // Create the thread terminate Event object.  The primary thread
    // will set it when it wants the worker thread to process an event.
    m_hTerminateEvent = CreateEvent (NULL,  // No Special Security attributes
                                     TRUE,  // Manually Resetable
                                     FALSE, // Not signalled at creation
                                     NULL); // No name

    // Return an appropriate Error to the caller if event creation fails.
    if (m_hTerminateEvent == NULL)
        return (FALSE);

    // We need to create a separate thread for managing the message queue
    m_hTimerThread = (HANDLE)_beginthreadex(
                          NULL,             // No Special Security Attributes
                          0,                // Default Stack Size
                          TimerThreadProc,  // Thread Function
                          this,             // Argument to the thread function
                          0,                // Run immediately
                          &iThreadID);      // Thread identifier returned


    // Return an appropriate Error to the caller if Message
    //  thread creation fails.
    if (m_hTimerThread == NULL)
        return (FALSE);


    return(TRUE);

}


/**
 *
 * Method Name: CRTCPTimer::TimerThreadProc
 *
 *
 * Inputs:      void * lpParameter   - An opaque element
 *
 * Outputs:     None
 *
 * Returns:     unsigned long
 *
 * Description: A static method that shall wake up periodically and perform
 *              some unit of work.
 *
 * Usage Notes:
 *
 *
 */
unsigned int __stdcall CRTCPTimer::TimerThreadProc(void * lpParameter)
{
    // NetThreadProc is a static method requiring the 'this' object to be
    // passed as an argument to gain access to internal data members.
    CRTCPTimer *poRTCPTimer = (CRTCPTimer *)lpParameter;

    // Loop until a terminate signal is received
    while (1)
    {

        // Wait for Terminate Event or timeout
        unsigned long dwRetValue =
                   WaitForSingleObject(poRTCPTimer->m_hTerminateEvent,
                       poRTCPTimer->m_ulTimerPeriod);    // Poll for Terminate

        // An Event was Detected.  It must be time to terminate.
        if (dwRetValue  == WAIT_OBJECT_0)
            break;

        // The timer must have expired.
        // Let's call the RTCPConnection processing routine
        poRTCPTimer->RTCPReportingAlarm();

    }


    // osPrintf(">>>>>  CRTCPTimer::RTPThreadProc() - Exiting <<<<<< \n");
    ExitThread(0);

    return 0;
}

#elif defined(_VXWORKS)
/**
 *
 * Method Name: CRTCPTimer::ReportingAlarm
 *
 *
 * Inputs:      timer_t tTimer     - Timer Handle
 *  int     iArgument  - Argument associated with alarming timer
 *
 * Outputs:     None
 *
 * Returns:     None
 *
 * Description: A static method that be called by a vxWorks timer object when
 *              a previously espablished time has expired and is alarming.
 *
 * Usage Notes:
 *
 *
 */
void  CRTCPTimer::ReportingAlarm(void* tTimer, intptr_t iArgument)
{
    CRTCPTimer   *poRTCPTimer = (CRTCPTimer  *)iArgument;

    // The argument is set to the associated Connection interface pointer.
    // Call the overloaded RTCP Reporting event notification method
    poRTCPTimer->RTCPReportingAlarm();
}
#elif defined(__pingtel_on_posix__)

#ifdef RTCP_LINUX_DEBUG
#include <sys/time.h>
#endif

void CRTCPTimer::ReportingAlarm(void* userData, intptr_t eventData)
{
#ifdef RTCP_LINUX_DEBUG
    struct timeval tv;
    gettimeofday(&tv, NULL);
    osPrintf("DEBUG: RTCP Timer expired! (64-second time = %02d.06%d)\n", tv.tv_sec & 63, tv.tv_usec);
#endif
    /* see comments in VXWORKS version of this call */
    CRTCPTimer * poRTCPTimer = (CRTCPTimer *) userData;
    poRTCPTimer->RTCPReportingAlarm();
}

#endif
#endif /* INCLUDE_RTCP ] */
