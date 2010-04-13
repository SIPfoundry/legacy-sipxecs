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
#include "os/OsSysLog.h"

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
   static DirectoryType LocalStateDirType;
   static DirectoryType LogDirType;
   static DirectoryType RunDirType;
   static DirectoryType TmpDirType;
   static DirectoryType DatabaseDirType;
   static DirectoryType VarDirType;
   static DirectoryType DataDirType;
   static DirectoryType BinDirType;
   static DirectoryType LibExecDirType;
   static DirectoryType NameType;

   /// Get a full path for a file in the specified directory type
   static OsPath Path(DirectoryType pathType, const char* fileName = NULL);
   /**<
    * The returned path will concatentate the base directory type with the
    * OsPath;:separator and the file name (if either already contains the
    * separator where this concatenation would put it, then it is not inserted).
    *
    * If no filename is specified, or if it is the null string, then the
    * name of the directory is returned with no trailing separator.
    */

   /// name of the configuration common to all services in the domain.
   static OsPath domainConfigPath();

   /// lookup keys for the domain configuration
   class DomainDbKey
   {
     public:
      static const char* SIP_DOMAIN_NAME;  ///< the name of the SIP domain for this sipXecs
      static const char* SIP_DOMAIN_ALIASES;  ///< SIP domain aliases and IP address and FQDN for all servers.
      static const char* SIP_REALM;        ///< the realm value used to authenticate
      static const char* SHARED_SECRET;    ///< shared secret for generating authentication hashes
      static const char* DEFAULT_LANGUAGE; ///< default language used by voice applications
      static const char* SUPERVISOR_PORT;  ///< xml-rpc port for the sipXsupervisor
      static const char* CONFIG_HOSTS;     ///< host names that may control sipXsupervisor
   };

   /// Default user name for a sipXecs service
   static const char* User();

   /// Default process group name for a sipXecs service
   static const char* Group();

   /// Name for the sipXecs system (can be overridden by environment or configure)
   static const char* Name();

   /// Read the log level from the specified config file and set it for the current process
   static OsSysLogPriority setLogPriority(const char* configSettingsFile, ///< path to configuration file
                              const char* servicePrefix, /**< the string "_LOG_LEVEL" is appended
                                                          *   to this prefix to find the config
                                                          *   directive that sets the level */
                              OsSysLogPriority defaultLevel = PRI_NOTICE /**< used if no directive
                                                                          * is found, or the value
                                                                          * found is not a valid
                                                                          * level name */
                              );

   /// Read the log level from a preloaded OsConfigDb, set it for the current process, and return
   /// it so that it can be set explicitly for other facilities
   static OsSysLogPriority setLogPriority(const OsConfigDb& configSettings, ///< configuration data
                              const char* servicePrefix, /**< the string "_LOG_LEVEL" is appended
                                                          *   to this prefix to find the config
                                                          *   directive that sets the level */
                              OsSysLogPriority defaultLevel = PRI_NOTICE /**< used if no directive
                                                                          * is found, or the value
                                                                          * found is not a valid
                                                                          * level name */
                              );

  protected:

   /// Translate a log level name string to the enum value
   static bool decodeLogLevel(UtlString& logLevel, OsSysLogPriority& priority);
   ///< @returns true iff a valid translation was found.

   /// constructor
   SipXecsService(const char* serviceName);

   /// destructor
   virtual ~SipXecsService();

  private:

   static const char* defaultDir(DirectoryType pathType);

   static const char* DefaultConfigurationDir;
   static const char* DefaultLogDir;
   static const char* DefaultLocalStateDir;
   static const char* DefaultRunDir;
   static const char* DefaultTmpDir;
   static const char* DefaultDatabaseDir;
   static const char* DefaultVarDir;
   static const char* DefaultDataDir;
   static const char* DefaultBinDir;
   static const char* DefaultLibExecDir;
   static const char* DefaultUser;
   static const char* DefaultGroup;
   static const char* DefaultName;

   UtlString  mServiceName;

   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   SipXecsService(const SipXecsService& nocopyconstructor);

   /// There is no assignment operator.
   SipXecsService& operator=(const SipXecsService& noassignmentoperator);
   // @endcond
};

#endif // _SIPXECSSERVICE_H_
