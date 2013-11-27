#include "sipx/proxy/AuthIdentityEncoder.h"

#include <sstream>
#include <utl/UtlString.h>
#include <os/OsDateTime.h>
#include <os/OsTime.h>
#include <net/NetBase64Codec.h>
#include <net/NetMd5Codec.h>

const char* SignatureFieldSeparator = ":";
const char* SignatureUrlParamName = "signature";

namespace sipx {
namespace proxy {

static UtlString secret = "vhKlr4SPmO6t/ar8o3T3h0YH";

void AuthIdentityEncoder::setSecretKey(const std::string& secretKey)
{
  secret = secretKey;
}

bool AuthIdentityEncoder::encodeAuthority(const std::string& identity, const std::string& callId, const std::string& fromTag, std::string& authority)
{
  UtlString sSignatureSecret;
  if (!NetBase64Codec::decode(secret, sSignatureSecret))
      return false;
    
   // calculate timestamp
   OsDateTime timestamp;
   OsDateTime::getCurTime(timestamp);
   OsTime osTime;
   timestamp.cvtToTimeSinceEpoch(osTime);
   long seconds = osTime.seconds();
   char stamp[65];
   sprintf(stamp, "%lX", seconds);
   UtlString strSignature(stamp);

   strSignature.append(SignatureFieldSeparator);

   // signature-hash=MD5(<timestamp><secret><from-tag><call-id><identity>)
   NetMd5Codec signatureHash;
   signatureHash.hash(stamp);
   signatureHash.hash(sSignatureSecret);
   signatureHash.hash(fromTag.c_str());
   signatureHash.hash(callId.c_str());
   signatureHash.hash(identity.c_str());

   signatureHash.appendHashValue(strSignature);
   authority = strSignature.data();
   
   return true;
}

} } // sipx::proxy