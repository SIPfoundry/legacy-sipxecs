
#include <rutil/Data.hxx>
#include <rutil/MD5Stream.hxx>
#include <rutil/DataStream.hxx>
#include <resip/stack/Symbols.hxx>
#include <os/OsLogger.h>

#include "WSRouter.h"
#include "AuthInformationGrabber.h"

#define DEFAULT_CACHE_LIFETIME 3600 * 24

AuthInformationGrabber::AuthInformationGrabber(WSRouter* pRouter, UserInfoCache* pUserInfoCache, jsonrpc::Client* pRpc) :
  repro::UserAuthGrabber(pRouter->repro()->getReproConfig().getDataStore()->mUserStore),
  _pRouter(pRouter),
  _pUserInfoCache(pUserInfoCache),
  _canDeleteCache(false),
  _pRpc(pRpc),
  _canDeleteRpc(false)
{
  if (!_pUserInfoCache)
  {
    _pUserInfoCache = new UserInfoCache(DEFAULT_CACHE_LIFETIME * 1000);
    _canDeleteCache = true;
  }
  
  //
  // Initialize the RPC connector.  
  // TODO:  Make this highly available
  //
  if (!_pRpc && _pRouter->getOption("rpc-url", _rpcUrl))
  {
    OS_LOG_INFO(FAC_SIP, "Setting up RPC service to " << _rpcUrl);
    _pRpc = new jsonrpc::Client(new jsonrpc::HttpClient(_rpcUrl));
    _canDeleteRpc = true;
  }
}
  
AuthInformationGrabber::~AuthInformationGrabber()
{
  if (_canDeleteCache)
    delete _pUserInfoCache;
  
  if (_canDeleteRpc)  
    delete _pRpc;
}

bool AuthInformationGrabber::getSipPassword(const resip::Data& user, const resip::Data& realm, resip::Data& password)
{ 
  
  if (!_pRpc)
    return false;
  
  Json::Value params;
  params["user"] = user.c_str();
  params["realm"] = realm.c_str();
  
  try
  {
    Json::Value result;
    
    _pRpc->CallMethod("getSipPassword", params, result);
    
    if (!result.isMember("error") && result.isMember("password"))
    {
      password = result["password"].asCString();
    }
    else
    {
      OS_LOG_ERROR(FAC_SIP, "AuthInformationGrabber::getSipPassword User Not Found");
    }
  }
  catch (jsonrpc::JsonRpcException e)
  {
      OS_LOG_ERROR(FAC_SIP, "AuthInformationGrabber::getSipPassword FAILED with error: "  << e.what());
      return false;
  }
  
  return !password.empty();
}

bool AuthInformationGrabber::getUserAuthInfo(const resip::Data& user, const resip::Data& realm, resip::Data& a1Hash)
{
  resip::Data password;
  if (!getSipPassword(user, realm, password))
    return false;
  
  MD5Stream a1;
  a1 << user
     << Symbols::COLON
     << realm
     << Symbols::COLON
     << password;
  a1.flush();
  a1Hash = a1.getHex();
  
  AuthInfoRecord rec;
  rec.password = password.c_str();
  rec.a1 = a1Hash.c_str();
  
  setCachedAuthInfo(user, realm, rec);
  return true;
}

bool AuthInformationGrabber::getCachedAuthInfo(const resip::Data& user, const resip::Data& realm, AuthInfoRecord& rec)
{
  std::ostringstream identity;
  identity << user << "@" << realm;
   
  UserInfoPtr ptr = _pUserInfoCache->get(identity.str());
  if (ptr)
  {
    OS_LOG_DEBUG(FAC_SIP, _pUserInfoCache << " - AuthInformationGrabber::getCachedAuthInfo(" << identity.str() << ") - " <<  ptr->a1);
    rec = *ptr;
    return true;
  }
  
  OS_LOG_DEBUG(FAC_SIP, _pUserInfoCache << " - AuthInformationGrabber::getCachedAuthInfo(" << identity.str() << ") - No Entry");
  return false;
}

void AuthInformationGrabber::setCachedAuthInfo(const resip::Data& user, const resip::Data& realm, const AuthInfoRecord& rec)
{
  //
  // Update the cache
  //
  std::ostringstream identity;
  identity << user << "@" << realm;
  
  UserInfoPtr cacheData  = UserInfoPtr(new AuthInfoRecord(rec));
  _pUserInfoCache->add(identity.str(), cacheData);
  OS_LOG_DEBUG(FAC_SIP, "AuthInformationGrabber::setCachedAuthInfo(" << identity.str() << ", " << rec.a1 << ")");
}
      
bool AuthInformationGrabber::process(resip::ApplicationMessage* msg)
{
  repro::UserInfoMessage* uinf = dynamic_cast<UserInfoMessage*>(msg);    // auth for repro's DigestAuthenticator
  resip::UserAuthInfo* uainf = dynamic_cast<resip::UserAuthInfo*>(msg);  // auth for DUM's ServerAuthManager
  
  if(uinf)
  {
    getUserAuthInfo(uinf->user(), uinf->realm(), uinf->mRec.passwordHash);
    DebugLog(<<"AuthInformationGrabber Grabbed user info for " 
                   << uinf->user() <<"@"<<uinf->realm()
                   << " : " << uinf->A1());
    return true;
  }
  else if(uainf)
  {
    resip::Data a1Hash;
    if (getUserAuthInfo(uainf->getUser(), uainf->getRealm(), a1Hash))
      uainf->setA1(a1Hash);
    
    if(uainf->getA1().empty())
    {
       uainf->setMode(resip::UserAuthInfo::UserUnknown);
    }
    DebugLog(<<"AuthInformationGrabber Grabbed user info for " 
                   << uainf->getUser() <<"@"<<uainf->getRealm()
                   << " : " << uainf->getA1());
    return true;
  }
  else
  {
    WarningLog(<<"Did not recognize message type...");
  }
  return false;
}

repro::UserAuthGrabber* AuthInformationGrabber::clone() const
{
  return static_cast<repro::UserAuthGrabber*>(new AuthInformationGrabber(_pRouter, _pUserInfoCache, _pRpc));
}