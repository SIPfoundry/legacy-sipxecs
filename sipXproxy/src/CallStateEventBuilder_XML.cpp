//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

#include "os/OsFS.h"
#include "os/OsSysLog.h"
#include "os/OsDateTime.h"
#include "utl/XmlContent.h"

// APPLICATION INCLUDES
#include "CallStateEventBuilder_XML.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// Note: the unit test will fail if this is defined, since it compares against expected results without it.
#undef PRETTYPRINT_EVENTS
#ifdef PRETTYPRINT_EVENTS
#  define PP_LF "\n"
#  define PP_IN "  "
#else
#  define PP_LF ""
#  define PP_IN ""
#endif

const char* CallEvent_Observer_Start =
"<call_event>" PP_LF
PP_IN "<observer>"
;

const char* ObserverEnd_ObsSeqStart =
"</observer>" PP_LF
PP_IN "<obs_seq>";

const char* ObsSeqEnd_ObsTimeStart =
"</obs_seq>" PP_LF
PP_IN "<obs_time>"
;

const char* ObsTimeEnd =
"</obs_time>" PP_LF
;

const char* CallEventElementEnd =
"</call_event>" PP_LF
;

const char* ObsMsgStart =
PP_IN "<obs_msg>" PP_LF
PP_IN PP_IN "<obs_status>"
;
const char* ObsMsgMiddle =
"</obs_status>" PP_LF
PP_IN PP_IN "<obs_text>"
;
const char* ObsText_Schema_ObsMsg_End =
"</obs_text>" PP_LF
PP_IN PP_IN "<uri>http://www.sipfoundry.org/sipX/schema/xml/cse-01-00</uri>" PP_LF
PP_IN "</obs_msg>" PP_LF
;

const char* CallRequestElementStart =
PP_IN "<call_request>" PP_LF
;
const char* CallRequestElementEnd =
PP_IN "</call_request>" PP_LF
;

const char* CallSetupElementStart =
PP_IN "<call_setup>" PP_LF
;
const char* CallSetupElementEnd =
PP_IN "</call_setup>" PP_LF
;

const char* CallFailureElementStart =
PP_IN "<call_failure>" PP_LF
;
const char* CallFailureElementEnd =
PP_IN "</call_failure>" PP_LF
;

const char* CallEndElementStart =
PP_IN "<call_end>" PP_LF
;
const char* CallEndElementEnd =
PP_IN "</call_end>" PP_LF
;

const char* Call_Dialog_CallId_Start =
PP_IN PP_IN "<call>" PP_LF
PP_IN PP_IN PP_IN "<dialog>" PP_LF
PP_IN PP_IN PP_IN PP_IN "<call_id>"
;
const char* CallIdEnd =
"</call_id>" PP_LF
;
const char* FromTagStart =
PP_IN PP_IN PP_IN PP_IN "<from_tag>"
;
const char* FromTagEnd =
"</from_tag>" PP_LF
;
const char* ToTagStart =
PP_IN PP_IN PP_IN PP_IN "<to_tag>"
;
const char* ToTagEnd =
"</to_tag>" PP_LF
;
const char* DialogEnd_FromFieldStart =
PP_IN PP_IN PP_IN "</dialog>" PP_LF
PP_IN PP_IN PP_IN "<from>"
;
const char* FromFieldEnd_ToFieldStart =
"</from>" PP_LF
PP_IN PP_IN PP_IN "<to>"
;
const char* ToField_CallEnd =
"</to>" PP_LF
PP_IN PP_IN "</call>" PP_LF
;

const char* ContactElementStart =
PP_IN PP_IN "<contact>"
;
const char* ContactElementEnd =
"</contact>" PP_LF
;

const char* Response_Status_Start =
PP_IN PP_IN "<response>" PP_LF
PP_IN PP_IN PP_IN"<status>"
;

const char* StatusEnd_ReasonStart =
"</status>" PP_LF
PP_IN PP_IN PP_IN"<reason>"
;
const char* Reason_Response_End =
"</reason>" PP_LF
PP_IN PP_IN "</response>" PP_LF
;
const char* ViaStart =
PP_IN PP_IN "<via>"
;
const char* ViaEnd =
"</via>" PP_LF
;

#define CONTENT_BUF_MAX 2048

const char* EventText[] =
{
   "BuilderReset",
   "CallRequestEvent",
   "CallSetupEvent",
   "CallFailureEvent",
   "CallEndEvent",
   "AddCallData",
   "AddVia",
   "CompleteCallEvent"
};

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/// Instantiate an event builder and set the observer name for its events
CallStateEventBuilder_XML::CallStateEventBuilder_XML(const char* observerDnsName ///< the DNS name to be recorded in all events
                                                     ) :
   CallStateEventBuilder(observerDnsName)
{
}


/// Destructor
CallStateEventBuilder_XML::~CallStateEventBuilder_XML()
{
}


/**
 * Generate a metadata event.
 * This method generates a complete event - it does not require that the callEventComplete method be called.
 */
void CallStateEventBuilder_XML::observerEvent(int sequenceNumber, ///< for ObserverReset, this should be zero
                                              const OsTime& timestamp,      ///< obtained using getCurTime(OsTime)
                                              ObserverEvent eventCode,
                                              const char* eventMsg ///< for human consumption
                                              )
{
   BuilderMethod eventMethod;
   switch (eventCode)
   {
   case ObserverReset:
      reset(); // because this event is ok any time, clear out any partial event.
      eventMethod = BuilderReset;
      break;
   default:
      assert(false);
      OsSysLog::add(FAC_SIP, PRI_ERR, "observerEvent: invalid eventCode %d", eventCode);
      eventMethod = InvalidEvent;
      break;
   }

   if (builderStateIsOk(eventMethod))
   {
      newEvent(sequenceNumber, timestamp, ObsMsgStart);
      char ec[11];
      sprintf(ec, "%d", eventCode);
      mCurrentEvent.append(ec);
      mCurrentEvent.append(ObsMsgMiddle);
      XmlEscape(mCurrentEvent, eventMsg);
      mCurrentEvent.append(ObsText_Schema_ObsMsg_End);
      mEventComplete = true;
   }
   else
   {
      assert(false);
      OsSysLog::add(FAC_SIP, PRI_ERR, "observerEvent: %d not allowed.", eventCode);
   }
}

/// Begin a Call Request Event - an INVITE without a to tag has been observed
/**
 * Requires:
 *   - callRequestEvent
 *   - addCallData (the toTag in the addCallRequest will be a null string)
 *   - addEventVia (at least for via index zero)
 *   - completeCallEvent
 */
void CallStateEventBuilder_XML::callRequestEvent(int sequenceNumber,
                                                 const OsTime& timestamp,      ///< obtain using getCurTime(OsTime)
                                                 const UtlString& contact,
                                                 const UtlString& references,
                                                 const UtlString& branch_id,
                                                 int              via_count,
                                                 const bool callerInternal
                                                 )
{
   if (builderStateIsOk(CallRequestEvent))
   {
      newEvent(sequenceNumber, timestamp, CallRequestElementStart);

      mLaterElement.append(ContactElementStart);
      XmlEscape(mLaterElement, contact);
      mLaterElement.append(ContactElementEnd);

      mEndElement = CallRequestElementEnd;
   }
   else
   {
      assert(false);
      OsSysLog::add(FAC_SIP, PRI_ERR, "CallStateEventBuilder_XML::callRequestEvent not allowed.");
   }
}


/// Begin a Call Setup Event - a 2xx response to an INVITE has been observed
/**
 * Requires:
 *   - callSetupEvent
 *   - addCallData
 *   - addEventVia (at least for via index zero)
 *   - completeCallEvent
 */
void CallStateEventBuilder_XML::callSetupEvent(int sequenceNumber,
                                               const OsTime& timestamp,      ///< obtain using getCurTime(OsTime)
                                               const UtlString& contact,
                                               const UtlString& calleeRoute,
                                               const UtlString& branch_id,
                                               int              via_count
                                               )
{
   if (builderStateIsOk(CallSetupEvent))
   {
      newEvent(sequenceNumber, timestamp, CallSetupElementStart);

      mLaterElement.append(ContactElementStart);
      XmlEscape(mLaterElement, contact);
      mLaterElement.append(ContactElementEnd);

      mEndElement = CallSetupElementEnd;
   }
   else
   {
      assert(false);
      OsSysLog::add(FAC_SIP, PRI_ERR, "CallStateEventBuilder_XML::callSetupEvent not allowed.");
   }
}


/// Begin a Call Failure Event - an error response to an INVITE has been observed
/**
 * Requires:
 *   - callFailureEvent
 *   - addCallData
 *   - addEventVia (at least for via index zero)
 *   - completeCallEvent
 */
void CallStateEventBuilder_XML::callFailureEvent(int sequenceNumber,
                                                 const OsTime& timestamp,      ///< obtain using getCurTime(OsTime)
                                                 const UtlString& branch_id,
                                                 int via_count,
                                                 int statusCode,
                                                 const UtlString& statusMsg
                                                 )
{
   if (builderStateIsOk(CallFailureEvent))
   {
      newEvent(sequenceNumber, timestamp, CallFailureElementStart);

      mLaterElement.append(Response_Status_Start);
      char sc[11];
      sprintf(sc, "%d", statusCode);
      mLaterElement.append(sc);
      mLaterElement.append(StatusEnd_ReasonStart);
      XmlEscape(mLaterElement, statusMsg);
      mLaterElement.append(Reason_Response_End);

      mEndElement = CallFailureElementEnd;
   }
   else
   {
      assert(false);
      OsSysLog::add(FAC_SIP, PRI_ERR, "CallStateEventBuilder_XML::callFailureEvent not allowed.");
   }
}


/// Begin a Call End Event - a BYE request has been observed
/**
 * Requires:
 *   - callEndEvent
 *   - addCallData
 *   - addEventVia (at least for via index zero)
 *   - completeCallEvent
 */
void CallStateEventBuilder_XML::callEndEvent(const int sequenceNumber,
                                             const OsTime& timestamp      ///< obtain using getCurTime(OsTime)
                                             )
{
   if (builderStateIsOk(CallEndEvent))
   {
      newEvent(sequenceNumber, timestamp, CallEndElementStart);

      mEndElement = CallEndElementEnd;
   }
   else
   {
      assert(false);
      OsSysLog::add(FAC_SIP, PRI_ERR, "CallStateEventBuilder_XML::callEndEvent not allowed.");
   }
}


void CallStateEventBuilder_XML::callTransferEvent(int mSequenceNumber,
                                                  const OsTime& timeStamp,
                                                  const UtlString& contact,
                                                  const UtlString& refer_to,
                                                  const UtlString& referred_by,
                                                  const UtlString& request_uri)
{
   // Not logging transfer events in XML
}


/// Add the dialog and call information for the event being built.
void CallStateEventBuilder_XML::addCallData(const int cseqNumber,
                                            const UtlString& callId,
                                            const UtlString& fromTag,  /// may be a null string
                                            const UtlString& toTag,    /// may be a null string
                                            const UtlString& fromField,
                                            const UtlString& toField
                                            )
{
   if (builderStateIsOk(AddCallData))
   {
      mCallInfo.append(Call_Dialog_CallId_Start);
      XmlEscape(mCallInfo, callId);
      mCallInfo.append(CallIdEnd);
      if (!fromTag.isNull())
      {
         mCallInfo.append(FromTagStart);
         XmlEscape(mCallInfo, fromTag);
         mCallInfo.append(FromTagEnd);
      }
      if (!toTag.isNull())
      {
         mCallInfo.append(ToTagStart);
         XmlEscape(mCallInfo, toTag);
         mCallInfo.append(ToTagEnd);
      }
      mCallInfo.append(DialogEnd_FromFieldStart);
      XmlEscape(mCallInfo, fromField);
      mCallInfo.append(FromFieldEnd_ToFieldStart);
      XmlEscape(mCallInfo, toField);
      mCallInfo.append(ToField_CallEnd);
   }
   else
   {
      assert(false);
      OsSysLog::add(FAC_SIP, PRI_ERR, "CallStateEventBuilder_XML::callEndEvent not allowed.");
   }
}


/// Add a via element for the event
/**
 * Record a Via from the message for this event
 * Calls to this routine are in reverse cronological order - the last
 * call for an event should be the via added by the message originator
 */
void CallStateEventBuilder_XML::addEventVia(const UtlString& via
                                            )
{
   if (builderStateIsOk(AddVia))
   {
      // construct the element locally
      UtlString viaElement;
      viaElement.append(ViaStart);
      XmlEscape(viaElement, via);
      viaElement.append(ViaEnd);

      // prepend it to the recorded vias so that the first one is first
      mViaHeader.prepend(viaElement);
   }
   else
   {
      assert(false);
      OsSysLog::add(FAC_SIP, PRI_ERR, "CallStateEventBuilder_XML::callEndEvent not allowed.");
   }
}


/// Indicates that all information for the current call event has been added.
void CallStateEventBuilder_XML::completeCallEvent()
{
   if (builderStateIsOk(CompleteCallEvent))
   {
      mEventComplete = true;
   }
   else
   {
      assert(false);
      OsSysLog::add(FAC_SIP, PRI_ERR, "CallStateEventBuilder_XML::completeCallEvent not allowed.");
   }
}

/// Clears all the object state
void CallStateEventBuilder_XML::reset()
{
   mCurrentEvent.remove(0);
   mCallInfo.remove(0);
   mViaHeader.remove(0);
   mLaterElement.remove(0);
   mEndElement.remove(0);
   mEventComplete = false;
}

void CallStateEventBuilder_XML::newEvent(int sequenceNumber,
                                         const OsTime& timestamp,      ///< obtain using getCurTime(OsTime)
                                         const char* elementStart
                                         )
{
   // assemble the parts common to all events
   mCurrentEvent = CallEvent_Observer_Start;
   XmlEscape(mCurrentEvent, observerName);
   mCurrentEvent.append(ObserverEnd_ObsSeqStart);
   char sn[11];
   sprintf(sn, "%d", sequenceNumber);
   mCurrentEvent.append(sn);
   mCurrentEvent.append(ObsSeqEnd_ObsTimeStart);
   OsDateTime timeValue(timestamp);
   UtlString timeString;
   timeValue.getIsoTimeStringZ(timeString);
   mCurrentEvent.append(timeString); // this format has nothing to escape
   mCurrentEvent.append(ObsTimeEnd);

   // add the start tag for the new element
   mCurrentEvent.append(elementStart);
}

/// Copies the element into the provided UtlString
bool  CallStateEventBuilder_XML::finishElement(UtlString& event)
/**<
 * @returns
 * - true if the returned element is validly constructed
 * - false if not (a caller error)
 */
{
   bool isComplete = mEventComplete;
   event.remove(0);

   if (isComplete)
   {
      event.append(mCurrentEvent);
      event.append(mCallInfo);
      event.append(mLaterElement);
      event.append(mViaHeader);
      event.append(mEndElement);
      event.append(CallEventElementEnd);
      event.append('\n');

      reset();
   }

   return isComplete;
}
