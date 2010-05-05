//
//
// Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ParkService_h_
#define _ParkService_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include "sipXecsService/SipXecsService.h"
#include "utl/UtlString.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipLineMgr;


/**
 * The ParkService class is a wrapper for the sipXpark server which implements
 * the SipXecsService interface.  This class initializes the SipXecsService, loads the config
 * from the <service>-config file, and runs until the shutdown flag is set.
 * It implements the configChanged callbacks to handle configuration change notifications
 * from the supervisor.
 */

class ParkService : public SipXecsService
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   //! Construct a service wrapper for the Server.
   ParkService(const char* serviceName, const char* servicePrefix, const char* version);

   ~ParkService();

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
         int& UdpPort,
         int& TcpPort,
         int& RtpBase,
         UtlString& bindIp,
         int& MaxSessions,
         UtlBoolean& OneButtonBLF,
         UtlString&   domain,
         UtlString&   realm,
         UtlString&   user,
         SipLineMgr* lineMgr,
         int& Lifetime,
         int& BlindXferWait,
         int& KeepAliveTime
         );


};

/* ============================ INLINE METHODS ============================ */

#endif  // _ParkService_h_
