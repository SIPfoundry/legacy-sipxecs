//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlHashMap.h>
#include <net/ProvisioningAgent.h>
#include <net/ProvisioningAgentXmlRpcAdapter.h>
#include "cp/SipPresenceMonitor.h"
#include "cp/XmlRpcSignIn.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        XmlRpcSignIn::XmlRpcSignIn
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

XmlRpcSignIn::XmlRpcSignIn(SipPresenceMonitor* pSipPresenceMonitor, int signInPort)
: ProvisioningClass("login")
{
   mpSipPresenceMonitor = pSipPresenceMonitor;

   // Start up the Provisioning Agent
   mpProvisioningAgent = new ProvisioningAgent();

   // Bind the Provisioning Agent to the XmlRpc Adapter
   mpProvisioningAgentXmlRpcAdapter = new ProvisioningAgentXmlRpcAdapter(mpProvisioningAgent, signInPort, false);

   // Register this class with the Provisioning Agent
   mpProvisioningAgent->registerClass(this);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        XmlRpcSignIn::~XmlRpcSignIn
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

XmlRpcSignIn::~XmlRpcSignIn()
{
   delete mpProvisioningAgentXmlRpcAdapter;
   delete mpProvisioningAgent;
}

/* ============================ MANIPULATORS ============================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        XmlRpcSignIn::Action
//
//  SYNOPSIS:
//
//  DESCRIPTION:
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ProvisioningAttrList* XmlRpcSignIn::Action(ProvisioningAttrList& rRequestAttributes)
{
   ProvisioningAttrList* pResponse;
   UtlString             uriString;
   UtlString             status;

   osPrintf("{method} = action\n{object-class} = login\n");
   rRequestAttributes.dumpAttributes();
   osPrintf("\n");

   // Extract the action attribute from the request
   // operate on either sign-in or sign-out attributes
   if (rRequestAttributes.getAttribute(XMLRPC_SIGN_IN_TAG, uriString)) {
      // See if the Agent is not already signed-in
      Url contactUrl(uriString);
      mpSipPresenceMonitor->getState(contactUrl, status);
      if (status.compareTo(STATUS_OPEN) == 0) {
         // Already signed-in, return error
         pResponse = new ProvisioningAttrList;
         pResponse->setAttribute("method-name", "action");
         pResponse->setAttribute("result-code", ProvisioningAgent::SUCCESS);
         pResponse->setAttribute("result-text", "SUCCESS: User already signed-in");
         return pResponse;
      }

      // Sign-in the Agent
      mpSipPresenceMonitor->setStatus(contactUrl, StateChangeNotifier::PRESENT);

      // Build up the response.
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "action");
      pResponse->setAttribute("result-code", ProvisioningAgent::SUCCESS);
      pResponse->setAttribute("result-text", "SUCCESS: sign-in");
      return pResponse;
   }
   else if (rRequestAttributes.getAttribute(XMLRPC_SIGN_OUT_TAG, uriString)) {
      // See if the Agent is not already signed-out
      Url contactUrl(uriString);
      mpSipPresenceMonitor->getState(contactUrl, status);
      if (status.compareTo(STATUS_CLOSED) == 0) {
         // Already signed-out, return error
         pResponse = new ProvisioningAttrList;
         pResponse->setAttribute("method-name", "action");
         pResponse->setAttribute("result-code", ProvisioningAgent::SUCCESS);
         pResponse->setAttribute("result-text", "SUCCESS: User already signed-out");
         return pResponse;
      }

      // Sign-out the Agent
      mpSipPresenceMonitor->setStatus(contactUrl, StateChangeNotifier::AWAY);

      // Build up the response.
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "action");
      pResponse->setAttribute("result-code", ProvisioningAgent::SUCCESS);
      pResponse->setAttribute("result-text", "SUCCESS: sign-out");
      return pResponse;
   }
   else if (rRequestAttributes.getAttribute(XMLRPC_SIGN_IN_STATUS_TAG, uriString)) {
      // Get the Agent status
      Url contactUrl(uriString);
      mpSipPresenceMonitor->getState(contactUrl, status);

      // Build up the response.
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "action");
      pResponse->setAttribute("result-code", ProvisioningAgent::SUCCESS);
      pResponse->setAttribute("result-text", status);
      return pResponse;
   }
   else {
      // Unrecognized or missing action-object
      pResponse = new ProvisioningAttrList;
      pResponse->setAttribute("method-name", "action");
      pResponse->setAttribute("result-code", ProvisioningAgent::FAILURE);
      pResponse->setAttribute("result-text", "Invalid action operation");
      return pResponse;
   }
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
