//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
////////////////////////////////////////////////////////////////////////
//////


#ifndef _SmimeBody_h_
#define _SmimeBody_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <net/HttpBody.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//! class to contain an PKCS7 (S/MIME) body
/*! This class can be used to create an encrypted S/MIME
    body as well as to decrypt an encrypted body.
 */
class SmimeBody : public HttpBody
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    enum ContentEncoding
    {
        SMIME_ENODING_UNKNOWN = 0,
        SMIME_ENODING_BINARY,
        SMIME_ENODING_BASE64
    };

/* ============================ CREATORS ================================== */

    //! default constructor
    SmimeBody();

    //! Construct an SmimeBody from a bunch of bytes
    SmimeBody(const char* bytes,
              int length,
              const char* contentEncodingValueString);

    //! Copy constructor
    SmimeBody(const SmimeBody& rSmimeBody);

    //! Anonymous copy used from HttpBody
    virtual SmimeBody* copy() const;

    //! Destructor
    virtual
    ~SmimeBody();

/* ============================ MANIPULATORS ============================== */

    //! Assignment operator
    SmimeBody& operator=(const SmimeBody& rhs);

    //! Decrypt this body using the given private key and cert. contained in the pkcs12 package
    /*! Decrypts this body using the derPkcs12PrivateKey.
     *  \param derPkcs12 - DER format pkcs12 container for the
     *         private key and public key/Certificate for a recipent who is
     *         allowed to decrypt this pkcs7 (S/MIME) encapsulated body.
     *  \param derPkcs12Length - length in bytes of derPkcs12PrivateKey
     *  \param pkcs12SymmetricKey - symetric key (password) used to protect
     *         (encrypt) the derPkcs12PrivateKey (the private key is
     *         contained in a pkcs12 in an encrypted format to protect
     *         it from theft).  Must be NULL terminated string.
     */
    UtlBoolean decrypt(const char* derPkcs12,
                      int derPkcs12Length,
                      const char* pkcs12SymmetricKey);

    //! Encrypt the given body for the given list of recipients
    /*! \param bodyToEncrypt - Body to encrypt, note bodyToEncrypt
    *         will be attached to and deleted with this SmimeBody.
    *         bodyToEncrypt can be retrieved after invoking decrypt
    *         using the getDecyptedBody method.
    *  \param numRecipients - number of recipients for which
    *         bodyToEncrypt will be encrypted.  For each recipient
    *         an element in derPublicKeyCerts and derPubliceKeyCertLengths
    *         must be given.
    *  \param derPublicKeyCerts - array containing a DER format
    *         certificate (containing the public key) for each recipient.
    *  \param derPubliceKeyCertLengths - length in bytes of the
    *         corresponding element in derPublicKeyCerts.
    */
    UtlBoolean encrypt(HttpBody* bodyToEncrypt,
                      int numRecipients,
                      const char* derPublicKeyCerts[],
                      int derPubliceKeyCertLengths[]);


    // Lower level utility to do S/MIME encryption using the NSS library.
    /*! Encrypts the given data for the recipients represented by the
     *  array of certificates containing the public keys.
     *  \param numRecipientCerts the number of recipient certificates in
     *         the derPublicKeyCerts array.
     *  \param derPublicKeyCerts - array containing DER format certificates.
     *  \param derPublicKeyCertLengths - array containing the length of the
     *         corresponding certificate DER data.
     *  \param dataToEncrypt - raw data to encrypt using PKCS7 S/MIME format
     *  \param dataToEncryptLength length in bytes of dataToEncrypt
     *  \param encryptedDataInBase64Format - TRUE: output encrypted data in
     *         base64 format, FALSE: output data in raw binary format.  Typically
     *         for SIP one should send in binary format.
     *  \param encryptedData - string containing the encrypted result.
     */
    static UtlBoolean nssSmimeEncrypt(int numRecipientCerts,
                                       const char* derPublicKeyCerts[],
                                       int derPublicKeyCertLengths[],
                                       const char* dataToEncrypt,
                                       int dataToEncryptLength,
                                       UtlBoolean encryptedDataInBase64Format,
                                       UtlString& encryptedData);

    // Lower level utility to do S/MIME decryption using the NSS library
    /*! Decrypts this body using the derPkcs12PrivateKey.
     *  \param derPkcs12 - DER format pkcs12 container for the
     *         private key and public key/Certificate for a recipent who is
     *         allowed to decrypt this pkcs7 (S/MIME) encapsulated body.
     *  \param derPkcs12Length - length in bytes of derPkcs12PrivateKey
     *  \param pkcs12Password - symetric key (password) used to protect
     *         (encrypt) the derPkcs12PrivateKey (the private key is
     *         contained in a pkcs12 in an encrypted format to protect
     *         it from theft).  Must be NULL terminated string.
     *  \param dataIsInBase64Format - TRUE: encrypted data is in base64
     *         format, FALSE: encrypted data is in binary format.
     *  \param dataToDecrypt - raw data to be decrypted. Must be in binary
     *         or base64 format.  Does NOT need to be NULL terminated.
     *  \param dataToDecryptLength - length of the data in dataToDecrypt
     *         to be decrypted.
     *  \param decryptedData - string to contain the resulting decrypted
     *         data.
     */
    static UtlBoolean nssSmimeDecrypt(const char* derPkcs12,
                                       int derPkcs12Length,
                                       const char* pkcs12Password,
                                       UtlBoolean dataIsInBase64Format,
                                       const char* dataToDecrypt,
                                       int dataToDecryptLength,
                                       UtlString& decryptedData);

    // Lower level utility to do S/MIME decryption using the OpenSSL library
    /*! Decrypts this body using the derPkcs12PrivateKey.
     *  \param derPkcs12 - DER format pkcs12 container for the
     *         private key and public key/Certificate for a recipent who is
     *         allowed to decrypt this pkcs7 (S/MIME) encapsulated body.
     *  \param derPkcs12Length - length in bytes of derPkcs12PrivateKey
     *  \param pkcs12Password - symetric key (password) used to protect
     *         (encrypt) the derPkcs12PrivateKey (the private key is
     *         contained in a pkcs12 in an encrypted format to protect
     *         it from theft).  Must be NULL terminated string.
     *  \param dataIsInBase64Format - TRUE: encrypted data is in base64
     *         format, FALSE: encrypted data is in binary format.
     *  \param dataToDecrypt - raw data to be decrypted. Must be in binary
     *         or base64 format.  Does NOT need to be NULL terminated.
     *  \param dataToDecryptLength - length of the data in dataToDecrypt
     *         to be decrypted.
     *  \param decryptedData - string to contain the resulting decrypted
     *         data.
     */
    static UtlBoolean opensslSmimeDecrypt(const char* derPkcs12,
                                   int derPkcs12Length,
                                   const char* pkcs12Password,
                                   UtlBoolean dataIsInBase64Format,
                                   const char* dataToDecrypt,
                                   int dataToDecryptLength,
                                   UtlString& decryptedData);

    //! Utility to convert PEM format data to DER format
    static UtlBoolean convertPemToDer(UtlString& pemData,
                                      UtlString& derData);

/* ============================ ACCESSORS ================================= */

    //! Gets the decrypted form of this body if available
    const HttpBody* getDecyptedBody() const;

/* ============================ INQUIRY =================================== */

   //! Query if this body has been decrypted
   UtlBoolean isDecrypted() const;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   HttpBody* mpDecryptedBody;
   enum ContentEncoding mContentEncoding;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SmimeBody_h_
