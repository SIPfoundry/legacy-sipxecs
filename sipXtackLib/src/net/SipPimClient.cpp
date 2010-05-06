//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <net/SipPimClient.h>
#include <net/HttpBody.h>
#include <net/SipMessage.h>
#include <net/SipUserAgent.h>
#include <net/NetMd5Codec.h>
#include <net/CallId.h>
#include <os/OsDateTime.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipPimClient::SipPimClient(SipUserAgent& userAgent,
                           Url& presentityAor) :
   OsServerTask("SipPimClient")
{
    presentityAor.toString(mFromField);
    mPresentityAor = presentityAor;
    mpUserAgent = &userAgent;

    // Register to get incoming MESSAGE requests
    OsMsgQ* myQueue = getMessageQueue();
    userAgent.addMessageObserver(*myQueue,
                                SIP_MESSAGE_METHOD,
                                TRUE, // requests
                                FALSE, // responces
                                TRUE, // incoming
                                FALSE); // outgoing

}

// Destructor
SipPimClient::~SipPimClient()
{
}

/* ============================ MANIPULATORS ============================== */


// Assignment operator
SipPimClient&
SipPimClient::operator=(const SipPimClient& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

//! Send a pager style instant message to the given destination
UtlBoolean SipPimClient::sendPagerMessage(Url& destinationAor,
                                          const char* messageText,
                                          int& responseCode,
                                          UtlString& responseCodeText)
{
    UtlBoolean returnCode = FALSE;
    responseCode = -1;
    responseCodeText.remove(0);

    if(messageText && *messageText)
    {
        // Construct the text body
        HttpBody* textBody = new HttpBody(messageText,
                                          strlen(messageText),
                                          CONTENT_TYPE_TEXT_PLAIN);

        // Construct the MESSAGE request
        UtlString toAddress = destinationAor.toString();
        UtlString requestUri;
        destinationAor.getUri(requestUri);
        UtlString callId;
        CallId::getNewCallId(callId);
        SipMessage messageRequest;
        messageRequest.setRequestData(SIP_MESSAGE_METHOD, requestUri,
                         mFromField, toAddress,
                         callId,
                         1, // sequenceNumber
                         NULL); // contactUrl

        // Attache the body
        messageRequest.setBody(textBody);
        messageRequest.setContentType(CONTENT_TYPE_TEXT_PLAIN);

        // Set the queue to which the response will be deposited
        // for this specific request.
        OsMsgQ responseQueue("SipPimClient::sendPagerMessage::responseQueue");
        messageRequest.setResponseListenerQueue(&responseQueue);

        // Send the request
        returnCode = mpUserAgent->send(messageRequest);

        // wait for the response
        OsMsg* qMessage = NULL;
        // For now we will block forever.  Theoretically this should
        // always get a response (e.g. worst case a 408 timed out).
        // If we do not wait forever, we need to be sure to wait the
        // the maximum transaction timeout period so that the qMessage
        // exists when the SipUserAgent queues the response.
        responseQueue.receive(qMessage);


        // If we got a response, get the response status code and text
        if(qMessage)
        {
            int msgType = qMessage->getMsgType();
            int msgSubType = qMessage->getMsgSubType();

            // SIP message
            if(msgType == OsMsg::PHONE_APP &&
               msgSubType == SipMessage::NET_SIP_MESSAGE)
            {
                const SipMessage* messageResponse =
                    ((SipMessageEvent*)qMessage)->getMessage();

                if(messageResponse && messageResponse->isResponse())
                {
                    responseCode = messageResponse->getResponseStatusCode();
                    messageResponse->getResponseStatusText(&responseCodeText);
                }
            }
        }
    }

    return(returnCode);
}

void SipPimClient::setIncomingImTextHandler(
                           void (*textHandler)(const UtlString& fromAddress,
                                 const char* textMessage,
                                 int textLength,
                                 const SipMessage& messageRequest))
{
    mpTextHandlerFunction = textHandler;
}


//! Update the presence state of the presentity indicate
UtlBoolean SipPimClient::updatePresenceState(SipxRpidStates newState)
{
    UtlBoolean returnCode = FALSE;
    return(returnCode);
}

UtlBoolean SipPimClient::handleMessage(OsMsg& eventMessage)
{
    int msgType = eventMessage.getMsgType();
    int msgSubType = eventMessage.getMsgSubType();

    // SIP message
    if(msgType == OsMsg::PHONE_APP &&
       msgSubType == SipMessage::NET_SIP_MESSAGE)
    {
        const SipMessage* sipMessage = ((SipMessageEvent&)eventMessage).getMessage();

        // If this is a MESSAGE request
        UtlString method;
        if(sipMessage) sipMessage->getRequestMethod(&method);
        method.toUpper();
        UtlBoolean responseSent = FALSE;
        if(sipMessage &&
            method.compareTo(SIP_MESSAGE_METHOD) == 0 &&
            !sipMessage->isResponse())
        {
            const HttpBody* messageBody = sipMessage->getBody();
            UtlString contentType = messageBody->getContentType();
            // Trim off the MIME parameters if present
            contentType.remove(strlen(CONTENT_TYPE_TEXT_PLAIN));


            // We have a text body and a callback handler function
            if(messageBody &&
               mpTextHandlerFunction &&
               contentType.compareTo(CONTENT_TYPE_TEXT_PLAIN, UtlString::ignoreCase) == 0)
            {
                const char* bodyBytes;
                ssize_t bodyLength;
                messageBody->getBytes(&bodyBytes, &bodyLength);
                UtlString fromField;
                sipMessage->getFromField(&fromField);

                // Send back a 200 response
                SipMessage response;
                response.setResponseData(sipMessage, SIP_OK_CODE, SIP_OK_TEXT);
                mpUserAgent->send(response);
                responseSent = TRUE;

                // Invoke the call back with the info
                mpTextHandlerFunction(fromField, bodyBytes, bodyLength,
                    *sipMessage);

            }

            if(!responseSent)
            {
                // Send an error as we do not accept the content type
                SipMessage badContentResponse;
                badContentResponse.setResponseData(sipMessage,
                                                    SIP_BAD_MEDIA_CODE,
                                                    SIP_BAD_MEDIA_TEXT);
                mpUserAgent->send(badContentResponse);
            }

        }
    }
    return(TRUE);
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */



/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */
