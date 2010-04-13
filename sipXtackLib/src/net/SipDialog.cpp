//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <net/SipDialog.h>
#include <net/SipMessage.h>
#include <utl/UtlHashMapIterator.h>
#include <os/OsSysLog.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipDialog::SipDialog(const SipMessage* initialMessage,
                     UtlBoolean isFromLocal)
{
   if(initialMessage)
   {
       UtlString callId;
       initialMessage->getCallIdField(&callId);
       append(callId);

       // The transaction was initiated from this side
       if((!initialMessage->isResponse() &&
           isFromLocal) ||
           (initialMessage->isResponse() &&
           !isFromLocal))
       {
           mLocalInitiatedDialog = TRUE;
           initialMessage->getFromUrl(mLocalField);
           mLocalField.getFieldParameter("tag", mLocalTag);
           initialMessage->getToUrl(mRemoteField);
           mRemoteField.getFieldParameter("tag", mRemoteTag);
           initialMessage->getCSeqField(&mInitialLocalCseq, &mInitialMethod);
           mLastLocalCseq = mInitialLocalCseq;
           mLastRemoteCseq = -1;
           mInitialRemoteCseq = -1;
       }
       // The transaction was initiated from the other side
       else
       {
           mLocalInitiatedDialog = FALSE;
           initialMessage->getFromUrl(mRemoteField);
           mRemoteField.getFieldParameter("tag", mRemoteTag);
           initialMessage->getToUrl(mLocalField);
           mLocalField.getFieldParameter("tag", mLocalTag);
           initialMessage->getCSeqField(&mInitialRemoteCseq, &mInitialMethod);
           mLastRemoteCseq = mInitialRemoteCseq;
           // Start local CSeq's at 1, because some UAs cannot handle 0.
           mLastLocalCseq = 0;
           mInitialLocalCseq = 0;
       }

       if(!initialMessage->isResponse())
       {
           UtlString uri;
           initialMessage->getRequestUri(&uri);
           if(isFromLocal)
           {
               msRemoteRequestUri = uri;
           }
           else
           {
               // Incoming initial Request, we need to set the Route set here
               if(initialMessage->isRecordRouteAccepted())
               {
                   initialMessage->buildRouteField(&mRouteSet);
               }
               msLocalRequestUri = uri;
           }
       }

       // :TODO: this should use a new interface to get a parsed value
       UtlString contact;
       // Get the Contact, but as an addr-spec.
       initialMessage->getContactUri(0, &contact);
       // If the message has been freshly composed but not yet sent, it
       // may not have a Contact value (since most application code allows
       // SipUserAgent to set the Contact).  In that case, do not attempt
       // to parse the null Contact value.  (The m*Contact member should be
       // updated later.)
       if (!contact.isNull())
       {
          if (isFromLocal)
          {
             mLocalContact.fromString(contact, TRUE);
          }
          else
          {
             mRemoteContact.fromString(contact, TRUE);
          }
       }
   }
   else
   {
       // Insert dummy values into fields that aren't automatically initialized.
       mLocalInitiatedDialog = FALSE;
       // Start local CSeq's at 1, because some UAs cannot handle 0.
       mLastLocalCseq = 0;
       mLastRemoteCseq = -1;
       mInitialLocalCseq = 0;
       mInitialRemoteCseq = -1;
   }

   mDialogState = DIALOG_UNKNOWN;
}

// Constructor
SipDialog::SipDialog(const char* callId,
                     const char* localField,
                     const char* remoteField)
    : UtlString(callId)
{
    mRemoteField = Url(remoteField);
    mRemoteField.getFieldParameter("tag", mRemoteTag);
    mLocalField = Url(localField);
    mLocalField.getFieldParameter("tag", mLocalTag);

    // Start local CSeq's at 1, because some UAs cannot handle 0.
    mInitialLocalCseq = 0;
    mInitialRemoteCseq = -1;
    mLastLocalCseq = 0;
    mLastRemoteCseq = -1;
    mDialogState = DIALOG_UNKNOWN;
}

// Copy constructor
SipDialog::SipDialog(const SipDialog& rSipDialog)
  : UtlString(rSipDialog)
{
   mLocalField = rSipDialog.mLocalField;
   mLocalTag = rSipDialog.mLocalTag;
   mRemoteField = rSipDialog.mRemoteField;
   mRemoteTag = rSipDialog.mRemoteTag;
   mLocalContact = rSipDialog.mLocalContact;
   mRemoteContact = rSipDialog.mRemoteContact;
   mRouteSet = rSipDialog.mRouteSet;
   mInitialMethod = rSipDialog.mInitialMethod;
   mLocalInitiatedDialog = rSipDialog.mLocalInitiatedDialog;
   mInitialLocalCseq = rSipDialog.mInitialLocalCseq;
   mInitialRemoteCseq = rSipDialog.mInitialRemoteCseq;
   mLastLocalCseq = rSipDialog.mLastLocalCseq;
   mLastRemoteCseq = rSipDialog.mLastRemoteCseq;
   mDialogState = rSipDialog.mDialogState;
   msLocalRequestUri = rSipDialog.msLocalRequestUri;
   msRemoteRequestUri = rSipDialog.msRemoteRequestUri;
}


// Destructor
SipDialog::~SipDialog()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipDialog&
SipDialog::operator=(const SipDialog& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   UtlString::operator=(rhs);  // assign fields for parent class


   mLocalField = rhs.mLocalField;
   mLocalTag = rhs.mLocalTag;
   mRemoteField = rhs.mRemoteField;
   mRemoteTag = rhs.mRemoteTag;
   mLocalContact = rhs.mLocalContact;
   mRemoteContact = rhs.mRemoteContact;
   mRouteSet = rhs.mRouteSet;
   mInitialMethod = rhs.mInitialMethod;
   mLocalInitiatedDialog = rhs.mLocalInitiatedDialog;
   mInitialLocalCseq = rhs.mInitialLocalCseq;
   mInitialRemoteCseq = rhs.mInitialRemoteCseq;
   mLastLocalCseq = rhs.mLastLocalCseq;
   mLastRemoteCseq = rhs.mLastRemoteCseq;
   mDialogState = rhs.mDialogState;
   msLocalRequestUri = rhs.msLocalRequestUri;
   msRemoteRequestUri = rhs.msRemoteRequestUri;
   // Do not copy mApplicationData

   return *this;
}


void SipDialog::updateDialogData(const SipMessage& message)
{
    UtlString messageCallId;
    message.getCallIdField(&messageCallId);
    Url messageFromUrl;
    message.getFromUrl(messageFromUrl);
    UtlString messageFromTag;
    messageFromUrl.getFieldParameter("tag", messageFromTag);
    Url messageToUrl;
    message.getToUrl(messageToUrl);
    UtlString messageToTag;
    messageToUrl.getFieldParameter("tag", messageToTag);

    int cSeq;
    UtlString method;
    message.getCSeqField(&cSeq, &method);
    int responseCode = message.getResponseStatusCode();

    // Figure out if the request is from the local or
    // the remote side
    if(isTransactionLocallyInitiated(messageCallId,
                                     messageFromTag,
                                     messageToTag))
    {
        // This message is part of a transaction initiated by
        // the local side of the dialog

        if(cSeq > mLastLocalCseq)
        {
            mLastLocalCseq = cSeq;
        }

        if(cSeq >= mLastLocalCseq)
        {
            // Always update the contact if it is set
            UtlString messageContact;
            // Get the Contact value, but as an addr-spec.
            if(message.getContactUri(0, &messageContact) &&
                !messageContact.isNull())
            {
                if(message.isResponse())
                {
                   mRemoteContact.fromString(messageContact, TRUE);
                }
                else
                {
                   mLocalContact.fromString(messageContact, TRUE);
                }
            }
        }

        // Cannot assume that we only establish a dialog with the
        // initial cseq.  For example if there is an authentication
        // challenge, the dialog will not be established until the
        // second transaction.
        if(cSeq == mLastLocalCseq)
        {
            // A successful response to an INVITE or SUBSCRIBE
            // make this early dialog a set up dialog
            if(mLocalInitiatedDialog &&
               message.isResponse() &&
               responseCode >= SIP_2XX_CLASS_CODE && // successful dialog setup
               responseCode < SIP_3XX_CLASS_CODE &&
               mRemoteTag.isNull() && // tag not set
               mRouteSet.isNull()) // have not yet set the route set
            {
                // Change this early dialog to a set up dialog.
                // The tag gets set in the 2xx response
                // so we need to update the URL
                message.getToUrl(mRemoteField);
                mRemoteField.getFieldParameter("tag", mRemoteTag);

                // Need to get the route set as well
                // Make sure the Request Method is allowed to set Record-Routes
                if(message.isRecordRouteAccepted())
                {
                    message.buildRouteField(&mRouteSet);
                }
            }
        }
    }
    else if(isTransactionRemotelyInitiated(messageCallId,
                                           messageFromTag,
                                           messageToTag))
    {
        int prevRemoteCseq = mLastRemoteCseq;

        // This message is part of a transaction initiated by
        // the callee/destination of the session
        if(cSeq > mLastRemoteCseq)
        {
            mLastRemoteCseq = cSeq;
        }

        if(cSeq >= mLastRemoteCseq)
        {
            // Always update the contact if it is set
            UtlString messageContact;
            // Get the Contact value, but as an addr-spec.
            if(message.getContactUri(0, &messageContact) &&
                !messageContact.isNull())
            {
                if(message.isResponse())
                {
                   mLocalContact.fromString(messageContact, TRUE);
                }
                else
                {
                   mRemoteContact.fromString(messageContact, TRUE);
                }
            }
        }

        // First transaction from the otherside
        if(cSeq == mLastRemoteCseq && prevRemoteCseq == -1)
        {
            // A response (e.g. NOTIFY) can come before we get the
            // successful response to the initial transaction
            if(!mLocalInitiatedDialog &&
               !message.isResponse() &&
               mRemoteTag.isNull()) // tag not set
            {
                // Change this early dialog to a set up dialog.
                // The tag gets set in the 2xx response
                // so we need to update the URL
                message.getFromUrl(mRemoteField);
                mRemoteField.getFieldParameter("tag", mRemoteTag);
            }
        }

        // First successful response from the local side
        if(cSeq == mLastRemoteCseq)
        {
            if(!mLocalInitiatedDialog &&
               message.isResponse() &&
               responseCode >= SIP_2XX_CLASS_CODE && // successful dialog setup
               responseCode < SIP_3XX_CLASS_CODE &&
               mLocalTag.isNull())
            {
                // Update the local tag
                message.getToUrl(mLocalField);
                mLocalField.getFieldParameter("tag", mLocalTag);
            }
        }
    }
}

void SipDialog::setRequestData(SipMessage& request, const char* method)
{
    UtlString methodString(method ? method : "");
    if(methodString.isNull())
    {
        request.getRequestMethod(&methodString);
    }

    // The request URI should be the remote contact
    UtlString remoteContact;
    // Use getUri() to get the contact in addr-spec format.
    // (mRemoteContact should have no field parameters, but if it has
    // URI parameters, toString would add <...>, which are not allowed
    // in URIs.)
    mRemoteContact.getUri(remoteContact);

    // If the remote contact is empty, use the remote request uri
    if (remoteContact.compareTo("sip:") == 0)
    {
         OsSysLog::add(FAC_ACD, PRI_DEBUG, "SipDialog::setRequestData - using remote request uri %s",
                       msRemoteRequestUri.data());
         request.setSipRequestFirstHeaderLine(methodString, msRemoteRequestUri);
    }
    else
    {
         request.setSipRequestFirstHeaderLine(methodString, remoteContact);
    }

    // The local field is the From field
    UtlString fromField;
    mLocalField.toString(fromField);
    request.setRawFromField(fromField);

    // The remote field is the To field
    UtlString toField;
    mRemoteField.toString(toField);
    request.setRawToField(toField);

    // Get the next local Cseq, the method should already be set
    getNextLocalCseq();
    request.setCSeqField(mLastLocalCseq, methodString);

    // Set the route header according to the route set
    if(!mRouteSet.isNull())
    {
        request.setRouteField(mRouteSet);
    }

    // Set the call-id
    request.setCallIdField(*this);
}

/* ============================ ACCESSORS ================================= */

void SipDialog::getHandle(UtlString& dialogHandle) const
{
    dialogHandle = *this; // callId
    dialogHandle.append(DIALOG_HANDLE_SEPARATOR);
    dialogHandle.append(mLocalTag);
    dialogHandle.append(DIALOG_HANDLE_SEPARATOR);
    dialogHandle.append(mRemoteTag);
}

void SipDialog::getEarlyHandle(UtlString& earlyDialogHandle) const
{
    // Do not add the tag for the side that did not initiate the dialog
    earlyDialogHandle = *this; // callId
    earlyDialogHandle.append(DIALOG_HANDLE_SEPARATOR);
    if(mLocalInitiatedDialog)
    {
        earlyDialogHandle.append(mLocalTag);
    }
    earlyDialogHandle.append(DIALOG_HANDLE_SEPARATOR);
    if(!mLocalInitiatedDialog)
    {
        earlyDialogHandle.append(mRemoteTag);
    }
}

void SipDialog::parseHandle(const char* dialogHandle,
                            UtlString& callId,
                            UtlString& localTag,
                            UtlString& remoteTag)
{
    callId="";
    localTag = "";
    remoteTag = "";

    // The call-id ends at the first comma
    const char* callIdEnd = strchr(dialogHandle,DIALOG_HANDLE_SEPARATOR);
    if(callIdEnd)
    {
        // Move past the first comma
        const char* localTagBegin = callIdEnd + 1;

        // Copy the call id
        callId.append(dialogHandle, callIdEnd - dialogHandle);

        // The local tag ends at the second comma
        const char* localTagEnd = strchr(localTagBegin, DIALOG_HANDLE_SEPARATOR);
        if(localTagEnd)
        {
            // Copy the local tag
            localTag.append(localTagBegin, localTagEnd - localTagBegin);

            // The remote tag begins beyond the comma
            const char* remoteTagBegin = localTagEnd + 1;

            // Copy the remote tag
            remoteTag.append(remoteTagBegin);
        }
    }
}

void SipDialog::reverseTags(const char* dialogHandle,
                            UtlString& reversedHandle)
{
    UtlString tag1;
    UtlString tag2;
    parseHandle(dialogHandle, reversedHandle, tag1, tag2);
    reversedHandle.capacity(strlen(dialogHandle) + 2);
    reversedHandle.append(DIALOG_HANDLE_SEPARATOR);
    reversedHandle.append(tag2);
    reversedHandle.append(DIALOG_HANDLE_SEPARATOR);
    reversedHandle.append(tag1);
}

void SipDialog::getCallId(UtlString& callId) const
{
    callId = *this;
}

void SipDialog::setCallId(const char* callId)
{
    remove(0);
    append(callId ? callId : "");
}

void SipDialog::getLocalField(Url& localField) const
{
    localField = mLocalField;
}

void SipDialog::getLocalTag(UtlString& localTag) const
{
    localTag = mLocalTag;
}

void SipDialog::setLocalField(const Url& localField)
{
    mLocalField = localField;
}

void SipDialog::getRemoteField(Url& remoteField) const
{
    remoteField = mRemoteField;
}

void SipDialog::getRemoteTag(UtlString& remoteTag) const
{
    remoteTag = mRemoteTag;
}

void SipDialog::setRemoteField(const Url& remoteField)
{
    mRemoteField = remoteField;
}

void SipDialog::getRemoteContact(Url& remoteContact) const
{
    remoteContact = mRemoteContact;
}

void SipDialog::setRemoteContact(const Url& remoteContact)
{
    mRemoteContact = remoteContact;
}

void SipDialog::getLocalContact(Url& localContact) const
{
    localContact = mLocalContact;
}

void SipDialog::setLocalContact(const Url& localContact)
{
    mLocalContact = localContact;
}

void SipDialog::getLocalRequestUri(UtlString& requestUri) const
{
   requestUri = msLocalRequestUri;
}

void SipDialog::setLocalRequestUri(const UtlString& requestUri)
{
   msLocalRequestUri = requestUri;
}

void SipDialog::getRemoteRequestUri(UtlString& requestUri) const
{
   requestUri = msRemoteRequestUri;
}

void SipDialog::setRemoteRequestUri(const UtlString& requestUri)
{
   msRemoteRequestUri = requestUri;
}


void SipDialog::getInitialMethod(UtlString& method) const
{
    method = mInitialMethod;
}

void SipDialog::setInitialMethod(const char* method)
{
    mInitialMethod = method;
}

int SipDialog::getLastLocalCseq() const
{
    return(mLastLocalCseq);
}

void SipDialog::setLastLocalCseq(int lastLocalCseq)
{
    mLastLocalCseq = lastLocalCseq;
}

int SipDialog::getLastRemoteCseq() const
{
    return(mLastRemoteCseq);
}

void SipDialog::setLastRemoteCseq(int lastRemoteCseq)
{
    mLastRemoteCseq = lastRemoteCseq;
}

int SipDialog::getNextLocalCseq()
{
    mLastLocalCseq++;
    return(mLastLocalCseq);
}

//int SipDialog::getDialogState() const
//{
//    return mDialogState;
//}

/* ============================ INQUIRY =================================== */


UtlBoolean SipDialog::isSameDialog(const SipMessage& message) const
{
    UtlString messageCallId;
    message.getCallIdField(&messageCallId);
    UtlBoolean isSameDialog = FALSE;
    if(messageCallId.compareTo(*this, UtlString::ignoreCase) == 0)
    {
        Url messageFromUrl;
        message.getFromUrl(messageFromUrl);
        UtlString messageFromTag;
        messageFromUrl.getFieldParameter("tag", messageFromTag);
        if(messageFromTag.compareTo(mLocalTag, UtlString::ignoreCase) == 0)
        {
            Url messageToUrl;
            message.getToUrl(messageToUrl);
            UtlString messageToTag;
            messageToUrl.getFieldParameter("tag", messageToTag);
            if(messageToTag.compareTo(mRemoteTag, UtlString::ignoreCase) == 0)
            {
                isSameDialog = TRUE;
            }
        }
        else if(messageFromTag.compareTo(mRemoteTag, UtlString::ignoreCase) == 0)
        {
            Url messageToUrl;
            message.getToUrl(messageToUrl);
            UtlString messageToTag;
            messageToUrl.getFieldParameter("tag", messageToTag);
            if(messageToTag.compareTo(mLocalTag, UtlString::ignoreCase) == 0)
            {
                isSameDialog = TRUE;
            }
        }

    }
    return(isSameDialog);

}

UtlBoolean SipDialog::isSameDialog(const UtlString& callId,
                                   const UtlString& localTag,
                                   const UtlString& remoteTag) const
{
    // Literal/exact match of tags only
    // i.e. do not allow a null tag to match a set tag
    UtlBoolean isSameDialog = FALSE;

    if(callId.compareTo(*this, UtlString::ignoreCase) == 0)
    {
       if(localTag.compareTo(mLocalTag, UtlString::ignoreCase) == 0 &&
          remoteTag.compareTo(mRemoteTag, UtlString::ignoreCase) == 0)
       {
           isSameDialog = TRUE;
       }

       else if(remoteTag.compareTo(mLocalTag, UtlString::ignoreCase) == 0 &&
           localTag.compareTo(mRemoteTag, UtlString::ignoreCase) == 0)
       {
           isSameDialog = TRUE;
       }
    }

    return(isSameDialog);
}

UtlBoolean SipDialog::isSameDialog(const char* dialogHandle)
{
    UtlString callId;
    UtlString localTag;
    UtlString remoteTag;
    parseHandle(dialogHandle, callId, localTag, remoteTag);
    return(isSameDialog(callId, localTag, remoteTag));
}

UtlBoolean SipDialog::isEarlyDialogFor(const SipMessage& message) const
{
    UtlString handle;

    message.getDialogHandle(handle);

    UtlString callId;
    UtlString localTag;
    UtlString remoteTag;
    parseHandle(handle, callId, localTag, remoteTag);

    return(isEarlyDialogFor(callId, localTag, remoteTag));
}

UtlBoolean SipDialog::isEarlyDialogFor(const UtlString& callId,
                                       const UtlString& localTag,
                                       const UtlString& remoteTag) const
{
    UtlBoolean isSameEarlyDialog = FALSE;

    // If the local tag is NULL the remote tag must match one of the
    // two given tags, to be an early dialog for the given dialog info
    if(mLocalTag.isNull())
    {
        if(localTag.compareTo(mRemoteTag, UtlString::ignoreCase) == 0 ||
           remoteTag.compareTo(mRemoteTag, UtlString::ignoreCase) == 0)
        {
            isSameEarlyDialog = TRUE;
        }
    }

    // If the remote tag is NULL the local tag must match one of the
    // two given tags, to be an early dialog for the given dialog info
    else if(mRemoteTag.isNull())
    {
        if(localTag.compareTo(mLocalTag, UtlString::ignoreCase) == 0 ||
           remoteTag.compareTo(mLocalTag, UtlString::ignoreCase) == 0)
        {
            isSameEarlyDialog = TRUE;
        }
    }

    return(isSameEarlyDialog);
}

UtlBoolean SipDialog::wasEarlyDialogFor(const UtlString& callId,
                                        const UtlString& localTag,
                                        const UtlString& remoteTag) const
{
    UtlBoolean wasSameEarlyDialog = FALSE;

    // Assume that if any either of the given tags matches
    // one of the dialog's tags that they shared the same
    // early dialog
    if(localTag.compareTo(mRemoteTag, UtlString::ignoreCase) == 0 ||
       remoteTag.compareTo(mRemoteTag, UtlString::ignoreCase) == 0 ||
       localTag.compareTo(mLocalTag, UtlString::ignoreCase) == 0 ||
       remoteTag.compareTo(mLocalTag, UtlString::ignoreCase) == 0)
    {
        wasSameEarlyDialog = TRUE;
    }

    return(wasSameEarlyDialog);
}

UtlBoolean SipDialog::isTransactionLocallyInitiated(const UtlString& callId,
                                                    const UtlString& fromTag,
                                                    const UtlString& toTag) const
{
    UtlBoolean isLocalDialog = FALSE;
    if(callId.compareTo(*this, UtlString::ignoreCase) == 0)
    {
        if(fromTag.compareTo(mLocalTag, UtlString::ignoreCase) == 0 &&
           (toTag.compareTo(mRemoteTag, UtlString::ignoreCase) == 0 ||
            toTag.isNull() || mRemoteTag.isNull()))
        {
            isLocalDialog = TRUE;
        }
    }

    return(isLocalDialog);
}

UtlBoolean SipDialog::isTransactionRemotelyInitiated(const UtlString& callId,
                                                     const UtlString& fromTag,
                                                     const UtlString& toTag) const
{
    UtlBoolean isRemoteDialog = FALSE;
    if(callId.compareTo(*this, UtlString::ignoreCase) == 0)
    {
        if(((toTag.compareTo(mLocalTag, UtlString::ignoreCase) == 0 ||
            toTag.isNull() || mLocalTag.isNull())) &&
           (fromTag.compareTo(mRemoteTag, UtlString::ignoreCase) == 0 ||
           mRemoteTag.isNull())) // If this is remotely initiated fromTag
           // cannot be a null string.  mRemoteTag can be a null string
           // as occurs when a remotely initiated NOTIFY is received
           // before the SUBSCRIBE response is received.
        {
            isRemoteDialog = TRUE;
        }
    }

    return(isRemoteDialog);
}

UtlBoolean SipDialog::isEarlyDialog() const
{
    // For now make the simple assumption that if one of
    // the tags is not set that this is an early dialog
    // Note: RFC 2543 clients only needed to optionally
    // set the tags.  I do not think we need to support
    // RFC 2543 in this class.
    UtlBoolean tagNotSet = FALSE;
    if(mLocalTag.isNull() || mRemoteTag.isNull())
    {
        tagNotSet = TRUE;
    }

    return(tagNotSet);
}

UtlBoolean SipDialog::isEarlyDialog(const char* handle)
{
    // For now make the simple assumption that if one of
    // the tags is not set that this is an early dialog
    // Note: RFC 2543 clients only needed to optionally
    // set the tags.  I do not think we need to support
    // RFC 2543 in this class.
    UtlBoolean tagNotSet = FALSE;
    if(handle && *handle)
    {
        UtlString dialogHandle(handle);
        UtlString callId;
        UtlString localTag;
        UtlString remoteTag;
        parseHandle(dialogHandle, callId, localTag, remoteTag);
        if(localTag.isNull() || remoteTag.isNull())
        {
            tagNotSet = TRUE;
        }
    }
    return(tagNotSet);

}

UtlBoolean SipDialog::isSameLocalCseq(const SipMessage& message) const
{
    int cseq;
    message.getCSeqField(&cseq, NULL);

    return(cseq == mLastLocalCseq);
}

UtlBoolean SipDialog::isSameRemoteCseq(const SipMessage& message) const
{
    int cseq;
    message.getCSeqField(&cseq, NULL);

    return(cseq == mLastRemoteCseq);
}

UtlBoolean SipDialog::isNextLocalCseq(const SipMessage& message) const
{
    int cseq;
    message.getCSeqField(&cseq, NULL);

    return(cseq > mLastLocalCseq);
}

UtlBoolean SipDialog::isNextRemoteCseq(const SipMessage& message) const
{
    int cseq;
    message.getCSeqField(&cseq, NULL);

    return(cseq > mLastRemoteCseq);
}

void SipDialog::toString(UtlString& dialogDumpString)
{
    // Serialize all the members into the dumpString
    char numberString[20];
    dialogDumpString="SipDialog: ";
    sprintf(numberString, "%p", this);
    dialogDumpString.append(numberString);
    dialogDumpString.append("\nCall-Id:");
    // The callId is stored in the UtlString base class data element
    dialogDumpString.append(*this);
    dialogDumpString.append("\nmLocalField:");
    UtlString tmpString;
    mLocalField.toString(tmpString);
    dialogDumpString.append(tmpString);
    dialogDumpString.append("\nmRemoteField:");
    mRemoteField.toString(tmpString);
    dialogDumpString.append(tmpString);
    dialogDumpString.append("\nmLocalTag:");
    dialogDumpString.append(mLocalTag);
    dialogDumpString.append("\nmRemoteTag:");
    dialogDumpString.append(mRemoteTag);
    dialogDumpString.append("\nmLocalContact:");
    mLocalContact.toString(tmpString);
    dialogDumpString.append(tmpString);
    dialogDumpString.append("\nmRemoteContact:");
    mRemoteContact.toString(tmpString);
    dialogDumpString.append(tmpString);
    dialogDumpString.append("\nmRouteSet:");
    dialogDumpString.append(mRouteSet);
    dialogDumpString.append("\nmInitialMethod:");
    dialogDumpString.append(mInitialMethod);
    dialogDumpString.append("\nmsLocalRequestUri:");
    dialogDumpString.append(msLocalRequestUri);
    dialogDumpString.append("\nmsRemoteRequestUri:");
    dialogDumpString.append(msRemoteRequestUri);
    dialogDumpString.append("\nmLocalInitiatedDialog:");
    dialogDumpString.append(mLocalInitiatedDialog ? "T" : "F");
    sprintf(numberString, "%d", mInitialLocalCseq);
    dialogDumpString.append("\nmInitialLocalCseq:");
    dialogDumpString.append(numberString);
    sprintf(numberString, "%d", mInitialRemoteCseq);
    dialogDumpString.append("\nmInitialRemoteCseq:");
    dialogDumpString.append(numberString);
    sprintf(numberString, "%d", mLastLocalCseq);
    dialogDumpString.append("\nmLastLocalCseq:");
    dialogDumpString.append(numberString);
    sprintf(numberString, "%d", mLastRemoteCseq);
    dialogDumpString.append("\nmLastRemoteCseq:");
    dialogDumpString.append(numberString);
    sprintf(numberString, "%d", mDialogState);
    dialogDumpString.append("\nmDialogState:");
    dialogDumpString.append(numberString);
}

void SipDialog::getStateString(DialogState state,
                               UtlString& stateString)
{
    switch(state)
    {
    case DIALOG_UNKNOWN:
        stateString = "DIALOG_UNKNOWN";
        break;
    case DIALOG_EARLY:
        stateString = "DIALOG_EARLY";
        break;
    case DIALOG_ESTABLISHED:
        stateString = "DIALOG_ESTABLISHED";
        break;
    case DIALOG_FAILED:
        stateString = "DIALOG_FAILED";
        break;
    case DIALOG_TERMINATED:
        stateString = "DIALOG_TERMINATED";
        break;

    // This should not happen
    default:
        stateString = "DIALOG_????: ";
        char stateCode[20];
        sprintf(stateCode, "%d", state);
        stateString.append(stateCode);
        break;
    }
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
