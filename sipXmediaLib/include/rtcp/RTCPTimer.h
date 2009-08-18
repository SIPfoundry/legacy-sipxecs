//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


//  Border Guard
#ifndef _RTCPTimer_h
#define _RTCPTimer_h

#include "rtcp/RtcpConfig.h"

//  Includes
#ifndef WIN32
#include <time.h>
#endif

#include "IRTCPConnection.h"
#include "IRTCPSession.h"

#ifdef __pingtel_on_posix__
#include "os/OsTime.h"
#include "os/OsCallback.h"
#include "os/OsTimer.h"
#endif


//  Defines
#define MILLI2SECS      1000
#define MILLI2NANO      1000000

/**
 *
 * Class Name:  CRTCPTimer
 *
 * Inheritance: CBaseClass       - Generic Base Class
 *
 *
 * Interfaces:  IRTCPTimer       - RTP Header Interface
 *
 * Description: The CRTCPTimer is a class that implements a renewable timer
 *              used to signal the periodic generation of RTCP Reports.
 *
 * Notes:
 *
 */
class CRTCPTimer
{

//  Public Methods
public:

/**
 *
 * Method Name:  CRTCPTimer() - Constructor
 *
 *
 * Inputs:   unsigned long ulTimerPeriod - amount of time in
 *           milliseconds to run before alarming
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

    CRTCPTimer(unsigned long ulTimerPeriod);


/**
 *
 * Method Name: ~CRTCPTimer() - Destructor
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: Shall deallocate and/or release all resources which were
 *              acquired over the course of runtime.
 *
 * Usage Notes: This shall override the virtual destructor in the base class so
 *              that deallocation specific to the derived class will be done
 *              despite the destruction being performed in the base class as
 *              part of the release.
 *
 */
    virtual ~CRTCPTimer(void);

/**
 *
 * Method Name: CRTCPTimer::Initialize
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  Boolean True/False
 *
 * Description: Create a timer thread that shall wakeup periodically and
 *              perform some operation and go back to sleep.
 *
 * Usage Notes: This shall override the virtual destructor in the base class so
 *              that deallocation specific to the derived class will be done
 *              despite the destruction being performed in the base class as
 *              part of the release.
 *
 */
    bool Initialize(void);

/**
 *
 * Method Name: SetReportTimer
 *
 *
 * Inputs:   unsigned long ulTimerPeriod
 *                          - Number of milliseconds to elapse before alarming
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The SetReportTimer method sets a timer to expire at a
 *              specified period.
 *
 *
 * Usage Notes:
 *
 *
 */
    virtual void SetReportTimer(unsigned long ulTimerPeriod);

/**
 *
 * Method Name: GetReportTimer
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  unsigned long - Number of milliseconds to elapse before alarming
 *
 * Description: The GetReportTimer method returns the value of the report timer.
 *
 * Usage Notes:
 *
 */
    virtual unsigned long GetReportTimer(void);

/**
 *
 * Method Name: RTCPReportingAlarm
 *
 *
 * Inputs:   IRTCPConnection *piRTCPConnection
 *                                  - Interface to associated RTCP Connection
 *           IRTCPSession    *piRTCPSession
 *                                  - Interface to associated RTCP Session
 *
 * Outputs:  None
 *
 * Returns:  None
 *
 * Description: The RTCPReportingAlarm() event method shall inform the
 *              recipient of the expiration of the RTCP Reporting Timer.  This
 *              signals the generation of the next round of RTCP Reports.
 *
 * Usage Notes: This method shall be overloaded by the superior class.
 *
 *
 */
    virtual void RTCPReportingAlarm(IRTCPConnection *piRTCPConnection=NULL,
                                    IRTCPSession *piRTCPSession=NULL) {};


protected:  // Protected Methods

/**
 *
 * Method Name: Shutdown
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
    bool Shutdown(void);

private:    // Private Methods

#ifdef WIN32 /* [ */
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
 *      responsible for generating a periodic event to signal the
 *      commencement of a new reporting period.
 *
 * Usage Notes:
 *
 *
 */
    bool CreateTimerThread(void);



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
    static unsigned int __stdcall TimerThreadProc(void * lpParameter);

#elif defined(_VXWORKS) /* ] [ */

/**
 *
 * Method Name: ReportingAlarm
 *
 *
 * Inputs:  timer_t tTimer     - Timer Handle
 *      int     iArgument  - Argument associated with alarming timer
 *
 * Outputs: None
 *
 * Returns: None
 *
 * Description: A static method that be called by a vxWorks timer object when
 *              a previously established time has expired and is alarming.
 *
 * Usage Notes:
 *
 *
 */
    static void ReportingAlarm(void* tTimer, intptr_t iArgument);

#elif defined(__pingtel_on_posix__) /* ] [ */

    static void ReportingAlarm(void* userData, intptr_t eventData);

#endif /* ] */


private:    // Private Data Members

/**
 *
 * Attribute Name:  m_ulTimerPeriod
 *
 * Type:            unsigned long
 *
 * Description:     the RTCP Report Period in milliseconds
 *
 */
    unsigned long    m_ulTimerPeriod;
#ifdef _VXWORKS /* [ */
    struct itimerspec m_stTimeout;
#endif /* ] */


#ifdef WIN32 /* [ */
/**
 *
 * Attribute Name:  m_hTerminateEvent
 *
 * Type:            HANDLE
 *
 * Description:  Event Handle used to signal the termination of the timer
 *               thread created when running in a Windows environment.
 *
 */
    HANDLE           m_hTerminateEvent;

/**
 *
 * Attribute Name:  m_hTimerThread
 *
 * Type:            HANDLE
 *
 * Description:     Thread Handle used to identify the timer thread
 *                  created when running in a Windows environment.
 *
 */
    HANDLE           m_hTimerThread;

#elif defined(_VXWORKS) /* ] [ */

/**
 *
 * Attribute Name:  m_tTimer
 *
 * Type:            timer_t
 *
 * Description:     Timer ID used to identify a timer resource under vxWorks.
 *
 */
    timer_t          m_tTimer;

#elif defined(__pingtel_on_posix__)
    OsTime * m_pTimeout;
    OsCallback * m_pCallback;
    OsTimer * m_pTimer;
#endif /* ] */

};

/**
 *
 * Method Name: SetReportTimer
 *
 *
 * Inputs:  unsigned long ulTimerPeriod  - Number of mSec between alarms
 *
 * Outputs: None
 *
 * Returns: None

 *
 * Description: The SetReportTimer method sets a timer to expire at a
 *              specified period.
 *
 * Usage Notes:
 *
 */
inline void CRTCPTimer::SetReportTimer(unsigned long ulTimerPeriod)
{

    m_ulTimerPeriod = ulTimerPeriod;
#ifdef _VXWORKS /* [ */
    // Set the next fire value
    m_stTimeout.it_value.tv_sec = ulTimerPeriod / MILLI2SECS;
    m_stTimeout.it_value.tv_nsec = (ulTimerPeriod % MILLI2SECS) * MILLI2NANO;

    // Set the next fire interval (period timer) as the value
    m_stTimeout.it_interval.tv_sec = ulTimerPeriod / MILLI2SECS;
    m_stTimeout.it_interval.tv_nsec = (ulTimerPeriod % MILLI2SECS) * MILLI2NANO;
#endif /* ] */

}

/**
 *
 * Method Name: GetReportTimer
 *
 *
 * Inputs:   None
 *
 * Outputs:  None
 *
 * Returns:  unsigned long - Number of milliseconds to elapse before alarming
 *
 * Description: Returns the value of the report timer.
 *
 * Usage Notes:
 *
 */
inline unsigned long CRTCPTimer::GetReportTimer(void)
{

    return(m_ulTimerPeriod);

}

#endif
