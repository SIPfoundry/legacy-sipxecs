//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ProvisioningAgentXmlRpcAdapter_h_
#define _ProvisioningAgentXmlRpcAdapter_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "net/XmlRpcMethod.h"

// DEFINES
#define EXPECTED_STRUCT_FAULT_CODE 1
#define EXPECTED_STRUCT_FAULT_STRING "Argument type error: expected a struct."

#define METHOD_DISPATCH_FAULT_CODE 2
#define METHOD_DISPATCH_FAULT_STRING "Failed to dispatch the target method procedure."

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlSList;
class UtlContainable;
class HttpRequestContext;
class XmlRpcDispatch;
class XmlRpcResponse;

/**
 * This class implements the mapping between the XmlRpc procedure "create" and
 * the ProvisioningAgent::create() member function.
 */
class ProvisioningAgentXmlRpcAdapter {
public:
/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

   /**
    * Default constructor
    */
   ProvisioningAgentXmlRpcAdapter(const ProvisioningAgent* pProvisioningAgent,
                                  int serverPort,
                                  bool secureTransport = false);

   /**
    * Destructor
    */
   virtual ~ProvisioningAgentXmlRpcAdapter();

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   XmlRpcDispatch* mpXmlRpcServer;


   ProvisioningAgentXmlRpcAdapter(const ProvisioningAgentXmlRpcAdapter& rProvisioningAgentXmlRpcAdapter);
   //:Copy constructor (not implemented for this class)

   ProvisioningAgentXmlRpcAdapter& operator=(const ProvisioningAgentXmlRpcAdapter& rhs);
   //:Assignment operator (not implemented for this class)
};


/**
 * This class implements the mapping between the XmlRpc procedure "create" and
 * the ProvisioningAgent::create() member function.
 */
class ProvisioningAgentXmlRpcCreate : public XmlRpcMethod {
public:
/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */
/**
 * Create a dynamic instance of this class and return the pointer to it.
 */
   static ProvisioningAgentXmlRpcCreate* get(void) {
      return new ProvisioningAgentXmlRpcCreate;
   }

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
/**
 * This is the actual method called by the underlying XmlRpc Dispatcher in
 * response to receiving a <methodCall>.  It will call the corresponding
 * ProvisioningAgent method whos instance is supplied in the
 * provisioningAgentInstance argument.
 */
   bool execute(const HttpRequestContext&      rContext,
                UtlSList&                      rParameters,
                void*                          pProvisioningAgentInstance,
                XmlRpcResponse&                rResponse,
                XmlRpcMethod::ExecutionStatus& rStatus);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};


/**
 * This class implements the mapping between the XmlRpc procedure "delete" and
 * the ProvisioningAgent::delete() member function.
 */
class ProvisioningAgentXmlRpcDelete : public XmlRpcMethod {
public:
/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */
/**
 * Create a dynamic instance of this class and return the pointer to it.
 */
   static ProvisioningAgentXmlRpcDelete* get(void) {
      return new ProvisioningAgentXmlRpcDelete;
   }

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
/**
 * This is the actual method called by the underlying XmlRpc Dispatcher in
 * response to receiving a <methodCall>.  It will call the corresponding
 * ProvisioningAgent method whos instance is supplied in the
 * provisioningAgentInstance argument.
 */
   bool execute(const HttpRequestContext&      rContext,
                UtlSList&                      rParameters,
                void*                          pProvisioningAgentInstance,
                XmlRpcResponse&                rResponse,
                XmlRpcMethod::ExecutionStatus& rStatus);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};


/**
 * This class implements the mapping between the XmlRpc procedure "set" and
 * the ProvisioningAgent::set() member function.
 */
class ProvisioningAgentXmlRpcSet : public XmlRpcMethod {
public:
/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */
/**
 * Create a dynamic instance of this class and return the pointer to it.
 */
   static ProvisioningAgentXmlRpcSet* get(void) {
      return new ProvisioningAgentXmlRpcSet;
   }

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
/**
 * This is the actual method called by the underlying XmlRpc Dispatcher in
 * response to receiving a <methodCall>.  It will call the corresponding
 * ProvisioningAgent method whos instance is supplied in the
 * provisioningAgentInstance argument.
 */
   bool execute(const HttpRequestContext&      rContext,
                UtlSList&                      rParameters,
                void*                          pProvisioningAgentInstance,
                XmlRpcResponse&                rResponse,
                XmlRpcMethod::ExecutionStatus& rStatus);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};


/**
 * This class implements the mapping between the XmlRpc procedure "get" and
 * the ProvisioningAgent::get() member function.
 */
class ProvisioningAgentXmlRpcGet : public XmlRpcMethod {
public:
/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */
/**
 * Create a dynamic instance of this class and return the pointer to it.
 */
   static ProvisioningAgentXmlRpcGet* get(void) {
      return new ProvisioningAgentXmlRpcGet;
   }

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
/**
 * This is the actual method called by the underlying XmlRpc Dispatcher in
 * response to receiving a <methodCall>.  It will call the corresponding
 * ProvisioningAgent method whos instance is supplied in the
 * provisioningAgentInstance argument.
 */
   bool execute(const HttpRequestContext&      rContext,
                UtlSList&                      rParameters,
                void*                          pProvisioningAgentInstance,
                XmlRpcResponse&                rResponse,
                XmlRpcMethod::ExecutionStatus& rStatus);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};


/**
 * This class implements the mapping between the XmlRpc procedure "action" and
 * the ProvisioningAgent::action() member function.
 */
class ProvisioningAgentXmlRpcAction : public XmlRpcMethod {
public:
/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */
/**
 * Create a dynamic instance of this class and return the pointer to it.
 */
   static ProvisioningAgentXmlRpcAction* get(void) {
      return new ProvisioningAgentXmlRpcAction;
   }

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
/**
 * This is the actual method called by the underlying XmlRpc Dispatcher in
 * response to receiving a <methodCall>.  It will call the corresponding
 * ProvisioningAgent method whos instance is supplied in the
 * provisioningAgentInstance argument.
 */
   bool execute(const HttpRequestContext&      rContext,
                UtlSList&                      rParameters,
                void*                          pProvisioningAgentInstance,
                XmlRpcResponse&                rResponse,
                XmlRpcMethod::ExecutionStatus& rStatus);

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

#endif  // _ProvisioningAgentXmlRpcAdapter_h_
