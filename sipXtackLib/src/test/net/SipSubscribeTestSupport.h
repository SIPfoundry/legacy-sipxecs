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
                            /// Aor as name-addr for a Contact header (no user part)
                            UtlString& aor_contact_name_addr,
                            /// Resource-ID (sip:user@hostport)
                            UtlString& resource_id
   );

// Service routine to listen for messages.
// Wait for messages.  Record any request and response received.
// Send response to any request received.
void runListener(OsMsgQ& msgQueue, ///< OsMsgQ to listen on
                                   //  Must have observer to userAgent set up.
                 SipUserAgent& userAgent, ///< SipUserAgent to send responses
                 OsTime timeout_first,
                 ///< Length of time to wait for the first message.
                 OsTime timeout_next,
                 ///< Length of time after last message to wait for further messages.
                 const SipMessage*& request,
                 ///< Pointer to any request that was received
                 const SipMessage*& response,
                 ///< Pointer to any response that was received
                 int responseCode,
                 ///< Response code to give to requests
                 UtlBoolean retry,
                 ///< TRUE to add "Retry-After: 0" to responsees
                 int expires,
                 ///< Expires value to add to SUBSCRIBE responses
                 UtlString* toTagp,
                 ///< To-tag that was created or used for response, or NULL
                 const SipMessage** request2p = NULL
                 ///< If non-NULL, allows a second request to be received,
                 ///  and is set to point to such a second request, if received.
   );

#endif // _SipSubscribeSupport_
