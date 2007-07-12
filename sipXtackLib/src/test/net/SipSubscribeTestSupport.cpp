//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#include "SipSubscribeTestSupport.h"

/* Support routines for the SipSubscribeServer and SipSubscribeClient tests. */

// Create a SipUserAgent.
void createTestSipUserAgent(UtlString& hostIp,
                            const char* user,
                            SipUserAgent*& userAgentp,
                            UtlString& aor_addr_spec,
                            UtlString& aor_name_addr,
                            UtlString& resource_id
                            )
{
   // Construct a user agent.
   // Listen on only a TCP port, as we do not want to have to specify a
   // fixed port number (to allow multiple executions of the test),
   // and the code to select a port automatically cannot be told to
   // open matching UDP and TCP ports.
   userAgentp = new SipUserAgent(PORT_DEFAULT, PORT_NONE, PORT_NONE,
                                 NULL, NULL, hostIp);
   userAgentp->start();

   // Construct the URI of the notifier, which is also the URI of
   // the subscriber.
   // Also construct the name-addr version of the URI, which may be
   // different if it has a "transport" parameter.
   // And the resource-id to use, which is the AOR with any
   // parameters stripped off.
   {
      char buffer[100];
      sprintf(buffer, "sip:%s@%s:%d", user, hostIp.data(),
              userAgentp->getTcpPort());
      resource_id = buffer;

      // Specify TCP transport, because that's all the UA listens to.
      // Using an address with a transport parameter also exercises
      // SipDialog to ensure it handles contact addresses with parameters.
      strcat(buffer, ";transport=tcp");
      aor_addr_spec = buffer;

      Url aor_uri(buffer, TRUE);
      aor_uri.toString(aor_name_addr);
   }

   // Start the SipUserAgent running.
   userAgentp->start();
}
