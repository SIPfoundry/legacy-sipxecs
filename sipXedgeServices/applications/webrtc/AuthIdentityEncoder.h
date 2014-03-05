#ifndef AUTHIDENTITYENCODER_H_INCLUDED
#define	AUTHIDENTITYENCODER_H_INCLUDED

#include <string>

class AuthIdentityEncoder
{
public:
  static void setSecretKey(const std::string& secretKey);
  static bool encodeAuthority(const std::string& identity, const std::string& callId, const std::string& fromTag, std::string& authority);
};


#endif	/* AUTHIDENTITYENCODER_H */

