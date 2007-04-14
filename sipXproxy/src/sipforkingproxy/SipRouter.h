// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _SipRouter_h_
#define _SipRouter_h_

// SYSTEM INCLUDES


// APPLICATION INCLUDES
#include <os/OsServerTask.h>


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipUserAgent;
class ForwardRules;
class SipMessage;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class SipRouter : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   SipRouter(SipUserAgent& sipUserAgent, 
             ForwardRules& forwardingRules,
             bool          useAuthServer,
             const char*   authServer,
             bool          shouldRecordRoute
             );

   virtual ~SipRouter();

/* ============================ MANIPULATORS ============================== */

   virtual UtlBoolean handleMessage(OsMsg& rMsg);

   /// Modify the message as needed to be proxied
   bool proxyMessage(SipMessage& sipRequest);
   ///< @returns true if message should be sent, false if not

   void addAuthRoute(SipMessage& request);         

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    SipUserAgent* mpSipUserAgent;
    ForwardRules* mpForwardingRules;
    bool          mShouldRecordRoute;
    UtlString     mRecordRoute;

    bool          mAuthEnabled;
    UtlString     mAuthRoute;

    SipRouter(const SipRouter& rSipRouter);
    //:Copy constructor

    SipRouter& operator=(const SipRouter& rhs);
    //:Assignment operator
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipRouter_h_
