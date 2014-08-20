/*
 * Copyright (c) eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */

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

#include <sipdb/EntityDB.h>



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

  AuthInformationGrabber(WSRouter* pRouter, UserInfoCache* pUserInfoCache = 0, jsonrpc::Client* pRpc = 0, const char* userCacheFile = 0);
  
  virtual ~AuthInformationGrabber();
      
  virtual bool process(resip::ApplicationMessage* msg);

  virtual repro::UserAuthGrabber* clone() const;
  
  bool getCachedAuthInfo(const resip::Data& user, const resip::Data& realm, AuthInfoRecord& rec);

  void setCachedAuthInfo(const resip::Data& user, const resip::Data& realm, const AuthInfoRecord& rec);
  
  bool getUserAuthInfo(const resip::Data& user, const resip::Data& realm, resip::Data& a1Hash);
  
  bool getSipPassword(const resip::Data& user, const resip::Data& realm, resip::Data& password);
  
  bool getSipPasswordFromDb(const resip::Data& user, const resip::Data& realm, resip::Data& password);

  bool loadCacheFromFile(const std::string& cacheFile);
private:
  WSRouter* _pRouter; 
  UserInfoCache* _pUserInfoCache;
  bool _canDeleteCache;
  std::string _rpcUrl;
  jsonrpc::Client* _pRpc;
  bool _canDeleteRpc;
  EntityDB* _pEntityDb;
};


#endif	/* AUTHINFORMATIONGRABBER_H */

