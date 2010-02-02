//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _SipDialogEvent_h_
#define _SipDialogEvent_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlSList.h>
#include <utl/UtlSListIterator.h>
#include <utl/UtlDListIterator.h>
#include <net/HttpBody.h>
#include <net/NameValuePairInsensitive.h>
#include <net/Url.h>
#include <os/OsDateTime.h>
#include <os/OsBSem.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

#define DIALOG_EVENT_TYPE "dialog"
#define DIALOG_SLA_EVENT_TYPE "dialog;sla"
#define DIALOG_EVENT_CONTENT_TYPE "application/dialog-info+xml"

#define BEGIN_DIALOG_INFO "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\""
#define END_DIALOG_INFO "</dialog-info>\n"

#define VERSION_EQUAL " version="
#define STATE_EQUAL " state="
#define ENTITY_EQUAL " entity="

#define DOUBLE_QUOTE "\""
#define END_BRACKET ">"
#define END_LINE ">\n"

#define BEGIN_DIALOG "<dialog id="
#define CALL_ID_EQUAL " call-id="
#define LOCAL_TAG_EQUAL " local-tag="
#define REMOTE_TAG_EQUAL " remote-tag="
#define DIRECTION_EQUAL " direction="
#define END_DIALOG "</dialog>\n"

#define BEGIN_STATE "<state"
#define EVENT_EQUAL " event="
#define CODE_EQUAL " code="
#define END_STATE "</state>\n"

#define BEGIN_DURATION "<duration>"
#define END_DURATION "</duration>\n"

#define BEGIN_LOCAL "<local>\n"
#define END_LOCAL "</local>\n"

#define BEGIN_REMOTE "<remote>\n"
#define END_REMOTE "</remote>\n"

#define BEGIN_IDENTITY "<identity"
#define DISPLAY_EQUAL " display="
#define END_IDENTITY "</identity>\n"

#define BEGIN_TARGET "<target uri=\""
#define END_TARGET "</target>\n"

#define BEGIN_DIALOG_PARAM "<param "
#define PNAME "pname=\""
// Note that draft-ietf-bliss-shared-appearances uses pval,
// while draft-anil-sipping-bla-0x use pvalue.
// Polycom sets have a setting in sip.cfg "voIpProt.SIP.dialog.usePvalue" (default 0, i.e. pval)
// More importantly, RFC 4235, which defines dialog events, specifies "pval" in section 4.1.6.2.
#define PVALUE "\" pval=\""
#define END_DIALOG_PARAM "\"/>\n"

// dialog-info states
#define STATE_FULL "full"
#define STATE_PARTIAL "partial"

// dialog states
#define STATE_TRYING "trying"
#define STATE_PROCEEDING "proceeding"
#define STATE_EARLY "early"
#define STATE_CONFIRMED "confirmed"
#define STATE_TERMINATED "terminated"

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//! Container for dialog element in the dialog event package
/**
 * This class contains all the contents presented in a dialog element of the
 * dialog event package described in draft-ietf-sipping-dialog-package-06.txt
 * (An INVITE Initiated Dialog Event Package for SIP). This class has the
 * methods to construct and manipulate the dialog and its sub-elements.
 */

class Dialog : public UtlContainable
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/**
 * @name ====================== Constructors and Destructors
 * @{
 */
   /// Constructor
   Dialog(const char* dialogId,
          const char* callId,
          const char* localTag,
          const char* remoteTag,
          const char* direction);

   // copy constructor
   Dialog(const Dialog& rDialog);

   /// Destructor
   ~Dialog();

   virtual UtlContainableType getContainableType() const;

   static const UtlContainableType TYPE;

   virtual unsigned int hash() const;

   int compareTo(const UtlContainable *b) const;

///@}

/**
 * @name ====================== Dialog Setting Interfaces
 *
 * These methods set/get the dialog element and sub-elements.
 *
 * @{
 */

   /// Render the XML for the dialog into the provided UtlString.
   void getBytes(UtlString& b, ssize_t& l);

   void getDialog(UtlString& dialogId,
                  UtlString& callId,
                  UtlString& localTag,
                  UtlString& remoteTag,
                  UtlString& direction) const;

   void getCallId(UtlString& callId) const;

   void setDialogId(const char* dialogId);

   void getDialogId(UtlString& dialogId) const;

   void setState(const char* state, const char* event, const char* code);

   void setTags(const char* local, const char* remote);

   void getState(UtlString& state, UtlString& event, UtlString& code) const;

   // start time of the dialog, or 0 if unknown
   void setDuration(const unsigned long duration);

   unsigned long getDuration() const;

   void setReplaces(const char* callId,
                    const char* localTag,
                    const char* remoteTag);

   void getReplaces(UtlString& callId,
                    UtlString& localTag,
                    UtlString& remoteTag) const;

   void setReferredBy(const char* url,
                      const char* display);

   void getReferredBy(UtlString& url,
                      UtlString& display) const;

   void setLocalIdentity(const char* identity,
                         const char* display);

   void getLocalIdentity(UtlString& identity,
                         UtlString& display) const;

   void setRemoteIdentity(const char* identity,
                          const char* display);

   void getRemoteIdentity(UtlString& identity,
                          UtlString& display) const;

   void setLocalTarget(const char* url);

   void getLocalTarget(UtlString& url) const;

   void setRemoteTarget(const char* url);

   void getRemoteTarget(UtlString& url) const;

   void addLocalParameter(NameValuePairInsensitive* nvp);

   //! Return an iterator that will retrieve all local parameters in the event.
    // This iterator is only valid as long as the SipDialogEvent is not
    // modified, and must be deleted by the caller before the SipDialogEvent
    // is deleted.
   UtlDListIterator* getLocalParameterIterator();

   bool setLocalParameter(const char* pname, const UtlString& pvalue);

   bool getLocalParameter(const char* pname, UtlString& pvalue);

   void addRemoteParameter(NameValuePairInsensitive* nvp);

   //! Return an iterator that will retrieve all remote parameters in the event.
    // This iterator is only valid as long as the SipDialogEvent is not
    // modified, and must be deleted by the caller before the SipDialogEvent
    // is deleted.
   UtlDListIterator* getRemoteParameterIterator();

   bool setRemoteParameter(const char* pname, const UtlString& pvalue);

   bool getRemoteParameter(const char* pname, UtlString& pvalue);
///@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

   // Set the unique identifier member by concatenating the call-id,
   // to-tag, and from-tag.
   void setIdentifier();

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   // Variables for dialog element
   UtlString mId;
   UtlString mCallId;
   UtlString mLocalTag;
   UtlString mRemoteTag;
   UtlString mDirection;
   // Unique identifier of the dialog
   UtlString mIdentifier;

   // Variables for state element
   UtlString mState;
   UtlString mEvent;
   UtlString mCode;

   // Variables for duration element
   long mDuration;              // start time of the dialog, or 0 if unknown

   // Variables for replaces element
   UtlString mNewCallId;
   UtlString mNewLocalTag;
   UtlString mNewRemoteTag;

   // Variables for referred-by element
   UtlString mReferredBy;
   UtlString mDisplay;

   // Variables for local element
   UtlString mLocalIdentity;
   UtlString mLocalDisplay;
   UtlString mLocalTarget;
   UtlString mLocalSessionDescription;
   UtlDList  mLocalParameters;
   UtlDList  mRemoteParameters;

   // Variables for remote element
   UtlString mRemoteIdentity;
   UtlString mRemoteDisplay;
   UtlString mRemoteTarget;
   UtlString mRemoteSessionDescription;

   // Disabled assignment operator
   Dialog& operator=(const Dialog& rhs);
};


//! Container for MIME type application/dialog-info+xml.
/**
 * This class contains all the contents presented in a dialog event package
 * described in draft-ietf-sipping-dialog-package-06.txt (An INVITE Initiated
 * Dialog Event Package for SIP). This class has the methods to construct and
 * manipulate the dialog events in a dialog event package.
 */
class SipDialogEvent : public HttpBody
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/**
 * @name ====================== Constructors and Destructors
 * @{
 */
   //! Construct an empty body of a dialog event package
   SipDialogEvent(const char* state,
                  const char* entity);

   //! Construct from an existing dialog event package in XML format
   SipDialogEvent(const char* bodyBytes);

   //! copy constructor
   SipDialogEvent(const SipDialogEvent& rSipDialogEvent);

   virtual SipDialogEvent* copy() const;

   //! Destructor that will free up the memory allocated for dialog contents if it is not being deleted
   virtual
      ~SipDialogEvent();

///@}

/**
 * @name ====================== Dialog Event Serialization Interfaces
 *
 * @{
 */

   //! Build the body of this object
   void buildBody(int* version = NULL) const;
   /**< If version is non-NULL, then the body text will be built with
    *   the recorded version number (mVersion), and that number will
    *   be returned to the caller in *version.
    *   If it is NULL, the body text will be built with the substitution
    *   placeholder, '&version;'.
    */

   //! Get the string length of this object
   //  Calls buildBody() (with a temporary argument).
   virtual ssize_t buildBodyGetLength() const;

   //! Get the serialized char representation of this dialog event.
   /*! Note that buildBody() must first be called to construct the character
    *  representation.
    *  \param bytes - pointer to the body text of the dialog event will
    *       be placed here.
    *  \param length - the number of bytes written (not including the
    *       null terminator).
    */
   virtual void getBytes(const char** bytes,
                         ssize_t* length) const;

   //! Get the serialized string representation of this dialog event.
   /*! Note that buildBody() must first be called to construct the character
    *  representation.
    *  \param bytes - UtlString into which the body text will be copied.
    *  \param length - the number of bytes written (not including the
    *       null terminator).
    */
   virtual void getBytes(UtlString* bytes,
                         ssize_t* length) const;

   //! Construct and get serialized char representation of this dialog event.
   //  Calls buildBody() (with a temporary argument) and then getBytes().
   virtual void buildBodyGetBytes(UtlString* bytes,
                                  ssize_t* length) const;

   void setEntity(const char* entity);

   void getEntity(UtlString& entity) const;

   void setState(const char* state);

   void getState(UtlString& state) const;

   // Get the version number from the dialog event body.
   int getVersion() const;

///@}

/**
 * @name ====================== Dialog Setting Interfaces
 *
 * These methods set/get the dialog element.
 *
 * @{
 */

   //! Insert a Dialog object.  It will be owned by this SipDialogEvent.
   void insertDialog(Dialog* dialog);

   //! Get the Dialog object from the hash table based on the callId
   //and tags.  If the mRemoteTag of a Dialog object in the hash table
   //is empty, then testing for match is only done on callId and
   //localTag.  Otherwise, all three fields are used.
   Dialog* getDialog(UtlString& callId,
                     UtlString& localTag,
                     UtlString& remoteTag);

   //! In the case where a empty SipDialog object is retrieved from the
   //DialogEventPublisher in handling a DISCONNECTED or FAILED message
   //the publisher still needs to find the dialog, even if it is just
   //by the callId. Work-around for XCL-98.
   Dialog* getDialogByCallId(UtlString& callId);

   //! Get the Dialog object from the hash table based on the dialogId.
   Dialog* getDialogByDialogId(UtlString& dialogId);

   //! Remove a Dialog object
   Dialog* removeDialog(Dialog* dialog);

   //! Check whether there is are any dialogs or not
   UtlBoolean isEmpty();

   //! Return an iterator that will retrieve all dialogs in the event.
   // This iterator is only valid as long as the SipDialogEvent is not
   // modified, and must be deleted by the caller before the SipDialogEvent
   // is deleted.
   UtlSListIterator* getDialogIterator();

///@}

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

   /// Parse an existing dialog event package from xml format into the internal representation.
   void parseBody(const char* bytes);


/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   //! Variables for dialog-info
   int mVersion;
   UtlString mDialogState;
   UtlString mEntity;

   //! Variables for dialog element
   UtlSList mDialogs;

   //! reader/writer lock for synchronization
   OsBSem mLock;

   //! Disabled assignment operator
   SipDialogEvent& operator=(const SipDialogEvent& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipDialogEvent_h_
