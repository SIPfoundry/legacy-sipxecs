//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsMsg.h>
#include <os/OsEventMsg.h>
#include <utl/UtlHashMapIterator.h>
#include <net/SipPublishServer.h>
#include <net/SipUserAgent.h>
#include <net/SipPublishServerEventStateMgr.h>
#include <net/SipPublishServerEventStateCompositor.h>
#include <net/HttpBody.h>
#include <net/SipMessage.h>


// Private class to contain event type and event specific utilities
class PublishServerEventData : public UtlString
{
public:
    PublishServerEventData();

    virtual ~PublishServerEventData();

    // Parent UtlString contains the eventType
    SipPublishServerEventStateCompositor* mpEventSpecificStateCompositor;
    SipUserAgent* mpEventSpecificUserAgent;
    SipPublishServerEventStateMgr* mpEventSpecificStateMgr;

private:
    //! DISALLOWED accidental copying
    PublishServerEventData(const PublishServerEventData& rPublishServerEventData);
    PublishServerEventData& operator=(const PublishServerEventData& rhs);
};

PublishServerEventData::PublishServerEventData()
{
    mpEventSpecificStateCompositor = NULL;
    mpEventSpecificUserAgent = NULL;
    mpEventSpecificStateMgr = NULL;
}

PublishServerEventData::~PublishServerEventData()
{
}


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

SipPublishServer* SipPublishServer::buildBasicServer(SipUserAgent& userAgent,
                                                     const char* eventType)
{
    SipPublishServer* newServer = NULL;

    // Create a default event state compositor
    SipPublishServerEventStateCompositor* eventStateCompositor = new SipPublishServerEventStateCompositor();

    // Create a default event state manager
    SipPublishServerEventStateMgr* eventStateMgr = new SipPublishServerEventStateMgr();

    newServer = new SipPublishServer(userAgent,
                                     *eventStateMgr,
                                     *eventStateCompositor);

    // Enable the server to accept the given SIP event package
    newServer->enableEventType(eventType,
                               &userAgent,
                               eventStateMgr,
                               eventStateCompositor);

    return(newServer);
}

// Constructor
SipPublishServer::SipPublishServer(SipUserAgent& defaultUserAgent,
                                   SipPublishServerEventStateMgr& defaultEventStateMgr,
                                   SipPublishServerEventStateCompositor& defaultEventStateCompositor)
   : mPublishServerMutex(OsMutex::Q_FIFO)
{
    mpDefaultUserAgent = &defaultUserAgent;
    mpDefaultEventStateMgr = &defaultEventStateMgr;
    mpDefaultCompositor = &defaultEventStateCompositor;
}


// Copy constructor NOT IMPLEMENTED
SipPublishServer::SipPublishServer(const SipPublishServer& rSipPublishServer)
   : mPublishServerMutex(OsMutex::Q_FIFO)
{
}


// Destructor
SipPublishServer::~SipPublishServer()
{
    // Iterate through and delete all the event data
    // TODO:
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipPublishServer&
SipPublishServer::operator=(const SipPublishServer& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}


UtlBoolean SipPublishServer::enableEventType(const char* eventTypeToken,
                                             SipUserAgent* userAgent,
                                             SipPublishServerEventStateMgr* eventStateMgr,
                                             SipPublishServerEventStateCompositor* eventStateCompositor)
{
    UtlBoolean addedEvent = FALSE;
    UtlString eventName(eventTypeToken ? eventTypeToken : "");
    lockForWrite();
    // Only add the event support if it does not already exist;
    PublishServerEventData* eventData =
        (PublishServerEventData*) mEventDefinitions.find(&eventName);

    if(!eventData)
    {
        addedEvent = TRUE;
        eventData = new PublishServerEventData();
        *((UtlString*)eventData) = eventName;
        eventData->mpEventSpecificUserAgent = userAgent ? userAgent : mpDefaultUserAgent;
        eventData->mpEventSpecificStateCompositor = eventStateCompositor ? eventStateCompositor : mpDefaultCompositor;
        eventData->mpEventSpecificStateMgr = eventStateMgr ? eventStateMgr : mpDefaultEventStateMgr;
        mEventDefinitions.insert(eventData);

        // Register an interest in PUBLISH requests for this event type
        eventData->mpEventSpecificUserAgent->addMessageObserver(*(getMessageQueue()),
                                                                SIP_PUBLISH_METHOD,
                                                                TRUE, // requests
                                                                FALSE, // not reponses
                                                                TRUE, // incoming
                                                                FALSE, // no outgoing
                                                                eventName,
                                                                NULL,
                                                                NULL);
    }

    unlockForWrite();

    return(addedEvent);
}

UtlBoolean SipPublishServer::disableEventType(const char* eventType)
{
    UtlString eventName(eventType);
    lockForWrite();

    PublishServerEventData* eventData =
       dynamic_cast <PublishServerEventData*> (mEventDefinitions.remove(&eventName));

    if (eventData)
    {
        // Unregister interest in PUBLISH requests for this event type
        eventData->mpEventSpecificUserAgent->removeMessageObserver(*(getMessageQueue()));

        delete eventData;
    }

    unlockForWrite();

    return eventData != NULL;
}

UtlBoolean SipPublishServer::handleMessage(OsMsg &eventMessage)
{
    int msgType = eventMessage.getMsgType();
    int msgSubType = eventMessage.getMsgSubType();

    // SIP message
    if(msgType == OsMsg::PHONE_APP &&
       msgSubType == SipMessage::NET_SIP_MESSAGE)
    {
        const SipMessage* sipMessage = ((SipMessageEvent&)eventMessage).getMessage();
        UtlString method;
        if(sipMessage)
        {
            sipMessage->getCSeqField(NULL, &method);
        }

        // PUBLISH requests
        if(sipMessage &&
           !sipMessage->isResponse() &&
           method.compareTo(SIP_PUBLISH_METHOD) == 0)
        {
            handlePublish(*sipMessage);
        }
    }

    return(TRUE);
}



/* ============================ ACCESSORS ================================= */

SipPublishServerEventStateCompositor*
SipPublishServer::getEventStateCompositor(const UtlString& eventType)
{
    SipPublishServerEventStateCompositor* eventStateCompositor = NULL;
    lockForRead();
    PublishServerEventData* eventData =
        (PublishServerEventData*) mEventDefinitions.find(&eventType);
    if(eventData)
    {
        eventStateCompositor = eventData->mpEventSpecificStateCompositor;
    }

    else
    {
        eventStateCompositor = mpDefaultCompositor;
    }
    unlockForRead();

    return(eventStateCompositor);
}

SipPublishServerEventStateMgr* SipPublishServer::getEventStateMgr(const UtlString& eventType)
{
    SipPublishServerEventStateMgr* eventStateMgr = NULL;
    lockForRead();
    PublishServerEventData* eventData =
        (PublishServerEventData*) mEventDefinitions.find(&eventType);
    if(eventData)
    {
        eventStateMgr = eventData->mpEventSpecificStateMgr;
    }

    else
    {
        eventStateMgr = mpDefaultEventStateMgr;
    }
    unlockForRead();

    return(eventStateMgr);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean SipPublishServer::handlePublish(const SipMessage& publishRequest)
{
    UtlBoolean handledPublish = FALSE;
    UtlString eventName;
    publishRequest.getEventFieldParts(&eventName);

    // Not modifying the PublishServerEventData, just reading it
    lockForRead();

    // Get the event specific handler and information
    PublishServerEventData* eventPackageInfo = (PublishServerEventData*)
        mEventDefinitions.find(&eventName);

    // We handle this event type
    if(eventPackageInfo)
    {
        handledPublish = TRUE;
        UtlString resourceId;
        UtlString eventTypeKey;
        SipPublishServerEventStateCompositor* compositor =
            eventPackageInfo->mpEventSpecificStateCompositor;

        // Get the keys used to identify the event state content
        compositor->getKeys(publishRequest,
                            resourceId,
                            eventTypeKey);

        SipMessage publishResponse;

        // Check if authenticated (or if it needs to be authenticated)
        if(compositor->isAuthenticated(publishRequest,
                                       resourceId,
                                       eventTypeKey,
                                       publishResponse))
        {
            // Check if authorized (or if authorization is required)
            if(compositor->isAuthorized(publishRequest,
                                        resourceId,
                                        eventTypeKey,
                                        publishResponse))
            {
                // The publication is allowed, so process the PUBLISH request

                // Modify the expiration if neccessary
                int expiration;
                if (publishRequest.getExpiresField(&expiration))
                {
                    // Check whether the expiration is too brief
                    if (!eventPackageInfo->mpEventSpecificStateMgr->checkExpiration(&expiration))
                    {
                         OsSysLog::add(FAC_SIP, PRI_ERR,
                                       "SipPublishServer::handlePublish interval too brief");

                         publishResponse.setResponseData(&publishRequest,
                                                         SIP_TOO_BRIEF_CODE, SIP_TOO_BRIEF_TEXT);
                         publishResponse.setMinExpiresField(expiration);

                         mpDefaultUserAgent->send(publishResponse);

                         unlockForRead();

                         return(handledPublish);
                    }
                }

                // Generate a new entity tag
                UtlString entity;
                eventPackageInfo->mpEventSpecificStateMgr->generateETag(entity);

                 UtlString sipIfMatchField;
                if (publishRequest.getSipIfMatchField(sipIfMatchField))
                {
                    // Check whether the SIP-If-Match header matches with an existing one
                    if (eventPackageInfo->mpEventSpecificStateMgr->publishExists(sipIfMatchField))
                    {
                        if (expiration == 0)
                        {
                            // Remove the publication
                            eventPackageInfo->mpEventSpecificStateMgr->removePublish(sipIfMatchField);
                        }
                        else
                        {
                            // Update the publication
                            eventPackageInfo->mpEventSpecificStateMgr->updatePublish(sipIfMatchField, entity, resourceId, eventTypeKey, expiration);
                        }
                    }
                    else
                    {
                         OsSysLog::add(FAC_SIP, PRI_ERR,
                                       "SipPublishServer::handlePublish interval too brief");

                         publishResponse.setResponseData(&publishRequest,
                                                         SIP_CONDITIONAL_REQUEST_FAILED_CODE,
                                                         SIP_CONDITIONAL_REQUEST_FAILED_TEXT);

                         mpDefaultUserAgent->send(publishResponse);

                         unlockForRead();

                         return(handledPublish);
                    }
                }
                else
                {
                    // Initial publish
                    eventPackageInfo->mpEventSpecificStateMgr->addPublish(entity, resourceId, eventTypeKey, expiration);
                }

                publishResponse.setResponseData(&publishRequest,
                                                SIP_ACCEPTED_CODE,
                                                SIP_ACCEPTED_TEXT);
                publishResponse.setExpiresField(expiration);

                // Send the response ASAP to minimize resend handling of request
                 eventPackageInfo->mpEventSpecificUserAgent->send(publishResponse);
            }
            // Not authorized
            else
            {
                // Send the response
                eventPackageInfo->mpEventSpecificUserAgent->send(publishResponse);
            }
        }

        // Not authenticated
        else
        {
            // Send the response
            eventPackageInfo->mpEventSpecificUserAgent->send(publishResponse);
        }
    }


    // We should not have received SUBSCRIBE requests for this event type
    // This event type has not been enabled in this SubscribeServer
    else
    {
        OsSysLog::add(FAC_SIP, PRI_ERR,
            "SipPublishServer::handlePublish event type: %s not enabled",
            eventName.data());

        SipMessage eventTypeNotHandled;
        eventTypeNotHandled.setResponseData(&publishRequest,
                                            SIP_BAD_EVENT_CODE, SIP_BAD_EVENT_TEXT);

        mpDefaultUserAgent->send(eventTypeNotHandled);
    }
    unlockForRead();

    return(handledPublish);
}


void SipPublishServer::lockForRead()
{
    mPublishServerMutex.acquireRead();
}

void SipPublishServer::unlockForRead()
{
    mPublishServerMutex.releaseRead();
}

void SipPublishServer::lockForWrite()
{
    mPublishServerMutex.acquireWrite();
}

void SipPublishServer::unlockForWrite()
{
    mPublishServerMutex.releaseWrite();
}

/* ============================ FUNCTIONS ================================= */
