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

#if defined(_VXWORKS)
#   include <taskLib.h>
#   include <netinet/in.h>
#endif

#include <stdio.h>
#include <stdlib.h>


// APPLICATION INCLUDES
#include "utl/UtlHashBagIterator.h"
#include "os/OsDateTime.h"
#include "os/OsQueuedEvent.h"
#include "os/OsTimer.h"
#include "os/OsEventMsg.h"
#include "os/OsConfigDb.h"
#include "os/OsRWMutex.h"
#include "os/OsReadLock.h"
#include "os/OsWriteLock.h"
#include "net/Url.h"
#include "net/SipLineMgr.h"
#include "net/SipObserverCriteria.h"
#include "net/SipUserAgent.h"
#include "net/SipMessage.h"
#include "net/NetMd5Codec.h"
#include "net/TapiMgr.h"
#include "net/SipRefreshMgr.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC INITIALIZERS

//#define TEST_PRINT 1
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SipLineMgr::SipLineMgr() :
    OsServerTask( "SipLineMgr-%d" ),
    mpRefreshMgr (NULL),
    mObserverMutex(OsRWMutex::Q_FIFO)
{   
}

SipLineMgr::~SipLineMgr()
{
    waitUntilShutDown();

    // Do not delete the refresh manager as it was
    // created in another context and we do not know
    // who else may be using it
}

void
SipLineMgr::StartLineMgr()
{
    if(!isStarted())
    {
        // start the thread
        start();
    }
    mIsStarted = TRUE;
}

UtlBoolean
SipLineMgr::handleMessage(OsMsg &eventMessage)
{
    UtlBoolean messageProcessed = FALSE;

    int msgType = eventMessage.getMsgType();
    // int msgSubType = eventMessage.getMsgSubType();
    UtlString method;
    if(msgType == OsMsg::PHONE_APP )
    {
        const SipMessage* sipMsg =
            static_cast<const SipMessage*>(((SipMessageEvent&)eventMessage).getMessage());
        int messageType = ((SipMessageEvent&)eventMessage).getMessageStatus();

        //get line information related to this identity
        UtlString Address;
        UtlString Protocol;
        UtlString User;
        int Port;
        UtlString toUrl;
        UtlString Label;
        sipMsg->getToAddress(&Address, &Port, &Protocol, &User, &Label);
        SipMessage::buildSipUri( &toUrl , Address , Port, Protocol, User , Label);

        SipLine* line = NULL;
          Url tempUrl ( toUrl );
        line = sLineList.getLine( tempUrl );

        if ( line)
        {   // is this a request with a timeout?
            if ( !sipMsg->isResponse() && (messageType==SipMessageEvent::TRANSPORT_ERROR) )
            {
                int CSeq;
                UtlString method;
                sipMsg->getCSeqField(&CSeq , &method);
                if (CSeq == 1) //first time registration - go directly to expired state
                {
                    line->setState(SipLine::LINE_STATE_EXPIRED);
                    SipLineEvent lineEvent(line, SipLineEvent::SIP_LINE_EVENT_NO_RESPONSE,"","",0,"No Response");
                    queueMessageToObservers(lineEvent);
                }
                else
                {
                    line->setState(SipLine::LINE_STATE_FAILED);
                    SipLineEvent lineEvent(line, SipLineEvent::SIP_LINE_EVENT_FAILED,"","",0,"No Response");
                    queueMessageToObservers(lineEvent);
                }

                // Log the failure
                syslog(FAC_LINE_MGR, PRI_ERR, "failed to register line (cseq=%d, no response): %s",
                        CSeq, line->getLineId().data()) ;
            }
            else if( sipMsg->isResponse() )
            {
                int responseCode = sipMsg->getResponseStatusCode();
                UtlString sipResponseText;
                sipMsg->getResponseStatusText(&sipResponseText);

                if( (responseCode>= SIP_2XX_CLASS_CODE && responseCode < SIP_3XX_CLASS_CODE ))
                {
                    //set line state to correct
                    line->setState(SipLine::LINE_STATE_REGISTERED);
                    SipLineEvent lineEvent(line, SipLineEvent::SIP_LINE_EVENT_SUCCESS,"","",responseCode,sipResponseText);
                    queueMessageToObservers(lineEvent);

                    // Log the success
                    int CSeq;
                    UtlString method;
                    sipMsg->getCSeqField(&CSeq , &method);
                    syslog(FAC_LINE_MGR, PRI_DEBUG, "registered line (cseq=%d): %s",
                            CSeq, line->getLineId().data()) ;
                }
                else if(  responseCode >= SIP_3XX_CLASS_CODE )
                {
                    // get error codes
                    UtlString nonce;
                    UtlString opaque;
                    UtlString realm;
                    UtlString scheme;
                    UtlString algorithm;
                    UtlString qop;

                    //get realm and scheme
                    if ( responseCode == HTTP_UNAUTHORIZED_CODE)
                    {
                        sipMsg->getAuthenticateData(&scheme, &realm, &nonce, &opaque,
                                                    &algorithm, &qop, HttpMessage::SERVER);
                    }
                    else if ( responseCode == HTTP_PROXY_UNAUTHORIZED_CODE)
                    {
                        sipMsg->getAuthenticateData(&scheme, &realm, &nonce, &opaque,
                                                    &algorithm, &qop, HttpMessage::PROXY);
                    }

                    //SDUATODO: LINE_STATE_FAILED after timing mechanism in place
                    line->setState(SipLine::LINE_STATE_EXPIRED);
                    SipLineEvent lineEvent(
                        line, SipLineEvent::SIP_LINE_EVENT_FAILED,
                        realm, scheme, responseCode, sipResponseText );
                    queueMessageToObservers(lineEvent);

                    // Log the failure
                    int CSeq;
                    UtlString method;
                    sipMsg->getCSeqField(&CSeq , &method);
                    syslog(FAC_LINE_MGR, PRI_ERR, "failed to register line (cseq=%d, auth): %s\nnonce=%s, opaque=%s,\nrealm=%s,scheme=%s,\nalgorithm=%s, qop=%s",
                        CSeq, line->getLineId().data(),
                        nonce.data(), opaque.data(), realm.data(), scheme.data(), algorithm.data(), qop.data()) ;
                }
            }
            messageProcessed = TRUE;
        } // if line
        line = NULL;
    }
    return  messageProcessed ;
}


UtlBoolean
SipLineMgr::addLine(SipLine&   line,
                    UtlBoolean doEnable)
{
    UtlBoolean added = FALSE;
    // check if it is a duplicate url
    if (!sLineList.isDuplicate(&line))
    {
        addToList(&line);
        if (line.getState() == SipLine::LINE_STATE_REGISTERED)
        {
            if (doEnable)
            {
               enableLine(line.getIdentity());
            }
        }
        added = TRUE;
        SipLineEvent lineEvent(&line, SipLineEvent::SIP_LINE_EVENT_LINE_ADDED);
        queueMessageToObservers(lineEvent);

        syslog(FAC_LINE_MGR, PRI_INFO, "SipLineMgr::addLine added line: %s",
                line.getIdentity().toString().data()) ;
    }

    return added;
}

void
SipLineMgr::deleteLine(const Url& identity)
{
    SipLine *line = NULL;
    SipLine *pDeleteLine = NULL ;

    line = sLineList.getLine(identity) ;
    if (line == NULL)
    {
        syslog(FAC_LINE_MGR, PRI_ERR, "SipLineMgr::deleteLine unable to delete line (not found): %s",
                identity.toString().data()) ;
       return;
    }

    if (line->getState() == SipLine::LINE_STATE_REGISTERED )
    {
        //add to temporary list - needed if chanllenged for credentials
        addToTempList(line);
        disableLine(identity, 0, identity.toString());
    }
    else
    {
        removeFromList(line);
        pDeleteLine = line ;
    }

    // notify the observers that the line was deleted
    SipLineEvent lineEvent( line, SipLineEvent::SIP_LINE_EVENT_LINE_DELETED );
    queueMessageToObservers( lineEvent );

    syslog(FAC_LINE_MGR, PRI_INFO, "SipLineMgr::deleteLine deleted line: %s",
            identity.toString().data()) ;

    if (pDeleteLine)
    {
        delete pDeleteLine ;
    }
}

void SipLineMgr::lineHasBeenUnregistered(const Url& identity)
{
    SipLine *line = NULL;

    line = sLineList.getLine(identity) ;
    if (line == NULL)
    {
        syslog(FAC_LINE_MGR, PRI_ERR, "unable to delete line (not found): %s",
                identity.toString().data()) ;
       return;
    }
    removeFromList(line);
    delete line;
}

UtlBoolean
SipLineMgr::enableLine(const Url& identity)
{
    SipLine *line = NULL;
    line = sLineList.getLine(identity) ;
    if ( line == NULL)
    {
        syslog(FAC_LINE_MGR, PRI_ERR, "unable to enable line (not found): %s",
                identity.toString().data()) ;
        return FALSE;
    }

    //SipLineEvent lineEvent(line, SipLineEvent::SIP_LINE_EVENT_LINE_ENABLED);
    //queueMessageToObservers(lineEvent);

    line->setState(SipLine::LINE_STATE_TRYING);
    Url canonical = line->getCanonicalUrl();
    Url preferredContact ;
    Url* pPreferredContact = NULL ;

    if (line->getPreferredContactUri(preferredContact))
    {
        pPreferredContact = &preferredContact ;
    }

    if (!mpRefreshMgr->newRegisterMsg(canonical, line->getLineId(), -1, pPreferredContact))
    {
        //duplicate ...call reregister
        mpRefreshMgr->reRegister(identity);
    }
    line = NULL;

    syslog(FAC_LINE_MGR, PRI_INFO, "enabled line: %s",
            identity.toString().data()) ;

    return TRUE;
}

void
SipLineMgr::disableLine(
    const Url& identity,
    UtlBoolean onStartup,
    const UtlString& lineId)
{
    SipLine *line = NULL;
    line = sLineList.getLine(identity) ;
    if ( line == NULL)
    {
        syslog(FAC_LINE_MGR, PRI_ERR, "SipLineMgr::disableLine unable to disable line (not found): %s",
                identity.toString().data()) ;
    }

    if (line->getState() == SipLine::LINE_STATE_REGISTERED ||
        line->getState() == SipLine::LINE_STATE_TRYING)
    {
        mpRefreshMgr->unRegisterUser(identity, onStartup, lineId);
    }

    SipLineEvent lineEvent(line, SipLineEvent::SIP_LINE_EVENT_LINE_DISABLED);
    queueMessageToObservers(lineEvent);

    syslog(FAC_LINE_MGR, PRI_INFO, "SipLineMgr::disableLine disabled line: %s",
            identity.toString().data()) ;

    line = NULL;
}

void
SipLineMgr::notifyChangeInLineProperties(Url& identity)
{
    SipLine *line = NULL;
    line = sLineList.getLine(identity) ;
    if (line == NULL)
    {
        // Ignore error, will be logged on remove/enable/disable/etc
    }

    SipLineEvent lineEvent(line, SipLineEvent::SIP_LINE_EVENT_LINE_CHANGED);
    queueMessageToObservers(lineEvent);
    line = NULL;
}

void
SipLineMgr::notifyChangeInOutboundLine(Url& identity)
{
    SipLine *line = NULL;
    line = sLineList.getLine(identity) ;
    if ( line == NULL)
    {
        // Ignore error, will be logged on remove/enable/disable/etc
    }
    SipLineEvent lineEvent(line, SipLineEvent::SIP_LINE_EVENT_OUTBOUND_CHANGED);
    queueMessageToObservers(lineEvent);
    line = NULL;
}


void SipLineMgr::setOwner(const UtlString& owner)
{
    mOwner.remove(0);
    mOwner.append(owner);
}

const UtlString&
SipLineMgr::getOwner() const
{
    return mOwner;
}

void
SipLineMgr::setDefaultOutboundLine(const Url& outboundLine)
{
   mOutboundLine = outboundLine;

   syslog(FAC_LINE_MGR, PRI_INFO, "default line changed: %s",
         outboundLine.toString().data()) ;

   notifyChangeInOutboundLine( mOutboundLine );
}

void
SipLineMgr::getDefaultOutboundLine(UtlString &rOutBoundLine)
{
    // check if default outbound is valid?
    UtlString host;
    mOutboundLine.getHostAddress(host);
    if( host.isNull())
    {
        setFirstLineAsDefaultOutBound();
    }
    rOutBoundLine.remove(0);
    rOutBoundLine.append(mOutboundLine.toString());
}

void
SipLineMgr::setFirstLineAsDefaultOutBound()
{
    SipLine line;
    if (!sLineList.getDeviceLine(&line))
    {
        sLineList.getFirstLine(&line);
    }
    Url outbound = line.getCanonicalUrl();
    setDefaultOutboundLine(outbound);
}

UtlBoolean
SipLineMgr::getLines(
       int maxLines /*[in]*/,
       int& actualLines /*[in/out]*/,
       SipLine lines[]/*[in/out]*/ ) const
{
    UtlBoolean linesFound = FALSE;
    linesFound = sLineList.linesInArray(
        maxLines, &actualLines, lines);
    return linesFound;
}

UtlBoolean
SipLineMgr::getLines(
    int maxLines /*[in]*/,
    int& actualLines /*[in/out]*/,
    SipLine* lines[]/*[in/out]*/) const
{
    UtlBoolean linesFound = FALSE;
    linesFound = sLineList.linesInArray(
        maxLines, &actualLines, lines );
    return linesFound;
}

int
SipLineMgr::getNumLines() const
{
    return sLineList.getListSize();
}


UtlBoolean
SipLineMgr::getLine(
    const UtlString& toField,
    const UtlString& localContact,
    const UtlString& requestURI,
    SipLine& sipline ) const
{
    SipLine* line = NULL;
    UtlString lineId;
    UtlString userId;

    // Check requestURI
    if (!line && !requestURI.isNull())
    {
        Url uri(requestURI, Url::AddrSpec);
        line = findLineByURL(uri, "requestURI") ;
    }

    // Check Local Contact
    if (!localContact.isNull())
    {
        // Normalize contact -- ??assumes addr_spec??
        UtlString temp;
        if( localContact.index("<") == UTL_NOT_FOUND)
        {
            temp.append("<");
            temp.append(localContact);
            temp.append(">");
        }
        else
        {
            temp.append(localContact);
        }
        Url localContactUrl(temp, Url::NameAddr);
        line = findLineByURL(localContactUrl, "localContact") ;
    }

    // Check toField
    if (!line && !toField.isNull())
    {
        Url toUrl(toField, Url::NameAddr);
        line = findLineByURL(toUrl, "toURL") ;
    }

    if(line)
    {
        sipline = *line;
        return TRUE;
    }

    return FALSE;
}

SipLine*
SipLineMgr::getLineforAuthentication(
    const SipMessage* request /*[in]*/,
    const SipMessage* response /*[in]*/,
    const UtlBoolean& isIncomingRequest,
    const UtlBoolean& fromTempList) const
{
    SipLine* line = NULL;
    UtlString lineId;
    Url      toFromUrl;
    UtlString toFromUri;
    UtlString userId;

    UtlString nonce;
    UtlString opaque;
    UtlString realm;
    UtlString scheme;
    UtlString algorithm;
    UtlString qop;

   // Get realm and scheme (hard way but not too expensive)
   if (response != NULL)
   {
      // is there a Proxy-Authenticate header with realm and nonce?
      if (!response->getAuthenticateData(&scheme, &realm, &nonce, &opaque,
                                         &algorithm, &qop, SipMessage::PROXY))
      {
         // No, is there a WWW-Authenticate header with realm and nonce?
         if (!response->getAuthenticateData(&scheme, &realm, &nonce, &opaque,
                                            &algorithm, &qop, SipMessage::SERVER))
         {
            // Report inability to get auth criteria (neither xxx-Authenticate header)
            UtlString callId ;
            UtlString method ;
            int sequenceNum ;

            response->getCallIdField(&callId);
            response->getCSeqField(&sequenceNum, &method);
            OsSysLog::add(FAC_LINE_MGR, PRI_ERR,
                          "SipLineMgr::getLineforAuthentication "
                          "unable get auth data for message:\ncallid='%s' cseq='%d' method='%s'",
                          callId.data(), sequenceNum, method.data()) ;
         }
         else // found WWW-Authenticate header with realm and nonce
         {
            OsSysLog::add(FAC_AUTH, PRI_DEBUG,
                          "SipLineMgr::getLineforAuthentication "
                          "SERVER auth request:\n"
                          "scheme='%s' realm='%s' nonce='%s' algorithm='%s' qop='%s'"
                          " default line '%s'",
                          scheme.data(), realm.data(), nonce.data(),
                          algorithm.data(), qop.data(), mOutboundLine.toString().data()) ;
         }
      }
      else // found Proxy-Authenticate header with realm and nonce
      {
         OsSysLog::add(FAC_AUTH, PRI_DEBUG,
                       "SipLineMgr::getLineforAuthentication "
                       "PROXY auth request:\n"
                       "scheme='%s' realm='%s' nonce='%s' algorithm='%s' qop='%s'"
                       " default line '%s'",
                       scheme.data(), realm.data(), nonce.data(),
                       algorithm.data(), qop.data(), mOutboundLine.toString().data()) ;
      }
   }

   // Get the LineID and userID
   if(isIncomingRequest)
   {
      //check line id in request uri
      UtlString requestUri;
      request->getRequestUri(&requestUri);
      UtlString temp;
      temp.append("<");
      temp.append(requestUri);
      temp.append(">");
      Url requestUriUrl(temp);
      requestUriUrl.getUrlParameter(SIP_LINE_IDENTIFIER , lineId);
      requestUriUrl.getUserId(userId);
   }
   else
   {
      //check line ID in contact
      UtlString contact;
      request->getContactEntry(0, &contact);
      Url contactUrl(contact);
      contactUrl.getUrlParameter(SIP_LINE_IDENTIFIER , lineId);
      contactUrl.getUserId(userId);
    }

   // Get the fromURL
   request->getFromUrl(toFromUrl);
   toFromUrl.removeFieldParameters();
   toFromUrl.setDisplayName("");
   toFromUrl.removeAngleBrackets();

   if (fromTempList)
   {
      line = sTempLineList.findLine(lineId.data(), realm.data(),
                                    toFromUrl, userId.data(), mOutboundLine) ;
   }

   if (line == NULL)
   {
      line = sLineList.findLine(lineId.data(), realm.data(),
                                toFromUrl, userId.data(), mOutboundLine) ;
   }

   if (line == NULL)
   {
      OsSysLog::add(FAC_LINE_MGR, PRI_ERR,
                    "SipLineMgr::getLineforAuthentication "
                    "unable to find auth credentials for From: '%s'",
                    toFromUrl.toString().data()) ;

       // Get the toURL
       request->getToUrl(toFromUrl);
       toFromUrl.removeFieldParameters();
       toFromUrl.setDisplayName("");
       toFromUrl.removeAngleBrackets();

       if (fromTempList)
       {
          line = sTempLineList.findLine(lineId.data(), realm.data(),
                                        toFromUrl, userId.data(), mOutboundLine) ;
       }

       if (line == NULL)
       {
          line = sLineList.findLine(lineId.data(), realm.data(),
                                    toFromUrl, userId.data(), mOutboundLine) ;
       }
   }

   if (line == NULL)
   {
      // Log the failure
      OsSysLog::add(FAC_AUTH, PRI_ERR,
                    "SipLineMgr::getLineforAuthentication "
                    "unable to find auth credentials: user='%s' realm='%s' To: '%s'",
                    userId.data(), realm.data(), toFromUrl.toString().data()) ;
   }
   else
   {
      // Log the SUCCESS
      OsSysLog::add(FAC_AUTH, PRI_INFO,
                    "SipLineMgr::getLineforAuthentication "
                    "found matching auth credentials user='%s'",
                    line->getUser().data());
   }

   return line;
}

UtlBoolean
SipLineMgr::isUserIdDefined( const SipMessage* request /*[in]*/ ) const
{
    SipLine* line = NULL;
    line = getLineforAuthentication(request, NULL, TRUE);
    if(line)
    {
            return TRUE;
    }
    else
    {
            return FALSE;
    }
}


UtlBoolean SipLineMgr::buildAuthorizationRequest(const SipMessage* response /*[in]*/,
                                                 const SipMessage* request /*[in]*/,
                                                 SipMessage* newAuthRequest /*[out]*/)
{
    UtlBoolean createdResponse = FALSE;
    // Get the userId and password from the DB for the URI
    int sequenceNum;
    HttpMessage::HttpEndpointEnum authorizationEntity = HttpMessage::SERVER;
    UtlString uri;
    UtlString method;
    UtlString nonce;
    UtlString opaque;
    UtlString realm;
    UtlString scheme;
    UtlString algorithm;
    UtlString qop;
    UtlString callId;

    response->getCSeqField(&sequenceNum, &method);
    response->getCallIdField(&callId) ;
    int responseCode = response->getResponseStatusCode();

    // Use the To uri as key to user and password to use
    if(responseCode == HTTP_UNAUTHORIZED_CODE)
    {
        authorizationEntity = HttpMessage::SERVER;
    }
    else if(responseCode == HTTP_PROXY_UNAUTHORIZED_CODE)
    {
        // For proxy we use the uri for the key to userId and password
        authorizationEntity = HttpMessage::PROXY;
    }

    // Get the digest authentication info. needed to create
    // a request with credentials
    response->getAuthenticateData( &scheme, 
                                   &realm, 
                                   &nonce, 
                                   &opaque,
                                   &algorithm, 
                                   &qop, 
                                   authorizationEntity);

    // Set to true when we determine that we have already sent the request 
    // with an Authorization header.
    UtlBoolean alreadyTriedOnce = FALSE;

    // if scheme is basic , we dont support it anymore and we
    //should not sent request again because the password has been
    //converted to digest already and the BASIC authentication will fail anyway
    if(scheme.compareTo(HTTP_BASIC_AUTHENTICATION, UtlString::ignoreCase) == 0)
    {
        alreadyTriedOnce = TRUE; //so that we never send request with basic authentication

        // Log error
        OsSysLog::add(FAC_AUTH, PRI_ERR,
                      "SipLineMgr::buildAuthorizationRequest "
                      " unable to handle basic auth: callid='%s' method='%s' cseq=%d realm='%s'",
                      callId.data(), method.data(), sequenceNum, realm.data()) ;
    }
    else
    {
        // Check to see if we already tried to send the credentials
        int requestAuthIndex = 0;
        UtlString requestUser;
        UtlString requestRealm;
        while (! alreadyTriedOnce
               && request->getDigestAuthorizationData(&requestUser, &requestRealm,
                                                      NULL, NULL, NULL, NULL,
                                                      authorizationEntity, requestAuthIndex))
        {
            // found an Authorization header, is it for our realm?
            if (realm.compareTo(requestRealm) == 0)
            {
                alreadyTriedOnce = TRUE;
                OsSysLog::add(FAC_AUTH, PRI_ERR,
                              "SipLineMgr::buildAuthorizationRequest "
                              "authentication has been sent but it was not accepted: "
                              "callid='%s', realm='%s', user='%s'",
                              callId.data(), realm.data(), requestUser.data()) ;
            }
            else
            {
               requestAuthIndex++;
            }
        }
    }
    // Find the line that sent the request that was challenged
    Url fromUrl;
    UtlString fromUri;
    UtlString userID;
    UtlString passToken;
    UtlBoolean credentialFound = FALSE;
    SipLine* line = NULL;
    //if challenged for an unregister request - then get credentials from temp list of lines
    int expires;
    int contactIndexCount = 0;
    UtlString contactEntry;

    if (!request->getExpiresField(&expires))
    {
        while ( request->getContactEntry(contactIndexCount , &contactEntry ))
        {
            UtlString expireStr;
            Url contact(contactEntry);
            contact.getFieldParameter(SIP_EXPIRES_FIELD, expireStr);
            expires = atoi(expireStr.data());
            if( expires == 0)
                break;
            contactIndexCount++;
        }
    }

    if( method.compareTo(SIP_REGISTER_METHOD) ==0  && expires == 0 )
    {
        line = getLineforAuthentication(request, response, FALSE, TRUE);
        if(line)
        {
            credentialFound = line->getCredentials(scheme, realm, &userID, &passToken);
            removeFromTempList(line);
        }
    }
    else
    {
        line = getLineforAuthentication(request, response, FALSE);
        if(line)
        {
            credentialFound = line->getCredentials(scheme, realm, &userID, &passToken);
        }
    }

    if( !alreadyTriedOnce)
    {
        if ( credentialFound )
        {
            OsSysLog::add(FAC_AUTH, PRI_INFO,
                          "SipLineMgr::buildAuthorizationRequest"
                          "found auth credentials for callid='%s': userid='%s' realm='%s'",
                          callId.data(), userID.data(), realm.data()) ;

            // Construct a new request with authorization and send it
            // the Sticky DNS fields will be copied by the copy constructor
            *newAuthRequest = *request;

            // Reset the transport parameters
#           ifdef TEST_PRINT
            int transportTimeStamp = newAuthRequest->getTransportTime();
            int lastResendDuration = newAuthRequest->getResendDuration();
            int timesSent = newAuthRequest->getTimesSent();
            int transportProtocol = newAuthRequest->getSendProtocol(); //OsSocket::UNKNOWN;
            int mFirstSent = newAuthRequest->isFirstSend();
            osPrintf( "SipLineMgr::buildAuthenticatedResponse "
                     "transTime: %d resendDur: %d timesSent: %d sendProtocol: %d isFirst: %d\n",
                     transportTimeStamp, lastResendDuration, timesSent,
                     transportProtocol, mFirstSent);
#           endif
            newAuthRequest->resetTransport();

#           ifdef TEST_PRINT
            transportTimeStamp = newAuthRequest->getTransportTime();
            lastResendDuration = newAuthRequest->getResendDuration();
            timesSent = newAuthRequest->getTimesSent();
            transportProtocol = newAuthRequest->getSendProtocol(); //OsSocket::UNKNOWN;
            mFirstSent = newAuthRequest->isFirstSend();
            OsSysLog::add(FAC_AUTH, PRI_DEBUG,
                          "SipLineMgr::buildAuthorizationRequest "
                          "transTime: %d resendDur: %d timesSent: %d sendProtocol: %d isFirst: %d",
                          transportTimeStamp, lastResendDuration, timesSent,
                          transportProtocol, mFirstSent);
#           endif

            // Get rid of the via as another will be added.
            newAuthRequest->removeTopVia();

            if(scheme.compareTo(HTTP_DIGEST_AUTHENTICATION, UtlString::ignoreCase) == 0)
            {
                UtlString responseHash;
                int nonceCount;
                // create the authorization in the request
                request->getRequestUri(&uri);

                // :TBD: cheat and use the cseq instead of a real nonce-count
                request->getCSeqField(&nonceCount, &method);
                nonceCount = (nonceCount + 1) / 2;

                request->getRequestMethod(&method);

                // Use unique tokens which are constant for this
                // session to generate a cnonce
                Url fromUrl;
                UtlString cnonceSeed;
                UtlString fromTag;
                UtlString cnonce;
                request->getCallIdField(&cnonceSeed);
                request->getFromUrl(fromUrl);
                fromUrl.getFieldParameter("tag", fromTag);
                cnonceSeed.append(fromTag);
                cnonceSeed.append("blablacnonce"); // secret
                NetMd5Codec::encode(cnonceSeed, cnonce);

                // Get the digest of the body
                const HttpBody* body = request->getBody();
                UtlString bodyDigest;
                const char* bodyString = "";
                if(body)
                {
                    ssize_t len;
                    body->getBytes(&bodyString, &len);
                    if(bodyString == NULL)
                        bodyString = "";
                }

                NetMd5Codec::encode(bodyString, bodyDigest);

                // Build the Digest hash response
                HttpMessage::buildMd5Digest(
                    passToken.data(),
                    algorithm.data(),
                    nonce.data(),
                    cnonce.data(),
                    nonceCount,
                    qop.data(),
                    method.data(),
                    uri.data(),
                    bodyDigest.data(),
                    &responseHash);

                newAuthRequest->setDigestAuthorizationData(
                    userID.data(),
                    realm.data(),
                    nonce.data(),
                    uri.data(),
                    responseHash.data(),
                    algorithm.data(),
                    cnonce.data(),
                    opaque.data(),
                    qop.data(),
                    nonceCount,
                    authorizationEntity);

            }

            // This is a new version of the message so increment the sequence number
            newAuthRequest->incrementCSeqNumber();

            // If the first hop of this message is strict routed,
            // we need to add the request URI host back in the route
            // field so that the send will work correctly
            //
            //  message is:
            //      METHOD something
            //      Route: xyz, abc
            //
            //  change to xyz:
            //      METHOD something
            //      Route: something, xyz, abc
            //
            //  which sends as:
            //      METHOD something
            //      Route: xyz, abc
            //
            // But if the first hop is loose routed:
            //
            //  message is:
            //      METHOD something
            //      Route: xyz;lr, abc
            //
            // leave URI and routes alone
            if ( newAuthRequest->isClientMsgStrictRouted() )
            {
                UtlString requestUri;
                newAuthRequest->getRequestUri(&requestUri);
                OsSysLog::add(FAC_AUTH, PRI_ERR,
                              "SipLineMgr::buildAuthorizationRequest "
                              "adding strict route callid='%s' route='%s'",
                              callId.data(), requestUri.data()) ;
                newAuthRequest->addRouteUri(requestUri);
            }
            createdResponse = TRUE;
        }
        else
        {
            OsSysLog::add(FAC_AUTH, PRI_ERR,
                          "SipLineMgr::buildAuthorizationRequest "
                          "could not find auth credentials for callid='%s' "
                          "lineId='%s' realm='%s'",
                          callId.data(), fromUri.data(), realm.data()) ;
        }
    }
    // Else we already tried to provide authentication
    // Or we do not have a userId and password for this uri
    // Let this error message through to the application
    else
    {
       OsSysLog::add(FAC_AUTH, PRI_ERR,
                     "SipLineMgr::buildAuthorizationRequest "
                     "previous authentication failed for callid='%s'",
                     callId.data());
    }

    return( createdResponse );
}

void SipLineMgr::addMessageObserver(OsMsgQ& messageQueue,
                                      void* observerData)
{
    SipObserverCriteria* observer = new SipObserverCriteria(observerData,
        &messageQueue,
        "",
        FALSE,
        FALSE,
        FALSE,
        FALSE,
        NULL);

    {
        // Add the observer and its filter criteria to the list lock scope
        OsWriteLock lock(mObserverMutex);
        mMessageObservers.insert(observer);
    }
}


UtlBoolean SipLineMgr::removeMessageObserver(OsMsgQ& messageQueue,
                                            void* pObserverData)
{
    OsWriteLock lock(mObserverMutex);

    SipObserverCriteria* pObserver = NULL ;
    UtlBoolean bRemovedObservers = FALSE ;

    // Traverse all of the observers and remove any that match the
    // message queue/observer data.  If the pObserverData is null, all
    // matching message queues will be removed.  Otherwise, only those
    // observers that match both the message queue and observer data
    // are removed.
    UtlHashBagIterator iterator(mMessageObservers);
    while((pObserver = (SipObserverCriteria*) iterator()))
    {
        if (pObserver->getObserverQueue() == &messageQueue)
        {
            if ((pObserverData == NULL) ||
                    (pObserverData == pObserver->getObserverData()))
            {
                bRemovedObservers = true ;

                UtlContainable* wasRemoved = mMessageObservers.removeReference(pObserver);
                if(wasRemoved)
                {
                   delete wasRemoved;
                }
            }
        }
    }
    return bRemovedObservers ;
}


void SipLineMgr::queueMessageToObservers(SipLineEvent& event)
{
    // Find all of the observers which are interested in this method and post the message
    UtlString observerMatchingKey("");
    SipObserverCriteria* observerCriteria = NULL;
    OsReadLock lock(mObserverMutex);

    UtlHashBagIterator observerIterator(mMessageObservers, &observerMatchingKey);
    do
    {
        observerCriteria = (SipObserverCriteria*) observerIterator();

        // If this message matches the filter criteria
        if(observerCriteria)
        {
            OsMsgQ* observerQueue = observerCriteria->getObserverQueue();
            void* observerData = observerCriteria->getObserverData();
            // Put the message in the observers queue
            event.setObserverData(observerData);
            observerQueue->send(event);
        }
    }
    while(observerCriteria != NULL);
}


void SipLineMgr::addToList(SipLine *line)
{
    sLineList.add(new SipLine(*line));
}


void SipLineMgr::removeFromList(SipLine *line)
{
    sLineList.remove(line);
}
void SipLineMgr::addToTempList(SipLine *line)
{
    sTempLineList.add(new SipLine(*line));
}

void SipLineMgr::removeFromTempList(SipLine *line)
{
    sTempLineList.remove(line);
}

void SipLineMgr::setDefaultContactUri(const Url& contactUri)
{
   mDefaultContactUri = contactUri;
}

UtlBoolean SipLineMgr::initializeRefreshMgr(SipRefreshMgr *refershMgr)
{
    if (refershMgr)
    {
        mpRefreshMgr = refershMgr;
        mpRefreshMgr->addMessageObserver(*(getMessageQueue()),
            SIP_REGISTER_METHOD,
            TRUE, // do want to get requests
            TRUE, // do want responses
            TRUE, // Incoming messages
            FALSE); // Don't want to see out going messages
        return true;
    }
    else
    {
        osPrintf("ERROR::SipLineMgr::SipLineMgr SIP REFRESH MGR NULL\n");
        return false;
    }
}

UtlBoolean
SipLineMgr::addCredentialForLine(
    const Url& identity,
    const UtlString strRealm,
    const UtlString strUserID,
    const UtlString md5Token,
    const UtlString type)
{
    SipLine *line = NULL;
    if (! (line = sLineList.getLine(identity)) )
    {
       OsSysLog::add(FAC_LINE_MGR, PRI_ERR,
                     "SipLineMgr::addCredentialForLine no line for identity '%s'",
                     identity.toString().data()
                     );
        return false;
    }

    // Construct A1
    UtlString a1Buffer;
    UtlString encodedA1;
    a1Buffer.append(strUserID);
    a1Buffer.append(':');
    a1Buffer.append(strRealm);
    a1Buffer.append(':');
    a1Buffer.append(md5Token);
    NetMd5Codec::encode(a1Buffer.data(), encodedA1);

    if (!line->addCredentials(strRealm, strUserID, encodedA1, type))
    {
        line = NULL;
        OsSysLog::add(FAC_LINE_MGR, PRI_ERR,
                      "SipLineMgr::addCredentialForLine addCredentials failed for"
                      " identity '%s' user '%s' realm '%s'",
                      identity.toString().data(), strUserID.data(), strRealm.data()
                      );
        return false;
    }
    return true;
}

UtlBoolean
SipLineMgr::deleteCredentialForLine(const Url& identity, const UtlString strRealm)
{
    SipLine *line = NULL;
    if (! (line = sLineList.getLine(identity)) )
    {
       OsSysLog::add(FAC_LINE_MGR, PRI_ERR,
                     "SipLineMgr::deleteCredentialForLine no line for identity '%s'",
                     identity.toString().data()
                     );
        return false;
    }
    line->removeCredential(&strRealm);
    return true;

}

int
SipLineMgr::getNumOfCredentialsForLine( const Url& identity ) const
{
    int numOfCredentials = 0;
    SipLine *line = NULL;
    if (! (line = sLineList.getLine( identity )) )
    {
       OsSysLog::add(FAC_LINE_MGR, PRI_ERR,
                     "SipLineMgr::getNumOfCredentialsForLine no line for identity '%s'",
                     identity.toString().data()
                     );
    } else
    {
        numOfCredentials = line->GetNumOfCredentials();
    }
    return numOfCredentials;
}

UtlBoolean
SipLineMgr::getCredentialListForLine(
    const Url& identity,
    int maxEnteries,
    int & actualEnteries,
    UtlString realmList[],
    UtlString userIdList[],
    UtlString typeList[],
    UtlString passTokenList[] )
{
    UtlBoolean retVal = FALSE;
    SipLine *line = NULL;
    if (! (line = sLineList.getLine(identity)) )
    {
       OsSysLog::add(FAC_LINE_MGR, PRI_ERR,
                     "SipLineMgr::getCredentialListForLine no line for identity '%s'",
                     identity.toString().data()
                     );
    } else
    {
        retVal = line->getAllCredentials(maxEnteries, actualEnteries, realmList, userIdList, typeList, passTokenList);
    }
    return retVal;
}

#ifdef DEPRECATED_SIPLINE_FEATURE
//line Call Handling
void SipLineMgr::setCallHandlingForLine(const Url& identity , UtlBoolean useCallHandling)
{
    SipLine *line = NULL;
    if (! (line = sLineList.getLine(identity)) )
    {
        osPrintf("ERROR::SipLineMgr::setCallHandlingForLine() - No Line for identity\n");
        return;
    }
    line->setCallHandling(useCallHandling);
    line = NULL;
}

UtlBoolean
SipLineMgr::getCallHandlingForLine(const Url& identity) const
{
    UtlBoolean retVal = FALSE;
    SipLine *line = NULL;
    if (! (line = sLineList.getLine(identity)) )
    {
        osPrintf("ERROR::SipLineMgr::getCallHandlingForLine() - No Line for identity \n");
    } else
    {
        retVal = line->getCallHandling();
        line = NULL;
    }
    return retVal;
}

//line auto enable
void SipLineMgr::setAutoEnableForLine(const Url& identity , UtlBoolean isAutoEnable)
{
    SipLine *line = NULL;
    if (! (line = sLineList.getLine(identity)) )
    {
        osPrintf("ERROR::SipLineMgr::setAutoEnableStatus() - No Line for identity\n");
        return;
    }
    line->setAutoEnableStatus(isAutoEnable);
    line = NULL;

}

UtlBoolean
SipLineMgr::getEnableForLine(const Url& identity) const
{
    UtlBoolean retVal = FALSE;
    SipLine *line = NULL;
    if (! (line = sLineList.getLine(identity)) )
    {
        osPrintf("ERROR::SipLineMgr::getEnableForLine() - No Line for identity \n");
    }
    else
    {
        retVal = line->getAutoEnableStatus();
        line = NULL;
    }
    return retVal;
}
#endif

//can only get state
int
SipLineMgr::getStateForLine( const Url& identity ) const
{
    int State = SipLine::LINE_STATE_UNKNOWN;
    SipLine *line = NULL;
    if (! (line = sLineList.getLine(identity)) )
    {
        osPrintf("ERROR::SipLineMgr::getStateForLine() - No Line for identity \n");
    } else
    {
      State =line->getState();
        line = NULL;
    }
    return State;
}

void
SipLineMgr::setStateForLine(
    const Url& identity,
    int state )
{
    SipLine *line = NULL;
    if (! (line = sLineList.getLine(identity)) )
    {
        osPrintf("ERROR::SipLineMgr::setStateForLine() - No Line for identity\n");
        return;
    }
    int previousState =line->getState();
    line->setState(state);

    if ( previousState != SipLine::LINE_STATE_PROVISIONED && state == SipLine::LINE_STATE_PROVISIONED)
    {
        disableLine(identity);
    }
    else if( previousState == SipLine::LINE_STATE_PROVISIONED && state == SipLine::LINE_STATE_REGISTERED)
    {
        enableLine(identity);
    }
    line = NULL;
}

// Line visibility
#ifdef DEPRECATED_SIPLINE_FEATURE
UtlBoolean
SipLineMgr::getVisibilityForLine( const Url& identity ) const
{
    UtlBoolean retVal = FALSE;
    SipLine *line = NULL;
    if (! (line = sLineList.getLine(identity)) )
    {
        osPrintf("ERROR::SipLineMgr::getVisibilityForLine() - No Line for identity \n");
    }
    else
    {
      retVal = line->getVisibility();
        line = NULL;
   }
    return retVal;
}
void SipLineMgr::setVisibilityForLine(const Url& identity , UtlBoolean Visibility)
{
    SipLine *line = NULL;
    if (! (line = sLineList.getLine(identity)) )
    {
        osPrintf("ERROR::SipLineMgr::setVisibilityForLine() - No Line for identity\n");
        return;
    }
    line->setVisibility(Visibility);
    line = NULL;
}
#endif

//line User
UtlBoolean
SipLineMgr::getUserForLine(const Url& identity, UtlString &User) const
{
    UtlBoolean retVal = FALSE;
    SipLine *line = NULL;
    if (! (line = sLineList.getLine(identity)) )
    {
        osPrintf("ERROR::SipLineMgr::getUserForLine() - No Line for identity \n");
    } else
    {
        User.remove(0);
        UtlString UserStr = line->getUser();
        User.append(UserStr);
        retVal = TRUE;
        line = NULL;
    }
    return retVal;
}
void SipLineMgr::setUserForLine(const Url& identity, const UtlString User)
{

    SipLine *line = NULL;
    if (! (line = sLineList.getLine(identity)) )
    {
        osPrintf("ERROR::SipLineMgr::setUserForLine() - No Line for identity\n");
        return;
    }
    line->setUser(User);
    line = NULL;
}

void SipLineMgr::setUserEnteredUrlForLine(const Url& identity, UtlString sipUrl)
{

    SipLine *line = NULL;
    if (! (line = sLineList.getLine(identity)) )
    {
        osPrintf("ERROR::SipLineMgr::setUserEnteredUrlForLine() - No Line for this Url\n");
        return;
    }
    line->setIdentityAndUrl(identity, Url(sipUrl));
    line = NULL;
}

UtlBoolean
SipLineMgr::getUserEnteredUrlForLine(
    const Url& identity,
    UtlString& rSipUrl) const
{
    UtlBoolean retVal = FALSE;
    SipLine *line = NULL;
    if (! (line = sLineList.getLine(identity)) )
    {
        osPrintf("ERROR::SipLineMgr::getUserEnteredUrlForLine() - No Line for this Url \n");
    } else
    {
        rSipUrl.remove(0);
        Url userEnteredUrl = line->getUserEnteredUrl();
        rSipUrl.append(userEnteredUrl.toString());
        retVal = TRUE;
        line = NULL;
    }
    return retVal;
}

UtlBoolean
SipLineMgr::getCanonicalUrlForLine(
    const Url& identity,
    UtlString& rSipUrl) const
{
    UtlBoolean retVal = FALSE;
    SipLine *line = NULL;
    if (! (line = sLineList.getLine(identity)) )
    {
        osPrintf("ERROR::SipLineMgr::getUserForLine() - No Line for this Url \n");
    } else
    {
        rSipUrl.remove(0);
        Url canonicalUrl = line->getCanonicalUrl();
        rSipUrl.append(canonicalUrl.toString());
        retVal = TRUE;
        line = NULL;
    }
    return retVal;
}

#ifdef DEPRECATED_SIPLINE_FEATURE
// Delete all line definitions from the specified configuration db.
void
SipLineMgr::purgeLines( OsConfigDb *pConfigDb )
{
    UtlString keyLast ;
    UtlString keyNext ;
    UtlString valueNext ;

    if (pConfigDb != NULL)
    {
        // Remove all PHONESET_LINE. keys
        OsConfigDb phonesetSubHash ;
        if (pConfigDb->getSubHash(BASE_PHONESET_LINE_KEY, phonesetSubHash) == OS_SUCCESS)
        {
            while (phonesetSubHash.getNext(keyLast, keyNext, valueNext) == OS_SUCCESS)
            {
                UtlString removeKey(BASE_PHONESET_LINE_KEY) ;
                removeKey.append(keyNext) ;
                pConfigDb->remove(removeKey) ;
                keyLast = keyNext ;
            }
        }

        // Remove all USER_LINE. keys
        OsConfigDb userSubHash ;
        keyLast.remove(0) ;
        if (pConfigDb->getSubHash(BASE_USER_LINE_KEY, userSubHash) == OS_SUCCESS)
        {
            while (userSubHash.getNext(keyLast, keyNext, valueNext) == OS_SUCCESS)
            {
                UtlString removeKey(BASE_USER_LINE_KEY) ;
                removeKey.append(keyNext) ;
                pConfigDb->remove(removeKey) ;
                keyLast = keyNext ;
            }
        }

        // Remove the Default outbound line
        pConfigDb->remove(USER_DEFAULT_OUTBOUND_LINE) ;
    }
}
#endif

UtlBoolean SipLineMgr::addLineAlias(const Url& identity, const Url& lineAlias)
{
    UtlBoolean bSuccess = false ;

    SipLine* pLine = sLineList.getLine(identity) ;
    if (pLine == NULL)
    {
        syslog(FAC_LINE_MGR, PRI_ERR, "SipLineMgr::addLineAlias unable to add alias (line not found): %s",
                identity.toString().data()) ;
    }
    else
    {
        pLine->addAlias(lineAlias) ;
        bSuccess = true ;
    }

    return bSuccess ;
}

#ifdef DEPRECATED_SIPLINE_FEATURE
// Load a single line
UtlBoolean
SipLineMgr::loadLine(
    OsConfigDb* pConfigDb,
    UtlString strSubKey,
    SipLine& line )
{
    UtlBoolean bSuccess = false ;
    UtlBoolean bAllowForwarding ;
    int iRegistration = SipLine::LINE_STATE_UNKNOWN;
    UtlString  strKey ;
    UtlString  strUrl ;
    UtlString  strValue ;

    if ( pConfigDb != NULL )
    {
        // Get URL
        strKey = strSubKey ;
        strKey.append(LINE_PARAM_URL) ;
        if (pConfigDb->get(strKey, strUrl))
        {
            if (!strUrl.isNull())
            {
                UtlString address;
                Url url(strUrl) ;
                Url identity(url);
                url.getHostAddress(address);
                if( address.isNull())    //dynamic IP address
                {
                    //get address and port from contact uri
                    UtlString contactHost;
                    mDefaultContactUri.getHostAddress(contactHost);
                    int contactPort = mDefaultContactUri.getHostPort();
                    identity.setHostAddress(contactHost);
                    identity.setHostPort(contactPort);
                } else
                {
                    UtlString uri;
                    url.getUri(uri);
                    identity = Url(uri);
                }
                line.setIdentityAndUrl(identity,url);
                bSuccess = true ;

                // Get Allow Forwarding (Default is ENABLE)
                strKey = strSubKey ;
                strKey.append(LINE_PARAM_ALLOW_FORWARDING) ;
                if ((pConfigDb->get(strKey, strValue)) &&
                strValue.compareTo(LINE_ALLOW_FORWARDING_ENABLE, UtlString::ignoreCase)==0)
                {
                    bAllowForwarding = true ;
                } else
                {
                    bAllowForwarding = false ;
                }
                line.setCallHandling(bAllowForwarding) ;

                // Get Registration (Default is Provision)
                strKey = strSubKey ;
                strKey.append(LINE_PARAM_REGISTRATION) ;
                if ((pConfigDb->get(strKey, strValue)) &&
                        strValue.compareTo(LINE_REGISTRATION_REGISTER, UtlString::ignoreCase)==0)
                {
                    iRegistration  = SipLine::LINE_STATE_REGISTERED ;
                }
                else
                {
                    iRegistration  = SipLine::LINE_STATE_PROVISIONED;
                }
                    //The list registration behaviour is independent of the autoenable behaviour.
                    //The autoEnable behaviour is just for persistence. Since we do not have guest
                    //login all the lines right now by default should have autoenable status true
                    //since this line is read from a file( ie is persistent) is is autoenabled.
                    line.setState(iRegistration) ;
                    line.setAutoEnableStatus(TRUE);

                // Load Credentials.  Attempt to load the max number of
                // credentials, however, if any credentials fail to load,
                // kick out. This is a safe optimization assuming that
                // credentials numbers are sequential.
                line.removeAllCredentials() ;
                for (int i=0; i<MAX_CREDENTIALS; i++)
                {
                    strKey = strSubKey ;
                    strKey.append(LINE_PARAM_CREDENTIAL) ;
                    char szTempBuf[32] ;
                    sprintf(szTempBuf, "%d", i+1) ;
                    strKey.append(szTempBuf) ;
                    strKey.append(".") ;

                    if (!loadCredential(pConfigDb, strKey, line))
                        break ;

                }
            }
        }
    }
    return bSuccess ;
}

// Store/save a single line
void SipLineMgr::storeLine(OsConfigDb* pConfigDb, UtlString strSubKey, SipLine line)
{
    UtlString strKey ;

    if (pConfigDb != NULL)
    {
        // Store URL
        strKey = strSubKey ;
        strKey.append(LINE_PARAM_URL) ;
        Url urlLine = line.getUserEnteredUrl() ;
        pConfigDb->set(strKey, urlLine.toString()) ;

        // Store Registration Type
        strKey = strSubKey ;
        strKey.append(LINE_PARAM_REGISTRATION) ;
        //get line state and if it is anything other than PROVISION,
        //set the line state to REGISTER because the other states are set
        //only for REGISTRATION behaviour
        int iLineState = line.getState();
        if (iLineState == SipLine::LINE_STATE_PROVISIONED)
            pConfigDb->set(strKey, LINE_REGISTRATION_PROVISION) ;
        else
            pConfigDb->set(strKey, LINE_REGISTRATION_REGISTER) ;

        //there is no need to set the autoenable state because if we are
        //saving it then it has to be autoenabled. The check should be performed
        //before saving the line

        // Store Call Handling Settings
        strKey = strSubKey ;
        strKey.append(LINE_PARAM_ALLOW_FORWARDING) ;
        if (line.getCallHandling())
            pConfigDb->set(strKey, LINE_ALLOW_FORWARDING_ENABLE) ;
        else
            pConfigDb->set(strKey, LINE_ALLOW_FORWARDING_DISABLE) ;

        int noOfCredentials = line.GetNumOfCredentials();
                if (noOfCredentials > 0)
                {
                        UtlString *strRealms = new UtlString[noOfCredentials] ;
                        UtlString *strUserIds = new UtlString[noOfCredentials] ;
                        UtlString *strTypes = new UtlString[noOfCredentials] ;
                        UtlString *strPassTokens = new UtlString[noOfCredentials] ;

                        int iCredentials = 0 ;

                        if (line.getAllCredentials(noOfCredentials, iCredentials, strRealms, strUserIds, strTypes, strPassTokens))
                        {
                                for (int i=0; i<iCredentials; i++)
                                {
                                        UtlString strCredentialKey(strSubKey) ;
                                        strCredentialKey.append(LINE_PARAM_CREDENTIAL) ;
                                        char szTempBuf[32] ;
                                        sprintf(szTempBuf, "%d", i+1) ;
                                        strCredentialKey.append(szTempBuf) ;
                                        strCredentialKey.append(".") ;

                                        storeCredential(pConfigDb, strCredentialKey, strRealms[i], strUserIds[i], strPassTokens[i], strTypes[i]) ;
                                }
                        }
                        delete [] strRealms;
                        delete [] strUserIds;
                        delete [] strTypes;
                        delete [] strPassTokens;
                }
    }
}

// Load a single credential and populate it within a line
UtlBoolean SipLineMgr::loadCredential(OsConfigDb* pConfigDb,
                                     UtlString strSubKey,
                                     SipLine& line)


{
    UtlBoolean bSuccess = false ;
    UtlString  strKey ;

    UtlString strRealm ;
    UtlString strUserId ;
    UtlString strPassToken ;

    if (pConfigDb != NULL)
    {
        // Get Realm
        strKey = strSubKey ;
        strKey.append(LINE_PARAM_CREDENTIAL_REALM) ;
        pConfigDb->get(strKey, strRealm) ;

        // Get User ID
        strKey = strSubKey ;
        strKey.append(LINE_PARAM_CREDENTIAL_USERID) ;
        pConfigDb->get(strKey, strUserId) ;

        // Get Password Token
        strKey = strSubKey ;
        strKey.append(LINE_PARAM_CREDENTIAL_PASSTOKEN) ;
        pConfigDb->get(strKey, strPassToken) ;

        if (!strUserId.isNull() && !strPassToken.isNull())
        {
            bSuccess = true ;
            line.addCredentials(strRealm, strUserId, strPassToken, HTTP_DIGEST_AUTHENTICATION) ;
        }
    }

    return bSuccess ;
}
#endif

SipLine* SipLineMgr::findLineByURL(const Url& url, const char* szType) const
{
    SipLine* pLine = NULL ;
    UtlString lineId ;
    UtlString userId ;

    // Check LineID
    url.getUrlParameter(SIP_LINE_IDENTIFIER, lineId) ;
    if( !lineId.isNull())
    {
        pLine = sLineList.getLine(lineId) ;
    }

    // Check Full Identity
    if (pLine == NULL)
    {
        pLine = sLineList.getLine(url) ;
    }

    // Check userID
    url.getUserId(userId);
    if(pLine == NULL && !userId.isNull())
    {
        int userIdmatches = 0;
        pLine = sLineList.getLine(userId, userIdmatches) ;
        if(userIdmatches > 1)
        {
            assert(pLine != NULL) ;
            if (pLine != NULL)
            {
                OsSysLog::add(FAC_LINE_MGR, PRI_WARNING,
                        "Multiple lines (%d) defined for userId %s, %s=%s selected=%s",
                        userIdmatches, userId.data(),
                        szType,
                        url.toString().data(),
                        pLine->getIdentity().toString().data()) ;
            }
        }
    }

    return pLine ;
}

#ifdef DEPRECATED_SIPLINE_FEATURE
// Store/save a single credential
void SipLineMgr::storeCredential(OsConfigDb* pConfigDb,
                                 UtlString strSubKey,
                                 UtlString strRealm,
                                 UtlString strUserId,
                                 UtlString strPassToken,
                                 UtlString strType)

{
    UtlString strKey ;

    if (pConfigDb != NULL)
    {
        // Store Realm
        strKey = strSubKey ;
        strKey.append(LINE_PARAM_CREDENTIAL_REALM) ;
        pConfigDb->set(strKey, strRealm) ;

        // Store User ID
        strKey = strSubKey ;
        strKey.append(LINE_PARAM_CREDENTIAL_USERID) ;
        pConfigDb->set(strKey, strUserId) ;

        // Store Password Token
        strKey = strSubKey ;
        strKey.append(LINE_PARAM_CREDENTIAL_PASSTOKEN) ;
        pConfigDb->set(strKey, strPassToken) ;
    }
}
#endif


void SipLineMgr::enableAllLines()
{
   int noOfLines =  getNumLines() ;
   int actualLines;
   int i = 0;

   // Allocate Lines
   SipLine *lines = new SipLine[noOfLines] ;
   // Get Actual lines and disable first
   if (getLines(noOfLines, actualLines, lines))
   {
      for( i =0 ; i< actualLines; i++)
      {
         if ( lines[i].getState() == SipLine::LINE_STATE_REGISTERED)
         {
            disableLine(lines[i].getIdentity(), TRUE, lines[i].getLineId());//unregister for startup
         }
      }
   }
   // enable lines again
   if (getLines(noOfLines, actualLines, lines))
   {
      for( i =0 ; i< actualLines; i++)
      {
         if ( lines[i].getState() == SipLine::LINE_STATE_REGISTERED)
         {
               enableLine(lines[i].getIdentity());
         }
      }
   }

    // Free Lines
    delete []lines ;
}
