//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// APPLICATION INCLUDES
#include <os/OsSysLog.h>
#include <os/OsLock.h>

#include <utl/UtlSListIterator.h>
#include <utl/UtlTokenizer.h>
#include <net/SdpBody.h>
#include <net/NameValuePair.h>
#include <net/NameValueTokenizer.h>
#include <net/SdpCodecFactory.h>
#include <utl/UtlTokenizer.h>
#include <net/NetBase64Codec.h>

//#define TEST_PRINT 1

#define MAXIMUM_LONG_INT_CHARS 20
#define MAXIMUM_MEDIA_TYPES 20

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define SDP_NAME_VALUE_DELIMITOR '='
#define SDP_SUBFIELD_SEPARATOR ' '
#define SDP_SUBFIELD_SEPARATORS "\t "

#define SDP_ATTRIB_NAME_VALUE_SEPARATOR ":"

#define SDP_AUDIO_MEDIA_TYPE "audio"
#define SDP_VIDEO_MEDIA_TYPE "video"
#define SDP_APPLICATION_MEDIA_TYPE "application"

#define SDP_RTP_MEDIA_TRANSPORT_TYPE "RTP/AVP"
#define SDP_SRTP_MEDIA_TRANSPORT_TYPE "RTP/SAVP"
#define SDP_MLAW_PAYLOAD 0
#define SDP_ALAW_PAYLOAD 8

#define SDP_NETWORK_TYPE "IN"
#define SDP_IP4_ADDRESS_TYPE "IP4"
#define NTP_TO_EPOCH_DELTA 2208988800UL


// STATIC VARIABLE INITIALIZATIONS
static int     sSessionCount = 5 ;  // Session version for SDP body
static OsMutex sSessionLock(OsMutex::Q_FIFO) ;

// The a= values that represent the directionality values.
const char* SdpBody::sdpDirectionalityStrings[4] =
{
   "inactive",
   "sendonly",
   "recvonly",
   "sendrecv"
};

// Calcuate the "reverse" of a given directionality.
const SdpDirectionality SdpBody::sdpDirectionalityReverse[4] =
{
   sdpDirectionalityInactive,
   sdpDirectionalityRecvOnly,
   sdpDirectionalitySendOnly,
   sdpDirectionalitySendRecv
};


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SdpBody::SdpBody(const char* bodyBytes, ssize_t byteCount)
{
   remove(0);
   append(SDP_CONTENT_TYPE);

   sdpFields = new UtlSList();

   if(bodyBytes)
   {
      if(byteCount < 0)
      {
         bodyLength = strlen(bodyBytes);
      }
      parseBody(bodyBytes, byteCount);
   }
   else
   {
      // this is the mandated order of the header fields
      addValue("v", "0" );
      addValue("o", "sipXecs 5 5 IN IP4 127.0.0.1");
      addValue("s");
      addValue("i");
      addValue("u");
      addValue("e");
      addValue("p");
      addValue("c");
      addValue("b");
   }
}

// Copy constructor
SdpBody::SdpBody(const SdpBody& rSdpBody) :
   HttpBody(rSdpBody)
{
   if(rSdpBody.sdpFields)
   {
      sdpFields = new UtlSList();
      rSdpBody.sdpFields->copyTo<NameValuePair>( *sdpFields );
   }
   else
   {
      sdpFields = NULL;
   }
}

SdpBody* SdpBody::copy() const
{
  return new SdpBody(*this);
}

// Destructor
SdpBody::~SdpBody()
{
   if(sdpFields)
   {
      while(! sdpFields->isEmpty())
      {
         delete sdpFields->get();
      }
      delete sdpFields;
   }
}

/* ============================ MANIPULATORS ============================== */

void SdpBody::parseBody(const char* bodyBytes, ssize_t byteCount)
{
   if(byteCount < 0)
   {
      bodyLength = strlen(bodyBytes);
   }

   if(bodyBytes)
   {
      UtlString name;
      UtlString value;
      int nameFound;
      NameValuePair* nameValue;

      NameValueTokenizer parser(bodyBytes, byteCount);
      do
      {
         name.remove(0);
         value.remove(0);
         nameFound = parser.getNextPair(SDP_NAME_VALUE_DELIMITOR,
                                        &name, &value);
         if(nameFound)
         {
            nameValue = new NameValuePair(name.data(), value.data());
            sdpFields->append(nameValue);

         }
      }
      while( !parser.isAtEnd() );
   }
}


// Assignment operator
SdpBody&
SdpBody::operator=(const SdpBody& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   // Copy the base class stuff
   this->HttpBody::operator=((const HttpBody&)rhs);

   if(sdpFields)
   {
      sdpFields->destroyAll();
   }

   if(rhs.sdpFields)
   {
      if(sdpFields == NULL)
      {
         sdpFields = new UtlSList();
      }

      NameValuePair* headerField;
      NameValuePair* copiedHeader = NULL;
      UtlSListIterator iterator((UtlSList&)(*(rhs.sdpFields)));
      while((headerField = (NameValuePair*) iterator()))
      {
         copiedHeader = new NameValuePair(*headerField);
         sdpFields->append(copiedHeader);
      }
   }

   return *this;
}

/* ============================ ACCESSORS ================================= */
void SdpBody::setStandardHeaderFields(const char* sessionName,
                                      const char* emailAddress,
                                      const char* phoneNumber,
                                      const char* originatorAddress)
{
   OsLock lock (sSessionLock) ;

   setOriginator("sipXecs", 5, sSessionCount++,
                 (originatorAddress && *originatorAddress) ?
                 originatorAddress : "127.0.0.1");
   setSessionNameField(sessionName);
   if(emailAddress && strlen(emailAddress) > 0)
   {
      setEmailAddressField(emailAddress);
   }
   if(phoneNumber && strlen(phoneNumber) > 0)
   {
      setPhoneNumberField(phoneNumber);
   }
}

void SdpBody::setSessionNameField(const char* sessionName)
{
   setValue("s", sessionName);
}

void SdpBody::setEmailAddressField(const char* emailAddress)
{
   setValue("e", emailAddress);
}

void SdpBody::setPhoneNumberField(const char* phoneNumber)
{
   setValue("p", phoneNumber);
}

void SdpBody::setValue(const char* name, const char* value)
{
   NameValuePair nvToMatch(name);
   NameValuePair* nvFound = NULL;
   UtlSListIterator iterator(*sdpFields);

   nvFound = (NameValuePair*) iterator.findNext(&nvToMatch);
   if(nvFound)
   {
      // field exists - replace the value
      nvFound->setValue(value);
   }
   else
   {
      addValue(name, value);
   }
}

int SdpBody::getMediaSetCount() const
{
   UtlSListIterator iterator(*sdpFields);
   NameValuePair mediaName("m");
   int count = 0;
   while(iterator.findNext(&mediaName))
   {
      count++;
   }
   return(count);

}

UtlBoolean SdpBody::getMediaType(int mediaIndex, UtlString* mediaType) const
{
   return(getMediaSubfield(mediaIndex, 0, mediaType));
}

UtlBoolean SdpBody::getMediaPort(int mediaIndex, int* port) const
{
   UtlString portString;
   UtlBoolean portFound = getMediaSubfield(mediaIndex, 1, &portString);
   ssize_t portCountSeparator;

   if(!portString.isNull())
   {
      // Remove the port pair count if it exists
      portCountSeparator = portString.index("/");
      if(portCountSeparator >= 0)
      {
         portString.remove(portCountSeparator);
      }

      *port = atoi(portString.data());
      portFound = TRUE;
   }

   return(portFound);
}

UtlBoolean SdpBody::getMediaRtcpPort(int mediaIndex, int* port) const
{
    UtlBoolean bFound = FALSE ;
    int iRtpPort ;

    if (getMediaPort(mediaIndex, &iRtpPort))
    {
        bFound = TRUE ;
        *port = iRtpPort + 1;

        UtlSListIterator iterator(*sdpFields);
        NameValuePair* nv = positionFieldInstance(mediaIndex, &iterator, "m");
        if(nv)
        {
            while ((nv = findFieldNameBefore(&iterator, "a", "m")))
            {
                UtlString typeAttribute ;
                UtlString portAttribute ;

                NameValueTokenizer::getSubField(nv->getValue(), 0,
                        ":", &typeAttribute) ;
                NameValueTokenizer::getSubField(nv->getValue(), 1,
                        ":", &portAttribute) ;

                if (typeAttribute.compareTo("rtcp", UtlString::ignoreCase) == 0)
                {
                    *port = atoi(portAttribute.data()) ;

                }
            }
        }
    }

    return bFound ;
}

UtlBoolean SdpBody::getMediaProtocol(int mediaIndex, UtlString* transportProtocol) const
{
   return(getMediaSubfield(mediaIndex, 2, transportProtocol));
}

UtlBoolean SdpBody::getMediaPayloadType(int mediaIndex, int maxTypes,
                                        int* numTypes, int payloadTypes[]) const
{
   UtlString payloadTypeString;
   int typeCount = 0;

   while(   typeCount < maxTypes
         && getMediaSubfield(mediaIndex, 3 + typeCount, &payloadTypeString))
   {

      if(!payloadTypeString.isNull())
      {
         payloadTypes[typeCount] = atoi(payloadTypeString.data());
         typeCount++;
      }
   }

   *numTypes = typeCount;

   return(typeCount > 0);
}

UtlBoolean SdpBody::getMediaAttribute(int mediaIndex, const UtlString& attributeNameTofind, UtlString* pAttributeValue ) const
{
   bool bAttributeFound = false;

   if( pAttributeValue )
   {
      pAttributeValue->remove( 0 );
   }

   NameValuePair* nv;
   nv = getRefToMediaAttributeByName( mediaIndex, attributeNameTofind );
   if( nv )
   {
      bAttributeFound = true;
      if( pAttributeValue )
      {
         const char* attribNameAndValue = NULL;
         attribNameAndValue = nv->getValue();
         if(attribNameAndValue)
         {
            NameValueTokenizer::getSubField(attribNameAndValue, 1,
                                            SDP_ATTRIB_NAME_VALUE_SEPARATOR, pAttributeValue );
         }
      }
   }
   return bAttributeFound;
}

UtlBoolean SdpBody::getMediaSubfield(int mediaIndex, int subfieldIndex, UtlString* subField) const
{
   UtlBoolean subfieldFound = FALSE;
   UtlSListIterator iterator(*sdpFields);
   NameValuePair* nv = positionFieldInstance(mediaIndex, &iterator, "m");
   const char* value;
   subField->remove(0);

   if(nv)
   {
      value =  nv->getValue();
      NameValueTokenizer::getSubField(value, subfieldIndex,
                                      SDP_SUBFIELD_SEPARATORS, subField);
      if(!subField->isNull())
      {
         subfieldFound = TRUE;
      }
   }
   return(subfieldFound);
}

UtlBoolean SdpBody::getPayloadRtpMap(int payloadType,
                                     UtlString& mimeSubtype,
                                     int& sampleRate,
                                     int& numChannels) const
{
   // an "a" record looks something like:
   // "a=rtpmap: <payloadType> <mimeSubtype/sampleRate[/numChannels]"

   // Loop through all of the "a" records
   UtlBoolean foundRtpMap = FALSE;
   UtlSListIterator iterator(*sdpFields);
   NameValuePair* nv = NULL;
   ssize_t aFieldIndex = 0;
   const char* value;
   UtlString aFieldType;
   UtlString payloadString;
   UtlString sampleRateString;
   UtlString numChannelString;
   UtlString aFieldMatch("a");

   while((nv = (NameValuePair*) iterator.findNext(&aFieldMatch)) != NULL)
   {
      value =  nv->getValue();

      // Verify this is an rtpmap "a" record
      NameValueTokenizer::getSubField(value, 0,
                                      " \t:/", // separators
                                      &aFieldType);
      if(aFieldType.compareTo("rtpmap") == 0)
      {
         // If this is the rtpmap for the requested payload type
         NameValueTokenizer::getSubField(value, 1,
                                         " \t:/", // separators
                                         &payloadString);
         if(atoi(payloadString.data()) == payloadType)
         {
            // The mime subtype is the 3rd subfield
            NameValueTokenizer::getSubField(value, 2,
                                            " \t:/", // separators
                                            &mimeSubtype);

            // The sample rate is the 4th subfield
            NameValueTokenizer::getSubField(value, 3,
                                            " \t:/", // separators
                                            &sampleRateString);
            sampleRate = atoi(sampleRateString.data());
            if(sampleRate <= 0) sampleRate = -1;

            // The number of channels is the 5th subfield
            NameValueTokenizer::getSubField(value, 4,
                                            " \t:/", // separators
                                            &numChannelString);
            numChannels = atoi(numChannelString.data());
            if(numChannels <= 0) numChannels = -1;

            // we are all done
            foundRtpMap = TRUE;
            break;
         }
      }

      aFieldIndex++;
   }
   return(foundRtpMap);
}

UtlBoolean SdpBody::getPayloadFormat(int payloadType,
                                     UtlString& fmtp,
                                     int& videoFmtp) const
{
   // an "a" record look something like:
   // "a=rtpmap: <payloadType> <mimeSubtype/sampleRate[/numChannels]"

   // Loop through all of the "a" records
   UtlBoolean foundRtpMap = FALSE;
   UtlBoolean foundField;
   UtlSListIterator iterator(*sdpFields);
   UtlString aFieldType;
   UtlString payloadString;
   NameValuePair* nv = NULL;
   int index = 0;
   int aFieldIndex = 0;
   const char* value;
   UtlString temp;
   UtlString aFieldMatch("a");

   while((nv = (NameValuePair*) iterator.findNext(&aFieldMatch)) != NULL)
   {
      value =  nv->getValue();

      // Verify this is an rtpmap "a" record
      NameValueTokenizer::getSubField(value, 0,
                                      " \t:/", // separators
                                      &aFieldType);
      if(aFieldType.compareTo("fmtp") == 0)
      {
         NameValueTokenizer::getSubField(value, 1,
                                         " \t:/", // separators
                                         &payloadString);
         if(atoi(payloadString.data()) == payloadType)
         {
            // If this is the rtpmap for the requested payload type
            foundField = NameValueTokenizer::getSubField(value, 3,
                                                    " \t:", // separators
                                                    &fmtp);


            videoFmtp = 0;
            index = 3;
            while (foundField && index < 7)
            {
                foundField = NameValueTokenizer::getSubField(value, index++,
                                                            " \t/:", // separators
                                                            &temp);
                if (temp.compareTo("CIF") == 0)
                {
                    videoFmtp |= SDP_VIDEO_FORMAT_CIF;
                }
                else if (temp.compareTo("QCIF") == 0)
                {
                    videoFmtp |= SDP_VIDEO_FORMAT_QCIF;
                }
                else if (temp.compareTo("SQCIF") == 0)
                {
                    videoFmtp |= SDP_VIDEO_FORMAT_SQCIF;
                }
            }
         }
      }

      aFieldIndex++;
   }
   return(foundRtpMap);
}

UtlBoolean SdpBody::getSrtpCryptoField(int mediaIndex,
                                       int index,
                                       SdpSrtpParameters& params) const
{
    UtlBoolean foundCrypto = FALSE;
    UtlBoolean foundField;
    UtlString aFieldType;
    UtlSListIterator iterator(*sdpFields);
    NameValuePair* nv = positionFieldInstance(mediaIndex, &iterator, "m");
    const char* value;
    UtlString aFieldMatch("a");
    UtlString indexString;
    UtlString cryptoSuite;
    UtlString temp;
    int size;
    char srtpKey[SRTP_KEY_LENGTH+1];

    size = sdpFields->entries();
    while((nv = (NameValuePair*) iterator.findNext(&aFieldMatch)) != NULL)
    {
        value =  nv->getValue();

        // Verify this is an crypto "a" record
        NameValueTokenizer::getSubField(value, 0,
                                        " \t:/", // separators
                                        &aFieldType);
        if(aFieldType.compareTo("crypto") == 0)
        {
            NameValueTokenizer::getSubField(value, 1,
                                            " \t:/", // separators
                                            &indexString);
            if(atoi(indexString.data()) == index)
            {
                foundCrypto = TRUE;

                // Encryption & authentication on by default
                params.securityLevel = SRTP_ENCRYPTION | SRTP_AUTHENTICATION;

                NameValueTokenizer::getSubField(value, 2,
                                                " \t:/", // separators
                                                &cryptoSuite);
                // Check the crypto suite
                if (cryptoSuite.compareTo("AES_CM_128_HMAC_SHA1_80") == 0)
                {
                    params.cipherType = AES_CM_128_HMAC_SHA1_80;
                }
                else if (cryptoSuite.compareTo("AES_CM_128_HAMC_SHA1_32") == 0)
                {
                    params.cipherType = AES_CM_128_HAMC_SHA1_32;
                }
                else if (cryptoSuite.compareTo("F8_128_HMAC_SHA1_80") == 0)
                {
                    params.cipherType = F8_128_HMAC_SHA1_80;
                }
                else
                {
                    //Couldn't find crypto suite, no secritiy
                    params.securityLevel = 0;
                }

                // Get key
                foundField = NameValueTokenizer::getSubField(value, 4,
                                                             " \t/:|", // separators
                                                             &temp);
                NetBase64Codec::decode(temp.length(), temp.data(), size, srtpKey);
                strncpy((char*)params.masterKey, srtpKey, SRTP_KEY_LENGTH);

                // Modify security level with session parameters
                for (int index=5; foundField; ++index)
                {
                    foundField = NameValueTokenizer::getSubField(value, index,
                                                                 " \t/:|", // separators
                                                                 &temp);
                    if (foundField)
                    {
                        if (temp.compareTo("UNENCRYPTED_SRTP") == 0)
                        {
                            params.securityLevel &= ~SRTP_ENCRYPTION;
                        }
                        if (temp.compareTo("UNAUTHENTICATED_SRTP") == 0)
                        {
                            params.securityLevel &= ~SRTP_AUTHENTICATION;
                        }
                    }
                }

            }
            break;
        }
    }
    return foundCrypto;
}

UtlBoolean SdpBody::getValue(int fieldIndex, UtlString* name, UtlString* value) const
{
   NameValuePair* nv = NULL;
   name->remove(0);
   value->remove(0);
   if(fieldIndex >=0)
   {
      nv = (NameValuePair*) sdpFields->at(fieldIndex);
      if(nv)
      {
         *name = *nv;
         *value = nv->getValue();
      }
   }
   return(nv != NULL);
}

UtlBoolean SdpBody::getMediaData(int mediaIndex, UtlString* mediaType,
                                 int* mediaPort, int* mediaPortPairs,
                                 UtlString* mediaTransportType,
                                 int maxPayloadTypes, int* numPayloadTypes,
                                 int payloadTypes[],
                                 SdpDirectionality* directionality) const
{
   UtlBoolean fieldFound = FALSE;
   UtlSListIterator iterator(*sdpFields);
   NameValuePair* nv = positionFieldInstance(mediaIndex, &iterator, "m");
   const char* value;
   UtlString portString;
   UtlString portPairString;
   ssize_t portCountSeparator;
   int typeCount = 0;
   UtlString payloadTypeString;

   if(nv)
   {
      fieldFound = TRUE;
      value =  nv->getValue();

      // media Type
      NameValueTokenizer::getSubField(value, 0,
                                      SDP_SUBFIELD_SEPARATORS, mediaType);

      // media port and media port pair count
      NameValueTokenizer::getSubField(value, 1,
                                      SDP_SUBFIELD_SEPARATORS, &portString);
      if(!portString.isNull())
      {
         // Copy for obtaining the pair count
         portPairString.append(portString);

         // Remove the port pair count if it exists
         portCountSeparator = portString.index("/");
         if(portCountSeparator >= 0)
         {
            portString.remove(portCountSeparator);

            // Get the other part of the field
            portPairString.remove(0, portCountSeparator + 1);
         }
         else
         {
            portPairString.remove(0);
         }

         *mediaPort = atoi(portString.data());
         if(portPairString.isNull())
         {
            *mediaPortPairs = 1;
         }
         else
         {
            *mediaPortPairs = atoi(portPairString.data());
         }
      }
      else
      {
         *mediaPort = 0;
         *mediaPortPairs = 0;
      }

      // media transport type
      NameValueTokenizer::getSubField(value, 2,
                                      SDP_SUBFIELD_SEPARATORS, mediaTransportType);

      // media payload/codec types
      NameValueTokenizer::getSubField(value,  3 + typeCount,
                                      SDP_SUBFIELD_SEPARATORS, &payloadTypeString);
      while(typeCount < maxPayloadTypes &&
            !payloadTypeString.isNull())
      {
         payloadTypes[typeCount] = atoi(payloadTypeString.data());
         typeCount++;
         NameValueTokenizer::getSubField(value,  3 + typeCount,
                                         SDP_SUBFIELD_SEPARATORS, &payloadTypeString);
      }
      *numPayloadTypes = typeCount;

      // Get the directionality attribute, if any.
      if(!getMediaDirection(mediaIndex, directionality))
      {
          // The default value is sendrecv if there is no attribute.
          *directionality = sdpDirectionalitySendRecv;
      }
   }

   return(fieldFound);
}

UtlBoolean SdpBody::getMediaDirection(int mediaIndex, SdpDirectionality* directionality) const
{
    UtlBoolean directionFound = FALSE;
    UtlSListIterator iterator(*sdpFields);
    NameValuePair* nv = positionFieldInstance(mediaIndex, &iterator, "m");

    if (nv)
    {
        NameValuePair* fieldNV;
        bool loop = true;
        while (loop && (fieldNV = dynamic_cast <NameValuePair*> (iterator())))
        {
            if (fieldNV->compareTo("m") == 0)
            {
                // If we get to the next m= line, stop.
                loop = false;
            }
            else if (fieldNV->compareTo("a") == 0)
            {
                // Check to see if this a= line is a directionality attribute.
                for (unsigned int i = 0; loop && i
                        < sizeof (sdpDirectionalityStrings)
                                / sizeof (sdpDirectionalityStrings[0]); i++)
                {
                    if (strcmp(fieldNV->getValue(), sdpDirectionalityStrings[i])
                            == 0)
                    {
                        // This is a directionality attribute.
                        // Save the code value and exit.
                        *directionality = static_cast <SdpDirectionality> (i);
                        loop = false;
                        directionFound = TRUE;
                    }
                }
                // If the a= line value was not a directionality attribute,
                // continue examining lines.
            }
            // All other lines are ignored in this search.
        }
    }

    return directionFound;
}

int SdpBody::findMediaType(const char* mediaType, int startMediaIndex) const
{
   NameValuePair* mediaField;
   UtlSListIterator iterator(*sdpFields);
   UtlBoolean mediaTypeFound = FALSE;
   int index = startMediaIndex;
   NameValuePair mediaName("m");
   mediaField = positionFieldInstance(startMediaIndex, &iterator, "m");
   const char* value;

   while(mediaField && ! mediaTypeFound)
   {
      value = mediaField->getValue();
      if(strstr(value, mediaType) == value)
      {
         mediaTypeFound = TRUE;
         break;
      }

      mediaField = (NameValuePair*) iterator.findNext(&mediaName);
      index++;
   }

   if(! mediaTypeFound)
   {
      index = -1;
   }
   return(index);
}

UtlBoolean SdpBody::getMediaAddress(int mediaIndex, UtlString* address) const
{
   UtlSListIterator iterator(*sdpFields);
   NameValuePair* nv;
   address->remove(0);
   const char* value = NULL;
   ssize_t ttlIndex;

   // Try to find a specific address for the given media set
   nv = positionFieldInstance(mediaIndex, &iterator, "m");
   if(nv)
   {
      nv = findFieldNameBefore(&iterator, "c", "m");
      if(nv)
      {
         value = nv->getValue();
         if(value)
         {
            NameValueTokenizer::getSubField(value, 2,
                                            SDP_SUBFIELD_SEPARATORS, address);
         }
      }

      // Did not find a specific address try to find the default
      if(address->isNull())
      {
         iterator.reset();
         nv = findFieldNameBefore(&iterator, "c", "m");

         // Default address exists in the header
         if(nv)
         {
            value = nv->getValue();
            if(value)
            {
               NameValueTokenizer::getSubField(value, 2,
                                               SDP_SUBFIELD_SEPARATORS, address);
            }
         }
      }

      if(!address->isNull())
      {
         // Check if there is a time to live attribute and remove it
         ttlIndex = address->index("/");
         if(ttlIndex >= 0)
         {
            address->remove(ttlIndex);
         }
      }
   }

   return(!address->isNull());
}


void SdpBody::getBestAudioCodecs(int numRtpCodecs, SdpCodec rtpCodecs[], // this version does not seem to be used 04/2009
                                 UtlString* rtpAddress, int* rtpPort,
                                 int* sendCodecIndex,
                                 int* receiveCodecIndex) const
{

   int mediaIndex = 0;
   UtlBoolean sendCodecFound = FALSE;
   UtlBoolean receiveCodecFound = FALSE;
   int numTypes;
   int payloadTypes[MAXIMUM_MEDIA_TYPES];
   int typeIndex;
   int codecIndex;

   rtpAddress->remove(0);
   *rtpPort = 0;
   *sendCodecIndex = -1;
   *receiveCodecIndex = -1;

   while(mediaIndex >= 0
         && (!sendCodecFound
             || !receiveCodecFound))
   {
      mediaIndex = findMediaType(SDP_AUDIO_MEDIA_TYPE, mediaIndex);

      if(mediaIndex >= 0)
      {
         getMediaPort(mediaIndex, rtpPort);

         if(*rtpPort >= 0)
         {
            getMediaPayloadType(mediaIndex, MAXIMUM_MEDIA_TYPES,
                                &numTypes, payloadTypes);
            for(typeIndex = 0; typeIndex < numTypes; typeIndex++)
            {
               // Until the real SdpCodec is needed we assume all of
               // the rtpCodecs are send AND receive.
               // We are also going to cheat and assume that all of
               // the media records are send AND receive
               for(codecIndex = 0; codecIndex < numRtpCodecs; codecIndex++)
               {
                  if(payloadTypes[typeIndex] ==
                     (rtpCodecs[codecIndex]).getCodecType())
                  {
                     sendCodecFound = TRUE;
                     receiveCodecFound = TRUE;
                     *sendCodecIndex = codecIndex;
                     *receiveCodecIndex = codecIndex;
                     getMediaAddress(mediaIndex, rtpAddress);
                     getMediaPort(mediaIndex, rtpPort);
                     break;
                  }
               }
               if(sendCodecFound && receiveCodecFound)
               {
                  break;
               }
            }
         }
         mediaIndex++;
      }
   }
}

// version used by SipConnection class
void SdpBody::getBestAudioCodecs(SdpCodecFactory& localRtpCodecs,
                                 int& numCodecsInCommon,
                                 SdpCodec**& codecsInCommonArray,
                                 UtlString& rtpAddress,
                                 int& rtpPort,
                                 int& rtcpPort,
                                 int& videoRtpPort,
                                 int& videoRtcpPort) const
{

   int mediaAudioIndex = 0;
   int mediaVideoIndex = 0;
   int numAudioTypes;
   int numVideoTypes;
   int audioPayloadTypes[MAXIMUM_MEDIA_TYPES];
   int videoPayloadTypes[MAXIMUM_MEDIA_TYPES];
   numCodecsInCommon = 0;
   codecsInCommonArray = new SdpCodec*[localRtpCodecs.getCodecCount()];

   rtpAddress.remove(0);
   rtpPort = 0;
   while(mediaAudioIndex >= 0 || mediaVideoIndex >=0)
   {
      mediaAudioIndex = findMediaType(SDP_AUDIO_MEDIA_TYPE, mediaAudioIndex);
      mediaVideoIndex = findMediaType(SDP_VIDEO_MEDIA_TYPE, mediaVideoIndex);

      if(mediaAudioIndex >= 0)
      {
         // This is kind of a bad assumption if there is more
         // than one media field, each might have a different
         // port and address
         getMediaPort(mediaAudioIndex, &rtpPort);
         getMediaRtcpPort(mediaAudioIndex, &rtcpPort);
         getMediaAddress(mediaAudioIndex, &rtpAddress);

         if(rtpPort >= 0)
         {
            getMediaPayloadType(mediaAudioIndex, MAXIMUM_MEDIA_TYPES,
                                &numAudioTypes, audioPayloadTypes);

            getMediaPayloadType(mediaVideoIndex, MAXIMUM_MEDIA_TYPES,
                                &numVideoTypes, videoPayloadTypes);

            // This is to handle the case that a mis-formatted message could have codecCount
            // not matching medieType count, we want to make sure we allocated enough space for
            // the array to "forgive" or "tolerate" this case.
            int maxTypes = numAudioTypes > numVideoTypes ? numAudioTypes : numVideoTypes;
            if(localRtpCodecs.getCodecCount() < maxTypes)
            {
               delete [] codecsInCommonArray;
               codecsInCommonArray = new SdpCodec*[maxTypes];
               memset(codecsInCommonArray, 0, sizeof(SdpCodec*) * maxTypes);
            }
            getCodecsInCommon(numAudioTypes,
                              numVideoTypes,
                              audioPayloadTypes,
                              videoPayloadTypes,
                              localRtpCodecs,
                              numCodecsInCommon,
                              codecsInCommonArray);
            if (numCodecsInCommon >0)
            {
               break;
            }
         }
         mediaAudioIndex++;
         mediaVideoIndex++;
      }
   }
}


void SdpBody::getEncryptionInCommon(SdpSrtpParameters& audioParams,
                                    SdpSrtpParameters& videoParams,
                                    SdpSrtpParameters& commonAudioParams,
                                    SdpSrtpParameters& commonVideoParams)
{
    // Hard-coding the encryption suite we support - this has to become
    // more flexible.
    memset(&commonAudioParams, 0, sizeof(SdpSrtpParameters));
    memset(&commonVideoParams, 0, sizeof(SdpSrtpParameters));

    if (audioParams.cipherType == AES_CM_128_HMAC_SHA1_80)
    {
        memcpy(&commonAudioParams, &audioParams, sizeof(SdpSrtpParameters));
    }
    if (videoParams.cipherType == AES_CM_128_HMAC_SHA1_80)
    {
        memcpy(&commonVideoParams, &videoParams, sizeof(SdpSrtpParameters));
    }
}


void SdpBody::getCodecsInCommon(int audioPayloadIdCount,
                                int videoPayloadIdCount,
                                int audioPayloadTypes[],
                                int videoPayloadTypes[],
                                SdpCodecFactory& localRtpCodecs,
                                int& numCodecsInCommon,
                                SdpCodec* codecsInCommonArray[]) const
{
   UtlString mimeSubtype;
   UtlString fmtp;
   int sampleRate;
   int numChannels;
   const SdpCodec* matchingCodec = NULL;
   int typeIndex;
   bool matchingDtmf2833Found = FALSE;
   int dynamicPayloadTypes[audioPayloadIdCount + videoPayloadIdCount];
   int countDynPT = 0;
   bool preferDtmf2833PTused = FALSE;

   numCodecsInCommon = 0;

   // pick all the audio codecs
   for(typeIndex = 0; typeIndex < audioPayloadIdCount; typeIndex++)
   {
      // Until the real SdpCodec is needed we assume all of
      // the rtpCodecs are send AND receive.
      // We are also going to cheat and assume that all of
      // the media records are send AND receive

      // Get the rtpmap for the payload type
      if(getPayloadRtpMap(audioPayloadTypes[typeIndex],
                          mimeSubtype, sampleRate, numChannels))
      {
         // Find a match for the mime type
         matchingCodec = localRtpCodecs.getCodec(MIME_TYPE_AUDIO, mimeSubtype.data());
         if((matchingCodec != NULL)
            && (matchingCodec->getSampleRate() == sampleRate || sampleRate == -1)
            && (matchingCodec->getNumChannels() == numChannels || numChannels == -1))
         {
            // Create a copy of the SDP codec and set
            // the payload type for it
            codecsInCommonArray[numCodecsInCommon] = new SdpCodec(*matchingCodec);
            codecsInCommonArray[numCodecsInCommon]->setCodecPayloadFormat(audioPayloadTypes[typeIndex]);
            numCodecsInCommon++;
#ifdef TEST_PRINT

            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SdpBody::getCodecsInCommon "
                          "found matching dynamic audio codec %d [%d]",
                          audioPayloadTypes[typeIndex], typeIndex);
#endif
            // we will always send 2833 Dtmf codec, need to know whether to add later
            if (matchingDtmf2833Found == FALSE // only runs if telephone-event not already found
                && matchingCodec->getCodecType() == SdpCodec::SDP_CODEC_TONES)
            {
#ifdef TEST_PRINT
                OsSysLog::add(FAC_SIP, PRI_DEBUG,
                              "SdpBody::getCodecsInCommon "
                              "found dtmf telephone-event format %d [%d]",
                          audioPayloadTypes[typeIndex], typeIndex);
#endif
                matchingDtmf2833Found = TRUE;
            }
         }
      }

      // If no payload type set and this is a static payload
      // type assume the payload type is the same as our internal
      // codec id
      else if(audioPayloadTypes[typeIndex] <=
              SdpCodec::SDP_CODEC_MAXIMUM_STATIC_CODEC)
      {
         if((matchingCodec = localRtpCodecs.getCodecByType(audioPayloadTypes[typeIndex])))
         {
            // Create a copy of the SDP codec and set
            // the payload type for it
            codecsInCommonArray[numCodecsInCommon] = new SdpCodec(*matchingCodec);
            codecsInCommonArray[numCodecsInCommon]->setCodecPayloadFormat(audioPayloadTypes[typeIndex]);
            numCodecsInCommon++;
#ifdef TEST_PRINT
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SdpBody::getCodecsInCommon "
                          "found matching static audio codec %d [%d]",
                          audioPayloadTypes[typeIndex], typeIndex);
#endif
         }
      }
      // if we need to add 2833 Dtmf, have to remember dynamic payload types already used
      if (matchingDtmf2833Found == FALSE // only runs if telephone-event not offered
          && audioPayloadTypes[typeIndex] > SdpCodec::SDP_CODEC_MAXIMUM_STATIC_CODEC)
      {
          dynamicPayloadTypes[countDynPT] = audioPayloadTypes[typeIndex];
          countDynPT++;
          if (audioPayloadTypes[typeIndex] == SdpCodec::SDP_CODEC_DEFAULT_TONES_CODEC)
          {
              preferDtmf2833PTused = TRUE;
#ifdef TEST_PRINT
              OsSysLog::add(FAC_SIP, PRI_DEBUG,
                            "SdpBody::getCodecsInCommon "
                            "2833 default audio codec %d [%d]",
                            audioPayloadTypes[typeIndex], typeIndex);
#endif
          }
      }
   }

   // pick all the video codecs
   for(typeIndex = 0; typeIndex < videoPayloadIdCount; typeIndex++)
   {
       int videoFmtp = 0;
       SdpCodec** sdpCodecArray;
       int numCodecs;
       int codecIndex;
      // Until the real SdpCodec is needed we assume all of
      // the rtpCodecs are send AND receive.
      // We are also going to cheat and assume that all of
      // the media records are send AND receive

      // Get the rtpmap for the payload type
      if(getPayloadRtpMap(videoPayloadTypes[typeIndex],
                          mimeSubtype, sampleRate, numChannels))
      {
         // Get the video fomat bitmap in videoFmtp
         getPayloadFormat(videoPayloadTypes[typeIndex], fmtp, videoFmtp);

         // Get all codecs with the same mime subtype. Same codecs with different
         // video resolutions are added separately but have the same mime subtype.
         // The codec negotiation depends on the fact that codecs with the same
         // mime subtype are added sequentially.
         localRtpCodecs.getCodecs(numCodecs, sdpCodecArray, "video", mimeSubtype);

         for (codecIndex = 0; codecIndex < numCodecs; ++codecIndex)
         {
            matchingCodec = sdpCodecArray[codecIndex];

            // In addition to everything else do a bit-wise comparison of video formats. For
            // every codec with the same sub mime type that supports one of the video formats
            // add a separate codec in the codecsInCommonArray.
            if((matchingCodec != NULL) && (matchingCodec->getVideoFormat() & videoFmtp) &&
                (matchingCodec->getSampleRate() == sampleRate ||
                sampleRate == -1) &&
                (matchingCodec->getNumChannels() == numChannels ||
                numChannels == -1
                ))
            {
                // Create a copy of the SDP codec and set
                // the payload type for it
                codecsInCommonArray[numCodecsInCommon] = new SdpCodec(*matchingCodec);
                codecsInCommonArray[numCodecsInCommon]->setCodecPayloadFormat(videoPayloadTypes[typeIndex]);
                numCodecsInCommon++;
#ifdef TEST_PRINT
                OsSysLog::add(FAC_SIP, PRI_DEBUG,
                              "SdpBody::getCodecsInCommon "
                              "found matching dynamic video codec %d [%d]",
                              videoPayloadTypes[typeIndex], typeIndex);
#endif
            }
         }

         // Delete the codec array we got to loop through codecs with the same mime subtype
         for (codecIndex = 0; codecIndex < numCodecs; ++codecIndex)
         {
            if (sdpCodecArray[codecIndex])
            {
                delete sdpCodecArray[codecIndex];
                sdpCodecArray[codecIndex] = NULL;
            }
         }
         delete[] sdpCodecArray;
      }

      // If no payload type set and this is a static payload
      // type assume the payload type is the same as our internal
      // codec id
      else if(videoPayloadTypes[typeIndex] <=
              SdpCodec::SDP_CODEC_MAXIMUM_STATIC_CODEC)
      {
         if((matchingCodec = localRtpCodecs.getCodecByType(videoPayloadTypes[typeIndex])))
         {
            // Create a copy of the SDP codec and set
            // the payload type for it
            codecsInCommonArray[numCodecsInCommon] = new SdpCodec(*matchingCodec);
            codecsInCommonArray[numCodecsInCommon]->setCodecPayloadFormat(videoPayloadTypes[typeIndex]);
            numCodecsInCommon++;
         }
      }
      // if we need to add 2833 Dtmf, have to remember dynamic payload types already used
      if (matchingDtmf2833Found == FALSE // only runs if telephone-event not offered
          && videoPayloadTypes[typeIndex] > SdpCodec::SDP_CODEC_MAXIMUM_STATIC_CODEC)
      {
          dynamicPayloadTypes[countDynPT] = videoPayloadTypes[typeIndex];
          countDynPT++;
          if (videoPayloadTypes[typeIndex] == SdpCodec::SDP_CODEC_DEFAULT_TONES_CODEC)
          {
              preferDtmf2833PTused = TRUE;
#ifdef TEST_PRINT
              OsSysLog::add(FAC_SIP, PRI_DEBUG,
                            "SdpBody::getCodecsInCommon "
                            "2833 default audio codec %d [%d]",
                            videoPayloadTypes[typeIndex], typeIndex);
#endif
          }
      }
   }
   if (matchingDtmf2833Found == FALSE)
   {
       int useThisPT = SdpCodec::SDP_CODEC_DEFAULT_TONES_CODEC;
       int ptOuter, iInner;

       if(preferDtmf2833PTused == TRUE)
       {
           for (ptOuter = SdpCodec::SDP_CODEC_MAXIMUM_STATIC_CODEC +1;
                 ptOuter <= SdpCodec::SDP_CODEC_MAXIMUM_DYNAMIC_CODEC;
                 ptOuter++)
           {
               for (iInner = 0; iInner < countDynPT; iInner++)
               {
                   if (dynamicPayloadTypes[iInner] == countDynPT)
                   {
#ifdef TEST_PRINT
                       OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                     "SdpBody::getCodecsInCommon "
                                     "already using %d index [%d]",
                                     ptOuter, iInner);
#endif
                       break;
                   }
               }
               if (iInner == countDynPT )
               {
                   useThisPT = ptOuter;
#ifdef TEST_PRINT
                   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                 "SdpBody::getCodecsInCommon "
                                 "ok to use dyn pt %d index [%d]",
                                 ptOuter, iInner);
#endif
                   break;
               }
           }
           if (useThisPT == SdpCodec::SDP_CODEC_DEFAULT_TONES_CODEC)
           {
               useThisPT = 0;
               OsSysLog::add(FAC_SIP, PRI_WARNING,
                             "SdpBody::getCodecsInCommon "
                             "no free dynamic PT (%d used)",
                             countDynPT);
           }
       }
       if (useThisPT != 0)
       {
           codecsInCommonArray[numCodecsInCommon] = new SdpCodec(SdpCodec::SDP_CODEC_TONES,
                                                                 useThisPT,
                                                                 "audio",
                                                                 MIME_SUBTYPE_DTMF_TONES) ;

           numCodecsInCommon++;
           OsSysLog::add(FAC_SIP, PRI_DEBUG,
                         "SdpBody::getCodecsInCommon "
                         "add tel-event codec PT %d numCodecs %d",
                         useThisPT, numCodecsInCommon);
       }

   }
}

void SdpBody::addAudioCodecs(const char* rtpAddress, int rtpAudioPort,
                             int rtcpAudioPort, int rtpVideoPort,
                             int rtcpVideoPort, int numRtpCodecs,
                             SdpCodec* rtpCodecs[],
                             SdpSrtpParameters& srtpParams)
{
   int codecArray[MAXIMUM_MEDIA_TYPES];
   int formatArray[MAXIMUM_MEDIA_TYPES];
   UtlString videoFormat;
   int codecIndex;
   int destIndex;
   int firstMimeSubTypeIndex = -1; // this value is not used - prevent compiler warning
   int preExistingMedia = getMediaSetCount();
   UtlString mimeType;
   UtlString seenMimeType;
   UtlString mimeSubType;
   UtlString prevMimeSubType = "none";
   int numAudioCodecs=0;
   int numVideoCodecs=0;

   memset(formatArray, 0, sizeof (formatArray));

   // If there are no media fields we only need one global one
   // for the SDP body
   if(!preExistingMedia)
   {
      addAddressData(rtpAddress);
      char timeString[100];
      sprintf(timeString, "%d %d", 0, //OsDateTime::getSecsSinceEpoch(),
              0);
      addValue("t", timeString);
   }

   // Stuff the SDP audio codes in an integer array
   for(codecIndex = 0, destIndex = 0;
       codecIndex < MAXIMUM_MEDIA_TYPES && codecIndex < numRtpCodecs;
       codecIndex++)
   {
      rtpCodecs[codecIndex]->getMediaType(mimeType);
      if (mimeType.compareTo("audio") == 0 || mimeType.compareTo("video") != 0)
      {
         seenMimeType = mimeType;
         ++numAudioCodecs;
         codecArray[destIndex++] =
             (rtpCodecs[codecIndex])->getCodecPayloadFormat();
      }
   }

   if (rtpAudioPort != 0)
   {
      // If any security is enabled we set RTP/SAVP and add a crypto field
      if (srtpParams.securityLevel)
      {
           // Add the media record
           addMediaData(SDP_AUDIO_MEDIA_TYPE, rtpAudioPort, 1,
                        SDP_SRTP_MEDIA_TRANSPORT_TYPE, numAudioCodecs,
                        codecArray);
          addSrtpCryptoField(srtpParams);
      }
      else
      {
          // Add the media record
           addMediaData(SDP_AUDIO_MEDIA_TYPE, rtpAudioPort, 1,
                        SDP_RTP_MEDIA_TRANSPORT_TYPE, numAudioCodecs,
                        codecArray);
      }
      // It is assumed that rtcp is the odd port immediately after the rtp port.
      // If that is not true, we must add a parameter to specify the rtcp port.
      if ((rtcpAudioPort > 0) && ((rtcpAudioPort != rtpAudioPort + 1) || (rtcpAudioPort % 2) == 0))
      {
          char cRtcpBuf[32] ;
          sprintf(cRtcpBuf, "rtcp:%d", rtcpAudioPort) ;

          addValue("a", cRtcpBuf) ;
      }

      // add attribute records defining the extended types
      addCodecParameters(numRtpCodecs, rtpCodecs, seenMimeType.data());

      // If this is not the only media record we need a local
      // address record for this media record
      if(preExistingMedia)
      {
         addAddressData(rtpAddress);
      }
   }

   // Stuff the SDP video codecs codes in an integer array
   for(codecIndex = 0, destIndex = -1;
       codecIndex < MAXIMUM_MEDIA_TYPES && codecIndex < numRtpCodecs;
       codecIndex++)
   {
      rtpCodecs[codecIndex]->getMediaType(mimeType);

      if (mimeType.compareTo("video") == 0)
      {
         rtpCodecs[codecIndex]->getEncodingName(mimeSubType);

         if (mimeSubType.compareTo(prevMimeSubType) == 0)
         {
            // If we still have the same mime type only change format. We're depending on the
            // fact that codecs with the same mime subtype are added sequentially to the
            // codec factory. Otherwise this won't work.
            formatArray[destIndex] |= (rtpCodecs[codecIndex])->getVideoFormat();
            // Note that if prevMimeSubType matches mimeSubType, then firstMimeSubTypeIndex
            // must have been given a value, also.
            (rtpCodecs[firstMimeSubTypeIndex])->setVideoFmtp(formatArray[destIndex]);
         }
         else
         {
            // New mime subtype - add new codec to codec list. Mark this index and put all
            // video format information into this codec because it will be looked at later.
            firstMimeSubTypeIndex = codecIndex;
            ++destIndex;
            prevMimeSubType = mimeSubType;
            ++numVideoCodecs;
            formatArray[destIndex] = (rtpCodecs[codecIndex])->getVideoFormat();
            codecArray[destIndex] =
                      (rtpCodecs[codecIndex])->getCodecPayloadFormat();
            (rtpCodecs[firstMimeSubTypeIndex])->setVideoFmtp(formatArray[destIndex]);
         }
      }
   }

   if (rtpVideoPort != 0)
   {
      // If any security is enabled we set RTP/SAVP and add a crypto field
      if (srtpParams.securityLevel)
      {
         // Add the media record
         addMediaData(SDP_VIDEO_MEDIA_TYPE, rtpVideoPort, 1,
                      SDP_SRTP_MEDIA_TRANSPORT_TYPE, numVideoCodecs,
                      codecArray);
         addSrtpCryptoField(srtpParams);
      }
      else
      {
         // Add the media record
         addMediaData(SDP_VIDEO_MEDIA_TYPE, rtpVideoPort, 1,
                      SDP_RTP_MEDIA_TRANSPORT_TYPE, numVideoCodecs,
                      codecArray);
      }
      // It is assumed that rtcp is the odd port immediately after the rtp port.
      // If that is not true, we must add a parameter to specify the rtcp port.
      if ((rtcpVideoPort > 0) && ((rtcpVideoPort != rtpVideoPort + 1) || (rtcpVideoPort % 2) == 0))
      {
          char cRtcpBuf[32] ;
          sprintf(cRtcpBuf, "rtcp:%d", rtcpVideoPort) ;

          addValue("a", cRtcpBuf) ;
      }

      // add attribute records defining the extended types
      addCodecParameters(numRtpCodecs, rtpCodecs, "video");

      // If this is not the only media record we need a local
      // address record for this media record
      if(preExistingMedia)
      {
         addAddressData(rtpAddress);
      }
   }
}

void SdpBody::addCodecParameters(int numRtpCodecs,
                                 SdpCodec* rtpCodecs[],
                                 const char *szMimeType)
{
   const SdpCodec* codec = NULL;
   UtlString mimeSubtype;
   int payloadType;
   int sampleRate;
   int numChannels;
   int videoFmtp;
   UtlString formatParameters;
   UtlString mimeType;
   UtlString prevMimeSubType = "none";
   UtlString formatTemp;

   for(int codecIndex = 0;
       codecIndex < MAXIMUM_MEDIA_TYPES && codecIndex < numRtpCodecs;
       codecIndex++)
   {
      codec = rtpCodecs[codecIndex];
      rtpCodecs[codecIndex]->getMediaType(mimeType);
      if(codec && mimeType.compareTo(szMimeType) == 0)
      {
         codec->getEncodingName(mimeSubtype);
         // Only add to map if we have a new mime sub type
         if (mimeSubtype.compareTo(prevMimeSubType) != 0)
         {
            prevMimeSubType = mimeSubtype;
            sampleRate = codec->getSampleRate();
            numChannels = codec->getNumChannels();
            codec->getSdpFmtpField(formatParameters);
            payloadType = codec->getCodecPayloadFormat();

            // Build an rtpmap
            addRtpmap(payloadType, mimeSubtype.data(),
                       sampleRate, numChannels);

            if ((videoFmtp=codec->getVideoFmtp()) != 0)
            {
                formatTemp = "size:";
                if (videoFmtp & SDP_VIDEO_FORMAT_CIF)
                {
                    formatTemp.append("CIF/");
                }
                if (videoFmtp & SDP_VIDEO_FORMAT_QCIF)
                {
                    formatTemp.append("QCIF/");
                }
                if (videoFmtp & SDP_VIDEO_FORMAT_SQCIF)
                {
                    formatTemp.append("SQCIF/");
                }
                formatParameters = formatTemp(0, formatTemp.length()-1);
            }

            // Add the format specific parameters if present
            if(!formatParameters.isNull())
            {
                addFormatParameters(payloadType,
                                    formatParameters.data());
            }
         }
      }
   }
}

void SdpBody::addAudioCodecs(const char* rtpAddress, int rtpAudioPort,
                             int rtcpAudioPort, int rtpVideoPort,
                             int rtcpVideoPort, int numRtpCodecs,
                             SdpCodec* rtpCodecs[], SdpSrtpParameters& srtpParams,
                             const SdpBody* sdpRequest)
{
   int preExistingMedia = getMediaSetCount();
   int mediaIndex = 0;
   UtlBoolean fieldFound = TRUE;
   UtlBoolean commonVideo = FALSE;
   UtlString mediaType;
   int mediaPort, audioPort, videoPort;
   int mediaPortPairs, audioPortPairs, videoPortPairs;
   SdpDirectionality mediaDirectionality, audioDirectionality, videoDirectionality;
   UtlString mediaTransportType, audioTransportType, videoTransportType;
   int numPayloadTypes, numAudioPayloadTypes, numVideoPayloadTypes;
   int payloadTypes[MAXIMUM_MEDIA_TYPES];
   int audioPayloadTypes[MAXIMUM_MEDIA_TYPES];
   int videoPayloadTypes[MAXIMUM_MEDIA_TYPES];
   int supportedPayloadTypes[MAXIMUM_MEDIA_TYPES];
   int formatArray[MAXIMUM_MEDIA_TYPES];
   SdpCodec* codecsInCommon[MAXIMUM_MEDIA_TYPES];
   int supportedPayloadCount;
   int destIndex;
   int firstMimeSubTypeIndex = -1; // this value is not used - prevent compiler warning
   SdpSrtpParameters receivedSrtpParams;
   SdpSrtpParameters receivedAudioSrtpParams;
   SdpSrtpParameters receivedVideoSrtpParams;
   UtlString prevMimeSubType = "none";
   UtlString mimeSubType;

   memset(formatArray, 0, sizeof(int)*MAXIMUM_MEDIA_TYPES);
   memset(&receivedSrtpParams,0, sizeof(SdpSrtpParameters));
   // if there are no media fields already, add a global
   // address field
   if(!preExistingMedia)
   {
      addAddressData(rtpAddress);
      char timeString[100];
      sprintf(timeString, "%d %d", 0, //OsDateTime::getSecsSinceEpoch(),
              0);
      addValue("t", timeString);
   }

   numPayloadTypes = 0 ;
   memset(&payloadTypes, 0, sizeof(int) * MAXIMUM_MEDIA_TYPES) ;

   audioPort = 0 ;
   audioPortPairs = 0 ;
   numAudioPayloadTypes = 0 ;
   memset(&audioPayloadTypes, 0, sizeof(int) * MAXIMUM_MEDIA_TYPES) ;
   memset(&receivedAudioSrtpParams,0 , sizeof(SdpSrtpParameters));
   audioDirectionality = sdpDirectionalitySendRecv;

   videoPort = 0 ;
   videoPortPairs = 0 ;
   numVideoPayloadTypes = 0 ;
   memset(&videoPayloadTypes, 0, sizeof(int) * MAXIMUM_MEDIA_TYPES) ;
   memset(&receivedVideoSrtpParams,0 , sizeof(SdpSrtpParameters));
   videoDirectionality = sdpDirectionalitySendRecv;

   // Loop through the fields in the sdpRequest
   while(fieldFound)
   {
      fieldFound = sdpRequest->getMediaData(mediaIndex, &mediaType,
                                            &mediaPort, &mediaPortPairs,
                                            &mediaTransportType,
                                            MAXIMUM_MEDIA_TYPES, &numPayloadTypes,
                                            payloadTypes, &mediaDirectionality);

      if(fieldFound)
      {
         // Check for unsupported stuff
         // i.e. non audio media, non RTP transported media, etc
         if((strcmp(mediaType.data(), SDP_AUDIO_MEDIA_TYPE) != 0 &&
            strcmp(mediaType.data(), SDP_VIDEO_MEDIA_TYPE) != 0) ||
            mediaPort <= 0 || mediaPortPairs <= 0 ||
            (strcmp(mediaTransportType.data(), SDP_RTP_MEDIA_TRANSPORT_TYPE) != 0 &&
             strcmp(mediaTransportType.data(), SDP_SRTP_MEDIA_TRANSPORT_TYPE) != 0))
         {
            mediaPort = 0;
            addMediaData(mediaType.data(),
                         mediaPort, mediaPortPairs,
                         mediaTransportType.data(),
                         numPayloadTypes,
                         payloadTypes);
            addAttribute(sdpDirectionalityStrings[sdpDirectionalityInactive]);
         }

         // Copy media fields and replace the port with:
         // rtpPort if one or more of the codecs are supported
         //       removing the unsupported codecs
         // zero if none of the codecs are supported
         else
         {
            sdpRequest->getSrtpCryptoField(mediaIndex, 1, receivedSrtpParams);
            if (mediaType.compareTo("audio") == 0)
            {
                audioPort = mediaPort;
                audioPortPairs = mediaPortPairs;
                audioTransportType = mediaTransportType;
                numAudioPayloadTypes = numPayloadTypes;
                memcpy(&audioPayloadTypes, &payloadTypes, sizeof(int)*MAXIMUM_MEDIA_TYPES);
                memcpy(&receivedAudioSrtpParams, &receivedSrtpParams, sizeof(SdpSrtpParameters));
                audioDirectionality = mediaDirectionality;
            }
            else
            {
                videoPort = mediaPort;
                videoPortPairs = mediaPortPairs;
                videoTransportType = mediaTransportType;
                numVideoPayloadTypes = numPayloadTypes;
                memcpy(&videoPayloadTypes, &payloadTypes, sizeof(int)*MAXIMUM_MEDIA_TYPES);
                memcpy(&receivedVideoSrtpParams, &receivedSrtpParams, sizeof(SdpSrtpParameters));
                videoDirectionality = mediaDirectionality;
            }
         }
      }
      mediaIndex++;
   }

   SdpCodecFactory codecFactory(numRtpCodecs,
                                rtpCodecs);

   supportedPayloadCount = 0;
   sdpRequest->getCodecsInCommon(numAudioPayloadTypes, numVideoPayloadTypes,
                                 audioPayloadTypes, videoPayloadTypes,
                                 codecFactory, supportedPayloadCount,
                                 codecsInCommon);

   SdpSrtpParameters commonAudioSrtpParams;
   SdpSrtpParameters commonVideoSrtpParams;
   memset(&commonAudioSrtpParams,0 , sizeof(SdpSrtpParameters));
   memset(&commonVideoSrtpParams,0 , sizeof(SdpSrtpParameters));
   getEncryptionInCommon(receivedAudioSrtpParams, receivedVideoSrtpParams,
                         commonAudioSrtpParams, commonVideoSrtpParams);


    // Add the modified list of supported codecs
    if(supportedPayloadCount)
    {
        // Do this for audio first
        destIndex = 0;
        int payloadIndex = 0;
        for(payloadIndex = 0;
            payloadIndex < supportedPayloadCount;
            payloadIndex++)
        {
            codecsInCommon[payloadIndex]->getMediaType(mediaType);
            if (mediaType.compareTo("audio") == 0)
            {
                supportedPayloadTypes[destIndex++] =
                        codecsInCommon[payloadIndex]->getCodecPayloadFormat();
            }
        }
        mediaPort = rtpAudioPort;
        mediaPortPairs = 1;
        addMediaData(SDP_AUDIO_MEDIA_TYPE,
                    mediaPort, mediaPortPairs,
                    audioTransportType.data(),
                    destIndex,
                    supportedPayloadTypes);
        addAttribute(sdpDirectionalityStrings[
                        sdpDirectionalityReverse[audioDirectionality]]);
        if (commonAudioSrtpParams.securityLevel)
        {
            addSrtpCryptoField(commonAudioSrtpParams);
        }

        if (strcmp(audioTransportType.data(), SDP_RTP_MEDIA_TRANSPORT_TYPE) == 0)
        {
            // It is assumed that rtcp is the odd port immediately after
            // the rtp port.  If that is not true, we must add a parameter
            // to specify the rtcp port.
            if ((rtcpAudioPort > 0) && ((rtcpAudioPort != rtpAudioPort + 1) || (rtcpAudioPort % 2) == 0))
            {
                char cRtcpBuf[32] ;
                sprintf(cRtcpBuf, "rtcp:%d", rtcpAudioPort) ;
                addValue("a", cRtcpBuf) ;
            }
        }

        addCodecParameters(supportedPayloadCount,
                            codecsInCommon, "audio");

        // Then do this for video
        destIndex = -1;
        for(payloadIndex = 0;
            payloadIndex < supportedPayloadCount;
            payloadIndex++)
        {
            codecsInCommon[payloadIndex]->getMediaType(mediaType);
            if (mediaType.compareTo("video") == 0)
            {
                // We've found at least one common video codec
                commonVideo = TRUE;
                codecsInCommon[payloadIndex]->getEncodingName(mimeSubType);

                // If we still have the same mime type only change format. We're depending on the
                // fact that codecs with the same mime subtype are added sequentially to the
                // codec factory. Otherwise this won't work.
                if (prevMimeSubType.compareTo(mimeSubType) == 0)
                {
                    formatArray[destIndex] |= (codecsInCommon[payloadIndex])->getVideoFormat();
                    // Note that if prevMimeSubType matches mimeSubType, then firstMimeSubTypeIndex
                    // must have been given a value, also.
                    (codecsInCommon[firstMimeSubTypeIndex])->setVideoFmtp(formatArray[destIndex]);
                }
                else
                {
                    ++destIndex;
                    prevMimeSubType = mimeSubType;
                    firstMimeSubTypeIndex = payloadIndex;
                    formatArray[destIndex] = (codecsInCommon[payloadIndex])->getVideoFormat();
                    supportedPayloadTypes[destIndex] =
                        codecsInCommon[firstMimeSubTypeIndex]->getCodecPayloadFormat();
                    (codecsInCommon[firstMimeSubTypeIndex])->setVideoFmtp(formatArray[destIndex]);
                }

            }
        }
        // Only add m-line if we actually have common video codecs
        if (commonVideo)
        {
            mediaPort = rtpVideoPort;
            mediaPortPairs = 1;
            addMediaData(SDP_VIDEO_MEDIA_TYPE,
                        mediaPort, mediaPortPairs,
                        videoTransportType.data(),
                        destIndex+1,
                        supportedPayloadTypes);
            addAttribute(sdpDirectionalityStrings[
                            sdpDirectionalityReverse[videoDirectionality]]);
            if (commonVideoSrtpParams.securityLevel)
            {
                addSrtpCryptoField(commonAudioSrtpParams);
            }

            if (strcmp(videoTransportType.data(), SDP_RTP_MEDIA_TRANSPORT_TYPE) == 0)
            {
                // It is assumed that rtcp is the odd port immediately after
                // the rtp port.  If that is not true, we must add a parameter
                // to specify the rtcp port.
                if ((rtcpVideoPort > 0) && ((rtcpVideoPort != rtpVideoPort + 1) || (rtcpVideoPort % 2) == 0))
                {
                    char cRtcpBuf[32] ;
                    sprintf(cRtcpBuf, "rtcp:%d", rtcpVideoPort) ;
                    addValue("a", cRtcpBuf) ;
                }
            }

            addCodecParameters(supportedPayloadCount,
                                codecsInCommon, "video");
        }
    }

    // Zero out the port to indicate none are supported
    else
    {
        mediaPort = 0;
        addMediaData("audio",
                    mediaPort, audioPortPairs,
                    audioTransportType.data(),
                    numAudioPayloadTypes,
                    audioPayloadTypes);
        addAttribute(sdpDirectionalityStrings[sdpDirectionalityInactive]);
        addMediaData("video",
                    mediaPort, videoPortPairs,
                    videoTransportType.data(),
                    numVideoPayloadTypes,
                    videoPayloadTypes);
        addAttribute(sdpDirectionalityStrings[sdpDirectionalityInactive]);
    }

    // Free up the codec copies
    if (codecsInCommon != NULL) {
        for(int codecIndex = 0; codecIndex < supportedPayloadCount; codecIndex++)
        {
            delete codecsInCommon[codecIndex];
            codecsInCommon[codecIndex] = NULL;
        }
    }


    if(preExistingMedia)
    {
       addAddressData(rtpAddress);
    }

    // Copy all atribute fields verbatim
    // someday
}

void SdpBody::addRtpmap(int payloadType,
                        const char* mimeSubtype,
                        int sampleRate,
                        int numChannels)
{
   UtlString fieldValue("rtpmap:");
   char buffer[256];
   sprintf(buffer, "%d %s/%d", payloadType, mimeSubtype,
           sampleRate);

   fieldValue.append(buffer);

   if(numChannels > 0)
   {
      sprintf(buffer, "/%d", numChannels);
      fieldValue.append(buffer);
   }

   // Add the "a" field
   addValue("a", fieldValue.data());
}


void SdpBody::addSrtpCryptoField(SdpSrtpParameters& params)
{
    UtlString fieldValue("crypto:1 ");

    switch (params.cipherType)
    {
    case AES_CM_128_HMAC_SHA1_80:
        fieldValue.append("AES_CM_128_HMAC_SHA1_80 ");
        break;
    case AES_CM_128_HAMC_SHA1_32:
        fieldValue.append("AES_CM_128_HAMC_SHA1_32 ");
        break;
    case F8_128_HMAC_SHA1_80:
        fieldValue.append("F8_128_HMAC_SHA1_80 ");
        break;
    default: break;
    }
    fieldValue.append("inline:");

    // Base64-encode key string
    UtlString base64Key;
    NetBase64Codec::encode(SRTP_KEY_LENGTH, (char*)params.masterKey, base64Key);

    // Remove padding
    while (base64Key(base64Key.length()-1) == '=')
    {
        base64Key = base64Key(0, base64Key.length()-1);
    }
    fieldValue.append(base64Key);

    if (!(params.securityLevel & SRTP_ENCRYPTION))
    {
        fieldValue.append(" UNENCRYPTED_SRTP");
    }
    if (!(params.securityLevel & SRTP_AUTHENTICATION))
    {
        fieldValue.append(" UNAUTHENTICATED_SRTP");
    }

   // Add the "a" field for the crypto attribute
   addValue("a", fieldValue.data());
}


void SdpBody::addFormatParameters(int payloadType,
                                  const char* formatParameters)
{
   // Build "a" field:
   // "a=fmtp <payloadFormat> <formatParameters>"
   UtlString fieldValue("fmtp:");
   char buffer[100];
   sprintf(buffer, "%d ", payloadType);
   fieldValue.append(buffer);
   fieldValue.append(formatParameters);


   // Add the "a" field
   addValue("a", fieldValue.data());
}

void SdpBody::addCandidateAttribute(const char* id,
                                    double qValue,
                                    const char* userFrag,
                                    const char* password,
                                    const char* unicastIp,
                                    int unicastPort,
                                    const char* candidateIp,
                                    int candidatePort)
{
    UtlString attributeData ;
    char buffer[64] ;

    attributeData.append("candidate:") ;

    attributeData.append(id) ;
    attributeData.append(" ") ;

    sprintf(buffer, "%.1f", qValue) ;
    attributeData.append(buffer) ;
    attributeData.append(" ") ;

    attributeData.append(userFrag) ;
    attributeData.append(" ") ;

    attributeData.append(password) ;
    attributeData.append(" ") ;

    attributeData.append(unicastIp) ;
    attributeData.append(" ") ;

    sprintf(buffer, "%d", unicastPort) ;
    attributeData.append(buffer) ;
    attributeData.append(" ") ;

    attributeData.append(candidateIp) ;
    attributeData.append(" ") ;

    sprintf(buffer, "%d", candidatePort) ;
    attributeData.append(buffer) ;

    addValue("a", attributeData) ;
}


UtlBoolean SdpBody::getCandidateAttribute(int index,
                                          UtlString& rId,
                                          double& rQvalue,
                                          UtlString& rUserFrag,
                                          UtlString& rPassword,
                                          UtlString& rUnicastIp,
                                          int& rUnicastPort,
                                          UtlString& rCandidateIp,
                                          int& rCandidatePort) const
{
    UtlBoolean found = FALSE;
    UtlSListIterator iterator(*sdpFields);
    NameValuePair* nv = NULL;
    int aFieldIndex = 0;
    const char* value;
    UtlString aFieldMatch("a");
    UtlString aFieldType ;

    while((nv = (NameValuePair*) iterator.findNext(&aFieldMatch)) != NULL)
    {
        value =  nv->getValue();

        // Verify this is an candidate "a" record
        UtlTokenizer tokenizer(value) ;
        if (tokenizer.next(aFieldType, ":"))
        {
            aFieldType.toLower() ;
            aFieldType.strip(UtlString::both, ' ') ;
            if(aFieldType.compareTo("candidate") == 0)
            {
                if (aFieldIndex == index)
                {
                    UtlString tmpQvalue ;
                    UtlString tmpUnicastPort ;
                    UtlString tmpCandidatePort ;

                    // candidate:id qValue userFrag password Ip port candidateIp candidatePort
                    if (    tokenizer.next(rId, " \t")  &&
                            tokenizer.next(tmpQvalue, " \t") &&
                            tokenizer.next(rUserFrag, " \t") &&
                            tokenizer.next(rPassword, " \t") &&
                            tokenizer.next(rUnicastIp, " \t") &&
                            tokenizer.next(tmpUnicastPort, " \t") &&
                            tokenizer.next(rCandidateIp, " \t") &&
                            tokenizer.next(tmpCandidatePort, " \t"))
                    {
                        rQvalue = atof(tmpQvalue) ;
                        rUnicastPort = atoi(tmpUnicastPort) ;
                        rCandidatePort = atoi(tmpCandidatePort) ;

                        // Strip leading : from id -- is this a UtlTokenizer Bug??
                        rId.strip(UtlString::leading, ':') ;
                        found = TRUE;
                        break;
                    }
                }
                aFieldIndex++ ;
            }
        }
    }

    return(found) ;
}

void SdpBody::addMediaData(const char* mediaType,
                           int portNumber, int portPairCount,
                           const char* mediaTransportType,
                           int numPayloadTypes,
                           int payloadTypes[])
{
   UtlString value;
   char integerString[MAXIMUM_LONG_INT_CHARS];
   int codecIndex;

   // Media type (i.e. audio, application or video)
   value.append(mediaType);
   value.append(SDP_SUBFIELD_SEPARATOR);

   // Port and optional number of port pairs
   sprintf(integerString, "%d", portNumber);
   value.append(integerString);
   if(portPairCount > 1)
   {
      sprintf(integerString, "%d", portPairCount);
      value.append("/");
      value.append(integerString);
   }
   value.append(SDP_SUBFIELD_SEPARATOR);

   // Media transport type
   value.append(mediaTransportType);
   //value.append(SDP_SUBFIELD_SEPARATOR);

   for(codecIndex = 0; codecIndex < numPayloadTypes; codecIndex++)
   {
      // Media payload type (i.e. alaw, mlaw, etc.)
      sprintf(integerString, "%c%d", SDP_SUBFIELD_SEPARATOR,
              payloadTypes[codecIndex]);
      value.append(integerString);
   }

   addValue("m", value.data());
}

void SdpBody::addAttribute( const char* pAttributeName,
                            const char* pAttributeValue )
{
   if( pAttributeName )
   {
      UtlString attributeToAdd( pAttributeName );
      if( pAttributeValue )
      {
         attributeToAdd.append(':');
         attributeToAdd.append( pAttributeValue );
      }
      addValue( "a", attributeToAdd.data() );
   }
}

bool SdpBody::insertMediaAttribute(int mediaIndex,
                                   const char* pAttributeName,
                                   const char* pAttributeValue )
{
   bool bInsertionPointFound = false;

   if( pAttributeName )
   {
      UtlSListIterator iterator( *sdpFields );

      if( mediaIndex >= 0 )
      {
         if( mediaIndex >= getMediaSetCount() || mediaIndex < 0 )
         {
            return false;
         }
         else
         {
            positionFieldInstance( mediaIndex, &iterator, "m" );
         }
      }

      // search for attribute insertion point
      while( !bInsertionPointFound )
      {
         const NameValuePair* nv = (NameValuePair*) iterator.peekAtNext();
         if( nv == 0 || nv->compareTo( "m" ) == 0 )
         {
            bInsertionPointFound = true;
         }
         else
         {
            iterator();
         }

      }
      if( bInsertionPointFound )
      {
         UtlString attributeToAdd( pAttributeName );
         if( pAttributeValue )
         {
            attributeToAdd.append(':');
            attributeToAdd.append( pAttributeValue );
         }
         NameValuePair* nv = new NameValuePair("a", attributeToAdd );
         iterator.insertAfterPoint( nv );
      }
   }
   return bInsertionPointFound;
}

/// Adds an attribute to an existing media description
bool SdpBody::removeMediaAttribute(int mediaIndex,
                                   const char* pAttributeName )
{
   bool bAttributeRemoved = false;
   if( pAttributeName )
   {
      NameValuePair* nv;
      nv = getRefToMediaAttributeByName( mediaIndex, pAttributeName );
      if( nv )
      {
         if( sdpFields->removeReference( nv ) )
         {
            bAttributeRemoved = true;
         }
      }
   }
   return bAttributeRemoved;
}

/// Modify IP address for the indicated media stream.
bool SdpBody::modifyMediaAddress(int mediaIndex,
                                 const char* pAddress )
{
   UtlSListIterator iterator( *sdpFields );
   bool bAddressSet = false;

   if( pAddress )
   {
      NameValuePair* nv;
      if( mediaIndex == -1 || ( mediaIndex < getMediaSetCount() && mediaIndex >= 0 ) )
      {
         if( mediaIndex != -1 )
         {
            positionFieldInstance( mediaIndex, &iterator, "m" );
         }
         nv = findFieldNameBefore( &iterator, "c", "m" );
         if(nv)
         {
            // c= line found, modify it.
            UtlString value = nv->getValue();
            bAddressSet = modifySdpSubfieldValue( value, 2, pAddress );
            nv->setValue( value );
         }
         else
         {
            UtlString cLineToAdd;
            cLineToAdd.append("IN IP4 ");
            cLineToAdd.append( pAddress );

            // reseed iterator since findFieldNameBefore() modifies it even when no matches are found
            iterator.reset();
            if( mediaIndex != -1 )
            {
               positionFieldInstance( mediaIndex, &iterator, "m" );
            }
            NameValuePair* newNv = new NameValuePair("c", cLineToAdd );

            while( ( nv = (NameValuePair*) iterator.peekAtNext() ) != 0 )
            {
               if( strcspn( nv->data(), "bzkatrm" ) == 0 )
               {
                  // next element is one of those elements that has to come after
                  // the c= line according to RFC4566.  This means that we found our
                  // insertion point.  Add our c= line here..
                  break;
               }
               else
               {
                  // next element is a line that must come before the c= line that
                  // we are trying to insert.  Keep looking of an insertion point
                  iterator();
               }
            }
            iterator.insertAfterPoint( newNv );
         }
         bAddressSet = true;
      }
   }
   return bAddressSet;
}

/// Set the port number for the indicated media stream.
bool SdpBody::modifyMediaPort(int mediaIndex, ///< which media description set to modify
                              int port)
{
   UtlSListIterator iterator( *sdpFields );
   bool bPortSet = false;

   if( mediaIndex < getMediaSetCount() && mediaIndex >= 0 )
   {
      NameValuePair* nv;
      nv = positionFieldInstance( mediaIndex, &iterator, "m" );
      if(nv)
      {
         // m= line found, modify it.
         char portText[14];
         sprintf( portText, "%d", port );
         UtlString value = nv->getValue();
         bPortSet = modifySdpSubfieldValue( value, 1, portText );
         nv->setValue( value );
      }
   }
   return bPortSet;
}

void SdpBody::addAddressData(const char* ipAddress)
{
   const char* networkType = SDP_NETWORK_TYPE;
   const char* addressType = SDP_IP4_ADDRESS_TYPE;
   addAddressData(networkType, addressType, ipAddress);
}

void SdpBody::addAddressData(const char* networkType, const char* addressType, const char* ipAddress)
{
   UtlString value;

   value.append(networkType);
   value.append(SDP_SUBFIELD_SEPARATOR);
   value.append(addressType);
   value.append(SDP_SUBFIELD_SEPARATOR);
   value.append(ipAddress);

   addValue("c", value.data());
}

void SdpBody::addValue(const char* name, const char* value, ssize_t fieldIndex)
{
   NameValuePair* nv = new NameValuePair(name, value);
   if(UTL_NOT_FOUND == fieldIndex)
   {
      sdpFields->append(nv);
   }
   else
   {
      sdpFields->insertAt(fieldIndex, nv);
   }
}

void SdpBody::addEpochTime(unsigned long epochStartTime, unsigned long epochEndTime)
{
   epochStartTime += NTP_TO_EPOCH_DELTA;
   if(epochEndTime != 0) // zero means unbounded in sdp, so don't convert it
   {
      epochEndTime += NTP_TO_EPOCH_DELTA;
   }
   addNtpTime(epochStartTime, epochEndTime);
}

void SdpBody::addNtpTime(unsigned long ntpStartTime, unsigned long ntpEndTime)
{
   char integerString[MAXIMUM_LONG_INT_CHARS];
   UtlString value;

   sprintf(integerString, "%lu", ntpStartTime);
   value.append(integerString);
   value.append(SDP_SUBFIELD_SEPARATOR);
   sprintf(integerString, "%lu", ntpEndTime);
   value.append(integerString);

   ssize_t timeLocation = findFirstOf("zkam");
   addValue("t", value.data(), timeLocation);
}

void SdpBody::setOriginator(const char* userId, int sessionId, int sessionVersion, const char* address)
{
   char integerString[MAXIMUM_LONG_INT_CHARS];
   UtlString value;

   const char* networkType = SDP_NETWORK_TYPE;
   const char* addressType = SDP_IP4_ADDRESS_TYPE;

   //o=<username> <session id> <version> <network type> <address type> <address>
   // o=mhandley 2890844526 2890842807 IN IP4 126.16.64.4

   value.append(userId);
   value.append(SDP_SUBFIELD_SEPARATOR);
   sprintf(integerString, "%d", sessionId);
   value.append(integerString);
   value.append(SDP_SUBFIELD_SEPARATOR);
   sprintf(integerString, "%d", sessionVersion);
   value.append(integerString);
   value.append(SDP_SUBFIELD_SEPARATOR);
   value.append(networkType);
   value.append(SDP_SUBFIELD_SEPARATOR);
   value.append(addressType);
   value.append(SDP_SUBFIELD_SEPARATOR);
   value.append(address);

   setValue("o", value.data());
}

ssize_t SdpBody::getLength() const
{
   UtlSListIterator iterator(*sdpFields);
   NameValuePair* nv = NULL;
   const char* value;
   ssize_t length = 0;

   while((nv = dynamic_cast<NameValuePair*>(iterator())))
   {
      value = nv->getValue();
      if(value)
      {
         length += (  nv->length()
                    + 3 // SDP_NAME_VALUE_DELIMITOR + CARRIAGE_RETURN_NEWLINE
                    + strlen(value)
                    );
      }
      else if (!isOptionalField(nv->data()))
      {
         // not optional, so append it even if empty
         length += (  nv->length()
                    + 3 // SDP_NAME_VALUE_DELIMITOR + CARRIAGE_RETURN_NEWLINE
                    );
      }
   }
   return(length);
}

void SdpBody::getBytes(const char** bytes, ssize_t* length) const
{
   // This version of getBytes exists so that a caller who is
   // calling this method through an HttpBody will get the right
   // thing - we fill in the mBody string and then return that.
   UtlString tempBody;
   getBytes( &tempBody, length );
   ((SdpBody*)this)->mBody = tempBody.data();
   *bytes = mBody.data();
}

void SdpBody::getBytes(UtlString* bytes, ssize_t* length) const
{
   UtlSListIterator iterator(*sdpFields);
   NameValuePair* nv = NULL;
   const char* value;
   bytes->remove(0);
   while((nv = dynamic_cast<NameValuePair*>(iterator())))
   {
      value = nv->getValue();
      if(value)
      {
         bytes->append(nv->data());
         bytes->append(SDP_NAME_VALUE_DELIMITOR);
         bytes->append(value);
         bytes->append(CARRIAGE_RETURN_NEWLINE);
      }
      else if (!isOptionalField(nv->data()))
      {
         // not optional, so append it even if empty
         bytes->append(nv->data());
         bytes->append(SDP_NAME_VALUE_DELIMITOR);
         bytes->append(CARRIAGE_RETURN_NEWLINE);
      }
   }

   *length = bytes->length();
}

int SdpBody::getFieldCount() const
{
   return(sdpFields->entries());
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


bool SdpBody::isOptionalField(const char* name) const
{
   const UtlString OptionalStandardFields("iuepcbzkar");

   return OptionalStandardFields.index(name) != UtlString::UTLSTRING_NOT_FOUND;
}

ssize_t SdpBody::findFirstOf( const char* headers )
{
   ssize_t first = UTL_NOT_FOUND;
   ssize_t found;
   size_t lookingFor;
   size_t headersToTry;

   for ( lookingFor = 0, headersToTry = strlen(headers);
         lookingFor < headersToTry;
         lookingFor++
        )
   {
      char thisHeader[2];
      thisHeader[0] = headers[lookingFor];
      thisHeader[1] = '\0';
      NameValuePair header(thisHeader,NULL);

      found = sdpFields->index(&header);
      if ( UTL_NOT_FOUND != found )
      {
         first = (UTL_NOT_FOUND == first) ? found : first > found ? found : first;
      }
   }

   return first;
}

NameValuePair* SdpBody::positionFieldInstance(int fieldInstanceIndex,
                                              UtlSListIterator* iter,
                                              const char* fieldName)
{
   UtlContainable* nv = NULL;
   if ( fieldInstanceIndex >= 0 )
   {
      NameValuePair fieldNV(fieldName);
      iter->reset();

      int index = 0;
      nv = iter->findNext(&fieldNV);
      while(nv && index < fieldInstanceIndex)
      {
         nv = iter->findNext(&fieldNV);
         index++;
      }
   }

   return((NameValuePair*) nv);
}

NameValuePair* SdpBody::findFieldNameBefore(UtlSListIterator* iter,
                                            const char* targetFieldName,
                                            const char* beforeFieldName)
{
   // Find a default address if one exist
   NameValuePair* nv = (NameValuePair*) (*iter)();
   while(nv)
   {
      // Target field not found before beforeFieldName
      if(strcmp(nv->data(), beforeFieldName) == 0)
      {
         nv = NULL;
         break;
      }
      // Target field found
      else if(strcmp(nv->data(), targetFieldName) == 0)
      {
         break;
      }
      nv = (NameValuePair*) (*iter)();
   }
   return(nv);
}

bool SdpBody::modifySdpSubfieldValue( UtlString& sdpLineValue,
                                      int subFieldToModifyIndex,
                                      const UtlString& subFiledReplacement )
{
   UtlString readSubfield;
   bool bReplacementDone = false;
   UtlString newSdpLineValue;

   int index = 0;
   while( NameValueTokenizer::getSubField( sdpLineValue, index, SDP_SUBFIELD_SEPARATORS, &readSubfield ) )
   {
      if( index != 0 )
      {
         newSdpLineValue.append( " " );
      }
      if( index == subFieldToModifyIndex ) // 2 is index of the address information element in c= line
      {
         newSdpLineValue.append( subFiledReplacement );
         bReplacementDone = true;
      }
      else
      {
         newSdpLineValue.append( readSubfield );
      }
      index++;
   }
   sdpLineValue = newSdpLineValue;
   return bReplacementDone;
}

NameValuePair* SdpBody::getRefToMediaAttributeByName( int mediaIndex,
                                                      const char* pAttributeNameToFind ) const
{
   UtlSListIterator iterator1(*sdpFields);
   UtlSListIterator iterator2(*sdpFields);
   NameValuePair* pMatch = 0;
   uint iteratorIndex;
   bool bAttributeFound = false;

   if( pAttributeNameToFind )
   {
      // The search for the attribute will be done in two passes.  First,
      // we will search for the attribute within the media description
      // designated by the suppliedc 'mediaIndex' parameter.  If that returns
      // no match, we will then search for the attribute in the session
      // description part of the SDP body.  The 'searchStartInterators'
      // contains the search starting point for both passes.
      UtlSListIterator* searchStartInterators[2] = { 0, 0 };
      if( positionFieldInstance(mediaIndex, &iterator1, "m") )
      {
         searchStartInterators[0] = &iterator1;
         searchStartInterators[1] = &iterator2;

         for( iteratorIndex = 0;
              !bAttributeFound && iteratorIndex < sizeof( searchStartInterators ) / sizeof( searchStartInterators[0] );
              iteratorIndex++ )
         {
            UtlSListIterator* pTempIterator = searchStartInterators[ iteratorIndex ];
            if( pTempIterator )
            {
               UtlString attribName;
               NameValuePair* nv;

               while( !pMatch && ( nv = findFieldNameBefore(pTempIterator, "a", "m") ) )
               {
                  const char* attribNameAndValue = nv->getValue();
                  if(attribNameAndValue)
                  {
                     NameValueTokenizer::getSubField(attribNameAndValue, 0,
                                                     SDP_ATTRIB_NAME_VALUE_SEPARATOR, &attribName);
                     if( attribName.compareTo( pAttributeNameToFind ) == 0 )
                     {
                        pMatch = nv;
                     }
                  }
               }
            }
         }
      }
   }
   return pMatch;
}

/* ============================ FUNCTIONS ================================= */
