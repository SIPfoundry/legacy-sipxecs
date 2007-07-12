//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _SipSubscribeSupport_
#define _SipSubscribeSupport_

#include <utl/UtlString.h>
#include <net/SipUserAgent.h>

/* Support routines for the SipSubscribeServer and SipSubscribeClient tests. */

// Create a SipUserAgent.
// Returned SipUserAgent is started.
// Caller must call shutdown() and delete it.
void createTestSipUserAgent(/// Host IP address or DNS name
                            UtlString& hostIp,
                            /// User name for its AOR.
                            const char* user,
                            /// Return pointer to SipUserAgent.
                            SipUserAgent*& userAgent,
                            /// AOR as addr-spec (URI)
                            UtlString& aor_addr_spec,
                            /// Aor as name-addr
                            UtlString& aor_name_addr,
                            /// Resource-ID (sip:user@hostport)
                            UtlString& resource_id
   );

#endif // _SipSubscribeSupport_
