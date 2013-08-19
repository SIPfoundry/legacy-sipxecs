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

#ifndef _DomainValidator_h_
#define _DomainValidator_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsConfigDb.h"
#include "utl/UtlHashMap.h"
#include "net/Url.h"

class DomainValidator
{
public:
  DomainValidator(OsConfigDb* configDb, const UtlString& domainOption);
  ~DomainValidator();

  bool isValidDomain(const Url& uri) const;
  bool isValidDomain(const UtlString& host, int port) const;
  void addValidDomain(const UtlString& domain);
  void addValidDomain(const UtlString& host, int port);

private:
  void addDomainAliases();

  OsConfigDb* _configDb; ///< this is owned by the main routine - do not delete
  UtlString _defaultDomain;
  UtlHashMap _validDomains;
};

#endif // _DomainValidator_h_
