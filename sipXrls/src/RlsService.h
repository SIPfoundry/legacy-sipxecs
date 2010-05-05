//
//
// Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _RlsService_h_
#define _RlsService_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include "sipXecsService/SipXecsService.h"
#include "utl/UtlString.h"
#include "ResourceListServer.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


/**
 * The RlsService class is a wrapper for the ResourceListServer which implements
 * the SipXecsService interface.  This class initializes the SipXecsService, loads the config
 * from the <service>-config file, and runs until the shutdown flag is set.
 * It implements the resourceChanged callback to handle configuration change notifications
 * from the supervisor.
 */

class RlsService : public SipXecsService
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   //! Construct a service wrapper for the ResourceListServer.
   RlsService(const char* serviceName, const char* servicePrefix, const char* version);

   ~RlsService();

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

   /// The real Resource List Server
   ResourceListServer* mResourceListServer;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _RlsService_h_
