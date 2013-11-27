#ifndef AUTHIDENTITYENCODER_H_INCLUDED
#define	AUTHIDENTITYENCODER_H_INCLUDED

#include <string>


namespace sipx {
namespace proxy {
  

class AuthIdentityEncoder
{
public:
  static void setSecretKey(const std::string& secretKey);
  static bool encodeAuthority(const std::string& identity, const std::string& callId, const std::string& fromTag, std::string& authority);
};


} } // sipx::proxy

#endif	/* AUTHIDENTITYENCODER_H */

