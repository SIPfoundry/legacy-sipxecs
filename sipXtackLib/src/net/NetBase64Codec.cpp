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
#include "utl/UtlRegex.h"
#include "net/NetBase64Codec.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const char* NetBase64Codec::Base64Codes =
   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

const RegEx ValidBase64("^([A-Za-z0-9+/]+)(={0,2})$");

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
NetBase64Codec::NetBase64Codec()
{
}

// Copy constructor
NetBase64Codec::NetBase64Codec(const NetBase64Codec& rNetBase64Codec)
{
}

// Destructor
NetBase64Codec::~NetBase64Codec()
{
}

/* ============================ MANIPULATORS ============================== */

void NetBase64Codec::encode(int dataSize, const char data[],
                            int& encodedDataSize, char encodedData[])
{
   int i;
   int j = 0;
   encodedDataSize = encodedSize(dataSize);

   for (i = 0; i < (dataSize - (dataSize % 3)); i+=3)   // Encode 3 bytes at a time.
   {
      encodedData[j]   = Base64Codes[ (data[i] & 0xfc) >> 2 ];
      encodedData[j+1] = Base64Codes[ ((data[i] & 0x03) << 4)   | ((data[i+1] & 0xf0) >> 4) ];
      encodedData[j+2] = Base64Codes[ ((data[i+1] & 0x0f) << 2) | ((data[i+2] & 0xc0) >> 6) ];
      encodedData[j+3] = Base64Codes[ (data[i+2] & 0x3f) ];
      j += 4;
   }

   i = dataSize - (dataSize % 3);  // Where we left off before.
   switch (dataSize % 3)
   {
   case 2:  // One character padding needed.
   {
      encodedData[j] = Base64Codes[ (data[i] & 0xfc) >> 2 ];
      encodedData[j+1] = Base64Codes[ ((data[i] & 0x03) << 4) | ((data[i+1] & 0xf0) >> 4) ];
      encodedData[j+2] = Base64Codes[ (data[i+1] & 0x0f) << 2 ];
      encodedData[j+3] = Base64Codes[64];  // Pad
      break;
   }
   case 1:  // Two character padding needed.
   {
      encodedData[j] = Base64Codes[ (data[i] & 0xfc) >> 2 ];
      encodedData[j+1] = Base64Codes[ (data[i] & 0x03) << 4 ];
      encodedData[j+2] = Base64Codes[64];  // Pad
      encodedData[j+3] = Base64Codes[64];  // Pad
      break;
   }
   }
   // encodedData[j+4] = NULL;
}


void NetBase64Codec::encode(int dataSize, const char data[], UtlString& encodedData)
{
   int numEncodeBytes = encodedSize(dataSize);
   char encodeBuffer[numEncodeBytes];

   encode(dataSize, data, numEncodeBytes, encodeBuffer);
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


/// @returns > 0 iff the encoded data is syntactically valid, 0 if not.
size_t NetBase64Codec::validEncodingBytes(int encodedDataSize, ///< number of encoded octets
                                          const char encodedData[]  ///< the encoded data 
                                          )
{
   bool syntaxIsValid = false;
   int  encodingBytes = 0;
   
   RegEx validBase64(ValidBase64);
   if (validBase64.Search(encodedData,encodedDataSize))
   {
      /*
       * The pattern of the string matched, now check to see if the number of padding
       * bytes is correct for the number of encoding bytes.
       */
      int ignoreOffset;
      int paddingBytes;

      validBase64.Match(1,ignoreOffset,encodingBytes); // the actual encoded value characters
      validBase64.Match(2,ignoreOffset,paddingBytes);  // the padding - trailing '=' characters
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
         break;
      }
   }
   else
   {
      // encodedData contained invalid characters
   }

   return syntaxIsValid ? encodingBytes : 0;
}

char NetBase64Codec::decodeChar(const char encoded)
{
   // This does not check for invalid characters, so call isValid before you call this!!!
   return(encoded == '=' ? 0 : strchr(Base64Codes, encoded) - Base64Codes );
}

bool NetBase64Codec::decode(int encodedDataSize, const char encodedData[],
                            int& dataSize, char data[])
{
   dataSize = 0;
   bool valid = isValid(encodedDataSize, encodedData);
   if (valid)
   {
      int i;
      int j = 0;
      dataSize = decodedSize(encodedDataSize, encodedData);

      for (i = 0; i < encodedDataSize; i+=4) // Work on 4 bytes at a time.
      {                         // Twiddle bits.
         data[j]   = (decodeChar(encodedData[i]) << 2) |
            ((decodeChar(encodedData[i+1]) & 0x30) >> 4);
         if(j + 1 < dataSize)
         {
            data[j+1] = ((decodeChar(encodedData[i+1]) & 0x0f) << 4) |
               ((decodeChar(encodedData[i+2]) & 0x3c) >> 2);

            if(j + 2 < dataSize)
            {
               data[j+2] = ((decodeChar(encodedData[i+2]) & 0x03) << 6) |
                  (decodeChar(encodedData[i+3]) & 0x3f);
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
                            UtlString& data        // output: the decoded data
                            )
{
   data.remove(0);

   bool valid = isValid(encodedData);
   if (valid)
   {
      size_t sizeNeeded = decodedSize(encodedData.length(), encodedData.data()) + 1;

      if (data.capacity(sizeNeeded) >= sizeNeeded)
      {
         int size = 0;
         valid = decode(encodedData.length(), encodedData.data(),
                        size, const_cast<char*>(data.data()));
         if (valid)
         {
            data.setLength(size);
            const_cast<char*>(data.data())[size] = '\000';
         }
      }
   }
   return valid;
}

int NetBase64Codec::decodedSize(int encodedDataSize, const char encodedData[])
{
   size_t nonPadBytes = validEncodingBytes(encodedDataSize, encodedData);
   
   return nonPadBytes ? (nonPadBytes * 3) / 4 : 0;
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */


/* ============================ FUNCTIONS ================================= */
