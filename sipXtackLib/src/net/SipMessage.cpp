//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdlib.h>
#include <string.h>
#ifdef _VXWORKS
#include <resparse/vxw/hd_string.h>
#endif

//uncomment next line to track the create and destroy of messages
//#define TRACK_LIFE
//#define TEST_PRINT
//#define TEST

// APPLICATION INCLUDES
#include <utl/UtlDListIterator.h>
#include <utl/UtlHashBagIterator.h>
#include <utl/UtlHashMap.h>
#include <net/CallId.h>
#include <net/MimeBodyPart.h>
#include <net/NameValueTokenizer.h>
#include <net/SipMessage.h>
#include <net/SipUserAgent.h>
#include <net/SmimeBody.h>
#include <net/Url.h>
#include <os/OsDateTime.h>
#include <os/OsSysLog.h>
#include "net/SipXauthIdentity.h"
//#include "net/SignedUrl.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const UtlContainableType SipMessage::TYPE = "SipMessage";
#define MAXIMUM_INTEGER_STRING_LENGTH 20

// STATIC VARIABLES
SipMessage::SipMessageFieldProps SipMessage::sSipMessageFieldProps;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipMessage::SipMessage(const char* messageBytes, ssize_t byteCount) :
   HttpMessage(messageBytes, byteCount)
{
#ifdef TRACK_LIFE
   osPrintf("Created SipMessage @ address:%X\n",this);
#endif

   mInterfacePort = PORT_NONE;
   mpSipTransaction = NULL;
   replaceShortFieldNames();
}

SipMessage::SipMessage(OsSocket* inSocket, ssize_t bufferSize) :
   HttpMessage(inSocket, bufferSize)
{
#ifdef TRACK_LIFE
   osPrintf("Created SipMessage @ address:%X\n",this);
#endif

   mInterfacePort = PORT_NONE;
   mpSipTransaction = NULL;
   replaceShortFieldNames();
}


// Copy constructor
SipMessage::SipMessage(const SipMessage& rSipMessage) :
   HttpMessage(rSipMessage)
{
#ifdef TRACK_LIFE
   osPrintf("Created SipMessage @ address:%X\n",this);
#endif
   replaceShortFieldNames();
   mInterfaceIp = rSipMessage.mInterfaceIp;
   mInterfacePort = rSipMessage.mInterfacePort;
   m_dnsProtocol = rSipMessage.m_dnsProtocol;
   m_dnsAddress = rSipMessage.m_dnsAddress;
   m_dnsPort = rSipMessage.m_dnsPort;
   mpSipTransaction = rSipMessage.mpSipTransaction;
}

// Destructor
SipMessage::~SipMessage()
{
#ifdef TRACK_LIFE
   osPrintf("Deleted SipMessage from address :%X\n",this);
#endif
}

// Assignment operator
SipMessage&
SipMessage::operator=(const SipMessage& rSipMessage)
{
   HttpMessage::operator =((HttpMessage&)rSipMessage);
   if (this != &rSipMessage)
   {
      replaceShortFieldNames();
      mInterfaceIp = rSipMessage.mInterfaceIp;
      mInterfacePort = rSipMessage.mInterfacePort;
      m_dnsProtocol = rSipMessage.m_dnsProtocol;
      m_dnsAddress = rSipMessage.m_dnsAddress;
      m_dnsPort = rSipMessage.m_dnsPort;
      mpSipTransaction = rSipMessage.mpSipTransaction;
   }
   return *this;
}

UtlContainableType SipMessage::getContainableType(void) const
{
   return SipMessage::TYPE;
};

/* ============================ MANIPULATORS ============================== */

UtlBoolean SipMessage::getShortName(const char* longFieldName,
                       UtlString* shortFieldName)
{
   NameValuePair longNV(longFieldName);
   UtlBoolean nameFound = FALSE;

   shortFieldName->remove(0);

   NameValuePair* shortNV =
      dynamic_cast <NameValuePair*> (sSipMessageFieldProps.mLongFieldNames.find(&longNV));
   if(shortNV)
   {
      shortFieldName->append(shortNV->getValue());
      nameFound = TRUE;
   }
   return(nameFound);
}

UtlBoolean SipMessage::getLongName(const char* shortFieldName,
                      UtlString* longFieldName)
{
   UtlBoolean nameFound = FALSE;

    // Short names are currently only 1 character long
    // If the short name is exactly 1 character long
    if(shortFieldName && shortFieldName[0] &&
        shortFieldName[1] == '\0')
    {
        UtlString shortNV(shortFieldName);

       NameValuePair* longNV =
          dynamic_cast <NameValuePair*> (sSipMessageFieldProps.mShortFieldNames.find(&shortNV));
       if(longNV)
       {
          *longFieldName = longNV->getValue();
          nameFound = TRUE;
       }
        // Optimization in favor of utility
        //else
        //{
        //    longFieldName->remove(0);
        //}
    }
   return(nameFound);
}

void SipMessage::replaceShortFieldNames()
{
   NameValuePair* nvPair;
   UtlString longName;
   size_t position;

   for ( position= 0;
         (nvPair = dynamic_cast<NameValuePair*>(mNameValues.at(position)));
         position++
        )
   {
      if(getLongName(nvPair->data(), &longName))
      {
         // There is a long form for this name, so replace it.
         mHeaderCacheClean = FALSE;
         NameValuePair* modified;

         /*
          * NOTE: the header name is the containable key, so we must remove the
          *       NameValuePair from the mNameValues list and then reinsert the
          *       modified version; you are not allowed to modify key values while
          *       an object is in a container.
          */
         modified = dynamic_cast<NameValuePair*>(mNameValues.removeAt(position));
         nvPair->remove(0);
         nvPair->append(longName);
         mNameValues.insertAt(position, modified);
      }
   }
}

/* ============================ ACCESSORS ================================= */
void SipMessage::setSipRequestFirstHeaderLine(const char* method,
                                              const char* uri,
                                              const char* protocolVersion)
{
   //fix for bug : 1667 - 12/18/2001
   Url tempRequestUri(uri, TRUE);
   Url::Scheme s = tempRequestUri.getScheme();
   // Log error if the URI is not parsable or is not sip:/sips:.
   if (!(s == Url::SipUrlScheme || s == Url::SipsUrlScheme))
   {
      OsSysLog::add(FAC_SIP, PRI_ERR,
                    "SipMessage::setSipRequestFirstHeaderLine "
                    "setting request-URI of SipMessage to non-SIP URI '%s'",
                    uri);
   }
   UtlString strRequestUri;
   tempRequestUri.removeUrlParameter("method");
   tempRequestUri.removeAngleBrackets();
   tempRequestUri.getUri(strRequestUri);

   setRequestFirstHeaderLine(method, strRequestUri.data(), protocolVersion);
}

void SipMessage::setRegisterData(const char* registererUri,
                        const char* registerAsUri,
                        const char* registrarServerUri,
                        const char* takeCallsAtUri,
                        const char* callId,
                        int sequenceNumber,
                        int expiresInSeconds)
{
   setRequestData(SIP_REGISTER_METHOD,
                  registrarServerUri, // uri
                  registererUri, // from
                  registerAsUri, // to
                  callId,
                  sequenceNumber,
                  takeCallsAtUri // Contact
      );

   setExpiresField(expiresInSeconds);
}

void SipMessage::setReinviteData(SipMessage* invite,
                                 const char* farEndContact,
                                 const char* contactUrl,
                                 UtlBoolean inviteFromThisSide,
                                 const char* routeField,
                                 const char* rtpAddress,
                                 int rtpAudioPort,
                                 int rtcpAudioPort,
                                 int rtpVideoPort,
                                 int rtcpVideoPort,
                                 int sequenceNumber,
                                 int numRtpCodecs,
                                 SdpCodec* rtpCodecs[],
                                 SdpSrtpParameters* srtpParams,
                                 int sessionReinviteTimer)
{
    UtlString toField;
    UtlString fromField;
    UtlString callId;
    UtlString contactUri;
    UtlString lastResponseContact;

    setInterfaceIpPort(invite->getInterfaceIp(), invite->getInterfacePort()) ;

    // Get the to, from and callId fields
    if(inviteFromThisSide)
    {
      invite->getToField(&toField);
      invite->getFromField(&fromField);
    }
    else// Reverse the to from, this invite came from the other side
    {
      invite->getToField(&fromField);
      invite->getFromField(&toField);
    }

   invite->getCallIdField(&callId);

   if (farEndContact)
      lastResponseContact.append(farEndContact);

   if ( !inviteFromThisSide && lastResponseContact.isNull())
   {
      //if invite from other side and LastResponseContact is null because there has not been any
      //final responses from the other side yet ...check the otherside's invite request and get the
      //contact field from the request
       invite->getContactUri(0, &lastResponseContact);
    }

   setInviteData(fromField,
         toField,
         lastResponseContact,
         contactUrl,
         callId,
         rtpAddress,
         rtpAudioPort,
         rtcpAudioPort,
         rtpVideoPort,
         rtcpVideoPort,
         srtpParams,
         sequenceNumber,
         numRtpCodecs,
         rtpCodecs,
         sessionReinviteTimer);

    // Set the route field if present
    if(routeField && routeField[0])
    {
        setRouteField(routeField);
    }

}

void SipMessage::setReinviteData(SipMessage* invite,
                                 const char* farEndContact,
                                 const char* contactUrl,
                                 UtlBoolean inviteFromThisSide,
                                 const char* routeField,
                                 int sequenceNumber,
                                 int sessionReinviteTimer)
{
    UtlString toField;
    UtlString fromField;
    UtlString callId;
    UtlString contactUri;
    UtlString lastResponseContact;

    setInterfaceIpPort(invite->getInterfaceIp(), invite->getInterfacePort()) ;

    // Get the to, from and callId fields
    if(inviteFromThisSide)
    {
      invite->getToField(&toField);
      invite->getFromField(&fromField);
    }
    else// Reverse the to from, this invite came from the other side
    {
      invite->getToField(&fromField);
      invite->getFromField(&toField);
    }

   invite->getCallIdField(&callId);

   if (farEndContact)
      lastResponseContact.append(farEndContact);

   if ( !inviteFromThisSide && lastResponseContact.isNull())
   {
      //if invite from other side and LastResponseContact is null because there has not been any
      //final responses from the other side yet ...check the otherside's invite request and get the
      //contact field from the request
       invite->getContactUri(0, &lastResponseContact);
    }

   setInviteData(fromField,
         toField,
         lastResponseContact,
         contactUrl,
         callId,
         sequenceNumber,
         sessionReinviteTimer);

    // Set the route field if present
    if(routeField && routeField[0])
    {
        setRouteField(routeField);
    }

}

void SipMessage::setInviteData(const char* fromField,
                               const char* toField,
                               const char* farEndContact,
                               const char* contactUrl,
                               const char* callId,
                               const char* rtpAddress,
                               int rtpAudioPort,
                               int rtcpAudioPort,
                               int rtpVideoPort,
                               int rtcpVideoPort,
                               SdpSrtpParameters* srtpParams,
                               int sequenceNumber,
                               int numRtpCodecs,
                               SdpCodec* rtpCodecs[],
                               int sessionReinviteTimer,
                               const char* headerPAI)
{
   setInviteData(fromField,
                 toField,
                 farEndContact,
                 contactUrl,
                 callId,
                 sequenceNumber,
                 sessionReinviteTimer,
                 headerPAI);

   addSdpBody(rtpAddress, rtpAudioPort, rtcpAudioPort,
              rtpVideoPort, rtcpVideoPort,
              numRtpCodecs, rtpCodecs,
              srtpParams);
}

void SipMessage::setInviteData(const char* fromField,
                               const char* toField,
                               const char* farEndContact,
                               const char* contactUrl,
                               const char* callId,
                               int sequenceNumber,
                               int sessionReinviteTimer,
                               const char* headerPAI)
{
   UtlString bodyString;
   UtlString uri;

   Url toUrl(toField);
   // Create the top header line

   // If we have a contact for the other side use it
    // for the URI, otherwise use the To field
   if (farEndContact && *farEndContact)
   {
      uri = farEndContact;
   }
   else
    {
        // Clean out the header field parameters if they exist
        Url uriUrl(toUrl); // copy constructor is more efficient than parsing the toField string
        //uri.append(uriUrl.toString());
        uriUrl.removeHeaderParameters();
        uriUrl.getUri(uri);
    }

   // Construct a PAI for the SipLine
   if (headerPAI != NULL && strlen(headerPAI) != 0)
   {
       Url fromUrl(fromField);
       UtlString idenStr(headerPAI);

       //fromUrl.setHeaderParameter(SIP_CALLID_FIELD,callId);
#ifdef TEST_PRINT
       OsSysLog::add(FAC_SIP, PRI_DEBUG,
                     "SipMessage::setInviteData "
                     "from='%s' headerPAI '%s' idenStr '%s' callId '%s'",
                     fromField, headerPAI, idenStr.data(), callId);
#endif

       SipXauthIdentity pAuthIdentity;
       pAuthIdentity.setIdentity(idenStr);
       pAuthIdentity.encodeUri(fromUrl, callId, fromUrl);

       {
           // Check for header fields in the To URL
           UtlString fheaderName;
           UtlString fheaderValue;
           int fheaderIndex = 0;
           // Look through the headers and add them to the message
           while(fromUrl.getHeaderParameter(fheaderIndex, fheaderName, fheaderValue))
           {
               // If the header is allowed to be passed through
               if(isUrlHeaderAllowed(fheaderName.data()))
               {
                  if (isUrlHeaderUnique(fheaderName.data()))
                  {
                     // If the field exists, change it, if does not exist, create it.
                     setHeaderValue(fheaderName.data(), fheaderValue.data(), 0);
                  }
                  else
                  {
                     addHeaderField(fheaderName.data(), fheaderValue.data());
                  }
 #ifdef TEST_PRINT
                  OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                "SipMessage::setInviteData "
                                "fname=%s, fvalue=%s\n",
                                fheaderName.data(), fheaderValue.data());
 #endif
               }
               else
               {
                  OsSysLog::add(FAC_SIP, PRI_WARNING,
                                "SipMessage::setInviteData "
                                "URL fheader '%s: %s' may not be added using a header parameter",
                                fheaderName.data(), fheaderValue.data());
               }

               fheaderIndex++;
           }
       }
   }

    // Check for header fields in the To URL
    UtlString headerName;
    UtlString headerValue;
    int headerIndex = 0;
    // Look through the headers and add them to the message
    while(toUrl.getHeaderParameter(headerIndex, headerName, headerValue))
    {
        // If the header is allowed to be passed through
        if(isUrlHeaderAllowed(headerName.data()))
        {
           if (isUrlHeaderUnique(headerName.data()))
           {
              // If the field exists, change it, if does not exist, create it.
              setHeaderValue(headerName.data(), headerValue.data(), 0);
           }
           else
           {
              addHeaderField(headerName.data(), headerValue.data());
           }
#ifdef TEST_PRINT
           OsSysLog::add(FAC_SIP, PRI_DEBUG,
                         "SipMessage::setInviteData "
                         "name=%s, value=%s\n",
                         headerName.data(), headerValue.data());
#endif
        }
        else
        {
           OsSysLog::add(FAC_SIP, PRI_WARNING,
                         "SipMessage::setInviteData "
                         "URL header '%s: %s' may not be added using a header parameter",
                         headerName.data(), headerValue.data());
        }

        headerIndex++;
    }

    // Remove the header fields from the URL as they
    // have been added to the message
    toUrl.removeHeaderParameters();
    UtlString toFieldString;
    toUrl.toString(toFieldString);

    setRequestData(SIP_INVITE_METHOD,
                   uri, // URI
                   fromField,
                   toFieldString.data(),
                   callId,
                   sequenceNumber,
                   contactUrl);

    // Set the session timer in seconds
    if(sessionReinviteTimer > 0)
    {
        setSessionExpires(sessionReinviteTimer);
    }

#ifdef TEST
   //osPrintf("SipMessage::setInviteData rtpAddress: %s\n", rtpAddress);
#endif
}

void SipMessage::addSdpBody(const char* rtpAddress, int rtpAudioPort, int rtcpAudioPort,
                            int rtpVideoPort, int rtcpVideoPort,
                            int numRtpCodecs, SdpCodec* rtpCodecs[],
                            SdpSrtpParameters* srtpParams)
{
   if(numRtpCodecs > 0)
   {
      UtlString bodyString;
      ssize_t len;

      // Create and add the SDP body
      SdpBody* sdpBody = new SdpBody();
      sdpBody->setStandardHeaderFields("phone-call",
                                       NULL,
                                       NULL,
                                       rtpAddress); // Originator address
      sdpBody->addAudioCodecs(rtpAddress, rtpAudioPort, rtcpAudioPort,
                              rtpVideoPort, rtcpVideoPort,
                              numRtpCodecs, rtpCodecs,
                              *srtpParams);

      setBody(sdpBody);

      // Add the content type for the body
      setContentType(SDP_CONTENT_TYPE);

      // Add the content length
      sdpBody->getBytes(&bodyString, &len);
      setContentLength(len);
   }
}

const SdpBody* SipMessage::getSdpBody(const char* derPkcs12,
                                      int derPkcs12Length,
                                      const char* pkcs12SymmetricKey) const
{
    const SdpBody* body = NULL;
    UtlString contentType;
    UtlString sdpType(SDP_CONTENT_TYPE);
    UtlString smimeType(CONTENT_SMIME_PKCS7);

    getContentType(&contentType);

    // Make them all lower case so they compare
    contentType.toLower();
    sdpType.toLower();
    smimeType.toLower();

    // If the body is of SDP type, return it
    if(contentType.compareTo(sdpType) == 0)
    {
        body = convertToSdpBody(getBody());
    }

    // If we have a private key and this is a S/MIME body
    else if(derPkcs12 &&
            derPkcs12Length > 0 &&
            pkcs12SymmetricKey &&
            contentType.compareTo(smimeType))
    {
        SmimeBody* smimeBody = (SmimeBody*) getBody();

        // Try to decrypt if it has not already been decrypted
        if(! smimeBody->isDecrypted())
        {
            // Try to decrypt using the given private key and cert.
            smimeBody->decrypt(derPkcs12,
                               derPkcs12Length,
                               pkcs12SymmetricKey);
        }

        // If it did not get encrypted, act like there is no SDP body
        if(smimeBody->isDecrypted())
        {
            const HttpBody* decryptedHttpBody = smimeBody->getDecyptedBody();
            // If the decrypted body is an SDP body type, use it
            if(strcmp(decryptedHttpBody->getContentType(), sdpType) == 0)
            {
                body = convertToSdpBody(decryptedHttpBody);
            }
        }
        else
        {
            OsSysLog::add(FAC_SIP, PRI_WARNING, "Could not decrypt S/MIME body");
        }
    }

    // Else if this is a multipart MIME body see
    // if there is an SDP part
    else
    {
        const HttpBody* multipartBody = getBody();
        if(multipartBody  && multipartBody->isMultipart())
        {
            int partIndex = 0;
            const HttpBody* bodyPart = NULL;
            while ((bodyPart = multipartBody->getMultipart(partIndex)))
            {
                if(strcmp(bodyPart->getContentType(), SDP_CONTENT_TYPE) == 0)
                {
                    // Temporarily disable while fixing multipart bodies
                    //body = (const SdpBody*) bodyPart;
                    body = convertToSdpBody(bodyPart);
                    break;
                }

                // Check for S/MIME body
                else if(strcmp(bodyPart->getContentType(), smimeType) == 0 &&
                        derPkcs12 &&
                        derPkcs12Length > 0 &&
                        pkcs12SymmetricKey)
                {
                    SmimeBody* smimeBody = (SmimeBody*) bodyPart;

                    // Try to decrypt if it has not already been decrypted
                    if(! smimeBody->isDecrypted())
                    {
                        // Try to decrypt using the given private key and cert.
                        smimeBody->decrypt(derPkcs12,
                                           derPkcs12Length,
                                           pkcs12SymmetricKey);
                    }

                    // If it did not get encrypted, act like there is no SDP body
                    if(smimeBody->isDecrypted())
                    {
                        const HttpBody* decryptedHttpBody = smimeBody->getDecyptedBody();
                        // If the decrypted body is an SDP body type, use it
                        if(strcmp(decryptedHttpBody->getContentType(), sdpType) == 0)
                        {
                            body = convertToSdpBody(decryptedHttpBody);
                            break;
                        }
                    }
                    else
                    {
                        OsSysLog::add(FAC_SIP, PRI_WARNING, "Could not decrypt S/MIME body");
                    }
                }
                partIndex++ ;
            }
        }
    }

    return(body);
}


UtlBoolean SipMessage::hasSdpBody(const char* derPkcs12,
                                  int derPkcs12Length,
                                  const char* pkcs12SymmetricKey) const
{
    const SdpBody * sdpBody = getSdpBody(derPkcs12, derPkcs12Length, pkcs12SymmetricKey);
    if ( sdpBody )
    {
        delete sdpBody;
        return TRUE;
    }
    return FALSE;
}

void SipMessage::setRequestData(const char* method, const char* uri,
                                const char* fromField, const char* toField,
                                const char* callId,
                                int sequenceNumber,
                                const char* contactUrl)
{
   // Create the top header line
   setSipRequestFirstHeaderLine(method, uri, SIP_PROTOCOL_VERSION);

   // Add the From field
   setRawFromField(fromField);

   // Add the To field
   setRawToField(toField);

   // Add the Call-Id field
   setCallIdField(callId);

   // Add the CSeq field
   setCSeqField(sequenceNumber, method);

   // Add the Contact field, if any.
   if(contactUrl && *contactUrl)
   {
      setContactField(contactUrl);
   }
}

void SipMessage::setResponseData(int statusCode,
                                 const char* statusText,
                                 const char* fromField,
                                 const char* toField,
                                 const char* callId,
                                 int sequenceNumber,
                                 const char* sequenceMethod,
                                 const char* localContact)
{
   // Create the top header line.
   setResponseFirstHeaderLine(SIP_PROTOCOL_VERSION, statusCode, statusText);

   // Add the From field.
   setRawFromField(fromField);

   // Add the To field.
   // Add the To-tag to the To field, if necessary.
   // @TODO@ This test for the presence of the 'tag' URI-parameter
   // isn't completely correct, but it is fast and works in all cases
   // I've ever seen.
   // To do this right requires the caller telling us whether a new
   // tag is required, or properly parsing toField.
   if (   statusCode != SIP_TRYING_CODE
       && strstr(toField, ";tag=") == NULL)
   {
      UtlString tagValue;
      CallId::getNewTag(tagValue);
      UtlString toFieldS(toField);
      setUriParameter(&toFieldS, "tag", tagValue);
      setRawToField(toFieldS);
   }
   else
   {
      setRawToField(toField);
   }

   // Add the Call-Id field.
   setCallIdField(callId);

   // Add the CSeq field.
   if (sequenceNumber >= 0)
   {
      setCSeqField(sequenceNumber, sequenceMethod);
   }

   // Add the local Contact.
   if (localContact)
   {
      setContactField(localContact);
   }
}

void SipMessage::setTryingResponseData(const SipMessage* request)
{
   setInterfaceIpPort(request->getInterfaceIp(), request->getInterfacePort()) ;
   setResponseData(request, SIP_TRYING_CODE, SIP_TRYING_TEXT);
}

void SipMessage::setInviteRingingData(const SipMessage* inviteRequest,
                                      const char* localContact)
{
   setInterfaceIpPort(inviteRequest->getInterfaceIp(), inviteRequest->getInterfacePort()) ;
   setResponseData(inviteRequest, SIP_RINGING_CODE, SIP_RINGING_TEXT);

   if (localContact)
   {
      setContactField(localContact) ;
   }
}

void SipMessage::setQueuedResponseData(const SipMessage* inviteRequest)
{
   setInterfaceIpPort(inviteRequest->getInterfaceIp(), inviteRequest->getInterfacePort()) ;
   setResponseData(inviteRequest, SIP_QUEUED_CODE, SIP_QUEUED_TEXT);
}

void SipMessage::setInviteBusyData(const char* fromField, const char* toField,
               const char* callId,
               int sequenceNumber)
{
      setResponseData(SIP_BUSY_CODE, SIP_BUSY_TEXT,
                     fromField, toField,
                     callId, sequenceNumber, SIP_INVITE_METHOD);
}

void SipMessage::setBadTransactionData(const SipMessage* inviteRequest)
{
   setInterfaceIpPort(inviteRequest->getInterfaceIp(), inviteRequest->getInterfacePort()) ;
   setResponseData(inviteRequest, SIP_BAD_TRANSACTION_CODE,
                   SIP_BAD_TRANSACTION_TEXT);
}

void SipMessage::setBadSubscriptionData(const SipMessage* inviteRequest)
{
   setInterfaceIpPort(inviteRequest->getInterfaceIp(), inviteRequest->getInterfacePort()) ;
   setResponseData(inviteRequest, SIP_BAD_SUBSCRIPTION_CODE,
                   SIP_BAD_SUBSCRIPTION_TEXT);
}

void SipMessage::setLoopDetectedData(const SipMessage* inviteRequest)
{
   setInterfaceIpPort(inviteRequest->getInterfaceIp(), inviteRequest->getInterfacePort()) ;
   setResponseData(inviteRequest, SIP_LOOP_DETECTED_CODE,
        SIP_LOOP_DETECTED_TEXT);
   setRequestDiagBody(*inviteRequest);
}

void SipMessage::setInviteBusyData(const SipMessage* inviteRequest)
{
   UtlString fromField;
   UtlString toField;
   UtlString callId;
   int sequenceNum;
   UtlString sequenceMethod;

   setInterfaceIpPort(inviteRequest->getInterfaceIp(), inviteRequest->getInterfacePort()) ;
   inviteRequest->getFromField(&fromField);
   inviteRequest->getToField(&toField);
   inviteRequest->getCallIdField(&callId);
   inviteRequest->getCSeqField(&sequenceNum, &sequenceMethod);

   setInviteBusyData(fromField.data(), toField.data(),
                     callId.data(), sequenceNum);

   setViaFromRequest(inviteRequest);
}

void SipMessage::setForwardResponseData(const SipMessage* request,
                     const char* forwardAddress)
{
   setInterfaceIpPort(request->getInterfaceIp(), request->getInterfacePort()) ;
   setResponseData(request, SIP_TEMPORARY_MOVE_CODE, SIP_TEMPORARY_MOVE_TEXT);

   // Add the contact field for the forward address
   UtlString contactAddress;
   //UtlString address;
   //UtlString protocol;
   //UtlString user;
   //UtlString userLabel;
   //int port;
   //parseAddressFromUri(forwardAddress, &address, &port,
   // &protocol, &user, &userLabel);
   //buildSipUri(&contactAddress, address.data(), port,
   // protocol.data(), user.data(), userLabel.data());
    Url contactUrl(forwardAddress);
    contactUrl.removeFieldParameters();
    contactUrl.toString(contactAddress);
   setContactField(contactAddress.data());
}

void SipMessage::setInviteBadCodecs(const SipMessage* inviteRequest,
                                    SipUserAgent* ua)
{
   setInterfaceIpPort(inviteRequest->getInterfaceIp(), inviteRequest->getInterfacePort()) ;
   char warningCodeString[MAXIMUM_INTEGER_STRING_LENGTH + 1];
   UtlString warningField;

   //setInviteBusyData(fromField.data(), toField.data(),
   // callId.data(), sequenceNum);
   setResponseData(inviteRequest, SIP_REQUEST_NOT_ACCEPTABLE_HERE_CODE,
                   SIP_REQUEST_NOT_ACCEPTABLE_HERE_TEXT);

   // Add a media not available warning
   // The text message must be a quoted string
   sprintf(warningCodeString, "%d ", SIP_WARN_MEDIA_INCOMPAT_MEDIA_CODE);
   warningField.append(warningCodeString);

   // Construct the agent field from information extracted from the
   // SipUserAgent.
   UtlString address;
   int port;
   // Get the address/port for UDP, since it's too hard to figure out what
   // protocol this message arrived on.
   ua->getViaInfo(OsSocket::UDP, address, port);
   warningField.append(address);
   if (port != 0)               // PORT_NONE
   {
      sprintf(warningCodeString, ":%d", port);
      warningField.append(warningCodeString);
   }

   warningField.append(" \"");
   warningField.append(SIP_WARN_MEDIA_INCOMPAT_MEDIA_TEXT);
   warningField.append("\"");
   addHeaderField(SIP_WARNING_FIELD, warningField.data());
}

void SipMessage::setRequestBadMethod(const SipMessage* request,
                            const char* allowedMethods)
{
   setInterfaceIpPort(request->getInterfaceIp(), request->getInterfacePort()) ;
   setResponseData(request, SIP_BAD_METHOD_CODE, SIP_BAD_METHOD_TEXT);

   // Add a methods supported field
   addHeaderField(SIP_ALLOW_FIELD, allowedMethods);
}

void SipMessage::setRequestUnimplemented(const SipMessage* request)
{
   setInterfaceIpPort(request->getInterfaceIp(), request->getInterfacePort()) ;
   setResponseData(request, SIP_UNIMPLEMENTED_METHOD_CODE,
        SIP_UNIMPLEMENTED_METHOD_TEXT);
}

void SipMessage::setRequestBadExtension(const SipMessage* request,
                            const char* disallowedExtension)
{
   setInterfaceIpPort(request->getInterfaceIp(), request->getInterfacePort()) ;
   setResponseData(request, SIP_BAD_EXTENSION_CODE, SIP_BAD_EXTENSION_TEXT);

   // Add a methods supported field
   addHeaderField(SIP_UNSUPPORTED_FIELD, disallowedExtension);
}

void SipMessage::setRequestBadContentEncoding(const SipMessage* request,
                   const char* allowedEncodings)
{
   setInterfaceIpPort(request->getInterfaceIp(), request->getInterfacePort()) ;
   setResponseData(request, SIP_BAD_MEDIA_CODE, SIP_BAD_MEDIA_TEXT);

   // Add a encodings supported field
   addHeaderField(SIP_ACCEPT_ENCODING_FIELD, allowedEncodings);
   addHeaderField(SIP_ACCEPT_FIELD, "application/sdp");

   const char* explanation = "Content Encoding value not supported";
   setBody(new HttpBody(explanation, strlen(explanation), CONTENT_TYPE_TEXT_PLAIN));
}

void SipMessage::setRequestBadAddress(const SipMessage* request)
{
   setInterfaceIpPort(request->getInterfaceIp(), request->getInterfacePort()) ;
   setResponseData(request, SIP_BAD_ADDRESS_CODE, SIP_BAD_ADDRESS_TEXT);
}

void SipMessage::setRequestBadVersion(const SipMessage* request)
{
   setInterfaceIpPort(request->getInterfaceIp(), request->getInterfacePort()) ;
   setResponseData(request, SIP_BAD_VERSION_CODE, SIP_BAD_VERSION_TEXT);
}

void SipMessage::setRequestBadRequest(const SipMessage* request)
{
   setInterfaceIpPort(request->getInterfaceIp(), request->getInterfacePort()) ;
   setResponseData(request, SIP_BAD_REQUEST_CODE, SIP_BAD_REQUEST_TEXT);
}

void SipMessage::setRequestBadUrlType(const SipMessage* request)
{
   setInterfaceIpPort(request->getInterfaceIp(), request->getInterfacePort()) ;
   setResponseData(request, SIP_UNSUPPORTED_URI_SCHEME_CODE,
                   SIP_UNSUPPORTED_URI_SCHEME_TEXT);
}

void SipMessage::setRequestDiagBody(SipMessage request)
{
   // The setBody method frees up the body before
   // setting the new one, if there is a body
   // We remove the body so that we can serialize
   // the message without getting the body
   request.setBody(NULL);

   UtlString sipFragString;
   ssize_t sipFragLen;
   request.getBytes(&sipFragString, &sipFragLen);

   // Create a body to contain the Vias from the request
   HttpBody* sipFragBody =
      new HttpBody(sipFragString.data(),
                   sipFragLen,
                   CONTENT_TYPE_MESSAGE_SIPFRAG);

   // Attach the body to the response
   setBody(sipFragBody);

   // Set the content type of the body to be sipfrag
   setContentType(CONTENT_TYPE_MESSAGE_SIPFRAG);
}

void SipMessage::setInviteOkData(const char* fromField,
                                 const char* toField,
                                 const char* callId,
                                 const SdpBody* inviteSdp,
                                 const char* rtpAddress,
                                 int rtpAudioPort,
                                 int rtcpAudioPort,
                                 int rtpVideoPort,
                                 int rtcpVideoPort,
                                 int numRtpCodecs,
                                 SdpCodec* rtpCodecs[],
                                 SdpSrtpParameters& srtpParams,
                                 int sequenceNumber,
                                 const char* localContact)
{
   SdpBody* sdpBody;
   UtlString bodyString;
   ssize_t len;

   setResponseData(SIP_OK_CODE, SIP_OK_TEXT, fromField, toField,
                     callId, sequenceNumber, SIP_INVITE_METHOD, localContact);

   // Create and add the SDP body
   sdpBody = new SdpBody();
   sdpBody->setStandardHeaderFields("phone-call",
                                    NULL,
                                    NULL,
                                    rtpAddress); //originator address

   // If the INVITE SDP is present pick the best
   if (inviteSdp)
   {
      sdpBody->addAudioCodecs(rtpAddress, rtpAudioPort, rtcpAudioPort,
                              rtpVideoPort, rtcpVideoPort,
                              numRtpCodecs, rtpCodecs, srtpParams,
                              inviteSdp);
   }

   else
   {
      sdpBody->addAudioCodecs(rtpAddress, rtpAudioPort, rtcpAudioPort,
                              rtpVideoPort, rtcpVideoPort,
                              numRtpCodecs, rtpCodecs, srtpParams);
   }
   setBody(sdpBody);

   // Add the content type
   setContentType(SDP_CONTENT_TYPE);

   // Add the content length
   sdpBody->getBytes(&bodyString, &len);
   setContentLength(len);
}

void SipMessage::setInviteOkData(const SipMessage* inviteRequest,
                const char* rtpAddress, int rtpAudioPort, int rtcpAudioPort,
                int rtpVideoPort, int rtcpVideoPort,
                int numRtpCodecs, SdpCodec* rtpCodecs[],
                SdpSrtpParameters& srtpParams,
                int maxSessionExpiresSeconds, const char* localContact)
{
   UtlString fromField;
   UtlString toField;
   UtlString callId;
   int sequenceNum;
   UtlString sequenceMethod;
   const SdpBody* inviteSdp = NULL;

   setInterfaceIpPort(inviteRequest->getInterfaceIp(), inviteRequest->getInterfacePort()) ;
   inviteRequest->getFromField(&fromField);
   inviteRequest->getToField(&toField);
   inviteRequest->getCallIdField(&callId);
   inviteRequest->getCSeqField(&sequenceNum, &sequenceMethod);
   inviteSdp = inviteRequest->getSdpBody();

   setInviteOkData(fromField.data(), toField.data(), callId,
               inviteSdp, rtpAddress, rtpAudioPort, rtcpAudioPort,
               rtpVideoPort, rtcpVideoPort,
               numRtpCodecs, rtpCodecs,
               srtpParams,
               sequenceNum, localContact);

   if ( inviteSdp )
   {
       delete inviteSdp;
   }

   setViaFromRequest(inviteRequest);

    UtlString recordRouteField;
    int recordRouteIndex;

    for( recordRouteIndex = 0;
         inviteRequest->getRecordRouteField( recordRouteIndex, &recordRouteField );
         recordRouteIndex++ )
    {
        // Don't do this as it will result in the UA at the other
        // end routing to itself.
        // Add the contact (if present) to the end of the record-route
        //UtlString contactUri;
        //if(inviteRequest->getContactUri(0, &contactUri))
        //{
        //    recordRouteField.insert(0, ',');
        //    recordRouteField.insert(0, contactUri.data());
        //}

        setRecordRouteField(recordRouteField.data(), recordRouteIndex);
    }

    int inviteSessionExpires;
    // If max session timer is less than the requested timer length
    // or if the other side did not request a timer, use
    // the max session timer
    if(!inviteRequest->getSessionExpires(&inviteSessionExpires) ||
        (maxSessionExpiresSeconds > 0 &&
        inviteSessionExpires > maxSessionExpiresSeconds))
    {
        inviteSessionExpires = maxSessionExpiresSeconds;
    }
    if(inviteSessionExpires > 0)
    {
        setSessionExpires(inviteSessionExpires);
    }
}

void SipMessage::setOkResponseData(const SipMessage* request,
                                   const char* localContact)
{
   setInterfaceIpPort(request->getInterfaceIp(), request->getInterfacePort()) ;
   // if request contains Path headers and the request is a REGISTER
   // then copy the Path headers in the response
   UtlString method;
   request->getRequestMethod(&method);
   if( method.compareTo(SIP_REGISTER_METHOD) == 0 )
   {
      const char* pathHeaderValue;
      for( int index = 0;  (pathHeaderValue = request->getHeaderValue(index, SIP_PATH_FIELD)); index++ )
      {
         addHeaderField(SIP_PATH_FIELD, pathHeaderValue);
      }
   }
   setResponseData(request, SIP_OK_CODE, SIP_OK_TEXT, localContact);
}

void SipMessage::setNotifyData(const SipMessage *subscribeRequest,
                               int localCSequenceNumber,
                               const char* route,
                               const char* stateField,
                               const char* eventField,
                               const char* id)
{
   UtlString fromField;
   UtlString toField;
   UtlString uri;
   UtlString callId;
   int dummySequenceNum;
   UtlString sequenceMethod;

   setInterfaceIpPort(subscribeRequest->getInterfaceIp(), subscribeRequest->getInterfacePort()) ;
   subscribeRequest->getFromField(&fromField);
   subscribeRequest->getToField(&toField);
   subscribeRequest->getCallIdField(&callId);
   subscribeRequest->getCSeqField(&dummySequenceNum, &sequenceMethod);

    // Set the NOTIFY event type
    if(eventField && *eventField)
    {
       UtlString eventHeaderValue(eventField);
       if (id && *id)
       {
          eventHeaderValue.append(";id=");
          eventHeaderValue.append(id);
       }
       setEventField(eventHeaderValue.data());
    }
    else
    {
        // Try to get it from the subscribe message
        UtlString subscribeEventField;
        subscribeRequest->getEventField(subscribeEventField);
        if(!subscribeEventField.isNull())
        {
            setEventField(subscribeEventField.data());
        }
    }

    // Set the Subscription-State header
    if(stateField && *stateField)
    {
      setHeaderValue(SIP_SUBSCRIPTION_STATE_FIELD, stateField);
    }
    else
    {
      setHeaderValue(SIP_SUBSCRIPTION_STATE_FIELD, SIP_SUBSCRIPTION_ACTIVE);;
    }

    // Set the route for the NOTIFY request
    if(route && *route)
    {
        setRouteField(route);
    }

    // Use contact if present
    if(subscribeRequest->getContactUri(0, &uri) && !uri.isNull())
    {

    }

    // Use the from field as we have nothing better to use
   else
   {
      uri.append(fromField.data());
   }


   setRequestData(SIP_NOTIFY_METHOD, uri.data(),
        toField.data(), fromField.data(), callId, localCSequenceNumber);
}

void SipMessage::setNotifyData(const char* uri,
                                const char* fromField,
                                const char* toField,
                                const char* callId,
                                int lastNotifyCseq,
                                const char* eventField,
                                const char* id,
                                const char* state,
                                const char* contact,
                                const char* routeField)
{
    // if uri is not set set it to the toField
    UtlString uriStr;
    if( uri && *uri )
    {
        uriStr.append(uri);
    }
    else if ( toField )
    {
        // check for toField  null
        uriStr.append( toField );
    }

    // if contact is not set set it to the fromField
    UtlString contactStr;
    if( contact && *contact )
    {
        contactStr.append(contact);
    }
    else if ( fromField )
    {
        // check for toField  null
        contactStr.append( fromField );
    }

    // Set the NOTIFY event type
    if( eventField && *eventField )
    {
        UtlString eventHeaderValue(eventField);
        if (id && *id)
        {
           eventHeaderValue.append(";id=");
           eventHeaderValue.append(id);
        }
        setEventField(eventHeaderValue.data());
    }

    // Set the Subscription-State header
    if(state && *state)
    {
      setHeaderValue(SIP_SUBSCRIPTION_STATE_FIELD, state);
    }
    else
    {
      setHeaderValue(SIP_SUBSCRIPTION_STATE_FIELD, SIP_SUBSCRIPTION_ACTIVE);
    }

    // Set the route should come from from SUBSCRIBE message
    if( routeField && *routeField )
    {
        setRouteField( routeField );
    }

   setRequestData(
        SIP_NOTIFY_METHOD,
        uriStr.data(),
        fromField,
        toField,
        callId,
        lastNotifyCseq,
        contactStr.data() );
}

void SipMessage::setSubscribeData(const char* uri,
                                  const char* fromField,
                                  const char* toField,
                                  const char* callId,
                                  int cseq,
                                  const char* eventField,
                                  const char* acceptField,
                                  const char* id,
                                  const char* contact,
                                  const char* routeField,
                                  int expiresInSeconds)
{
   setRequestData(SIP_SUBSCRIBE_METHOD, uri,
                  fromField, toField,
                  callId,
                  cseq,
                  contact);

   // Set the event type, if any.
   if (eventField && *eventField)
   {
      UtlString eventHeaderValue(eventField);
      if (id && *id)
      {
         eventHeaderValue.append(";id=");
         eventHeaderValue.append(id);
      }
      setEventField(eventHeaderValue.data());
      setHeaderValue(SIP_EVENT_FIELD, eventHeaderValue, 0);
   }

   // Set the content type, if any.
   if (acceptField && *acceptField)
   {
      setHeaderValue(SIP_ACCEPT_FIELD, acceptField, 0);
   }

   // Set the route, if any.
   if (routeField && *routeField)
   {
      setRouteField(routeField);
   }

   //setExpires
   setExpiresField(expiresInSeconds);
}

void SipMessage::setEnrollmentData(const char* uri,
                       const char* fromField,
                       const char* toField,
                       const char* callId,
                       int CSeq,
                       const char* contactUrl,
                       const char* protocolField,
                       const char* profileField,
                       int expiresInSeconds)
{
    setRequestData(SIP_SUBSCRIBE_METHOD, uri,
                     fromField, toField,
                     callId,
                     CSeq,
                            contactUrl);

    // Set the event type
    setHeaderValue(SIP_EVENT_FIELD, SIP_EVENT_CONFIG, 0);

    // Set the protocols
    setHeaderValue(SIP_CONFIG_ALLOW_FIELD, protocolField, 0);

    // Set the profile
    setHeaderValue(SIP_CONFIG_REQUIRE_FIELD, profileField, 0);

   //setRxpires
   setExpiresField(expiresInSeconds);
}

void SipMessage::setVoicemailData(const char* fromField,
                       const char* toField,
                  const char* uri,
                  const char* contactUrl,
                  const char* callId,
                  int CSeq,
                       int expiresInSeconds)
{
    setRequestData(SIP_SUBSCRIBE_METHOD, uri,
                     fromField, toField,
                     callId,
                     CSeq,
                           contactUrl);
    // Set the event type
    setHeaderValue(SIP_EVENT_FIELD, SIP_EVENT_MESSAGE_SUMMARY, 0);
    // Set the allow field
    setHeaderValue(SIP_ACCEPT_FIELD, CONTENT_TYPE_SIMPLE_MESSAGE_SUMMARY, 0);
   setExpiresField(expiresInSeconds);
}

void SipMessage::setRequestTerminatedResponseData(const SipMessage* request)
{
   setInterfaceIpPort(request->getInterfaceIp(), request->getInterfacePort()) ;
   setResponseData(request, SIP_REQUEST_TERMINATED_CODE, SIP_REQUEST_TERMINATED_TEXT);
}

void SipMessage::setRequestUnauthorized(const SipMessage* request,
                            const char* authenticationScheme,
                            const char* authenticationRealm,
                            const char* authenticationNonce,
                            const char* authenticationOpaque,
                            enum HttpEndpointEnum authEntity)
{
    setInterfaceIpPort(request->getInterfaceIp(), request->getInterfacePort()) ;
    if(authEntity == SERVER)
    {
        setResponseData(request,
                        HTTP_UNAUTHORIZED_CODE,
                        HTTP_UNAUTHORIZED_TEXT);
    }
    else
    {
        setResponseData(request,
                        HTTP_PROXY_UNAUTHORIZED_CODE,
                        HTTP_PROXY_UNAUTHORIZED_TEXT);
    }

    setAuthenticateData(authenticationScheme, authenticationRealm,
                        authenticationNonce, authenticationOpaque,
                        NULL,
                        authEntity);
}

// This method is needed to cover the symetrical situation which is
// specific to SIP authorization (i.e. authentication and authorize
// fields may be in either requests or responses
UtlBoolean SipMessage::verifyMd5Authorization(const char* userId,
                                              const char* password,
                                              const char* nonce,
                                              const char* realm,
                                              const char* uri,
                                              enum HttpEndpointEnum authEntity) const
{
    UtlString uriString;
    UtlString method;

    if(isResponse())
    {
        int seqNum;
         // What is the correct URI for a response Authorization
        if(uri)
        {
            uriString.append(uri);
        }
        getCSeqField(&seqNum, &method);
    }
    else
    {
       // If the uri (should be Auth header uri parameter) is
       // passed in use it.
       if(uri)
       {
          uriString.append(uri);
       }
       // Otherwise dig out the request URI.  Note: it is not a good
       // idea to use the request uri to validate the digest hash as
       // it may not exactly match the Auth header uri parameter (in
       // which the validation will fail).
       else
       {
          getRequestUri(&uriString);
          OsSysLog::add(FAC_SIP,PRI_DEBUG,
                        "SipMessage::verifyMd5Authorization "
                        "using request URI: %s instead of Auth header uri parameter for digest\n",
                        uriString.data());
       }
       getRequestMethod(&method);
    }

#ifdef TEST
    OsSysLog::add(FAC_SIP,PRI_DEBUG,
                  "SipMessage::verifyMd5Authorization - "
                  "userId='%s', password='%s', nonce='%s', "
                  "realm='%s', uri='%s', method='%s' \n",
                   userId, password, nonce,
                  realm, uriString.data(), method.data());
#endif

    UtlBoolean isAllowed = FALSE;
    isAllowed = HttpMessage::verifyMd5Authorization(userId,
                                                    password,
                                                    nonce,
                                                    realm,
                                                    method.data(),
                                                    uriString.data(),
                                                    authEntity);
    return isAllowed;
}

void SipMessage::setResponseData(const SipMessage* request,
                         int responseCode,
                         const char* responseText,
                         const char* localContact,
                         UtlBoolean echoRecordRouteIfApplicable)
{
   setInterfaceIpPort(request->getInterfaceIp(), request->getInterfacePort()) ;
   UtlString fromField;
   UtlString toField;
   UtlString callId;
   int sequenceNum;
   UtlString sequenceMethod;

   request->getFromField(&fromField);
   request->getToField(&toField);
   request->getCallIdField(&callId);
   request->getCSeqField(&sequenceNum, &sequenceMethod);

   setResponseData(responseCode, responseText,
      fromField.data(), toField.data(), callId,
               sequenceNum, sequenceMethod.data(), localContact) ;

   setViaFromRequest(request);

   // If this is a response for a Request that accepts Record-Routes
   // then echo the Record-Routes in the Response
   if(echoRecordRouteIfApplicable &&
      request->isRecordRouteAccepted())
   {
      UtlString recordRouteField;
      int recordRouteIndex;

      for( recordRouteIndex = 0;
           request->getRecordRouteField( recordRouteIndex, &recordRouteField );
           recordRouteIndex++ )
      {
         setRecordRouteField(recordRouteField.data(), recordRouteIndex);
      }
   }
}

void SipMessage::setAckData(const char* uri,
                     const char* fromField,
                     const char* toField,
               const char* callId,
               int sequenceNumber)
{
   setRequestData(SIP_ACK_METHOD, uri,
                     fromField, toField,
                     callId, sequenceNumber);
}

void SipMessage::setAckData(const SipMessage* inviteResponse,
                     const SipMessage* inviteRequest,
                            const char* contact,
                            int sessionTimerExpires)
{
   UtlString fromField;
   UtlString toField;
   UtlString uri;
   UtlString callId;
   int sequenceNum;
   int responseCode ;
   UtlString sequenceMethod;
   UtlString requestContact;
   UtlBoolean SetDNSParameters = FALSE;

   setInterfaceIpPort(inviteResponse->getInterfaceIp(), inviteResponse->getInterfacePort()) ;
   inviteResponse->getFromField(&fromField);
   inviteResponse->getToField(&toField);
   responseCode = inviteResponse->getResponseStatusCode();

   // SDUA
   //Set URI

   //Only for 2xx responses check the record route and contact field of response
   if ( responseCode >= SIP_OK_CODE && responseCode < SIP_MULTI_CHOICE_CODE)
   {
       // Set route field if recordRoute was set.
      UtlString routeField;
      UtlString requestToField;
      if(inviteResponse->buildRouteField(&routeField))
      {
         setRouteField(routeField.data());

      }

        // Loose_route always put contact in URI
        inviteResponse->getContactUri( 0 , &uri);
        if (uri.isNull())
        {
            if(inviteRequest)
            inviteRequest->getRequestUri(&uri);
          else
            uri.append(toField.data());
        }

      //if no record route and no contact filed which is a problem of the
      //other side because they should have a contact field
      //We should be more tolerant and use the request uri of INVITE
      //or the to field
      if(uri.isNull())
      {
         if(inviteRequest)
         {
            inviteRequest->getRequestUri(&uri);
         }
         else
         {
            uri.append(toField.data());
         }
      }
   }
   else
   {
      //set uri from request if no contact field in the response
      UtlString routeField;
      if (inviteRequest)
      {
         inviteRequest->getRequestUri(&uri);
         OsSysLog::add(FAC_SIP,
                       (uri.data()[0] == '<') ? PRI_ERR : PRI_DEBUG,
                       "SipMessage::setAckData inviteRequest->getRequestUri() = '%s'",
                       uri.data());
         inviteRequest->getRouteField(&routeField);
         if(!routeField.isNull())
            setRouteField(routeField);
      }
      else
         uri.append(toField.data());

      SetDNSParameters = TRUE;
   }

   //set senders contact
   if(contact && *contact)
   {
      setContactField(contact);
   }
   else if ( inviteRequest)
   {
      if( inviteRequest->getContactField(0, requestContact))
      {
         setContactField(requestContact);
      }
   }

   //if no record route or contact add sticky DNS
   if ( SetDNSParameters)
   {
      // set the DNS fields
      UtlString protocol;
      UtlString address;
      UtlString port;

      if ( inviteResponse->getDNSField(&protocol , &address , &port))
      {
         setDNSField(protocol , address , port);
      }
   }

   inviteResponse->getCallIdField(&callId);
   inviteResponse->getCSeqField(&sequenceNum, &sequenceMethod);

   setAckData(uri.data(), fromField.data(), toField.data(), callId, sequenceNum);

    if(sessionTimerExpires > 0)
        setSessionExpires(sessionTimerExpires);
}

void SipMessage::setAckErrorData(const SipMessage* byeRequest)
{
   setInterfaceIpPort(byeRequest->getInterfaceIp(), byeRequest->getInterfacePort()) ;
   setResponseData(byeRequest, SIP_BAD_REQUEST_CODE, SIP_BAD_REQUEST_TEXT);
}

void SipMessage::setByeData(const char* uri,
                            const char* fromField,
                            const char* toField,
                            const char* callId,
                            const char* localContact,
                            int sequenceNumber)
{
   setRequestData(SIP_BYE_METHOD, uri,
                     fromField, toField,
                     callId, sequenceNumber, localContact);
}

void SipMessage::setByeData(const SipMessage* inviteRequest,
                            const char* remoteContact,
                            UtlBoolean byeToCallee,
                            int localCSequenceNumber,
                            const char* routeField,
                            const char* alsoInviteUri,
                            const char* localContact)
{
   UtlString fromField;
   UtlString toField;
   UtlString uri;
   UtlString callId;
   int dummySequenceNum;
   UtlString sequenceMethod;
   UtlString remoteContactString;

   setInterfaceIpPort(inviteRequest->getInterfaceIp(), inviteRequest->getInterfacePort()) ;

   if (remoteContact)
      remoteContactString.append(remoteContact);

   inviteRequest->getFromField(&fromField);
   inviteRequest->getToField(&toField);
   inviteRequest->getCallIdField(&callId);
   inviteRequest->getCSeqField(&dummySequenceNum, &sequenceMethod);

   if(routeField && *routeField)
   {
       setRouteField(routeField);
   }

   if (!remoteContactString.isNull())
   {
      uri.append(remoteContactString);
   }

    // Use the route uri if set
    if(!uri.isNull())
    {
    }

    // Use the original uri from the INVITE if the INVITE is from
    // this side.
   else if(byeToCallee)
   {
      inviteRequest->getRequestUri(&uri);
   }

    // Use contact if present
    else if(inviteRequest->getContactUri(0, &uri) && !uri.isNull())
    {

    }

   else
   {
      uri.append(fromField.data());
   }

   if(byeToCallee)
   {
      setByeData(uri.data(), fromField.data(), toField.data(), callId, localContact, localCSequenceNumber);
   }
   else
   {
      setByeData(uri.data(), toField.data(), fromField.data(), callId, localContact, localCSequenceNumber);
   }


    if(alsoInviteUri && *alsoInviteUri)
    {
        if(!isRequireExtensionSet(SIP_CALL_CONTROL_EXTENSION))
        {
            addRequireExtension(SIP_CALL_CONTROL_EXTENSION);
        }
        addAlsoUri(alsoInviteUri);
    }

}


void SipMessage::setReferData(const SipMessage* inviteRequest,
                     UtlBoolean referToCallee,
                            int localCSequenceNumber,
                            const char* routeField,
                            const char* contactUrl,
                            const char* remoteContactUrl,
                            const char* transferTargetAddress,
                            const char* targetCallId)
{
   UtlString fromField;
   UtlString toField;
   UtlString uri;
   UtlString callId;
   int dummySequenceNum;
   UtlString sequenceMethod;

   setInterfaceIpPort(inviteRequest->getInterfaceIp(), inviteRequest->getInterfacePort()) ;
   inviteRequest->getFromField(&fromField);
   inviteRequest->getToField(&toField);
   inviteRequest->getCallIdField(&callId);
   inviteRequest->getCSeqField(&dummySequenceNum, &sequenceMethod);

    if(routeField && *routeField)
    {
        setRouteField(routeField);

        // Get the first entry in the route field.
        //getRouteUri(0, &uri);
    }

    if(remoteContactUrl)
    {
        uri = remoteContactUrl;
    }

    // Use the route uri if set
    else if(!referToCallee)
    {
        inviteRequest->getContactUri(0, &uri);
    }

    // Use the original uri from the INVITE if the INVITE is from
    // this side.
   else if(referToCallee)
   {
      inviteRequest->getRequestUri(&uri);
   }

    // Use contact if present
    else if(inviteRequest->getContactUri(0, &uri) && !uri.isNull())
    {

    }

   else
   {
      uri.append(fromField.data());
   }

    UtlString referredByField;
   if(referToCallee)
   {
        setRequestData(SIP_REFER_METHOD, uri.data(),
                  fromField.data(), toField.data(),
                  callId, localCSequenceNumber,
                        contactUrl);

        Url referToUrl(fromField);
        referToUrl.removeFieldParameters();
        referToUrl.includeAngleBrackets();
        referToUrl.toString(referredByField);
   }
   else
   {
        setRequestData(SIP_REFER_METHOD, uri.data(),
                  toField.data(), fromField.data(),
                  callId, localCSequenceNumber,
                        contactUrl);

        Url referToUrl(toField);
        referToUrl.removeFieldParameters();
        referToUrl.includeAngleBrackets();
        referToUrl.toString(referredByField);
   }

    //referredByField.append("; ");
    if(transferTargetAddress && *transferTargetAddress)
    {
        UtlString targetAddress(transferTargetAddress);
        Url targetUrl(targetAddress);
        UtlString referTo;

        if(targetCallId && *targetCallId)
        {
            //targetAddress.append("?Call-ID=");
            //targetAddress.append(targetCallId);
            targetUrl.setHeaderParameter(SIP_CALLID_FIELD, targetCallId);
        }

        // Include angle brackets on the Refer-To header.  We don't
        // need to do this, but it is the friendly thing to do.
        targetUrl.includeAngleBrackets();
        targetUrl.toString(referTo);

        // This stuff went away in the Transfer-05/Refer-02 drafts
        // We need angle brackets for the refer to url in the referred-by
        //targetUrl.includeAngleBrackets();
        //targetUrl.toString(targetAddress);
        //referredByField.append("ref=");
        //referredByField.append(targetAddress);

        setReferredByField(referredByField.data());
        setReferToField(referTo.data());
    }

}

void SipMessage::setReferOkData(const SipMessage* referRequest)
{
   setInterfaceIpPort(referRequest->getInterfaceIp(), referRequest->getInterfacePort()) ;
   setResponseData(referRequest, SIP_OK_CODE, SIP_OK_TEXT);
}

void SipMessage::setReferDeclinedData(const SipMessage* referRequest)
{
   setInterfaceIpPort(referRequest->getInterfaceIp(), referRequest->getInterfacePort()) ;
   setResponseData(referRequest, SIP_DECLINE_CODE, SIP_DECLINE_TEXT);
}

void SipMessage::setReferFailedData(const SipMessage* referRequest)
{
   setInterfaceIpPort(referRequest->getInterfaceIp(), referRequest->getInterfacePort()) ;
   setResponseData(referRequest, SIP_SERVICE_UNAVAILABLE_CODE, SIP_SERVICE_UNAVAILABLE_TEXT);
}

void SipMessage::setOptionsData(const SipMessage* inviteRequest,
                        const char* remoteContact,
                        UtlBoolean optionsToCallee,
                        int localCSequenceNumber,
                        const char* routeField,
                        const char* localContact)
{
   UtlString fromField;
   UtlString toField;
   UtlString uri;
   UtlString callId;
   int dummySequenceNum;
   UtlString sequenceMethod;

   setInterfaceIpPort(inviteRequest->getInterfaceIp(), inviteRequest->getInterfacePort()) ;
   inviteRequest->getFromField(&fromField);
   inviteRequest->getToField(&toField);
   inviteRequest->getCallIdField(&callId);
   inviteRequest->getCSeqField(&dummySequenceNum, &sequenceMethod);

    if(routeField && *routeField)
    {
        setRouteField(routeField);

        // We do not do this any more with loose and strict routing
        // The URI is set to the remote contact (target URI) and
        // the route comes directy from the Record-Route field
        // Get the first entry in the route field.
        //getRouteUri(0, &uri);
    }

   //set the uri to the contact uri returned in the last response from the other side
   if (remoteContact && *remoteContact)
   {
      uri.append(remoteContact);
   }

    // Use the remoteContact uri if set
    if(!uri.isNull())
    {
    }

    // Use the original uri from the INVITE if the INVITE is from
    // this side.
   else if(optionsToCallee)
   {
      inviteRequest->getRequestUri(&uri);
   }

    // Use contact if present
    else if(inviteRequest->getContactUri(0, &uri) && !uri.isNull())
    {

    }

   else
   {
      uri.append(fromField.data());
   }

    UtlString referredByField;
   if(optionsToCallee)
   {
        setRequestData(SIP_OPTIONS_METHOD, uri.data(),
                  fromField.data(), toField.data(),
                  callId, localCSequenceNumber, localContact);

        referredByField = fromField;
   }
   else
   {
        setRequestData(SIP_OPTIONS_METHOD, uri.data(),
                  toField.data(), fromField.data(),
                  callId, localCSequenceNumber, localContact);

        referredByField = toField;
   }
}

void SipMessage::setByeErrorData(const SipMessage* byeRequest)
{
   setInterfaceIpPort(byeRequest->getInterfaceIp(), byeRequest->getInterfacePort()) ;
   setResponseData(byeRequest, SIP_BAD_REQUEST_CODE, SIP_BAD_REQUEST_TEXT);
}

void SipMessage::setCancelData(const char* fromField, const char* toField,
               const char* callId,
               int sequenceNumber)
{
   setRequestData(SIP_CANCEL_METHOD, toField,
                     fromField, toField,
                     callId, sequenceNumber);
}

void SipMessage::setCancelData(const SipMessage* inviteRequest,
                               const UtlString* reasonValue)
{
   UtlString uri;
   UtlString fromField;
   UtlString toField;
   UtlString callId;
   int sequenceNum;
   UtlString sequenceMethod;

   setInterfaceIpPort(inviteRequest->getInterfaceIp(), inviteRequest->getInterfacePort()) ;

   inviteRequest->getFromField(&fromField);
   inviteRequest->getToField(&toField);
   inviteRequest->getCallIdField(&callId);
   inviteRequest->getCSeqField(&sequenceNum, &sequenceMethod);
   inviteRequest->getRequestUri(&uri);

   //setCancelData(fromField.data(), toField.data(), callId, sequenceNum);
   setRequestData(SIP_CANCEL_METHOD, uri,
                  fromField, toField,
                  callId, sequenceNum);

   // Set the Reason header if a value was provided.
   if (reasonValue && !reasonValue->isNull())
   {
      setReasonField(reasonValue->data());
   }
}

void SipMessage::setPublishData(const char* uri,
                                const char* fromField,
                                const char* toField,
                                const char* callId,
                                int cseq,
                                const char* eventField,
                                const char* id,
                                const char* sipIfMatchField,
                                int expiresInSeconds)
{
    setRequestData(SIP_PUBLISH_METHOD, uri,
                   fromField, toField,
                   callId,
                   cseq,
                   NULL);

    // Set the event type
    if( eventField && *eventField )
    {
        UtlString eventHeaderValue(eventField);
        if (id && *id)
        {
           eventHeaderValue.append(";id=");
           eventHeaderValue.append(id);
        }
        setEventField(eventHeaderValue.data());
        setHeaderValue(SIP_EVENT_FIELD, eventHeaderValue, 0);
    }

    // Set the SipIfMatch field
    if( sipIfMatchField && *sipIfMatchField )
    {
       setSipIfMatchField(sipIfMatchField);
    }

   // setExpires
   setExpiresField(expiresInSeconds);
}

void SipMessage::applyTargetUriHeaderParams()
{
   UtlString uriWithHeaderParams;
   getRequestUri(&uriWithHeaderParams);

   Url requestUri(uriWithHeaderParams, TRUE);

   int header;
   UtlString hdrName;
   UtlString hdrValue;
   for (header=0; requestUri.getHeaderParameter(header, hdrName, hdrValue); header++ )
   {
      // If the header is allowed in a header parameter?
      if(isUrlHeaderAllowed(hdrName.data()))
      {
         if (0 == hdrName.compareTo(SIP_ROUTE_FIELD, UtlString::ignoreCase))
         {
            /*
             * The Route header requires special handling
             *   Examine each redirected route and ensure that it is a loose route
             */
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipMessage::applyTargetUriHeaderParams found route header '%s'",
                          hdrValue.data()
                          );

            UtlString routeParams;
            int route;
            UtlString thisRoute;
            for (route=0;
                 NameValueTokenizer::getSubField(hdrValue.data(), route,
                                                 SIP_MULTIFIELD_SEPARATOR, &thisRoute);
                 thisRoute.remove(0), route++
                 )
            {
               Url newRouteUrl(thisRoute.data());
               UtlString unusedValue;
               if ( ! newRouteUrl.getUrlParameter("lr", unusedValue, 0))
               {
                  newRouteUrl.setUrlParameter("lr",NULL); // force a loose route
               }

               UtlString newRoute;
               newRouteUrl.toString(newRoute);
               if (!routeParams.isNull())
               {
                  routeParams.append(SIP_MULTIFIELD_SEPARATOR);
               }
               routeParams.append(newRoute);
            }
            // If we found any routes, push them onto the route set
            if (!routeParams.isNull())
            {
               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                             "SipMessage::applyTargetUriHeaderParams adding route(s) '%s'",
                             routeParams.data()
                             );
               addRouteUri(routeParams.data());
            }

         }
         else if (isUrlHeaderUnique(hdrName.data()))
         {
            // If the field exists, change it,
            // if does not exist, create it.
            setHeaderValue(hdrName.data(), hdrValue.data(), 0);
         }
         else
         {
            addHeaderField(hdrName.data(), hdrValue.data());
         }
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_WARNING, "URL header disallowed: %s: %s",
                       hdrName.data(), hdrValue.data());
      }
   }
   if (header)
   {
      // Remove the header fields from the URL as they
      // have been added to the message
      UtlString uriWithoutHeaderParams;
      requestUri.removeHeaderParameters();
      // Use getUri to get the addr-spec formmat, not the name-addr
      // format, because uriWithoutHeaderParams will be used as the
      // request URI of the request.
      requestUri.getUri(uriWithoutHeaderParams);

      changeRequestUri(uriWithoutHeaderParams);
   }
}


void SipMessage::addVia(const char* domainName,
                        int port,
                        const char* protocol,
                        const char* branchId,
                        const bool  bIncludeRport)
{
   UtlString viaField(SIP_PROTOCOL_VERSION);
   char portString[MAXIMUM_INTEGER_STRING_LENGTH + 1];

   viaField.append("/");
   if(protocol && strlen(protocol))
   {
      viaField.append(protocol);
   }

   // Default the protocol if not set
   else
   {
      viaField.append(SIP_TRANSPORT_TCP);
   }
   viaField.append(" ");
   viaField.append(domainName);
   if(portIsValid(port))
   {
      sprintf(portString, ":%d", port);
      viaField.append(portString);
   }

    if(branchId && *branchId)
    {
        viaField.append(';');
        viaField.append("branch");
        viaField.append('=');
        viaField.append(branchId);
    }

    if (bIncludeRport)
    {
        viaField.append(';');
        viaField.append("rport");
    }

   addViaField(viaField.data());
}

void SipMessage::addViaField(const char* viaField, UtlBoolean afterOtherVias)
{
    mHeaderCacheClean = FALSE;

   NameValuePair* nv = new NameValuePair(SIP_VIA_FIELD, viaField);
    // Look for other via fields
    ssize_t fieldIndex = mNameValues.index(nv);

    if(fieldIndex == UTL_NOT_FOUND)
    {
#       ifdef TEST_PRINT
        UtlDListIterator iterator(mNameValues);

        //remove whole line
        NameValuePair* nv = NULL;
        while((nv = dynamic_cast <NameValuePair*> (iterator())))
        {
            osPrintf("\tName: %s\n", nv->data());
        }
#       endif
    }

    mHeaderCacheClean = FALSE;

    if(fieldIndex == UTL_NOT_FOUND || !afterOtherVias)
    {
      mNameValues.insert(nv);
    }
    else
    {
        mNameValues.insertAt(fieldIndex, nv);
    }
}

void SipMessage::setTopViaTag(const char* tagValue,
                              const char* tagName)
{
   setViaTag( tagValue, tagName, 0 );
}

UtlBoolean SipMessage::setViaTag(const char* tagValue,
                                 const char* tagName,
                                 int subFieldIndex)
{
   UtlBoolean success = FALSE;
   UtlString selectedVia;
   //get last via field and remove it
   if( getViaFieldSubField(&selectedVia, subFieldIndex) )
   {
      //parse all name=value pairs into a collectable object
      UtlSList list;
      parseViaParameters(selectedVia,list);

      //create an iterator to walk the list
      UtlSListIterator iterator(list);
      NameValuePair* nvPair;
      UtlString newVia;
      UtlBoolean bFoundTag = FALSE;
      UtlString value;

      while ((nvPair = dynamic_cast <NameValuePair*> (iterator())))
      {
         value.remove(0);

         //only if we have something in our newVia string do we add a semicolon
         if (newVia.length())
         {
            newVia.append(";");
         }

         //always append the name part of the value pair
         newVia.append(nvPair->data());

         UtlString strPairName = nvPair->data();
         UtlString strTagName = tagName;

         //convert both to upper
         strPairName.toUpper();
         strTagName.toUpper();

         //if the value we are looking for is found, then we are going to replace the value with this value
         if (strTagName == strPairName)
         {
            if (tagValue)
            {
               value = tagValue;
            }
            else
            {
               //the value could come in as NULL.  In this case make it be an empty string
               value = "";
            }

            bFoundTag = TRUE;
         }
         else
         {
            //ok we didn't find the one we are looking for
            value = nvPair->getValue();
         }

         //if the value has a length then append it
         if (value.length())
         {
            newVia.append("=");
            newVia.append(value);
         }
      }

      //if we didn't find the tag we are looking for after looping, then
      //we should add the name and value pair at the end
      if (!bFoundTag)
      {
         //add a semicolon before our new name value pair is added
         newVia.append(";");
         newVia.append(tagName);

         //only if it is non-NULL and has a length do we add the equal and value
         //So, if the value is "" we will only put the name (without equal)
         if (tagValue  && strlen(tagValue))
         {
            newVia.append("=");
            newVia.append(tagValue);
         }
      }

      // at this point newVia contains the new via string with the new tag.
      success = setFieldSubfield( SIP_VIA_FIELD, subFieldIndex, newVia );
      list.destroyAll();
   }
   return success;
}

void SipMessage::setViaFromRequest(const SipMessage* request)
{
   UtlString viaSubField;
   int subFieldindex = 0;

   while(request->getViaFieldSubField(&viaSubField, subFieldindex ))
   {
#ifdef TEST
       OsSysLog::add(FAC_SIP, PRI_DEBUG,
                     "SipMessage::setViaFromRequest "
                     "Adding via field: %s\n",
                     viaSubField.data());
#endif
      addViaField(viaSubField.data(), FALSE);
      subFieldindex++;
   }
}

void SipMessage::setReceivedViaParams(const UtlString& fromIpAddress,
                                      int              fromPort
                                      )
{
   UtlString lastAddress;
   UtlString lastProtocol;
   int lastPort;

   int receivedPort;

   UtlBoolean receivedSet;
   UtlBoolean maddrSet;
   UtlBoolean receivedPortSet;

   // Check that the via is set to the address from whence
   // this message came
   getTopVia(&lastAddress, &lastPort, &lastProtocol,
             &receivedPort, &receivedSet, &maddrSet, &receivedPortSet);

   // The via address is different from that of the sockets
   UtlBoolean receivedTagAdded = false;
   if(lastAddress.compareTo(fromIpAddress) != 0)
   {
      // Add a receive from tag

      setTopViaTag(fromIpAddress.data());
      receivedTagAdded = true;
   }

   // If the rport tag is present the sender wants to
   // know what port this message was received from
   int tempLastPort = lastPort;
   if (!portIsValid(lastPort))
   {
      tempLastPort = 5060;
   }

   if (receivedPortSet || receivedTagAdded)
   {
      // The rport tag (see RFC3581) is added if the endpoint explicitly requested it or
      // if we detected that the IP address advertized in the Via does not match the IP
      // address that the request was received from.  The latter case is normally
      // indicative of the presence of a NAT in the network - given that, the rport
      // is unilaterally added to help NAT traversal.
      char portString[20];
      sprintf(portString, "%d", fromPort);
      setTopViaTag(portString, "rport");
   }
}

void SipMessage::setCallIdField(const char* callId)
{
    setHeaderValue(SIP_CALLID_FIELD, callId);
}

void SipMessage::setCSeqField(int sequenceNumber, const char* method)
{
   UtlString value;
   char numString[HTTP_LONG_INT_CHARS];

   sprintf(numString, "%d", sequenceNumber);

   value.append(numString);
   value.append(SIP_SUBFIELD_SEPARATOR);
   value.append(method);

    setHeaderValue(SIP_CSEQ_FIELD, value.data());
}

void SipMessage::incrementCSeqNumber()
{
    int seqNum;
    UtlString seqMethod;
    if(!getCSeqField(&seqNum, &seqMethod))
    {
        seqNum = 1;
        seqMethod.append("UNKNOWN");
    }
    seqNum++;
    setCSeqField(seqNum, seqMethod.data());
}

void SipMessage::buildSipUri(UtlString* url, const char* address, int port,
                      const char* protocol, const char* user,
                      const char* userLabel, const char* tag)
{
   Url result;

   // Insert the elements of the URI.
   result.setHostAddress(address);
   result.setHostPort(port);
   if (protocol)
   {
      result.setUrlParameter("transport", protocol);
   }
   result.setUserId(user);
   result.setDisplayName(userLabel);
   if (tag)
   {
      result.setFieldParameter("tag", tag);
   }

   // Force <...> even if it is not necessary, to make the result
   // more predictable.
   result.includeAngleBrackets();

   // Generate as a string.
   *url = result.toString();
}

void SipMessage::setFromField(const char* url)
{
   UtlString value;
   UtlString address;
   UtlString protocol;
   UtlString user;
   UtlString userLabel;
   int port;

   parseAddressFromUri(url, &address, &port, &protocol, &user,
      &userLabel);
   buildSipUri(&value, address.data(), port, protocol.data(),
      user.data(), userLabel.data());

   // If the field exists change it, if does not exist create it.
   setHeaderValue(SIP_FROM_FIELD, value.data(), 0);
}

void SipMessage::setFromField(const char* address, int port,
                       const char* protocol,
                       const char* user, const char* userLabel)
{
   UtlString url;
   buildSipUri(&url, address, port, protocol, user, userLabel);

   // If the field exists change it, if does not exist create it
   setHeaderValue(SIP_FROM_FIELD, url.data(), 0);
}

void SipMessage::setRawToField(const char* url)
{
   setHeaderValue(SIP_TO_FIELD, url, 0);
}

void SipMessage::setRawFromField(const char* url)
{
   // If the field exists change it, if does not exist create it
   setHeaderValue(SIP_FROM_FIELD, url, 0);
}

void SipMessage::setToField(const char* address, int port,
                       const char* protocol,
                       const char* user,
                       const char* userLabel)
{
   UtlString url;

   buildSipUri(&url, address, port, protocol, user, userLabel);

   // If the field exists change it, if does not exist create it
   setHeaderValue(SIP_TO_FIELD, url.data(), 0);
}

void SipMessage::setExpiresField(int expiresInSeconds)
{
   char secondsString[MAXIMUM_INTEGER_STRING_LENGTH];
   sprintf(secondsString, "%d", expiresInSeconds);

   // If the field exists change it, if does not exist create it
   setHeaderValue(SIP_EXPIRES_FIELD, secondsString, 0);
}

void SipMessage::setAcceptField( const char* acceptField )
{
    setHeaderValue(SIP_ACCEPT_FIELD, acceptField);
}

void SipMessage::setAcceptEncodingField( const char* acceptEncodingValue )
{
    setHeaderValue(SIP_ACCEPT_ENCODING_FIELD, acceptEncodingValue);
}

void SipMessage::setAcceptLanguageField( const char* acceptLanguageValue )
{
    setHeaderValue(SIP_ACCEPT_LANGUAGE_FIELD, acceptLanguageValue);
}

void SipMessage::setRequireField( const char* requireValue )
{
    setHeaderValue(SIP_REQUIRE_FIELD, requireValue);
}

void SipMessage::setRetryAfterField( const char* retryAfterValue )
{
    setHeaderValue(SIP_RETRY_AFTER_FIELD, retryAfterValue);
}

void SipMessage::setUnsupportedField( const char* unsupportedValue )
{
    setHeaderValue(SIP_UNSUPPORTED_FIELD, unsupportedValue);
}

void SipMessage::setMinExpiresField(int minimumExpiresInSeconds)
{
   char secondsString[MAXIMUM_INTEGER_STRING_LENGTH];
   sprintf(secondsString, "%d", minimumExpiresInSeconds);

   // If the field exists change it, if does not exist create it
   setHeaderValue(SIP_MIN_EXPIRES_FIELD, secondsString, 0);
}

void SipMessage::setContactField(const char* contactField, int index)
{
   // If the field exists change it.  If it does not exist, create it.
   setHeaderValue(SIP_CONTACT_FIELD, contactField, index);
}

void SipMessage::setRequestDispositionField(const char* dispositionField)
{
   // If the field exists change it, if does not exist create it
   setHeaderValue(SIP_REQUEST_DISPOSITION_FIELD, dispositionField, 0);
}

void SipMessage::addRequestDisposition(const char* dispositionToken)
{
    // Append to the field value already there, if it exists
    UtlString field;
    getRequestDispositionField(&field);
    if(!field.isNull())
    {
        field.append(' ');
    }

    field.append(dispositionToken);
    setRequestDispositionField(field.data());
}

void SipMessage::setWarningField(int code, const char* hostname, const char* text)
{
   UtlString warningContent;
   size_t sizeNeeded = 3 /* warning code size */ + strlen(hostname) + strlen(text) + 3 /* blanks & null */;
   size_t allocated = warningContent.capacity(sizeNeeded);

   if (allocated >= sizeNeeded)
   {
      sprintf((char*)warningContent.data(), "%3d %s \"%s\"", code, hostname, text);

      setHeaderValue(SIP_WARNING_FIELD, warningContent.data());
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_WARNING,
                    "SipMessage::setWarningField value too large (max %zu) host '%s' text '%s'",
                    allocated, hostname, text
                    );
   }
}

void SipMessage::changeUri(const char* newUri)
{
   UtlString uriString;

    // Remove the stuff that should not be in a URI
    Url cleanUri(newUri);
    cleanUri.getUri(uriString);
   changeRequestUri(uriString.data());
}

UtlBoolean SipMessage::getMaxForwards(int& maxForwards) const
{
    const char* value = getHeaderValue(0, SIP_MAX_FORWARDS_FIELD);

    if(value)
    {
        maxForwards = atoi(value);
    }
    return(value != NULL);
}


void SipMessage::setMaxForwards(int maxForwards)
{
    char buf[64];
    sprintf(buf, "%d", maxForwards);
    setHeaderValue(SIP_MAX_FORWARDS_FIELD,buf, 0);
}

void SipMessage::decrementMaxForwards()
{
    int maxForwards;
    if(!getMaxForwards(maxForwards))
    {
        maxForwards = SIP_DEFAULT_MAX_FORWARDS;
    }
    maxForwards--;
    setMaxForwards(maxForwards);
}

void SipMessage::getFromField(UtlString* field) const
{
   const char* value = getHeaderValue(0, SIP_FROM_FIELD);

   if(value)
   {
        *field = value;
   }
    else
    {
        field->remove(0);
    }
}

void SipMessage::getToField(UtlString* field) const
{
   const char* value = getHeaderValue(0, SIP_TO_FIELD);

   if(value)
   {
        *field = value;
   }
    else
    {
        field->remove(0);
    }
}

void SipMessage::getToUrl(Url& toUrl) const
{
    const char* toField = getHeaderValue(0, SIP_TO_FIELD);

    // Parse the To field as a name-addr.
    toUrl = toField ? toField : "";
}

void SipMessage::getFromUrl(Url& fromUrl) const
{
    const char* fromField = getHeaderValue(0, SIP_FROM_FIELD);

    // Parse the From field as a name-addr.
    fromUrl = fromField ? fromField : "";
}

void SipMessage::getFromLabel(UtlString* label) const
{
   UtlString field;
   ssize_t labelEnd;

   getFromField(&field);

   label->remove(0);

   if(!field.isNull())
   {
      labelEnd = field.index(" <");
      if(labelEnd >= 0)
      {
         label->append(field.data());
         label->remove(labelEnd);
      }
   }
}

void SipMessage::getToLabel(UtlString* label) const
{
   UtlString field;
   ssize_t labelEnd;

   getToField(&field);

   label->remove(0);

   if(!field.isNull())
   {
      labelEnd = field.index(" <");
      if(labelEnd >= 0)
      {
         label->append(field.data());
         label->remove(labelEnd);
      }
   }
}

UtlBoolean SipMessage::parseParameterFromUri(const char* uri,
                                            const char* parameterName,
                                            UtlString* parameterValue)
{
    UtlString parameterString(parameterName);
    UtlString uriString(uri);
    parameterString.append("=");
    // This may need to be changed to be make case insensative
    ssize_t parameterStart = uriString.index(parameterString.data());
    // 0, UtlString::ignoreCase);

    parameterValue->remove(0);

    //osPrintf("SipMessage::parseParameterFromUri uri: %s parameter: %s index: %d\n",
    //    uriString.data(), parameterString.data(), parameterStart);
    if(parameterStart >= 0)
    {
        parameterStart += parameterString.length();
        uriString.remove(0, parameterStart);
        NameValueTokenizer::frontTrim(&uriString, " \t");
        //osPrintf("SipMessage::parseParameterFromUri uriString: %s index: %d\n",
        //  uriString.data(), parameterStart);
        NameValueTokenizer::getSubField(uriString.data(), 0,
            " \t;>", parameterValue);

    }

    return(parameterStart >= 0);
}

void SipMessage::parseAddressFromUri(const char* uri,
                            UtlString* address,
                            int* port,
                            UtlString* protocol,
                            UtlString* user,
                            UtlString* userLabel,
                            UtlString* tag)
{
   // A SIP url looks like the following:
   // "user label <SIP:user@address:port ;tranport=protocol> ;tag=nnnn"
   Url parsedUri(uri);

   if (address)
   {
      parsedUri.getHostAddress(*address);
   }
   if (port)
   {
      *port = parsedUri.getHostPort();
   }
   if (protocol)
   {
      parsedUri.getUrlParameter("transport",*protocol);
   }
   if (user)
   {
      parsedUri.getUserId(*user);
   }
   if (userLabel)
   {
      parsedUri.getDisplayName(*userLabel);
   }
   if (tag)
   {
      parsedUri.getFieldParameter("tag", *tag);
   }
}

void SipMessage::setUriParameter(UtlString* uri, const char* parameterName,
                                 const char* parameterValue)
{
    UtlString parameterString(parameterName);

    //only append the '=' if not null and has a length
    if (parameterValue && *parameterValue != '\0')
        parameterString.append('=');

   ssize_t tagIndex = uri->index(parameterString.data());

   // Tag already exists, replace it
   if(tagIndex >= 0)
   {
      //osPrintf("Found tag at index: %d\n", tagIndex);
        // Minimally start after the tag name & equal sign
      tagIndex+= parameterString.length();
      ssize_t tagSpace = uri->index(' ', tagIndex);
      ssize_t tagTab = uri->index('\t', tagIndex);
      ssize_t tagSemi = uri->index(';', tagIndex);
      ssize_t tagEnd = tagSpace;
      if(tagTab >= tagIndex && (tagTab < tagEnd || tagEnd < tagIndex))
      {
         tagEnd = tagTab;
      }
      if(tagSemi >= tagIndex && (tagSemi < tagEnd || tagEnd < tagIndex))
      {
         tagEnd = tagSemi;
      }

      // Remove up to the separator
      if(tagEnd >= tagIndex)
      {
         uri->remove(tagIndex, tagEnd - tagIndex);
      }
      // Remove to the end, no separator found
      else
      {
         uri->remove(tagIndex);
      }

        //only insert the value if not null and has a length
        if (parameterValue && *parameterValue != '\0')
          uri->insert(tagIndex, parameterValue);
   }

   // Tag does not exist append it
   else
   {
      //osPrintf("Found no tag appending to the end\n");
      uri->append(";");
        uri->append(parameterString.data());

        //only add the value if not null and has a length
        if (parameterValue && *parameterValue != '\0')
         uri->append(parameterValue);
   }
}

void SipMessage::setToFieldTag(const char* tagValue)
{
   UtlString toField;
   getToField(&toField);
   //osPrintf("To field before: \"%s\"\n", toField.data());
   setUriTag(&toField, tagValue);
   //osPrintf("To field after: \"%s\"\n", toField.data());
   setRawToField(toField.data());
}

void SipMessage::setToFieldTag(int tagValue)
{
    char tagString[MAXIMUM_INTEGER_STRING_LENGTH];
    sprintf(tagString, "%d", tagValue);
    setToFieldTag(tagString);
}

void SipMessage::setFromFieldTag(const char* tagValue)
{
   UtlString fromField;
   getFromField(&fromField);
   //osPrintf("From field before: \"%s\"\n", fromField.data());
   setUriTag(&fromField, tagValue);
   //osPrintf("From field after: \"%s\"\n", fromField.data());
   setRawFromField(fromField.data());
}

void SipMessage::setUriTag(UtlString* uri, const char* tagValue)
{
    setUriParameter(uri, "tag", tagValue);
}

void SipMessage::getUri(UtlString* address,
                        int* port,
                        UtlString* protocol,
                        UtlString* user) const
{

   UtlString uriField;
   getRequestUri(&uriField);

   if( !uriField.isNull())
   {
      //Uri field will only have URL parameters . So add angle backets around the
      //whole string. else the url parameters will be trated as filed and header parameters

      Url uriUrl(uriField, TRUE); // is addrSpec

      if(address)
      {
          uriUrl.getHostAddress(*address);
      }
      if(protocol)
      {
          uriUrl.getUrlParameter("transport", *protocol);
      }
      if(port)
      {
          *port = uriUrl.getHostPort();
      }

      if(user)
      {
         uriUrl.getUserId(*user);
      }
   }
// parseAddressFromUri(uriField.data(), address, port, protocol, user);
}

void SipMessage::getFromAddress(UtlString* address, int* port, UtlString* protocol,
                        UtlString* user, UtlString* userLabel,
                        UtlString* tag) const
{
   UtlString uri;
   getFromField(&uri);

   parseAddressFromUri(uri.data(), address, port, protocol, user, userLabel,
      tag);
}

void SipMessage::getToAddress(UtlString* address, int* port, UtlString* protocol,
                       UtlString* user, UtlString* userLabel,
                       UtlString* tag) const
{
   UtlString uri;
   getToField(&uri);

   parseAddressFromUri(uri.data(), address, port, protocol, user, userLabel,
      tag);
}

void SipMessage::getFromUri(UtlString* uri) const
{
   const char* value = getHeaderValue(0, SIP_FROM_FIELD);
   if (value)
   {
      Url value_uri(value);
      value_uri.getUri(*uri);
   }
   else
   {
      // No From header, so clear the output string.
      uri->remove(0);
   }
}

UtlBoolean SipMessage::getResponseSendAddress(UtlString& address,
                                             int& port,
                                             UtlString& protocol) const
{
    int receivedPort;
    UtlBoolean receivedSet;
    UtlBoolean maddrSet;
    UtlBoolean receivedPortSet;

    // use the via as the place to send the response
    getTopVia(&address, &port, &protocol, &receivedPort,
              &receivedSet, &maddrSet, &receivedPortSet);

    // If the sender of the request indicated support of
    // rport (i.e. received port) send this response back to
    // the same port it came from
    if(receivedPortSet && portIsValid(receivedPort))
    {
        OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipMessage::getResponseSendAddress response to receivedPort %s:%d not %d\n",
            address.data(), receivedPort, port);
        port = receivedPort;
    }

    // No  via, use the from
    if(address.isNull())
    {
        OsSysLog::add(FAC_SIP, PRI_WARNING,
                      "SipMessage::getResponseSendAddress No VIA address, using From address\n");

        getFromAddress(&address, &port, &protocol);
    }

    return(!address.isNull());
}

void SipMessage::convertProtocolStringToEnum(const char* protocolString,
                                             OsSocket::IpProtocolSocketType& protocolEnum)
{
    if(strcasecmp(protocolString, SIP_TRANSPORT_UDP) == 0)
    {
        protocolEnum = OsSocket::UDP;
    }
    else if(strcasecmp(protocolString, SIP_TRANSPORT_TCP) == 0)
    {
        protocolEnum = OsSocket::TCP;
    }

    else if(strcasecmp(protocolString, SIP_TRANSPORT_TLS) == 0)
    {
        protocolEnum = OsSocket::SSL_SOCKET;
    }
    else
    {
        OsSysLog::add(FAC_SIP, PRI_ERR,
                      "SipMessage::convertProtocolStringToEnum unrecognized protocol: %s",
                      protocolString);
        protocolEnum = OsSocket::UNKNOWN;
    }

}

void SipMessage::convertProtocolEnumToString(OsSocket::IpProtocolSocketType protocolEnum,
                                            UtlString& protocolString)
{
   protocolString = OsSocket::ipProtocolString(protocolEnum);
}

void SipMessage::getToUri(UtlString* uri) const
{
   const char* value = getHeaderValue(0, SIP_TO_FIELD);
   if (value)
   {
      Url value_uri(value);
      value_uri.getUri(*uri);
   }
   else
   {
      // No To header, so clear the output string.
      uri->remove(0);
   }
}

UtlBoolean SipMessage::getWarningCode(int* warningCode, int index) const
{
   const char* value = getHeaderValue(index, SIP_WARNING_FIELD);
   UtlString warningField;
   ssize_t endOfCode;

   *warningCode = 0;

   if(value)
   {
      warningField.append(value);
      endOfCode = warningField.index(SIP_SUBFIELD_SEPARATOR);
      if(endOfCode > 0)
      {
         warningField.remove(endOfCode);
         *warningCode = atoi(warningField.data());
      }
   }
   return(value != NULL);
}

UtlBoolean SipMessage::removeTopVia()
{
   // Do not remove the whole via header line. Remove only the first subfield.
   UtlBoolean fieldFound = FALSE;
   UtlString NewViaHeader;
   UtlString viaField;

   if ( getViaField( &viaField , 0))
   {
      ssize_t posSubField = viaField.first(SIP_MULTIFIELD_SEPARATOR);
      if (posSubField != UTL_NOT_FOUND)
      {
         viaField.remove(0, posSubField + strlen(SIP_MULTIFIELD_SEPARATOR));
         NewViaHeader = viaField.strip(UtlString::both , ' ');
      }
   }

   NameValuePair viaHeaderField(SIP_VIA_FIELD);

   // Remove whole line.
   NameValuePair* nv = dynamic_cast <NameValuePair*> (mNameValues.find(&viaHeaderField));
   if(nv)
   {
      mHeaderCacheClean = FALSE;
      mNameValues.destroy(nv);
      nv = NULL;
      fieldFound = TRUE;
   }
   // Add updated line.
   if ( !NewViaHeader.isNull())
   {
      addViaField( NewViaHeader);
   }
   return(fieldFound);
}

UtlBoolean SipMessage::getViaField(UtlString* viaField, int index) const
{
   const char* value = getHeaderValue(index, SIP_VIA_FIELD);

   viaField->remove(0);
   if(value)
   {
      viaField->append(value);
   }
   return(value != NULL);
}

UtlBoolean SipMessage::getViaFieldSubField(UtlString* viaSubField, int subFieldIndex) const
{
   UtlBoolean retVal = FALSE;
   UtlString Via;
   if (getFieldSubfield(SIP_VIA_FIELD, subFieldIndex, &Via) )
   {
      viaSubField->remove(0);
      if(!Via.isNull())
      {
         viaSubField->append(Via);
         retVal = TRUE;
      }
   }
   return retVal;
}

void SipMessage::getTopVia(UtlString* address,
                           int* port,
                           UtlString* protocol,
                           int* receivedPort,
                           UtlBoolean* receivedSet,
                           UtlBoolean* maddrSet,
                           UtlBoolean* receivedPortSet) const
{
   UtlString Via;

   UtlString sipProtocol;
   UtlString url;
   UtlString receivedAddress;
   UtlString receivedPortString;
   UtlString maddr;
   ssize_t index;

   // why are these removed twice?  one time port is set to -1, later it is set to 0?
   address->remove(0);
   *port = PORT_NONE;
   protocol->remove(0);

   // Initialize caller's values as if nothing found in via
   if (port)
   {
      *port = 0;
   }
   if (address)
   {
      address->remove(0);
   }
   if (protocol)
   {
      protocol->remove(0);
   }
   if (receivedSet)
   {
      *receivedSet = FALSE;
   }
   if (maddrSet)
   {
      *maddrSet = FALSE;
   }
   if (receivedPortSet)
   {
      *receivedPortSet = FALSE;
   }

   // Get the first (most recent) Via value, which is the one that tells
   // how to send the response.
   if (getFieldSubfield(SIP_VIA_FIELD, 0, &Via))
   {
      NameValueTokenizer::getSubField(Via, 0, SIP_SUBFIELD_SEPARATORS,
                                      &sipProtocol);
      NameValueTokenizer::getSubField(Via, 1, SIP_SUBFIELD_SEPARATORS,
                                      &url);

      index = sipProtocol.index('/');
      if(index >= 0)
      {
         sipProtocol.remove(0, index + 1);
         index = sipProtocol.index('/');
      }

      if(index >= 0)
      {
         sipProtocol.remove(0, index + 1);
      }
      if (protocol)
      {
         protocol->append(sipProtocol.data());
      }

      Url viaParam(url,TRUE);
      if (address)
      {
         viaParam.getHostAddress(*address);
      }
      if (port)
      {
         *port = viaParam.getHostPort();
      }
      UtlBoolean receivedFound =
         viaParam.getUrlParameter("received", receivedAddress);
      UtlBoolean maddrFound = viaParam.getUrlParameter("maddr", maddr);
      UtlBoolean receivedPortFound =
         viaParam.getUrlParameter("rport", receivedPortString);

      // The maddr takes precedence over the received address
      if(address && !maddr.isNull())
      {
          // why isn't this remove/append like receivedAddress in "elseif"?
         *address = maddr;
      }

      // The received address takes precedence over the sent-by address
      else if(address && !receivedAddress.isNull())
      {
         address->remove(0);
         address->append(receivedAddress.data());
      }

      if(receivedPort
         && !receivedPortString.isNull())
      {
         *receivedPort = atoi(receivedPortString.data());
      }
      else if(receivedPort)
      {
         *receivedPort = PORT_NONE;
      }

      if(receivedSet)
      {
         *receivedSet = receivedFound;
      }
      if(maddrSet)
      {
         *maddrSet = maddrFound;
      }
      if(receivedPortSet)
      {
         *receivedPortSet = receivedPortFound;
      }
   }
}

UtlBoolean SipMessage::getViaTag(const char* viaField,
                                const char* tagName,
                                UtlString& tagValue)
{
    UtlString strNameValuePair;
    UtlBoolean tagFound = FALSE;
    UtlHashBag viaParameters;

    parseViaParameters(viaField,viaParameters);
    UtlString nameMatch(tagName);
    NameValuePair *pair = (NameValuePair *)viaParameters.find(&nameMatch);

    if (pair)
    {
        tagValue = pair->getValue();
        tagFound = TRUE;
    }
    else
        tagValue.remove(0);

    viaParameters.destroyAll();

    return(tagFound);
}


void SipMessage::getCallIdField(UtlString* callId) const
{
   const char* value = getHeaderValue(0, SIP_CALLID_FIELD);

   if(value)
   {
        *callId = value;
    }
    else
    {
        callId->remove(0);
    }
}

void SipMessage::getReferencesField(UtlString* references) const
{
   const char* value = getHeaderValue(0, SIP_REFERENCES_FIELD);
   references->remove(0);
   if (value) {
       references->append(value);
   }
}

void SipMessage::getDialogHandle(UtlString& dialogHandle) const
{
    getCallIdField(&dialogHandle);
    // Separator
    dialogHandle.append(',');

    Url messageFromUrl;
    getFromUrl(messageFromUrl);
    UtlString fromTag;
    messageFromUrl.getFieldParameter("tag", fromTag);
    dialogHandle.append(fromTag);
    // Separator
    dialogHandle.append(',');

    Url messageToUrl;
    getToUrl(messageToUrl);
    UtlString toTag;
    messageToUrl.getFieldParameter("tag", toTag);
    dialogHandle.append(toTag);
}

void SipMessage::getDialogHandleReverse(UtlString& dialogHandle) const
{
    getCallIdField(&dialogHandle);

    // Separator
    dialogHandle.append(',');

    Url messageToUrl;
    getToUrl(messageToUrl);
    UtlString toTag;
    messageToUrl.getFieldParameter("tag", toTag);
    dialogHandle.append(toTag);

    // Separator
    dialogHandle.append(',');

    Url messageFromUrl;
    getFromUrl(messageFromUrl);
    UtlString fromTag;
    messageFromUrl.getFieldParameter("tag", fromTag);
    dialogHandle.append(fromTag);
}

UtlBoolean SipMessage::getCSeqField(int* sequenceNum, UtlString* sequenceMethod) const
{
   const char* value = getHeaderValue(0, SIP_CSEQ_FIELD);
   if(value)
   {
        // Too slow:
       /*UtlString sequenceNumString;
      NameValueTokenizer::getSubField(value, 0,
               SIP_SUBFIELD_SEPARATORS, &sequenceNumString);
      *sequenceNum = atoi(sequenceNumString.data());

      NameValueTokenizer::getSubField(value, 1,
               SIP_SUBFIELD_SEPARATORS, sequenceMethod);*/

        // Ignore white space in the begining
        int valueStart = strspn(value, SIP_SUBFIELD_SEPARATORS);

        // Find the end of the sequence number
        int numStringLen = strcspn(&value[valueStart], SIP_SUBFIELD_SEPARATORS)
            - valueStart;

        // Get the method
        if(sequenceMethod)
        {
            *sequenceMethod = &value[numStringLen + valueStart];
            NameValueTokenizer::frontBackTrim(sequenceMethod, SIP_SUBFIELD_SEPARATORS);

            if(numStringLen > MAXIMUM_INTEGER_STRING_LENGTH)
            {
               OsSysLog::add(FAC_SIP, PRI_ERR,
                             "SipMessage::getCSeqField CSeq field '%.*s' containes %d digits, which exceeds MAXIMUM_INTEGER_STRING_LENGTH (%d).  Truncated.\n",
                             numStringLen, &value[valueStart], numStringLen,
                             MAXIMUM_INTEGER_STRING_LENGTH);
               numStringLen = MAXIMUM_INTEGER_STRING_LENGTH;
            }
        }

        if(sequenceNum)
        {
            // Convert the sequence number
            char numBuf[MAXIMUM_INTEGER_STRING_LENGTH + 1];
            memcpy(numBuf, &value[valueStart], numStringLen);
            numBuf[numStringLen] = '\0';
            *sequenceNum = atoi(numBuf);
        }
   }
    else
    {
        if(sequenceNum)
        {
            *sequenceNum = -1;
        }

        if(sequenceMethod)
        {
            sequenceMethod->remove(0);
        }
    }

    return(value != NULL);
}

UtlBoolean SipMessage::getContactUri(int addressIndex, UtlString* uri) const
{
   UtlBoolean uriFound = getContactField(addressIndex, *uri);
   if (uriFound)
   {
      // Parse as a name-addr.
      Url parsed(uri->data(), FALSE);
      // Un-parse as an addr-spec.
      parsed.getUri(*uri);
   }
   return uriFound;
}

UtlBoolean SipMessage::getContactField(int addressIndex, UtlString& contactField) const
{
    const char* value = getHeaderValue(addressIndex, SIP_CONTACT_FIELD);
    contactField = value ? value : "";

    return(value != NULL);
}


// Make sure that the getContactEntry does the right thing for


UtlBoolean SipMessage::getContactEntry(int addressIndex, UtlString* uriAndParameters) const
{
   // return(getFieldSubfield(SIP_CONTACT_FIELD, addressIndex, uriAndParameters));

   UtlBoolean contactFound = FALSE;
   int currentHeaderFieldValue = 0;
   int currentEntryValue = 0;
   const char* value = NULL;

   while ( (value = getHeaderValue(currentHeaderFieldValue, SIP_CONTACT_FIELD)) &&
      (currentEntryValue <= addressIndex) )
   {
      uriAndParameters->remove(0);
      if(value)
      {
           int addressStart = 0;
           int addressCount = 0;
           int charIndex = 0;
           int doubleQuoteCount = 0;

           while(1)
           {
               if(value[charIndex] == '"')
               {
                   doubleQuoteCount++;
               }

               // We found a comma that is not in the middle of a quoted string
               if(   (value[charIndex] == ',' || value[charIndex] == '\0')
                  && !(doubleQuoteCount % 2)
                  )
               {
                   if(currentEntryValue == addressIndex)
                   {

                       uriAndParameters->append(&value[addressStart], charIndex - addressStart);
                       currentEntryValue ++;
                       contactFound = TRUE;
                       break;
                   }
                   currentEntryValue ++;
                   addressStart = charIndex + 1;
                   addressCount++;
               }

               if(value[charIndex] == '\0') break;
               charIndex++;
           }
      }
      currentHeaderFieldValue++;
   }
   return(contactFound);
}


UtlBoolean SipMessage::getContactAddress(int addressIndex,
                                        UtlString* contactAddress,
                                        int* contactPort,
                                        UtlString* protocol,
                                    UtlString* user,
                                        UtlString* userLabel) const
{
    UtlString uri;
    UtlBoolean foundUri = getContactUri(addressIndex, &uri);

    if(foundUri) parseAddressFromUri(uri.data(), contactAddress,
                  contactPort, protocol,
                  user,
                  userLabel);

    return(foundUri);
}

UtlBoolean SipMessage::getRequireExtension(int extensionIndex,
                                UtlString* extension) const
{
   return(getFieldSubfield(SIP_REQUIRE_FIELD, extensionIndex, extension));
}

UtlBoolean SipMessage::getProxyRequireExtension(int extensionIndex,
                                                UtlString* extension) const
{
   return(getFieldSubfield(SIP_PROXY_REQUIRE_FIELD, extensionIndex, extension));
}

void SipMessage::addRequireExtension(const char* extension)
{
    addHeaderField(SIP_REQUIRE_FIELD, extension);
}

/// Retrieve the event type, id, and other params from the Event Header
UtlBoolean SipMessage::getEventField(UtlString* eventType,
                                     UtlString* eventId, //< set to the 'id' parameter value if not NULL
                                     UtlHashMap* params  //< holds parameter name/value pairs if not NULL
                                     ) const
{
   UtlString  eventField;
   UtlBoolean gotHeader = getEventField(eventField);

   if (NULL != eventId)
   {
      eventId->remove(0);
   }

   if (gotHeader)
   {
      NameValueTokenizer::getSubField(eventField, 0, ";", eventType);
      NameValueTokenizer::frontBackTrim(eventType, " \t");

      UtlString eventParam;
      for (int param_idx = 1;
           NameValueTokenizer::getSubField(eventField.data(), param_idx, ";", &eventParam);
           param_idx++
           )
      {
         UtlString name;
         UtlString value;

         NameValueTokenizer paramPair(eventParam);
         if (paramPair.getNextPair('=',&name,&value))
         {
            if (0==name.compareTo("id",UtlString::ignoreCase) && NULL != eventId)
            {
               *eventId=value;
            }
            else if (NULL != params)
            {
               UtlString* returnedName  = new UtlString(name);
               UtlString* returnedValue = new UtlString(value);
               params->insertKeyAndValue( returnedName, returnedValue );
            }
         }
         else
         {
            OsSysLog::add(FAC_SIP,PRI_WARNING,"invalid event parameter '%s'", eventParam.data());
         }
      }
   }

   return gotHeader;
}


UtlBoolean SipMessage::getEventField(UtlString& eventField) const
{
   const char* value = getHeaderValue(0, SIP_EVENT_FIELD);
    eventField.remove(0);

   if(value)
   {
      eventField.append(value);
   }

   return(value != NULL);
}

void SipMessage::setEventField(const char* eventField)
{
    setHeaderValue(SIP_EVENT_FIELD, eventField, 0);
}

UtlBoolean SipMessage::getExpiresField(int* expiresInSeconds) const
{
   const char* fieldValue = getHeaderValue(0, SIP_EXPIRES_FIELD);
   if(fieldValue)
   {
        UtlString subfieldText;
        NameValueTokenizer::getSubField(fieldValue, 1,
               " \t:;,", &subfieldText);

        //
        if(subfieldText.isNull())
        {
            *expiresInSeconds = atoi(fieldValue);
        }
        // If there is more than one token assume it is a text date
        else
        {
            long dateExpires = OsDateTime::convertHttpDateToEpoch(fieldValue);
            long dateSent = 0;
            // If the date was not set in the message
            if(!getDateField(&dateSent))
            {
#ifdef TEST
                osPrintf("Date field not set\n");
#endif

                // Assume date sent is now
                dateSent = OsDateTime::getSecsSinceEpoch();
            }
#ifdef TEST_PRINT
            osPrintf("Expires date: %ld\n", dateExpires);
            osPrintf("Current time: %ld\n", dateSent);
#endif

            *expiresInSeconds = dateExpires - dateSent;
        }
   }
   else
   {
      *expiresInSeconds = -1;
   }

   return(fieldValue != NULL);
}

UtlBoolean SipMessage::getRequestDispositionField(UtlString* dispositionField) const
{
   const char* value = getHeaderValue(0, SIP_REQUEST_DISPOSITION_FIELD);
    dispositionField->remove(0);

   if(value)
   {
      dispositionField->append(value);
   }

   return(value != NULL);
}

UtlBoolean SipMessage::getRequestDisposition(int tokenIndex,
                                UtlString* dispositionToken) const
{
   return(getFieldSubfield(SIP_REQUEST_DISPOSITION_FIELD, tokenIndex,
        dispositionToken));
}

UtlBoolean SipMessage::getRecordRouteField(int index, UtlString* recordRouteField) const
{
    const char* fieldValue = getHeaderValue(index, SIP_RECORD_ROUTE_FIELD);
    recordRouteField->remove(0);
    if(fieldValue) recordRouteField->append(fieldValue);
    return(fieldValue != NULL);
}

UtlBoolean SipMessage::getRecordRouteUri(int index, UtlString* recordRouteUri) const
{
    //UtlString recordRouteField;
    //UtlBoolean fieldExists = getRecordRouteField(&recordRouteField);
    //NameValueTokenizer::getSubField(recordRouteField.data(), index,
   //          ",", recordRouteUri);
    UtlBoolean fieldExists = getFieldSubfield(SIP_RECORD_ROUTE_FIELD, index, recordRouteUri);
    NameValueTokenizer::frontBackTrim(recordRouteUri, " \t");
    return(fieldExists && !recordRouteUri->isNull());
}

void SipMessage::setRecordRouteField(const char* recordRouteField,
                                     int index)
{
    setHeaderValue(SIP_RECORD_ROUTE_FIELD, recordRouteField, index);
}

void SipMessage::addRecordRouteUri(const char* recordRouteUri)
{
    UtlString recordRouteField;
    UtlString recordRouteUriString;

    if(recordRouteUri && strchr(recordRouteUri, '<') == NULL)
    {
        recordRouteUriString.append('<');
        recordRouteUriString.append(recordRouteUri);
        recordRouteUriString.append('>');
    }
    else if(recordRouteUri)
    {
        recordRouteUriString.append(recordRouteUri);
    }

    recordRouteField.insert(0, recordRouteUriString);

    // Record route is always added on the top
    NameValuePair* rrHeader =
       new NameValuePair(SIP_RECORD_ROUTE_FIELD, recordRouteUriString.data());

    mHeaderCacheClean = FALSE;

    ssize_t firstRR = mNameValues.index(rrHeader);
    mNameValues.insertAt(UTL_NOT_FOUND == firstRR ? 0 : firstRR, rrHeader);
}

// isClientMsgStrictRouted returns whether or not a message
//    is set up such that it requires strict routing.
//    This is appropriate only when acting as a UAC
UtlBoolean SipMessage::isClientMsgStrictRouted() const
{
    UtlBoolean result;
    UtlString routeField;

    if ( getRouteField( &routeField ) )
    {
       Url routeUrl(routeField,Url::NameAddr);
       UtlString valueIgnored;

       // there is a route header, so see if it is loose routed or not
       result = (   (   Url::SipUrlScheme == routeUrl.getScheme()
                     || Url::SipsUrlScheme == routeUrl.getScheme())
                 && !routeUrl.getUrlParameter( "lr", valueIgnored )
                 );
    }
    else
    {
       // no route field value
       result = FALSE;
    }

    return result;
}


UtlBoolean SipMessage::getRouteField(UtlString* routeField) const
{
    const char* fieldValue = getHeaderValue(0, SIP_ROUTE_FIELD);
    routeField->remove(0);
    if(fieldValue) routeField->append(fieldValue);
    return(fieldValue != NULL);
}

void SipMessage::addRouteUri(const char* routeUri)
{
   UtlString routeField;
   UtlString routeParameter;
   const char* bracketPtr = strchr(routeUri, '<');
   if(bracketPtr == NULL)
   {
      routeParameter.append('<');
   }
   routeParameter.append(routeUri);
   bracketPtr = strchr(routeUri, '>');
   if(bracketPtr == NULL)
   {
      routeParameter.append('>');
   }
   // If there is already a route header
   if(getRouteField( &routeField))
   {
      routeParameter.append(SIP_MULTIFIELD_SEPARATOR);

      // Remove the entire header
      removeHeader(SIP_ROUTE_FIELD, 0);
   }

   //add the updated header
   routeField.insert(0,routeParameter);
   insertHeaderField(SIP_ROUTE_FIELD, routeField.data(), 0);
}

void SipMessage::addLastRouteUri(const char* routeUri)
{
    if(routeUri && *routeUri)
    {
        // THis is the cheap brut force way to do this
        int index = 0;
        const char* routeField = NULL;
        while ((routeField = getHeaderValue(index, SIP_ROUTE_FIELD)))
        {
            index++;
        }

        UtlString routeString(routeField ? routeField : "");
        if(routeField)
        {
            // Add a field separator
            routeString.append(SIP_MULTIFIELD_SEPARATOR);
        }
        // Make sure the route is in name-addr format
        if(strstr(routeUri,"<") <= 0)
        {
            routeString.append("<");
        }

        routeString.append(routeUri);
        if(strstr(routeUri, ">") <= 0)
        {
            routeString.append(">");
        }

        setHeaderValue(SIP_ROUTE_FIELD, routeString.data(), index);
    }
}

UtlBoolean SipMessage::getLastRouteUri(UtlString& routeUri,
                                      int& lastIndex)
{
    int index = 0;

    UtlString tempRoute;
    while(getFieldSubfield(SIP_ROUTE_FIELD, index, &tempRoute))
    {
        index++;
        routeUri = tempRoute;
    }

    index--;
    lastIndex = index;

    return(!routeUri.isNull());
}

UtlBoolean SipMessage::getRouteUri(int index, UtlString* routeUri) const
{
    UtlString routeField;
    UtlBoolean fieldExists = getFieldSubfield(SIP_ROUTE_FIELD, index, routeUri);
    NameValueTokenizer::frontBackTrim(routeUri, " \t");
    return(fieldExists && !routeUri->isNull());
}

UtlBoolean SipMessage::removeRouteUri(int index, UtlString* routeUri)
{
    UtlString newRouteField;
    UtlString aRouteUri;
    UtlBoolean uriFound = FALSE;
    int uriIndex = 0;
    while(getFieldSubfield(SIP_ROUTE_FIELD, uriIndex, &aRouteUri))
    {
#ifdef TEST_PRINT
        osPrintf("removeRouteUri::routeUri[%d]: %s\n", uriIndex, newRouteField.data());
#endif
        if(uriIndex == index)
        {
            *routeUri = aRouteUri;
            uriFound = TRUE;
        }
        else
        {
            if(!newRouteField.isNull())
            {
                newRouteField.append(',');
            }
            ssize_t routeUriIndex = aRouteUri.index('<');
            if(routeUriIndex < 0)
            {
                aRouteUri.insert(0, '<');
                aRouteUri.append('>');
            }
            newRouteField.append(aRouteUri.data());
        }
#ifdef TEST_PRINT
        osPrintf("removeRouteUri::newRouteField: %s\n", newRouteField.data());
#endif
        uriIndex++;
    }

    // Remove all the old route headers.
    while(removeHeader(SIP_ROUTE_FIELD, 0))
    {
    }

    // Set the route field to contain the uri list with the indicated
    // uri removed.
    if(!newRouteField.isNull())
    {
        insertHeaderField(SIP_ROUTE_FIELD, newRouteField.data());
    }

    return(uriFound);
}

void SipMessage::setRouteField(const char* routeField)
{
#ifdef TEST_PRINT
    osPrintf("setRouteField: %s\n", routeField);
#endif
    setHeaderValue(SIP_ROUTE_FIELD, routeField, 0);
}

UtlBoolean SipMessage::buildRouteField(UtlString* routeFld) const
{
    UtlBoolean recordRouteFound = FALSE;
    UtlString contactUri;
   UtlString routeField;

    // If request, build from recordRoute verbatum
    if(!isResponse())
    {
#ifdef TEST_PRINT
        osPrintf("SipMessage::buildRouteField recordRoute verbatum\n");
#endif
        //recordRouteFound = getRecordRouteField(routeField);
        int recordRouteIndex = 0;
        routeField.remove(0);
        UtlString recordRouteUri;
        while(getRecordRouteUri(recordRouteIndex, &recordRouteUri))
        {
            if(!routeField.isNull())
            {
                routeField.append(',');
            }
            routeField.append(recordRouteUri.data());
#ifdef TEST_PRINT
            osPrintf("SipMessage::buildRouteField recordRouteUri[%d] %s\n",
                recordRouteIndex, recordRouteUri.data());
#endif
            recordRouteIndex++;
        }
        if(recordRouteIndex) recordRouteFound = TRUE;
    }

    // If response, build from recordeRoute in reverse order
    else
    {
#ifdef TEST_PRINT
        osPrintf("SipMessage::buildRouteField recordRoute reverse\n");
#endif
        UtlString recordRouteUri;
        routeField.remove(0);
        int index = 0;
        ssize_t recordRouteUriIndex;
        while(getRecordRouteUri(index, &recordRouteUri))
        {
            recordRouteFound = TRUE;
            if(index > 0)
            {
                routeField.insert(0, ", ");
            }

            recordRouteUriIndex = recordRouteUri.index('<');
            if(recordRouteUriIndex < 0)
            {
                recordRouteUri.insert(0, '<');
                recordRouteUri.append('>');
            }

#ifdef TEST_PRINT
            osPrintf("SipMessage::buildRouteField recordRouteUri[%d] %s\n",
                index, recordRouteUri.data());
#endif
            routeField.insert(0, recordRouteUri.data());
            index++;
        }
    }

#ifdef LOOSE_ROUTE
    // In either case if contact is present add to the end of the route list.
    if(recordRouteFound && getContactUri(0, &contactUri))
    {
        routeField.append(", ");
        ssize_t contactUriIndex = contactUri.index('<');
        if(contactUriIndex < 0)
        {
            contactUri.insert(0, '<');
            contactUri.append('>');
        }
        routeField.append(contactUri.data());
    }
#endif

#ifdef TEST_PRINT
    osPrintf("buildRouteField: %s\n", routeField.data());
#endif

   if (recordRouteFound)
   {
      //clear the previous recourd route field and set it to the new one
      routeFld->remove(0);
      routeFld->append(routeField);
   }
    return(recordRouteFound);
}

void SipMessage::setSipXNatRoute(const char* routeUri)
{
   setHeaderValue(SIP_SIPX_NAT_ROUTE_FIELD, routeUri );
}

bool SipMessage::getSipXNatRoute(UtlString* uriString)
{
   bool result = false;
   uriString->remove( 0 );
   const char *pUri = getHeaderValue( 0, SIP_SIPX_NAT_ROUTE_FIELD );
   if( pUri )
   {
      *uriString = pUri;
      result = true;
   }
   return result;
}

void SipMessage::removeSipXNatRoute( void )
{
   removeHeader( SIP_SIPX_NAT_ROUTE_FIELD, 0 );
}

UtlBoolean SipMessage::getPathUri(int index, UtlString* pathUri) const
{
   UtlString pathField;
   UtlBoolean fieldExists = getFieldSubfield(SIP_PATH_FIELD, index, pathUri);
   NameValueTokenizer::frontBackTrim(pathUri, " \t");
   return(fieldExists && !pathUri->isNull());
}

void SipMessage::addPathUri(const char* pathUri)
{
    UtlString pathField;
    UtlString pathUriString;

    if(pathUri && *pathUri)
    {
       if(strchr(pathUri, '<') == NULL)
       {
          pathUriString.append('<');
          pathUriString.append(pathUri);
          pathUriString.append('>');
       }
       else
       {
          pathUriString.append(pathUri);
       }

       const char* fieldValue = getHeaderValue(0, SIP_PATH_FIELD);
       if(fieldValue)
       {
          pathUriString.append(SIP_MULTIFIELD_SEPARATOR);
          pathUriString.append(fieldValue);
          removeHeader(SIP_PATH_FIELD, 0);       // Remove the entire header
       }
       //add the updated header
       insertHeaderField(SIP_PATH_FIELD, pathUriString.data(), 0);
    }
}

void SipMessage::addLastPathUri(const char* pathUri)
{
    if(pathUri && *pathUri)
    {
        int index = 0;
        const char* pathField = NULL;
        while ((pathField = getHeaderValue(index, SIP_PATH_FIELD)))
        {
            index++;
        }

        UtlString pathString(pathField ? pathField : "");
        if(pathField)
        {
            // Add a field separator
           pathString.append(SIP_MULTIFIELD_SEPARATOR);
        }
        // Make sure the route is in name-addr format
        if(strstr(pathUri,"<") <= 0)
        {
           pathString.append("<");
        }

        pathString.append(pathUri);
        if(strstr(pathUri, ">") <= 0)
        {
           pathString.append(">");
        }

        setHeaderValue(SIP_PATH_FIELD, pathString.data(), index);
    }
}


void SipMessage::buildReplacesField(UtlString& replacesField,
                                    const char* callId,
                                    const char* fromField,
                                    const char* toField)
{
    replacesField = callId;

    replacesField.append(";to-tag=");
    Url toUrl(toField);
    UtlString toTag;
    toUrl.getFieldParameter("tag", toTag);
    replacesField.append(toTag);

    replacesField.append(";from-tag=");
    Url fromUrl(fromField);
    UtlString fromTag;
    fromUrl.getFieldParameter("tag", fromTag);
    replacesField.append(fromTag);
}

UtlBoolean SipMessage::getFieldSubfield(const char* fieldName, int addressIndex, UtlString* uri) const
{
   UtlBoolean uriFound = FALSE;
   UtlString url, urlBackup;
   int fieldIndex = 0;
   ssize_t subFieldIndex = 0;
   int index = 0;
   const char* value = getHeaderValue(fieldIndex, fieldName);

   uri->remove(0);

   while(value && index <= addressIndex)
   {
      subFieldIndex = 0;
      NameValueTokenizer::getSubField(value, subFieldIndex, SIP_MULTIFIELD_SEPARATOR, &url);
#ifdef TEST
      osPrintf("Got field: \"%s\" subfield[%d]: %s\n", fieldName, fieldIndex, url.data());
#endif

      while(!url.isNull() && index < addressIndex)
      {
         urlBackup = url;
         subFieldIndex++;
         index++;
         NameValueTokenizer::getSubField(value, subFieldIndex,
                                         SIP_MULTIFIELD_SEPARATOR, &url);
#ifdef TEST
         osPrintf("Got field: \"%s\" subfield[%d]: %s\n", fieldName, fieldIndex, url.data());
#endif
      }

      if(index == addressIndex && !url.isNull())
      {
         uri->append(url.data());
         uriFound = TRUE;
            break;
      }
      else if(index > addressIndex)
      {
         break;
      }

      fieldIndex++;
      value = getHeaderValue(fieldIndex, fieldName);
   }

   // If we were looking for the last subfield, set 'uri' to return
   // the last one we found.
   if( addressIndex == BOTTOM_SUBFIELD && !urlBackup.isNull() )
   {
      uri->append( urlBackup.data() );
      uriFound = TRUE;
   }

   return(uriFound);
}

UtlBoolean SipMessage::setFieldSubfield(const char* fieldName,
	   				int addressIndex,
					const UtlString& newSubfieldValue)
{
   UtlBoolean bfieldToModifyFound = FALSE;
   UtlString url;
   UtlString modifiedField;
   int index = 0;
   int fieldIndex = 0;
   const char* field = getHeaderValue(fieldIndex, fieldName);

   while( !bfieldToModifyFound && field )
   {
      ssize_t subFieldIndex = 0;
      modifiedField.remove(0);
      while( NameValueTokenizer::getSubField( field, subFieldIndex, SIP_MULTIFIELD_SEPARATOR, &url ) && !url.isNull() )
      {
         if( subFieldIndex != 0 )
         {
            modifiedField.append( SIP_MULTIFIELD_SEPARATOR );
         }

         if( index == addressIndex )
         {
            // we found the subfield we need to modify - append the new subfield value
            modifiedField.append( newSubfieldValue );
            bfieldToModifyFound = TRUE;
         }
         else
         {
            modifiedField.append( url );
         }
         subFieldIndex++;
         index++;
      }

      if( bfieldToModifyFound )
      {
         setHeaderValue( fieldName, modifiedField, fieldIndex );
      }
      else
      {
         fieldIndex++;
         field = getHeaderValue(fieldIndex, fieldName);
      }
   }
   return( bfieldToModifyFound );
}

UtlBoolean SipMessage::getContentEncodingField(UtlString* contentEncodingField) const
{
   const char* value = getHeaderValue(0, SIP_CONTENT_ENCODING_FIELD);

   contentEncodingField->remove(0);
   if(value)
   {
      contentEncodingField->append(value);
   }
    return(value != NULL);
}

UtlBoolean SipMessage::getSessionExpires(int* sessionExpiresSeconds) const
{
    const char* value = getHeaderValue(0, SIP_SESSION_EXPIRES_FIELD);

   if(value)
   {
      *sessionExpiresSeconds = atoi(value);
   }
    else
    {
        *sessionExpiresSeconds = 0;
    }
    return(value != NULL);
}

bool SipMessage::hasSelfHeader() const
{
   UtlString value;
   getUserAgentField(&value);
   if (value.isNull())
   {
      getServerField(&value);
   }
   return ! value.isNull();
}

void SipMessage::getServerField(UtlString* serverFieldValue) const
{
   const char* server = getHeaderValue(0, SIP_SERVER_FIELD);
   serverFieldValue->remove(0);
   if(server)
   {
      serverFieldValue->append(server);
   }
}

void SipMessage::setServerField(const char* serverField)
{
   setHeaderValue(SIP_SERVER_FIELD, serverField);
}


void SipMessage::setSessionExpires(int sessionExpiresSeconds)
{
   char numString[HTTP_LONG_INT_CHARS];

   sprintf(numString, "%d", sessionExpiresSeconds);
    setHeaderValue(SIP_SESSION_EXPIRES_FIELD, numString);
}

UtlBoolean SipMessage::getSupportedField(UtlString& supportedField) const
{
    return(getFieldSubfield(SIP_SUPPORTED_FIELD, 0, &supportedField));
}

void SipMessage::setSupportedField(const char* supportedField)
{
    setHeaderValue(SIP_SUPPORTED_FIELD, supportedField);
}

UtlBoolean SipMessage::isInSupportedField(const char* token) const
{
   return isInSpecifiedHeaderField(token, SIP_SUPPORTED_FIELD);
}

UtlBoolean SipMessage::getRequireField(UtlString& requireField) const
{
    return(getFieldSubfield(SIP_REQUIRE_FIELD, 0, &requireField));
}

UtlBoolean SipMessage::isInRequireField(const char* token) const
{
   return isInSpecifiedHeaderField(token, SIP_REQUIRE_FIELD);
}

UtlBoolean SipMessage::isInSpecifiedHeaderField(const char* token, const char* header) const
{
   UtlBoolean tokenFound = FALSE;
   UtlString url;
   int fieldIndex = 0;
   int subFieldIndex = 0;
   const char* value = getHeaderValue(fieldIndex, header);

   while (value && !tokenFound)
   {
      subFieldIndex = 0;
      NameValueTokenizer::getSubField(value, subFieldIndex,
                                      SIP_MULTIFIELD_SEPARATOR, &url);
#ifdef TEST
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "Got field: \"%s\" subfield[%d]: %s\n", value,
               fieldIndex, url.data());
#endif
      url.strip(UtlString::both);
      if (url.compareTo(token, UtlString::ignoreCase) == 0)
      {
         tokenFound = TRUE;
      }

      while (!url.isNull() && !tokenFound)
      {
         subFieldIndex++;
         NameValueTokenizer::getSubField(value, subFieldIndex,
                                         SIP_MULTIFIELD_SEPARATOR, &url);
         url.strip(UtlString::both);
#ifdef TEST
         OsSysLog::add(FAC_SIP, PRI_DEBUG, "Got field: \"%s\" subfield[%d]: %s\n", SIP_SUPPORTED_FIELD,
                  fieldIndex, url.data());
#endif

         if (url.compareTo(token, UtlString::ignoreCase) == 0)
         {
            tokenFound = TRUE;
         }
      }

      fieldIndex++;
      value = getHeaderValue(fieldIndex, SIP_SUPPORTED_FIELD);
   }

   return(tokenFound);
}

// Call control accessors
UtlBoolean SipMessage::getAlsoUri(int index, UtlString* alsoUri) const
{
    return(getFieldSubfield(SIP_ALSO_FIELD, index, alsoUri));
}

UtlBoolean SipMessage::getAlsoField(UtlString* alsoField) const
{
    const char* value = getHeaderValue(0, SIP_ALSO_FIELD);
    alsoField->remove(0);
    if(value) alsoField->append(value);
    return(value != NULL);
}

void SipMessage::setAlsoField(const char* alsoField)
{
    setHeaderValue(SIP_ALSO_FIELD, alsoField);
}

void SipMessage::addAlsoUri(const char* alsoUri)
{
    // Append to the field value already there, if it exists
    UtlString field;
    if(getAlsoField(&field) && !field.isNull())
    {
        field.append(SIP_MULTIFIELD_SEPARATOR);
      field.append(SIP_SINGLE_SPACE);
    }

    if(!strchr(alsoUri, '<')) field.append('<');
    field.append(alsoUri);
    if(!strchr(alsoUri, '>')) field.append('>');
    setAlsoField(field.data());
}

void SipMessage::setRequestedByField(const char* requestedByUri)
{
    setHeaderValue(SIP_REQUESTED_BY_FIELD, requestedByUri);
}

UtlBoolean SipMessage::getRequestedByField(UtlString& requestedByField) const
{
    const char* value = getHeaderValue(0, SIP_REQUESTED_BY_FIELD);
    requestedByField.remove(0);
    if(value) requestedByField.append(value);
    return(value != NULL);
}

void SipMessage::setReferToField(const char* referToField)
{
    setHeaderValue(SIP_REFER_TO_FIELD, referToField);
}

UtlBoolean SipMessage::getReferToField(UtlString& referToField) const
{
    const char* value = getHeaderValue(0, SIP_REFER_TO_FIELD);
    referToField.remove(0);
    if(value) referToField.append(value);
    return(value != NULL);
}

void SipMessage::setReferredByField(const char* referredByField)
{
    setHeaderValue(SIP_REFERRED_BY_FIELD, referredByField);
}

UtlBoolean SipMessage::getReferredByField(UtlString& referredByField) const
{
    const char* value = getHeaderValue(0, SIP_REFERRED_BY_FIELD);
    referredByField.remove(0);
    if(value) referredByField.append(value);
    return(value != NULL);
}

UtlBoolean SipMessage::getReferredByUrls(UtlString* referrerUrl,
                                        UtlString* referredToUrl) const
{
    if(referrerUrl) referrerUrl->remove(0);
    if(referredToUrl) referredToUrl->remove(0);
    const char* value = getHeaderValue(0, SIP_REFERRED_BY_FIELD);
    if(value)
    {
        // The first element is the referrer URL
        if(referrerUrl) NameValueTokenizer::getSubField(value, 0,
            ";", referrerUrl);

        // The second element is the referred to URL
        if(referredToUrl) NameValueTokenizer::getSubField(value, 1,
            ";", referredToUrl);
    }
    return(value != NULL);
}

UtlBoolean SipMessage::getReplacesData(UtlString& callId,
                                      UtlString& toTag,
                                      UtlString& fromTag) const
{
    callId.remove(0);
    toTag.remove(0);
    fromTag.remove(0);

    const char* replacesField = getHeaderValue(0, SIP_REPLACES_FIELD);

    UtlString parameter;
    UtlString name;
    UtlString value("");
    int parameterIndex = 1;

   if (replacesField)
    {
        // Get the callId
       NameValueTokenizer::getSubField(replacesField, 0,
                   ";", &callId);
       NameValueTokenizer::frontBackTrim(&callId, " \t");

       // Look through the rest of the parameters
       do
       {
          // Get a name value pair
          NameValueTokenizer::getSubField(replacesField, parameterIndex,
                   ";", &parameter);


          // Parse out the parameter name
          NameValueTokenizer::getSubField(parameter.data(), 0,
                                  "=", &name);
          name.toLower();
          NameValueTokenizer::frontBackTrim(&name, " \t");

          // Parse out the parameter value
          NameValueTokenizer::getSubField(parameter.data(), 1,
                                  "=", &value);
          NameValueTokenizer::frontBackTrim(&value, " \t");

          // Set the to and from tags when we find them
          if(name.compareTo("to-tag") == 0)
          {
             toTag = value;
          }
          else if(name.compareTo("from-tag") == 0)
          {
             fromTag = value;
          }

          parameterIndex++;
       } while(!parameter.isNull());
    }

    // return true only if all required parameters were found
    UtlBoolean result;
    if (!callId.isNull() && !toTag.isNull() && !fromTag.isNull())
    {
       result = TRUE;
    }
    else
    {
       result = FALSE;

       // make sure no data is returned with a bad result
       callId.remove(0);
       toTag.remove(0);
       fromTag.remove(0);
    }

    return result;
}

void SipMessage::setAllowField(const char* allowField)
{
    setHeaderValue(SIP_ALLOW_FIELD, allowField);
}

UtlBoolean SipMessage::getAllowField(UtlString& allowField) const
{
    const char* value;
    int allowIndex = 0;
    allowField.remove(0);
    while ((value = getHeaderValue(allowIndex, SIP_ALLOW_FIELD)))
    {
        if(value && *value)
        {
            if(!allowField.isNull()) allowField.append(", ");
            allowField.append(value);
        }
        allowIndex++;
    }
    return(value != NULL);
}

void SipMessage::setAllowEventsField(const char* allowField)
{
    setHeaderValue(SIP_ALLOW_EVENTS_FIELD, allowField);
}

UtlBoolean SipMessage::getAllowEventsField(UtlString& allowField) const
{
    const char* value;
    int allowIndex = 0;
    allowField.remove(0);
    while ((value = getHeaderValue(allowIndex, SIP_ALLOW_EVENTS_FIELD)))
    {
        if(value && *value)
        {
            if(!allowField.isNull())
            {
               allowField.append(", ");
            }
            allowField.append(value);
        }
        allowIndex++;
    }
    return(!allowField.isNull());
}

/* ============================ INQUIRY =================================== */

UtlBoolean SipMessage::isResponse() const
{
   UtlBoolean responseType = FALSE;
   //UtlString firstHeaderField;

   //getFirstHeaderLinePart(0, &firstHeaderField);
   if(mFirstHeaderLine.index(SIP_PROTOCOL_VERSION) == 0)
   {
      responseType = TRUE;
   }

   return(responseType);
}

UtlBoolean SipMessage::isServerTransaction(UtlBoolean isOutgoing) const
{
    UtlBoolean returnCode;

    if(isResponse())
    {
        if(isOutgoing)
        {
            returnCode = TRUE;
        }
        else
        {
            returnCode = FALSE;
        }
    }
    else
    {
        if(isOutgoing)
        {
            returnCode = FALSE;
        }
        else
        {
            returnCode = TRUE;
        }
    }

    return(returnCode);
}

UtlBoolean SipMessage::isSameMessage(const SipMessage* message,
                           UtlBoolean responseCodesMustMatch) const
{
   UtlBoolean isSame = FALSE;
   UtlString thisMethod, thatMethod;
   int thisSequenceNum, thatSequenceNum;
   UtlString thisSequenceMethod, thatSequenceMethod;

   if(message)
   {
      // Compare the method, To, From, CallId, Sequence number and
      // sequence method
      UtlBoolean thatIsResponse = message->isResponse();
      UtlBoolean thisIsResponse = isResponse();
      int thisResponseCode = 38743;
      int thatResponseCode = 49276;

      // Both are responses or requests
      if(thatIsResponse == thisIsResponse)
      {
         if(!thisIsResponse)
         {
            getRequestMethod(&thisMethod);
            message->getRequestMethod(&thatMethod);
         }
         else
         {
            thisResponseCode = getResponseStatusCode();
            thatResponseCode = message->getResponseStatusCode();
         }
         if( (thisIsResponse && !responseCodesMustMatch) ||
            (thisIsResponse && responseCodesMustMatch &&
               thisResponseCode == thatResponseCode) ||
            (!thisIsResponse && thisMethod.compareTo(thatMethod) == 0))
         {
            if(isSameSession(message))
            {
               getCSeqField(&thisSequenceNum, &thisSequenceMethod);
               message->getCSeqField(&thatSequenceNum, &thatSequenceMethod);
               if(thisSequenceNum == thatSequenceNum &&
                  thisSequenceMethod.compareTo(thatSequenceMethod) == 0)
               {
                  isSame = TRUE;
               }

            }
         }
      }
   }

   return(isSame);
}

UtlBoolean SipMessage::isSameSession(const SipMessage* message) const
{
   UtlBoolean isSame = FALSE;
   UtlString localCallId;
   UtlString otherCallId;

   // Messages from the same session have the same To-tags, From-tags and CallId
   if(message)
   {
      getCallIdField(&localCallId);
      message->getCallIdField(&otherCallId);
      if(0 == localCallId.compareTo(otherCallId))
      {
         Url otherFromUrl;
         Url localFromUrl;
         Url otherToUrl;
         Url localToUrl;

         message->getFromUrl(otherFromUrl);
         getFromUrl(localFromUrl);
         message->getToUrl(otherToUrl);
         getToUrl(localToUrl);

         if (isSameSession(localFromUrl, otherFromUrl, FALSE) &&
             isSameSession(localToUrl, otherToUrl, TRUE))
         {
            isSame = TRUE;
         }
      }
   }
   return(isSame);
}

UtlBoolean SipMessage::isSameSession(const Url& firstUrl,
                                     const Url& secondUrl,
                                     UtlBoolean comparingToUrl)
{
   UtlBoolean isSame = FALSE;

   UtlString firstTag;
   UtlString secondTag;

   firstUrl.getFieldParameter("tag", firstTag);
   secondUrl.getFieldParameter("tag", secondTag);

   if(comparingToUrl)
   {
      if (firstTag.isNull() ||
          secondTag.isNull() ||
          0 == firstTag.compareTo(secondTag))
      {
         isSame = TRUE;
      }
   }
   else
   {
      if(firstTag.isNull() && secondTag.isNull())
      {
         // If the tags are null default back to RFC 2543 comparison
         // This will maintain some backwards compatibility with RFC 2543

         UtlString firstAddress;
         UtlString secondAddress;
         UtlString firstProtocol;
         UtlString secondProtocol;
         UtlString firstUser;
         UtlString secondUser;
         int firstPort;
         int secondPort;

         firstUrl.getHostAddress(firstAddress);
         secondUrl.getHostAddress(secondAddress);
         firstPort = firstUrl.getHostPort();
         secondPort = secondUrl.getHostPort();
         firstUrl.getUserId(firstUser);
         secondUrl.getUserId(secondUser);
         firstUrl.getUrlParameter("transport", firstProtocol);
         secondUrl.getUrlParameter("transport", secondProtocol);

         if (firstAddress.compareTo(secondAddress) == 0 &&
               (firstPort == secondPort ||
                (firstPort == PORT_NONE && secondPort == SIP_PORT) ||
                (firstPort == SIP_PORT && secondPort == PORT_NONE)) &&
             firstProtocol.compareTo(secondProtocol) == 0 &&
             firstUser.compareTo(secondUser) == 0)
         {
            isSame = TRUE;
         }
      }
      else if (0 == firstTag.compareTo(secondTag))
      {
         isSame = TRUE;
      }
   }

   return (isSame);
}

UtlBoolean SipMessage::isResponseTo(const SipMessage* request) const
{
   UtlBoolean isPair = FALSE;
   UtlString thisMethod, thatMethod;
   int thisSequenceNum, thatSequenceNum;
   UtlString thisSequenceMethod, thatSequenceMethod;

   // If this is a response and request is a request
   if(request && !request->isResponse() && isResponse())
   {
      // Compare the To, From, CallId, Sequence number and
      // sequence method
      if(isSameSession(request))
      {
         getCSeqField(&thisSequenceNum, &thisSequenceMethod);
         request->getCSeqField(&thatSequenceNum, &thatSequenceMethod);
         if(thisSequenceNum == thatSequenceNum &&
            thisSequenceMethod.compareTo(thatSequenceMethod) == 0)
         {
            isPair = TRUE;
         }

      }

   }

   return(isPair);
}

UtlBoolean SipMessage::isAckFor(const SipMessage* inviteResponse) const
{
   UtlBoolean isPair = FALSE;
   UtlString thisMethod;
   int thisSequenceNum, thatSequenceNum;
   UtlString thisSequenceMethod, thatSequenceMethod;

   // If this is an ACK request and that is an INVITE response
   if(inviteResponse && inviteResponse->isResponse() && !isResponse())
   {
      getRequestMethod(&thisMethod);
      // Compare the To, From, CallId, Sequence number and  sequence method
      if(thisMethod.compareTo(SIP_ACK_METHOD) == 0 && isSameSession(inviteResponse))
      {
         getCSeqField(&thisSequenceNum, &thisSequenceMethod);
         inviteResponse->getCSeqField(&thatSequenceNum, &thatSequenceMethod);
         if(thisSequenceNum == thatSequenceNum &&
            thatSequenceMethod.compareTo(SIP_INVITE_METHOD) == 0)
         {
            isPair = TRUE;
         }
      }
   }

   return(isPair);
}
//SDUA
UtlBoolean SipMessage::isInviteFor(const SipMessage* cancelRequest) const
{
   UtlBoolean isPair = FALSE;
   UtlString thisMethod;
   // If this is an CANCEL request and that is an INVITE response
   if(cancelRequest && !isResponse())
   {
      getRequestMethod(&thisMethod);
      // Compare the To, From, CallId, Sequence number and  sequence method
      if(thisMethod.compareTo( SIP_INVITE_METHOD) == 0 && isSameTransaction(cancelRequest))
         isPair = TRUE;
   }
   return(isPair);
}

UtlBoolean SipMessage::isSameTransaction(const SipMessage* message) const
{
   // Compare the To, From, CallId, Sequence number and  sequence method
   UtlBoolean isPair = FALSE;
   int thisSequenceNum, thatSequenceNum;
   UtlString thisSequenceMethod, thatSequenceMethod;

   if( isSameSession(message))
   {
      getCSeqField(&thisSequenceNum, &thisSequenceMethod);
      message->getCSeqField(&thatSequenceNum, &thatSequenceMethod);
      if(thisSequenceNum == thatSequenceNum )
      {
         isPair = TRUE;
      }
   }
   return(isPair);
}


UtlBoolean SipMessage::isRequestDispositionSet(const char* dispositionToken) const
{
    UtlString field;
    int tokenIndex = 0;
    UtlBoolean matchFound = FALSE;
    while(getRequestDisposition(tokenIndex, &field))
    {
        field.toUpper();
        if(field.compareTo(dispositionToken) == 0)
        {
            matchFound = TRUE;
            break;
        }
    }

    return(matchFound);
}

UtlBoolean SipMessage::isRequireExtensionSet(const char* extension) const
{
    UtlString extensionString;
    UtlBoolean alreadySet = FALSE;
    int extensionIndex = 0;
    while(getRequireExtension(extensionIndex, &extensionString))
    {
        extensionString.toLower();
        if(extensionString.compareTo(extension) == 0)
        {
            alreadySet = TRUE;
        }

    }
    return(alreadySet);
}

UtlBoolean SipMessage::isRecordRouteAccepted( void ) const
{
   UtlBoolean isRecordRoutable;

   if( isResponse() )
   {
      isRecordRoutable = FALSE;
   }
   else
   {
      // We are dealing with a request, check if it can
      // accept a Record-Route header.  If the request
      // is not REGISTER, MESSAGE or PUBLISH, the request
      // is assumed to accept Record-Route headers.
      UtlString method;
      getRequestMethod(&method);

      if (method.compareTo(SIP_MESSAGE_METHOD)  == 0 ||
          method.compareTo(SIP_REGISTER_METHOD) == 0 ||
          method.compareTo(SIP_PUBLISH_METHOD)  == 0 )
      {
         isRecordRoutable = FALSE;
      }
      else
      {
         isRecordRoutable = TRUE;
      }
   }
   return isRecordRoutable;
}

UtlBoolean SipMessage::isUrlHeaderAllowed(const char* headerFieldName)
{
    UtlString name(headerFieldName);
    name.toUpper();

    return (!sSipMessageFieldProps.mDisallowedUrlHeaders.contains(&name));
}

UtlBoolean SipMessage::isUrlHeaderUnique(const char* headerFieldName)
{
    UtlString name(headerFieldName);
    name.toUpper();

    return (sSipMessageFieldProps.mUniqueUrlHeaders.contains(&name));
}

//SDUA
UtlBoolean SipMessage::getDNSField( UtlString * Protocol , UtlString * Address, UtlString * Port) const
{

   //protocol can be empty by default
   if( !m_dnsAddress.isNull() && !m_dnsPort.isNull())
   {
      Protocol->remove(0);
      Address->remove(0);
      Port->remove(0);

      Protocol->append(m_dnsProtocol);
      Address->append(m_dnsAddress);
      Port->append(m_dnsPort);
      return (true);
   }
   else
   {
      return (false);
   }
}

void SipMessage::setDNSField( const char* Protocol , const char* Address, const char* Port)
{
   m_dnsProtocol.remove(0);
   m_dnsAddress.remove(0);
   m_dnsPort.remove(0);

   m_dnsProtocol.append(Protocol);
   m_dnsAddress.append(Address);
   m_dnsPort.append(Port);
}

void SipMessage::clearDNSField()
{
   m_dnsProtocol.remove(0);
   m_dnsAddress.remove(0);
   m_dnsPort.remove(0);
}

void SipMessage::setTransaction(SipTransaction* transaction)
{
    mpSipTransaction = transaction;
}

SipTransaction* SipMessage::getSipTransaction() const
{
    return(mpSipTransaction);
}

void SipMessage::ParseContactFields(const SipMessage *registerResponse,
                                    const SipMessage *SipRequest,
                                    const UtlString &subField,
                                    int& subFieldRetVal)
{
   //get the request contact value ...so that we can find out the expires subfield value
   // for this contact from the list of contacts returned byt the Rgister server
   UtlString RequestContactValue;
   SipRequest->getContactEntry(0 , &RequestContactValue);

   UtlString contactField;
   int indexContactField = 0;

   while (registerResponse->getContactEntry(indexContactField , &contactField))
   {
      if ( strstr(contactField, RequestContactValue ) != NULL)
      {
         UtlString subfieldText;
         int subfieldIndex = 0;
         UtlString subfieldName;
         UtlString subfieldValue;
         NameValueTokenizer::getSubField(contactField.data(), subfieldIndex, ";", &subfieldText);
         while(!subfieldText.isNull())
         {
            NameValueTokenizer::getSubField(subfieldText.data(), 0, "=", &subfieldName);
            NameValueTokenizer::getSubField(subfieldText.data(), 1, "=", &subfieldValue);
#               ifdef TEST_PRINT
            osPrintf("ipMessage::ParseContactFields found contact parameter[%d]: \"%s\" value: \"%s\"\n",
               subfieldIndex, subfieldName.data(), subfieldValue.data());
#               endif
            subfieldName.toUpper();
            if(subfieldName.compareTo(subField, UtlString::ignoreCase) == 0 &&
               subField.compareTo(SIP_EXPIRES_FIELD, UtlString::ignoreCase)== 0)
            {

               //see if more than one token in the expire value
               NameValueTokenizer::getSubField(subfieldValue, 1,
               " \t:;,", &subfieldText);

               // if not ...time is in seconds
               if(subfieldText.isNull())
               {
                  subFieldRetVal = atoi(subfieldValue);
               }
               // If there is more than one token assume it is a text date
               else
               {
                  // Get the expiration date
                  long dateExpires = OsDateTime::convertHttpDateToEpoch(subfieldValue);
                  long dateSent = 0;
                  // If the date was not set in the message
                  if(!registerResponse->getDateField(&dateSent))
                  {
                     #ifdef TEST_PRINT
                     osPrintf("Date field not set\n");
                     #endif
                     // Assume date sent is now
                     dateSent = OsDateTime::getSecsSinceEpoch();
                  }
                  #ifdef TEST_PRINT
                  osPrintf("Contact expires date: %ld\n", dateExpires); osPrintf("Current time: %ld\n", dateSent);
                  #endif
                  subFieldRetVal = dateExpires - dateSent;
               }
               break;
            }//any other field
            else if(subfieldName.compareTo(subField, UtlString::ignoreCase) == 0)
            {
               subFieldRetVal = atoi(subfieldValue);
            }

            subfieldIndex++;
            NameValueTokenizer::getSubField(contactField.data(), subfieldIndex, ";", &subfieldText);
         }
      }
      indexContactField ++;
   }
   return ;

}

const UtlString& SipMessage::getInterfaceIp() const
{
    return mInterfaceIp;
}

int SipMessage::getInterfacePort() const
{
    return mInterfacePort;
}

void SipMessage::setInterfaceIpPort(const char* ip, int port)
{
    mInterfaceIp = ip;
    mInterfacePort = port ;
}

/// Get the name/value pairs for a Via field
///
///
void SipMessage::parseViaParameters( const char* viaField
                                    ,UtlContainer& viaParamList
                                    )

{
    const char* pairSeparator = ";";
    const char* namValueSeparator = "=";

    const char* nameAndValuePtr;
    ssize_t nameAndValueLength;
    const char* namePtr;
    ssize_t nameLength;
    ssize_t nameValueIndex = 0;
   UtlString value;
    ssize_t lastCharIndex = 0;
    ssize_t relativeIndex;
    ssize_t nameValueRelativeIndex;
    ssize_t viaFieldLength = strlen(viaField);

    do
    {
#       ifdef  TEST_PRINT
        osPrintf("SipMessage::parseViaParameters: \"%s\" lastCharIndex: %zu",
                 &(viaField[lastCharIndex]), lastCharIndex);
#       endif
        // Pull out a name value pair
        NameValueTokenizer::getSubField(&(viaField[lastCharIndex]),
                                        viaFieldLength - lastCharIndex,
                                        0,
                                        pairSeparator,
                                        nameAndValuePtr,
                                        nameAndValueLength,
                                        &relativeIndex);
        lastCharIndex += relativeIndex;

        if(nameAndValuePtr && nameAndValueLength > 0)
        {
            // Separate the name and value
            NameValueTokenizer::getSubField(nameAndValuePtr,
                                            nameAndValueLength,
                                            0,
                                            namValueSeparator,
                                            namePtr,
                                            nameLength,
                                            &nameValueRelativeIndex);

            // Get rid of leading white space in the name
            while(nameLength > 0 &&
                  (*namePtr == ' ' ||
                   *namePtr == '\t'))
            {
                nameLength--;
                namePtr++;
            }

            if(nameLength > 0)
            {
                ssize_t valueSeparatorOffset = strspn(&(namePtr[nameLength]),
                                                  namValueSeparator);
                const char* valuePtr = &(namePtr[nameLength]) + valueSeparatorOffset;
                ssize_t valueLength = nameAndValueLength -
                    (valuePtr - nameAndValuePtr);

                // If there is a value
                if(valueSeparatorOffset <= 0 ||
                   *valuePtr == '\0' ||
                   valueLength <= 0)
                {
                    valuePtr = NULL;
                    valueLength = 0;
                }

                NameValuePair* newNvPair = new NameValuePair("");
                newNvPair->append(namePtr, nameLength);
                if(valuePtr)
                {
                    value.remove(0);
                    value.append(valuePtr, valueLength);
                    NameValueTokenizer::frontBackTrim(&value, " \t\n\r");
                    newNvPair->setValue(value);
                }
                else
                {
                    newNvPair->setValue("");
                }

                NameValueTokenizer::frontBackTrim(newNvPair, " \t\n\r");

                // Add a name, value pair to the list
                viaParamList.insert(newNvPair);

                nameValueIndex++;
            }
        }
    } while(   nameAndValuePtr
            && nameAndValueLength > 0
            && viaField[lastCharIndex] != '\0'
            );
}

void SipMessage::useChunkedBody(bool useChunked)
{
   if (useChunked)
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT,
                    "SipMessage::useChunkedBody chunked encoding is not allowed in SIP");
      assert(useChunked);
   }
}

void SipMessage::setSipIfMatchField(const char* sipIfMatchField)
{
    setHeaderValue(SIP_IF_MATCH_FIELD, sipIfMatchField, 0);
}

UtlBoolean SipMessage::getSipIfMatchField(UtlString& sipIfMatchField) const
{
    const char* fieldValue = getHeaderValue(0, SIP_IF_MATCH_FIELD);
    sipIfMatchField.remove(0);
    if(fieldValue) sipIfMatchField.append(fieldValue);
    return(fieldValue != NULL);
}

void SipMessage::setSipETagField(const char* sipETagField)
{
    setHeaderValue(SIP_ETAG_FIELD, sipETagField, 0);
}

UtlBoolean SipMessage::getSipETagField(UtlString& sipETagField) const
{
    const char* fieldValue = getHeaderValue(0, SIP_ETAG_FIELD);
    sipETagField.remove(0);
    if(fieldValue) sipETagField.append(fieldValue);
    return(fieldValue != NULL);
}


/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

SipMessage::SipMessageFieldProps::SipMessageFieldProps()
{
   // Call the initializer functions.
   initNames();
   initDisallowedUrlHeaders();
   initUniqueUrlHeaders();
}


SipMessage::SipMessageFieldProps::~SipMessageFieldProps()
{
   // free the various value mappings
   mLongFieldNames.destroyAll();
   mShortFieldNames.destroyAll();
   mDisallowedUrlHeaders.destroyAll();
   mUniqueUrlHeaders.destroyAll();
}


void SipMessage::SipMessageFieldProps::initNames()
{
   // Load the table to translate long header names to short names.

   mLongFieldNames.insert(new NameValuePair(SIP_CONTENT_TYPE_FIELD, SIP_SHORT_CONTENT_TYPE_FIELD));
   mLongFieldNames.insert(new NameValuePair(SIP_CONTENT_ENCODING_FIELD, SIP_SHORT_CONTENT_ENCODING_FIELD));
   mLongFieldNames.insert(new NameValuePair(SIP_FROM_FIELD, SIP_SHORT_FROM_FIELD));
   mLongFieldNames.insert(new NameValuePair(SIP_CALLID_FIELD, SIP_SHORT_CALLID_FIELD));
   mLongFieldNames.insert(new NameValuePair(SIP_CONTACT_FIELD, SIP_SHORT_CONTACT_FIELD));
   mLongFieldNames.insert(new NameValuePair(SIP_CONTENT_LENGTH_FIELD, SIP_SHORT_CONTENT_LENGTH_FIELD));
   mLongFieldNames.insert(new NameValuePair(SIP_REFERRED_BY_FIELD, SIP_SHORT_REFERRED_BY_FIELD));
   mLongFieldNames.insert(new NameValuePair(SIP_REFER_TO_FIELD, SIP_SHORT_REFER_TO_FIELD));
   mLongFieldNames.insert(new NameValuePair(SIP_SUBJECT_FIELD, SIP_SHORT_SUBJECT_FIELD));
   mLongFieldNames.insert(new NameValuePair(SIP_SUPPORTED_FIELD, SIP_SHORT_SUPPORTED_FIELD));
   mLongFieldNames.insert(new NameValuePair(SIP_TO_FIELD, SIP_SHORT_TO_FIELD));
   mLongFieldNames.insert(new NameValuePair(SIP_VIA_FIELD, SIP_SHORT_VIA_FIELD));
   mLongFieldNames.insert(new NameValuePair(SIP_EVENT_FIELD, SIP_SHORT_EVENT_FIELD));

   // Reverse the pairs to load the table to translate short header names to
   // long ones.

   UtlHashBagIterator iterator(mLongFieldNames);
   NameValuePair* nvPair;
   while ((nvPair = dynamic_cast <NameValuePair*> (iterator())))
   {
      mShortFieldNames.insert(new NameValuePair(nvPair->getValue(),
                                                 nvPair->data()));
   }
}

void SipMessage::SipMessageFieldProps::initDisallowedUrlHeaders()
{
   // These headers may NOT be passed through in a URL to
   // be set in a message

   mDisallowedUrlHeaders.insert(new UtlString(SIP_FROM_FIELD));
   mDisallowedUrlHeaders.insert(new UtlString(SIP_CONTACT_FIELD));
   mDisallowedUrlHeaders.insert(new UtlString(SIP_SHORT_CONTACT_FIELD));
   mDisallowedUrlHeaders.insert(new UtlString(SIP_CONTENT_LENGTH_FIELD));
   mDisallowedUrlHeaders.insert(new UtlString(SIP_SHORT_CONTENT_LENGTH_FIELD));
   mDisallowedUrlHeaders.insert(new UtlString(SIP_CONTENT_TYPE_FIELD));
   mDisallowedUrlHeaders.insert(new UtlString(SIP_SHORT_CONTENT_TYPE_FIELD));
   mDisallowedUrlHeaders.insert(new UtlString(SIP_CONTENT_ENCODING_FIELD));
   mDisallowedUrlHeaders.insert(new UtlString(SIP_SHORT_CONTENT_ENCODING_FIELD));
   mDisallowedUrlHeaders.insert(new UtlString(SIP_CSEQ_FIELD));
   mDisallowedUrlHeaders.insert(new UtlString(SIP_RECORD_ROUTE_FIELD));
   mDisallowedUrlHeaders.insert(new UtlString(SIP_REFER_TO_FIELD));
   mDisallowedUrlHeaders.insert(new UtlString(SIP_REFERRED_BY_FIELD));
   mDisallowedUrlHeaders.insert(new UtlString(SIP_TO_FIELD));
   mDisallowedUrlHeaders.insert(new UtlString(SIP_SHORT_TO_FIELD));
   mDisallowedUrlHeaders.insert(new UtlString(SIP_USER_AGENT_FIELD));
   mDisallowedUrlHeaders.insert(new UtlString(SIP_VIA_FIELD));
   mDisallowedUrlHeaders.insert(new UtlString(SIP_SHORT_VIA_FIELD));
}

void SipMessage::SipMessageFieldProps::initUniqueUrlHeaders()
{
   // These headers may occur only once in a message, so a URI header
   // parameter overrides the existing header in the message.

   mUniqueUrlHeaders.insert(new UtlString(SIP_EXPIRES_FIELD));
   mUniqueUrlHeaders.insert(new UtlString(SIP_ROUTE_FIELD));
}

void SipMessage::normalizeProxyRoutes(const SipUserAgent* sipUA,
                                      Url& requestUri,
                                      UtlSList* removedRoutes
                                      )
{
   UtlString requestUriString;
   Url topRouteUrl;
   UtlString topRouteValue;

   /*
    * Check the request URI and the topmost route
    *   - Detect and correct for any strict router upstream
    *     as specified by RFC 3261 section 16.4 Route Information Preprocessing:
    *
    *       The proxy MUST inspect the Request-URI of the request.  If the
    *       Request-URI of the request contains a value this proxy previously
    *       placed into a Record-Route header field (see Section 16.6 item 4),
    *       the proxy MUST replace the Request-URI in the request with the last
    *       value from the Route header field, and remove that value from the
    *       Route header field.  The proxy MUST then proceed as if it received
    *       this modified request.
    *
    *   - Pop off the topmost route until it is not me
    *
    * Note that this loop always executes at least once, and that:
    *   - it leaves requestUri set correctly
    */
   bool doneNormalizingRouteSet = false;
   while (! doneNormalizingRouteSet)
   {
      // Check the request URI.
      //    If it has 'lr' parameter that is me, then the sender was a
      //    strict router (it didn't recognize the loose route indication)
      getRequestUri(&requestUriString);
      requestUri.fromString(requestUriString, TRUE /* is a request uri */);

      UtlString noValue;
      if (   requestUri.getUrlParameter("lr", noValue, 0)
          && sipUA->isMyHostAlias(requestUri)
          )
      {
         /*
          * We need to fix it (convert it back to a loose route)..
          * - pop the last route and put it in the request URI
          *   see RFC 3261 section 16.4
          *
          * For example:
          *   INVITE sip:mydomain.com;lr SIP/2.0
          *   Route: <sip:proxy.example.com;lr>, <sip:user@elsewhere.example.com>
          * becomes:
          *   INVITE sip:user@elsewhere.example.com SIP/2.0
          *   Route: <sip:proxy.example.com;lr>
          */
         UtlString lastRouteValue;
         int lastRouteIndex;
         if ( getLastRouteUri(lastRouteValue, lastRouteIndex) )
         {
            removeRouteUri(lastRouteIndex, &lastRouteValue);

            // Put the last route in as the request URI
            changeUri(lastRouteValue); // this strips appropriately

            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipMessage::normalizeProxyRoutes "
                          "strict route '%s' replaced with uri from '%s'",
                          requestUriString.data(), lastRouteValue.data());
            if (removedRoutes)
            {
               // save a copy of the route we're removing for the caller.
               // this looks just like this was properly loose routed.
               // use the output from Url::toString so that all are normalized.
               UtlString* removedRoute = new UtlString;
               requestUri.toString(*removedRoute);
               removedRoutes->append(removedRoute);
               // caller is responsible for deleting the savedRoute
            }
         }
         else
         {
            OsSysLog::add(FAC_SIP, PRI_WARNING, "SipMessage::normalizeProxyRoutes  "
                          "found 'lr' in Request-URI with no Route; stripped 'lr'"
                          );

            requestUri.removeUrlParameter("lr");
            UtlString newUri;
            requestUri.toString(newUri);
            changeRequestUri(newUri); // this strips appropriately
         }

         // note: we've changed the request uri and the route set,
         //       but not set doneNormalizingRouteSet, so we go around again...
      }
      else // topmost route was not a loose route uri we put there...
      {
         if ( getRouteUri(0, &topRouteValue) )
         {
            /*
             * There is a Route header... if it is a route to this proxy, pop it off.
             * For example:
             *   INVITE sip:user@elsewhere.example.com SIP/2.0
             *   Route: <sip:mydomain.com;lr>, <sip:proxy.example.com;lr>
             * becomes:
             *   INVITE sip:user@elsewhere.example.com SIP/2.0
             *   Route: <sip:proxy.example.com;lr>
             */
            topRouteUrl.fromString(topRouteValue,FALSE /* not a request uri */);
            if ( sipUA->isMyHostAlias(topRouteUrl) )
            {
               if (removedRoutes)
               {
                  // save a copy of the route we're removing for the caller.
                  // use the output from Url::toString so that all are normalized.
                  UtlString* savedRoute = new UtlString();
                  topRouteUrl.toString(*savedRoute);
                  removedRoutes->append(savedRoute);
                  // caller is responsible for deleting the savedRoute
               }
               UtlString removedRoute;
               removeRouteUri(0, &removedRoute);

               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                             "SipMessage::normalizeProxyRoutes popped route to self '%s'",
                             removedRoute.data()
                             );
            }
            else // topmost route is someone else
            {
               doneNormalizingRouteSet = true;
            }
         }
         else // no more routes
         {
            doneNormalizingRouteSet = true;
         }
      }
   } // while ! doneNormalizingRouteSet
}

SdpBody* SipMessage::convertToSdpBody(const HttpBody* pHttpBody)
{
    SdpBody* pSdpBody = NULL;

    if(pHttpBody)
    {
        const char* theBody;
        ssize_t theLength;
        pHttpBody->getBytes(&theBody, &theLength);
        pSdpBody = new SdpBody(theBody,theLength);
    }
    return pSdpBody;

}

void SipMessage::setDiagnosticSipFragResponse(const SipMessage& message,
                                              int               responseCode,
                                              const char*       responseText,
                                              int               warningCode,
                                              const char*       warningText,
                                              const UtlString&  address
                                              )
{
   setResponseData(&message, responseCode, responseText);

   setWarningField(warningCode, address.data(), warningText);

   UtlString sipFragString;
   ssize_t sipFragLen;
   message.getBytes(&sipFragString, &sipFragLen, false /* don't inlcude the body */);

   // Create a body to contain the Vias from the request
   HttpBody* sipFragBody =
      new HttpBody(sipFragString.data(), sipFragLen, CONTENT_TYPE_MESSAGE_SIPFRAG);

   // Attach the body to the response
   setBody(sipFragBody);

   // Set the content type of the body to be sipfrag
   setContentType(CONTENT_TYPE_MESSAGE_SIPFRAG);
}

void SipMessage::setReasonField(const char* reasonString)
{
    setHeaderValue(SIP_REASON_FIELD, reasonString);
}
