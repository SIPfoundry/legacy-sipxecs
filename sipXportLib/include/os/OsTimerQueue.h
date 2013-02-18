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


#ifndef _OsTimerQueue_h_
#define _OsTimerQueue_h_

// SYSTEM INCLUDES
#include <queue>

// APPLICATION INCLUDES
#include <boost/shared_ptr.hpp>
#include "os/OsMsgQ.h"
#include "os/OsTimer.h"

// DEFINES

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


/**
 * This class implements a queue of one-shot timers OsTimer.
 *
 * It offers a method schedule() to create and start a one-shot timer
 * to signal at a specified time.
 *
 * Also the stop() method will stop all timers from the queue and remove
 * them from the internal container.
 * *
 * @nosubgrouping
 */
class OsTimerQueue: public boost::noncopyable
{
    // The unit test class needs access to private data.
    friend class OsTimerQueueTest;

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    // Needed to wrap a OsTimer because it is noncopyable.
    typedef boost::shared_ptr<OsTimer> OsTimerPtr;

    // These are needed to synchronize access to internal container.
    typedef boost::mutex mutex;
    typedef boost::lock_guard<mutex> mutex_lock;

/* ============================ CREATORS ================================== */

   /** @name Constructors
    *
    * Constructors specify how timers added in the queue will signal the
    * application when fired. The signaling method cannot be changed over the
    * lifetime of the timer queue. Information needed to construct and
    * fire a timer (OsMsg, period information) can be specified by the usage of
    * schedule method.
    *
    * @{
    */

   /** Construct a timer queue for timers that signals by calling
    *  @code
    *  pQueue->send(*pMsg)      // Note that pQueue->send() copies *pMsg.
    *  @endcode
    */
   OsTimerQueue(OsMsgQ* pQueue);       ///< Queue to send message to.
   /// @}

   /// Destructor
   virtual ~OsTimerQueue();
   /* Will stop all the timers expired or not and will remove them from the
    * queue.
    */

/* ============================ MANIPULATORS ============================== */

   /** @name Timer schedule methods
    *
    * These methods allows creation and and starting of timers by providing:
    *   - OsMsg message to be used by the timer when signaling;
    *   - the expiration interval when the timer should fire;
    *
    *  This method has the following responsibilities:
    *  - creates the timer;
    *  - store timer in the queue;
    *  - start the created timer;
    *  - remove from the queue all timers that fired and are expired.
    *
    * They return OS_SUCCESS if timer start operation was successful and OS_FAILED if
    * it failed.
    *
    * @{
    */

   /** Start a timer that will fire once at the (current time + offset).
    *  The timer will signal by calling
    *  @code
    *  pQueue->send(*pMsg)      // Note that pQueue->send() copies *pMsg.
    *  @endcode
    */
   OsStatus scheduleOneshotAfter(OsMsg* pMsg, const OsTime& offset);
   /* The timers are stored in queue ordered by the firing interval.
    *
    * The OsTimer takes ownership of *pMsg.
    * When the OsTimer fires, a copy of *pMsg is queued to *pQueue.
    * (The copy is made using pMsg->createCopy().)
    */

   /// @}

   /// Stop and removes all timers from the queue
   void stop();
   /**<
    * The call to stop is synchronous.
    * For one-shot timers that are expired stop() has no effect.
    */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   /// Data needed to store information about created timers and
   /// keep them ordered in the queue
   struct OsTimerData
   {
        OsTimerData():
         _at(), _timer() {}
        OsTimerData(const OsTime& at,  OsTimerPtr timer):
         _at(at), _timer(timer) {}

        OsTimerData(const OsTimerData &r)
        {
         _at = r._at;
         _timer = r._timer;
        }

        OsTimerData& operator=(const OsTimerData &r)
        {
         _at = r._at;
         _timer = r._timer;

         return *this;
        }

        virtual ~OsTimerData()
        {
        }

        OsTime _at; /// Expiration date in the future
        OsTimerPtr _timer;  /// Actual timer ptr
   };

   /// Method to compare timers to keep them ordered.
   struct OsTimerCompare
   {
      bool operator() (const OsTimerData& a, const OsTimerData& b) const
      {
          // timers are compared by the expiration date.
          return (a._at > b._at);
      }
   };

   // Priority queue to store timers ordered by expiration time.
   typedef std::priority_queue<OsTimerData,std::vector<OsTimerData>, OsTimerCompare> OrderedTimerQueue;

   /// Helper to remove from the queue all timers which expire before the specified time.
   void clearUntil(const OsTime& at);
   /* Timers which expired are removed from the queue.
    * Timer which have not fired yet are stopped and then removed.
    */

    /// The message queue where to send the messages when a timer fires.
    OsMsgQ* _signalQueue;

    /// Ordered queue to store all started timers
    OrderedTimerQueue _timers;

    /// Needed to guard access to _timers
    mutex _queueMutex;
};

#endif  // _OsTimerQueue_h_
