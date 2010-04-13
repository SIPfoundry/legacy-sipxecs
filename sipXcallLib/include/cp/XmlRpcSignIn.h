//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _XmlRpcSignIn_h_
#define _XmlRpcSignIn_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <net/ProvisioningClass.h>
#include <net/ProvisioningAttrList.h>

// DEFINES
// Provisioning Tag Definitions
#define XMLRPC_SIGN_IN_TAG               "sign-in"
#define XMLRPC_SIGN_OUT_TAG              "sign-out"
#define XMLRPC_SIGN_IN_STATUS_TAG        "sign-in-status"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipPresenceMonitor;
class ProvisioningAgent;
class ProvisioningAgentXmlRpcAdapter;


class XmlRpcSignIn : public ProvisioningClass {
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   // Default constructor
   XmlRpcSignIn(SipPresenceMonitor* pSipPresenceMonitor, int signInPort);

   // Destructor
   ~XmlRpcSignIn();

/* ============================ MANIPULATORS ============================== */

   ProvisioningAttrList* Action(ProvisioningAttrList& rRequestAttributes);

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
   private:
   SipPresenceMonitor* mpSipPresenceMonitor;  // Reference to the associated AgentManager object
   ProvisioningAgent* mpProvisioningAgent;    // Reference to the underlying ProvisioningAgent object
   ProvisioningAgentXmlRpcAdapter* mpProvisioningAgentXmlRpcAdapter;  // Reference to the underlying Xml-Rpc adapter object.
};

#endif  // _XmlRpcSignIn_h_
