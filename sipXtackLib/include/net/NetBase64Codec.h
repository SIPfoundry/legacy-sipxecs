//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
////////////////////////////////////////////////////////////////////////

#ifndef _NetBase64Codec_h_
#define _NetBase64Codec_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlString.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


/// Provides methods for translating to and from base64 encoding.
/**
 * Base 64 is a convenient encoding used to translate arbitrary binary
 * data into a fixed 64 character subset of ascii (plus one additional
 * character used to indicate padding).
 *
 * @nosubgrouping
 */
class NetBase64Codec
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   // ================================================================
   /** @name                  Encoding Alphabets
    *
    * These constant alphabets are the sets of characters used in the encoded form;
    * these specific constants MUST be used when calling - it will not work to
    * define another alphabet outside this class.
    */
   ///@{

   typedef const char* Base64Alphabet; ///< type for alphabet parameters in this class

   static Base64Alphabet RFC4648MimeAlphabet;    ///< 'traditional' alphabet (the default)
   static Base64Alphabet RFC4648UrlSafeAlphabet; ///< avoids characters unsafe in URLs
   static Base64Alphabet SipTokenSafeAlphabet;   /**< uses only characters allowed in SIP 'token'
                                                  *   and avoids other characters with special
                                                  *   meaning in various sipXecs contexts
                                                  */

   //@}

   // ================================================================
   /** @name                  Encoding Operations
    *
    * These methods translate from the binary data to the encodedData string
    */
   ///@{

   /// Encode from one array into another
   static void encode(int dataSize,         ///< the size of the binary data in octets
                      const char data[],    ///< the binary data - not null terminated
                      int& encodedDataSize, ///< output: the size of the encoded data in octets
                      char encodedData[],   ///< output: the encoded data
                      Base64Alphabet alphabet = RFC4648MimeAlphabet ///< encoding alphabet
                      );

   /// Encode from an array into a UtlString
   static void encode(int dataSize,          ///< the size of the binary data in octets
                      const char data[],     ///< the binary data - not null terminated
                      UtlString& encodedData,///< output: the encoded data
                      Base64Alphabet alphabet = RFC4648MimeAlphabet ///< encoding alphabet
                      );

   /// Encode from one UtlString into another.
   static void encode(const UtlString& data, ///< size is data.length(), not null terminated
                      UtlString& encodedData,///< output: the encoded data
                      Base64Alphabet alphabet = RFC4648MimeAlphabet ///< encoding alphabet
                      )
   {
      NetBase64Codec::encode(data.length(),data.data(),encodedData, alphabet);
   };

   /// @returns the number of encoded octets for given number of input binary octets
   static int encodedSize(int dataSize);

   ///@}

   // ================================================================
   /** @name                  Decoding Operations
    *
    * The decoding methods translate from the encoded parameter to the binary data
    * All return false if the encoded data value contained any characters
    * that are not legal in the base64 alphabet.
    */
   ///@{

   /// @returns true iff the encoded data is syntactically valid.
   static bool isValid(int encodedDataSize,      ///< the size of the encoded data in octets
                       const char encodedData[], ///< the encoded data
                       Base64Alphabet alphabet = RFC4648MimeAlphabet ///< encoding alphabet
                       )
   {
      return validEncodingBytes(encodedDataSize, encodedData, alphabet) > 0;
   }


   /// @returns true iff the encoded data is syntactically valid.
   static bool isValid(const UtlString encodedData, ///< size is data.length(), not null terminated
                       Base64Alphabet alphabet = RFC4648MimeAlphabet ///< encoding alphabet
                       )
   {
      return validEncodingBytes(encodedData.length(), encodedData.data(), alphabet) > 0;
   }

   /// Decode from the character encodedData to the binary data array.
   static bool decode(int encodedDataSize,      ///< the size of the encoded data in octets
                      const char encodedData[], ///< the encoded data
                      int& dataSize,            ///< output: the size of the binary data in octets
                      char data[],              ///< output: the binary data - not null terminated
                      Base64Alphabet alphabet = RFC4648MimeAlphabet ///< encoding alphabet
                      );
   ///< @returns false and no data if the encodedData contains any invalid characters.

   /// Decode from one UtlString into another
   static bool decode(const UtlString encodedData, ///< size is data.length(), not null terminated
                      UtlString& data,             ///< output: the decoded data
                      Base64Alphabet alphabet = RFC4648MimeAlphabet ///< encoding alphabet
                      );
   ///< @returns false and no data if the encodedData contains any invalid characters.

   /// Compute the number of output binary octets for given set of encoded octets.
   static int decodedSize(int encodedDataSize,
                          const char encodedData[],
                          Base64Alphabet alphabet = RFC4648MimeAlphabet ///< encoding alphabet
                          );
   ///< @returns zero if the encodedData contains any invalid characters.

   /// Compute the number of output binary octets for given set of encoded octets.
   static int decodedSize(const UtlString& encodedData, ///< size is data.length()
                          Base64Alphabet alphabet = RFC4648MimeAlphabet ///< encoding alphabet
                          )
   {
      ///< @returns zero if the encodedData contains any invalid characters.
      return decodedSize(encodedData.length(), encodedData.data(), alphabet);
   }


  private:

   inline static char decodeChar(const char encoded,     ///< encoded data character
                                 Base64Alphabet alphabet ///< encoding alphabet
                                 );

   /// @returns > 0 iff the encoded data is syntactically valid, 0 if not.
   static size_t validEncodingBytes(int encodedDataSize,      ///< number of encoded octets
                                    const char encodedData[], ///< the encoded data
                                    Base64Alphabet alphabet   ///< encoding alphabet
                                    );

   ///@}

   // @cond INCLUDENOCOPY
   NetBase64Codec();
   //:Default constructor (disabled)


   virtual
      ~NetBase64Codec();

   NetBase64Codec(const NetBase64Codec& rNetBase64Codec);
   //:Copy constructor (disabled)

   NetBase64Codec& operator=(const NetBase64Codec& rhs);
   //:Assignment operator (disabled)

   // @endcond
};

#endif  // _NetBase64Codec_h_
