// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _DMCONFIG_H_
#define _DMCONFIG_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <os/OsBSem.h>
#include <net/XmlRpcMethod.h>
#include <net/XmlRpcDispatch.h>

// DEFINES
#define ILLEGAL_PARAM_FAULT_CODE 1
#define ILLEGAL_PARAM_FAULT_STRING "Illegal param"

#define TOO_MANY_PARAMS_FAULT_CODE 2
#define TOO_MANY_PARAMS_FAULT_STRING "Too many params"

#define ADD_EXTENSION_METHOD "addExtension"
#define REMOVE_EXTENSION_METHOD "removeExtension"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipDialogMonitor;


/**
 * A AddExtension is a XmlRpcMethod that adds an extension to be monitored
 * by the dialog monitor. It has two parameters:
 * 
 * - groupName that the extension belongs to
 * - extensionUrl that specifies the extension
 * 
 */

class AddExtension : public XmlRpcMethod
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   /// Get the instance of this method.
   static AddExtension* get();
   
   /// Destructor
   virtual ~AddExtension();

/* ============================ MANIPULATORS ============================== */

   /// Execute the method.
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params, ///< request param list
                        void* userData, ///< user data
                        XmlRpcResponse& response, ///< request response
                        XmlRpcMethod::ExecutionStatus& status); ///< execution status
   
   
/* ============================ ACCESSORS ================================= */

   
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   /// Constructor
   AddExtension();

   /// Disabled copy constructor
   AddExtension(const AddExtension& rAddExtension);

   /// Disabled assignment operator
   AddExtension& operator=(const AddExtension& rhs);

};

/**
 * A RemoveExtension is a XmlRpcMethod that adds an extension to be monitored
 * by the dialog monitor. It has two parameters:
 * 
 * - groupName that the extension belongs to
 * - extensionUrl that specifies the extension
 * 
 */

class RemoveExtension : public XmlRpcMethod
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   /// Get the instance of this method.
   static RemoveExtension* get();
   
   /// Destructor
   virtual ~RemoveExtension();

/* ============================ MANIPULATORS ============================== */

   /// Execute the method.
   virtual bool execute(const HttpRequestContext& requestContext, ///< request context
                        UtlSList& params, ///< request param list
                        void* userData, ///< user data
                        XmlRpcResponse& response, ///< request response
                        XmlRpcMethod::ExecutionStatus& status); ///< execution status
   
   
/* ============================ ACCESSORS ================================= */

   
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   /// Constructor
   RemoveExtension();

   /// Disabled copy constructor
   RemoveExtension(const RemoveExtension& rRemoveExtension);

   /// Disabled assignment operator
   RemoveExtension& operator=(const RemoveExtension& rhs);

};


/**
 * A DialogMonitorConfig is a object that contains a group of XML-RPC methods
 * derived from XmlRpcMethod. This object provides the configuration ability
 * for ACD-like clients to provision the dialog monitor.
 * 
 */

class DialogMonitorConfig
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   /// Create a dispatch object.
   DialogMonitorConfig(XmlRpcDispatch* dispatch, SipDialogMonitor* dialogMonitor);

   /// Destructor.
   virtual ~DialogMonitorConfig();

/* ============================ MANIPULATORS ============================== */


/* ============================ ACCESSORS ================================= */

   
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   XmlRpcDispatch* mpDispatch;
  
   /// Disabled copy constructor
   DialogMonitorConfig(const DialogMonitorConfig& rDialogMonitorConfig);

   /// Disabled assignment operator
   DialogMonitorConfig& operator=(const DialogMonitorConfig& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _DMCONFIG_H_


