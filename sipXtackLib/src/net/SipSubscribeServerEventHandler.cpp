//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsSysLog.h>
#include <utl/UtlString.h>
#include <os/OsSysLog.h>
#include <net/SipSubscribeServerEventHandler.h>
#include <net/SipPublishContentMgr.h>
#include <net/SipMessage.h>
#include <net/Url.h>


// Private class to contain callback for eventTypeKey
class SubscribeServerSubscriptionState : public UtlString
{
public:
    SubscribeServerSubscriptionState();

    virtual ~SubscribeServerSubscriptionState();

    // Parent UtlString contains the dialog handle

private:
    //! DISALLOWED accidental copying
    SubscribeServerSubscriptionState(const SubscribeServerSubscriptionState& rSubscribeServerSubscriptionState);
    SubscribeServerSubscriptionState& operator=(const SubscribeServerSubscriptionState& rhs);
};



// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */
SubscribeServerSubscriptionState::SubscribeServerSubscriptionState()
{
}
SubscribeServerSubscriptionState::~SubscribeServerSubscriptionState()
{
}

// Constructor
SipSubscribeServerEventHandler::SipSubscribeServerEventHandler()
{
}


// Copy constructor NOT IMPLEMENTED
SipSubscribeServerEventHandler::SipSubscribeServerEventHandler(const SipSubscribeServerEventHandler& rSipSubscribeServerEventHandler)
{
}


// Destructor
SipSubscribeServerEventHandler::~SipSubscribeServerEventHandler()
{
    // Iterate through and delete all the dialogs
    // TODO:
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipSubscribeServerEventHandler&
SipSubscribeServerEventHandler::operator=(const SipSubscribeServerEventHandler& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

UtlBoolean SipSubscribeServerEventHandler::getKeys(const SipMessage& subscribeRequest,
                                                   UtlString& resourceId,
                                                   UtlString& eventTypeKey,
                                                   UtlString& eventType)
{
    // default resourceId is the identity
    UtlString uriString;
    subscribeRequest.getRequestUri(&uriString);
    Url uri(uriString);
    uri.getIdentity(resourceId);
    // Make the resourceId be a proper URI by prepending "sip:".
    resourceId.prepend("sip:");

    // Default event key is the event type with no parameters
    subscribeRequest.getEventField(&eventTypeKey, NULL);
    // Event type is the same.
    eventType = eventTypeKey;

    return(TRUE);
}

UtlBoolean SipSubscribeServerEventHandler::isAuthenticated(const SipMessage& subscribeRequest,
                                                           SipMessage& subscribeResponse)
{
    // By default no authentication required
    return TRUE;
}

UtlBoolean SipSubscribeServerEventHandler::isAuthorized(const SipMessage& subscribeRequest,
                                                       SipMessage& subscribeResponse)
{
    // By default no authorization required
    return TRUE;
}

UtlBoolean SipSubscribeServerEventHandler::getNotifyContent(const UtlString& resourceId,
                                                            const UtlString& eventTypeKey,
                                                            const UtlString& eventType,
                                                            SipPublishContentMgr& contentMgr,
                                                            const char* acceptHeaderValue,
                                                            SipMessage& notifyRequest,
                                                            UtlBoolean fullState)
{
    UtlBoolean gotBody = FALSE;
    // Default behavior is to just go get the content from
    // the content manager and attach it to the notify
    HttpBody* messageBody = NULL;
    UtlBoolean isDefaultEventContent;
    gotBody = contentMgr.getContent(resourceId,
                                    eventTypeKey,
                                    eventType,
                                    acceptHeaderValue,
                                    messageBody,
                                    isDefaultEventContent,
                                    fullState);

    // The body will be freed with the NOTIFY message.
    if(messageBody)
    {
        const char* contentTypePtr = messageBody->getContentType();
        UtlString contentType;
        if(contentTypePtr)
        {
            contentType = contentTypePtr;
        }
        else
        {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                "SipSubscribeServerEventHandler::getNotifyContent body published for resourceId: '%s' eventTypeKey: '%s' with no content type",
                resourceId.data() ? resourceId.data() : "<null>",
                eventTypeKey.data() ? eventTypeKey.data() : "<null>");

            contentType = "text/unknown";
        }

        notifyRequest.setContentType(contentType);
        notifyRequest.setBody(messageBody);

        UtlString request;
        ssize_t requestLength;
        notifyRequest.getBytes(&request, &requestLength);
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "SipSubscribeServerEventHandler::getNotifyContent resourceId '%s', eventTypeKey '%s' contentType '%s' NOTIFY message length = %zu, message = '%s'",
                      resourceId.data(), eventTypeKey.data(),
                      contentType.data(), requestLength, request.data());
    }

    return(gotBody);
}

/* ============================ ACCESSORS ================================= */



/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
