//
//
// Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ProxyService_h_
#define _ProxyService_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include "sipXecsService/SipXecsService.h"
#include "net/SipUserAgent.h"
#include "utl/UtlString.h"
#include "ForwardRules.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


/**
 * The ProxyService class is a wrapper for the sipXproxy which implements
 * the SipXecsService interface.  This class initializes the SipXecsService, loads the config
 * from the <service>-config file, and runs until the shutdown flag is set.
 * It implements the configChanged callbacks to handle configuration change notifications
 * from the supervisor.
 */

class ProxyService : public SipXecsService
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   //! Construct a service wrapper for the Server.
   ProxyService(const char* serviceName, const char* servicePrefix, const char* version);

   ~ProxyService();

   /// Load the initial config and start the service
   void run();

   /// Implement config change callback for SipXecsService
   void configDbChanged(UtlString& configDbFile);

   /// Implement config file change callback for SipXecsService
   void resourceChanged(UtlString& fileType, UtlString& configFile);

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   UtlBoolean loadInitialConfig(
         int& proxyTcpPort,
         int& proxyUdpPort,
         int& proxyTlsPort,
         UtlString& bindIp,
         int& maxForwards,
         UtlString& domainName,
         UtlString& proxyRecordRoute,
         UtlString& routeName,
         UtlString& authScheme,
         UtlString& ipAddress,
         bool& enableCallStateLogObserver,
         bool& enableCallStateDbObserver,
         int& dnsSrvTimeout,
         int& maxNumSrvRecords,
         UtlString& callStateLogFileName,
         UtlBoolean& recurseOnlyOne300,
         UtlString& hostAliases,
         int&      staleTcpTimeout,
         UtlString& callStateDbHostName,
         UtlString& callStateDbName,
         UtlString& callStateDbUserName,
         UtlString& callStateDbDriver,
         ForwardRules& forwardingRules
         );

   void loadConfig();

   void closeIMDBConnections();

   SipUserAgent*  mpSipUserAgent;
   UtlBoolean     mClosingIMDB;
   OsMutex        mLockMutex;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _ProxyService_h_
