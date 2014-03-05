#include <boost/algorithm/string.hpp>
#include <os/OsConfigDb.h>

#include "DomainConfig.h"

#define DOMAIN_CONFIG SIPX_CONFDIR "/domain-config"   
#define SIP_DOMAIN_NAME "SIP_DOMAIN_NAME"
#define SIP_REALM "SIP_REALM"
#define SHARED_SECRET "SHARED_SECRET"
#define SIP_DOMAIN_ALIASES "SIP_DOMAIN_ALIASES"
   
DomainConfig* DomainConfig::_pInstance = 0;
OsConfigDb gDomainConfig;

DomainConfig* DomainConfig::instance()
{
  if (!_pInstance)
    _pInstance = new DomainConfig();
  
  return _pInstance;
}

void DomainConfig::delete_instance()
{
  delete _pInstance;
}

static std::vector<std::string> string_tokenize(const std::string& str, const char* tok)
{
  std::vector<std::string> tokens;
  boost::split(tokens, str, boost::is_any_of(tok), boost::token_compress_on);
  return tokens;
}


static std::string get_config_string(const char* key)
{
  UtlString value;
  gDomainConfig.get(key, value);
  if (!value.isNull())
    return value.data();
  return std::string();
}

DomainConfig::DomainConfig()
{
  gDomainConfig.loadFromFile(DOMAIN_CONFIG);
}

DomainConfig::~DomainConfig()
{
  
}

std::string DomainConfig::getDomainName() const
{
  return get_config_string(SIP_DOMAIN_NAME);
}

std::vector<std::string> DomainConfig::getAliases() const
{
  std::string tokens = get_config_string(SIP_DOMAIN_ALIASES);
  std::vector<std::string> ret;
  
  if (!tokens.empty())
    ret = string_tokenize(tokens, ",");
  
  return ret;
}

std::string DomainConfig::getSharedSecret() const
{
  return get_config_string(SHARED_SECRET);
}

std::string DomainConfig::getRealm() const
{
  return get_config_string(SIP_REALM);
}




