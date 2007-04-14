// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsQueuedEvent.h"
#include "os/OsEventMsg.h"
#include "os/OsTimer.h"
#include "WatchDog.h"
#include "os/OsDateTime.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
   const int USER_PROCESS_EVENT = 1; //signifies message is user status check
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
WatchDog::WatchDog(int nWatchInterval,MonitoredProcess **processList,int processCount) :
mpWatchDogTimer(NULL)
{
   // Create the watchdog event/timer
   mpWatchDogEvent = new OsQueuedEvent(*getMessageQueue(), 0) ;
   mpWatchDogTimer = new OsTimer(*mpWatchDogEvent) ;         
 
   // Create the watchdog event/timer
   mpWatchDogUserChangeEvent = new OsQueuedEvent(*getMessageQueue(), 0) ;
   mpWatchDogUserChangeEvent->setUserData(USER_PROCESS_EVENT); //set subtype to signify user action
   mpWatchDogUserChangeTimer = new OsTimer(*mpWatchDogUserChangeEvent) ;         

   // Finally, set the timers
   mpWatchDogTimer->periodicEvery(OsTime(5, 0),OsTime(nWatchInterval, 0));
   mpWatchDogUserChangeTimer->periodicEvery(OsTime(5, 0),OsTime(5, 0));

   //save off some important things
   mpProcessList = processList;
   mnProcessCount = processCount;
}

// Copy constructor
WatchDog::WatchDog(const WatchDog& rWatchDog)
{}

// Destructor
WatchDog::~WatchDog()
{
   delete mpWatchDogEvent;
   delete mpWatchDogTimer;

   delete mpWatchDogUserChangeEvent;
   delete mpWatchDogUserChangeTimer;
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
WatchDog& 
WatchDog::operator=(const WatchDog& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}
   
UtlBoolean WatchDog::handleMessage(OsMsg &rMsg)
{
   int         eventData;
   int         userData;
   int         loop;

   UtlBoolean   returnValue = TRUE;
   OsEventMsg* pEventMsg;

   UtlString dateString;
   OsDateTime dt;
   OsDateTime::getCurTime(dt);
   dt.getHttpTimeString(dateString);  

   if (rMsg.getMsgType() == OsMsg::OS_EVENT)
   {
      //for now I'll leave these here... although im not using them now
       // I may in the near future
      pEventMsg = (OsEventMsg*) &rMsg;
      pEventMsg->getEventData(eventData);
      pEventMsg->getUserData(userData);

   
      if (userData == USER_PROCESS_EVENT)
      {
          //here we need to loop through all the process objects and call
          //their check method
          for (loop = 0;loop < mnProcessCount;loop++)
          {
             mpProcessList[loop]->ApplyUserRequestedState();
          }
#ifdef DEBUG
          osPrintf("Last state change check occurred at:         %s\n",dateString.data());
#endif /* DEBUG */
      }
      else
      {
          //here we need to loop through all the process objects and request reports to be sent
          //yes, I could put the report call in the loop below with check, but I'd rather 
          //have all processes flush and report before adding any new ones.
          for (loop = 0;loop < mnProcessCount;loop++)
          {
             mpProcessList[loop]->sendReports();
          }

          //here we need to loop through all the process objects and call
          //their check method
          for (loop = 0;loop < mnProcessCount;loop++)
          {
             mpProcessList[loop]->check();
          }

#ifdef DEBUG
          osPrintf("Last check occurred at:         %s\n",dateString.data());
#endif /* DEBUG */
      }
   }

   return(returnValue);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */

