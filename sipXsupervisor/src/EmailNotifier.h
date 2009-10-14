//
//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _EmailNotifier_h_
#define _EmailNotifier_h_

// SYSTEM INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "utl/UtlHashMap.h"
#include "NotifierBase.h"

// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Sends emails containing alarm description and resolution to configured contacts
class EmailNotifier : public NotifierBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   ///Default constructor
   EmailNotifier();

   ///Destructor
   virtual ~EmailNotifier();

/* ============================ MANIPULATORS ============================== */

   /// Format and send email notification to any configured contacts.
   virtual OsStatus handleAlarm(
         const OsTime alarmTime,           ///< time alarm was reported
         const UtlString& callingHost,     ///< host on which event occurred
         const cAlarmData* alarmData,      ///< pointer to alarmData structure
         const UtlString& alarmParameters  ///< formatted message with parameters
         );

   /// Initialize notifier (including loading parameters from the provided xml element).
   virtual OsStatus init(
         TiXmlElement* emailElement,       ///< pointer to xml config element for this notifier
         TiXmlElement* groupElement        ///< pointer to xml group config element for this notifier
         );

   /// Load strings needed by this notifier from the provided xml element.
   virtual OsStatus initStrings(
         TiXmlElement* element             ///< pointer to xml config element for this notifier
         );


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   UtlHashMap mContacts;                   ///< list of contacts to send mail to

   UtlString mSmtpServer;                  ///< default localhost; cannot be configured
   UtlString mReplyTo;                     ///< default postmaster\@localhost; can be configured

   // localizable strings to build email notification message
   UtlString mEmailStrFrom;                ///< email comes From this string
   UtlString mEmailStrSubject;             ///< email subject line
   UtlString mEmailStrIntro;               ///< first line of email
   UtlString mEmailStrAlarm;               ///< Alarm:
   UtlString mEmailStrTime;                ///< Time:
   UtlString mEmailStrHost;                ///< Host:
   UtlString mEmailStrSeverity;            ///< Severity:
   UtlString mEmailStrDescription;         ///< Description:
   UtlString mEmailStrResolution;          ///< Resolution

   /// no copy constructor
   EmailNotifier(const EmailNotifier& nocopy);

   ///Assignment operator
   EmailNotifier& operator=(const EmailNotifier& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _EmailNotifier_h_

