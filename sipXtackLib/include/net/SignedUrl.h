// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
//Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _SIGNEDURL_H_
#define _SIGNEDURL_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "utl/UtlBool.h"
#include "net/Url.h"
#include "os/OsDateTime.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipXauthIdentityTest;

/**
 * This class offers a collection of static methods that allows applications to sign
 * unsigned URLs and validate signed URLs.  URLs get signed by adding an extra URL parameter
 * called 'sipX-sig' that carries a signature computed using the following formula:
 * MD5(<secret><userinfo><hostport>)
 *   
 * Through the use of this class, a sipXecs component can generate a signed URL to be
 * used in headers such a Route, Path and Record-Route.   Other sipXecs components
 * can check the signature to ensure that URL-containing headers were added by     
 * a trusted sipXecs component.
 */

class SignedUrl
{
  public:
     /// Initialize the secret value used to sign the URL.
     static void setSecret(const char* secret /**< a random value used as input to sign the
                                               *   URL.  This should be chosen such that it:
                                               * - is hard for an attacker to guess (includes at
                                               *   least 32 bits of cryptographicaly random data)
                                               * - is the same for all components in a given installation
                                               */
                           );
     /**<
      * This must be called once at initialization time,
      * before any URL objects are signed.
      *
      * It may be called after that, but doing so with a
      * new value will invalidate any outstanding identities.
      */

     static void sign( Url& urlToSign ); /**< Reference to URL to be signed. */
     /**<
      * This static function adds a signature to the provided URL by adding a
      * sipX-sig URL parameter containing the signature.
      */
     
     static UtlBoolean isUrlSigned( Url& signedUrl ); /**< Reference to URL whose signature is
                                                           to be verified. */
     /**<
      * This static function verifies if a URL is signed and if it is, verifes if the
      * signature is valid.  When this function returns TRUE, a the calling application
      * can trust that the URL has been created by a trusted component of the system.
      */
     
  private:
     
     // @cond INCLUDENOCOPY
     // Class only contains static memebers, do not allow its instantiation. 
     SignedUrl(){};
     ~SignedUrl(){};
     SignedUrl(const SignedUrl& nocopyconstructor){};

     // There is no assignment operator.
     SignedUrl& operator=(const SignedUrl& noassignmentoperator);
     // @endcond     

     static void computeSignature( const UtlString& userInfo, const UtlString& hostPort, UtlString &signature );
     static const char* SignatureUrlParamName;
     static UtlString sSignatureSecret;
   
};

#endif // _SIGNEDURL_H_
