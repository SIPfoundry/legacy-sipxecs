//
//
// Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _AppearanceAgentService_h_
#define _AppearanceAgentService_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include "sipXecsService/SipXecsService.h"
#include "utl/UtlString.h"
#include "AppearanceAgent.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


/**
 * The AppearanceAgentService class is a wrapper for the AppearanceAgent which implements
 * the SipXecsService interface.  This class initializes the SipXecsService, loads the config
 * from the <service>-config file, and runs until the shutdown flag is set.
 * It implements the resourceChanged callback to handle configuration change notifications
 * from the supervisor.
 */

class AppearanceAgentService : public SipXecsService
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   //! Construct a service wrapper for the Shared Appearance Agent.
   AppearanceAgentService(const char* serviceName, const char* servicePrefix, const char* version);

   ~AppearanceAgentService();

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

   UtlBoolean loadConfig(
         int& udpPort,
         int& tcpPort,
         UtlString& bindIp,
         UtlString& appearanceGroupFile,
         UtlString& domainName,
         UtlString& realm,
         int& refreshInterval,
         int& resubscribeInterval,
         int& minResubscribeInterval,
         int& seizedResubscribeInterval,
         int& serverMinExpiration,
         int& serverDefaultExpiration,
         int& serverMaxExpiration
         );

   /// The real Appearance Agent
   AppearanceAgent* mAppearanceAgent;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _AppearanceAgentService_h_
