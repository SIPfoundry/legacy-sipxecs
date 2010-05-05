//
//
// Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _MwiStatusService_h_
#define _MwiStatusService_h_

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


/**
 * The MwiStatusService class is a wrapper for the sipstatus service (Voicemail MWI)  which implements
 * the SipXecsService interface.  This class initializes the SipXecsService, loads the config
 * from the <service>-config file, and runs until the shutdown flag is set.
 * It implements the configChanged callbacks to handle configuration change notifications
 * from the supervisor.
 */

class MwiStatusService : public SipXecsService
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   //! Construct a service wrapper for the Server.
   MwiStatusService(const char* serviceName, const char* servicePrefix, const char* version);

   ~MwiStatusService();

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

   void closeIMDBConnections();

   UtlBoolean     mClosingIMDB;
   OsMutex        mLockMutex;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MwiStatusService_h_
