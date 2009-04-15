// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "os/OsSysLog.h"
#include "net/NetMd5Codec.h"

// APPLICATION INCLUDES
#include "net/SignedUrl.h"

// CONSTANTS
const char* SignedUrl::SignatureUrlParamName = "sipX-sig";

// STATIC VARIABLES
UtlString   SignedUrl::sSignatureSecret;

/// Initialize the secret value used to sign hashes.
void SignedUrl::setSecret(const char* secret )
{
   if (!sSignatureSecret.isNull() && sSignatureSecret.compareTo(secret))
   {
      OsSysLog::add(FAC_SIP,PRI_NOTICE,
                    "SignedUrl::setSecret called more than once;\n"
                    " previously signed URLs will now fail signature checks"
                    );
   }
   sSignatureSecret.remove(0);
   sSignatureSecret.append(secret);
}



void SignedUrl::sign( Url& urlToSign )
{
   UtlString existingUrlSignature;
   UtlString urlString;
   
   urlToSign.toString( urlString );
   if( urlToSign.getUrlParameter( SignatureUrlParamName, existingUrlSignature ) == TRUE )
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SignedUrl::sign URL '%s' already signed - updating signature",
                    urlString.data() );
   }
   UtlString userInfo;
   UtlString hostPort;
   UtlString strSignature;
   
   urlToSign.getUserId( userInfo );
   urlToSign.getHostWithPort( hostPort );
 
   computeSignature( userInfo, hostPort, strSignature );
   urlToSign.setUrlParameter( SignatureUrlParamName, strSignature.data() );

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SignedUrl::sign URL signed: '%s' with signature '%s'",
                 urlString.data(), strSignature.data() );
}

UtlBoolean SignedUrl::isUrlSigned( Url& signedUrl )
{
   UtlBoolean bUrlProperlySigned;
   UtlString existingUrlSignature;
   UtlString urlString;
   
   signedUrl.toString( urlString );
   if( signedUrl.getUrlParameter( SignatureUrlParamName, existingUrlSignature ) == FALSE )
   {
      bUrlProperlySigned = FALSE;
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SignedUrl::isUrlSigned URL '%s' not signed",
                    urlString.data() );
   }
   else
   {
      UtlString userInfo;
      UtlString hostPort;
      UtlString strReferenceSignature;
      
      signedUrl.getUserId( userInfo );
      signedUrl.getHostWithPort( hostPort );
      computeSignature( userInfo, hostPort, strReferenceSignature );

      if( strReferenceSignature.compareTo( existingUrlSignature ) == 0 )
      {
         bUrlProperlySigned = TRUE;
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                        "SignedUrl::isUrlSigned URL '%s' is properly signed",
                        urlString.data() );
      }
      else
      {
         bUrlProperlySigned = FALSE;
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                        "SignedUrl::isUrlSigned URL '%s' does not have a valid signature. "
                        "Expected signature: '%s'", urlString.data(), strReferenceSignature.data() );
      }
   }
   return bUrlProperlySigned;   
}

void SignedUrl::computeSignature( const UtlString& userInfo, 
                                  const UtlString& hostPort, 
                                  UtlString& signature )
{
   // signature-hash=MD5(<secret><userinfo><hostport>)
   NetMd5Codec signatureHash;

   signatureHash.hash( sSignatureSecret );
   if( !userInfo.isNull() )
   {
      signatureHash.hash( userInfo );
   }
   signatureHash.hash( hostPort );
   signature.remove(0);
   signatureHash.appendHashValue( signature );
}
