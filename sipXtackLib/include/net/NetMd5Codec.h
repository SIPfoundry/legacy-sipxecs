//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
////////////////////////////////////////////////////////////////////////
#ifndef _NetMd5Codec_h_
#define _NetMd5Codec_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <net/NetBase64Codec.h>
#include <utl/UtlString.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define MD5_SIZE NetMd5Codec::ENCODED_SIZE
// a recognizer for an MD5 value for use in regular expressions
#define MD5_REGEX "[0-9a-f]{32}"
#define MD5_B64SIG_REGEX "[0-9a-zA-Z_`]{22}"

// FORWARD DECLARATIONS

//@cond NOINCLUDE
/* ============================ INLINE METHODS ============================ */
/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991.
 * All rights reserved.
 *
 * License to copy and use this software is granted provided that it
 * is identified as the "RSA Data Security, Inc. MD5 Message-Digest
 * Algorithm" in all material mentioning or referencing this software
 * or this function.
 *
 * License is also granted to make and use derivative works provided
 * that such works are identified as "derived from the RSA Data
 * Security, Inc. MD5 Message-Digest Algorithm" in all material
 * mentioning or referencing the derived work.
 *
 * RSA Data Security, Inc. makes no representations concerning either
 * the merchantability of this software or the suitability of this
 * software for any particular purpose. It is provided "as is"
 * without express or implied warranty of any kind.
 *
 * These notices must be retained in any copies of any part of this
 * documentation and/or software.
 */

//64-bit port: this extern "C" causes problems...
#ifdef __cplusplus
   extern "C" {
#endif

/* PROTOTYPES should be set to one if and only if the compiler supports
  function argument prototyping.
The following makes PROTOTYPES default to 0 if it has not already
  been defined with C compiler flags.
 */
#ifndef PROTOTYPES
#define PROTOTYPES 1
#endif

/* POINTER defines a generic pointer type */
typedef unsigned char* POINTER;

/* UINT2 defines a two byte word */
typedef unsigned short int UINT2;

/* UINT4 defines a four byte word */
typedef unsigned int UINT4;

/* PROTO_LIST is defined depending on how PROTOTYPES is defined above.
If using PROTOTYPES, then PROTO_LIST returns the list, otherwise it
  returns an empty list.
 */
#if PROTOTYPES
#define PROTO_LIST(list) list
#else
#define PROTO_LIST(list) ()
#endif


/* MD5 context. */
typedef struct {
  UINT4 state[4];                                   /* state (ABCD) */
  UINT4 count[2];        /* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];                         /* input buffer */
} MD5_CTX_PT;

void MD5Init PROTO_LIST ((MD5_CTX_PT *));
void MD5Update PROTO_LIST
  ((MD5_CTX_PT *, const unsigned char *, size_t));
void MD5Final PROTO_LIST ((unsigned char [16], MD5_CTX_PT *));

#ifdef __cplusplus
   }
#endif
//@endcond

/// Calculate the MD5 digest hash as defined by RSA Data Security.
class NetMd5Codec
/**<
 * This class is a hash computer; to use it, construct the NetMd5Codec object
 * and then pass each of the inputs to be incorporated into the hash value
 * to the 'hash' method.  When all inputs have incorporated, call appendHashValue
 * to finalize the hash computation and get the ASCII representation of the
 * hash value.
 * @code
 * NetMd5Codec md5;
 * md5.hash("foo");
 * md5.hash("bar");
 *
 * UtlString foobarHash;
 * md5.appendHashValue(foobarHash);
 * @endcode
 */
{
  public:

   /// The number of ascii characters in the conventional hash representation.
   static const unsigned int ENCODED_SIZE;
   static const unsigned int BASE64_ENCODED_SIZE;

   /// Prepare to generate an md5 hash value.
   NetMd5Codec();

   ~NetMd5Codec();

   /// Copy a partially computed hash.
   NetMd5Codec(const NetMd5Codec& rNetMd5Codec);
   /**<
    * This is useful when many calculations share a common input prefix.
    * A NetMd5Codec object can be created, and the prefix hashed into it.
    * Each subsequent calculatation can then create a new NetMd5Codec
    * object using this method, and then hash the unique values into it
    * and obtain the output as usual.
    */

   /// Provide input to the hash - may be called repeatedly with additional input.
   void hash(const void*  input,
             size_t length
             );

   /// Convenient version of hash for UtlString.
   void hash(const UtlString& utlString)
   {
      hash(utlString.data(), utlString.length());
   };

   /// Append the ascii representation of the md5 value to output.
   void appendHashValue(UtlString& output);
   /**<
    * It is an error (which asserts) to call this function more than once.
    */

   /// Append a base64-encoded ascii representation of the md5 value to output.
   void appendBase64Sig(UtlString& output, NetBase64Codec::Base64Alphabet alphabet=NetBase64Codec::SipTokenSafeAlphabet);
   /**<
    * @note It is an error (which asserts) to call this function more than once.
    *
    * This is a specialized function for producing a more compact text representation
    * of the hash value than the standard hex form.  This version represents the
    * hash as a 22 character base64-encoded string.  This should _not_ be used when
    * recovery of the original binary representation of the hash is needed, since the
    * two padding characters from the base64 encoding are removed, so the reverse
    * transformation is non-trivial.  If the purpose of the hash value is just to
    * compare it to another hash value for equality (the usual case for MD5 hashes),
    * the reverse transformation is not needed and this method may be used.
    */

   /// Convenience method for use when all the data to be hashed is contiguous - standard encoding.
   static void encode(const char* text, UtlString& encodedText);
   /**<
    * Generate the MD5 hash of *test into encodedText (as a string of
    * 32 hex digits).
    *
    * If the inputs to the hash are not already contiguous, don't construct
    * a string to pass to this method - use the constructor/hash/appendHashValue
    * sequence described above to avoid the data copies.
    */

   /// Convenience method for use when all the data to be hashed is contiguous - base64 encoding
   static void encodeBase64Sig(const char* text, UtlString& encodedText,
                               NetBase64Codec::Base64Alphabet alphabet=NetBase64Codec::SipTokenSafeAlphabet);
   /**<
    * Generate the MD5 hash of *test into encodedText (as a string of
    * 24 base64 digits).
    *
    * If the inputs to the hash are not already contiguous, don't construct
    * a string to pass to this method - use the constructor/hash/appendHashValue
    * sequence described above to avoid the data copies.
    *
    * This method produces the same encoding as appendBase64Sig - see that method
    * for when this is an appropriate encoding.
    */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   MD5_CTX_PT    mContext;
   unsigned char mDigest[16];
   bool          mFinalized;

   // @cond NOINCLUDE
   NetMd5Codec& operator=(const NetMd5Codec& rhs);
   //:Assignment operator (disabled)
   // @endcond
};

#endif  // _NetMd5Codec_h_
