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

// SYSTEM INCLUDES
#include <assert.h>
#include <stdlib.h>

// APPLICATION INCLUDES
#include "os/OsLogger.h"
#include "os/OsSocket.h"
#include "statusserver/DomainValidator.h"
#include "net/SipMessage.h"
#include "net/NameValueTokenizer.h"
#include "sipXecsService/SipXecsService.h"

DomainValidator::DomainValidator(OsConfigDb* configDb, const UtlString& domainOption):
    _configDb(configDb)
{
  // get default domain from the given domain option (i.e. SIP_STATUS_DOMAIN_NAME)
  _configDb->get(domainOption, _defaultDomain);
   if (_defaultDomain.isNull())
   {
     OsSocket::getHostIp(&_defaultDomain);
     Os::Logger::instance().log(FAC_SIP, PRI_CRIT, "DomainValidator::DomainValidator "
                    "%s not configured using IP '%s'",
                    domainOption.data(), _defaultDomain.data());
   }
   else
   {
     Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "DomainValidator::DomainValidator "
                    "using domain '%s'",
                    _defaultDomain.data());
   }

   // make sure that the unspecified domain name is also valid
   addValidDomain(_defaultDomain);

   addDomainAliases();
}

void DomainValidator::addDomainAliases()
{
  // read the domain configuration
   OsConfigDb domainConfig;
   domainConfig.loadFromFile(SipXecsService::domainConfigPath());

   // Domain Aliases are other domain names that this component accepts as valid in the request URI
   UtlString domainAliases;
   domainConfig.get(SipXecsService::DomainDbKey::SIP_DOMAIN_ALIASES, domainAliases);

   if (!domainAliases.isNull())
   {
      Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "DomainValidator::DomainValidator "
                    "SIP_DOMAIN_ALIASES : %s", domainAliases.data());
   }
   else
   {
      Os::Logger::instance().log(FAC_SIP, PRI_ERR, "DomainValidator::DomainValidator "
                    "SIP_DOMAIN_ALIASES not found.");
   }

   UtlString aliasString;
   int aliasIndex = 0;
   while(NameValueTokenizer::getSubField(domainAliases.data(), aliasIndex,
                                         ", \t", &aliasString))
   {
      Url aliasUrl(aliasString);
      UtlString hostAlias;
      aliasUrl.getHostAddress(hostAlias);
      int port = aliasUrl.getHostPort();

      addValidDomain(hostAlias,port);
      aliasIndex++;
   }
}

bool DomainValidator::isValidDomain(const Url& uri) const
{
   bool isValid = false;

   UtlString domain;
   uri.getHostAddress(domain);
   domain.toLower();

   int port = uri.getHostPort();
   if (port == PORT_NONE)
   {
      port = SIP_PORT;
   }

   return isValidDomain(domain, port);
}

bool DomainValidator::isValidDomain(const UtlString& host, int port) const
{
   bool isValid = false;

   UtlString domain(host);
   domain.toLower();

   domain.append(":");
   domain.appendNumber(port);

   if ( _validDomains.contains(&domain) )
   {
      isValid = true;
      Os::Logger::instance().log(FAC_AUTH, PRI_DEBUG,
                    "DomainValidator::isValidDomain(%s) VALID",
                    domain.data()) ;
   }

   return isValid;
}

void DomainValidator::addValidDomain(const UtlString& domain)
{
  // get the url parts for the domain
   Url domainUrl(domain);
   UtlString domainHost;

   int domainPort = domainUrl.getHostPort();
   domainUrl.getHostAddress(domainHost);

   // make sure that the unspecified domain name is also valid
   addValidDomain(domainHost, domainPort);
}

void DomainValidator::addValidDomain(const UtlString& host, int port)
{
   UtlString* valid = new UtlString(host);
   valid->toLower();

   if (PORT_NONE == port)
   {
     port = SIP_PORT;
   }

   valid->append(":");
   valid->appendNumber(port);

   Os::Logger::instance().log(FAC_AUTH, PRI_DEBUG, "DomainValidator::addValidDomain(%s)", valid->data()) ;

   _validDomains.insert(valid);
}

DomainValidator::~DomainValidator()
{
  _validDomains.destroyAll();
}
