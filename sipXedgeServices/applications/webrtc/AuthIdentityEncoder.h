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

