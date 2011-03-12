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
//#include <os/OsDateTime.h>
#include <os/OsFS.h>
#include <os/linux/OsDateTimeLinux.h>
#include "ACDRtRecord.h"
#include "ACDCall.h"
#include "ACDAgent.h"
#include "ACDServer.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
extern OsSysLogPriority gACD_DEBUG;

// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDRtRecord::ACDRtRecord
//
//  SYNOPSIS:
//
//  DESCRIPTION: Default constructor
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDRtRecord::ACDRtRecord(UtlString& rLogDirectory) : mLock(OsMutex::Q_FIFO)
{
   // Get/Apply RT Log Filename
   //
   if (rLogDirectory.isNull() || !OsFileSystem::exists(rLogDirectory)) {
      // If the log file directory exists use that, otherwise place the log
      // in the current directory
      OsPath workingDirectory;
      if (OsFileSystem::exists(CONFIG_LOG_DIR)) {
         rLogDirectory = CONFIG_LOG_DIR;
         OsPath path(rLogDirectory);
         path.getNativePath(workingDirectory);

         OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDRtRecord::ACDRtRecord [LogDir %s]",
                 rLogDirectory.data());
      }
      else {
         OsPath path;
         OsFileSystem::getWorkingDirectory(path);
         path.getNativePath(workingDirectory);

         OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDRtRecord::ACDRtRecord [LogDir %s]",
                 workingDirectory.data());
      }

      mEventFile = workingDirectory +
                   OsPathBase::separator +
                   RT_LOG_FILE;
   }
   else {
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDRtRecord::ACDRtRecord [LogDir %s]",
                 rLogDirectory.data());

      mEventFile = rLogDirectory +
                   OsPathBase::separator +
                   RT_LOG_FILE;
   }

   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDRtRecord::ACDRtRecord - Created ACDRtRecord object");
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDRtRecord::~ACDRtRecord
//
//  SYNOPSIS:
//
//  DESCRIPTION: Destructor
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ACDRtRecord::~ACDRtRecord()
{
   OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDRtRecord::~ACDRtRecord Deleting ACDRtRecord object");
}

/* ============================ MANIPULATORS ============================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDRtRecord::appendCallEvent()
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDRtRecord::appendCallEvent(int event, UtlString queueString, ACDCall* pCallRef, bool agentRec)
{

   // read the local time
   UtlString dateString = NULL;
   OsDateTime nowTime;
   OsDateTime::getCurTime(nowTime);
   nowTime.getIsoTimeStringZus(dateString);
   char realTimeDataRecord[512];

   UtlString eventString;
   eventString = getEventString(EVENT_CALL, event);
   if (NULL != eventString.data()) {
      if (agentRec) {
         sprintf(realTimeDataRecord, "%s : %s : %s : %s : %s : %d\n",
            eventString.data(), dateString.data(), queueString.data(),
            pCallRef->getAcdAgent()->getUriString()->data(),
            pCallRef->getCallIdentity(), pCallRef->getCallHandle());
      }
      else if (TERMINATE == event) {
         sprintf(realTimeDataRecord, "%s : %s : %s : %d\n",
            eventString.data(), dateString.data(),
            pCallRef->getCallIdentity(), pCallRef->getCallHandle());

      }
      else {
         sprintf(realTimeDataRecord, "%s : %s : %s : %s : %d\n",
            eventString.data(), dateString.data(),
            queueString.data(), pCallRef->getCallIdentity(),
            pCallRef->getCallHandle());

      }

      mLock.acquire();

      mFp = fopen( mEventFile.data(), "a" );
      if ( mFp ) {
         fprintf( mFp, realTimeDataRecord);
         fflush(mFp);
         fclose(mFp);
      }
      else {
         OsSysLog::add(FAC_ACD, PRI_ERR,
                       "ACDRtRecord::appendCallEvent - File not found for recording event ACDRtRecord::appendCallEvent [event %s, error %d '%s']",
                       eventString.data(), errno, strerror(errno));
      }

      mLock.release();
   }
}

void ACDRtRecord::appendTransferCallEvent(int event, ACDCall* pCallRef)
{

   // read the local time
   UtlString dateString = NULL;
   OsDateTime nowTime;
   OsDateTime::getCurTime(nowTime);
   nowTime.getIsoTimeStringZus(dateString);
   char realTimeDataRecord[512];

   UtlString eventString;
   eventString = getEventString(EVENT_TRANSFER, event);
   if ((NULL != eventString.data()) && (NULL != pCallRef)) {
      sprintf(realTimeDataRecord, "%s : %s : %s : %s : %d\n",
         eventString.data(), dateString.data(),
         pCallRef->getAcdAgent() ?
            pCallRef->getAcdAgent()->getUriString()->data() : "(none)",
         pCallRef->getCallIdentity(), pCallRef->getCallHandle());

      mLock.acquire();

      mFp = fopen( mEventFile.data(), "a" );
      if ( mFp ) {
         fprintf( mFp, realTimeDataRecord);
         fflush(mFp);
         fclose(mFp);
      }
      else {
         OsSysLog::add(FAC_ACD, PRI_ERR,
                       "ACDRtRecord::appendTransferCallEvent - File not found for recording event ACDRtRecord::appendTransferCallEvent [event %s, error %d '%s']",
                       eventString.data(), errno, strerror(errno));
      }

      mLock.release();
   }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//  NAME:        ACDRtRecord::appendAgentEvent()
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////
void ACDRtRecord::appendAgentEvent(int event, UtlString* pQueueListString, ACDAgent* pAgentRef)
{

   // read the local time
   UtlString dateString = NULL;
   OsDateTime nowTime;
   OsDateTime::getCurTime(nowTime);
   nowTime.getIsoTimeStringZus(dateString);
   char realTimeDataRecord[512];

   UtlString eventString;
   eventString = getEventString(EVENT_AGENT, event);
   if (NULL != eventString.data()) {
      OsSysLog::add(FAC_ACD, gACD_DEBUG, "ACDRtRecord::appendAgentEvent event %s, queue %s",
                 eventString.data(), pQueueListString->data());
      sprintf(realTimeDataRecord, "%s : %s : %s : %s\n", eventString.data(), dateString.data(),
              pQueueListString->data(), pAgentRef->getUriString()->data());

      mLock.acquire();

      mFp = fopen( mEventFile.data(), "a" );
      if ( mFp ) {
         fprintf( mFp, realTimeDataRecord);
         fflush(mFp);
         fclose(mFp);
      }
      else {
         OsSysLog::add(FAC_ACD, PRI_ERR,
                       "ACDRtRecord::appendAgentEvent - File not found for recording event ACDRtRecord::appendAgentEvent [event %s, error %d '%s']",
                       eventString.data(), errno, strerror(errno));
      }

      mLock.release();
   }

}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDRtRecord::appendAcdEvent()
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

void ACDRtRecord::appendAcdEvent(int event)
{

   // read the local time
   UtlString dateString = NULL;
   OsDateTime nowTime;
   OsDateTime::getCurTime(nowTime);
   nowTime.getIsoTimeStringZus(dateString);
   char realTimeDataRecord[512];

   UtlString eventString;
   eventString = getEventString(EVENT_ACD, event);
   if (NULL != eventString.data()) {
      sprintf(realTimeDataRecord, "%s : %s\n", eventString.data(), dateString.data());

      mLock.acquire();

      mFp = fopen( mEventFile.data(), "a" );
      if ( mFp ) {
         fprintf( mFp, realTimeDataRecord);
         fflush(mFp);
         fclose(mFp);
      }
      else {
         OsSysLog::add(FAC_ACD, PRI_ERR,
                       "appendAcdEvent - File not found for recording event ACDRtRecord::appendCallEvent [event %s, error %d '%s']",
                       eventString.data(), errno, strerror(errno));
      }

      mLock.release();
   }

}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ACDRtRecord::getEventString(int event_type, int event)
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

UtlString ACDRtRecord::getEventString(int event_type, int event)
{
   if (event_type < EVENT_MIN || event_type > EVENT_MAX) {
      OsSysLog::add(FAC_ACD, PRI_ERR, "ACDRtRecord::getEventString - Invalid event type [%d] ACDRtRecord::getEventString",
					event_type);
      return NULL;
   }

   if (EVENT_CALL == event_type) {
      switch (event) {
         case ENTER_QUEUE: return ("enter-queue");
         case PICK_UP: return ("pick-up");
         case TERMINATE: return ("terminate");
         default : {
         OsSysLog::add(FAC_ACD, PRI_ERR, "ACDRtRecord::getEventString - Invalid event [%d] ACDRtRecord::getEventString",
					   event);
         return NULL;
         }
      }
   }
   else if (EVENT_AGENT == event_type) {
      switch (event) {
        case SIGNED_IN_AGENT: return ("sign-in-agent");
        case SIGNED_OUT_AGENT: return ("sign-out-agent");
        default : {
        OsSysLog::add(FAC_ACD, PRI_ERR, "ACDRtRecord::getEventString - Invalid event [%d] ACDRtRecord::getEventString",
			              event);
        return NULL;
        }
      }
   }
   else if (EVENT_ACD == event_type) {
       switch (event) {
         case START_ACD: return ("start-acd");
	 case STOP_ACD: return ("stop-acd");
	 default : {
	 OsSysLog::add(FAC_ACD, PRI_ERR, "ACDRtRecord::getEventString - Invalid event [%d] ACDRtRecord::getEventString",
							 event);
	 return NULL;
	 }
       }
    }
   else if (EVENT_TRANSFER == event_type) {
       switch (event) {
         case TRANSFER: return ("transfer");
	 case TERMINATE_TRANSFER: return ("terminate");
	 default : {
	 OsSysLog::add(FAC_ACD, PRI_ERR, "ACDRtRecord::getEventString - Invalid event [%d] ACDRtRecord::getEventString",
							 event);
	 return NULL;
	 }
       }
    }

   return NULL;
}


/* ============================ ACCESSORS ============================== */
