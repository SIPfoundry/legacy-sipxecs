// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <net/SipDialogMonitor.h>
#include "dmConfig.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Return a pointer to the XmlRpcMethod, creating it if necessary
AddExtension* AddExtension::get()
{
   return (new AddExtension());
}


// Copy constructor
AddExtension::AddExtension(const AddExtension& rAddExtension)
{
}

// Destructor
AddExtension::~AddExtension()
{
}

/* ============================ MANIPULATORS ============================== */


// Assignment operator
AddExtension&
AddExtension::operator=(const AddExtension& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;
}

/* ============================ ACCESSORS ================================= */

bool AddExtension::execute(const HttpRequestContext& requestContext,
                           UtlSList& params,
                           void* userData,
                           XmlRpcResponse& response,
                           XmlRpcMethod::ExecutionStatus& status)
{
   bool result = false;
   
   int totalParams = params.entries();
   if (totalParams > 2)
   {
      response.setFault(TOO_MANY_PARAMS_FAULT_CODE, TOO_MANY_PARAMS_FAULT_STRING);
      result = false;
   }
   else
   {
      UtlString groupName;
      UtlString extension;
      for (int index = 0; index < totalParams; index++)
      {
               
         UtlContainable *value = params.at(index);
         if (index == 0 || index == 1)
         {
            UtlString paramType(value->getContainableType());
            if (paramType.compareTo("UtlString") == 0)
            {
               if (index == 0)
               {
                  groupName = *((UtlString *)value);
               }
               else
               {
                  extension = *((UtlString *)value);
               }
               
               result = true;
            }
            else
            {
               response.setFault(ILLEGAL_PARAM_FAULT_CODE, ILLEGAL_PARAM_FAULT_STRING);
               result = false;
            }  
         }
      }
      
      if (result)
      {
         SipDialogMonitor* dialogMonitor = (SipDialogMonitor *) userData;
         
         Url extensionUrl(extension);
             
         dialogMonitor->addExtension(groupName, extensionUrl);
         
         status = XmlRpcMethod::OK;
         
         // Construct the response
         UtlString responseText("method call \"addExtension\" successful");
         response.setResponse(&responseText);
      }
   }
   
   return true;
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Constructor
AddExtension::AddExtension()
{
}

// Return a pointer to the XmlRpcMethod, creating it if necessary
RemoveExtension* RemoveExtension::get()
{
   return (new RemoveExtension());
}


// Copy constructor
RemoveExtension::RemoveExtension(const RemoveExtension& rRemoveExtension)
{
}

// Destructor
RemoveExtension::~RemoveExtension()
{
}

/* ============================ MANIPULATORS ============================== */


// Assignment operator
RemoveExtension&
RemoveExtension::operator=(const RemoveExtension& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;
}

/* ============================ ACCESSORS ================================= */

bool RemoveExtension::execute(const HttpRequestContext& requestContext,
                           UtlSList& params,
                           void* userData,
                           XmlRpcResponse& response,
                           XmlRpcMethod::ExecutionStatus& status)
{
   bool result = false;

   int totalParams = params.entries();
   if (totalParams > 2)
   {
      response.setFault(TOO_MANY_PARAMS_FAULT_CODE, TOO_MANY_PARAMS_FAULT_STRING);
      result = false;
   }
   else
   {
      UtlString groupName;
      UtlString extension;
      for (int index = 0; index < totalParams; index++)
      {
               
         UtlContainable *value = params.at(index);
         if (index == 0 || index == 1)
         {
            UtlString paramType(value->getContainableType());
            if (paramType.compareTo("UtlString") == 0)
            {
               if (index == 0)
               {
                  groupName = *((UtlString *)value);
               }
               else
               {
                  extension = *((UtlString *)value);
               }
               
               result = true;
            }
            else
            {
               response.setFault(ILLEGAL_PARAM_FAULT_CODE, ILLEGAL_PARAM_FAULT_STRING);
               result = false;
            }  
         }
      }
      
      if (result)
      {
         SipDialogMonitor* dialogMonitor = (SipDialogMonitor *) userData;
             
         Url extensionUrl(extension);

         dialogMonitor->removeExtension(groupName, extensionUrl);
         
         status = XmlRpcMethod::OK;
         
         // Construct the response
         UtlString responseText("method call \"removeExtension\" successful");
         response.setResponse(&responseText);
      }
   }
   
   return true;
}


/* //////////////////////////// PRIVATE /////////////////////////////////// */

// Constructor
RemoveExtension::RemoveExtension()
{
}

/* ============================ FUNCTIONS ================================= */

// Constructor
DialogMonitorConfig::DialogMonitorConfig(XmlRpcDispatch* dispatch, SipDialogMonitor* dialogMonitor)
{
   mpDispatch = dispatch;
   mpDispatch->addMethod(ADD_EXTENSION_METHOD, (XmlRpcMethod::Get *)AddExtension::get, (void *)dialogMonitor);
   mpDispatch->addMethod(REMOVE_EXTENSION_METHOD, (XmlRpcMethod::Get *)RemoveExtension::get, (void *)dialogMonitor);
}

// Copy constructor
DialogMonitorConfig::DialogMonitorConfig(const DialogMonitorConfig& rDialogMonitorConfig)
{
}

// Destructor
DialogMonitorConfig::~DialogMonitorConfig()
{
   mpDispatch->removeMethod(ADD_EXTENSION_METHOD);
   mpDispatch->removeMethod(REMOVE_EXTENSION_METHOD);
}

/* ============================ MANIPULATORS ============================== */


// Assignment operator
DialogMonitorConfig&
DialogMonitorConfig::operator=(const DialogMonitorConfig& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;
}
