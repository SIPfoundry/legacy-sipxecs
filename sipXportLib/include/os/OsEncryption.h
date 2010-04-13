//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsEncryption_h_
#define _OsEncryption_h_

// SYSTEM INCLUDES
#ifdef HAVE_SSL
#define OSENCRYPTION
#endif

#if defined (OSENCRYPTION)
#include <openssl/evp.h>
#include <openssl/x509.h>
#endif

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsTime.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define OE_MAX_KEY_LEN 64

#define OE_MAX_RESULTS_HEADER_LEN 32

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//! Wrapper and helper around system encryption routines.
/*!
  Hide encryption details like:
    what alg. is chosen
    initialization details of alg.
    messy details allocating buffers padded to minumal key length

  Create one instance per encryption/decryption operation. As such, this
   class makes no provisions to be multi-threaded

  FUTURE: enum encryption alg posibilities and add approp accessor methods. today
    there is only one, PBE/DES via OpenSSL
 */
class OsEncryption
{
 public:
    //! Create one for each encryption/decryption operation
    OsEncryption(void);

    virtual ~OsEncryption(void);

    //! Data to feed to encryption, not touched and NOT copied, so keep it around
    void setDataPointer(unsigned char *pIn, int inLen);

    //! How large is the untouched data buffer
    int getDataLen(void);

    //! Pointer to untouched data buffer
    unsigned char *getDataPointer(void);

    //! If you want to prepend this to the results header for file identification purposes
    void setResultsHeader(const unsigned char *header, int headerLen);

    //! After [en/de]crypting, here's your results
    unsigned char *getResults(void);

    //! After [en/de]crypting get your results here
    int getResultsLen(void);

    //! set secret password
    void setKey(const unsigned char *key, int keyLen);

    //! operate after setting approp. input. . NOTE: This will return OS_FAILED on vxworks. */
    OsStatus decrypt(void);

    //! operate after setting approp. input. NOTE: This will return OS_FAILED on vxworks. */
    OsStatus encrypt(void);

    //! free all, called in descructor
    OsStatus release(void);

    //! DEBUG turn on/off
    static UtlBoolean sIgnoreEncryption;

 protected:

    //! OpenSSL state differentation direction for API calls
    enum Direction
    {
        DECRYPT = 0,
        ENCRYPT = 1
    };

    //! allocate OpenSSL stuff
    OsStatus init(Direction direction);

    //! common handling of OpenSSL's errors
    UtlBoolean openSslError(void);

    //! common [en/de]crypt method
    OsStatus crypto(Direction direction);

 private:


#if defined (OSENCRYPTION)
    X509_ALGOR *mAlgorithm;

    EVP_CIPHER_CTX mContext;
#endif

    unsigned char *mSalt;     // defeats brute force decryption via appling dictionary

    int mSaltLen;

    unsigned char mKey[OE_MAX_KEY_LEN]; // storage of password

    int mKeyLen;

    unsigned char *mData;            // pointer to storage of data

    int mDataLen;

    unsigned char *mResults;         // allocated storage of results

    int mResultsLen;

    unsigned char mHeader[OE_MAX_RESULTS_HEADER_LEN];    // set/expect extra data in results buffer

    int mHeaderLen;

    // TEST: See unittests/EncryptionTest

};

#endif // _OsEncryption_h_
