//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>
#include <string.h>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsSysLog.h"
#include "utl/UtlRegex.h"
#include "net/NetBase64Codec.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define PAD_CHAR 64

NetBase64Codec::Base64Alphabet NetBase64Codec::RFC4648MimeAlphabet =
   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

NetBase64Codec::Base64Alphabet NetBase64Codec::RFC4648UrlSafeAlphabet =
   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_=";

NetBase64Codec::Base64Alphabet NetBase64Codec::SipTokenSafeAlphabet =
   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_`'";

const RegEx Valid_RFC4648MimeAlphabet(   "^([A-Za-z0-9+/]+)(={0,2})$");
const RegEx Valid_RFC4648UrlSafeAlphabet("^([A-Za-z0-9_-]+)(={0,2})$");
const RegEx Valid_SipTokenSafeAlphabet(  "^([A-Za-z0-9_`]+)('{0,2})$");

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

void NetBase64Codec::encode(int dataSize, const char data[],
                            int& encodedDataSize, char encodedData[],
                            Base64Alphabet alphabet
                            )
{
   int i;
   int j = 0;
   encodedDataSize = encodedSize(dataSize);

   for (i = 0; i < (dataSize - (dataSize % 3)); i+=3)   // Encode 3 bytes at a time.
   {
      encodedData[j]   = alphabet[ (data[i] & 0xfc) >> 2 ];
      encodedData[j+1] = alphabet[ ((data[i] & 0x03) << 4)   | ((data[i+1] & 0xf0) >> 4) ];
      encodedData[j+2] = alphabet[ ((data[i+1] & 0x0f) << 2) | ((data[i+2] & 0xc0) >> 6) ];
      encodedData[j+3] = alphabet[ (data[i+2] & 0x3f) ];
      j += 4;
   }

   i = dataSize - (dataSize % 3);  // Where we left off before.
   switch (dataSize % 3)
   {
   case 2:  // One character padding needed.
   {
      encodedData[j] = alphabet[ (data[i] & 0xfc) >> 2 ];
      encodedData[j+1] = alphabet[ ((data[i] & 0x03) << 4) | ((data[i+1] & 0xf0) >> 4) ];
      encodedData[j+2] = alphabet[ (data[i+1] & 0x0f) << 2 ];
      encodedData[j+3] = alphabet[PAD_CHAR];  // Pad
      break;
   }
   case 1:  // Two character padding needed.
   {
      encodedData[j] = alphabet[ (data[i] & 0xfc) >> 2 ];
      encodedData[j+1] = alphabet[ (data[i] & 0x03) << 4 ];
      encodedData[j+2] = alphabet[PAD_CHAR];  // Pad
      encodedData[j+3] = alphabet[PAD_CHAR];  // Pad
      break;
   }
   }
   // encodedData[j+4] = NULL;
}


void NetBase64Codec::encode(int dataSize, const char data[], UtlString& encodedData,
                            Base64Alphabet alphabet)
{
   int numEncodeBytes = encodedSize(dataSize);
   char encodeBuffer[numEncodeBytes];

   encode(dataSize, data, numEncodeBytes, encodeBuffer, alphabet);
   encodedData.remove(0);
   encodedData.append(encodeBuffer, numEncodeBytes);
}

int NetBase64Codec::encodedSize(int dataSize)
{
   int size = dataSize / 3;
   if(dataSize % 3) size++;

   size *= 4;

   return(size);
}


/// @returns the number of valid non-pad characters in the encodedData
size_t NetBase64Codec::validEncodingBytes(int encodedDataSize, ///< number of encoded octets
                                          const char encodedData[],   ///< the encoded data
                                          Base64Alphabet alphabet
                                          )
{
   bool syntaxIsValid = false;
   int  encodingBytes = 0;

   const RegEx* validBase64rxp;

   if ( alphabet == RFC4648MimeAlphabet )
   {
      validBase64rxp = &Valid_RFC4648MimeAlphabet;
   }
   else if ( alphabet == RFC4648UrlSafeAlphabet )
   {
      validBase64rxp = &Valid_RFC4648UrlSafeAlphabet;
   }
   else if ( alphabet == SipTokenSafeAlphabet )
   {
      validBase64rxp = &Valid_SipTokenSafeAlphabet;
   }
   else
   {
      OsSysLog::add(FAC_NET, PRI_CRIT,
                    "NetBase64Codec::validEncodingBytes invalid alphabet [%s]", alphabet);
      assert(false);
   }

   RegEx validBase64(*validBase64rxp);

   if (validBase64.Search(encodedData,encodedDataSize))
   {
      /*
       * The pattern of the string matched, now check to see if the number of padding
       * bytes is correct for the number of encoding bytes.
       */
      int ignoreOffset;
      int paddingBytes;

      validBase64.Match(1,ignoreOffset,encodingBytes); // the actual encoded value characters
      validBase64.Match(2,ignoreOffset,paddingBytes);  // the padding - trailing pad characters
      switch (paddingBytes)
      {
      case 0:
         syntaxIsValid = (0 == encodingBytes % 4);
         break;
      case 1:
         syntaxIsValid = (3 == encodingBytes % 4);
         break;
      case 2:
         syntaxIsValid = (2 == encodingBytes % 4);
         break;
      default:
         // for any other value, the syntax is not valid
         OsSysLog::add(FAC_NET, PRI_ERR,
                       "NetBase64Codec::validEncodingBytes invalid number of pad characters"
                       );
         break;
      }
   }
   else
   {
      // encodedData contained invalid characters
      OsSysLog::add(FAC_NET, PRI_ERR,
                    "NetBase64Codec::validEncodingBytes invalid characters in encoded data"
                    );
   }

   return syntaxIsValid ? encodingBytes : 0;
}

char NetBase64Codec::decodeChar(const char encoded,
                                Base64Alphabet alphabet)
{
   // This does not check for invalid characters, so call isValid before you call this!!!
   return(encoded == alphabet[PAD_CHAR] ? 0 : strchr(alphabet, encoded) - alphabet );
}

bool NetBase64Codec::decode(int encodedDataSize, const char encodedData[],
                            int& dataSize, char data[],
                            Base64Alphabet alphabet)
{
   unsigned char* udata = (unsigned char*)data;

   dataSize = 0;
   bool valid = isValid(encodedDataSize, encodedData, alphabet);
   if (valid)
   {
      int i;
      int j = 0;
      dataSize = decodedSize(encodedDataSize, encodedData, alphabet);

      for (i = 0; i < encodedDataSize; i+=4) // Work on 4 bytes at a time.
      {                         // Twiddle bits.
         udata[j]   = ( (decodeChar(encodedData[i], alphabet) << 2)
                       |((decodeChar(encodedData[i+1], alphabet) & 0x30) >> 4)
                       );
         if(j + 1 < dataSize)
         {
            udata[j+1] = ( ((decodeChar(encodedData[i+1], alphabet) & 0x0f) << 4)
                          |((decodeChar(encodedData[i+2], alphabet) & 0x3c) >> 2)
                          );

            if(j + 2 < dataSize)
            {
               udata[j+2] = ( ((decodeChar(encodedData[i+2], alphabet) & 0x03) << 6)
                             |(decodeChar(encodedData[i+3], alphabet) & 0x3f)
                             );
            }
         }
         j += 3;
      }
   }

   return valid;
}

// Decode from one UtlString into another
bool NetBase64Codec::decode(const UtlString encodedData, /* sizeis data.length(),
                                                          * not null terminated */
                            UtlString& data,  ///< the encoded data
                            Base64Alphabet alphabet
                            )
{
   data.remove(0);

   bool valid = isValid(encodedData, alphabet);
   if (valid)
   {
      size_t sizeNeeded = decodedSize(encodedData.length(), encodedData.data(), alphabet) + 1;

      if (data.capacity(sizeNeeded+1) >= sizeNeeded+1)
      {
         int size = 0;
         valid = decode(encodedData.length(), encodedData.data(),
                        size, const_cast<char*>(data.data()), alphabet);
         if (valid)
         {
            data.setLength(size);
         }
      }
   }
   return valid;
}

int NetBase64Codec::decodedSize(int encodedDataSize, const char encodedData[],
                                Base64Alphabet alphabet)
{
   size_t nonPadBytes = validEncodingBytes(encodedDataSize, encodedData, alphabet);

   return nonPadBytes ? (nonPadBytes * 3) / 4 : 0;
}
