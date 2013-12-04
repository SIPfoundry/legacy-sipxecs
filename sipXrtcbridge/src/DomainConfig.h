#ifndef DOMAINCONFIG_H_INCLUDED
#define	DOMAINCONFIG_H_INCLUDED


#include <string>
#include <vector>

class DomainConfig
{

public:
  static DomainConfig* instance();
  static void delete_instance();
  
  std::string getDomainName() const;
  std::vector<std::string> getAliases() const;
  std::string getSharedSecret() const;
  std::string getRealm() const;
  
private:
  DomainConfig();
  ~DomainConfig();
  
  static DomainConfig* _pInstance;
};

#endif	// DOMAINCONFIG_H_INCLUDED

