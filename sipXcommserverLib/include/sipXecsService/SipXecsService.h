// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////
#ifndef _SIPXECSSERVICE_H_
#define _SIPXECSSERVICE_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "os/OsFS.h"
#include "os/OsConfigDb.h"

// DEFINES
// TYPEDEFS
typedef const char* const DirectoryType;

// CONSTANTS

/// Superclass for common features of all sipXecs services
/**
 * This class provides for the common features of sipXecs service processes.
 */
class SipXecsService
{
  public:

   static DirectoryType ConfigurationDirType;
   static DirectoryType LogDirType;
   static DirectoryType RunDirType;
   static DirectoryType TmpDirType;
   static DirectoryType DatabaseDirType;

   static OsPath Path(DirectoryType pathType, const char* fileName);

   /// name of the configuration common to all services in the domain.
   static OsPath domainConfigPath(); 
   
   /// lookup keys for the domain configuration
   class DomainDbKey
   {
     public:
      static const char* SIP_DOMAIN_NAME; ///< the name of the SIP domain for this sipXecs
      static const char* SHARED_SECRET;   ///< shared secret for generating authentication hashes
   };

  protected:

   /// constructor
   SipXecsService(const char* serviceName);

   /// destructor
   virtual ~SipXecsService();

  private:

   static const char* defaultDir(DirectoryType pathType);
   
   static const char* DefaultConfigurationDir;
   static const char* DefaultLogDir;
   static const char* DefaultRunDir;
   static const char* DefaultTmpDir;
   static const char* DefaultDatabaseDir;

   UtlString  mServiceName;

   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   SipXecsService(const SipXecsService& nocopyconstructor);

   /// There is no assignment operator.
   SipXecsService& operator=(const SipXecsService& noassignmentoperator);
   // @endcond     
};

#endif // _SIPXECSSERVICE_H_
