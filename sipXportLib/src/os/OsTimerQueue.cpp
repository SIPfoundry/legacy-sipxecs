/*
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */


#include "os/OsTimerQueue.h"
#include "os/OsLogger.h"

OsTimerQueue::OsTimerQueue(OsMsgQ *pQueue):
        _signalQueue(pQueue)
{
    // In case pQueue is NULL the timer queue object will be unusable.
    // It's better to assert here than continue.
    OS_LOG_AND_ASSERT(
            pQueue ,
            FAC_KERNEL,
            "OsTimerQueue::OsTimerQueue pQueue is NULL");
}

OsTimerQueue::~OsTimerQueue()
{
    // stop and remove all timers
    stop();
}

OsStatus OsTimerQueue::scheduleOneshotAfter(OsMsg* pMsg, const OsTime& offset)
{
    OsStatus ret = OS_SUCCESS;
    OsTime now;

    // pMsg is not checked as it's not this object responsibility

    // check to see if signal queue is valid
    assert(_signalQueue);

    // first remove all timers which expired until now
    OsDateTime::getCurTime(now);
    clearUntil(now);

    //create and start the timer
    OsTimerPtr timer = OsTimerPtr(new OsTimer(pMsg, _signalQueue));
    ret = timer->oneshotAfter(offset);
    if (OS_SUCCESS == ret)
    {
        OsTime at = now + offset;

        //timer started successfully so add it to the queue
        mutex_lock lock(_queueMutex);
        _timers.push(OsTimerData(at, timer));
    }

    return ret;
}

void OsTimerQueue::clearUntil(const OsTime &at)
{
    mutex_lock lock(_queueMutex);

    // see if all timers have to be cleaned
    UtlBoolean cleanAll = (OsTime::OS_INFINITY == at);

    while (_timers.size() > 0)
    {
        // get the first timer from the queue
        OsTimerQueue::OsTimerData timerData(_timers.top());
        // see if it has to be removed
        if (cleanAll || (timerData._at < at))
        {
            // try to stop it
            timerData._timer->stop(TRUE);
            // remove the timer from the queue
            _timers.pop();
        }
        else
        {
            // reached the specified time so stop removal here
            break;
        }
    }
}

void OsTimerQueue::stop()
{
    clearUntil(OsTime::OS_INFINITY);
}
