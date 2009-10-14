//
//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _NotifierBase_h_
#define _NotifierBase_h_

// SYSTEM INCLUDES
#include "os/OsStatus.h"
#include "xmlparser/ExtractContent.h"
#include "AlarmData.h"

// APPLICATION INCLUDES

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlString;

/// Base class for alarm Notifiers
class NotifierBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   ///Default constructor
   NotifierBase();

   ///Destructor
   virtual ~NotifierBase();

/* ============================ MANIPULATORS ============================== */

   ///Assignment operator
   NotifierBase& operator=(const NotifierBase& rhs);

   /// Perform any configured actions for this alarm
   virtual OsStatus handleAlarm( const OsTime      alarmTime,     ///< time alarm was reported
                                 const UtlString&  callingHost,   ///< host on which event occurred
                                 const cAlarmData* alarmData,     ///< configured data for the alarm
                                 const UtlString&  alarmMessage   ///< formatted message with parameters
   ) = 0;
   /**<
    * NOTE: handleAlarm is called within the xmlrpc processing, and should execute quickly.
    * If the call might block (e.g. sending emails), then send an asynchronous request
    * to a separate task.
    * @returns OS_SUCCESS if the alarm was processed by the Notifier
   */

   /// Initialize notifier (including loading parameters from the provided xml element).
   virtual OsStatus init(
         TiXmlElement* element1,           ///< pointer to xml config element1 for this notifier
         TiXmlElement* element2            ///< pointer to xml config element2 for this notifier
         )
      = 0;

   /// Load strings needed by this notifier from the provided xml element.
   virtual OsStatus initStrings(
         TiXmlElement* element             ///< pointer to xml config element for this notifier
         )
      {return OS_SUCCESS;}


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   /// no copy constructor
   NotifierBase(const NotifierBase& nocopy);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _NotifierBase_h_

