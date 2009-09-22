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
#include <os/OsSysLog.h>
#include "net/XmlRpcRequest.h"
#include "net/XmlRpcResponse.h"
#include "net/XmlRpcDispatch.h"
#include "net/ProvisioningAgent.h"
#include "net/ProvisioningAttrList.h"
#include "net/ProvisioningAgentXmlRpcAdapter.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningAgentXmlRpcAdapter::ProvisioningAgentXmlRpcAdapter
//
//  SYNOPSIS:
//
//  DESCRIPTION: Default constructor
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ProvisioningAgentXmlRpcAdapter::ProvisioningAgentXmlRpcAdapter(const ProvisioningAgent* pProvisioningAgent,
                                                               int serverPort,
                                                               bool secureTransport)
{
   mpXmlRpcServer = new XmlRpcDispatch(serverPort, secureTransport, "/RPC2");

   mpXmlRpcServer->addMethod("create", (XmlRpcMethod::Get*)ProvisioningAgentXmlRpcCreate::get, (void*)pProvisioningAgent);
   mpXmlRpcServer->addMethod("delete", (XmlRpcMethod::Get*)ProvisioningAgentXmlRpcDelete::get, (void*)pProvisioningAgent);
   mpXmlRpcServer->addMethod("set",    (XmlRpcMethod::Get*)ProvisioningAgentXmlRpcSet::get,    (void*)pProvisioningAgent);
   mpXmlRpcServer->addMethod("get",    (XmlRpcMethod::Get*)ProvisioningAgentXmlRpcGet::get,    (void*)pProvisioningAgent);
   mpXmlRpcServer->addMethod("action", (XmlRpcMethod::Get*)ProvisioningAgentXmlRpcAction::get, (void*)pProvisioningAgent);

   OsSysLog::add(FAC_ACD, PRI_DEBUG,
                 "Creating XmlRpcDispatch on port: %d(%s)",
                 serverPort, secureTransport ? "SSL" : "NON-SSL");
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningAgentXmlRpcAdapter::~ProvisioningAgentXmlRpcAdapter
//
//  SYNOPSIS:
//
//  DESCRIPTION: Destructor
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

ProvisioningAgentXmlRpcAdapter::~ProvisioningAgentXmlRpcAdapter()
{
   mpXmlRpcServer->removeMethod("create");
   mpXmlRpcServer->removeMethod("delete");
   mpXmlRpcServer->removeMethod("set");
   mpXmlRpcServer->removeMethod("get");
   mpXmlRpcServer->removeMethod("action");

   delete mpXmlRpcServer;
   mpXmlRpcServer= NULL;
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* ============================ RPC FUNCTIONS ============================= */

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningAgentXmlRpcCreate::execute
//
//  SYNOPSIS:
//
//  DESCRIPTION: This is the actual method called by the underlying XmlRpc Dispatcher in
//               response to receiving a <methodCall>.  It will call the corresponding
//               ProvisioningAgent Create method whos instance is supplied in the
//               provisioningAgentInstance argument.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool ProvisioningAgentXmlRpcCreate::execute(const HttpRequestContext&      rContext,
                                            UtlSList&                      rParameters,
                                            void*                          pProvisioningAgentInstance,
                                            XmlRpcResponse&                rResponse,
                                            XmlRpcMethod::ExecutionStatus& rStatus)
{
   rStatus = XmlRpcMethod::OK;

   // Extract the request argument structure from the head of the SList.
   UtlContainable *pRequestArgs = rParameters.at(0);

   // Verify that a parameter list was given
   if (pRequestArgs != NULL) {
      // Verify that the parameter list is a structure ("UtlHashMap").
      if (UtlString(pRequestArgs->getContainableType()) == "UtlHashMap") {
         // Now call the Provisioning Agent.
         ProvisioningAttrList requestAttributes(dynamic_cast<UtlHashMap*>(pRequestArgs));
         ProvisioningAttrList* pResponseAttributes;
         pResponseAttributes = ((ProvisioningAgent*)pProvisioningAgentInstance)->Create(requestAttributes);

         if (pResponseAttributes == NULL) {
            // Method failure.  Report error back to client
            rResponse.setFault(METHOD_DISPATCH_FAULT_CODE, METHOD_DISPATCH_FAULT_STRING);
         }
         else {
            // Encode the response
            rResponse.setResponse(dynamic_cast<UtlContainable*>(pResponseAttributes->getData()));

            // and clean up the responsettributes list.
            delete pResponseAttributes;
         }
      }
      else {
         // Bad parameter list.  Report error back to client
         rResponse.setFault(EXPECTED_STRUCT_FAULT_CODE, EXPECTED_STRUCT_FAULT_STRING);
      }
   }
   else {
      // Missing parameter list.  Report error back to client
      rResponse.setFault(EXPECTED_STRUCT_FAULT_CODE, EXPECTED_STRUCT_FAULT_STRING);
   }

   return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningAgentXmlRpcDelete::execute
//
//  SYNOPSIS:
//
//  DESCRIPTION: This is the actual method called by the underlying XmlRpc Dispatcher in
//               response to receiving a <methodCall>.  It will call the corresponding
//               ProvisioningAgent Delete method whos instance is supplied in the
//               provisioningAgentInstance argument.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool ProvisioningAgentXmlRpcDelete::execute(const HttpRequestContext&      rContext,
                                            UtlSList&                      rParameters,
                                            void*                          pProvisioningAgentInstance,
                                            XmlRpcResponse&                rResponse,
                                            XmlRpcMethod::ExecutionStatus& rStatus)
{
   rStatus = XmlRpcMethod::OK;

   // Extract the request argument structure from the head of the SList.
   UtlContainable *pRequestArgs = rParameters.at(0);

   // Verify that a parameter list was given
   if (pRequestArgs != NULL) {
      // Verify that the parameter list is a structure ("UtlHashMap").
      if (UtlString(pRequestArgs->getContainableType()) == "UtlHashMap") {
         // Now call the Provisioning Agent.
         ProvisioningAttrList requestAttributes(dynamic_cast<UtlHashMap*>(pRequestArgs));
         ProvisioningAttrList* pResponseAttributes;
         pResponseAttributes = ((ProvisioningAgent*)pProvisioningAgentInstance)->Delete(requestAttributes);

         if (pResponseAttributes == NULL) {
            // Method failure.  Report error back to client
            rResponse.setFault(METHOD_DISPATCH_FAULT_CODE, METHOD_DISPATCH_FAULT_STRING);
         }
         else {
            // Encode the response
            rResponse.setResponse(dynamic_cast<UtlContainable*>(pResponseAttributes->getData()));

            // and clean up the responsettributes list.
            delete pResponseAttributes;
         }
      }
      else {
         // Missing parameter list.  Report error back to client
         rResponse.setFault(EXPECTED_STRUCT_FAULT_CODE, EXPECTED_STRUCT_FAULT_STRING);
      }
   }
   else {
      // Bad parameter list.  Report error back to client
      rResponse.setFault(EXPECTED_STRUCT_FAULT_CODE, EXPECTED_STRUCT_FAULT_STRING);
   }

   return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningAgentXmlRpcSet::execute
//
//  SYNOPSIS:
//
//  DESCRIPTION: This is the actual method called by the underlying XmlRpc Dispatcher in
//               response to receiving a <methodCall>.  It will call the corresponding
//               ProvisioningAgent Set method whos instance is supplied in the
//               provisioningAgentInstance argument.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool ProvisioningAgentXmlRpcSet::execute(const HttpRequestContext&      rContext,
                                         UtlSList&                      rParameters,
                                         void*                          pProvisioningAgentInstance,
                                         XmlRpcResponse&                rResponse,
                                         XmlRpcMethod::ExecutionStatus& rStatus)
{
   rStatus = XmlRpcMethod::OK;

   // Extract the request argument structure from the head of the SList.
   UtlContainable *pRequestArgs = rParameters.at(0);

   // Verify that a parameter list was given
   if (pRequestArgs != NULL) {
      // Verify that the parameter list is a structure ("UtlHashMap").
      if (UtlString(pRequestArgs->getContainableType()) == "UtlHashMap") {
         // Now call the Provisioning Agent.
         ProvisioningAttrList requestAttributes(dynamic_cast<UtlHashMap*>(pRequestArgs));
         ProvisioningAttrList* pResponseAttributes;
         pResponseAttributes = ((ProvisioningAgent*)pProvisioningAgentInstance)->Set(requestAttributes);

         if (pResponseAttributes == NULL) {
            // Method failure.  Report error back to client
            rResponse.setFault(METHOD_DISPATCH_FAULT_CODE, METHOD_DISPATCH_FAULT_STRING);
         }
         else {
            // Encode the response
            rResponse.setResponse(dynamic_cast<UtlContainable*>(pResponseAttributes->getData()));

            // and clean up the responsettributes list.
            delete pResponseAttributes;
         }
      }
      else {
         // Missing parameter list.  Report error back to client
         rResponse.setFault(EXPECTED_STRUCT_FAULT_CODE, EXPECTED_STRUCT_FAULT_STRING);
      }
   }
   else {
      // Bad parameter list.  Report error back to client
      rResponse.setFault(EXPECTED_STRUCT_FAULT_CODE, EXPECTED_STRUCT_FAULT_STRING);
   }

   return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningAgentXmlRpcGet::execute
//
//  SYNOPSIS:
//
//  DESCRIPTION: This is the actual method called by the underlying XmlRpc Dispatcher in
//               response to receiving a <methodCall>.  It will call the corresponding
//               ProvisioningAgent Get method whos instance is supplied in the
//               provisioningAgentInstance argument.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool ProvisioningAgentXmlRpcGet::execute(const HttpRequestContext&      rContext,
                                         UtlSList&                      rParameters,
                                         void*                          pProvisioningAgentInstance,
                                         XmlRpcResponse&                rResponse,
                                         XmlRpcMethod::ExecutionStatus& rStatus)
{
   rStatus = XmlRpcMethod::OK;

   // Extract the request argument structure from the head of the SList.
   UtlContainable *pRequestArgs = rParameters.at(0);

   // Verify that a parameter list was given
   if (pRequestArgs != NULL) {
      // Verify that the parameter list is a structure ("UtlHashMap").
      if (UtlString(pRequestArgs->getContainableType()) == "UtlHashMap") {
         // Now call the Provisioning Agent.
         ProvisioningAttrList requestAttributes(dynamic_cast<UtlHashMap*>(pRequestArgs));
         ProvisioningAttrList* pResponseAttributes;
         pResponseAttributes = ((ProvisioningAgent*)pProvisioningAgentInstance)->Get(requestAttributes);

         if (pResponseAttributes == NULL) {
            // Method failure.  Report error back to client
            rResponse.setFault(METHOD_DISPATCH_FAULT_CODE, METHOD_DISPATCH_FAULT_STRING);
         }
         else {
            // Encode the response
            rResponse.setResponse(dynamic_cast<UtlContainable*>(pResponseAttributes->getData()));

            // and clean up the responsettributes list.
            delete pResponseAttributes;
         }
      }
      else {
         // Missing parameter list.  Report error back to client
         rResponse.setFault(EXPECTED_STRUCT_FAULT_CODE, EXPECTED_STRUCT_FAULT_STRING);
      }
   }
   else {
      // Bad parameter list.  Report error back to client
      rResponse.setFault(EXPECTED_STRUCT_FAULT_CODE, EXPECTED_STRUCT_FAULT_STRING);
   }

   return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  NAME:        ProvisioningAgentXmlRpcAction::execute
//
//  SYNOPSIS:
//
//  DESCRIPTION: This is the actual method called by the underlying XmlRpc Dispatcher in
//               response to receiving a <methodCall>.  It will call the corresponding
//               ProvisioningAgent Action method whos instance is supplied in the
//               provisioningAgentInstance argument.
//
//  RETURNS:     None.
//
//  ERRORS:      None.
//
//  CAVEATS:     None.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

bool ProvisioningAgentXmlRpcAction::execute(const HttpRequestContext&      rContext,
                                            UtlSList&                      rParameters,
                                            void*                          pProvisioningAgentInstance,
                                            XmlRpcResponse&                rResponse,
                                            XmlRpcMethod::ExecutionStatus& rStatus)
{
   rStatus = XmlRpcMethod::OK;

   // Extract the request argument structure from the head of the SList.
   UtlContainable *pRequestArgs = rParameters.at(0);

   // Verify that a parameter list was given
   if (pRequestArgs != NULL) {
      // Verify that the parameter list is a structure ("UtlHashMap").
      if (UtlString(pRequestArgs->getContainableType()) == "UtlHashMap") {
         // Now call the Provisioning Agent.
         ProvisioningAttrList requestAttributes(dynamic_cast<UtlHashMap*>(pRequestArgs));
         ProvisioningAttrList* pResponseAttributes;
         pResponseAttributes = ((ProvisioningAgent*)pProvisioningAgentInstance)->Action(requestAttributes);

         if (pResponseAttributes == NULL) {
            // Method failure.  Report error back to client
            rResponse.setFault(METHOD_DISPATCH_FAULT_CODE, METHOD_DISPATCH_FAULT_STRING);
         }
         else {
            // Encode the response
            rResponse.setResponse(dynamic_cast<UtlContainable*>(pResponseAttributes->getData()));

            // and clean up the responsettributes list.
            delete pResponseAttributes;
         }
      }
      else {
         // Missing parameter list.  Report error back to client
         rResponse.setFault(EXPECTED_STRUCT_FAULT_CODE, EXPECTED_STRUCT_FAULT_STRING);
      }
   }
   else {
      // Bad parameter list.  Report error back to client
      rResponse.setFault(EXPECTED_STRUCT_FAULT_CODE, EXPECTED_STRUCT_FAULT_STRING);
   }

   return true;
}
