#ifndef AUTHINFORMATIONGRABBER_H_INCLUDED
#define	AUTHINFORMATIONGRABBER_H_INCLUDED

#include <jsonrpc/rpc.h>
#include <Poco/ExpireCache.h>

#include "rutil/Logger.hxx"
#include "repro/Worker.hxx"
#include "resip/stack/Message.hxx"
#include "repro/UserInfoMessage.hxx"
#include "repro/UserAuthGrabber.hxx"
#include "resip/dum/UserAuthInfo.hxx"


class WSRouter;

class AuthInformationGrabber : public repro::UserAuthGrabber
{
public:
  struct AuthInfoRecord
  {
    std::string password;
    std::string a1;
  };
  
  typedef Poco::ExpireCache<std::string, AuthInfoRecord> UserInfoCache;
  typedef Poco::SharedPtr<AuthInfoRecord> UserInfoPtr;

  AuthInformationGrabber(WSRouter* pRouter, UserInfoCache* pUserInfoCache = 0, jsonrpc::Client* pRpc = 0);
  
  virtual ~AuthInformationGrabber();
      
  virtual bool process(resip::ApplicationMessage* msg);

  virtual repro::UserAuthGrabber* clone() const;
  
  bool getCachedAuthInfo(const resip::Data& user, const resip::Data& realm, AuthInfoRecord& rec);

  void setCachedAuthInfo(const resip::Data& user, const resip::Data& realm, const AuthInfoRecord& rec);
  
  bool getUserAuthInfo(const resip::Data& user, const resip::Data& realm, resip::Data& a1Hash);
  
  bool getSipPassword(const resip::Data& user, const resip::Data& realm, resip::Data& password);
  
private:
  WSRouter* _pRouter; 
  UserInfoCache* _pUserInfoCache;
  bool _canDeleteCache;
  std::string _rpcUrl;
  jsonrpc::Client* _pRpc;
  bool _canDeleteRpc;
};


#endif	/* AUTHINFORMATIONGRABBER_H */

