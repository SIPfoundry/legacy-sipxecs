//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _SipDialog_h_
#define _SipDialog_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES

#include <os/OsDefs.h>
#include <utl/UtlHashMap.h>
#include <net/Url.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define DIALOG_HANDLE_SEPARATOR ','

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipMessage;

//! Class for containing SIP dialog state information
/*! In SIP a dialog is defined by the SIP Call-Id
 *  header and the tag parameter from the SIP To
 *  and From header fields.  An early dialog has
 *  has only the tag set on one side, the transaction
 *  originator side.  In the initial transaction the
 *  the originator tag in in the From header field.
 *  The final destination sets the To header field
 *  tag in the initial transaction.
 *
 * \par Local and Remote
 *  As the To and From fields get swapped depending
 *  upon which side initiates a transaction (i.e.
 *  sends a request) local and remote are used in
 *  SipDialog to label tags, fields and information.
 *  Local and Remote are unabiquous when used in
 *  an end point.  In a proxy context the SipDialog
 *  can still be used.  One can visualize the
 *  sides of the dialog by thinking Left and Right
 *  instead of local and remote.
 *
 *  This class is intended to depricate the SipSession
 *  class.
 */
class SipDialog : public UtlString
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    enum DialogState
    {
        DIALOG_UNKNOWN,
        DIALOG_EARLY,
        DIALOG_ESTABLISHED,
        DIALOG_FAILED,
        DIALOG_TERMINATED
    };

/* ============================ CREATORS ================================== */

    //! Default Dialog constructor
    /*! Optionally construct a dialog from the given message
     *
     * \param initialMessage - message to initiate the dialog, typically this
     *        is a request.
     * \param isFromLocal - The message was sent from this side of the transaction.
     *        When the SipDialog is used in a proxy context, local and remote
     *        is not obvious.  A way to think about it in a proxy scenario is
     *        to think of local to be the left side and remote to be the right
     *        side of the transaction.
     */
    SipDialog(const SipMessage* initialMessage = NULL,
              UtlBoolean isFromLocal = TRUE);

    //! Constructor accepting the basic pieces of a session callId, toUrl, and from Url.
    /*! Optionally construct a dialog from the given message
     *
     * \param callId - sip message call-id header value
     * \param localField - sip message To or From field value representing the
     *        local side of the dialog.
     * \param remoteField - sip message To or From field value representing the
     *        remote side of the dialog.
     */
    SipDialog(const char* callId, const char* localField, const char* remoteField);

    //! Destructor
    virtual
    ~SipDialog();


/* ============================ MANIPULATORS ============================== */

    //! update the dialog information based upon the given message
    /*! Typically this updates things like the Contact, CSeq headers and
     *  tag information for the dialog.
     *  \param message - SIP message which is assumed to be part of this
     *         dialog.
     */
    void updateDialogData(const SipMessage& message);

    //! Set fields in next SIP request for this dialog
    /*! Set the request URI, call-id, To, From, Route and Cseq headers
     *  fields for the given request to be sent in the given dialog.  The
     *  last local cseq of the dialog is incremented and set in the request.
     *  \param method - the sip request method for this request.
     *  \param request - the request which is to be part of this dialog
     *         and sent as orginating from the local side of the dialog.
     */
    void setRequestData(SipMessage& request, const char* method);

/* ============================ ACCESSORS ================================= */


    //! Gets a string handle that can be used to uniquely identify this dialog
    void getHandle(UtlString& dialogHandle) const;

    //! Get the early dialog handle for this dialog
    void getEarlyHandle(UtlString& earlyDialogHandle) const;

    //! Gets the call-id, and tags from the dialogHandle
    static void parseHandle(const char* dialogHandle,
                            UtlString& callId,
                            UtlString& localTag,
                            UtlString& remoteTag);

    //! Reverse the order of the tags in the handle
    static void reverseTags(const char* dialogHandle,
                            UtlString& reversedHandle);

    //! Get the SIP call-id header value for this dialog
    void getCallId(UtlString& callId) const;
    //! Set the SIP call-id header value for this dialog
    void setCallId(const char* callId);

    //! Get the SIP To/From header value for the local side of this dialog
    void getLocalField(Url& localUrl) const;
    //! Get the tag from the SIP To/From header value for the local side of this dialog
    void getLocalTag(UtlString& localTag) const;
    //! Set the SIP To/From header value for the local side of this dialog
    void setLocalField(const Url& localUrl);

    //! Get the SIP To/From header value for the remote side of this dialog
    void getRemoteField(Url& remoteUrl) const;
    //! Get the tag from the SIP To/From header value for the remote side of this dialog
    void getRemoteTag(UtlString& remoteTag) const;
    //! Set the SIP To/From header value for the remote side of this dialog
    void setRemoteField(const Url& remoteUrl);

    //! Get the SIP Contact header value for the remote side of this dialog
    void getRemoteContact(Url& remoteContact) const;
    //! Set the SIP Contact header value for the remote side of this dialog
    void setRemoteContact(const Url& remoteContact);

    //! Get the SIP Contact header value for the local side of this dialog
    void getLocalContact(Url& localContact) const;
    //! Get the SIP Contact header value for the remote side of this dialog
    void setLocalContact(const Url& localContact);

    //! Get the SIP method of the request that initiated this dialog
    void getInitialMethod(UtlString& method) const;
    //! Set the SIP method of the request that initiated this dialog
    void setInitialMethod(const char* method);

    //! Get the next (incremented) SIP Cseq number for the local side
    int getNextLocalCseq();
    //! Get the last used SIP Cseq number for the local side
    int getLastLocalCseq() const;
    //! Set the last used SIP Cseq number for the local side
    void setLastLocalCseq(int seqNum);

    //! Get the last used SIP Cseq number for the remote side
    int getLastRemoteCseq() const;
    //! Set the last used SIP Cseq number for the remote side
    void setLastRemoteCseq(int seqNum);

    //! Get the request URI for the local side
    /*! This may be different than the local contact.  This is
     *  what was received in the last request from the remote
     *  side.
     */
    void getLocalRequestUri(UtlString& requestUri) const;
    //! Set the request URI for the local side
    void setLocalRequestUri(const UtlString& requestUri);
    //! Get the request URI for the remote side
    /*! This is typically meaningless for the remote side
     *  when observed from the local end point as it should
     *  not be different than the local contact.  However
     *  in some applications it may be possible to observe
     *  what the request URI is on the remote side or in
     *  a proxy in which case this may be interesting.
     */
    void getRemoteRequestUri(UtlString& requestUri) const;
    //! Set the request URI for the remote side
    void setRemoteRequestUri(const UtlString& requestUri);

    //int getDialogState() const;

    //! Debug method to dump the contents of this SipDialog into a string
    void toString(UtlString& dialogDumpString);

    //! Get a string representation for the state value
    static void getStateString(DialogState state,
                               UtlString& stateString);

/* ============================ INQUIRY =================================== */

    //! Compare the message to see if it matches this dialog
    /*! A dialog matches if the SIP Call-Id header and
     *  the tags from the SIP message To and From field
     *  match those of this dialog.  The tags are compared in
     * both directions.
     */
    UtlBoolean isSameDialog(const SipMessage& message) const;

    //! Compare the given dialog indentifiers match those of this dialog
    /*! The tags are compared in both directions.
     */
    UtlBoolean isSameDialog(const UtlString& callId,
                            const UtlString& localTag,
                            const UtlString& remoteTag) const;

    //! Compare the given dialog handle with that of this dialog
    /*! The tags are compared in both directions.
     */
    UtlBoolean isSameDialog(const char* dialogHandle);

    //! Determine if this is an early dialog
    UtlBoolean isEarlyDialog() const;

    //! Determine if the given handle is for an early dialog
    /*! That is check if one of the tags is null
     */
    static UtlBoolean isEarlyDialog(const char* dialogHandle);

    //! Checks if this is an early dialog for the given SIP message
    UtlBoolean isEarlyDialogFor(const SipMessage& message) const;

    //! Checks if this is an early dialog for the given SIP message
    UtlBoolean isEarlyDialogFor(const UtlString& callId,
                                const UtlString& localTag,
                                const UtlString& remoteTag) const;

    //! Checks if this was an early dialog for the given SIP message
    /*! This dialog is considered to have been an early dialog if
     *  the SIP Call-Id and one of the given tags matches one of
     *  the tags of this dialog.
     */
    UtlBoolean wasEarlyDialogFor(const UtlString& callId,
                                 const UtlString& localTag,
                                 const UtlString& remoteTag) const;

    //! Query if the transaction request was sent from the local side
    /*! If the request was sent from the local side, the fromTag will
     *  match the local tag.
     */
    UtlBoolean isTransactionLocallyInitiated(const UtlString& callId,
                                             const UtlString& fromTag,
                                             const UtlString& toTag) const;

    //! Query if the transaction request was sent from the remote side
    /*! If the request was sent from the local side, the fromTag will
     *  match the remote tag.
     */
    UtlBoolean isTransactionRemotelyInitiated(const UtlString& callId,
                                              const UtlString& fromTag,
                                              const UtlString& toTag) const;

    //! Check if message and SIP local Cseq match
    UtlBoolean isSameLocalCseq(const SipMessage& message) const;

    //! Check if message and SIP remote Cseq match
    UtlBoolean isSameRemoteCseq(const SipMessage& message) const;

    //! Check if mesage cseq is after the last local transaction
    UtlBoolean isNextLocalCseq(const SipMessage& message) const;

    //! Check if mesage cseq is after the last remote transaction
    UtlBoolean isNextRemoteCseq(const SipMessage& message) const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    //! Copy constructor
    SipDialog(const SipDialog& rSipDialog);

    //! Assignment operator
    SipDialog& operator=(const SipDialog& rhs);

    // The callId is stored in the UtlString base class data element
    Url mLocalField; // To or From depending on who initiated the transaction
    Url mRemoteField; // To or From depending on who initiated the transaction
    UtlString mLocalTag;
    UtlString mRemoteTag;
    Url mLocalContact;
    Url mRemoteContact;
    UtlString mRouteSet;
    UtlString mInitialMethod;
    UtlString msLocalRequestUri;
    UtlString msRemoteRequestUri;
    UtlBoolean mLocalInitiatedDialog;
    int mInitialLocalCseq;
    int mInitialRemoteCseq;
    int mLastLocalCseq;
    int mLastRemoteCseq;
    int mDialogState;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipDialog_h_
