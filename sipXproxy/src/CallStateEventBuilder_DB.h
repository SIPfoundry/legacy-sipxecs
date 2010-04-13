//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _CallStateEventBuilder_DB_h_
#define _CallStateEventBuilder_DB_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "CallStateEventBuilder.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * This CallStateEventBuilder constructs events as database rows according
 * to the specification doc/cdr/call-state-events.html
 *
 * for usage of the event generation interfaces, see CallStateEventBuilder
 */
class CallStateEventBuilder_DB : public CallStateEventBuilder
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/* ============================ CREATORS ================================== */

   /// Instantiate an event builder and set the observer name for its events
   CallStateEventBuilder_DB(const char* observerDnsName ///< the DNS name to be recorded in all events
                             );

   /// Destructor
   virtual ~CallStateEventBuilder_DB();

   /// Generate a metadata event.
   void observerEvent(int sequenceNumber, ///< for ObserverReset, this should be zero
                      const OsTime& timestamp,      ///< obtain using getCurTime(OsTime)
                      ObserverEvent eventCode,
                      const char* eventMsg ///< for human consumption
                      );
   /**<
    * This method generates a complete event - it does not require that the callEventComplete method be called.
    */

   /// Begin a Call Request Event - an INVITE without a to tag has been observed
   void callRequestEvent(int sequenceNumber,
                         const OsTime& timestamp,      ///< obtain using getCurTime(OsTime)
                         const UtlString& contact,
                         const UtlString& references,
                         const UtlString& branch_id,
                         int              via_count,
                         const bool callerInternal
                         );
   /**<
    * Requires:
    *   - callRequestEvent
    *   - addCallData (the toTag in the addCallRequest will be a null string)
    *   - addEventVia (at least for via index zero)
    *   - completeCallEvent
    */

   /// Begin a Call Setup Event - a 2xx response to an INVITE has been observed
   void callSetupEvent(int sequenceNumber,
                       const OsTime& timestamp,      ///< obtain using getCurTime(OsTime)
                       const UtlString& contact,
                       const UtlString& calleeRoute,
                       const UtlString& branch_id,
                       int              via_count
                       );
   /**<
    * Requires:
    *   - callSetupEvent
    *   - addCallData
    *   - addEventVia (at least for via index zero)
    *   - completeCallEvent
    */

   /// Begin a Call Failure Event - an error response to an INVITE has been observed
   void callFailureEvent(int sequenceNumber,
                         const OsTime& timestamp,      ///< obtain using getCurTime(OsTime)
                         const UtlString& branch_id,
                         int via_count,
                         int statusCode,
                         const UtlString& statusMsg
                         );
   /**<
    * Requires:
    *   - callFailureEvent
    *   - addCallData
    *   - addEventVia (at least for via index zero)
    *   - completeCallEvent
    */

   /// Begin a Call End Event - a BYE request has been observed
   void callEndEvent(int sequenceNumber,
                     const OsTime& timestamp      ///< obtain using getCurTime(OsTime)
                     );
   /**<
    * Requires:
    *   - callEndEvent
    *   - addCallData
    *   - addEventVia (at least for via index zero)
    *   - completeCallEvent
    */

   /// Begin a Call transfer event - a REFER event has been observed
   void callTransferEvent(int sequenceNumber,
                          const OsTime& timeStamp,
                          const UtlString& contact,
                          const UtlString& refer_to,
                          const UtlString& referred_by,
                          const UtlString& request_uri);
   /**<
    * Requires:
    *   - callTransferEvent
    *   - addCallData
    *   - completeCallEvent
    */

   /// Add the dialog and call information for the event being built.
   void addCallData(const int cseqNumber,
                    const UtlString& callId,
                    const UtlString& fromTag,  /// may be a null string
                    const UtlString& toTag,    /// may be a null string
                    const UtlString& fromField,
                    const UtlString& toField
                    );

   /// Add a via element for the event
   void addEventVia(const UtlString& via
                    );
   /**<
    * Record a Via from the message for this event
    * Calls to this routine are in reverse cronological order - the last
    * call for an event should be the via added by the message originator
    */

   /// Indicates that all information for the current call event has been added.
   void completeCallEvent();

   /// Copies the element into the provided UtlString
   bool finishElement(UtlString& event);
   /**<
    * @returns
    * - true if the returned element is validly constructed
    * - false if not (a caller error)
    */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:
   UtlString mCurrentEvent;
   UtlString mCallInfo;
   UtlString mReferHeader;
   UtlString mLaterElement;
   UtlString mContactElement;
   UtlString mReferElement;
   UtlString mFailureElement;
   UtlString mViaHeader;
   UtlString mEndElement;
   UtlString mRequestUri;
   UtlString mReferences;
   UtlString mCallerInternal;
   UtlString mCalleeRoute;
   UtlString mBranchId;
   UtlString mViaCount;
   bool      mEventComplete;

   void newEvent(int sequenceNumber,
                 const OsTime& timestamp,
                 const char* elementStart,
                 const char eventType = '-'
                 );

   void reset();

   void replaceSingleQuotes(const UtlString& value, UtlString& newValue);

   /// no copy constructor or assignment operator
   CallStateEventBuilder_DB(const CallStateEventBuilder_DB& rCallStateEventBuilderDB);
   CallStateEventBuilder_DB operator=(const CallStateEventBuilder_DB& rCallStateEventBuilderDB);
};

/* ============================ INLINE METHODS ============================ */

#endif    // _CallStateEventBuilder_DB_h_
