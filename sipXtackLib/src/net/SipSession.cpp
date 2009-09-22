//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <net/SipSession.h>
#include <net/SipMessage.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipSession::SipSession(const SipMessage* initialMessage,
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
           initialMessage->getFromUrl(mLocalUrl);
           initialMessage->getToUrl(mRemoteUrl);
           initialMessage->getCSeqField(&mInitialLocalCseq, &mInitialMethod);
           initialMessage->getRequestUri(&msLocalRequestUri);
           mLastFromCseq = mInitialLocalCseq;
           mLastToCseq = -1;
       }
       // The transaction was initiated from the other side
       else
       {
           initialMessage->getFromUrl(mRemoteUrl);
           initialMessage->getToUrl(mLocalUrl);
           initialMessage->getCSeqField(&mInitialRemoteCseq, &mInitialMethod);
           initialMessage->getRequestUri(&msRemoteRequestUri);
           mLastToCseq = mInitialRemoteCseq;
           mLastFromCseq = -1;
           mInitialLocalCseq = -1;
       }

       if(!initialMessage->isResponse())
       {
           UtlString uri;
           initialMessage->getRequestUri(&uri);
           if(isFromLocal)
           {
               mRemoteContact = uri;
           }
           else
           {
               mLocalContact = uri;
           }
       }

       UtlString contact;
       initialMessage->getContactUri(0, &contact);
       if(isFromLocal)
       {
           mLocalContact = contact;
       }
       else
       {
           mRemoteContact = contact;
       }
   }
   else
   {
       mLastFromCseq = -1;
       mLastToCseq = -1;
       mInitialLocalCseq = -1;
       mInitialRemoteCseq = -1;
   }

   mSessionState = SESSION_UNKNOWN;
}

// Constructor
SipSession::SipSession(const char* callId, const char* toUrl, const char* fromUrl)
    : UtlString(callId)
{
    mRemoteUrl = Url(toUrl);
    mLocalUrl = Url(fromUrl);

    mInitialLocalCseq = -1;
    mInitialRemoteCseq = -1;
    mLastFromCseq = -1;
    mLastToCseq = -1;
    mSessionState = SESSION_UNKNOWN;
}

// Copy constructor
SipSession::SipSession(const SipSession& rSipSession)
  : UtlString(rSipSession)
{
   mLocalUrl = rSipSession.mLocalUrl;
   mRemoteUrl = rSipSession.mRemoteUrl;
   mLocalContact = rSipSession.mLocalContact;
   mRemoteContact = rSipSession.mRemoteContact;
   mInitialMethod = rSipSession.mInitialMethod;
   mInitialLocalCseq = rSipSession.mInitialLocalCseq;
   mInitialRemoteCseq = rSipSession.mInitialRemoteCseq;
   mLastFromCseq = rSipSession.mLastFromCseq;
   mLastToCseq = rSipSession.mLastToCseq;
   mSessionState = rSipSession.mSessionState;
   msLocalRequestUri = rSipSession.msLocalRequestUri;
   msRemoteRequestUri = rSipSession.msRemoteRequestUri;
   setCallId(rSipSession.data());

   UtlHashMapIterator toTagMapIterator(rSipSession.mProvisionalToTags);
   toTagMapIterator.reset();

   while(NULL != toTagMapIterator())
   {
       UtlString * key = new UtlString (*(dynamic_cast<UtlString *> (toTagMapIterator.key())));
       UtlString * value = new UtlString (*(dynamic_cast<UtlString *> (toTagMapIterator.value())));
       mProvisionalToTags.insertKeyAndValue(key, value);
   }
}


// Destructor
SipSession::~SipSession()
{
    mProvisionalToTags.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipSession&
SipSession::operator=(const SipSession& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   UtlString::operator=(rhs);  // assign fields for parent class


   mLocalUrl = rhs.mLocalUrl;
   mRemoteUrl = rhs.mRemoteUrl;
   mLocalContact = rhs.mLocalContact;
   mRemoteContact = rhs.mRemoteContact;
   mInitialMethod = rhs.mInitialMethod;
   mInitialLocalCseq = rhs.mInitialLocalCseq;
   mInitialRemoteCseq = rhs.mInitialRemoteCseq;
   mLastFromCseq = rhs.mLastFromCseq;
   mLastToCseq = rhs.mLastToCseq;
   mSessionState = rhs.mSessionState;
   msLocalRequestUri = rhs.msLocalRequestUri;
   msRemoteRequestUri = rhs.msRemoteRequestUri;

   UtlHashMapIterator toTagMapIterator(rhs.mProvisionalToTags);
   toTagMapIterator.reset();

   while(NULL != toTagMapIterator())
   {
      UtlString * key = new UtlString (*(dynamic_cast<UtlString *> (toTagMapIterator.key())));
      UtlString * value = new UtlString (*(dynamic_cast<UtlString *> (toTagMapIterator.value())));
      mProvisionalToTags.insertKeyAndValue(key, value);
   }

   return *this;
}

void SipSession::updateSessionData(SipMessage& message)
{

    int cSeq;
    UtlString method;
    message.getCSeqField(&cSeq, &method);
    int responseCode = message.getResponseStatusCode();

    // Figure out if the request is from the session initiator or
    // the destination
    if(isMessageFromInitiator(message))
    {
        // This message is part of a transaction initiated by
        // the caller/initiator of the session

        if(cSeq > mLastFromCseq)
        {
            mLastFromCseq = cSeq;

            if(method.compareTo(SIP_BYE_METHOD) == 0)
            {
                mSessionState = SESSION_TERMINATED;
            }
        }

        if(cSeq == mInitialLocalCseq)
        {
            if(message.isResponse())
            {
                if(method.compareTo(SIP_INVITE_METHOD) == 0)
                {
                    if(responseCode >= SIP_OK_CODE &&
                        responseCode < SIP_3XX_CLASS_CODE)
                    {
                        mSessionState = SESSION_SETUP;
                        // The tag gets set in the 2xx response
                        // so we need to update the URL
                        message.getToUrl(mRemoteUrl);
                    }
                    else if(responseCode > SIP_3XX_CLASS_CODE)
                    {
                        // The session failed to be setup
                        mSessionState = SESSION_FAILED;
                    }
                }
            }
            else
            {
                if(method.compareTo(SIP_INVITE_METHOD) == 0)
                {
                    mSessionState = SESSION_INITIATED;
                }
                else if(method.compareTo(SIP_CANCEL_METHOD) == 0)
                {
                    mSessionState = SESSION_FAILED;
                }
            }


        }
    }
    else if(isMessageFromDestination(message))
    {
        // This message is part of a transaction initiated by
        // the callee/destination of the session
        if(cSeq > mLastToCseq)
        {
            mLastToCseq = cSeq;

            if(method.compareTo(SIP_BYE_METHOD) == 0)
            {
                mSessionState = SESSION_TERMINATED;
            }
        }
    }

}

/* ============================ ACCESSORS ================================= */

void SipSession::getCallId(UtlString& callId)
{
    callId = data();
}

void SipSession::setCallId(const char* callId)
{
    remove(0);
    append(callId ? callId : "");
}

void SipSession::getFromUrl(Url& fromUrl)
{
    fromUrl = mLocalUrl;
}

void SipSession::setFromUrl(const Url& fromUrl)
{
    mLocalUrl = fromUrl;
}

void SipSession::getToUrl(Url& toUrl)
{
    toUrl = mRemoteUrl;
}

void SipSession::setToUrl(const Url& toUrl)
{
    mRemoteUrl = toUrl;
}

void SipSession::getRemoteContact(Url& remoteContact)
{
    remoteContact = mRemoteContact;
}

void SipSession::setRemoteContact(const Url& remoteContact)
{
    mRemoteContact = remoteContact;
}

void SipSession::getLocalContact(Url& localContact)
{
    localContact = mLocalContact;
}

void SipSession::setLocalContact(const Url& localContact)
{
    mLocalContact = localContact;
}

void SipSession::getLocalRequestUri(UtlString& requestUri)
{
   requestUri = msLocalRequestUri;
}

void SipSession::setLocalRequestUri(UtlString& requestUri)
{
   msLocalRequestUri = requestUri;
}

void SipSession::getRemoteRequestUri(UtlString& requestUri)
{
   requestUri = msRemoteRequestUri;
}

void SipSession::setRemoteRequestUri(UtlString& requestUri)
{
   msRemoteRequestUri = requestUri;
}


void SipSession::getInitialMethod(UtlString& method)
{
    method = mInitialMethod;
}

void SipSession::setInitialMethod(const char* method)
{
    mInitialMethod = method;
}

int SipSession::getLastFromCseq()
{
    return(mLastFromCseq);
}

void SipSession::setLastFromCseq(int lastFromCseq)
{
    mLastFromCseq = lastFromCseq;
}

int SipSession::getLastToCseq()
{
    return(mLastToCseq);
}

void SipSession::setLastToCseq(int lastToCseq)
{
    mLastToCseq = lastToCseq;
}

int SipSession::getNextFromCseq()
{
    mLastFromCseq++;
    return(mLastFromCseq);
}

void SipSession::getProvisionalToTags(UtlHashMap& provisionalToTags)
{
    UtlHashMapIterator toTagMapIterator(mProvisionalToTags);
    toTagMapIterator.reset();

    while(NULL != toTagMapIterator())
    {
        UtlString * key = new UtlString (*(dynamic_cast<UtlString *> (toTagMapIterator.key())));
        UtlString * value = new UtlString (*(dynamic_cast<UtlString *> (toTagMapIterator.value())));
        provisionalToTags.insertKeyAndValue(key, value);
    }
}

void SipSession::setProvisionalToTags(UtlHashMap& provisionalToTags)
{
    mProvisionalToTags.destroyAll();

    UtlHashMapIterator toTagMapIterator(provisionalToTags);
    toTagMapIterator.reset();

    while(NULL != toTagMapIterator())
    {
        UtlString * key = new UtlString (*(dynamic_cast<UtlString *> (toTagMapIterator.key())));
        UtlString * value = new UtlString (*(dynamic_cast<UtlString *> (toTagMapIterator.value())));
        if(!mProvisionalToTags.contains(key))
        {
            mProvisionalToTags.insertKeyAndValue(key, value);
        }
        else
        {
            delete key;
            delete value;
        }
    }
}

void SipSession::toString(UtlString& output) const
{
    UtlString temp ;
    char cTemp[64] ;

    output.append("SipSession {");

    output.append("\n  mLocalUrl=");
    mLocalUrl.toString(temp);
    output.append(temp);

    output.append("\n  mRemoteUrl=");
    mRemoteUrl.toString(temp);
    output.append(temp);

    output.append("\n  mLocalContact=");
    mLocalContact.toString(temp);
    output.append(temp);

    output.append("\n  mRemoteContact=");
    mRemoteContact.toString(temp);
    output.append(temp);

    output.append("\n  mInitialMethod=");
    output.append(mInitialMethod);

    output.append("\n  msLocalRequestUri=");
    output.append(msLocalRequestUri);

    output.append("\n  msRemoteRequestUri=");
    output.append(msRemoteRequestUri);

    output.append("\n  mInitialLocalCseq=");
    sprintf(cTemp, "%d", mInitialLocalCseq);
    output.append(cTemp);

    output.append("\n  mInitialRemoteCseq=");
    sprintf(cTemp, "%d", mInitialRemoteCseq);
    output.append(cTemp);

    output.append("\n  mLastFromCseq=");
    sprintf(cTemp, "%d", mLastFromCseq);
    output.append(cTemp);

    output.append("\n  mLastToCseq=");
    sprintf(cTemp, "%d", mLastToCseq);
    output.append(cTemp);

    output.append("\n  mSessionState=");
    sprintf(cTemp, "%d", mSessionState);
    output.append(cTemp);

    output.append("}\n");
}

/* ============================ INQUIRY =================================== */

UtlBoolean SipSession::isMessageFromInitiator(const SipMessage& message) const
{
    Url messageFromUrl;
    Url messageToUrl;
    UtlString messageCallId;
    message.getFromUrl(messageFromUrl);
    message.getToUrl(messageToUrl);
    message.getCallIdField(&messageCallId);

    return
       messageCallId.compareTo(*this) == 0 &&
       (message.isResponse() ?
        (SipMessage::isSameSession(mRemoteUrl, messageFromUrl) &&
         SipMessage::isSameSession(mLocalUrl, messageToUrl, TRUE)) :
        (SipMessage::isSameSession(mRemoteUrl, messageToUrl, TRUE) &&
         SipMessage::isSameSession(mLocalUrl, messageFromUrl)));
}

UtlBoolean SipSession::isMessageFromDestination(const SipMessage& message) const
{
    Url messageFromUrl;
    Url messageToUrl;
    UtlString messageCallId;
    message.getFromUrl(messageFromUrl);
    message.getToUrl(messageToUrl);
    message.getCallIdField(&messageCallId);

    return
       messageCallId.compareTo(*this) == 0 &&
       (message.isResponse() ?
        (SipMessage::isSameSession(mRemoteUrl, messageToUrl, TRUE) &&
         SipMessage::isSameSession(mLocalUrl, messageFromUrl)) :
        (SipMessage::isSameSession(mRemoteUrl, messageFromUrl) &&
         SipMessage::isSameSession(mLocalUrl, messageToUrl, TRUE)));
}

UtlBoolean SipSession::isSameSession(const SipMessage& message) const
{
    return(isMessageFromInitiator(message) ||
           isMessageFromDestination(message));

}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
