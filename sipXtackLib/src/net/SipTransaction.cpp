//
// Copyright (C) 2007, 2010 Avaya, Inc., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////



// SYSTEM INCLUDES
#include <stdlib.h>
#include <assert.h>

// APPLICATION INCLUDES
#include <os/OsDateTime.h>
#include <os/OsTimer.h>
#include <os/OsQueuedEvent.h>
#include <os/OsEvent.h>
#include <net/SipTransaction.h>
#include <net/BranchId.h>
#include <net/SipMessage.h>
#include <net/SipUserAgent.h>
#include <net/SipMessageEvent.h>
#include <net/NetMd5Codec.h>
#include <net/SipTransactionList.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define SIP_UDP_RESEND_TIMES 4          // Maximum number of times to send/resend messages via UDP
#define SIP_TCP_RESEND_TIMES 4          // Maximum number of times to send/resend messages via TCP
#define MIN_Q_DELTA_SQUARE 0.0000000001 // Smallest Q difference is 0.00001
#define UDP_LARGE_MSG_LIMIT  1200       // spec says 1300, but we may have to add another via

//#define LOG_FORKING
//#define ROUTE_DEBUG
//#define DUMP_TRANSACTIONS
//#define RESPONSE_DEBUG
//#define TEST_PRINT
//#define TEST_TOUCH
//#define LOG_TRANSLOCK

/* //////////////////////////// PUBLIC //////////////////////////////////// */
UtlBoolean SipTransaction::enableTcpResend = FALSE;
UtlBoolean SipTransaction::SendTryingForNist = TRUE;
/* ============================ CREATORS ================================== */

// Constructor
SipTransaction::SipTransaction(SipMessage* initialMsg,
                               UtlBoolean  isOutgoing,
                               UtlBoolean  userAgentTransaction,
                               BranchId*   parentBranch
                               )
   : mpBranchId(NULL)
   , mRequestMethod("")
   , mIsUaTransaction(userAgentTransaction)
   , mSendToPort(PORT_NONE)
   , mSendToProtocol(OsSocket::UNKNOWN)
   , mpDnsDestinations(NULL)
   , mpRequest(NULL)
   , mpLastProvisionalResponse(NULL)
   , mpLastFinalResponse(NULL)
   , mpAck(NULL)
   , mpCancel(NULL)
   , mpCancelResponse(NULL)
   , mpParentTransaction(NULL)
   , mTransactionStartTime(-1)
   , mTransactionState(TRANSACTION_LOCALLY_INIITATED)
   , mDispatchedFinalResponse(FALSE)
   , mProvisionalSdp(FALSE)
   , mIsCanceled(FALSE)
   , mIsRecursing(FALSE)
   , mIsDnsSrvChild(FALSE)
   , mQvalue(1.0)
   , mExpires(-1)
   , mIsBusy(FALSE)
   , mProvoExtendsTimer(FALSE)
   , mWaitingList(NULL)
   , _markedForDeletion(false)
{

#  ifdef ROUTE_DEBUG
   {
      Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                    "SipTransaction::_ new %p msg %p %s %s",
                    this, &initialMsg,
                    isOutgoing ? "OUTGOING" : "INCOMING",
                    userAgentTransaction ? "UA" : "SERVER"
                    );
   }
#  endif

   if(initialMsg)
   {
       mIsServerTransaction = initialMsg->isServerTransaction(isOutgoing);

       initialMsg->getCallIdField(&mCallId);

       // Set the hash key
       buildHash(*initialMsg, isOutgoing, *this);

       initialMsg->getCSeqField(&mCseq, &mRequestMethod);
       if(!initialMsg->isResponse())
       {
           initialMsg->getRequestUri(&mRequestUri);
           initialMsg->getRequestMethod(&mRequestMethod);

           // Do not attach the request here as it will get passed in
           // later for handleOutgoing or handleIncoming

           if(   0 != mRequestMethod.compareTo(SIP_INVITE_METHOD) // not INVITE
              || !initialMsg->getExpiresField(&mExpires))            // or no Expires header field
           {
               mExpires = -1;
           }
       }
       else // this is a response
       {
           // Do not attach the response here as it will get passed in
           // later for handleOutgoing or handleIncoming
       }

       initialMsg->getToUrl(mToField);
       initialMsg->getFromUrl(mFromField);

       if(!mIsServerTransaction) // is this a new client transaction?
       {
          if (mIsUaTransaction)
          {
             // Yes - create a new branch id
             mpBranchId = new BranchId(*initialMsg);
          }
          else
          {
             if (parentBranch)
             {
                // child transaction - use loop key from parent
                mpBranchId = new BranchId(*parentBranch, *initialMsg);
             }
             else
             {
                // new server transaction
                mpBranchId = new BranchId(*initialMsg);
             }
          }
       }
       else
       {
          // This is a server transaction, so the branch id is
          // created by the client and passed in the via
           UtlString viaField;
           initialMsg->getViaFieldSubField(&viaField, 0);
           UtlString branchValue;
           SipMessage::getViaTag(viaField.data(), "branch", branchValue);
           mpBranchId = new BranchId(branchValue);
       }
   }
   else
   {
       mIsServerTransaction = FALSE;
   }

   touch(); // sets mTimeStamp
   mTransactionCreateTime = mTimeStamp;
#ifdef DUMP_TRANSACTIONS
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                  "SipTransaction::constructor dumps whole TransactionTree ");
    justDumpTransactionTree();
#endif
}


// Destructor
SipTransaction::~SipTransaction()
{
#ifdef TEST_PRINT
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "SipTransaction::~ *******************************");
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "SipTransaction::~ Deleting messages at:");
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "SipTransaction::~    %p", mpRequest);
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "SipTransaction::~    %p", mpLastProvisionalResponse);
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "SipTransaction::~    %p", mpLastFinalResponse);
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "SipTransaction::~    %p", mpAck);
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "SipTransaction::~    %p", mpCancel);
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "SipTransaction::~    %p", mpCancelResponse);
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "SipTransaction::~ *******************************");
#endif

    // Optimization: stop timers before doing anything else
    deleteTimers();

    if(mpBranchId)
    {
       delete mpBranchId;
       mpBranchId = NULL;
    }

    if(mpRequest)
    {
       delete mpRequest;
       mpRequest = NULL;
    }

    if(mpLastProvisionalResponse)
    {
       delete mpLastProvisionalResponse;
       mpLastProvisionalResponse = NULL;
    }

    if(mpLastFinalResponse)
    {
       delete mpLastFinalResponse;
       mpLastFinalResponse = NULL;
    }

    if(mpAck)
    {
       delete mpAck;
       mpAck = NULL;
    }

    if(mpCancel)
    {
       delete mpCancel;
       mpCancel = NULL;
    }

    if(mpCancelResponse)
    {
       delete mpCancelResponse;
       mpCancelResponse = NULL;
    }

    if(mpDnsDestinations)
    {
       delete[] mpDnsDestinations;
    }

    if(mWaitingList)
    {
        int numEvents = mWaitingList->entries();

        if(mpParentTransaction)
        {
            Os::Logger::instance().log(FAC_SIP, PRI_ERR, 
                          "SipTransaction::~SipTransaction"
                          " non parent has %d waiting events",
                          numEvents);
        }

        if(numEvents > 0)
        {
            // Cannot call signalAllAvailable as it traverses what
            // may be a broken (i.e. partially deleted) tree
            UtlVoidPtr* eventNode = NULL;
            while ((eventNode = (UtlVoidPtr*) mWaitingList->get()))
            {
                if(eventNode)
                {
                    OsEvent* waitingEvent = (OsEvent*) eventNode->getValue();
                    if(waitingEvent)
                    {
                        // If it is already signaled, the other side
                        // is no longer waiting for the event, so this
                        // side must delete the event.
                        if(waitingEvent->signal(0) == OS_ALREADY_SIGNALED)
                        {
                            delete waitingEvent;
                            waitingEvent = NULL;
                        }
                    }
                    delete eventNode;
                    eventNode = NULL;
                }
            }

            Os::Logger::instance().log(FAC_SIP, PRI_ERR,
                          "SipTransaction::~ "
                          "%d waiting events in list",
                          numEvents);
        }

        delete mWaitingList;
        mWaitingList = NULL;
    }

    //
    // Break the parent/child chain
    //
    if (mpParentTransaction)
      mpParentTransaction->unlinkChild(this);

    UtlSListIterator iterator(mChildTransactions);
    SipTransaction* childTransaction = NULL;
    while ((childTransaction = (SipTransaction*) iterator()))
    {
      childTransaction->mpParentTransaction = 0;
    }
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipTransaction&
SipTransaction::operator=(const SipTransaction& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

enum SipTransaction::messageRelationship
SipTransaction::addResponse(SipMessage*& response,
                            UtlBoolean isOutGoing,                  // send/receive
                            enum messageRelationship relationship)  // casual/serious
{
#ifdef DUMP_TRANSACTIONS
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                  "SipTransaction::addResponse enter"
                  " Tx %p has calling state- %s, "
                  "message is %s, relationship is %s",
                  this, stateString(mTransactionState),
                  isOutGoing ? "outGoing" : "inComing",
                  relationshipString(relationship));
#endif

    if (relationship == MESSAGE_UNKNOWN)
    {
        relationship = whatRelation(*response, isOutGoing);
    }

    switch(relationship)
    {

    case MESSAGE_REQUEST:
        // I do not know why this should ever occur
        // Typically the transaction will first be created with a request
        if (mpRequest)
        {
           Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                         "SipTransaction::addResponse"
                         " of request to existing transaction, IGNORED");
            delete response ;
            response = NULL;
        }
        else
        {
            mpRequest = response;
        }

        if (mTransactionState < TRANSACTION_CALLING)
        {
            mTransactionState = TRANSACTION_CALLING;
            OsTime time;
            OsDateTime::getCurTimeSinceBoot(time);
            mTransactionStartTime = time.seconds();
        }
        break;

    case MESSAGE_PROVISIONAL:
#ifdef TEST_PRINT
        {
            int responseCode = response->getResponseStatusCode();
            int lastResponseCode = -1;

            if (mpLastProvisionalResponse)
            {
                lastResponseCode = mpLastProvisionalResponse->getResponseStatusCode();
            }

            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                          "SipTransaction::addResponse"
                          " got provo - last=%p/%d new=%p/%d outgoing=%d extend=%d",
                          mpLastProvisionalResponse, lastResponseCode,
                          response, responseCode, isOutGoing, mProvoExtendsTimer );
        }
#endif
        if (mpLastProvisionalResponse)
        {
            delete mpLastProvisionalResponse;
        }
        mpLastProvisionalResponse = response;
        if(mTransactionState < TRANSACTION_PROCEEDING)
        {
           mTransactionState = TRANSACTION_PROCEEDING;
        }

        // Set flag if this is an incoming provisional response >100  (101-199)
        // The flag will be tested if the expires timer fires
        // If TRUE, CANCEL will _not_ be sent, instead, a new Expires timer will be started
        if (! isOutGoing )
        {
            int responseCode = mpLastProvisionalResponse->getResponseStatusCode();
            if (   responseCode > SIP_1XX_CLASS_CODE
                && responseCode < SIP_2XX_CLASS_CODE)
            {
                mProvoExtendsTimer = TRUE;
            }
        }

        // Check if there is early media
        // We need this state member as there may be multiple
        // provisional responses and we cannot rely upon the fact
        // that the last one has SDP or not to indicate that there
        // was early media or not.
        if (!mProvisionalSdp)
        {
            if((response->hasSdpBody()))
            {
                mProvisionalSdp = TRUE;
            }
        }
        break;

    case MESSAGE_FINAL:
        if (mpLastFinalResponse)
        {
            delete mpLastFinalResponse;
        }
        mpLastFinalResponse = response;
        if (mTransactionState < TRANSACTION_COMPLETE)
        {
           mTransactionState = TRANSACTION_COMPLETE;
        }
        break;

    case MESSAGE_ACK:
    case MESSAGE_2XX_ACK:
        // relate error ACK or 2xx-ACK-at-the-server to the original INVITE transaction
        // 2xx ACK will be sent to proxy and forwarded to next hop
        if (mpAck)
        {
            Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                          "SipTransaction::addResponse"
                          " ACK already exists, IGNORED");
            delete response ;
            response = NULL;
        }
        else
        {
            mpAck = response;
        }
        break;

    case MESSAGE_2XX_ACK_PROXY:
        // special case 2xx-ACK-at-proxy needs to be sent as new SIP request to the next hop
        if (mpRequest)
        {
           Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                         "SipTransaction::addResponse"
                         " of 2xx ACK to transaction with existing request message, IGNORED");
            delete response ;
            response = NULL;
        }
        else
        {
            mpRequest = response;
        }
        break;


    case MESSAGE_CANCEL:
        if (mpCancel)
        {
            Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                          "SipTransaction::addResponse "
                          "CANCEL already exists, IGNORED");
            delete response ;
            response = NULL;
        }
        else
        {
            mpCancel = response;
        }
        break;

    case MESSAGE_CANCEL_RESPONSE:
        if (mpCancelResponse)
        {
            Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                          "SipTransaction::addResponse "
                          "CANCEL response already exists, IGNORED");
            delete response ;
            response = NULL;
        }
        else
        {
            mpCancelResponse = response;
        }
        break;

    case MESSAGE_UNKNOWN:
    case MESSAGE_UNRELATED:
    case MESSAGE_DUPLICATE:
    default:
        Os::Logger::instance().log(FAC_SIP, PRI_ERR,
                      "SipTransaction::addResponse "
                      "message with bad relationship: %d",
                      relationship);
        delete response ;
        response = NULL;
        break;
    }

#ifdef DUMP_TRANSACTIONS
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                  "SipTransaction::addResponse leave"
                  " Tx %p has calling state- %s, "
                  "message is %s, relationship is %s",
                  this, stateString(mTransactionState),
                  isOutGoing ? "outGoing" : "inComing",
                  relationshipString(relationship));
#endif
    return(relationship);
}

UtlBoolean SipTransaction::handleOutgoing(SipMessage& outgoingMessage,
                                          SipUserAgent& userAgent,
                                          SipTransactionList& transactionList,
                                          enum messageRelationship relationship)
{
    UtlBoolean isResponse = outgoingMessage.isResponse();
    SipMessage* message = &outgoingMessage;

    UtlBoolean sendSucceeded = FALSE;
    UtlString method;
    int cSeq;
    UtlString seqMethod;

    outgoingMessage.getCSeqField(&cSeq, &seqMethod);
    outgoingMessage.getRequestMethod(&method);

#ifdef DUMP_TRANSACTIONS
    enum messageRelationship wasRel = relationship;
#endif

    if (relationship == MESSAGE_UNKNOWN)
    {
       relationship = whatRelation(outgoingMessage, TRUE);
    }

#ifdef DUMP_TRANSACTIONS
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, 
                  "SipTransaction::handleOutgoing "
                  "rel-param=%d rel-now=%d",
                  wasRel, relationship);
#endif

    if (relationship == MESSAGE_DUPLICATE)
    {
        // If this transaction was contructed with this message
        // it will appear as a duplicate
        if(!isResponse &&
            mpRequest &&
            !mIsServerTransaction &&
            mpRequest->getTimesSent() == 0 &&
            mRequestMethod.compareTo(method) == 0)
        {

            message = mpRequest;
        }
        else
        {
            Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                          "SipTransaction::handleOutgoing "
                          "send of duplicate message");
        }

    }

    UtlBoolean addressRequiresDnsSrvLookup(FALSE);
    UtlString toAddress;
    int port = PORT_NONE;
    OsSocket::IpProtocolSocketType protocol = OsSocket::UNKNOWN;

    if(isResponse)
    {
        UtlString protocolString;
        message->getResponseSendAddress(toAddress,
                                        port,
                                        protocolString);
#       ifdef ROUTE_DEBUG
        Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                      "SipTransaction::handleOutgoing"
                      " called getResponseSendAddress, "
                      "returned toAddress = '%s', port = %d, protocolString = '%s'",
                      toAddress.data(), port, protocolString.data());
#       endif
        SipMessage::convertProtocolStringToEnum(protocolString.data(),
                                                protocol);
    }
    else
    {
        // Fix the request so that it is ready to send
        prepareRequestForSend(*message,
                              userAgent,
                              addressRequiresDnsSrvLookup,      // decision is returned
                              toAddress,
                              port,
                              protocol);
#       ifdef ROUTE_DEBUG
        Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                      "SipTransaction::handleOutgoing"
                      " called prepareRequestForSend, returned toAddress = '%s', port = %d, "
                      "protocol = OsSocket::SocketProtocolTypes(%d), "
                      "addressRequiresDnsSrvLookup = %d",
                      toAddress.data(), port, protocol,
                      addressRequiresDnsSrvLookup);
#       endif

        if (toAddress.isNull())
        {
           UtlString bytes;
           ssize_t length;
           message->getBytes(&bytes, &length);
           Os::Logger::instance().log(FAC_SIP, PRI_ERR,
                         "SipTransaction::handleOutgoing "
                         "Unable to obtain To address.  Message is '%s'",
                         bytes.data());
        }

        if(mSendToAddress.isNull())
        {
#          ifdef ROUTE_DEBUG
           Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                         "SipTransaction::handleOutgoing "
                         "setting mSendTo* variables");
#          endif
            mSendToAddress = toAddress;
            mSendToPort = port;
            mSendToProtocol = protocol;
        }
    }   // end prep for send

    // Do not send out CANCEL requests unless tx is DNS child.
    // They do not actually send requests and so should not
    // send CANCELs either.
    if(   !isResponse
       && !mIsDnsSrvChild
       && (method.compareTo(SIP_CANCEL_METHOD) == 0))
    {
        if (Os::Logger::instance().willLog(FAC_SIP, PRI_DEBUG))
        {
            UtlString requestString;
            ssize_t len;
            outgoingMessage.getBytes(&requestString, &len);
            UtlString transString;
            toString(transString, TRUE);
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                          "SipTransaction::handleOutgoing "
                          "should not send CANCEL on DNS parent\n%s\n%s",
                requestString.data(),
                transString.data());
        }
    }

    // Request that requires DNS SRV lookup
    else if (   !isResponse                                 // request
             && addressRequiresDnsSrvLookup                 // already decided
             && method.compareTo(SIP_CANCEL_METHOD) != 0    // not CANCEL
             && !mIsDnsSrvChild                             // not already looked up
             )
    {
        if(mpRequest != NULL)
        {
            if (Os::Logger::instance().willLog(FAC_SIP, PRI_WARNING))
            {
                UtlString requestString;
                ssize_t len;
                outgoingMessage.getBytes(&requestString, &len);
                UtlString transString;
                toString(transString, TRUE);
                Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                    "SipTransaction::handleOutgoing "
                    "mpRequest should be NULL\n%s\n%s",
                    requestString.data(),
                    transString.data());
            }
        }

        // treat forwarding 2xx ACK the same as other requests
        if (  relationship != MESSAGE_REQUEST
           && relationship != MESSAGE_2XX_ACK_PROXY)
        {
            if (Os::Logger::instance().willLog(FAC_SIP, PRI_WARNING))
            {
                UtlString requestString;
                ssize_t len;
                outgoingMessage.getBytes(&requestString, &len);
                UtlString transString;
                toString(transString, TRUE);
                Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                    "SipTransaction::handleOutgoing "
                    "invalid relationship: %s\n%s\n%s",
                    relationshipString(relationship),
                    requestString.data(),
                    transString.data());
            }
        }

        // Make a copy to attach to the transaction
        SipMessage* requestCopy = new SipMessage(outgoingMessage);

#ifdef TEST_PRINT
        Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                      "SipTransaction::handleOutgoing "
                      "calling addResp "
                      " Tx %p has relationship is %s",
                      this, relationshipString(relationship));
#endif
        addResponse(requestCopy,
                    TRUE, // outgoing
                    relationship);

        // Look up the DNS records, create the child transactions,
        // and start pursuing the first child.
        sendSucceeded = recurseDnsSrvChildren(userAgent, transactionList, requestCopy);
    }
    else    // It is a response, cancel, dnsChild, or an ack for failure
    {       // these messages do not get dns lookup, do not get protocol change
        sendSucceeded = doFirstSend(*message,
                                    relationship,
                                    userAgent,
                                    toAddress,
                                    port,
                                    protocol);

        touch();
    }
#ifdef DUMP_TRANSACTIONS
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                  "SipTransaction::handleOutgoing dumps whole TransactionTree %d", sendSucceeded);
    justDumpTransactionTree();
#endif

    return(sendSucceeded);
} // end handleOutgoing

void SipTransaction::prepareRequestForSend(SipMessage& request,
                                           SipUserAgent& userAgent,
                                           UtlBoolean& addressRequiresDnsSrvLookup,
                                           UtlString& toAddress,
                                           int& port,
                                           OsSocket::IpProtocolSocketType& toProtocol)
{
    UtlString protocol;

    // Make sure max-forwards is set and it is not
    // greater than the default value
    int maxForwards;
    int defaultMaxForwards = userAgent.getMaxForwards();
    if(!request.getMaxForwards(maxForwards) ||
        maxForwards > defaultMaxForwards)
    {
        request.setMaxForwards(defaultMaxForwards);
    }

    UtlBoolean ackFor2xx = FALSE;   // ACKs for 200 resp get new routing and URI
    UtlString method;
    request.getRequestMethod(&method);

    if(method.compareTo(SIP_ACK_METHOD) == 0
       && mpLastFinalResponse)
    {
        int responseCode;
        responseCode = mpLastFinalResponse->getResponseStatusCode();
        if(responseCode >= SIP_2XX_CLASS_CODE &&
           responseCode < SIP_3XX_CLASS_CODE)
        {
            ackFor2xx = TRUE;
        }
    }
    // Conditions which don't need new routing, no DNS lookup
    if(mIsDnsSrvChild               // DNS lookup already done
       && !mSendToAddress.isNull()  // sendTo address is known
       && !ackFor2xx)               // ACK for error response, follows response rules
    {
        toAddress = mSendToAddress;
        port = mSendToPort;
        toProtocol = mSendToProtocol;
        addressRequiresDnsSrvLookup = FALSE;

#      ifdef ROUTE_DEBUG
        {
           UtlString protoString;
           SipMessage::convertProtocolEnumToString(toProtocol,protoString);
           Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                         "SipTransaction::prepareRequestForSend "
                         "%p - SRV child ready"
                         "   to %s:%d via '%s'",
                         &request,
                         toAddress.data(), port, protoString.data()
                         );
        }
#      endif
    }

    // Requests must be routed.
    // ACK for a 2xx response follows request rules
    else
    {
        // process header parameters in the request uri,
        // especially moving any route parameters to route headers
        request.applyTargetUriHeaderParams();

        // Default to use the proxy
        userAgent.getProxyServer(0, &toAddress, &port, &protocol);
#       ifdef ROUTE_DEBUG
        Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                      "SipTransaction::prepareRequestForSend"
                      " %p got proxy toAddress '%s', port %d, protocol '%s'",
                      &request, toAddress.data(), port, protocol.data());
#       endif

        // Try to get top-most route
        UtlString routeUri;
        UtlString routeAddress;
        int routePort;
        UtlString routeProtocol;
        request.getRouteUri(0, &routeUri);
        Url routeUrlParser(routeUri);
        UtlString dummyValue;
        UtlBoolean nextHopLooseRoutes = routeUrlParser.getUrlParameter("lr", dummyValue, 0);
        UtlString maddr;
        routeUrlParser.getUrlParameter("maddr", maddr);

        UtlString routeHost;
        SipMessage::parseAddressFromUri(routeUri.data(),
                                        &routeHost, &routePort, &routeProtocol);

        // All of this URL manipulation should be done via
        // the Url (routeUrlParser) object.  However to
        // be safe, we are only using it to get the maddr.
        // If the maddr is present use it as the address
        if(!maddr.isNull())
        {
            routeAddress = maddr;
        }
        else
        {
            routeAddress = routeHost;
        }

#       ifdef ROUTE_DEBUG
        Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                      "SipTransaction::prepareRequestForSend "
                      "%p getting first route uri: '%s'",
                      &request, routeUri.data());
#       endif

        // If there is no route use the configured outbound proxy
        if(routeAddress.isNull())
        {
            // Set by earlier call to userAgent.getProxyServer
            // this path is for debug purposes only
#       ifdef ROUTE_DEBUG
        Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                      "SipTransaction::prepareRequestForSend "
                      "route address is null");
#       endif
        }
        else // there is a route
        {
            toAddress = routeAddress;   // from top-most route or maddr
            port = routePort;
            protocol = routeProtocol;   // can be null if not set in header

            // If this is not a loose route set the URI
            UtlString value;
            if(!nextHopLooseRoutes)
            {
               //Change the URI in the first line to the route Uri
               // so pop the first route uri
                request.removeRouteUri(0, &routeUri);
#               ifdef ROUTE_DEBUG
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                              "SipTransaction::prepareRequestForSend %p"
                              " removing route, no proxy, uri: '%s'",
                              &request, routeUri.data());
#               endif

                // We need to push the URI on the end of the routes
                UtlString uri;
                request.getRequestUri(&uri);
                request.addLastRouteUri(uri.data());

                // Set the URI to the popped route
                UtlString ChangedUri;
                routeUrlParser.getUri(ChangedUri);
                request.changeRequestUri(ChangedUri);
            }
#           ifdef ROUTE_DEBUG
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                          "SipTransaction::prepareRequestForSend "
                          "%p - using route address"
                          "   to %s:%d via '%s'",
                          &request, toAddress.data(), port, protocol.data()
                          );
#           endif
        }   // end "there is a route"

        // No proxy, no route URI, check for an X-sipX-NAT-Route header then
        // try to use URI from message if none found.
        if(toAddress.isNull())
        {
           UtlString uriString;
           request.getSipXNatRoute(&uriString);
           if( uriString.isNull() )
           {
              request.getRequestUri(&uriString);
           }
           Url requestUri(uriString, TRUE);

           requestUri.getHostAddress(toAddress);
           port = requestUri.getHostPort();
           requestUri.getUrlParameter("transport", protocol);
           if(requestUri.getUrlParameter("maddr", maddr) &&
              !maddr.isNull())
           {
              toAddress = maddr;
           }
#          ifdef ROUTE_DEBUG
           Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                         "SipTransaction::prepareRequestForSend %p - "
                         "   using request URI address: %s:%d via '%s'",
                         &request, toAddress.data(), port, protocol.data()
                         );
#          endif
        }

        // No proxy, route URI, temporary sipX route or message URI, use the To field
        if(toAddress.isNull())
        {
           request.getToAddress(&toAddress, &port, &protocol);
#          ifdef ROUTE_DEBUG
           Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                         "SipTransaction::prepareRequestForSend %p "
                         "No URI address, using To address"
                         "   to %s:%d via '%s'",
                         &request, toAddress.data(), port, protocol.data()
                         );
#          endif
        }

        UtlString toField;
        request.getToField(&toField);
#ifdef TEST_PRINT
        Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                      "SipTransaction::prepareRequestForSend "
                      "UA Sending SIP REQUEST to: \"%s\" port: %d",
                      toAddress.data(), port);
#endif

        //SDUA
        UtlString sPort;
        UtlString thisMethod;
        request.getRequestMethod(&thisMethod);

        //check if CANCEL method and has corresponding INVITE
        if(strcmp(thisMethod.data(), SIP_CANCEL_METHOD) == 0)
        {
            //Cancel uses same DNS parameters as matching invite transaction
            //find corresponding INVITE request
            //SipMessage * InviteMsg =  sentMessages.getInviteFor( &message);
            if ( !mIsServerTransaction &&
                 mpRequest &&
                 mRequestMethod.compareTo(SIP_INVITE_METHOD) == 0)
            {
                //copy DNS parameters
                if ( mpRequest->getDNSField(&protocol , &toAddress , &sPort))
                {
                    request.setDNSField( protocol , toAddress , sPort);
                }
            }
        }

        //USE CONTACT OR RECORD ROUTE FIELDS FOR 200 OK responses
        //check if ACK method and if it has contact field set
        //if contact field is set then it is a 200 OK response
        //therefore do not set sticky DNS prameters or DNS look up
        //if DNS field is present request
        if (request.getDNSField(&protocol , &toAddress, &sPort))
        {
            port = atoi(sPort.data());
        }
        else
        {
            addressRequiresDnsSrvLookup = TRUE;
        }

        // If no one specified which protocol
        if(protocol.isNull())
        {
            toProtocol = OsSocket::UNKNOWN;
        }
        else
        {
            SipMessage::convertProtocolStringToEnum(protocol.data(),
                                                    toProtocol);
        }
    }
#   ifdef ROUTE_DEBUG
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                  "SipTransaction::prepareRequestForSend "
                  "%p prepared SIP REQUEST"
                  "   DNS SRV lookup: %s"
                  "   to %s:%d via '%s'",
                  &request,
                  addressRequiresDnsSrvLookup ? "NEEDED" : "NOT NEEDED",
                  toAddress.data(), port, protocol.data()
                  );
#   endif
}

UtlBoolean SipTransaction::doFirstSend(SipMessage& message,
                                       enum messageRelationship relationship,
                                       SipUserAgent& userAgent,
                                       UtlString& toAddress,
                                       int& port,
                                       OsSocket::IpProtocolSocketType& toProtocol)
{
    UtlBoolean sendSucceeded = FALSE;
    UtlBoolean isResponse = message.isResponse();
    UtlString method;
    UtlString seqMethod;
    int responseCode = -1;

    OsSocket::IpProtocolSocketType sendProtocol = message.getSendProtocol();
    // Time (in milliseconds) after which this message should be resent.
    int resendInterval;

#   ifdef ROUTE_DEBUG
    {
       UtlString logProtocol;
       SipMessage::convertProtocolEnumToString(toProtocol, logProtocol);
       Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                     "SipTransaction::doFirstSend "
                     "%p %s to %s:%d via '%s'",
                     &message, relationshipString(relationship),
                     toAddress.data(), port, logProtocol.data()
                     );
    }
#   endif

    if (toProtocol == OsSocket::UNKNOWN)
    {
       if (sendProtocol == OsSocket::UNKNOWN)
       {
          /*
           * This problem should be fixed by XECS-414 which sends an
           * ACK for a 2xx response through the normal routing
           * to determine the protocol using DNS SRV lookups
           */
          toProtocol = OsSocket::UDP;
          Os::Logger::instance().log(FAC_SIP, PRI_ERR,
                        "SipTransaction::doFirstSend "
                        "protocol not explicitly set - using UDP"
                        );
       }
       else
       {
          toProtocol = sendProtocol;
       }
    }

    // Responses:
    if(isResponse)
    {
        responseCode = message.getResponseStatusCode();
        int cSeq;
        message.getCSeqField(&cSeq, &seqMethod);

#       ifdef ROUTE_DEBUG
        {
           UtlString protocolStr;
           SipMessage::convertProtocolEnumToString(toProtocol, protocolStr);
           Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                         "SipTransaction::doFirstSend %p "
                         "Sending RESPONSE to: '%s':%d via: '%s'",
                         this, toAddress.data(), port, protocolStr.data());
        }
#       endif
        // This is the first send, save the address and port to which it get sent
        message.setSendAddress(toAddress.data(), port);
        message.setFirstSent();
    }
    // Requests:
    else
    {
        // This is the first send, save the address and port to which it gets sent.
        message.setSendAddress(toAddress.data(), port);
        message.setFirstSent();
        message.getRequestMethod(&method);

        // Add a Via header, now that we know the protocol.

        // Get the via info.
        UtlString viaAddress;
        UtlString viaProtocolString;
        SipMessage::convertProtocolEnumToString(toProtocol, viaProtocolString);
        int viaPort;

        userAgent.getViaInfo(toProtocol,
                             viaAddress,
                             viaPort);

        // Add the via field data
        message.addVia(viaAddress.data(),
                       viaPort,
                       viaProtocolString,
                       mpBranchId->data(),
                       (toProtocol == OsSocket::UDP) && userAgent.getUseRport());

#       ifdef ROUTE_DEBUG
        Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                      "SipTransaction::doFirstSend message "
                      "send %s:%d via %s",
                      toAddress.data(), port, viaProtocolString.data());
#       endif
    }

    if(
          toProtocol == OsSocket::TCP
#   ifdef SIP_TLS
       || toProtocol == OsSocket::SSL_SOCKET
#   endif
       )
    {
        sendProtocol = toProtocol;
        // Set resend timer based on user agent TCP resend time.
        resendInterval = userAgent.getReliableTransportTimeout();
    }
    else
    {
        if(toProtocol != OsSocket::UDP)
        {
            Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                "SipTransaction::doFirstSend "
                "%p unknown protocol: %d using UDP",
                &message, toProtocol);
        }

        sendProtocol = OsSocket::UDP;
        // Set resend timer based on user agent UDP resend time.
        resendInterval = userAgent.getUnreliableTransportTimeout();
    }

    // Set the transport information
    message.setResendInterval(resendInterval);
    message.setSendProtocol(sendProtocol);
    message.touchTransportTime();

    SipMessage* transactionMessageCopy = NULL;

    if (relationship == MESSAGE_REQUEST ||
        relationship == MESSAGE_PROVISIONAL ||
        relationship == MESSAGE_FINAL ||
        relationship == MESSAGE_CANCEL ||
        relationship == MESSAGE_CANCEL_RESPONSE ||
        relationship == MESSAGE_ACK ||
        relationship == MESSAGE_2XX_ACK ||
        relationship == MESSAGE_2XX_ACK_PROXY)      // can treat same as any ACK here
    {
        // Make a copy to attach to the transaction
        transactionMessageCopy = new SipMessage(message);

        // Need to add the message to the transaction before it
        // is sent to avoid the race of receiving the response before
        // the request is added to the transaction.
#ifdef TEST_PRINT
        Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                      "SipTransaction::doFirstSend "
                      "calling addResp "
                      " Tx %p has relationship is %s",
                      this, relationshipString(relationship));
#endif
        addResponse(transactionMessageCopy,
                    TRUE, // outgoing
                    relationship);
    }

    // Save the transaction in the message to make
    // it easier to find the transaction on timeout or
    // transport error, e.g. connect failure, far-end closed
    message.setTransaction(this);

    if (toProtocol == OsSocket::TCP)
    {
       sendSucceeded = userAgent.sendTcp(&message,
                                         toAddress.data(),
                                         port);
    }
    else if (toProtocol == OsSocket::SSL_SOCKET)
    {
       sendSucceeded = userAgent.sendTls(&message,
                                         toAddress.data(),
                                         port);
    }
    else
    {
       sendSucceeded = userAgent.sendUdp(&message,
                                         toAddress.data(),
                                         port);
    }

    if(   MESSAGE_REQUEST == relationship
       && !sendSucceeded
       )
    {
        mTransactionState = TRANSACTION_TERMINATED;
    }

#   ifdef TEST_PRINT
    message.dumpTimeLog();
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                  "SipTransaction::doFirstSend "
                  "set Scheduling & resend data");
#   endif

    // Increment the sent counter after the send so the logging messages are acurate.
    message.incrementTimesSent();
    if(transactionMessageCopy) transactionMessageCopy->incrementTimesSent();

    if(sendSucceeded)
    {
        // Schedule a resend timeout for requests and final INVITE failure
        // responses (2xx-class INVITE responses will be resent
        // by user agents only)
        if(   (   ! isResponse
               && strcmp(method.data(), SIP_ACK_METHOD) != 0
               )
           || (   isResponse
               && (   responseCode >= SIP_3XX_CLASS_CODE
                   || (mIsUaTransaction && responseCode >= SIP_OK_CODE)
                   )
               && strcmp(seqMethod.data(), SIP_INVITE_METHOD) == 0
               )
           )
        {
#           ifdef TEST_PRINT
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                          "SipTransaction::doFirstSend "
                          "Scheduling UDP %s timeout in %d msec",
                          method.data(),
                          userAgent.getUnreliableTransportTimeout());
#           endif

            if(transactionMessageCopy) transactionMessageCopy->setTransaction(this);

            // Make a separate copy for the SIP message resend timer
            SipMessageEvent* resendEvent =
                new SipMessageEvent(new SipMessage(message),
                                    SipMessageEvent::TRANSACTION_RESEND);
#           ifdef TEST_PRINT
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                          "SipTransaction::doFirstSend "
                          "timer scheduled for: %p",
                          resendEvent->getMessage());
#           endif

            // Set an event timer to resend the message.
            // When it fires, queue a message to the SipUserAgent.
            OsMsgQ* incomingQ = userAgent.getMessageQueue();
            OsTimer* timer = new OsTimer(incomingQ, resendEvent);
            mTimers.append(timer);
            // Set the resend timer based on resendInterval.
            OsTime timerTime(0, resendInterval * 1000);
            timer->oneshotAfter(timerTime);
#ifdef TEST_PRINT
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                          "SipTransaction::doFirstSend "
                          "added timer %p to timer list, resend time = %f secs",
                          timer, resendInterval / 1000.0);
#endif

            // If this is a client transaction and we are sending
            // a request, set an expires timer for the transaction
            if(!mIsServerTransaction &&
               !isResponse)
            {
                // Time (sec) after which to expire the transaction.
                // Start with the mExpires value of this SipTransaction object.
                int expireSeconds = mExpires;
                // The upper limit is the user agent's mDefaultExpiresSeconds.
                int maxExpires = userAgent.getDefaultExpiresSeconds();
                // We cancel DNS SRV children after the configured DNS SRV timeout.
                // The timeout is ignored if we receive any response.
                // If this is the only child, do not set a short (DNS SRV) timeout
                if(mIsDnsSrvChild &&
                   mpParentTransaction &&
                   mpParentTransaction->isChildSerial())
                {
                    expireSeconds = userAgent.getDnsSrvTimeout();
                }
                // Normal client transaction
                else if(expireSeconds <= 0)
                {
                    if(mpParentTransaction &&
                        mpParentTransaction->isChildSerial())
                    {
                        // Transactions that fork serially get a different default expiration.
                        expireSeconds = userAgent.getDefaultSerialExpiresSeconds();
                    }
                    else
                    {
                        expireSeconds = maxExpires;
                    }
                }

                // Make sure the expiration is not longer than
                // the maximum length of time we keep a transaction around
                if(expireSeconds > maxExpires)
                {
                    expireSeconds = maxExpires;
                }

                // Make a separate copy of the message for the transaction expires timer.
                SipMessageEvent* expiresEvent =
                    new SipMessageEvent(new SipMessage(message),
                                        SipMessageEvent::TRANSACTION_EXPIRATION);

                OsTimer* expiresTimer = new OsTimer(incomingQ, expiresEvent);
                mTimers.append(expiresTimer);
#ifdef TEST_PRINT
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                              "SipTransaction::doFirstSend "
                              "added timer %p to timer list, expire time = %d secs",
                              expiresTimer, expireSeconds);
#endif

                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                              "SipTransaction::doFirstSend"
                              " transaction %p setting timeout %d secs.",
                              this, expireSeconds
                              );

                OsTime expiresTime(expireSeconds, 0);
                expiresTimer->oneshotAfter(expiresTime);
            }
        }
    }

    return(sendSucceeded);
} // end doFirstSend

void SipTransaction::handleResendEvent(const SipMessage& outgoingMessage,
                                       SipUserAgent& userAgent,
                                       enum messageRelationship relationship,
                                       SipTransactionList& transactionList,
                                       int& nextTimeout,
                                       SipMessage*& delayedDispatchedMessage)
{
#ifdef TEST_PRINT
   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                 "SipTransaction::handleResendEvent %p", this);
#endif
    if (delayedDispatchedMessage)
    {
        Os::Logger::instance().log(FAC_SIP, PRI_WARNING, 
                      "SipTransaction::handleResendEvent"
                      " %p delayedDispatchedMessage is not NULL", this);
        delayedDispatchedMessage = NULL;
    }

    // nextTimeout is in msec.
    nextTimeout = 0;

    // If this is not a duplicate, then there is something worng
    if (relationship != MESSAGE_DUPLICATE 
        && relationship != MESSAGE_CANCEL)
    {
        Os::Logger::instance().log(FAC_SIP, PRI_WARNING, 
                      "SipTransaction::handleResendEvent"
                      " %p timeout message is not duplicate: %s",
                      this, relationshipString(relationship));
    }

    // Responses
    if (outgoingMessage.isResponse())
    {
        // The only responses which should have a timeout set are INVITE responses 
        // (UA server transactions only)

        if (mpLastFinalResponse == NULL)
        {
            Os::Logger::instance().log(FAC_SIP, PRI_ERR, 
                          "SipTransaction::handleResendEvent"
                          " response timeout with no response");
        }

        // We should get here if the ACK has not yet been received AND:
        // this is a user agent server transaction 
        // this is a proxy transaction with an error final responses (i.e. !mIsUaTransaction)
        if (/*mIsUaTransaction && // vs. proxy */
            mIsServerTransaction  // vs. client
            && mpAck == NULL 
            && mpLastFinalResponse)
        {
            // We have not yet received the ACK

            // Use mpLastFinalResponse, not outgoingMessage since mpLastFinalResponse may be a newer final response.
            // outgoingMessage is a snapshot that was taken when the timer was set.
            UtlBoolean sentOk = doResend(*mpLastFinalResponse, userAgent, nextTimeout);
            // doResend() sets nextTimeout.

            if(sentOk)
            {
                // Schedule the next timeout
                // As this is a resend, we should be able to use the same copy of the SIP message for the next timeout
#ifdef TEST_PRINT
                if(outgoingMessage.getSipTransaction() == NULL)
                {
                    UtlString msgString;
                    ssize_t msgLen;
                    outgoingMessage.getBytes(&msgString, &msgLen);
                    Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                                  "SipTransaction::handleResendEvent "
                                  "reschedule of response resend with NULL transaction, message = '%s'",
                                  msgString.data());
                }
#endif

                // Schedule a timeout for requests which do not receive a response
                SipMessageEvent* resendEvent = new SipMessageEvent(new SipMessage(outgoingMessage),
                                                                   SipMessageEvent::TRANSACTION_RESEND);

                OsMsgQ* incomingQ = userAgent.getMessageQueue();
                OsTimer* timer = new OsTimer(incomingQ, resendEvent);
                mTimers.append(timer);
#ifdef TEST_PRINT
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                              "SipTransaction::handleResendEvent "
                              "added timer %p to timer list, resend resp time = %f secs",
                              timer, nextTimeout / 1000.0);
#endif

                // Convert from msecs to usecs.
                OsTime lapseTime(0, nextTimeout * 1000);
                timer->oneshotAfter(lapseTime);
            }
            else // doResend failed
            {
                if( MESSAGE_REQUEST == relationship )
                {
                    // TBD-KME not sure how it can get here, code path is if (is a response).. if (is a request)....
                    mTransactionState = TRANSACTION_TERMINATED;
                }

                // Do this outside so that we do not get blocked on locking or delete the transaction out from under ouselves
                // Cleanup the message
                //delete outgoingMessage;
                //outgoingMessage = NULL;
                //userAgent.dispatch(outgoingMessage,
                //                   SipMessageEvent::TRANSPORT_ERROR);
            }
        } // end handling legal response timeout

        // The ACK was received so we can quit
        // We should get here if the ACK has been received AND:
        // this is a user agent server transaction 
        // this is a proxy transaction with an error final responses (i.e. !mIsUaTransaction)
        else if (/* do we care if proxy or UA?? mIsUaTransaction && // vs. proxy */
            mIsServerTransaction // vs. client
            && mpAck 
            && mpLastFinalResponse)
        {
            nextTimeout = -1;
        }

    }

    // Requests
    else
    {
        // This should never be the case
        if(outgoingMessage.isFirstSend())
        {
            Os::Logger::instance().log(FAC_SIP, PRI_WARNING, 
                          "SipTransaction::handleResendEvent"
                          " %p called for first time send of message", this);
        }
         else if (!mIsCanceled 
                  && mpLastFinalResponse == NULL
                  && mpLastProvisionalResponse == NULL
                  && mTransactionState == TRANSACTION_CALLING)
        {
            UtlString method;
            outgoingMessage.getRequestMethod(&method);

            // This is a resend, retrieve the address and port to send the message to.
#ifdef TEST_PRINT
            UtlString toAddress;
            int port;
            outgoingMessage.getSendAddress(&toAddress, &port);
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                          "SipTransaction::handleResendEvent "
                          "Resend request %s:%d",
                          toAddress.data(), port);
#endif

            SipMessage* resendMessage = NULL;
            if(method.compareTo(SIP_ACK_METHOD) == 0)
            {
                Os::Logger::instance().log(FAC_SIP, PRI_WARNING, 
                              "SipTransaction::handleResendEvent"
                              " resend of ACK");
                resendMessage = mpAck;
            }
            else if(method.compareTo(SIP_CANCEL_METHOD) == 0)
            {
                resendMessage = mpCancel;
            }
            else
            {
                resendMessage = mpRequest;
            }
            UtlBoolean sentOk = doResend(*resendMessage,
                                         userAgent, nextTimeout);
            // doResend() sets nextTimeout.

            if(sentOk && nextTimeout > 0)
            {
                // Schedule the next timeout
#ifdef TEST_PRINT
                if(outgoingMessage.getSipTransaction() == NULL)
                {
                    UtlString msgString;
                    ssize_t msgLen;
                    outgoingMessage.getBytes(&msgString, &msgLen);
                    Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                        "SipTransaction::handleResendEvent "
                        "reschedule of request resend with NULL transaction, message = '%s'",
                        msgString.data());
                }
#endif
                // As this is a resend, we should be able to use the same copy of the SIP message for the next timeout

                // Schedule a timeout for requests which do not receive a response
                SipMessageEvent* resendEvent = new SipMessageEvent(new SipMessage(outgoingMessage),
                                                                   SipMessageEvent::TRANSACTION_RESEND);

                OsMsgQ* incomingQ = userAgent.getMessageQueue();
                OsTimer* timer = new OsTimer(incomingQ, resendEvent);
                mTimers.append(timer);
#ifdef TEST_PRINT
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                              "SipTransaction::handleResendEvent "
                              "added timer %p to timer list, resend request time = %f secs",
                              timer, nextTimeout / 1000.0);
#endif

                // Convert from msecs to usecs.
                OsTime lapseTime(0, nextTimeout * 1000);
                timer->oneshotAfter(lapseTime);
            }
            else
            {
                // Do this outside so that we do not get blocked
                // on locking or delete the transaction out
                // from under ouselves
                // Cleanup the message
                //delete outgoingMessage;
                //outgoingMessage = NULL;
                //userAgent.dispatch(outgoingMessage,
                //                   SipMessageEvent::TRANSPORT_ERROR);
#ifdef TEST_PRINT
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                              "SipTransaction::handleResendEvent "
                              "sentOk: %d nextTimeout: %d",
                              sentOk, nextTimeout);
#endif
                if(!sentOk)
                {
                    if ( MESSAGE_REQUEST == relationship )
                    {
                        mTransactionState = TRANSACTION_TERMINATED;
                    }
                    else
                    {
                        mTransactionState = TRANSACTION_COMPLETE;
                    }
                }
                else if ( nextTimeout < 0 )
                {
                    mTransactionState = TRANSACTION_COMPLETE;
#ifdef TEST_PRINT
                    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                                  "SipTransaction::handleResendEvent "
                                  "failed to send request, TRANSACTION_COMPLETE");
#endif
                }
                // else nextTimeout == 0, which should mean the
                // final response was received

            }
        }
        else
        {
            // We are all done, do not reschedule and do not send transport error
            nextTimeout = -1;
            if(mTransactionState == TRANSACTION_CALLING)
            {
                    mTransactionState = TRANSACTION_COMPLETE;
            }
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, 
                          "SipTransaction::handleResendEvent"
                          " no response, TRANSACTION_COMPLETE");
        }
    }

    if(mpParentTransaction)
    {
        mpParentTransaction->handleChildTimeoutEvent(*this,
                                                     outgoingMessage,
                                                     userAgent,
                                                     relationship,
                                                     transactionList,
                                                     nextTimeout,
                                                     delayedDispatchedMessage);
    }

    touch();
} // end handleResendEvent

void SipTransaction::handleExpiresEvent(const SipMessage& outgoingMessage,
                                        SipUserAgent& userAgent,
                                        enum messageRelationship relationship,
                                        SipTransactionList& transactionList,
                                        int& nextTimeout,
                                        SipMessage*& delayedDispatchedMessage,
                                        bool extendable)
{
#ifdef TEST_PRINT
   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                 "SipTransaction::handleExpiresEvent -1 %p", this);
#endif

    if (delayedDispatchedMessage)
    {
#      ifdef TEST_PRINT
       Os::Logger::instance().log(FAC_SIP, PRI_WARNING, 
                     "SipTransaction::handleExpiresEvent"
                      " delayedDispatchedMessage not NULL");
#      endif

       delayedDispatchedMessage = NULL;
    }

    // Responses
    if (outgoingMessage.isResponse())
    {
#       ifdef TEST_PRINT
        Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                      "SipTransaction::handleExpiresEvent"
                      " %p expires event timed out on SIP response", 
                      this);
#       endif
    }

    // Requests
    else if (mProvoExtendsTimer && extendable)
    {
        // Extends(creates new) expire timer if appropriate provisional response has come in since 
        // the previous time-out.
        mProvoExtendsTimer = FALSE;

        // set new timer UNLESS mIsDnsSrvChild would cause the timeout event to be ignored anyway.
        if (!mIsDnsSrvChild)
        {
            // Keep separate message copy for the timer
            SipMessage* pRequestMessage = NULL;
            pRequestMessage = new SipMessage(*mpRequest);

            SipMessageEvent* expiresEvent =
                new SipMessageEvent(pRequestMessage,
                                    SipMessageEvent::TRANSACTION_EXPIRATION_TIMER_C);

            OsMsgQ* incomingQ = userAgent.getMessageQueue();
            OsTimer* expiresTimer = new OsTimer(incomingQ, expiresEvent);
            mTimers.append(expiresTimer);

            // This must be a Timer C expiration, and it is always
            // userAgent.getDefaultExpiresSeconds().
            int expireSeconds = userAgent.getDefaultExpiresSeconds();
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, 
                          "SipTransaction::handleExpiresEvent"
                          " provoExtendsTimer - transaction %p setting timeout %d secs.",
                          this, expireSeconds);
#ifdef TEST_PRINT
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                          "SipTransaction::handleExpiresEvent "
                          "provoExtendsTimer - Tx %p"
                          "add timer %p to timer list, expire time = %d secs ",
                          this, expiresTimer, expireSeconds);
#endif
            OsTime expiresTime(expireSeconds, 0);
            expiresTimer->oneshotAfter(expiresTime);
        }
    }
    else
    {
        // When should we send CANCEL?
#       ifdef TEST_PRINT
        Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                      "SipTransaction::handleExpiresEvent -2 %p",
                      this);
#       endif
        // Do not cancel a DNS child that received any response.
        // The parent client transaction may later be canceled which will recursively cancel the children.  
        if (mIsDnsSrvChild
            && (mpLastProvisionalResponse || mpLastFinalResponse))
        {
            // no-op
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                          "SipTransaction::handleExpiresEvent "
                          "%p ignoring timeout cancel of DNS SRV child "
                          "because '%s' '%s' '%s' ", 
                          this, 
                          (mpLastProvisionalResponse? "got provo resp ":""),
                          ((mpLastProvisionalResponse && mpLastFinalResponse)? "and ":""),
                          (mpLastFinalResponse? "got final resp ":""));
        }

        // Do not cancel an early dialog with early media if this transaction is a child to a serial search/fork.
        // This may be a gateway sending IVR prompts ala American Airlines, so we do not want to cancel in
        // the middle of the user entering DTMF
        else if (   !mIsDnsSrvChild
                 && !mIsServerTransaction
                 && mpParentTransaction
                 && mpParentTransaction->isChildSerial()
                 && mRequestMethod.compareTo(SIP_INVITE_METHOD) == 0
                 && isChildEarlyDialogWithMedia())
        {
            // no op
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                          "SipTransaction::handleExpiresEvent"
                          " %p ignoring timeout cancel of early media branch of serial search",
                          this);
        }

        // Only cancel transaction which are still doing something [Do not cancel a completed transaction]
        else if ((mIsRecursing                                  // somewhere else in the tree is doing something
                  || mTransactionState == TRANSACTION_CALLING
                  || mTransactionState == TRANSACTION_PROCEEDING
                  || mTransactionState == TRANSACTION_LOCALLY_INIITATED))
        {
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                          "SipTransaction::handleExpiresEvent"
                          " %p canceling expired transaction",
                          this);

            // This transaction has expired; cancel it.
            cancel(userAgent, transactionList);

        }

        // Check the parents in the heirarchy to see if there are other branches to pursue
        if (mpParentTransaction)
        {
            mpParentTransaction->handleChildTimeoutEvent(*this,
                                                         outgoingMessage,
                                                         userAgent,
                                                         relationship,
                                                         transactionList,
                                                         nextTimeout,
                                                         delayedDispatchedMessage);
        }
        // This is the top most parent and it is a client transaction, we need to find the best result
        else if (!mIsServerTransaction)
        {
            handleChildTimeoutEvent(*this,
                                     outgoingMessage,
                                     userAgent,
                                     relationship,
                                     transactionList,
                                     nextTimeout,
                                     delayedDispatchedMessage);
        }

        touch();
    }
}

UtlBoolean SipTransaction::handleChildIncoming(//SipTransaction& child,
                                     SipMessage& incomingMessage,
                                     SipUserAgent& userAgent,
                                     enum messageRelationship relationship,
                                     SipTransactionList& transactionList,
                                     UtlBoolean childSaysShouldDispatch,
                                     SipMessage*& delayedDispatchedMessage)
{
    UtlBoolean shouldDispatch = childSaysShouldDispatch;

    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                  "SipTransaction::handleChildIncoming "
                  "%p relationship %s parent %p",
                  this, relationshipString(relationship), mpParentTransaction
                  );

    if(   relationship == MESSAGE_FINAL
       || relationship == MESSAGE_PROVISIONAL
       )
    {
        int responseCode = incomingMessage.getResponseStatusCode();

        // If there is a parent pass it up
        if(mpParentTransaction)
        {

#           ifdef TEST_PRINT
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                          "SipTransaction::handleChildIncoming "
                          "%p provoExtends this=>%d parent=>%d",
                          this, mProvoExtendsTimer, 
                          mpParentTransaction->mProvoExtendsTimer );
#           endif
            mpParentTransaction->mProvoExtendsTimer = mProvoExtendsTimer;

            // May want to short cut this and first get
            // the top most parent.  However if the state
            // change is interesting to intermediate (i.e.
            // not top most parent) transactions we need to
            // do it this way (recursively)
            shouldDispatch =
                mpParentTransaction->handleChildIncoming(//child,
                                            incomingMessage,
                                            userAgent,
                                            relationship,
                                            transactionList,
                                            childSaysShouldDispatch,
                                            delayedDispatchedMessage);
#           ifdef TEST_PRINT
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                          "SipTransaction::handleChildIncoming "
                          "%p parent says %d",
                          this, shouldDispatch);
#           endif
        }
        else // this is the topmost parent transaction
        {
            // We do not dispatch if this is a server transaction
            // as the server transaction is the consumer of the
            // message.  If there is no server transaction as the
            // top most parent, then we assume the consumer is a
            // local application (i.e. message queue).
            if(mIsServerTransaction)
            {
               // If responseCode > 100 && <= 299
               if (   (responseCode >  SIP_TRYING_CODE)
                   && (responseCode <  SIP_3XX_CLASS_CODE)
                   )
               {
                  Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                                "SipTransaction::handleChildIncoming %p "
                                "topmost parent dispatching %d",
                                this, responseCode );
                  shouldDispatch = TRUE;
               }
               else
               {
#                 ifdef DISPATCH_DEBUG
                  Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                                "SipTransaction::handleChildIncoming %p "
                                "topmost parent NOT dispatching %d.",
                                this, responseCode );
#                 endif
                  shouldDispatch = FALSE;
               }
            }

            // CANCEL is hop by hop and should not be dispatched unless
            // the parent transaction was the originator of the CANCEL
            // request
            else if(!mIsCanceled)
            {
                int tempCseq;
                UtlString method;
                incomingMessage.getCSeqField(&tempCseq, &method);
                if(method.compareTo(SIP_CANCEL_METHOD) == 0)
                {
                    shouldDispatch = FALSE;
                }
            }
        }   // end top-most parent

#       ifdef TEST_PRINT
        Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                      "SipTransaction::handleChildIncoming "
                      "%p check response %d",
                      this, responseCode);
#       endif

        if(responseCode < SIP_TRYING_CODE)
        {
            // What is this????
            Os::Logger::instance().log(FAC_SIP, PRI_ERR,
                          "SipTransaction::handleChildIncoming"
                          " dropped invalid response code: %d",
                          responseCode);
        }

        // 100 Trying is hop by hop do not forward it
        else if(responseCode == SIP_TRYING_CODE)
        {
            // no op
        }

        // If this is a successful 2XX or provisional response
        // forward it immediately
        // Once a final response is sent we no longer
        // send provisional responses, but we still send 2XX
        // class responses
        else if(   responseCode < SIP_3XX_CLASS_CODE
                && (   mpLastFinalResponse == NULL
                    || responseCode >= SIP_2XX_CLASS_CODE
                    )
                )
        {
            // If this is a server transaction for which the
            // response must be forwarded upstream
            if(mIsServerTransaction)
            {
                // Forward immediately
                SipMessage response(incomingMessage);
                response.removeTopVia();
                response.resetTransport();
                response.clearDNSField();
#ifdef TEST_PRINT
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                              "SipTransaction::handleChildIncoming "
                              "immediately forwarding %d response",
                              responseCode);
#endif
                handleOutgoing(response,
                                userAgent,
                                transactionList,
                                relationship);
            }

            // If we got a good response for which forking
            // or recursion should be canceled
            if(mpParentTransaction == NULL)
            {
                // Got a success
                if(responseCode >= SIP_2XX_CLASS_CODE)
                {
                    // Set the Reason data to send in CANCEL
                    setCancelReasonValue("SIP",
                                         responseCode,
                                         SIP_REASON_CALL_ANSWERED_ELSEWHERE);

                    // We are done - cancel all the outstanding requests
                    cancelChildren(userAgent,
                                   transactionList);
                }
            }

            // Keep track of the fact that we dispatched a final
            // response (but not for 1xx responses where xx > 00)
            if(   shouldDispatch
               && responseCode >= SIP_2XX_CLASS_CODE
               )
            {
#               ifdef TEST_PRINT
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                              "SipTransaction::handleChildIncoming %p"
                              " should dispatch final response %d",
                              this, __LINE__);
#               endif
                mDispatchedFinalResponse = TRUE;
            }

            // This should not occur.  All 2xx class messages for which
            // there is no parent server transaction should get dispatched
            else if(   mpParentTransaction == NULL
                    && responseCode >= SIP_2XX_CLASS_CODE
                    )
            {
               // xmlscott: despite the comment above,
               //           this happens a lot and seems to not always be bad,
               //           so I'm changing the priority to get it out of the logs.
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                              "SipTransaction::handleChildIncoming "
                              "%d response with parent client transaction NOT dispatched",
                              responseCode);
            }
        }       // end server response forwarding
        else
        {
            // 3XX class responses
            if(responseCode <= SIP_4XX_CLASS_CODE)
            {
                // Recursion is handled by the child
#               ifdef TEST_PRINT
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                              "SipTransaction::handleChildIncoming %p"
                              " 3XX response - should not dispatch",
                              this);
#               endif

                // Wait until all the children have been searched
                // before dispatching
                shouldDispatch = FALSE;
            }
            // 4XX, 5XX, and 6XX class responses
            // (See previous version for code that causes 6XX responses
            // to cancel all outstanding forks.)
            else
            {
                // See if there are other outstanding child transactions

                // If there are none and no more to recurse find the
                // best result

                // If there are more to recurse do so.

                // Wait until all the children have been searched
                // before dispatching
                shouldDispatch = FALSE;
            }

#           ifdef TEST_PRINT
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                          "SipTransaction::handleChildIncoming %p"
                          " response=%d parent=%p final=%p dispatched=%d",
                          this, responseCode, mpParentTransaction,
                          mpLastFinalResponse, mDispatchedFinalResponse
               );
#           endif

            // If this is the server transaction and we have not
            // yet sent back a final response check what is next
            if(   (   mIsServerTransaction
                   && mpLastFinalResponse == NULL
                   )
               // or if this is the parent client transaction on a UA
               // and we have not yet dispatched the final response
               || (   mpParentTransaction == NULL
                   && ! mIsServerTransaction
                   && mpLastFinalResponse == NULL
                   && ! mDispatchedFinalResponse
                   )
               )
            {
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, 
                              "SipTransaction::handleChildIncoming %p", this );

                if(mpParentTransaction)
                {
                    Os::Logger::instance().log(FAC_SIP, PRI_ERR,
                                  "SipTransaction::handleChildIncoming %p "
                                  "server transaction is not top most parent", this);
                }

                // See if there is anything to sequentially search
                // startSequentialSearch returns TRUE if something
                // is still searching or it starts the next sequential
                // search
                if(startSequentialSearch(userAgent, transactionList))
                {
                }

                // Special case for when there is no server transaction
                // The parent client transaction, when it first gets a
                // 3xx response has no children, so we need to create them
                else if(   mChildTransactions.isEmpty()
                        && recurseChildren(userAgent, transactionList) // true if something started
                        )
                {
#                   ifdef TEST_PRINT
                    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, 
                                  "SipTransaction::handleChildIncoming "
                                  "%p creating children for 3XX",
                                  this);
#                   endif
                }

                // Not waiting for outstanding transactions to complete
                else
                {
                    SipMessage bestResponse;
                    if(findBestResponse(bestResponse))
                    {
#                       ifdef TEST_PRINT
                        Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                                      "SipTransaction::handleChildIncoming"
                                      " %p sending best response",
                                      this);
#                       endif

                        if (Os::Logger::instance().willLog(FAC_SIP, PRI_DEBUG))
                        {
                           int bestResponseCode = bestResponse.getResponseStatusCode();
                           UtlString callId;
                           bestResponse.getCallIdField(&callId);
                           Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                                         // Format the Call-Id so it looks like
                                         // a header line in a SIP message,
                                         // so log processors see it.
                                         "SipTransaction::handleChildIncoming "
                                         "response %d for Call-Id '%s'",
                                         bestResponseCode, callId.data());
                        }

                        if(mIsServerTransaction)
                        {
#ifdef TEST_PRINT
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                              "SipTransaction::handleChildIncoming "
                              "outgoing call 1");
#endif
                            handleOutgoing(bestResponse,
                                            userAgent,
                                            transactionList,
                                            MESSAGE_FINAL);
                        }

                        if(!mDispatchedFinalResponse)
                        {
                            // Get the best message out to be dispatched.
                            if(delayedDispatchedMessage)
                            {
                                delete delayedDispatchedMessage;
                                delayedDispatchedMessage = NULL;
                            }
                            delayedDispatchedMessage =
                                new SipMessage(bestResponse);

#                           ifdef DISPATCH_DEBUG
                            int delayedResponseCode =
                               delayedDispatchedMessage->getResponseStatusCode();
                            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                                          "SipTransaction::handleChildIncoming %p "
                                          "delayed dispatch of %d\n",
                                          this, delayedResponseCode );
#                           endif
#                           ifdef TEST_PRINT
                            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                                          "SipTransaction::handleChildIncoming %p "
                                          "should dispatch delayed message %d",
                                          this, __LINE__);
#                           endif
                            mDispatchedFinalResponse = TRUE;
                        }
                    }
                }
            }
        }       // end 3xx response

        // The response itself is getting dispatched
        if(   shouldDispatch
           && responseCode >= SIP_2XX_CLASS_CODE
           )
        {
            // Keep track of the fact that we dispatched a final
            // response
            mDispatchedFinalResponse = TRUE;
#           ifdef TEST_PRINT
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                          "SipTransaction::handleChildIncoming "
                          "%p should dispatch final response %d",
                          this, __LINE__);
#           endif

            if(delayedDispatchedMessage)
            {
                // This is probably a bug.  This should not
                // occur.  For now log some noise and
                // drop the delayed response, if this ever
                // occurs
                // xmlscott: lowered priority
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                              "SipTransaction::handleChildIncoming"
                              " %p dropping delayed response", this);
                delete delayedDispatchedMessage;
                delayedDispatchedMessage = NULL;
            }
        }
    }  // end new responses

    else if(relationship == MESSAGE_DUPLICATE)
    {
        // Proxy client transaction received a duplicate INVITE
        // response
        if(incomingMessage.isResponse() &&
           //mIsUaTransaction &&
           mRequestMethod.compareTo(SIP_INVITE_METHOD) == 0)
        {
            int responseCode = incomingMessage.getResponseStatusCode();

            // The proxy must resend duplicate 2xx class responses
            // for reliability
            if(responseCode >= SIP_2XX_CLASS_CODE &&
                responseCode < SIP_3XX_CLASS_CODE)
            {
                // If there is more than one Via, send it upstream.
                // The calling UAC should resend the ACK, not the
                // proxy.
                UtlString dummyVia;
                if(incomingMessage.getViaField(&dummyVia, 1))
                {
                    SipTransaction* parent = getTopMostParent();
                    if(parent &&
                       parent->mIsServerTransaction)
                    {
                         Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                                       "SipTransaction::handleChildIncoming "
                                       "proxy resending server transaction response %d",
                                       responseCode);
                        userAgent.sendStatelessResponse(incomingMessage);
                    }
                }

                // The ACK originated here, resend it
                else
                {
                    if(mpAck)
                    {
                        // Resend the ACK
                        SipMessage ackCopy(*mpAck);
                        ackCopy.removeTopVia();
                        userAgent.sendStatelessRequest(ackCopy,
                                                       mSendToAddress,
                                                       mSendToPort,
                                                       mSendToProtocol,
                                                       mpBranchId->utlString());
                    }

                    // If this is a duplicate 2xx response and there is only
                    // one Via on the reponse, this UA should be the caller UAC.
                    // We should have an ACK that was sent for the original 2xx
                    // response.
                    else
                    {
                        Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                                      "SipTransaction::handleChildIncoming "
                                      "duplicate 2xx response received on UAC for INVITE "
                                      "with no ACK"
                                      );
                    }

                }

            }

            // INVITE with final response that failed
            else if(responseCode >= SIP_3XX_CLASS_CODE)
            {
                // For failed INVITE transactions, the ACK is
                // sent hop-by-hop
                if(mpAck)
                {
                    // Resend the ACK
                    SipMessage ackCopy(*mpAck);
                    ackCopy.removeTopVia();
                    userAgent.sendStatelessRequest(ackCopy,
                                                   mSendToAddress,
                                                   mSendToPort,
                                                   mSendToProtocol,
                                                   mpBranchId->utlString());
                }

                // No ACK for a duplicate failed response.  Something
                // is wrong.  The ACK should have been created locally
                // for the previous 3xx, 4xx, 5xx or 6xx final response
                else
                {
                    Os::Logger::instance().log(FAC_SIP, PRI_CRIT,
                                  "SipTransaction::handleChildIncoming "
                                  "duplicate final error response rcvd for INVITE with no ACK");
                }
            }
        }
    }

    return(shouldDispatch);
} // end handleChildIncoming

void SipTransaction::handleChildTimeoutEvent(SipTransaction& child,
                                    const SipMessage& outgoingMessage,
                                    SipUserAgent& userAgent,
                                    enum messageRelationship relationship,
                                    SipTransactionList& transactionList,
                                    int& nextTimeout,
                                    SipMessage*& delayedDispatchedMessage)
{
    if(mpParentTransaction)
    {
        // For now recurse.  We might be able to short cut this
        // and go straight to the top-most parent
        mpParentTransaction->handleChildTimeoutEvent(child,
                                                     outgoingMessage,
                                                     userAgent,
                                                     relationship,
                                                     transactionList,
                                                     nextTimeout,
                                                     delayedDispatchedMessage);
    }

    // Top most parent
    else
    {
#       ifdef LOG_FORKING
        Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, 
                      "SipTransaction::handleChildTimeoutEvent"
                      " found top most parent: %p", this);
#       endif
        {
            UtlBoolean isResponse = outgoingMessage.isResponse();
            UtlString method;
            outgoingMessage.getRequestMethod(&method);

            if(   ! isResponse
               && method.compareTo(SIP_ACK_METHOD) == 0
               )
            {
#               ifdef LOG_FORKING
                Os::Logger::instance().log(FAC_SIP, PRI_ERR, 
                              "SipTransaction::handleChildTimeoutEvent"
                              " timeout of ACK");
#               endif
            }

            else if(   ! isResponse
                    && method.compareTo(SIP_CANCEL_METHOD) == 0
                    )
            {
            }

            else if(   relationship == MESSAGE_DUPLICATE
                    && ! isResponse
                    )
            {
                // Check if we should still be trying
                if(nextTimeout > 0)
                {
                    // Still trying
                }
                // This transaction is done or has given up
                // See if we should start the next sequential search
                else
                {
                    // We do not dispatch proxy transactions
                    nextTimeout = -1;

#                   ifdef LOG_FORKING
                    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, 
                                  "SipTransaction::handleChildTimeoutEvent"
                                  " %p", this);
#                   endif

                    if(startSequentialSearch(userAgent, transactionList))
                    {
                        // TRUE if still have destinations to try
#                       ifdef LOG_FORKING
                        Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                                      "SipTransaction::handleChildTimeoutEvent "
                                      "%p starting/still searching", this);
#                       endif
                    }

                    // Not waiting for outstanding transactions to complete
                    // and we have not yet sent a final response
                    else if(mpLastFinalResponse == NULL)
                    {
                        SipMessage bestResponse;
                        UtlBoolean foundBestResponse = findBestResponse(bestResponse);
                        // 2XX class responses are sent immediately so we
                        //  should not send it again
                        int bestResponseCode = bestResponse.getResponseStatusCode();
                        if (Os::Logger::instance().willLog(FAC_SIP, PRI_DEBUG))
                        {
                           UtlString callId;
                           bestResponse.getCallIdField(&callId);
#                          ifdef LOG_FORKING
                           Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                                         // Format the Call-Id so it looks like
                                         // a header line in a SIP message,
                                         // so log processors see it.
                                         "SipTransaction::handleChildTimeoutEvent "
                                         "response %d for Call-Id '%s'",
                                         bestResponseCode, callId.data());
#                          endif
                        }

                        // There is nothing to send if unless this is a server transaction
                        // (this is the top most parent, for a client transaction, the response gets "dispatched").
                        if(   bestResponseCode >= SIP_3XX_CLASS_CODE
                           && mIsServerTransaction
                           && foundBestResponse
                           )
                        {
                            SipMessage betterResponse;
                            SipMessage& rpUseThisMsg = bestResponse;

#                           ifdef LOG_FORKING
                            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                                          "SipTransaction::handleChildTimeoutEvent "
                                          "%p sending best response", this);
#                           endif

                            // this is a timeout event, choose 408 over 302 or 404
                            if (  bestResponseCode < SIP_4XX_CLASS_CODE
                               || bestResponseCode == SIP_NOT_FOUND_CODE)
                            {
                                bestResponseCode = SIP_REQUEST_TIMEOUT_CODE;
                                betterResponse.setResponseData(mpRequest,
                                                             SIP_REQUEST_TIMEOUT_CODE,
                                                             SIP_REQUEST_TIMEOUT_TEXT);
                                rpUseThisMsg = betterResponse;
                            }

                            if (   (SIP_REQUEST_TIMEOUT_CODE == bestResponseCode)
                                && (!rpUseThisMsg.hasSelfHeader())
                                )
                            {
                               userAgent.setSelfHeader(rpUseThisMsg);
                            }

#ifdef TEST_PRINT
                            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                                          "SipTransaction::handleChildTimeoutEvent "
                                          "outgoing call 2");
#endif
                            // sends best response
                            handleOutgoing(rpUseThisMsg,
                                           userAgent,
                                           transactionList,
                                           MESSAGE_FINAL);
                        }
                        else
                        {
#                           ifdef LOG_FORKING
                            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                                          "SipTransaction::handleChildTimeoutEvent "
                                          "%p not sending %d best response",
                                          this, bestResponseCode);

#                           endif
                        }

                        if(foundBestResponse &&
                            !mDispatchedFinalResponse)
                        {
                            if(delayedDispatchedMessage)
                            {
                                delete delayedDispatchedMessage;
                                delayedDispatchedMessage = NULL;
                            }
                            delayedDispatchedMessage = new SipMessage(bestResponse);
#                           ifdef TEST_PRINT
                            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                                          "SipTransaction::handleChildTimeoutEvent "
                                          "%p should dispatch final response %d",
                                          this, __LINE__);
#                           endif
                            mDispatchedFinalResponse = TRUE;
                        }
                    }   // mpLastFinalResponse was NULL 
                }   // nextTimeout not > 0
            } // Request timeout
        } // server transaction
    }   // topmost parent
}

UtlBoolean SipTransaction::startSequentialSearch(SipUserAgent& userAgent,
                                                 SipTransactionList& transactionList)
{
    UtlSListIterator iterator(mChildTransactions);
    SipTransaction* childTransaction = NULL;
    UtlBoolean childStillProceeding = FALSE;
    UtlBoolean startingNewSearch = FALSE;

#ifdef TEST_PRINT
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                  "SipTransaction::startSequentialSearch %p", this);
#endif

    while ((childTransaction = (SipTransaction*) iterator()))
    {
        if(   ! childTransaction->mIsCanceled
           && (   childTransaction->mTransactionState == TRANSACTION_CALLING
               || childTransaction->mTransactionState == TRANSACTION_PROCEEDING
               )
           )
        {
            // The child is not done
            childStillProceeding = TRUE;
#           ifdef LOG_FORKING
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, 
                          "SipTransaction::startSequentialSearch "
                          "%p child: %p still proceeding",
                          this, childTransaction);
#           endif
        }

        else if( childTransaction->mIsRecursing )
        {
            // See if the grand children or decendants are still searching
            if(childTransaction->startSequentialSearch(userAgent,
                transactionList))
            {
                childStillProceeding = TRUE;
#               ifdef LOG_FORKING
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, 
                              "SipTransaction::startSequentialSearch "
                              "%p child: %p decendent still proceeding",
                              this, childTransaction);
#               endif
            }
            else
            {
#               ifdef LOG_FORKING
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, 
                              "SipTransaction::startSequentialSearch "
                              "%p child: %p no longer proceeding",
                              this, childTransaction);
#               endif
            }
        }

        // A child has completed and may be recursed
        else if(   ! childStillProceeding
                && (   childTransaction->mTransactionState == TRANSACTION_COMPLETE
                    || childTransaction->mTransactionState == TRANSACTION_CONFIRMED
                    )
                && ! mIsCanceled
                && ! childTransaction->mIsCanceled
                )
        {
#           ifdef LOG_FORKING
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, 
                          "SipTransaction::startSequentialSearch "
                          "%p child: %p completed, now recursing",
                          this, childTransaction);
#           endif
            UtlBoolean recurseStartedNewSearch =
                childTransaction->recurseChildren(userAgent,
                                                  transactionList);
            if(!startingNewSearch)
            {
                startingNewSearch = recurseStartedNewSearch;
            }

            // Do not break out of the loop because we want
            // to check all the currently proceeding transaction
            // to see if we should recurse.
#           ifdef LOG_FORKING
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, 
                          "SipTransaction::startSequentialSearch "
                          "%p child: %p startingNewSearch: %s",
                          this, childTransaction, startingNewSearch ? "True" : "False");
#                         endif
        }

        // If there is another sequential search to kick off on this
        // parent
        else if(   ! childStillProceeding
                && ! startingNewSearch
                && childTransaction->mTransactionState == TRANSACTION_LOCALLY_INIITATED
                && ! mIsCanceled
                && ! childTransaction->mIsCanceled
                )
        {
            UtlBoolean recurseStartedNewSearch = FALSE;

            if(mpDnsDestinations)   // tells us DNS lookup was done, check for valid records at lower level
            {
                recurseStartedNewSearch  = recurseDnsSrvChildren(userAgent, transactionList);
            }
            else
            {
                recurseStartedNewSearch = recurseChildren(userAgent, transactionList);
            }

            if(!startingNewSearch)
            {
                startingNewSearch = recurseStartedNewSearch;
            }
#           ifdef LOG_FORKING
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, 
                          "SipTransaction::startSequentialSearch "
                          "%p child: %p starting sequential startingNewSearch: %s",
                          this, childTransaction,
                          startingNewSearch ? "True" : "False");
#           endif
            if( recurseStartedNewSearch )
            {
                break;
            }
            else
            {
#               ifdef LOG_FORKING
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                              "SipTransaction::startSequentialSearch "
                              "%p failed to find child to transaction to pursue", this);
#               endif
            }
        }
    }

    mIsRecursing = childStillProceeding || startingNewSearch;
#   ifdef LOG_FORKING
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, 
                  "SipTransaction::startSequentialSearch -1-"
                  "%p recursing: %s childStillProceeding: %s startingNewSearch:%s",
                  this, mIsRecursing ? "True" : "False",
                  childStillProceeding ? "True" : "False",
                  startingNewSearch ? "True" : "False");
#   endif
    return(mIsRecursing);
}

UtlBoolean SipTransaction::recurseDnsSrvChildren(SipUserAgent& userAgent,
                                                 SipTransactionList& transactionList,
                                                 SipMessage* pRequest)
{
    // If this is a client transaction requiring DNS lookup
    // and we need to create the children to recurse
    if(!mIsServerTransaction &&         // is client transaction
        !mIsDnsSrvChild &&              // is not traversing DNS record tree
        mpDnsDestinations == NULL &&    // the one required DNS lookup has not yet happened
        mpRequest &&                    // only applicable to requests not sent
        mpLastFinalResponse == NULL &&  // should be no response yet
        mChildTransactions.isEmpty())
    {
        if(mSendToAddress.isNull())
        {
            Os::Logger::instance().log(FAC_SIP, PRI_ERR, 
                          "SipTransaction::recurseDnsSrvChildren"
                          " no send address");
        }
        else if(mTransactionState < TRANSACTION_CONFIRMED)
        {
            mTransactionState = TRANSACTION_CONFIRMED;
            UtlString scheme;
            scheme = "sip";

            // Do the DNS lookup for the request destination but first
            // determine whether to force the msg to be sent via TCP.
            OsSocket::IpProtocolSocketType msgSizeProtocol = getPreferredProtocol();

            if (pRequest)
            {
              Url requestUri;
              UtlString requestUriString;
              pRequest->getRequestUri(&requestUriString);
              requestUri.fromString(requestUriString, TRUE /* is a request uri */);

              if (requestUri.getScheme() == Url::SipsUrlScheme)
              {
                scheme = "sips";
                msgSizeProtocol = OsSocket::SSL_SOCKET;
              }
              else
              {
                UtlString transport;
                requestUri.getUrlParameter("transport", transport);
                if (transport == "tls" || transport == "TLS")
                {
                  scheme = "sips";
                  msgSizeProtocol = OsSocket::SSL_SOCKET;
                }

              }
            }

            
            mpDnsDestinations = SipSrvLookup::servers(mSendToAddress.data(),
                                                      scheme.data(), 
                                                      mSendToProtocol,
                                                      mSendToPort,
                                                      msgSizeProtocol);
            
            if(scheme == "sips" && (!mpDnsDestinations || !mpDnsDestinations[0].isValidServerT()))
            {
              Os::Logger::instance().log(FAC_SIP, PRI_ERR,
                          "SipTransaction::recurseDnsSrvChildren"
                          " TLS is not set for host %", mSendToAddress.data());

              //
              // if DNS/SRV lookup of a sips uri failed, default scheme to sip
              //
              scheme = "sip";
              mpDnsDestinations = SipSrvLookup::servers(mSendToAddress.data(),
                                                      scheme.data(),
                                                      mSendToProtocol,
                                                      mSendToPort,
                                                      msgSizeProtocol);
            }

            // HACK:
            // Add a via to this request so when we set a timer it is
            // identified (by branchId) which transaction it is related to
            if(mpRequest)
            {
                // This via should never see the light of day
                // (or rather the bits of the network).
                mpRequest->addVia("127.0.0.1",
                                  9999,
                                  "UNKNOWN",
                                  mpBranchId->data());
            }

            // Save a pointer to this transaction in the stored
            // request in this transaction so that it is carried into
            // the requests that are attached to the timer events.
            // Then when the timer events are being processed, it will
            // be quicker to find the relevant transaction.

            mpRequest->setTransaction(this);

            // Set the transaction expires timeout(s) for the DNS parent (this transaction)

            // We will set one timer (with TRANSACTION_EXPIRATION
            // event) for the ordinary "transaction expiration" timer.
            // If this is an INVITE, we will set another timer (with
            // TRANSACTION_EXPIRATION_TIMER_C event) for the "Timer C"
            // timer.  Since "Timer C" can be extended by receiving
            // 101-199 responses, we need to handle its events separately.

            bool isInvite = mRequestMethod.compareTo(SIP_INVITE_METHOD) == 0;

            // If request is INVITE, start Timer C.
            if (isInvite)
            {
               // Make copy of the request for the timer event.
               SipMessageEvent* expiresEvent =
                  new SipMessageEvent(new SipMessage(*mpRequest),
                                      SipMessageEvent::TRANSACTION_EXPIRATION_TIMER_C);
               OsMsgQ* incomingQ = userAgent.getMessageQueue();
               OsTimer* expiresTimer = new OsTimer(incomingQ, expiresEvent);
               mTimers.append(expiresTimer);

               // Timer C is always userAgent.getDefaultExpiresSeconds().
               int expireSeconds = userAgent.getDefaultExpiresSeconds();
               OsTime expiresTime(expireSeconds, 0);

               // Get everything set up before starting the timer.
               expiresTimer->oneshotAfter(expiresTime);

#ifdef TEST_PRINT
               Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                             "SipTransaction::recurseDnsSrvChildren "
                             "added Timer C timer %p to timer list, expire time = %d secs",
                             expiresTimer, expireSeconds);
#endif

               Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, 
                             "SipTransaction::recurseDnsSrvChildren"
                             "Timer C transaction %p setting timeout %d secs.",
                             this, expireSeconds
               );
            }

            // All requests may have a expiration timer.

            // Basic expiration time is provided by the Expires header, if any.
            // Note that "Expires: 0" is legitimate and causes the transaction
            // to time out immediately.
            int expireSeconds = mExpires;
            // If no Expires, and this is a serial child, use
            // userAgent.getDefaultSerialExpiresSeconds().
            if (   expireSeconds < 0
                && mpParentTransaction
                && mpParentTransaction->isChildSerial())
            {
               expireSeconds = userAgent.getDefaultSerialExpiresSeconds();
            }
            // If not an INVITE transaction (and so does not have Timer C),
            // limit the expiration to userAgent.getSipStateTransactionTimeout()/1000.
            if (!isInvite)
            {
               int maxExpires = userAgent.getSipStateTransactionTimeout()/1000;
               if (expireSeconds < 0 || expireSeconds > maxExpires)
               {
                  expireSeconds = maxExpires;
               }
            }

            // If this results in an expiration time to be enforced, start
            // a timer.
            if (expireSeconds >= 0)
            {
               // Make copy of the request for the timer event.
               SipMessageEvent* expiresEvent =
                  new SipMessageEvent(new SipMessage(*mpRequest),
                                      SipMessageEvent::TRANSACTION_EXPIRATION);
               OsMsgQ* incomingQ = userAgent.getMessageQueue();
               OsTimer* expiresTimer = new OsTimer(incomingQ, expiresEvent);
               mTimers.append(expiresTimer);
               OsTime expiresTime(expireSeconds, 0);

               // Get everything set up before starting the timer.
               expiresTimer->oneshotAfter(expiresTime);

#ifdef TEST_PRINT
               Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                             "SipTransaction::recurseDnsSrvChildren "
                             "added Expire timer %p to timer list, expire time = %d secs",
                             expiresTimer, expireSeconds);
#endif

               Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, 
                             "SipTransaction::recurseDnsSrvChildren"
                             "Expire transaction %p setting timeout %d secs.",
                             this, expireSeconds
               );
            }

            if(mpDnsDestinations && mpDnsDestinations[0].isValidServerT())   // leave redundant check at least for now
            {
                int numSrvRecords = 0;
                int maxSrvRecords = userAgent.getMaxSrvRecords();

                // Create child transactions for each SRV record
                // up to the maximum
                while(numSrvRecords < maxSrvRecords &&
                    mpDnsDestinations[numSrvRecords].isValidServerT())
                {
                    // will not be a server transaction
                    SipTransaction* childTransaction =
                        new SipTransaction(mpRequest,
                                           TRUE, // outgoing
                                           mIsUaTransaction,
                                           (  mpParentTransaction
                                            ? mpParentTransaction->mpBranchId
                                            : NULL )
                                           ); // same as parent

                    mpDnsDestinations[numSrvRecords].
                       getIpAddressFromServerT(childTransaction->mSendToAddress);

                    childTransaction->mSendToPort =
                        mpDnsDestinations[numSrvRecords].getPortFromServerT();

                    childTransaction->mSendToProtocol =
                        mpDnsDestinations[numSrvRecords].getProtocolFromServerT();

#                   ifdef ROUTE_DEBUG
                         {
                            UtlString protoString;
                            SipMessage::convertProtocolEnumToString(childTransaction->mSendToProtocol,
                                                                    protoString);
                            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                                          "SipTransaction::recurseDnsSrvChildren "
                                          "new DNS SRV child %s:%d via '%s'",
                                          childTransaction->mSendToAddress.data(),
                                          childTransaction->mSendToPort,
                                          protoString.data());
                         }
#                   endif

                    // Do not create child for unsupported protocol types
                    if(childTransaction->mSendToProtocol == OsSocket::UNKNOWN)
                    {
                        maxSrvRecords++;
                        delete childTransaction;
                        childTransaction = NULL;
                    }
                    else
                    {
                        // Set the q values of the child based upon the parent
                        // As DNS SRV is recursed serially the Q values are decremented
                        // by a factor of the record index
                        childTransaction->mQvalue = mQvalue - numSrvRecords * 0.0001;

                        // Inherit the expiration from the parent
                        childTransaction->mExpires = mExpires;

                        // Mark this as a DNS child
                        childTransaction->mIsDnsSrvChild = TRUE;

                        childTransaction->mIsBusy = mIsBusy;

                        // Add it to the list
                        transactionList.addTransaction(childTransaction);

                        // Link it in to this parent
                        linkChild(*childTransaction);
                    }

                    numSrvRecords++;
                }   // end create SRV child transactions
            }   // end valid SRV records
            // We got no useful DNS records back
            else
            {
                UtlString protoString;
                SipMessage::convertProtocolEnumToString(mSendToProtocol, protoString);

                Os::Logger::instance().log(FAC_SIP, PRI_WARNING, 
                              "SipTransaction::recurseDnsSrvChildren "
                              "no valid DNS records found for sendTo sip:'%s':%d proto = '%s'",
                              mSendToAddress.data(), mSendToPort, protoString.data());
            }
        }
    }

    UtlBoolean childRecursed = FALSE;
    UtlBoolean childRecursing = FALSE;
    if(!mIsServerTransaction &&
        !mIsDnsSrvChild &&
        mpDnsDestinations &&                            // means sendto address was not NULL
        mpDnsDestinations[0].isValidServerT() &&        // means DNS search returned at least
                                                        // one destination address
        mpRequest)
    {
        UtlSListIterator iterator(mChildTransactions);
        SipTransaction* childTransaction = NULL;

        while ((   childTransaction = (SipTransaction*) iterator())
                && !childRecursed
                && !childRecursing)
        {
            if(childTransaction->mTransactionState == TRANSACTION_LOCALLY_INIITATED)
            {
                // Make a local copy to modify and send
                SipMessage recursedRequest(*mpRequest);

                // Clear the address and port of the previous send
                // of the parent request.
                recursedRequest.removeTopVia(); // the fake via for identifying this TX
                recursedRequest.resetTransport();
                recursedRequest.clearDNSField();

#               ifdef LOG_FORKING
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                              "SipTransaction::recurseDnsSrvChildren "
                              "%p sending child transaction request %s:%d protocol: %d",
                              this,
                              childTransaction->mSendToAddress.data(),
                              childTransaction->mSendToPort,
                              childTransaction->mSendToProtocol);
#               endif
                // Start the transaction by sending its request
                if(childTransaction->handleOutgoing(recursedRequest,
                                                 userAgent,
                                                 transactionList,
                                                 MESSAGE_REQUEST))
                {
                    childRecursed = TRUE;
                }

            }

            // If there is a child transaction that is currently
            // being pursued, do not start any new searches
            else if ((   childTransaction->mTransactionState == TRANSACTION_CALLING
                      || childTransaction->mTransactionState == TRANSACTION_PROCEEDING)
                      && !childTransaction->mIsCanceled)
            {
                childRecursing = TRUE;
#               ifdef LOG_FORKING
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, 
                              "SipTransaction::recurseDnsSrvChildren "
                              "%p still pursing", this);
#               endif
            }

            // This parent is not canceled (implicit) and we found a non-canceled
            // DNS SRV child with any sort of response, so there is no need to
            // recurse.  We have a DNS SRV child that has succeeded (at least in
            // as much as getting a response).
            else if(!childTransaction->mIsCanceled &&
                (childTransaction->mpLastProvisionalResponse ||
                childTransaction->mpLastFinalResponse))
            {
                break;
            }

            else
            {
#               ifdef LOG_FORKING
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, 
                              "SipTransaction::recurseDnsSrvChildren "
                              "%p transaction not recursed state: %s", this,
                              stateString(childTransaction->mTransactionState));
#               endif
            }
        }   // end while
    }
    else
    {
       Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                     "SipTransaction::recurseDnsSrvChildren "
                     "Returning false:  "
                     "%p isrecursing %s "
                     "mIsServerTransaction = %d, "
                     "mIsDnsSrvChild = %d, mpDnsDestinations = %p, "
                     "mpDnsDestinations[0].isValidServerT() = %d, "
                     "mpRequest = %p",
                     this, mIsRecursing ? "True" : "False",
                     mIsServerTransaction, mIsDnsSrvChild,
                     mpDnsDestinations,
                     // Only examine mpDnsDestinations[0] if mpDnsDestinations != NULL.
                     mpDnsDestinations ? mpDnsDestinations[0].isValidServerT() : 0,
                     mpRequest);
    }

    if (childRecursed)
    {    
        mIsRecursing = TRUE;
    }
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                  "SipTransaction::recurseDnsSrvChildren "
                  "%p isrecursing %s"
                  "mIsServerTransaction = %d, "
                  "mIsDnsSrvChild = %d, mpDnsDestinations = %p, "
                  "mpDnsDestinations[0].isValidServerT() = %d, "
                  "mpRequest = %p",
                  this, mIsRecursing ? "True" : "False",
                  mIsServerTransaction, mIsDnsSrvChild,
                  mpDnsDestinations,
                  // Only examine mpDnsDestinations[0] if mpDnsDestinations != NULL.
                  mpDnsDestinations ? mpDnsDestinations[0].isValidServerT() : 0,
                  mpRequest);
    return(childRecursed);
}

UtlBoolean SipTransaction::recurseChildren(SipUserAgent& userAgent,
                                           SipTransactionList& transactionList)
{
    UtlBoolean childRecursed = FALSE;

#   ifdef LOG_FORKING
#   ifdef TEST_PRINT
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                  "SipTransaction::recurseChildren %p", this);
#   endif

    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                  "SipTransaction::recurseChildren %p", this);
#   endif

    if(mpRequest == NULL)
    {
        UtlString transactionString;
        toString(transactionString, TRUE);
        Os::Logger::instance().log(FAC_SIP, PRI_ERR, 
                      "SipTransaction::recurseChildren "
                      "NULL mpResponse\n======>\n%s\n======>",
                      transactionString.data());
    }

    if(mpLastFinalResponse && mpRequest)
    {
        SipTransaction* childTransaction = NULL;
        int responseCode = mpLastFinalResponse->getResponseStatusCode();

#       ifdef TEST_PRINT
        Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                      "SipTransaction::recurseChildren "
                      "lastfinalsresponse & request forking enabled: %d ce: %d",
                      userAgent.isForkingEnabled(),
                      mChildTransactions.isEmpty()
           );
#       endif
        // If this is a client transaction for which we received
        // a 3XX response on which recursion makes sense,
        // decide whether or not to pursue the contacts
        if(   userAgent.isForkingEnabled()
           && responseCode >= SIP_3XX_CLASS_CODE
           && responseCode < SIP_4XX_CLASS_CODE
           && mChildTransactions.isEmpty()
           )
        {
            // collect all the fork urls and calculate the loop detection hash
            UtlSList children;
            UtlString contactString;
            for ( int contactIndex = 0;
                  mpLastFinalResponse->getContactEntry(contactIndex, &contactString);
                  contactIndex++
                  )
            {
                Url contactUrl(contactString);
                mpBranchId->addFork(contactUrl);

                children.insert(new UtlString(contactString));
            }

            /*
             * Before actually creating the child transactions, check for loops.
             * This look for a match between the loop detection key we just built
             * and one in an earlier via of the request; if they match, the request
             * has had exactly this set of contacts before, so reject it as a loop.
             */
            unsigned int loopHop = mpBranchId->loopDetected(*mpRequest);
            if (!loopHop) // no loop was detected
            {
               // no loop - go over the list of child addresses and create the new transactions
               UtlSListIterator nextChild(children);
               UtlString* contact;
               bool hasInsertedDiversion = false;
               const char* diversionHeader = mpLastFinalResponse->getHeaderValue(0, SIP_DIVERSION_FIELD);
               while ((contact = dynamic_cast<UtlString*>(nextChild())))
               {
                  Url contactUrl(*contact);

                  // Make sure we do not add the contact twice and
                  // we have not already pursued this contact
                  if(!isUriRecursed(contactUrl))
                  {
                    //
                    // RFC 5806: Insert the diversion header if it is specified in the 3xx response
                    //
                    if (!hasInsertedDiversion && diversionHeader)
                    {
                      hasInsertedDiversion = true;
                      mpRequest->addDiversionUri(diversionHeader);
                    }

                     // will not be a server transaction
                     // will not be a ua transaction
                     childTransaction = new SipTransaction(mpRequest,
                                                           TRUE, // outgoing
                                                           FALSE,
                                                           mpBranchId
                                                           ); // proxy transaction

#                   ifdef LOG_FORKING
                     Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                                   "SipTransaction::recurseChildren "
                                   "%p adding child %p for contact: '%s'",
                                   this, childTransaction, contact->data());
#                   endif
                     // Add it to the list of all transactions
                     transactionList.addTransaction(childTransaction);

                     // Set the URI of the copy to that from the contact
                     contactUrl.getUri(childTransaction->mRequestUri);

                     // Set the Q value and Expires value
                     UtlString qString;
                     UtlString expiresString;
                     double qValue = 1.0;
                     if(contactUrl.getFieldParameter("q", qString))
                        qValue = atof(qString.data());
                     int expiresSeconds = -1;
                     if((mRequestMethod.compareTo(SIP_INVITE_METHOD) == 0) &&
                        contactUrl.getHeaderParameter("expires", expiresString))
                     {
                        if(Url::isDigitString(expiresString.data()))
                        {
                           // All digits, the format is relative seconds
                           expiresSeconds = atoi(expiresString.data());
                        }
                        else // Alphanumeric, it is an HTTP absolute date
                        {
                           // This format is allowed in RFC 2543, though not in RFC 3261.
                           expiresSeconds =
                              OsDateTime::convertHttpDateToEpoch(expiresString.data());
                           OsTime time;
                           OsDateTime::getCurTimeSinceBoot(time);
                           expiresSeconds -= time.seconds();
                        }
                     }

                     // Set the values of the child
                     childTransaction->mQvalue = qValue;
                     childTransaction->mExpires = expiresSeconds;

                     // Link it in to this parent
                     linkChild(*childTransaction);
                  }  // isUriRecursed was not set
                  else // We have already recursed this contact
                  {
                     Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, 
                                   "SipTransaction::recurseChildren "
                                   "%p already recursed: %s",
                                   this, contactString.data());
                  }
               } // end for each contact
            }   // no loops
            else
            {
               // detected a loop, so change the redirection response into a loop detected.
               Os::Logger::instance().log(FAC_SIP, PRI_WARNING, 
                             "SipTransaction::recurseChildren "
                             "loop detected on call '%s' "
                             "%d hops ago had same contacts",
                             mCallId.data(), loopHop
                             );

               UtlString myAddress;
               int myPort;
               userAgent.getViaInfo(OsSocket::TCP, myAddress, myPort);
               Url mySipAddress;
               mySipAddress.setHostAddress(myAddress.data());
               mySipAddress.setHostPort(myPort);

               UtlString myHostport;
               mySipAddress.getHostWithPort(myHostport);

               char loopDetectedText[128];
               sprintf(loopDetectedText, SIP_LOOP_DETECTED_TEXT " with %d hops ago", loopHop);

               SipMessage* loopDetectedResponse = new SipMessage;
               loopDetectedResponse->setDiagnosticSipFragResponse(*mpRequest,
                                                                  SIP_LOOP_DETECTED_CODE,
                                                                  loopDetectedText,
                                                                  SIP_WARN_MISC_CODE,
                                                                  loopDetectedText,
                                                                  myHostport
                                                                 );
               userAgent.setSelfHeader(*loopDetectedResponse);

               loopDetectedResponse->resetTransport();
               loopDetectedResponse->clearDNSField();

               // Change the redirect response we have into the loop detected response
#ifdef TEST_PRINT
               Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                             "SipTransaction::recurseChildren "
                             "calling addResp "
                             " Tx %p loop detected",
                             this);
#endif
               addResponse(loopDetectedResponse, FALSE /* incoming */, MESSAGE_FINAL );

               childRecursed = FALSE;
            }   // end loop detected

            children.destroyAll(); // release the urls for the forks
        }   // end processing contacts   

        double nextQvalue = -1.0;
        int numRecursed = 0;
        UtlSListIterator iterator(mChildTransactions);
        while ((childTransaction = (SipTransaction*) iterator()))
        {
            // Until a request is successfully sent, reset the
            // current Q value at which transactions of equal
            // value are searched in parallel
            if(numRecursed == 0)
            {
               nextQvalue = -1.0;
            }

            if(childTransaction->mTransactionState == TRANSACTION_LOCALLY_INIITATED)
            {
                double qDelta = nextQvalue - childTransaction->mQvalue;
                double qDeltaSquare = qDelta * qDelta;

#               ifdef LOG_FORKING
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, 
                              "SipTransaction::recurseChildren"
                              " %p qDelta: %f qDeltaSquare: %f mQvalue: %f",
                              this, qDelta, qDeltaSquare, childTransaction->mQvalue);
#               endif

                if(nextQvalue <= 0.0 ||
                   qDeltaSquare < MIN_Q_DELTA_SQUARE)
                {
                    nextQvalue = childTransaction->mQvalue;

#                   ifdef LOG_FORKING
                    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, 
                                  "SipTransaction::recurseChildren"
                                  " %p should recurse child: %p q: %f",
                                  this, childTransaction, childTransaction->mQvalue);
#                   endif

                    // Make a local copy to modify and send
                    SipMessage recursedRequest(*mpRequest);

                    // Clear the address and port of the previous send
                    // of the parent request.
                    recursedRequest.removeTopVia();
                    recursedRequest.resetTransport();
                    recursedRequest.clearDNSField();

                    // If there was a loose route pop it off
                    // The assumption is that this was previously routed
                    // to the redirect server.  Perhaps we can get the
                    // parent's parent's request to use here instead.  I just
                    // cannot work it out in my head right now
                    UtlString routeUri;
                    recursedRequest.getRouteUri(0, &routeUri);
                    Url routeUrlParser(routeUri);
                    UtlString dummyValue;
                    UtlBoolean nextHopLooseRoutes = routeUrlParser.getUrlParameter("lr", dummyValue, 0);
                    if(nextHopLooseRoutes)
                    {
                        recursedRequest.removeRouteUri(0, &routeUri);
                    }

                    // Correct the URI of the request to be the recursed URI
                    recursedRequest.setSipRequestFirstHeaderLine(mRequestMethod,
                                                                 childTransaction->mRequestUri, 
                                                                 SIP_PROTOCOL_VERSION);

                    // Decrement max-forwards
                    int maxForwards;
                    if(!recursedRequest.getMaxForwards(maxForwards))
                    {
                        recursedRequest.setMaxForwards(userAgent.getMaxForwards() - 1);
                    }
                    else
                    {
                        recursedRequest.setMaxForwards(maxForwards - 1);
                    }

#                   ifdef LOG_FORKING
                    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, 
                                  "SipTransaction::recurseChildren"
                                  " %p sending child transaction request", this);
#                   endif
                    // Start the transaction by sending its request
                    if(childTransaction->handleOutgoing(recursedRequest,
                                                        userAgent,
                                                        transactionList,
                                                        MESSAGE_REQUEST))
                    {

                        numRecursed++;
                        // Recursing is TRUE
                        childRecursed = TRUE; // Recursion disabled
                    }
                }   // end sending recursed request

                else
                {
#                   ifdef LOG_FORKING
                    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, 
                                  "SipTransaction::recurseChildren"
                                  " %p nextQvalue: %f qDeltaSquare: %f", this,
                                  nextQvalue, qDeltaSquare);
#                   endif
                }
            }   // child state is LOCAL

            // If there is a child transaction that is currently
            // being pursued, do not start any new searches
            else if((   childTransaction->mTransactionState == TRANSACTION_CALLING 
                     || childTransaction->mTransactionState == TRANSACTION_PROCEEDING)
                     && !childTransaction->mIsCanceled)
            {
                nextQvalue = childTransaction->mQvalue;
#               ifdef LOG_FORKING
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, 
                              "SipTransaction::recurseChildren"
                              " %p still pursing", this);
#               endif
            }

            else
            {
#               ifdef LOG_FORKING
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, 
                              "SipTransaction::recurseChildren"
                              " %p transaction not recursed state: %s",
                              this, stateString(childTransaction->mTransactionState));
#               endif
            }

            // Optionally we only look at the first contact
            // for 300 response (not 300 class e.g. 302)
            if (userAgent.recurseOnlyOne300Contact() 
                && responseCode == SIP_MULTI_CHOICE_CODE) 
            {
                    break;
            }
        }
    }

    if (childRecursed)
    {     
        mIsRecursing = TRUE;
    }
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                  "SipTransaction::recurseChildren "
                  "%p isrecursing %s"
                  "mIsServerTransaction = %d, "
                  "mIsDnsSrvChild = %d, mpDnsDestinations = %p, "
                  "mpDnsDestinations[0].isValidServerT() = %d, ",
                  this, mIsRecursing ? "True" : "False",
                  mIsServerTransaction, mIsDnsSrvChild,
                  mpDnsDestinations,
                  // Only examine mpDnsDestinations[0] if mpDnsDestinations != NULL.
                  mpDnsDestinations ? mpDnsDestinations[0].isValidServerT() : 0);
    return(childRecursed);
}

void SipTransaction::getChallengeRealms(const SipMessage& response, UtlSList& realmList)
{
   // Get any the proxy authentication challenges from the new challenge
   UtlString authField;
   for (unsigned authIndex = 0;
        response.getAuthenticateField(authIndex, HttpMessage::PROXY, authField);
        authIndex++
        )
   {
      UtlString challengeRealm;
      if (   HttpMessage::parseAuthenticateData(authField,
                                                NULL, // scheme
                                                &challengeRealm,
                                                NULL, // nonce
                                                NULL, // opaque
                                                NULL, // algorithm
                                                NULL, // qop
                                                NULL  // domain
                                                  )
          && !realmList.contains(&challengeRealm)
          )
      {
         realmList.insert(new UtlString(challengeRealm));
      }
   }
}

UtlBoolean SipTransaction::findBestResponse(SipMessage& bestResponse)
{
#   ifdef TEST_PRINT
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                  "SipTransaction::findBestResponse %p", this);
#   endif
    int responseFoundCount = 0;
    UtlBoolean retVal = FALSE;

    retVal = findBestChildResponse(bestResponse, responseFoundCount);

    return retVal;
}

enum SipTransaction::ResponsePriority SipTransaction::findRespPriority(int responseCode)
{
    enum SipTransaction::ResponsePriority respPri;

#ifdef RESPONSE_DEBUG
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                  "SipTransaction::findRespPriority"
                  " %p response code %d",
                  this, responseCode);
#endif
    switch (responseCode)
    {
    case HTTP_UNAUTHORIZED_CODE:        // 401
    case HTTP_PROXY_UNAUTHORIZED_CODE:  // 407
        // 401 & 407 are better than any other 4xx, 5xx or 6xx
        respPri = RESP_PRI_CHALLENGE;
        break;

    case SIP_REQUEST_TERMINATED_CODE:   // 487
        // 487 is better than any other 4xx, 5xx, or 6xx if the
        // transaction has been canceled.
        respPri = RESP_PRI_CANCEL;
        break;

    case SIP_3XX_CLASS_CODE:            // 300
    case SIP_TEMPORARY_MOVE_CODE:       // 302
    case SIP_PERMANENT_MOVE_CODE:       // 301
    case SIP_USE_PROXY_CODE:            // 305
        // 3xx is better than 4xx
        respPri = RESP_PRI_3XX;
        break;

    case SIP_BAD_REQUEST_CODE:              // 400
    case SIP_FORBIDDEN_CODE:                // 403
    case SIP_DECLINE_CODE:                  // 603 - not a typo
    case SIP_BAD_METHOD_CODE:               // 405
    case SIP_REQUEST_TIMEOUT_CODE:          // 408
    case SIP_CONDITIONAL_REQUEST_FAILED_CODE:    // 412
    case SIP_BAD_MEDIA_CODE:                // 415
    case SIP_UNSUPPORTED_URI_SCHEME_CODE:   // 416
    case SIP_BAD_EXTENSION_CODE:            // 420
    case SIP_EXTENSION_REQUIRED_CODE:       // 421
    case SIP_TOO_BRIEF_CODE:                // 423
    case SIP_TEMPORARILY_UNAVAILABLE_CODE:   // 480
    case SIP_BAD_TRANSACTION_CODE:          // 481
    case SIP_LOOP_DETECTED_CODE:            // 482
    case SIP_TOO_MANY_HOPS_CODE:            // 483
    case SIP_BAD_ADDRESS_CODE:              // 484
    case SIP_BUSY_CODE:                     // 486
    case SIP_REQUEST_NOT_ACCEPTABLE_HERE_CODE:   // 488
    case SIP_BAD_EVENT_CODE:                // 489
        respPri = RESP_PRI_4XX;
        break;

    case SIP_NOT_FOUND_CODE:                // 404
        respPri = RESP_PRI_404;
        break;

    case SIP_SERVER_INTERNAL_ERROR_CODE:    // 500
    case SIP_UNIMPLEMENTED_METHOD_CODE:     // 501
    case SIP_SERVICE_UNAVAILABLE_CODE:      // 503
    case SIP_BAD_VERSION_CODE:              // 505
        respPri = RESP_PRI_5XX;
        break;

    case SIP_6XX_CLASS_CODE:                // 600
        respPri = RESP_PRI_6XX;
        break;

    default:
        // not a specific code we defined, select by category
        if (responseCode >= SIP_6XX_CLASS_CODE)    // must be 6xx
        {
            respPri = RESP_PRI_6XX;
        }
        else if (responseCode >= SIP_5XX_CLASS_CODE)   // must be 5xx
        {
            respPri = RESP_PRI_5XX;
        }
        else if (responseCode >= SIP_4XX_CLASS_CODE)   // must be 4xx
        {
            respPri = RESP_PRI_4XX;
        }
        else if (responseCode >= SIP_3XX_CLASS_CODE)   // must be 3xx
        {
            respPri = RESP_PRI_3XX;
        }
        else    // best response must still be empty
        {
            respPri = RESP_PRI_NOMATCH;
        }
        break;
    }
#ifdef RESPONSE_DEBUG
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                  "SipTransaction::findRespPriority"
                  " %p response code %d returns %d",
                  this, responseCode, respPri);
#endif
    return respPri;
}

UtlBoolean SipTransaction::findBestChildResponse(SipMessage& bestResponse, int responseFoundCount)
{
#   ifdef TEST_PRINT
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                  "SipTransaction::findBestChildResponse start %p",
                  this);
#   endif

    UtlSListIterator iterator(mChildTransactions);
    SipTransaction* childTransaction = NULL;
    UtlBoolean responseFound = FALSE;
    SipMessage* childResponse = NULL;
    int bestResponseCode = -1;
    int childResponseCode = -1;
    UtlBoolean foundChild = FALSE;
    UtlBoolean useThisResp = FALSE;
    enum ResponsePriority respPri = RESP_PRI_NOMATCH, bestPri = RESP_PRI_NOMATCH;
    int pathNum = 0;

    UtlSList proxyRealmsSeen;
    while ((childTransaction = (SipTransaction*) iterator()))
    {
        // Check the child's decendents first
        // Note: we need to check the child's children even if this child
        // has no response.
        foundChild = childTransaction->findBestChildResponse(bestResponse, responseFoundCount);
        if(foundChild)
        {
           responseFound = TRUE;
        }

        childResponse = childTransaction->mpLastFinalResponse;
#ifdef RESPONSE_DEBUG
        Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                      "SipTransaction::findBestChildResponse"
                      " %p child %p returns %d with lastResp %p (bestResp %p)",
                      this, childTransaction, foundChild, childResponse, &bestResponse);
#endif
        if(childResponse)
        {
            bestResponseCode = bestResponse.getResponseStatusCode();
            childResponseCode = childResponse->getResponseStatusCode();

            respPri = findRespPriority(childResponseCode);
            bestPri = findRespPriority(bestResponseCode);

#ifdef RESPONSE_DEBUG
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                          "SipTransaction::findBestChildResponse"
                          " %p child %p status/pri %d/%d (bestResp %d/%d)",
                          this, childTransaction, childResponseCode, respPri, bestResponseCode, bestPri);
#endif
            if (bestPri == RESP_PRI_NOMATCH)
            {
                // this is the first response we have found
                useThisResp = TRUE;
                pathNum = 1;
            }
            else
            {
                pathNum = 2;

                switch (respPri)
                {
                case RESP_PRI_CHALLENGE:
                    pathNum = 3;
                    if (bestPri == RESP_PRI_CHALLENGE)
                    {
                        pathNum = 4;
                        /*
                         * Some implementations get confused if there is more than one
                         * Proxy challenge for the same realm, so filter out any extra
                         * ones.  Since ours are all generated with the same shared secret,
                         * a challenge from any sipXproxy is acceptable at any sipXproxy;
                         * it is possible that this will cause problems with multiple
                         * proxy authentication requests being forwarded from some
                         * downstream forking proxy that does not have that quality, but
                         * it's better to get something back to the UA that won't break it.
                         * We can't filter and do this only to our own realm, because at this
                         * level in the stack we have no idea what our own realm is :-(
                         */
                        getChallengeRealms(bestResponse, proxyRealmsSeen);

                        // Get the proxy authenticate challenges
                        UtlString authField;
                        unsigned  authIndex;
                        for (authIndex = 0;
                             childResponse->getAuthenticateField(authIndex, 
                                                                 HttpMessage::PROXY,
                                                                 authField);
                             authIndex++
                             )
                        {
                           UtlString challengeRealm;
                           if (HttpMessage::parseAuthenticateData(authField,
                                                                  NULL, // scheme
                                                                  &challengeRealm,
                                                                  NULL, // nonce
                                                                  NULL, // opaque
                                                                  NULL, // algorithm
                                                                  NULL, // qop
                                                                  NULL  // domain
                                                                  )
                               )
                           {
                              if (!proxyRealmsSeen.contains(&challengeRealm))
                              {
                                 proxyRealmsSeen.insert(new UtlString(challengeRealm));
                                 bestResponse.addAuthenticateField(authField, HttpMessage::PROXY);
                              }
                              else
                              {
                                 Os::Logger::instance().log(FAC_SIP, PRI_INFO,
                                               "SipTransaction::findBestChildResponse"
                                               " removing redundant proxy challenge:\n   %s",
                                               authField.data());
                              }
                           }
                           else
                           {
                              Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                                            "SipTransaction::findBestChildResponse"
                                            " removing unparsable proxy challenge:\n   %s",
                                            authField.data());
                           }
                        }

                        // Get the UA server authenticate challenges
                        for (authIndex = 0;
                             childResponse->getAuthenticateField(authIndex,
                                                                 HttpMessage::SERVER, 
                                                                 authField);
                             authIndex++
                             )
                         {
                             bestResponse.addAuthenticateField(authField, HttpMessage::SERVER);
                         }
                    }   // end child and best both want to challenge
                    else if (bestPri >= RESP_PRI_4XX)  // bestResp == cancel will top this
                    {
                        pathNum = 5;
                        // 401 & 407 are better than any other 4xx, 5xx or 6xx
                        useThisResp = TRUE;
                    }
                    else
                    {
                        pathNum = 6;
                    }
                    break;  // end child wants to challenge
                case RESP_PRI_CANCEL:
                    // 487 is better than any other 4xx, 5xx, or 6xx if the
                    // transaction has been canceled.
                    // This improves the odds that we send a 487 response to
                    // canceled transactions, which is not required, but tends
                    // to make UAs behave better.
                    pathNum = 7;
                    if ( childTransaction->mIsCanceled
                         && bestPri >= RESP_PRI_4XX)   // bestResp == 401, 407, 487 will top this
                    {
                        // An unforked 3xx response
                        useThisResp = TRUE;
                        pathNum = 8;
                    }
                    break;  // end 487 cancel
                case RESP_PRI_3XX:
                    pathNum = 9;
                    // 3xx is better than 4xx
                    if ( bestPri >= RESP_PRI_4XX   // bestResp == 401, 407, 487 will top this
                         && childTransaction->mChildTransactions.isEmpty())
                    {
                        useThisResp = TRUE;
                        pathNum = 10;
                    }
                    break;
                case RESP_PRI_4XX:
                    pathNum = 11;
                    if ( bestPri > RESP_PRI_4XX )
                    {
                        pathNum = 12;
                        useThisResp = TRUE;
                    }
                    break;
                case RESP_PRI_5XX:
                    pathNum = 13;
                    if ( bestPri > RESP_PRI_5XX )
                    {
                        pathNum = 14;
                        useThisResp = TRUE;
                    }
                    break;
                case RESP_PRI_6XX:
                    pathNum = 15;
                    if ( bestPri > RESP_PRI_6XX )
                    {
                        pathNum = 16;
                        useThisResp = TRUE;
                    }
                    break;
                default:
                    pathNum = 17;
                    useThisResp = TRUE;
                    break;
                }   // end respPri switch
            }   // end bestresp is valid

            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                          "SipTransaction::findBestChildResponse"
                          " %p child %p status/pri %d/%d (bestResp %d/%d) usethis=%d pathNum %d",
                          this, childTransaction, childResponseCode, respPri, bestResponseCode, bestPri, useThisResp, pathNum);

            if (useThisResp)
            {
                pathNum = 18;
                bestResponse = *(childResponse);

                // Not supposed to return 503 unless we know that
                // there is absolutely no way to reach the end point
                if(childResponseCode == SIP_SERVICE_UNAVAILABLE_CODE)
                {
                    pathNum = 19;
                    bestResponse.setResponseFirstHeaderLine(SIP_PROTOCOL_VERSION,
                        SIP_SERVER_INTERNAL_ERROR_CODE,
                        SIP_SERVER_INTERNAL_ERROR_TEXT);
                }

                bestResponse.removeTopVia();
                bestResponse.resetTransport();
                bestResponse.clearDNSField();
                responseFound = TRUE;
            }
        }
    }
    proxyRealmsSeen.destroyAll();

    // We have made it to the top and there are no responses
    if (!responseFound && mpParentTransaction == NULL)
    {
        if (mpRequest)
        {
            bestResponse.setResponseData(mpRequest,
                                         SIP_REQUEST_TIMEOUT_CODE,
                                         SIP_REQUEST_TIMEOUT_TEXT);
            responseFound = TRUE;
        }
        else
        {
            Os::Logger::instance().log(FAC_SIP, PRI_ERR, 
                          "SipTransaction::findBestChildResponse "
                          "no request");
        }
    }

    if(responseFound)
    {
        const char* firstHeaderLine = bestResponse.getFirstHeaderLine();

        if(firstHeaderLine == NULL ||
            *firstHeaderLine == '\0')
        {
            if (Os::Logger::instance().willLog(FAC_SIP, PRI_WARNING))
            {
                UtlString msgString;
                ssize_t msgLen;
                bestResponse.getBytes(&msgString, &msgLen);

                // We got a bad response
                Os::Logger::instance().log(FAC_SIP, PRI_ERR,
                              "SipTransaction::findBestChildResponse "
                              "invalid response:\n%s",
                              msgString.data());
            }
        }
    }
#ifdef RESPONSE_DEBUG
    Os::Logger::instance().log(FAC_SIP, PRI_INFO,
                  "SipTransaction::findBestChildResponse"
                  " end %p child %p status/pri %d/%d (bestResp %d/%d) usethis=%d pathNum %d",
                  this, childTransaction, childResponseCode, respPri, bestResponseCode, bestPri, useThisResp, pathNum);
#endif
    return(responseFound);
}

UtlBoolean SipTransaction::doResend(SipMessage& resendMessage,
                                   SipUserAgent& userAgent,
                                   int& nextTimeout)
{
    // The timeout to set from this resend to the next resend. (msec)
    nextTimeout = 0;
    // Get how many times we have sent this message before.
    int numTries = resendMessage.getTimesSent();
    // Get the sending protocol.
    OsSocket::IpProtocolSocketType protocol = resendMessage.getSendProtocol();
    int lastTimeout = resendMessage.getResendInterval();
    // Get the address/port to which to send the message.
    UtlString sendAddress;
    int sendPort;
    resendMessage.getSendAddress(&sendAddress, &sendPort);
    UtlBoolean sentOk = FALSE;

    // TCP/TLS gets SIP_TCP_RESEND_TIMES tries
    // UDP gets SIP_UDP_RESEND_TIMES tries

    if (protocol == OsSocket::UDP)
    {
        if (numTries < SIP_UDP_RESEND_TIMES)
        {
            // Try UDP again
            if (userAgent.sendUdp(&resendMessage, sendAddress.data(), sendPort))
            {
                // Do this after the send so that the log message is correct
                resendMessage.incrementTimesSent();

                // Schedule the timeout for the next resend.
                nextTimeout = lastTimeout * 2;
                if (nextTimeout > userAgent.getMaxResendTimeout())
                {
                    nextTimeout = userAgent.getMaxResendTimeout();
                }
                resendMessage.setResendInterval(nextTimeout);

                sentOk = TRUE;
            }
        }
    } // UDP
    else if (   protocol == OsSocket::TCP
#ifdef SIP_TLS
            || protocol == OsSocket::SSL_SOCKET
#endif
           )
    {
        if (numTries < SIP_TCP_RESEND_TIMES)
        {
            bool r = !SipTransaction::enableTcpResend;
            // Try sending again.
            if (SipTransaction::enableTcpResend)
            {
              if (protocol == OsSocket::TCP)
              {
                 r = userAgent.sendTcp(&resendMessage,
                                       sendAddress.data(),
                                       sendPort);
              }
  #ifdef SIP_TLS
              else if (protocol == OsSocket::SSL_SOCKET)
              {
                 r = userAgent.sendTls(&resendMessage,
                                       sendAddress.data(),
                                       sendPort);
              }
  #endif
            }

            if (r)
            {
                // Do this after the send so that the log message is correct
                resendMessage.incrementTimesSent();

                // Schedule the timeout for the next resend.
                nextTimeout = lastTimeout * 2;
                if (nextTimeout > userAgent.getMaxResendTimeout())
                {
                    nextTimeout = userAgent.getMaxResendTimeout();
                }
                resendMessage.setResendInterval(nextTimeout);

                sentOk = TRUE;
            }
        }

    } // TCP/TLS

    return sentOk;
} // end doResend

UtlBoolean SipTransaction::handleIncoming(SipMessage& incomingMessage,
                                         SipUserAgent& userAgent,
                                         enum messageRelationship relationship,
                                         SipTransactionList& transactionList,
                                         SipMessage*& delayedDispatchedMessage)
{
   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                 "SipTransaction::handleIncoming "
                 "%p relationship %s",
                 this, relationshipString(relationship)
                 );

    if(delayedDispatchedMessage)
    {
        Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                      "SipTransaction::handleIncoming "
                      "delayedDispatchedMessage not NULL");
        delayedDispatchedMessage = NULL;
    }

    UtlBoolean shouldDispatch = FALSE;

    if(relationship == MESSAGE_UNKNOWN)
    {
        relationship = whatRelation(incomingMessage,
                                    TRUE);
    }

    // This is a message was already recieved once
    if(relationship == MESSAGE_DUPLICATE)
    {
        // Update the time stamp so that this message
        // does not get garbage collected right away.
        // This is so that rogue UAs that keep sending
        // the same message for a long (i.e. longer than
        // transaction timeout) period of time are ignored.
        // Otherwise this message looks like a new message
        // after a transaction timeout and the original
        // copy gets garbage collected.  We explicitly
        // do NOT touch the outgoing (i.e. sentMessages)
        // as we do not want to keep responding after
        // the transaction timeout.
        //previousMessage->touchTransportTime();

        // If it is a request resend the response if it exists
        if(!(incomingMessage.isResponse()))
        {
            SipMessage* response = NULL; //sentMessages.getResponseFor(message);
            UtlString method;
            incomingMessage.getRequestMethod(&method);
            if(method.compareTo(SIP_ACK_METHOD) == 0)
            {
                // Do nothing, we already have the ACK
            }
            else if(method.compareTo(SIP_CANCEL_METHOD) == 0)
            {
                // Resend the CANCEL response
                response = mpCancelResponse;
            }
            else
            {
                // Resend the final response if there is one
                // Otherwise resend the provisional response
                response = mpLastFinalResponse ?
                    mpLastFinalResponse : mpLastProvisionalResponse;

                // If there is no response, then construct a 100 response
                // to quench resends.  (This is a deviation from RFC 3261,
                // which prescribes that 100's should only be sent for INVITE
                // requests, but if we are seeing resends of another request,
                // for efficiency we need to quench resends.)
                if (!response)
                {
                   // Create a 100 Trying and send it.
                   if (method.compareTo(SIP_INVITE_METHOD) == 0 || SipTransaction::SendTryingForNist)
		   {
		     SipMessage trying;
                     trying.setTryingResponseData(&incomingMessage);
                     // We cannot set 'response' to 'trying', as 'trying' does
                     // not have its transport parameters set.  So we have to
                     // pass it to handleOutgoing().
#                    ifdef TEST_PRINT
                     Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                                   "SipTransaction::handleIncoming "
                                   "sending trying response for resent non-INVITE request");
#                    endif
                     handleOutgoing(trying,
                                    userAgent,
                                    transactionList,
                                    MESSAGE_PROVISIONAL);
		   }

                }
            }

#ifdef TEST_PRINT
            if (!response)
            {
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                          "SipTransaction::handleIncoming "
                          "duplicate request, no response");
            }
            else
            {
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                          "SipTransaction::handleIncoming "
                          "duplicate request, response protocol: %d",
                          response->getSendProtocol());
            }
#endif
            if(response)
            {
                int sendProtocol = response->getSendProtocol();
                UtlString sendAddress;
                int sendPort;
                response->getSendAddress(&sendAddress, &sendPort);

                switch (sendProtocol)
                {
                case OsSocket::UDP:
                    userAgent.sendUdp(response, sendAddress.data(), sendPort);
                    break;

                case OsSocket::TCP:
                    userAgent.sendTcp(response, sendAddress.data(), sendPort);
                    break;

#               ifdef SIP_TLS
                case OsSocket::SSL_SOCKET:
                    userAgent.sendTls(response, sendAddress.data(), sendPort);
                    break;
#               endif

                default:
                   Os::Logger::instance().log(FAC_SIP, PRI_CRIT, 
                                 "SipTransaction::handleIncoming"
                                 " invalid response send protocol %d", sendProtocol);
                }

#ifdef TEST_PRINT
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                              "SipTransaction::handleIncoming "
                              "resending response");
#endif
            }
        }

        // If it is an INVITE response, resend the ACK if it exists
        //
        else
        {
            int cSeq;
            UtlString seqMethod;
            incomingMessage.getCSeqField(&cSeq, &seqMethod);

            // We assume ACK will only exist if this was an INVITE
            // transaction.  We resend only if this is a
            // UA transaction.
            if (   !seqMethod.compareTo(SIP_CANCEL_METHOD)
                && mpAck
                && mIsUaTransaction
                )
            {
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, 
                              "SipTransaction::handleIncoming "
                              "resending ACK");

                int sendProtocol = mpAck->getSendProtocol();
                UtlString sendAddress;
                int sendPort;
                mpAck->getSendAddress(&sendAddress, &sendPort);

                switch (sendProtocol)
                {
                case OsSocket::UDP:
                    userAgent.sendUdp(mpAck, sendAddress.data(), sendPort);
                    break;
                case OsSocket::TCP:
                    userAgent.sendTcp(mpAck, sendAddress.data(), sendPort);
                    break;
#               ifdef SIP_TLS
                case OsSocket::SSL_SOCKET:
                   userAgent.sendTls(mpAck, sendAddress.data(), sendPort);
                   break;
#               endif
                default:
                   Os::Logger::instance().log(FAC_SIP, PRI_CRIT, 
                                 "SipTransaction::handleIncoming"
                                 " invalid ACK send protocol %d", sendProtocol);
                }

#ifdef TEST_PRINT
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                              "SipTransaction::handleIncoming resent ACK");
#endif
            }
        }
    }

    // The first time we received this message
    else if(relationship == MESSAGE_FINAL)
    {
        if(mpAck)
        {
            int cSeq;
            UtlString seqMethod;
            incomingMessage.getCSeqField(&cSeq, &seqMethod);
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                          "SipTransaction::handleIncoming "
                          "resending ACK for final response %s",
                          seqMethod.data()
                          );

            int sendProtocol = mpAck->getSendProtocol();
            UtlString sendAddress;
            int sendPort;
            mpAck->getSendAddress(&sendAddress, &sendPort);

            switch (sendProtocol)
            {
            case OsSocket::UDP:
               userAgent.sendUdp(mpAck, sendAddress.data(), sendPort);
               break;
            case OsSocket::TCP:
               userAgent.sendTcp(mpAck, sendAddress.data(), sendPort);
               break;
#           ifdef SIP_TLS
            case OsSocket::SSL_SOCKET:
               userAgent.sendTls(mpAck, sendAddress.data(), sendPort);
               break;
#           endif
            default:
               Os::Logger::instance().log(FAC_SIP, PRI_CRIT, 
                             "SipTransaction::handleIncoming"
                             " invalid ACK send protocol %d", sendProtocol);
            }
            mpAck->incrementTimesSent();
            mpAck->touchTransportTime();

            shouldDispatch = TRUE;

            Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                          "SipTransaction::handleIncoming "
                          "received final response,"
                          "sending existing ACK"
                          );
        }
        else
        {
            // If this is an error final response to an INVITE
            // We can automatically construct the ACK here:
            if(   mpRequest
               && (mRequestMethod.compareTo(SIP_INVITE_METHOD) == 0)
               && (incomingMessage.getResponseStatusCode() >= SIP_3XX_CLASS_CODE)
               )
            {
                SipMessage ack;
                ack.setAckData(&incomingMessage,
                    mpRequest);

               // SDUA
               // is err code > 300 , set the DNS data files in the response
               // We may not need this any more as the ACK for error
               // responses is now done here
               UtlString protocol;
               UtlString address;
               UtlString port;
               if (mpRequest->getDNSField( &protocol , &address , &port))
               {
                   ack.setDNSField(protocol,
                                   address,
                                   port);
               }
#ifdef TEST_PRINT
               Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                             "SipTransaction::handleIncoming "
                             "%p sending ACK for error response",
                             this);
#endif
               handleOutgoing(ack,
                              userAgent,
                              transactionList,
                              MESSAGE_ACK);

               shouldDispatch = TRUE;
            }   // end INVITE failure response

            // Non INVITE response or 2XX INVITE responses for which
            // there is no ACK yet get dispatched.  The app layer must
            // generate the ACK for 2XX responses as it may contain
            // SDP
            else
            {
                shouldDispatch = TRUE;
            }
        }

        SipMessage* responseCopy = new SipMessage(incomingMessage);
#ifdef TEST_PRINT
        Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                      "SipTransaction::handleIncoming -1- "
                      "calling addResp "
                      " Tx %p has relationship is %s",
                      this, relationshipString(relationship));
#endif
        addResponse(responseCopy,
                    FALSE, // Incoming
                    relationship);

    } // End if new final response

    // Requests, provisional responses, CANCEL response
    else
    {
        SipMessage* responseCopy = new SipMessage(incomingMessage);
#ifdef TEST_PRINT
        Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                      "SipTransaction::handleIncoming -2- "
                      "calling addResp "
                      " Tx %p has relationship is %s",
                      this, relationshipString(relationship));
#endif
        addResponse(responseCopy,
                    FALSE, // Incoming
                    relationship);

        if(relationship == MESSAGE_REQUEST &&
           mIsServerTransaction)
        {
            if(mpLastProvisionalResponse)
            {
               Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                             "SipTransaction::handleIncoming"
                             " new request with an existing provisional response");
            }
            else if(mpLastFinalResponse)
            {
                Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                              "SipTransaction::handleIncoming"
                              " new request with an existing final response");
            }
            // INVITE transactions we can send trying to stop the resends
            else if(mRequestMethod.compareTo(SIP_INVITE_METHOD) == 0)
            {
                // Create and send a 100 Trying
                SipMessage trying;
                trying.setTryingResponseData(&incomingMessage);

#               ifdef TEST_PRINT
              	Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                	      "SipTransaction::handleIncoming "
                          "sending trying response");
#               endif
                handleOutgoing(trying,
                               userAgent,
                               transactionList,
                               MESSAGE_PROVISIONAL);
            }
        }   // end is server request transaction

        // If this transaction was marked as canceled
        // but we could not send the CANCEL until we
        // got a provisional response, we can now send
        // the CANCEL (we only send CANCEL for INVITEs).
        if(relationship == MESSAGE_PROVISIONAL
           && ! mIsServerTransaction
           && mIsCanceled
           && mpRequest
           && mRequestMethod.compareTo(SIP_INVITE_METHOD) == 0)
        {
            if (mpCancel)
            {
                // mpCancel is populated when doFirstSend calls addResponse, so
                // mpCancel != 0 means that a CANCEL was already sent.
                // This test could be put in the previous 'if', but it
                // is separate to allow this debug message.
                Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                              "SipTransaction::handleIncoming "
                              "got provisional response but already sent CANCEL");
            }
            else
            {
                SipMessage cancel;

                cancel.setCancelData(mpRequest, &mCancelReasonValue);
    #ifdef TEST_PRINT
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                              "SipTransaction::handleIncoming "
                              "sending CANCEL after receiving first provisional response");
    #endif
                handleOutgoing(cancel,
                               userAgent,
                               transactionList,
                               MESSAGE_CANCEL);
            }
        }     // end send delayed CANCEL now that we got provisional response

        // Incoming CANCEL, respond and cancel children
        else if(mIsServerTransaction &&
            relationship == MESSAGE_CANCEL)
        {
            if(mpRequest)
            {
                UtlString reqMethod;
                mpRequest->getRequestMethod(&reqMethod);
                if(reqMethod.compareTo(SIP_INVITE_METHOD) == 0 &&
                   (mTransactionState == TRANSACTION_PROCEEDING ||
                   mTransactionState == TRANSACTION_CALLING))
                {
                    // Proxy transaction
                    if(!mIsUaTransaction)
                    {
                        // Append the Reason value of the incoming CANCEL
                        // to any value we already have for outgoing CANCELs.
                        UtlString reasonTxt;
                        reasonTxt = incomingMessage.getHeaderValue(0, SIP_REASON_FIELD);
                        if (!reasonTxt.isNull())
                        {
                           if (!mCancelReasonValue.isNull())
                           {
                              mCancelReasonValue.append(", ");
                           }
                           mCancelReasonValue.append(reasonTxt.data());
                        }

                        cancelChildren(userAgent,
                                       transactionList);
                        shouldDispatch = FALSE;
                    }

                    // UA transaction
                    else
                    {
                        shouldDispatch = TRUE;
                    }

                    if(mpLastFinalResponse == NULL)
                    {
                        // I think this is wrong only the app. layer of a
                        // UAS should response with 487
                        // Respond to the server transaction INVITE
                        //SipMessage inviteResponse;
                        //inviteResponse.setResponseData(mpRequest,
                        //                    SIP_REQUEST_TERMINATED_CODE,
                        //                    SIP_REQUEST_TERMINATED_TEXT);
                        //handleOutgoing(inviteResponse, userAgent,
                        //    MESSAGE_FINAL);
                    }

                }

                // Too late to cancel, non-INVITE request and
                // successfully canceled INVITEs all get a
                // 200 response to the CANCEL
                SipMessage cancelResponse;
                cancelResponse.setResponseData(&incomingMessage,
                                    SIP_OK_CODE,
                                    SIP_OK_TEXT);
#ifdef TEST_PRINT
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                              "SipTransaction::handleIncoming "
                              "sending CANCEL response");
#endif
                handleOutgoing(cancelResponse,
                               userAgent,
                               transactionList,
                               MESSAGE_CANCEL_RESPONSE);
            }

            // No request for to cancel
            else
            {
                // Send a transaction not found error
                SipMessage cancelError;
                cancelError.setBadTransactionData(&incomingMessage);
#ifdef TEST_PRINT
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                              "SipTransaction::handleIncoming "
                              "transaction not found, sending CANCEL response");
#endif
                handleOutgoing(cancelError,
                               userAgent,
                               transactionList,
                               MESSAGE_CANCEL_RESPONSE);
            }

        } // End cancel request

        else
        {
            // ACK received for server transaction
            if(mIsServerTransaction &&
               relationship == MESSAGE_ACK)
            {
                int responseCode = -1;
                if(mpLastFinalResponse)
                {
                    responseCode =
                        mpLastFinalResponse->getResponseStatusCode();
                }

                // If this INVITE transaction ended in an error final response
                // we do not forward the message via the clients transactions
                // the client generates its own ACK
                if(responseCode >= SIP_3XX_CLASS_CODE)
                {
                    shouldDispatch = FALSE;
                }

                // Else if this was a successful INVITE the ACK should:
                //     only come to this proxy if the INVITE was record-routed
                //  or the previous hop that send this ACK incorrectly sent the
                //     ACK to the same URI as the INVITE
                else
                {
                    shouldDispatch = TRUE;
                }
            }

            else    // covers 2xx ACKs
            {
                shouldDispatch = TRUE;
            }

        }       // end "else" that seems to take care of ACKs
    } // End: Requests, provisional responses, Cancel response

#   ifdef TEST_PRINT
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                  "SipTransaction::handleIncoming "
                  "%p asking parent shouldDispatch=%d delayed=%p",
                  this, shouldDispatch, delayedDispatchedMessage );
#   endif

#   ifdef DISPATCH_DEBUG
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                  "SipTransaction::handleIncoming %p "
                  "before handleChildIncoming shouldDispatch=%d delayed=%p",
                  this, shouldDispatch, delayedDispatchedMessage );
#   endif

    shouldDispatch =
        handleChildIncoming(incomingMessage,
                            userAgent,
                            relationship,
                            transactionList,
                            shouldDispatch,
                            delayedDispatchedMessage);

#   ifdef DISPATCH_DEBUG
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                  "SipTransaction::handleIncoming %p "
                  "after handleChildIncoming shouldDispatch=%d delayed=%p",
                  this, shouldDispatch, delayedDispatchedMessage);
#   endif
#   ifdef TEST_PRINT
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                  "SipTransaction::handleIncoming %p "
                  "parent says shouldDispatch=%d delayed=%p",
                  this, shouldDispatch, delayedDispatchedMessage);
#   endif
    touch();

    return(shouldDispatch);
} // end handleIncoming

void SipTransaction::removeTimer(OsTimer* timer)
{
   mTimers.removeReference(timer);
}

void SipTransaction::deleteTimers()
{
    OsTimer* timer = NULL;

    while ((timer = dynamic_cast<OsTimer*>(mTimers.get() /* pop one timer */)))
    {
#       ifdef TEST_PRINT
        Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                      "SipTransaction::deleteTimers "
                      "tx- %p deleting timer %p",
                      this, timer);
#       endif

        //
        // The timer owns the event.  we must delete it here
        //
        timer->stop(FALSE /* do not block */);
        SipMessageEvent* pMsgEvent = (SipMessageEvent*) timer->getUserData();
        delete pMsgEvent;

        // We always delete the timer.
        delete timer;
    }
}

void SipTransaction::stopTimers()
{
    UtlSListIterator iterator(mTimers);
    OsTimer* timer = NULL;

    while ((timer = (OsTimer*)iterator()))
    {
        timer->stop();
    }
}


void SipTransaction::cancel(SipUserAgent& userAgent,
                            SipTransactionList& transactionList)
{
    if(mIsServerTransaction)
    {
        // Should not get here this is only for kids (i.e. child client transactions).
        Os::Logger::instance().log(FAC_SIP, PRI_ERR, 
                      "SipTransaction::cancel "
                      "called on server transaction");
    }

    else if(!mIsCanceled)
    {
        mIsCanceled = TRUE;

        if(mpRequest)
        {
            if(mpCancel)
            {
                Os::Logger::instance().log(FAC_SIP, PRI_ERR,
                              "SipTransaction::cancel"
                              " cancel request already exists");
            }
            // Do not send CANCELs for non-INVITE transactions
            // (all the other state stuff should be done)
            else if(mTransactionState == TRANSACTION_PROCEEDING
                    && mIsDnsSrvChild
                    && mRequestMethod.compareTo(SIP_INVITE_METHOD) == 0)
            {
                // We can only send a CANCEL if we have heard
                // back a provisional response.  If we have a
                // final response it is too late
                SipMessage cancel;
                cancel.setCancelData(mpRequest, &mCancelReasonValue);
#ifdef TEST_PRINT
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                              "SipTransaction::cancel "
                              "sending CANCEL");
#endif
                handleOutgoing(cancel,
                               userAgent,
                               transactionList,
                               MESSAGE_CANCEL);
            }

            //if(mIsRecursing)
            {
                cancelChildren(userAgent,
                               transactionList);
            }
        }

        // If this transaction has been initiated (i.e. request
        // has been created and sent)
        else if(mTransactionState != TRANSACTION_LOCALLY_INIITATED)
        {
            Os::Logger::instance().log(FAC_SIP, PRI_ERR,
                          "SipTransaction::cancel "
                          "no request");
        }

    }
    else
    {
        Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                      "SipTransaction::cancel "
                      "already canceled");
    }
}

void SipTransaction::cancelChildren(SipUserAgent& userAgent,
                                    SipTransactionList& transactionList)
{
    // Cancel all the child transactions
    UtlSListIterator iterator(mChildTransactions);
    SipTransaction* childTransaction = NULL;
    while ((childTransaction = (SipTransaction*) iterator()))
    {
       // Append the Reason value of this transaction to any value we
       // already have for the child transaction.
       if (!mCancelReasonValue.isNull())
       {
          if (!childTransaction->mCancelReasonValue.isNull())
          {
             childTransaction->mCancelReasonValue.append(", ");
          }
          childTransaction->mCancelReasonValue.append(mCancelReasonValue.data());
       }
       childTransaction->cancel(userAgent,
                                transactionList);
    }
}

void SipTransaction::unlinkChild(SipTransaction* pChild)
{
  mChildTransactions.removeReference(pChild);
}

void SipTransaction::linkChild(SipTransaction& newChild)
{
    if (newChild.mpParentTransaction)
    {
        Os::Logger::instance().log(FAC_SIP, PRI_WARNING, 
                      "SipTransaction::linkChild "
                      "child.parent is not NULL");
    }
    newChild.mpParentTransaction = this;
    newChild.mIsBusy = mIsBusy;

    if (mChildTransactions.containsReference(&newChild))
    {
        Os::Logger::instance().log(FAC_SIP, PRI_WARNING, 
                      "SipTransaction::linkChild "
                      "child already a child");
    }
    else
    {
        // The children are supposed to be sorted by Q value, largest first
        UtlSListIterator iterator(mChildTransactions);
        SipTransaction* childTransaction = NULL;
        UtlBoolean childInserted = FALSE;
        int sortIndex = 0;

        while (!childInserted && (childTransaction = (SipTransaction*) iterator()))
        {
            if(childTransaction->mQvalue < newChild.mQvalue)
            {
                mChildTransactions.insertAt(sortIndex, &newChild);
                childInserted = TRUE;
            }
            else
            {
               sortIndex++;
            }
        }

        // It goes last
        if (!childInserted)
        {
           mChildTransactions.append(&newChild);
        }
    }

    if(mIsServerTransaction && mIsUaTransaction)
    {
        mIsUaTransaction = FALSE;
        Os::Logger::instance().log(FAC_SIP, PRI_WARNING, 
                      "SipTransaction::linkChild"
                      " converting server UA transaction to server proxy transaction");
    }
#ifdef DUMP_TRANSACTIONS
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                  "SipTransaction::linkChild dumps whole TransactionTree");
    justDumpTransactionTree();
#endif
}

void SipTransaction::dumpTransactionTree(UtlString& dumpstring,
                                         UtlBoolean dumpMessagesAlso)
{
    SipTransaction* parent = getTopMostParent();
    if(parent == NULL) parent = this;

    if(parent)
    {
        parent->toString(dumpstring, dumpMessagesAlso);
        parent->dumpChildren(dumpstring, dumpMessagesAlso);
    }
}


void SipTransaction::justDumpTransactionTree(void)
{
    UtlString transTree;
    SipTransaction* parent = getTopMostParent();

    if (!parent)
    {
        parent = this;
    }
    parent->dumpTransactionTree(transTree, FALSE);
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                  "SipTransactionList::justDumpTransactionTree"
                  " transaction: %p "
                  " top-most transaction tree: %s",
                  this,
                  transTree.data()
                  );
}

void SipTransaction::dumpChildren(UtlString& dumpstring,
                                  UtlBoolean dumpMessagesAlso)
{
    UtlSListIterator iterator(mChildTransactions);
    SipTransaction* childTransaction = NULL;
    UtlString childString;

    while ((childTransaction = (SipTransaction*) iterator()))
    {
        // Dump Child
        childString.remove(0);
        childTransaction->toString(childString, dumpMessagesAlso);
        dumpstring.append(childString);

        // Dump childred recursively
        childString.remove(0);
        childTransaction->dumpChildren(childString, dumpMessagesAlso);
        dumpstring.append(childString);
    }
}

void SipTransaction::toString(UtlString& dumpString,
                              UtlBoolean dumpMessagesAlso)
{
    char numBuffer[64];

    dumpString.append("  SipTransaction dump:\n\tthis: ");
    sprintf(numBuffer, "%p", this);
    dumpString.append(numBuffer);

    dumpString.append("\n\thash: ");
    dumpString.append(this->data());
    dumpString.append("\n\tmCallId: ");
    dumpString.append(mCallId);
    dumpString.append("\n\tmpBranchId->data(): ");
    dumpString.append(mpBranchId->data());
    dumpString.append("\n\tmRequestUri: ");
    dumpString.append(mRequestUri);
    dumpString.append("\n\tmSendToAddress: ");
    dumpString.append(mSendToAddress);
    dumpString.append("\n\tmSendToPort: ");
    sprintf(numBuffer, "%d", mSendToPort);
    dumpString.append(numBuffer);
    dumpString.append("\n\tmSendToProtocol: ");
    UtlString protocolString;
    SipMessage::convertProtocolEnumToString(mSendToProtocol,
        protocolString);
    dumpString.append(protocolString);
    dumpString.append("\n\tmCancelReasonValue: ");
    dumpString.append(mCancelReasonValue.data());
    //sprintf(numBuffer, "%d", mSendToProtocol);
    //dumpString.append(numBuffer);

    if(mpDnsDestinations)
    {
        dumpString.append("\n\tmpDnsSrvRecords:\n\t\tPref\tWt\tType\tName(ip):Port");
        UtlString srvName;
        UtlString srvIp;
        char srvRecordNums[128];
        for (int i=0; mpDnsDestinations[i].isValidServerT(); i++)
        {
           mpDnsDestinations[i].getHostNameFromServerT(srvName);
           mpDnsDestinations[i].getIpAddressFromServerT(srvIp);
           sprintf(srvRecordNums, "\n\t\t%d\t%d\t%d\t",
                   mpDnsDestinations[i].getPriorityFromServerT(),
                   mpDnsDestinations[i].getWeightFromServerT(),
                   mpDnsDestinations[i].getProtocolFromServerT());
           dumpString.append(srvRecordNums);
           dumpString.append(srvName);
           dumpString.append("(");
           dumpString.append(srvIp);
           sprintf(srvRecordNums, "):%d",
                   mpDnsDestinations[i].getPortFromServerT());
           dumpString.append(srvRecordNums);
        }
    }
    else
    {
        dumpString.append("\n\tmpDnsSrvRecords: NULL");
    }

    dumpString.append("\n\tmFromField: ");
    dumpString.append(mFromField.toString());
    dumpString.append("\n\tmToField: ");
    dumpString.append(mToField.toString());
    dumpString.append("\n\tmRequestMethod: ");
    dumpString.append(mRequestMethod);
    dumpString.append("\n\tmCseq: ");
    sprintf(numBuffer, "%d", mCseq);
    dumpString.append(numBuffer);
    dumpString.append("\n\tmIsServerTransaction: ");
    dumpString.append(mIsServerTransaction ? "TRUE" : " FALSE");
    dumpString.append("\n\tmIsUaTransaction: ");
    dumpString.append(mIsUaTransaction ? "TRUE" : " FALSE");

    UtlString msgString;
    ssize_t len;

    dumpString.append("\n\tmpRequest: ");
    if(mpRequest && dumpMessagesAlso)
    {
        mpRequest->getBytes(&msgString, &len);
        dumpString.append("\n==========>\n");
        dumpString.append(msgString);
        dumpString.append("\n==========>\n");
    }
    else
    {
        sprintf(numBuffer, "%p", mpRequest);
        dumpString.append(numBuffer);
    }

    dumpString.append("\n\tmpLastProvisionalResponse: ");
    if(mpLastProvisionalResponse && dumpMessagesAlso)
    {
        mpLastProvisionalResponse->getBytes(&msgString, &len);
        dumpString.append("\n==========>\n");
        dumpString.append(msgString);
        dumpString.append("\n==========>\n");
    }
    else
    {
        if (mpLastProvisionalResponse)
        {
            sprintf(numBuffer, "%d ", mpLastProvisionalResponse->getResponseStatusCode());
            dumpString.append(numBuffer);
        }
        sprintf(numBuffer, "%p", mpLastProvisionalResponse);
        dumpString.append(numBuffer);
    }

    dumpString.append("\n\tmpLastFinalResponse: ");
    if(mpLastFinalResponse && dumpMessagesAlso)
    {
        mpLastFinalResponse->getBytes(&msgString, &len);
        dumpString.append("\n==========>\n");
        dumpString.append(msgString);
        dumpString.append("\n==========>\n");
    }
    else
    {
        if (mpLastFinalResponse)
        {
            sprintf(numBuffer, "%d ", mpLastFinalResponse->getResponseStatusCode());
            dumpString.append(numBuffer);
        }
        sprintf(numBuffer, "%p", mpLastFinalResponse);
        dumpString.append(numBuffer);
    }

    dumpString.append("\n\tmpAck: ");
    if(mpAck && dumpMessagesAlso)
    {
        mpAck->getBytes(&msgString, &len);
        dumpString.append("\n==========>\n");
        dumpString.append(msgString);
        dumpString.append("\n==========>\n");
    }
    else
    {
        sprintf(numBuffer, "%p", mpAck);
        dumpString.append(numBuffer);
    }

    dumpString.append("\n\tmpCancel: ");
    if(mpCancel && dumpMessagesAlso)
    {
        mpCancel->getBytes(&msgString, &len);
        dumpString.append("\n==========>\n");
        dumpString.append(msgString);
        dumpString.append("\n==========>\n");
    }
    else
    {
        sprintf(numBuffer, "%p", mpCancel);
        dumpString.append(numBuffer);
    }

    dumpString.append("\n\tmpCancelResponse: ");
    if(mpCancelResponse && dumpMessagesAlso)
    {
        mpCancelResponse->getBytes(&msgString, &len);
        dumpString.append("\n==========>\n");
        dumpString.append(msgString);
        dumpString.append("\n==========>\n");
    }
    else
    {
        if (mpCancelResponse)
        {
            sprintf(numBuffer, "%d", mpCancelResponse->getResponseStatusCode());
            dumpString.append(numBuffer);
        }
        sprintf(numBuffer, "%p", mpCancelResponse);
        dumpString.append(numBuffer);
    }

    dumpString.append("\n\tmpParentTransaction: ");
    sprintf(numBuffer, "%p", mpParentTransaction);
    dumpString.append(numBuffer);

    UtlSListIterator iterator(mChildTransactions);
    SipTransaction* childTransaction = NULL;
    int childCount = 0;
    while ((childTransaction = (SipTransaction*) iterator()))
    {
        dumpString.append("\n\tmChildTransactions[");
        sprintf(numBuffer, "%d]: %p", childCount, childTransaction);
        dumpString.append(numBuffer);
        childCount++;
    }
    if(childCount == 0)
    {
        dumpString.append("\n\tmChildTransactions: none");
    }

    dumpString.append("\n\tmTransactionCreateTime: ");
    sprintf(numBuffer, "%ld", mTransactionCreateTime);
    dumpString.append(numBuffer);

    dumpString.append("\n\tmTransactionStartTime: ");
    sprintf(numBuffer, "%ld", mTransactionStartTime);
    dumpString.append(numBuffer);

    dumpString.append("\n\tmTimeStamp: ");
    sprintf(numBuffer, "%ld", mTimeStamp);
    dumpString.append(numBuffer);

    dumpString.append("\n\tmTransactionState: ");
    dumpString.append(stateString(mTransactionState));

    dumpString.append("\n\tmIsCanceled: ");
    dumpString.append(mIsCanceled ? "TRUE" : " FALSE");

    dumpString.append("\n\tmIsRecursing: ");
    dumpString.append(mIsRecursing ? "TRUE" : " FALSE");

    dumpString.append("\n\tmIsDnsSrvChild: ");
    dumpString.append(mIsDnsSrvChild ? "TRUE" : " FALSE");

    dumpString.append("\n\tmProvisionalSdp: ");
    dumpString.append(mProvisionalSdp ? "TRUE" : " FALSE");

    dumpString.append("\n\tmProvoExtendsTimer: ");
    dumpString.append(mProvoExtendsTimer ? "TRUE" : "FALSE");

    dumpString.append("\n\tmQvalue: ");
    sprintf(numBuffer, "%lf", mQvalue);
    dumpString.append(numBuffer);

    dumpString.append("\n\tmExpires: ");
    sprintf(numBuffer, "%d", mExpires);
    dumpString.append(numBuffer);

    dumpString.append("\n\tmIsBusy: ");
    sprintf(numBuffer, "%d", mIsBusy);
    dumpString.append(numBuffer);

    dumpString.append("\n\tmBusyTaskName: ");
    dumpString.append(mBusyTaskName);

    dumpString.append("\n\tmWaitingList: ");
    sprintf(numBuffer, "%p ", mWaitingList);
    dumpString.append(numBuffer);
    if(mWaitingList)
    {
       sprintf(numBuffer, "(%ld)", (long)mWaitingList->entries());
        dumpString.append(numBuffer);
    }

    dumpString.append("\n");
}


void SipTransaction::notifyWhenAvailable(OsEvent* availableEvent)
{
    SipTransaction* parent = getTopMostParent();
    if(parent == NULL) parent = this;

    if(parent && availableEvent)
    {
        if(parent->mWaitingList == NULL)
        {
            parent->mWaitingList = new UtlSList();
        }

        UtlSList* list = parent->mWaitingList;

        UtlVoidPtr* eventNode = new UtlVoidPtr(availableEvent);

        list->append(eventNode);
    }
    else
    {
        Os::Logger::instance().log(FAC_SIP, PRI_ERR, 
                      "SipTransaction::notifyWhenAvailable"
                      " parent: %p avialableEvent: %p",
                      parent, availableEvent);
    }
}

void SipTransaction::signalNextAvailable()
{
    SipTransaction* parent = getTopMostParent();
    if(parent == NULL) parent = this;

    if(parent && parent->mWaitingList)
    {
        // Remove the first event that is waiting for this transaction
        UtlVoidPtr* eventNode = (UtlVoidPtr*) parent->mWaitingList->get();

        if(eventNode)
        {
            OsEvent* waitingEvent = (OsEvent*) eventNode->getValue();

            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, 
                          "SipTransaction::signalNextAvailable"
                          " %p signaling: %p",
                          parent, waitingEvent);

            if(waitingEvent)
            {
                // If the event is already signaled, the other side
                // gave up waiting, so this side needs to free up
                // the event.
                if(waitingEvent->signal(1) == OS_ALREADY_SIGNALED)
                {
                    delete waitingEvent;
                    waitingEvent = NULL;
                }
            }
            delete eventNode;
            eventNode = NULL;
        }
    }
}

void SipTransaction::signalAllAvailable()
{
    SipTransaction* parent = getTopMostParent();
    if(parent == NULL) parent = this;

    if(parent && parent->mWaitingList)
    {
        UtlSList* list = parent->mWaitingList;
        // Remove the first event that is waiting for this transaction
        UtlVoidPtr* eventNode = NULL;
        while ((eventNode = (UtlVoidPtr*) list->get()))
        {
            if(eventNode)
            {
                OsEvent* waitingEvent = (OsEvent*) eventNode->getValue();

                if(waitingEvent)
                {
                    // If the event is already signaled, the other side
                    // gave up waiting, so this side needs to free up
                    // the event.
                    if(waitingEvent->signal(1) == OS_ALREADY_SIGNALED)
                    {
                        delete waitingEvent;
                        waitingEvent = NULL;
                    }
                }
                delete eventNode;
                eventNode = NULL;
            }
        }
    }
}


/* ============================ ACCESSORS ================================= */

const char* SipTransaction::stateString(enum transactionStates state)
{
   const char* stateStrings[NUM_TRANSACTION_STATES+1] =
      {
         "UNKNOWN",
         "LOCALLY_INITIATED",
         "CALLING",
         "PROCEEDING",
         "COMPLETE",
         "CONFIRMED",
         "TERMINATED",
         "UNDEFINED"
      };
   return (state >= TRANSACTION_UNKNOWN && state < NUM_TRANSACTION_STATES)
      ? stateStrings[state]
      : stateStrings[NUM_TRANSACTION_STATES] /* UNDEFINED */;
}

const char* SipTransaction::relationshipString(enum messageRelationship relationship)
{
   const char* relationshipStrings[NUM_RELATIONSHIPS+1] =
      {
         "UNKNOWN",
         "UNRELATED",
         "SAME_SESSION",
         "DIFFERENT_BRANCH",
         "REQUEST",
         "PROVISIONAL",
         "FINAL",
         "NEW_FINAL",
         "CANCEL",
         "CANCEL_RESPONSE",
         "ACK",
         "2XX_ACK",
         "MESSAGE_2XX_ACK_PROXY",
         "DUPLICATE",
         "UNDEFINED"
      };
   return (relationship >= MESSAGE_UNKNOWN && relationship < NUM_RELATIONSHIPS)
      ? relationshipStrings[relationship]
      : relationshipStrings[NUM_RELATIONSHIPS] /* UNDEFINED */;
}

void SipTransaction::buildHash(const SipMessage& message,
                              UtlBoolean isOutgoing,
                              UtlString& hash)
{
    UtlBoolean isServerTransaction =
        message.isServerTransaction(isOutgoing);

    message.getCallIdField(&hash);
    hash.append(isServerTransaction ? 's' : 'c');

    int cSeq;
    //UtlString method;
    message.getCSeqField(&cSeq, NULL /*&method*/);
    char cSeqString[20];
    sprintf(cSeqString, "%d", cSeq);
    hash.append(cSeqString);
}

SipTransaction* SipTransaction::getTopMostParent() const
{
    SipTransaction* topParent = NULL;
    if(mpParentTransaction)
    {
        topParent = mpParentTransaction->getTopMostParent();

        if(topParent == NULL)
        {
            topParent = mpParentTransaction;
        }
    }

    return(topParent);
}

void SipTransaction::getCallId(UtlString& callId) const
{
    callId = mCallId;
}

enum SipTransaction::transactionStates SipTransaction::getState() const
{
    return(mTransactionState);
}

/*long SipTransaction::getStartTime() const
{
    return(mTransactionStartTime);
}*/

long SipTransaction::getTimeStamp() const
{
    return(mTimeStamp);
}

void SipTransaction::touch()
{
    // We touch the whole parent-child tree so that
    // none of transactions get garbage collected
    // until they are all stale.  This saves checking
    // up and down the tree during garbage collection
    // to see if there are any still active transactions.

    SipTransaction* topParent = getTopMostParent();

    // We end up setting the date twice on this
    // transaction if it is not the top most parent
    // but so what.  The alternative is using a local
    // variable to hold the date.  There is no net
    // savings.
    OsTime time;
    OsDateTime::getCurTimeSinceBoot(time);

    //
    // if the transaction is already in completed state, do not reset the timestamp
    //
    if (getState() < TRANSACTION_COMPLETE)
        mTimeStamp = time.seconds();

    //osPrintf("SipTransaction::touch seconds: %ld usecs: %ld\n",
    //    time.seconds(), time.usecs());
    //mTimeStamp = OsDateTime::getSecsSinceEpoch();

    if(topParent)
    {
        topParent->touchBelow(mTimeStamp);
    }
    else
    {
        touchBelow(mTimeStamp);
    }
}

void SipTransaction::touchBelow(int newDate)
{
    //
    // if the transaction is already in completed state, do not reset the timestamp
    //
    if (getState() < TRANSACTION_COMPLETE)
      mTimeStamp = newDate;

#ifdef TEST_TOUCH
    UtlString serialized;
    toString(serialized, FALSE);
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                  "SipTransaction::touchBelow "
                  "'%s'",
                  serialized.data());
#endif // TEST_TOUCH

    SipTransaction* child = NULL;
    UtlSListIterator iterator(mChildTransactions);
    while((child = (SipTransaction*) iterator()))
    {
        child->touchBelow(newDate);
    }
}


SipMessage* SipTransaction::getRequest()
{
    return(mpRequest);
}

SipMessage* SipTransaction::getLastProvisionalResponse()
{
    return(mpLastProvisionalResponse);
}

SipMessage* SipTransaction::getLastFinalResponse()
{
    return(mpLastFinalResponse);
}

void SipTransaction::markBusy()
{
#   ifdef LOG_TRANSLOCK
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, 
                  "SipTransaction::markBusy %p", this);
#   endif

    if(mpParentTransaction)
    {
        mpParentTransaction->markBusy();
    }
    else
    {
        OsTime time;
        OsDateTime::getCurTimeSinceBoot(time);
        int busyTime = time.seconds();
        // Make sure it is not equal to zero
        if(!busyTime)
        {
            busyTime++;
        }
        doMarkBusy(busyTime);

        OsTask* busyTask = OsTask::getCurrentTask();
        if(busyTask)
        {
            mBusyTaskName = busyTask->getName();
        }
        else
        {
            mBusyTaskName = "";
        }
    }
}

void SipTransaction::doMarkBusy(int markData)
{
#   ifdef LOG_TRANSLOCK
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                  "SipTransaction::doMarkBusy(%d) %p", markData, this);
#   endif

    mIsBusy = markData;

    // Recurse through the children and mark them busy
    UtlSListIterator iterator(mChildTransactions);
    SipTransaction* childTransaction = NULL;
    while ((childTransaction = (SipTransaction*) iterator()))
    {
        childTransaction->doMarkBusy(markData);
    }
}

void SipTransaction::markAvailable()
{
#   ifdef LOG_TRANSLOCK
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                  "SipTransaction::markAvailable %p", this);
#   endif

    // Recurse to the parent
    if(mpParentTransaction) mpParentTransaction->markAvailable();

    // This is the top most parent
    else
    {
        touch(); //set the last used time for parent and all children
        doMarkBusy(0);
        signalNextAvailable();
    }
}

/*void SipTransaction::doMarkAvailable()
{
    mIsBusy = FALSE;

    // Recurse through the children and mark them available
    UtlSListIterator iterator(mChildTransactions);
    SipTransaction* childTransaction = NULL;
    while(childTransaction = (SipTransaction*) iterator())
    {
        childTransaction->doMarkAvailable();
    }
}
*/

/* ============================ INQUIRY =================================== */

UtlBoolean SipTransaction::isServerTransaction() const
{
    return(mIsServerTransaction);
}

UtlBoolean SipTransaction::isDnsSrvChild() const
{
    return(mIsDnsSrvChild);
}

UtlBoolean SipTransaction::isUaTransaction() const
{
    return(mIsUaTransaction);
}

UtlBoolean SipTransaction::isChildSerial()
{
    // The child transactions are supposed to be sorted by
    // Q value.  So if we look at the first and last and they
    // are different then there are serially searched children

    UtlBoolean isSerial = FALSE;
    SipTransaction* child = (SipTransaction*)mChildTransactions.first();
    if(child)
    {
        double q1 = child->mQvalue;

        child = (SipTransaction*)mChildTransactions.last();
        if(child)
        {
            double q2 = child->mQvalue;
            if((q1-q2)*(q1-q2) > MIN_Q_DELTA_SQUARE)
            {
                isSerial = TRUE;
            }
        }

    }

    return(isSerial);
}

UtlBoolean SipTransaction::isEarlyDialogWithMedia()
{
    UtlBoolean earlyDialogWithMedia = FALSE;

    if (mProvisionalSdp
        && mTransactionState > TRANSACTION_LOCALLY_INIITATED
        && mTransactionState < TRANSACTION_COMPLETE)
    {
        earlyDialogWithMedia = TRUE;

        // This should not occur, the state should be ?TERMINATED?
        if(mIsCanceled)
        {
            Os::Logger::instance().log(FAC_SIP, PRI_ERR, 
                          "SipTransaction::isEarlyDialogWithMedia"
                          " transaction state: %s incorrect for canceled transaction",
                          stateString(mTransactionState));
        }

        // This should not occur, the state should be COMPLETE or CONFIRMED
        if (mIsRecursing)
        {
           Os::Logger::instance().log(FAC_SIP, PRI_ERR, 
                         "SipTransaction::isEarlyDialogWithMedia"
                         " transaction state: %s incorrect for recursing transaction",
                          stateString(mTransactionState));
        }
    }

    return(earlyDialogWithMedia);
}

UtlBoolean SipTransaction::isChildEarlyDialogWithMedia()
{
    UtlBoolean earlyDialogWithMedia = FALSE;
    UtlSListIterator iterator(mChildTransactions);
    SipTransaction* childTransaction = NULL;

    while ((childTransaction = (SipTransaction*) iterator()))
    {
        // If the state is initiated, no request has been sent for this transaction and
        // this and all other children after this one in the list should be in the same state
        if (childTransaction->mTransactionState == TRANSACTION_LOCALLY_INIITATED)
        {
            break;
        }

        earlyDialogWithMedia = childTransaction->isEarlyDialogWithMedia();
    }

    return(earlyDialogWithMedia);
}

UtlBoolean SipTransaction::isMethod(const char* methodToMatch) const
{
    return(strcmp(mRequestMethod.data(), methodToMatch) == 0);
}

enum SipTransaction::messageRelationship
SipTransaction::whatRelation(const SipMessage& message,
                             UtlBoolean isOutgoing) const
{
    enum messageRelationship relationship;
#   ifdef LOG_FORKING
    int matchClause;
#   define SET_RELATIONSHIP(r) \
    {                          \
       relationship = r;       \
       matchClause = __LINE__; \
    }
#   else
#   define SET_RELATIONSHIP(r) \
    {                          \
       relationship = r;       \
    }
#   endif
    SET_RELATIONSHIP(MESSAGE_UNKNOWN);

    UtlString msgCallId;
    message.getCallIdField(&msgCallId);

    // Note: this is nested to bail out as soon as possible
    // for efficiency reasons

#   ifdef LOG_FORKING
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                  "SipTransaction::whatRelation %s\n"
                  "\tmsg call: '%s'\n"
                  "\tcmp call: '%s'",
                  isOutgoing ? "out" : "in",
                  msgCallId.data(), mCallId.data());
#   endif

    // CallId matches
    if(mCallId.compareTo(msgCallId) == 0)
    {
        // PREP WORK
        // Matching call-id found, now get other message parts that will be needed:
        // -- CSeq, method, top-most via, branch from top-most via
        // -- Is it a response? , if available get final response code
        // After this prep, we can determine the relationship between the message and this transaction
        int msgCseq;
        UtlString msgMethod;
        message.getCSeqField(&msgCseq, &msgMethod);
        UtlBoolean isResponse = message.isResponse();
        int lastFinalResponseCode = mpLastFinalResponse ?
            mpLastFinalResponse->getResponseStatusCode() : -1;

        UtlString viaField;
        UtlString msgBranch;
        UtlBoolean msgHasVia = message.getViaFieldSubField(&viaField, 0);
        if (msgHasVia)
          SipMessage::getViaTag(viaField.data(), "branch", msgBranch);

#       ifdef LOG_FORKING
        Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                      "SipTransaction::whatRelation "
                      "call id matched\n"
                      "\tvia: '%s'\n\tbranch: '%s'",
                      viaField.data(), msgBranch.data());
#       endif

        bool branchPrefixSet = BranchId::isRFC3261(msgBranch);

        UtlBoolean toTagMatches = FALSE;
        int toTagFinalRespIsSet = -1;
        UtlBoolean fromTagMatches;
        UtlBoolean branchIdMatches = mpBranchId->equals(msgBranch);
        UtlBoolean mustCheckTags;

        // In most cases, if the 3261-branch prefix is set, we only need to match the branch parameter
        // to determine if this is the same transaction or not.
        // EXCEPT - ACK to INVITEs with 200 response are really a different transaction.
        // For convenience we consider them to be the same.
        // EXCEPT - CANCEL is also a different transaction that we store in the same SipTransaction object.

        // These are the cases without the 3261 branch prefix and the ACK/CANCEL cases
        if(!branchPrefixSet                                         // no RFC3261 branch id
           || (!isResponse                                          // OR message is a request
               && (msgMethod.compareTo(SIP_CANCEL_METHOD) == 0          //  which is a CANCEL request
                   || (msgMethod.compareTo(SIP_ACK_METHOD) == 0         // OR an ACK request 
                       && lastFinalResponseCode < SIP_3XX_CLASS_CODE    // AND this transaction never got error response
                       && lastFinalResponseCode >= SIP_2XX_CLASS_CODE)  // (ie, 200 ACK)
                   || (!mIsServerTransaction                        // or ???
                       && mTransactionState == TRANSACTION_LOCALLY_INIITATED))))    // this value is only set in constructor
        {
            // Must do expensive tag matching in these cases
            // parse the To and From fields into a Url object.
            mustCheckTags = TRUE;

            Url msgFrom;
            UtlString msgFromTag;
            UtlString fromTag;
            message.getFromUrl(msgFrom);
            msgFrom.getFieldParameter("tag", msgFromTag);

            Url *pFromUrl = (Url *)&mFromField;
            pFromUrl->getFieldParameter("tag", fromTag);
            fromTagMatches = msgFromTag.compareTo(fromTag) == 0;
        }
        else
        {
            // has RFC3261-style branch prefix, we can just check branch id
            mustCheckTags = FALSE;
            toTagMatches  = branchIdMatches;
            fromTagMatches = branchIdMatches;
        }

#       ifdef LOG_FORKING
        Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                      "SipTransaction::whatRelation  %s %s %s %s",
                      branchIdMatches ? "BranchMatch" : "NoBranchMatch",
                      mustCheckTags ? "CheckTags" : "NoCheckTags",
                      fromTagMatches ? "FromTagMatch" : "NoFromTagMatch",
                      toTagMatches ? "ToTagMatch" : "NoToTagMatch"
                      );
#       endif

        // DETERMINE RELATIONSHIP
        // Prep work is done, now check for matches...
        // Check from-Tag(or branch) then to-tag(or branch)
        if(fromTagMatches)      // or rfc-3261 branch match
        {
            UtlString msgToTag;
            // We don't check for to-tag matches until we know the from-tag matches.
            // We don't check either tag if we can determine a match from the (rfc-3261) branch.
            // Tharsing the To field into a Url is expensive so we avoid it if we can.
            if(mustCheckTags)   // ie. message doesn't have rfc-3261 style branch
            {
                Url msgTo;

                UtlString toTag;
                message.getToUrl(msgTo);
                msgTo.getFieldParameter("tag", msgToTag);

                Url *pToUrl = (Url *)&mToField;
                pToUrl->getFieldParameter("tag", toTag);

                toTagMatches = (toTag.isNull() || toTag.compareTo(msgToTag) == 0);
            }

            // To field tag or rfc-3261 branch match
            if(toTagMatches)
            {
                if(mCseq == msgCseq)
                {
                    UtlBoolean isMsgServerTransaction = message.isServerTransaction(isOutgoing);
                    // The message and this transaction are both part of either a server or a client transaction
                    if(isMsgServerTransaction == mIsServerTransaction)
                    {
                        UtlString finalResponseToTag;
                        // Make sure final response has a To-tag (exception: rfc-3261 via branch matches)
                        if (mpLastFinalResponse && mustCheckTags)
                        {
                            Url responseTo;
                            mpLastFinalResponse->getToUrl(responseTo);
                            toTagFinalRespIsSet = responseTo.getFieldParameter("tag", finalResponseToTag);
                        }

                        UtlString msgUri;
                        UtlBoolean parentBranchIdMatches = FALSE;
                        if (!isResponse                                              // request
                            && !mIsServerTransaction                                 // AND client transaction
                            && mTransactionState == TRANSACTION_LOCALLY_INIITATED)   // ???
                        {
                            SipTransaction* parent = getTopMostParent();
                            // Should this matter whether it is a server or client TX?
                            if (parent 
                                && parent->mIsServerTransaction 
                                && parent->mpBranchId->equals(msgBranch))
                            {
                                // We know this is a request that originated from
                                // this client transaction's parent's server          // TBD kme ???
                                // transaction.
                                parentBranchIdMatches = TRUE;
                                message.getRequestUri(&msgUri);
                            }
                            else if ((parent == NULL) && !msgHasVia)
                            {
                                // This is a client transaction with no parent and
                                // it originated from this UA because there are no
                                // vias
                                parentBranchIdMatches = TRUE;
                                message.getRequestUri(&msgUri);
                            }
                        }
                        Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                                      "SipTransaction::whatRelation "
                                      "toTagFinalRespIsSet %d", toTagFinalRespIsSet);
#if 0
                        Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                                      "SipTransaction::whatRelation "
                                      "complicated %d%d%d %d%d%d%d %d '%s' '%s' '%s'",
                                      branchIdMatches, parentBranchIdMatches, msgUri.compareTo(mRequestUri),
                                      mIsUaTransaction, mIsServerTransaction, isResponse, msgHasVia, toTagFinalRespIsSet,
                                      finalResponseToTag.data(), msgToTag.data(), msgMethod.data());
#endif
                        // Check to see if -
                        // the branch of this message matches the transaction
                        // OR the message branch matches the parent's branch AND the RequestURI matches
                        // OR message is an ACK request for this client UA(no more vias) which had a 2xx final response
                        // OR message is an ACK request for this server which had a 2xx final response
                        // OR message is an ACK request which had a 2xx final response, to be forwarded by proxy
                        // OR message is CANCEL request for this client UA(no more vias)

                        // This is the request for this client transaction and the Via had not been added yet for this
                        // transaction.  So the branch is not there yet.
                        if (branchIdMatches
                           || (parentBranchIdMatches 
                               && msgUri.compareTo(mRequestUri) == 0)
                           || (mIsUaTransaction             // UA client ACK transaction for 2xx
                               && !mIsServerTransaction 
                               && !isResponse 
                               && !msgHasVia 
                               && msgMethod.compareTo(SIP_ACK_METHOD) == 0 
                               && lastFinalResponseCode < SIP_3XX_CLASS_CODE 
                               && lastFinalResponseCode >= SIP_2XX_CLASS_CODE)
                           || (mIsUaTransaction             // UA server ACK transaction for 2xx
                               && mIsServerTransaction 
                               && !isResponse 
                               && msgMethod.compareTo(SIP_ACK_METHOD) == 0 
                               && lastFinalResponseCode < SIP_3XX_CLASS_CODE 
                               && lastFinalResponseCode >= SIP_2XX_CLASS_CODE 
                               && finalResponseToTag.compareTo(msgToTag) == 0 )
                           || (!mIsUaTransaction            // proxy forwards ACK transaction for 2xx
                               //&& mIsServerTransaction 
                               && !isResponse 
                               && msgMethod.compareTo(SIP_ACK_METHOD) == 0 
                               && lastFinalResponseCode < SIP_3XX_CLASS_CODE 
                               && lastFinalResponseCode >= SIP_2XX_CLASS_CODE )
                           || (mIsUaTransaction             // UA client CANCEL transaction
                               && !mIsServerTransaction 
                               && !isResponse 
                               && !msgHasVia 
                               && msgMethod.compareTo(SIP_CANCEL_METHOD) == 0))
                        {
                            if (isResponse)
                            {
                                int msgResponseCode = message.getResponseStatusCode();

                                // Provisional responses
                                if(msgResponseCode < SIP_2XX_CLASS_CODE)
                                {
                                    SET_RELATIONSHIP(MESSAGE_PROVISIONAL);
                                }

                                // Final responses
                                else
                                {
                                    if(msgMethod.compareTo(SIP_ACK_METHOD) == 0)
                                    {
                                        Os::Logger::instance().log(FAC_SIP, PRI_ERR,
                                                      "SipTransaction::whatRelation "
                                                      " ACK response");
                                    }

                                    else if (msgMethod.compareTo(SIP_CANCEL_METHOD) == 0)
                                    {
                                        SET_RELATIONSHIP(MESSAGE_CANCEL_RESPONSE);
                                    }

                                    else if (mpLastFinalResponse)
                                    {
                                        int finalResponseCode = mpLastFinalResponse->getResponseStatusCode();
                                        if (finalResponseCode == msgResponseCode)
                                        {
                                            if (msgMethod.compareTo(SIP_INVITE_METHOD) == 0)
                                            {
                                                // for INVITE, check if to-tag matches
                                                if (!mustCheckTags)
                                                {
                                                    Url msgTo;

                                                    message.getToUrl(msgTo);
                                                    msgTo.getFieldParameter("tag", msgToTag);

                                                    UtlString toTag;
                                                    Url toUrl;
                                                    mpLastFinalResponse->getToUrl(toUrl);
                                                    toUrl.getFieldParameter("tag", toTag);

                                                    toTagMatches = (toTag.isNull() || toTag.compareTo(msgToTag) == 0);
                                                }

                                                if (toTagMatches)
                                                {
#ifdef TEST_PRINT
                                                    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                                                                  "SipTransaction::whatRelation "
                                                                  "DUPLICATED msg finalResponseCode - %d toTagMatches %d",
                                                                  finalResponseCode, toTagMatches);
#endif
                                                    relationship = MESSAGE_DUPLICATE;
                                                }
                                                else
                                                {
#ifdef TEST_PRINT
                                                    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                                                                  "SipTransaction::whatRelation "
                                                                  "MESSAGE_NEW_FINAL - finalResponseCode - %d toTagMatches %d",
                                                                  finalResponseCode, toTagMatches);
#endif
                                                    relationship = MESSAGE_NEW_FINAL;
                                                }

                                            }
                                            else
                                            {
                                                // non-invite, don't care about to-tag
#ifdef TEST_PRINT
                                               Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                                                             "SipTransaction::whatRelation "
                                                             "DUPLICATED msg finalResponseCode - %d",
                                                             finalResponseCode);
#endif
                                               relationship = MESSAGE_DUPLICATE;
                                            }
                                        }
                                        else
                                        {
                                            SET_RELATIONSHIP(MESSAGE_NEW_FINAL);
                                        }
                                    }
                                    else
                                    {
                                        SET_RELATIONSHIP(MESSAGE_FINAL);
                                    }
                                }
                            }

                            // Requests
                            else
                            {
                                if(mpRequest)
                                {
                                    UtlString previousRequestMethod;
                                    mpRequest->getRequestMethod(&previousRequestMethod);

                                    if(previousRequestMethod.compareTo(msgMethod) == 0)
                                    {
#ifdef TEST_PRINT
                                        Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                                                      "SipTransaction::whatRelation "
                                                      "DUPLICATED msg previousRequestMethod - %s",
                                                      previousRequestMethod.data());
#endif
                                        SET_RELATIONSHIP(MESSAGE_DUPLICATE);
                                    }
                                    else if(msgMethod.compareTo(SIP_ACK_METHOD) == 0)
                                    {
                                        if(mpLastFinalResponse)
                                        {
                                            int finalResponseCode = mpLastFinalResponse->getResponseStatusCode();
                                            if(finalResponseCode >= SIP_3XX_CLASS_CODE)
                                            {
                                                SET_RELATIONSHIP(MESSAGE_ACK);
                                            }
                                            else    // 2xx response
                                            {
                                                if(!mIsUaTransaction)
                                                {
                                                   // changed this to DEBUG from WARNING,
                                                   // it should be ok to get here in the new ACK version
                                                   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                                                                 "SipTransaction::whatRelation "
                                                                 " ACK matches transaction"
                                                                 " with 2XX class response");
                                                }
                                                if (mIsServerTransaction)
                                                {
                                                    SET_RELATIONSHIP(MESSAGE_2XX_ACK);
                                                }
                                                else
                                                {
#ifdef TEST_PRINT
                                                    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                                                                  "SipTransaction::whatRelation "
                                                                  "ACK PROXY is REQUEST");
#endif
                                                    SET_RELATIONSHIP(MESSAGE_2XX_ACK_PROXY);
                                                }
                                            }       // end ACK with 2xx final response
                                        }       // end ACK with valid lastFinalResponse
                                        else
                                        {
                                            Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                                                          "SipTransaction::whatRelation"
                                                          " ACK matches transaction "
                                                          "with NO final response");
                                            SET_RELATIONSHIP(MESSAGE_ACK);
                                        }
                                    }       // end ACK with mpRequest
                                    else if(msgMethod.compareTo(SIP_CANCEL_METHOD) == 0)
                                    {
                                        SET_RELATIONSHIP(MESSAGE_CANCEL);
                                    }
                                    else
                                    {
                                        SET_RELATIONSHIP(MESSAGE_DUPLICATE);
                                        Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                                                      "SipTransaction::messageRelationship"
                                                      " found %s request for transaction with %s",
                                                      msgMethod.data(),
                                                      previousRequestMethod.data());
                                    }
                                }       // no entry in mpRequest
                                else
                                {
                                    if(msgMethod.compareTo(SIP_CANCEL_METHOD) == 0)
                                    {
                                        SET_RELATIONSHIP(MESSAGE_CANCEL);
                                    }
                                    else if(msgMethod.compareTo(SIP_ACK_METHOD) == 0)
                                    {
                                        SET_RELATIONSHIP(MESSAGE_ACK);
                                    }
                                    else
                                    {
                                        SET_RELATIONSHIP(MESSAGE_REQUEST);
                                    }
                                }       // end ACK/CANCEL/request choice
                            }   // end requests that got past complex if
                        }  else { SET_RELATIONSHIP(MESSAGE_DIFFERENT_BRANCH); }   // complex test failed
                    } else { SET_RELATIONSHIP(MESSAGE_DIFFERENT_BRANCH); }     // mIsServerTransaction values don't match
                } else { SET_RELATIONSHIP(MESSAGE_SAME_SESSION); }         // no Cseq match
            } else { SET_RELATIONSHIP(MESSAGE_UNRELATED); }            // no to tag (or branch-id) match
        } else { SET_RELATIONSHIP(MESSAGE_UNRELATED); }            // no from tag (or branch-id) match
    } else { SET_RELATIONSHIP(MESSAGE_UNRELATED); }            // no callid match

#   ifdef LOG_FORKING
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                  "SipTransaction::whatRelation "
                  "returning %s set on %d"
                  ,relationshipString(relationship)
                  ,matchClause
                  );
#   endif

    return(relationship);
}

UtlBoolean SipTransaction::isBusy()
{
    return(mIsBusy);
}

#if 0 // TODO redundant?
UtlBoolean SipTransaction::isUriChild(Url& uri)
{
    UtlSListIterator iterator(mChildTransactions);
    SipTransaction* childTransaction = NULL;
    UtlBoolean childHasSameUri = FALSE;
    UtlString uriString;
    uri.getUri(uriString);

    while ((childTransaction = (SipTransaction*) iterator()))
    {
        if(uriString.compareTo(childTransaction->mRequestUri) == 0)
        {
            childHasSameUri = TRUE;
            break;
        }
    }

    return(childHasSameUri);
}
#endif

UtlBoolean SipTransaction::isUriRecursed(Url& uri)
{
    SipTransaction* parent = getTopMostParent();
    if(parent == NULL) parent = this;
    UtlString uriString;
    uri.getUri(uriString);

    return(isUriRecursedChildren(uriString));
}

UtlBoolean SipTransaction::isUriRecursedChildren(UtlString& uriString)
{
    UtlBoolean childHasSameUri = FALSE;

    UtlSListIterator iterator(mChildTransactions);
    SipTransaction* childTransaction;

    while (!childHasSameUri && (childTransaction = (SipTransaction*) iterator()))
    {
       childHasSameUri = (   childTransaction->mTransactionState > TRANSACTION_LOCALLY_INIITATED
                          && (   0 == uriString.compareTo(childTransaction->mRequestUri)
                              || isUriRecursedChildren(uriString)
                              )
                          );
    }

    return(childHasSameUri);
}

void SipTransaction::setCancelReasonValue(const char* protocol,
                                          int responseCode,
                                          const char* reasonText)
{
   mCancelReasonValue = protocol;
   mCancelReasonValue.append(";cause=");
   mCancelReasonValue.appendNumber(responseCode);
   if (reasonText && reasonText[0])
   {
      mCancelReasonValue.append(";text=\"");
      mCancelReasonValue.append(reasonText);
      mCancelReasonValue.append("\"");
   }
}

//: Determine best choice for protocol, based on message size
//  Default is UDP, returns TCP only for large messages
OsSocket::IpProtocolSocketType SipTransaction::getPreferredProtocol()
{
    ssize_t msgLength;
    UtlString msgBytes;
    OsSocket::IpProtocolSocketType retProto = OsSocket::UDP;

    if (1)
    {
        mpRequest->getBytes(&msgBytes, &msgLength);
        if (msgLength > UDP_LARGE_MSG_LIMIT)
        {
            retProto = OsSocket::TCP;
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                          "SipTransaction::getPreferredProtocol "
                          "change %d to %d for size %zd"
                          ,mSendToProtocol
                          ,retProto
                          ,msgLength
                          );
        }
    }
    return retProto;
}


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
