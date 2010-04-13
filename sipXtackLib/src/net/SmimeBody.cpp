//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <net/SmimeBody.h>
#include <net/HttpMessage.h>
#include <os/OsSysLog.h>

//#define ENABLE_OPENSSL_SMIME
#ifdef ENABLE_OPENSSL_SMIME
#   include <openssl/bio.h>
#   include <openssl/pem.h>
#   include <openssl/crypto.h>
#   include <openssl/err.h>
#   include <openssl/pkcs12.h>
#endif

#ifndef _WIN32 // for now, only compile this stuff on Windows, so as to not break the build
#  undef ENABLE_NSS_SMIME
#endif
#ifdef ENABLE_NSS_SMIME
#   include <nspr.h>
#   include <nss.h>
#   include <ssl.h>
#   include <cms.h>
#   include <pk11func.h>
#   include <p12.h>
#   include <base64.h>
    extern "C" CERTCertificate *
    __CERT_DecodeDERCertificate(SECItem *derSignedCert, PRBool copyDER, char *nickname);
#endif

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SmimeBody::SmimeBody(const char* bytes,
                     int length,
                     const char* smimeEncodingType)
{
    bodyLength = length;
    mBody.append(bytes, length);

    // set the mime type in the parent UtlString
    remove(0);
    append(CONTENT_SMIME_PKCS7);

    mContentEncoding = SMIME_ENODING_UNKNOWN;
    if(smimeEncodingType)
    {
        UtlString sMimeEncodingString(smimeEncodingType);
        sMimeEncodingString.toUpper();

        if(sMimeEncodingString.compareTo(HTTP_CONTENT_TRANSFER_ENCODING_BINARY, UtlString::ignoreCase) == 0)
        {
            mContentEncoding = SMIME_ENODING_BINARY;
        }
        else if(sMimeEncodingString.compareTo(HTTP_CONTENT_TRANSFER_ENCODING_BASE64, UtlString::ignoreCase) == 0)
        {
            mContentEncoding = SMIME_ENODING_BASE64;
        }
        else
        {
            // TODO: We could probably put a hack in here to heuristically
            // determine if the encoding is base64 or not based upon the
            // byte values.
            OsSysLog::add(FAC_SIP, PRI_ERR,
                "Invalid transport encoding for S/MIME content");
        }
    }

    mpDecryptedBody = NULL;
}

// Copy constructor
SmimeBody::SmimeBody(const SmimeBody& rSmimeBody)
  : HttpBody(rSmimeBody)
{
    mpDecryptedBody = NULL;
    if(rSmimeBody.mpDecryptedBody)
    {
       mpDecryptedBody = rSmimeBody.mpDecryptedBody->copy();
    }

    // set mime type in parent UtlString
    remove(0);
    append(CONTENT_SMIME_PKCS7);
    mContentEncoding = rSmimeBody.mContentEncoding;
}

SmimeBody* SmimeBody::copy() const
{
    return new SmimeBody(*this);
}

// Destructor
SmimeBody::~SmimeBody()
{
   if(mpDecryptedBody)
   {
       delete mpDecryptedBody;
       mpDecryptedBody = NULL;
   }
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SmimeBody&
SmimeBody::operator=(const SmimeBody& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   // Copy the parent
   *((HttpBody*)this) = rhs;

   // Remove the decrypted body if one is attached,
   // in case we copy over it
   if(mpDecryptedBody)
   {
       delete mpDecryptedBody;
       mpDecryptedBody = NULL;
   }

   // If the source has a body copy it
   if(rhs.mpDecryptedBody)
   {
       mpDecryptedBody = rhs.mpDecryptedBody->copy();
   }

   mContentEncoding = rhs.mContentEncoding;
   return *this;
}

UtlBoolean SmimeBody::decrypt(const char* derPkcs12,
                              int derPkcs12Length,
                              const char* pkcs12Password)
{
    UtlBoolean decryptionSucceeded = FALSE;

    UtlString decryptedData;

#ifdef ENABLE_OPENSSL_SMIME
    decryptionSucceeded =
        opensslSmimeDecrypt(derPkcs12,
                            derPkcs12Length,
                            pkcs12Password,
                            (mContentEncoding == SMIME_ENODING_BASE64),
                            mBody.data(),
                            mBody.length(),
                            decryptedData);
#elif ENABLE_NSS_SMIME
    OsSysLog::add(FAC_SIP, PRI_ERR, "NSS S/MIME decrypt not implemented");
#endif

    // Decryption succeeded, so create a HttpBody for the result
    if(decryptionSucceeded &&
        decryptedData.length() > 0)
    {
        HttpBody* newDecryptedBody = NULL;
        // Need to read the headers before the real body to see
        // what the content type of the decrypted body is
        UtlDList bodyHeaders;
        int parsedBytes =
            HttpMessage::parseHeaders(decryptedData.data(),
                                      decryptedData.length(),
                                      bodyHeaders);

        UtlString contentTypeName(HTTP_CONTENT_TYPE_FIELD);
        NameValuePair* contentType =
            (NameValuePair*) bodyHeaders.find(&contentTypeName);
        UtlString contentEncodingName(HTTP_CONTENT_TRANSFER_ENCODING_FIELD);
        NameValuePair* contentEncoding =
            (NameValuePair*) bodyHeaders.find(&contentEncodingName);

        const char* realBodyStart = decryptedData.data() + parsedBytes;
        int realBodyLength = decryptedData.length() - parsedBytes;

        newDecryptedBody =
            HttpBody::createBody(realBodyStart,
                                 realBodyLength,
                                 contentType ? contentType->getValue() : NULL,
                                 contentEncoding ? contentEncoding->getValue() : NULL);

        bodyHeaders.destroyAll();

        // If one already exists, delete it.  This should not typically
        // be the case.  Infact it might make sense to make this method
        // a no-op if a decrypted body already exists
        if(mpDecryptedBody)
        {
            delete mpDecryptedBody;
            mpDecryptedBody = NULL;
        }

        mpDecryptedBody = newDecryptedBody;
    }

    return(decryptionSucceeded);
}

UtlBoolean SmimeBody::encrypt(HttpBody* bodyToEncrypt,
                              int numRecipients,
                              const char* derPublicKeyCerts[],
                              int derPubliceKeyCertLengths[])
{
    UtlBoolean encryptionSucceeded = FALSE;

    // Clean up an residual decrypted body or encrypted body content.
    // Should typically not be any.
    if(mpDecryptedBody)
    {
        delete mpDecryptedBody;
        mpDecryptedBody = NULL;
    }
    mBody.remove(0);


    if(bodyToEncrypt)
    {
        UtlString dataToEncrypt;
        UtlString contentType = bodyToEncrypt->getContentType();

        // Add the content-type and content-encoding headers to
        // the body to be decrypted so that when the otherside
        // decrypts this body, it can tell what the content is
        dataToEncrypt ="Content-Type: ";
        dataToEncrypt.append(contentType);
        dataToEncrypt.append(END_OF_LINE_DELIMITER);
        dataToEncrypt.append("Content-Transfer-Encoding: binary");
        dataToEncrypt.append(END_OF_LINE_DELIMITER);
        dataToEncrypt.append(END_OF_LINE_DELIMITER);

        // Append the real body content
        const char* dataPtr;
        ssize_t dataLength;
        bodyToEncrypt->getBytes(&dataPtr, &dataLength);
        dataToEncrypt.append(dataPtr, dataLength);

        // Attach the decrypted version of the body for
        // future reference.
        mpDecryptedBody = bodyToEncrypt;

        UtlString encryptedData;

#ifdef ENABLE_OPENSSL_SMIME
        OsSysLog::add(FAC_SIP, PRI_ERR,
            "SmimeBody::opensslSmimeEncrypt not implemented");

#elif ENABLE_NSS_SMIME
        UtlBoolean encryptedDataInBase64Format = FALSE;

        encryptionSucceeded =
            nssSmimeEncrypt(numRecipients,
                            derPublicKeyCerts,
                            derPubliceKeyCertLengths,
                            dataToEncrypt.data(),
                            dataToEncrypt.length(),
                            encryptedDataInBase64Format,
                            mBody);
#endif

        encryptedData = mBody;
        // There should always be content if encryption succeeds
        if(encryptionSucceeded &&
           encryptedData.length() <= 0)
        {
            encryptionSucceeded = FALSE;
            OsSysLog::add(FAC_SIP, PRI_ERR,
                "SmimeBody::encrypt no encrypted content");
        }
    }

    return(encryptionSucceeded);
}

#ifdef ENABLE_NSS_SMIME
static void nssOutToUtlString(void *sink, const char *data, unsigned long dataLength)
{
    printf("nssOutToUtlString recieved %d bytes\n", dataLength);
    UtlString* outputSink = (UtlString*) sink;
    outputSink->append(data, dataLength);
}

#endif

UtlBoolean SmimeBody::nssSmimeEncrypt(int numResipientCerts,
                           const char* derPublicKeyCerts[],
                           int derPublicKeyCertLengths[],
                           const char* dataToEncrypt,
                           int dataToEncryptLength,
                           UtlBoolean encryptedDataInBase64Format,
                           UtlString& encryptedData)
{
    UtlBoolean encryptionSucceeded = FALSE;
    encryptedData.remove(0);

#ifdef ENABLE_NSS_SMIME

    // nickname can be NULL as we are not putting this in a database
    char *nickname = NULL;
    // copyDER = true so we copy the DER format cert passed in so memory
    // is internally alloc'd and freed
    PRBool copyDER = true;

    // Create an envelope or container for the encrypted data
    SECOidTag algorithm = SEC_OID_DES_EDE3_CBC; // or  SEC_OID_AES_128_CBC
    // Should be able to get the key size from the cert somehow
    int keysize = 1024;
    NSSCMSMessage* cmsMessage = NSS_CMSMessage_Create(NULL);
    NSSCMSEnvelopedData* myEnvelope =
        NSS_CMSEnvelopedData_Create(cmsMessage, algorithm, keysize);

    // Do the following for each recipient if there is more than one.
    // For each recipient:
    for(int certIndex = 0; certIndex < numResipientCerts; certIndex++)
    {
        // Convert the DER to a NSS CERT
        SECItem derFormatCertItem;
        SECItem* derFormatCertItemPtr = &derFormatCertItem;
        derFormatCertItem.data = (unsigned char*) derPublicKeyCerts[certIndex];
        derFormatCertItem.len = derPublicKeyCertLengths[certIndex];
        CERTCertificate* myCertFromDer = NULL;
        myCertFromDer = __CERT_DecodeDERCertificate(&derFormatCertItem,
                                                   copyDER,
                                                   nickname);

        // Add just the recipient Subject key Id, if it exists to the envelope
        // This is the minimal information needed to identify which recipient
        // the the symetric/session key is encrypted for
        NSSCMSRecipientInfo* recipientInfo = NULL;

        // Add the full set of recipient information including
        // the Cert. issuer location and org. info.
        recipientInfo =
            NSS_CMSRecipientInfo_Create(cmsMessage, myCertFromDer);

        if(recipientInfo)
        {
            if(NSS_CMSEnvelopedData_AddRecipient(myEnvelope , recipientInfo) !=
                SECSuccess)
            {
                NSS_CMSEnvelopedData_Destroy(myEnvelope);
                myEnvelope = NULL;
                NSS_CMSRecipientInfo_Destroy(recipientInfo);
            }
        }
        // No recipientInfo
        else
        {
            NSS_CMSEnvelopedData_Destroy(myEnvelope);
            myEnvelope = NULL;
        }

    } //end for each recipient

    // Get the content out of the envelop
    NSSCMSContentInfo* envelopContentInfo =
       NSS_CMSEnvelopedData_GetContentInfo(myEnvelope);

    //TODO: why are we copying or setting the content pointer from the envelope into the msg????????
    if (NSS_CMSContentInfo_SetContent_Data(cmsMessage,
                                           envelopContentInfo,
                                           NULL,
                                           PR_FALSE) !=
        SECSuccess)
    {
        // release cmsg and other stuff
        NSS_CMSEnvelopedData_Destroy(myEnvelope);
        myEnvelope = NULL;
    }

    //TODO: why are we copying or setting the content pointer from the message and
    // putting it back into the msg????????
    NSSCMSContentInfo* messageContentInfo =
        NSS_CMSMessage_GetContentInfo(cmsMessage);
    if(NSS_CMSContentInfo_SetContent_EnvelopedData(cmsMessage,
                                                   messageContentInfo,
                                                   myEnvelope) !=
       SECSuccess)
    {
        // release cmsg and other stuff
        NSS_CMSEnvelopedData_Destroy(myEnvelope);
        myEnvelope = NULL;
    }

    if(cmsMessage)
    {
        // Create an encoder  and context to do the encryption.
        // The encodedItem will stort the encoded result
        //SECItem encodedItem;
        //encodedItem.data = NULL;
        //encodedItem.len = 0;
        //SECITEM_AllocItem(NULL, &encodedItem, 0);
        printf("start encoder\n");
        NSSCMSEncoderContext* encoderContext =
            NSS_CMSEncoder_Start(cmsMessage, nssOutToUtlString, &encryptedData, NULL,
                                 NULL, NULL, NULL, NULL, NULL, NULL, NULL);

        // Add encrypted content
        printf("update encoder\n");
        NSS_CMSEncoder_Update(encoderContext, dataToEncrypt, dataToEncryptLength);

        // Finished encrypting
        printf("finish encoder\n");
        NSS_CMSEncoder_Finish(encoderContext);

        myEnvelope = NULL;

        if(encryptedData.length() > 0)
        {
            encryptionSucceeded = TRUE;
        }

        // Clean up the message memory, the envelop gets cleaned up
        // with the message
        NSS_CMSMessage_Destroy(cmsMessage);
        cmsMessage = NULL;
    }

#else
    OsSysLog::add(FAC_SIP, PRI_ERR, "SmimeBody::nssSmimeEncrypt invoked with ENABLE_NSS_SMIME not defined");
#endif

    return(encryptionSucceeded);
}

UtlBoolean SmimeBody::nssSmimeDecrypt(const char* derPkcs12,
                               int derPkcs12Length,
                               const char* pkcs12Password,
                               UtlBoolean dataIsInBase64Format,
                               const char* dataToDecrypt,
                               int dataToDecryptLength,
                               UtlString& decryptedData)
{
    UtlBoolean decryptSucceeded = FALSE;
    decryptedData.remove(0);

#ifdef ENABLE_NSS_SMIME
    OsSysLog::add(FAC_SIP, PRI_ERR, "SmimeBody::nssSmimeDecrypt not implemented");

    ////// BEGIN WARNING: THIS CODE HAS NOT BEEN TESTED AT ALL ///////

    // allocate a temporaty slot in the database
    PK11SlotInfo *slot = PK11_GetInternalKeySlot();
    PRBool swapUnicode = PR_FALSE;
    SEC_PKCS12DecoderContext *p12Decoder = NULL;

    // Need to put the pkcs12 password into a SECItem
    SECItem passwordItem;
    passwordItem.data = (unsigned char*) pkcs12Password;
    passwordItem.len = strlen(pkcs12Password);
    SECItem uniPasswordItem;
    uniPasswordItem.data = NULL;
    uniPasswordItem.len = 0;

#ifdef IS_LITTLE_ENDIAN
    swapUnicode = PR_TRUE;
#endif

    // Allocate a temporary internal slot
    slot = PK11_GetInternalSlot();
    if(slot == NULL)
    {
        OsSysLog::add(FAC_SIP, PRI_ERR, "unable to use slot in NSS dataqbase for S/MIME decryption");
    }
    else
    {
        // Do UNICODE conversion of password based upon the platform
        // (not sure this is even neccessary in our application).  I do not
        // know how we would get unicode passwords
        if(0) //P12U_UnicodeConversion(NULL, &uniPasswordItem, passwordItem, PR_TRUE,
			  //    swapUnicode) != SECSuccess)
        {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                "NSS Unicode conversion failed for PKCS12 object for S/MIME decryption");
        }
        else
        {
            // Initialze the decoder for the PKCS12 container for the private key
            p12Decoder = SEC_PKCS12DecoderStart(&passwordItem, slot, NULL,
				    NULL, NULL, NULL, NULL, NULL);
            if(!p12Decoder)
            {
                OsSysLog::add(FAC_SIP, PRI_ERR,
                    "failed to initialize PKCS12 decoder to extract private key for S/MIME decryption");
            }
            else
            {
                // Add the PKCS12 data to the decoder
                if(SEC_PKCS12DecoderUpdate(p12Decoder,
                                           (unsigned char *) derPkcs12,
                                           derPkcs12Length) != SECSuccess ||
                   // Validate the decoded PKCS12
                   SEC_PKCS12DecoderVerify(p12Decoder) != SECSuccess)

                {
                    OsSysLog::add(FAC_SIP, PRI_ERR,
                        "unable to decrypt PKCS12 for S/MIME decryption. Perhaps invalid PKCS12 or PKCS12 password");
                }
                else
                {
                    // Import the private key and certificate from the
                    // decoded PKCS12 into the database
                    if(SEC_PKCS12DecoderImportBags(p12Decoder) != SECSuccess)
                    {
                        OsSysLog::add(FAC_SIP, PRI_ERR,
                            "failed to import private key and certificate into NSS database");
                    }
                    else
                    {
                        // Put the S/MIME data in a SECItem
                        SECItem dataToDecodeItem;
                        dataToDecodeItem.data = (unsigned char *) dataToDecrypt;
                        dataToDecodeItem.len = dataToDecryptLength;

                        if(dataIsInBase64Format)
                        {
                            // TODO:
                            // Use some NSS util. to convert base64 to binary
                            OsSysLog::add(FAC_SIP, PRI_ERR,
                                "NSS decrypt of base64 S/MIME message not implemented");
                        }
                        else
                        {
                            // Decode the S/MIME blob
                            NSSCMSMessage *cmsMessage =
                                NSS_CMSMessage_CreateFromDER(&dataToDecodeItem,
                                                             nssOutToUtlString,
                                                             &decryptedData,
                                                             NULL, NULL,
                                                             NULL, NULL);


                            if(cmsMessage &&
                               decryptedData.length() > 0)
                            {
                                decryptSucceeded = TRUE;
                            }

                            // TODO:
                            // Remove the temporary private key from the
                            // database using the slot handle
                        }
                    }

                }
            }
        }
    }

    // Clean up
    if(p12Decoder)
    {
	    SEC_PKCS12DecoderFinish(p12Decoder);
    }
    if(uniPasswordItem.data)
    {
	    SECITEM_ZfreeItem(&uniPasswordItem, PR_FALSE);
    }

    ////// END WARNING   /////
#else
    OsSysLog::add(FAC_SIP, PRI_ERR, "SmimeBody::nssSmimeDecrypt invoked with ENABLE_NSS_SMIME not defined");
#endif

    return(decryptSucceeded);
}


UtlBoolean SmimeBody::convertPemToDer(UtlString& pemData,
                                      UtlString& derData)
{
    UtlBoolean conversionSucceeded = FALSE;
    derData.remove(0);

#ifdef ENABLE_NSS_SMIME
    // Code from NSS secutil.c

    char* body = NULL;
    char* pemDataPtr = (char*) pemData.data();

	/* check for headers and trailers and remove them */
	if ((body = strstr(pemDataPtr, "-----BEGIN")) != NULL) {
	    char *trailer = NULL;
	    pemData = strdup(body);
	    body = PORT_Strchr(body, '\n');
	    if (!body)
		body = PORT_Strchr(pemDataPtr, '\r'); /* maybe this is a MAC file */
	    if (body)
		trailer = strstr(++body, "-----END");
	    if (trailer != NULL) {
		*trailer = '\0';
	    } else {
		OsSysLog::add(FAC_SIP, PRI_ERR,
            "input has header but no trailer\n");
	    }
	} else {
	    body = pemDataPtr;
	}

	/* Convert to binary */
    SECItem derItem;
    derItem.data = NULL;
    derItem.len = 0;
	if(ATOB_ConvertAsciiToItem(&derItem, body))
    {
        OsSysLog::add(FAC_SIP, PRI_ERR,
            "error converting PEM base64 data to binary");
    }
    else
    {
        derData.append(((char*)derItem.data), derItem.len);
        conversionSucceeded = TRUE;
    }
#else
    OsSysLog::add(FAC_SIP, PRI_ERR,
        "SmimeBody::convertPemToDer implemented with NSS and OpenSSL disabled");
#endif

    return(conversionSucceeded);
}

/* ============================ ACCESSORS ================================= */

const HttpBody* SmimeBody::getDecyptedBody() const
{
    return(mpDecryptedBody);
}

/* ============================ INQUIRY =================================== */

UtlBoolean SmimeBody::isDecrypted() const
{
    return(mpDecryptedBody != NULL);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

/* ============================ FUNCTIONS ================================= */
