//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>
#include <ctype.h>
#include <time.h>

#ifdef __pingtel_on_posix__
#include <stdlib.h>
#include <netdb.h>
#endif

#ifdef HAVE_SSL
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#endif

#ifdef _VXWORKS
#include <../config/pingtel/pingtel.h>
#include <resolvLib.h>
#endif

#ifdef TEST
#include "utl/UtlMemCheck.h"
#endif

// APPLICATION INCLUDES
#include "os/OsExcept.h"
#include "os/OsNameDb.h"
#include "os/OsEncryption.h"
#include "os/OsSocket.h"
#include "os/OsLogger.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS
static const char gSalt[] =
{
    (unsigned char)0xc9, (unsigned char)0x36, (unsigned char)0x78, (unsigned char)0x99,
    (unsigned char)0x52, (unsigned char)0x3e, (unsigned char)0xea, (unsigned char)0xf2
};

// STATIC VARIABLE INITIALIZATIONS
UtlBoolean OsEncryption::sIgnoreEncryption = FALSE;

// Wrapper around encryption, currently OpenSSL, specifically DES w/MD5
// See:
//   http://www.catfive.org/cgi-bin/man2web?program=EVP_CipherInit&section=3
// For OpenSLL API information


/* //////////////////////////// PUBLIC //////////////////////////////////// */
OsEncryption::OsEncryption(void)
{
    mSalt = (unsigned char *)gSalt;
    mSaltLen = sizeof(gSalt);
    memset(&mKey, 0, sizeof(mKey));
    mKeyLen = 0;
    mData = NULL;
    mDataLen = 0;
    mResults = NULL;
    mResultsLen = 0;
    memset(&mHeader, 0, sizeof(mHeader));
    mHeaderLen = 0;
#if defined (OSENCRYPTION)
    mAlgorithm = NULL;
    memset(&mContext, 0, sizeof(mContext));
#endif
}


OsEncryption::~OsEncryption(void)
{
    release(); // open encryption algorithms
}


void OsEncryption::setKey(const unsigned char* key, int keyLen)
{
    if (key != NULL)
    {
        mKeyLen = (OE_MAX_KEY_LEN < keyLen ? OE_MAX_KEY_LEN : keyLen);
        memcpy(mKey, key, mKeyLen);
    }
}

void OsEncryption::setResultsHeader(const unsigned char* header, int headerLen)
{
    mHeaderLen = (OE_MAX_RESULTS_HEADER_LEN < headerLen ? OE_MAX_RESULTS_HEADER_LEN : headerLen);
    memcpy(mHeader, header, mHeaderLen);
}

// Does not make a copy
void OsEncryption::setDataPointer(unsigned char* data, int dataLen)
{
    mData = data;
    mDataLen = dataLen;
}

int OsEncryption::getDataLen(void)
{
    return mDataLen;
}

unsigned char *OsEncryption::getDataPointer(void)
{
    return mData;
}

// Null terminiated incase this helps
unsigned char *OsEncryption::getResults(void)
{
    return mResults;
}

int OsEncryption::getResultsLen(void)
{
    return mResultsLen;
}

OsStatus OsEncryption::decrypt(void)
{
    return crypto(DECRYPT);
}

OsStatus OsEncryption::encrypt(void)
{
    return crypto(ENCRYPT);
}

// Call this after you're done with encryption
OsStatus OsEncryption::release(void)
{
    OsStatus retval = OS_FAILED;

#if defined(OSENCRYPTION)
    // TODO: Analyze performance v.s. memory gains to never release this
    if (mAlgorithm != NULL)
    {
        X509_ALGOR_free(mAlgorithm);
        mAlgorithm = NULL;
    }

    if (mResults != NULL)
    {
        OPENSSL_free(mResults);
        mResults = NULL;
        mResultsLen = 0;
    }

    retval = OS_SUCCESS;
#endif

    return retval;
}

/* //////////////////////////// PRIVATE/PROTECTED //////////////////////////////////// */
OsStatus OsEncryption::init(Direction direction)
{
    OsStatus retval = OS_FAILED;

#if defined(OSENCRYPTION)
    release();

    if (mKeyLen > 0 && mKey != NULL && mDataLen > 0 && mData != NULL)
    {
        ERR_clear_error();

        SSLeay_add_all_algorithms();
        mAlgorithm = PKCS5_pbe_set(NID_pbeWithMD5AndDES_CBC,
            PKCS5_DEFAULT_ITER, mSalt, mSaltLen);

        if (mAlgorithm != NULL)
        {
            EVP_CIPHER_CTX_init(&(mContext));
            if (EVP_PBE_CipherInit(mAlgorithm->algorithm, (const char *)mKey, mKeyLen,
                                   mAlgorithm->parameter, &(mContext), (int)direction))
            {
                int blockSize = EVP_CIPHER_CTX_block_size(&mContext);
                int allocLen = mDataLen + mHeaderLen + blockSize + 1; // plus 1 for null terminator on decrypt
                mResults = (unsigned char *)OPENSSL_malloc(allocLen);
                if (mResults == NULL)
                {
                    Os::Logger::instance().log(FAC_AUTH, PRI_ERR, "Could not allocate cryption buffer(size=%d)",
                                  allocLen);
                }
                else
                {
                    retval = OS_SUCCESS;
                }
            }
            else
            {
                Os::Logger::instance().log(FAC_AUTH, PRI_ERR, "Could not initialize cipher");
            }
        }
        else
        {
            Os::Logger::instance().log(FAC_AUTH, PRI_ERR, "Could not initialize cryption algorithm");
        }
    }
    else
    {
        Os::Logger::instance().log(FAC_AUTH, PRI_ERR, "No encryption key(%d) or data(%d) set.\n",
            mKeyLen, mDataLen);
    }
#endif

    return retval;
}

OsStatus OsEncryption::crypto(Direction direction)
{
    OsStatus retval = init(direction);

#if defined(OSENCRYPTION)
    if (retval == OS_SUCCESS)
    {
        if (sIgnoreEncryption)
        {
            memcpy(mResults, mData, mDataLen);
            mResultsLen = mDataLen;
        }
        else
        {
            retval = OS_FAILED;
            unsigned char *in = mData;
            int inLen = mDataLen;
            unsigned char *out = mResults;
            int outLen = 0;

            if (mHeaderLen > 0)
            {
                if (direction == ENCRYPT)
                {
                    // copy in header
                    memcpy(out, mHeader, mHeaderLen);
                    out += mHeaderLen;
                    outLen += mHeaderLen;
                }
                else
                {
                    // ignore header
                    in += mHeaderLen;
                    inLen -= mHeaderLen;
                }
            }

            int outLenPart1 = 0;
            if (EVP_CipherUpdate(&(mContext), out, &outLenPart1, in, inLen))
            {
                out += outLenPart1;
                int outLenPart2 = 0;
                if (EVP_CipherFinal(&(mContext), out, &outLenPart2))
                {
                    outLen += outLenPart1 + outLenPart2;
                    retval = OS_SUCCESS;
                    mResults[outLen] = 0;
                    mResultsLen = outLen;
                }
            }
        }
    }

    if (retval != OS_SUCCESS)
    {
        openSslError();
        release();
    }

#endif
   return retval;
}


UtlBoolean OsEncryption::openSslError(void)
{
#if defined(OSENCRYPTION)
    unsigned long err = ERR_get_error();
    if (err != 0)
    {
        ERR_load_crypto_strings();
        ERR_load_ERR_strings();
        char errbuff[256];
        errbuff[0] = 0;
        ERR_error_string_n(err, errbuff, sizeof(errbuff));
        osPrintf("OpenSLL ERROR:\n\tlib:%s\n\tfunction:%s\n\treason:%s\n",
            ERR_lib_error_string(err),
            ERR_func_error_string(err),
            ERR_reason_error_string(err));
        ERR_free_strings();

        return TRUE;
    }
#endif

    return FALSE;
}
