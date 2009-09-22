//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _SipNotifyStateTask_h_
#define _SipNotifyStateTask_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsServerTask.h>
#include <os/OsDefs.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipUserAgent;
class SipMessage;
class CommandSecurityPolicy;
class OsQueuedEvent;
class OsTimer;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class SipNotifyStateTask : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    static void defaultReboot();

    static void defaultBinaryMessageWaiting(const char* toUrl,
                                      UtlBoolean newMessages);

    static void defaultDetailMessageWaiting(const char* toUrl,
                                      const char* messageMediaType,
                                      UtlBoolean absoluteValues,
                                      int totalNewMessages,
                                      int totalOldMessages,
                                      int totalUntouchedMessages,
                                      int urgentUntouchedMessages,
                                      int totalSkippedMessages,
                                      int urgentSkippedMessages,
                                      int totalFlaggedMessages,
                                      int urgentFlaggedMessages,
                                      int totalReadMessages,
                                      int urgentReadMessages,
                                      int totalAnsweredMessages,
                                      int urgentAnsweredMessages,
                                      int totalDeletedMessages,
                                      int urgentDeletedMessages);
    // totalNewMessages and totalOldMessages are summary totals
    //      accross all other categories.
    //! param: messageMediaType = "Voicemail", "Email", "Fax", "Video", etc
    //! param: absoluteValues - the message counts are absolute counts (TRUE) or deltas (FALSE)
    // If these are absolute values negative number for a message count means unspecified
    // If these are deltas there is know way to know if the values are not set


/* ============================ CREATORS ================================== */

   SipNotifyStateTask(const UtlString& checkSyncPolicy, SipUserAgent* pSipUserAgent = NULL);
     //:Default constructor

   virtual
   ~SipNotifyStateTask();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual UtlBoolean handleMessage(OsMsg& eventMessage);

/* ============================ ACCESSORS ================================= */

   void setRebootFunction(void (*rebootNotifyFunction)());

   void setBinaryMessageWaitingFunction(void (*binaryMessageWaitingFunc)(
                                        const char* toUrl,
                                        UtlBoolean newMessages));

   void setDetailMessageWaitingFunction(void (*requestProcessor)(
                                      const char* toUrl,
                                      const char* messageMediaType,
                                      UtlBoolean absoluteValues,
                                      int totalNewMessages,
                                      int totalOldMessages,
                                      int totalUntouchedMessages,
                                      int urgentUntouchedMessages,
                                      int totalSkippedMessages,
                                      int urgentSkippedMessages,
                                      int totalFlaggedMessages,
                                      int urgentFlaggedMessages,
                                      int totalReadMessages,
                                      int urgentReadMessages,
                                      int totalAnsweredMessages,
                                      int urgentAnsweredMessages,
                                      int totalDeletedMessages,
                                      int urgentDeletedMessages));

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   OsStatus handleCheckSyncEvent(const SipMessage* source) ;

   UtlBoolean scheduleRunScript(UtlString* pContent,
                               CommandSecurityPolicy *pPolicy,
                               int seconds) ;

   OsStatus doRunScript(UtlString* pContent,
                        CommandSecurityPolicy *pPolicy) ;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    SipUserAgent*  mpSipUserAgent;
    UtlString       mCheckSyncPolicy ;  // policy for handling check-sync events
    OsQueuedEvent* mpRunScriptEvent ;  // queued event for scripts (if phone is busy)
    OsTimer*       mpRunScriptTimer ;  // timer for scripts (if phone is busy)

    void(*mpRebootFunction)();

    void(*mpBinaryMessageWaitingFunction)(const char* toUrl,
                                        UtlBoolean newMessages);

    void(*mpDetailedMessageWaitingFunction)(
                                      const char* toUrl,
                                      const char* messageMediaType,
                                      UtlBoolean absoluteValues,
                                      int totalNewMessages,
                                      int totalOldMessages,
                                      int totalUntouchedMessages,
                                      int urgentUntouchedMessages,
                                      int totalSkippedMessages,
                                      int urgentSkippedMessages,
                                      int totalFlaggedMessages,
                                      int urgentFlaggedMessages,
                                      int totalReadMessages,
                                      int urgentReadMessages,
                                      int totalAnsweredMessages,
                                      int urgentAnsweredMessages,
                                      int totalDeletedMessages,
                                      int urgentDeletedMessages);

    UtlBoolean getStatusTotalUrgent(const char* status,
                                 UtlBoolean absoluteValues,
                                 int parameterIndex,
                                 int& total,
                                 int& urgent);

   SipNotifyStateTask(const SipNotifyStateTask& rSipNotifyStateTask);
     //:Copy constructor (disabled)

   SipNotifyStateTask& operator=(const SipNotifyStateTask& rhs);
     //:Assignment operator (disabled)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipNotifyStateTask_h_
