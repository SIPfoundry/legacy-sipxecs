//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

// SYSTEM INCLUDES
#include <assert.h>
#include <ctype.h>

// APPLICATION INCLUDES
#include "os/OsEventMsg.h"
#include "os/OsNotification.h"
#include "os/OsQueuedEvent.h"
#include "os/OsReadLock.h"
#include "os/OsTimer.h"
#include "os/OsWriteLock.h"
#include "ps/PsButtonInfo.h"
#include "ps/PsButtonTask.h"
#include "ps/PsPhoneTask.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
PsButtonTask* PsButtonTask::spInstance = 0;
OsBSem        PsButtonTask::sLock(OsBSem::Q_PRIORITY, OsBSem::FULL);

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Return a pointer to the Button task, creating it if necessary
PsButtonTask* PsButtonTask::getButtonTask(void)
{
   UtlBoolean isStarted;

   // If the task object already exists, and the corresponding low-level task
   // has been started, then use it
   if (spInstance != NULL && spInstance->isStarted())
      return spInstance;

   // If the task does not yet exist or hasn't been started, then acquire
   // the lock to ensure that only one instance of the task is started
   sLock.acquire();
   if (spInstance == NULL)
       spInstance = new PsButtonTask();

   isStarted = spInstance->isStarted();
   if (!isStarted)
   {
      isStarted = spInstance->start();
      assert(isStarted);
   }
   sLock.release();

   return spInstance;
}

// Destructor
PsButtonTask::~PsButtonTask()
{
   OsWriteLock lock(mMutex);       // acquire a write lock

   doCleanup();
   delete mpKeybdDev;
   spInstance = NULL;
}

/* ============================ MANIPULATORS ============================== */

// Cause the Button task to (re)initialize itself.
// The task will allocate an array [0..maxButtonIndex] of PsButtonInfo
// objects to hold button state.
OsStatus PsButtonTask::init(const int maxButtonIndex)
{
   int         i;
   OsWriteLock lock(mMutex);      // acquire a write lock

   doCleanup();                   // release old dynamic storage (if any)

   mMaxBtnIdx = maxButtonIndex;
   mpButtonInfo = new PsButtonInfo[maxButtonIndex+1];
   mpRepTimers  = new OsTimer*[maxButtonIndex+1];

   for (i=0; i <= maxButtonIndex; i++)
      mpRepTimers[i] = NULL;

   return OS_SUCCESS;
}

// Create a button message and post it to the Button task,
// Return the result of the message send operation.
OsStatus PsButtonTask::postEvent(const int msg, void* source,
                                 const int buttonIndex,
                                 const OsTime& rTimeout)
{
   PsMsg        buttonMsg(msg, source, buttonIndex,
                          mpButtonInfo[buttonIndex].getId());
   OsStatus     res;

   res = postMessage(buttonMsg, rTimeout);
   return res;
}

// Set the button information for the button designated by "index".
OsStatus PsButtonTask::setButtonInfo(const int index, const int buttonId,
                                                                         const char* name,
                                     const int eventMask,
                                     const OsTime& repInterval)
{
   OsWriteLock  lock(mMutex);      // acquire a write lock
   PsButtonInfo buttonInfo(buttonId, name, eventMask, repInterval);

    mpButtonInfo[index] = buttonInfo;

   return OS_SUCCESS;
}

/* ============================ ACCESSORS ================================= */

// Return the button information for the button designated by "index".
const PsButtonInfo& PsButtonTask::getButtonInfo(const int index)
{
   OsReadLock lock(mMutex);       // acquire a read lock

   assert((mpButtonInfo != NULL) && (index <= mMaxBtnIdx));
   return mpButtonInfo[index];
}

// Find the index for the given button ID value
int PsButtonTask::getButtonIndex(int buttonId)
{
   int matchIndex = -1;
   int index;
   OsReadLock lock(mMutex);       // acquire a read lock

   if (mpButtonInfo != NULL)
   {
      for (index=0; index <= mMaxBtnIdx; index++)
      {
         if (mpButtonInfo[index].getId() == buttonId)
         {
            matchIndex = index;
            break;
         }
      }
   }

   return matchIndex;
}

// Find the index for the given button name
int PsButtonTask::getButtonIndex(const char* buttonName)
{
        int matchIndex = -1;

        if (!buttonName)
                return matchIndex;

        int len = strlen(buttonName);
        char* name = new char[len + 1];

        int i = 0;
        while(i < len)
        {
                name[i] = toupper(buttonName[i]);
                i++;
        }
        name[len] = 0;

   OsReadLock lock(mMutex);       // acquire a read lock

   if (mpButtonInfo != NULL)
   {
      for (i=0; i <= mMaxBtnIdx; i++)
      {
         if (!strcmp(mpButtonInfo[i].getName(), name))
         {
            matchIndex = i;
            break;
         }
      }
   }

   delete[] name;

   return matchIndex;
}

int PsButtonTask::getMaxButtonIndex()
{
        return mMaxBtnIdx;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

// Default constructor (called only indirectly via getButtonTask())
PsButtonTask::PsButtonTask()
:  OsServerTask("PsButton"),
   mMaxBtnIdx(-1),                 // not yet initialized
   mMutex(OsRWMutex::Q_PRIORITY),  // create mutex for protecting data
   mpButtonInfo(NULL),             // not yet initialized
   mpKeybdDev(NULL),               // not yet initialized
   mpRepTimers(NULL)               // not yet initialized
{
   mpKeybdDev = PsKeybdDev::getKeybdDev(this);
   mpKeybdDev->enableIntr();
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Handle an incoming message.
// Return TRUE if the message was handled, otherwise FALSE.
UtlBoolean PsButtonTask::handleMessage(OsMsg& rMsg)
{
   UtlBoolean processed;

   switch (rMsg.getMsgType())
   {
   case OsMsg::OS_EVENT:
      processed = handleEventMessage((OsEventMsg&) rMsg);
      break;
   case OsMsg::PS_MSG:
      processed = handlePhoneMessage((PsMsg&) rMsg);
      break;
   default:
                assert(FALSE);
      processed = FALSE;
      break;                            // unexpected message type
   }

   return processed;
}

// Handle an incoming event message (timer expiration).
// Return TRUE if the message was handled, otherwise FALSE.
UtlBoolean PsButtonTask::handleEventMessage(const OsEventMsg& rMsg)
{
   intptr_t       index;
   PsButtonInfo* pButtonInfo;
   PsPhoneTask*  pPhoneTask;
   UtlBoolean     processed;
   OsTimer*      pTimer;
   OsStatus      res;

   OsWriteLock lock(mMutex);        // acquire a write lock

   if (mpButtonInfo == NULL)        // if not yet initialized, just return
      return FALSE;

   processed = TRUE;

   switch(rMsg.getMsgSubType())
   {
   case OsEventMsg::NOTIFY:
      rMsg.getUserData((void*&)index);                  // get button index
      rMsg.getEventData((intptr_t&)pTimer);         // get timer object
      assert(index > 0 && index <= mMaxBtnIdx);  // check for valid index
      pButtonInfo = &mpButtonInfo[index];        // get ptr to button info

      // In order to send the phone set task a button repeat message, three
      // conditions must be satisfied:
      // 1) The button must currently be down
      // 2) The BUTTON_REPEAT event must be enabled for this button
      // 3) The pointers to the OsTimer object in the mpRepTimers array and
      //    the event message must match (otherwise, we may be processing an
      //    event message for an OsTimer object that has already be released.
      if ((pButtonInfo->getState() == PsButtonInfo::DOWN) &&
          (pButtonInfo->getEventMask() & PsButtonInfo::BUTTON_REPEAT) &&
          (mpRepTimers[index] == pTimer))
      {                             // post msg to the phone set
         pPhoneTask = PsPhoneTask::getPhoneTask();
         res = pPhoneTask->postEvent(PsMsg::BUTTON_REPEAT,  // msg type
                                     this,                  // source
                                     index,                 // button index
                                     pButtonInfo->getId()); // button id
         assert(res == OS_SUCCESS);
      }
      break;
   default:
      processed = FALSE;            // unexpected message subtype
      break;
   }

   return processed;
}

// Handle an incoming message from the keyboard controller.
// Return TRUE if the message was handled, otherwise FALSE.
UtlBoolean PsButtonTask::handlePhoneMessage(PsMsg& rMsg)
{
   int           buttonId;
   int           index = 0;
   PsButtonInfo* pButtonInfo = NULL;
   PsPhoneTask*  pPhoneTask;
   UtlBoolean     processed;
   OsStatus      res;

   OsWriteLock lock(mMutex);        // acquire a write lock
   res = OS_SUCCESS;

   if (mpButtonInfo == NULL)        // if not yet initialized, just return
      return res;

   switch (rMsg.getMsg())           // for messages related to button presses
   {                                //  get the corresponding button info
   case PsMsg::BUTTON_UP:
   case PsMsg::BUTTON_DOWN:
   case PsMsg::BUTTON_REPEAT:
      buttonId = rMsg.getParam2();
      pButtonInfo = NULL;

      for (index=0; index <= mMaxBtnIdx; index++)
      {
         if (mpButtonInfo[index].getId() == buttonId)
         {
            rMsg.setStringParam1(mpButtonInfo[index].getName());
            break;
         }
      }
      assert(index <= mMaxBtnIdx);

      pButtonInfo = &mpButtonInfo[index];
      break;
   default:
      break;
   }

   processed = TRUE;

   pPhoneTask = PsPhoneTask::getPhoneTask();
   switch (rMsg.getMsg())
   {
   case PsMsg::BUTTON_UP:
      // printf("BUTTON_UP: buttonId = %d, buttonName = %s\n",
      //        buttonId, mpButtonInfo[index].getName());
      pButtonInfo->setState(PsButtonInfo::UP);   // set the button state
      if (pButtonInfo->getEventMask() & PsButtonInfo::BUTTON_UP)
      {                             // post message to the phone set
         res = pPhoneTask->postMessage(rMsg);
         assert(res == OS_SUCCESS);
      }
      disableTimer(index);          // disable the repeat timer (if any)
      break;
   case PsMsg::BUTTON_DOWN:
   case PsMsg::BUTTON_REPEAT:
      // printf("BUTTON_DOWN/BUTTON_REPEAT: buttonId = %d, buttonName = %s\n",
      //        buttonId, mpButtonInfo[index].getName());
      pButtonInfo->setState(PsButtonInfo::DOWN); // set the button state
      if (pButtonInfo->getEventMask() & PsButtonInfo::BUTTON_DOWN)
      {                             // post message to the phone set
         res = pPhoneTask->postMessage(rMsg);
         assert(res == OS_SUCCESS);
      }

      if (pButtonInfo->getEventMask() & PsButtonInfo::BUTTON_REPEAT)
      {                             // enable and arm the repeat timer
         enableTimer(index);
      }
      break;
   case PsMsg::BUTTON_GET_INFO:
      break;
   case PsMsg::BUTTON_SET_INFO:
      res = pPhoneTask->postMessage(rMsg);
      assert(res == OS_SUCCESS);
      break;
   case PsMsg::KEYPAD_STATE:
      // printf("KEYPAD_STATE: isKeyUp = %d, buttonId = %d\n",
      //        rMsg.getParam1(), rMsg.getParam2());
      break;
   case PsMsg::SCROLL_CHANGE:
      // printf("SCROLL_CHANGE: delta = %d\n", rMsg.getParam1());
      break;
   case PsMsg::SCROLL_STATE:
      // printf("SCROLL_STATE: count = %d\n", rMsg.getParam1());
      break;
   case PsMsg::TOUCHSCREEN_CHANGE:
      // printf("TOUCHSCREEN_CHANGE: x=%d, y=%d\n",
      //        rMsg.getParam1(), rMsg.getParam2());
      break;
   case PsMsg::TOUCHSCREEN_STATE:
      // printf("TOUCHSCREEN_STATE: x=%d, y=%d\n",
      //        rMsg.getParam1(), rMsg.getParam2());
      break;
   default:
                assert(FALSE);
      processed = FALSE;            // unexpected message
      break;
   }

   if (rMsg.getSentFromISR())       // indicate that we are done with the msg
      ((PsMsg&) rMsg).setInUse(FALSE);

   return processed;
}

// Disable the repeat timer for the designated button.
// Do nothing if there is no repeat timer in effect for the button.
// A write lock should be acquired before calling this method.
void PsButtonTask::disableTimer(intptr_t index)
{
   OsTimer*        pTimer;

   pTimer = mpRepTimers[index];

   if (pTimer != NULL)
   {
      delete pTimer;
      mpRepTimers[index] = NULL;
   }
}

// Enable the repeat timer for the designated button.
// A write lock should be acquired before calling this method.
void PsButtonTask::enableTimer(intptr_t index)
{
   OsQueuedEvent* pNotifier;
   OsTime         repInterval;
   OsStatus       res;

   if (mpRepTimers[index] != NULL)  // if there already is a repeat timer,
      disableTimer(index);          //  disable it

   mpButtonInfo[index].getRepInterval(repInterval);
   if (repInterval.isInfinite())    // if the repeat interval is infinite,
      return;                       //  don't bother enabling it

   mpRepTimers[index] = new OsTimer(&mIncomingQ, (void*)index);
   res = mpRepTimers[index]->periodicEvery(repInterval, repInterval);
   assert(res == OS_SUCCESS);
}

// Release dynamically allocated storage.
// A write lock should be acquired before calling this method.
void PsButtonTask::doCleanup(void)
{
   int i;

   // destroy the array of repetition timer objects
   if (mpRepTimers != NULL)
   {
      for (i=0; i <= mMaxBtnIdx; i++)
      {
         disableTimer(i);
      }
      delete[] mpRepTimers;
      mpRepTimers = NULL;
   }

   // destroy the array of button info objects
   if (mpButtonInfo != NULL)
   {
      delete[] mpButtonInfo;
      mpButtonInfo = NULL;
   }
}

/* ============================ FUNCTIONS ================================= */
