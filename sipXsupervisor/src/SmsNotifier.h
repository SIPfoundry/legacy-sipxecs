//
//
// Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _SmsNotifier_h_
#define _SmsNotifier_h_

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
class SmsNotifier : public NotifierBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   ///Default constructor
   SmsNotifier();

   ///Destructor
   virtual ~SmsNotifier();

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
         TiXmlElement* element,            ///< pointer to xml config element for this notifier
         TiXmlElement* dummy               ///< currently unused
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
   UtlString mSmsStrFrom;                ///< email comes From this string
   UtlString mSmsStrSubject;             ///< email subject line
   UtlString mSmsStrHost;                ///< Host:

   /// no copy constructor
   SmsNotifier(const SmsNotifier& nocopy);

   ///Assignment operator
   SmsNotifier& operator=(const SmsNotifier& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SmsNotifier_h_

