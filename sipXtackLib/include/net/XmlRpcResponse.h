//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _XMLRPCRESPPONSE_H_
#define _XMLRPCRESPPONSE_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <utl/UtlSList.h>
#include <xmlparser/tinyxml.h>
#include "net/Url.h"
#include "net/XmlRpcBody.h"

// DEFINES
#define ILL_FORMED_CONTENTS_FAULT_STRING "Ill-formed XML contents "
#define INVALID_ELEMENT_FAULT_STRING "Message not schema valid: "
#define UNREGISTERED_METHOD_FAULT_STRING "Method has not been registered "
#define AUTHENTICATION_REQUIRED_FAULT_STRING "Authentication is required"
#define EMPTY_PARAM_VALUE_FAULT_STRING "Empty param value "
#define CONNECTION_FAILURE_FAULT_STRING "Connection Failed"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// for backward compatibility with old #define codes
#define ILL_FORMED_CONTENTS_FAULT_CODE     XmlRpcResponse::IllFormedContents
#define INVALID_ELEMENT                    XmlRpcResponse::InvalidElement
#define UNREGISTERED_METHOD_FAULT_CODE     XmlRpcResponse::UnregisteredMethod
#define AUTHENTICATION_REQUIRED_FAULT_CODE XmlRpcResponse::AuthenticationRequired
#define EMPTY_PARAM_VALUE_FAULT_CODE       XmlRpcResponse::EmptyParameterValue

// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS

/**
 * This object is used to create a XML-RPC response to a XmlRpcRequest request.
 * setResponse() is used for creating the response, and getResponse() is
 * used for getting the value from the request. Furthermore, setFault() is for
 * creating a fault response, and getFault() is used for getting the fault code
 * and fault string in the fault response.
 *
 */

class XmlRpcResponse
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   /// Contruct a XML-RPC response
   XmlRpcResponse();

   /// Destructor
   virtual ~XmlRpcResponse();

   /// Fault code values.
   typedef enum
      {
         IllFormedContents = -1,      ///< xmlrpc message was not well formed xml
         InvalidElement = -2,         ///< message is not schema-valid - text should explain
         UnregisteredMethod = -3,     ///< no server found for requested method
         AuthenticationRequired = -4, ///< request was not properly authenticated
         EmptyParameterValue = -5,    ///< missing value for a required parameter
         ConnectionFailure = -6,      ///< unable to connect to service
         HttpFailure = -7             ///< http returned a non-2xx status
      } FaultCode;
   /**
    * Values used by this subsystem in the fault code;
    * Applications may use these or any integer value.
    */

/* ============================ MANIPULATORS ============================== */

   /// Set the method name (for logging and comment in return)
   void setMethod(const UtlString& methodName);

   /// Set the XML-RPC response
   bool setResponse(UtlContainable* value); ///< value for the response

   /// Set the fault code and fault string in a fault response
   bool setFault(int faultCode, const char* faultString);
   /**<
    * This function will create a fault response
    *
    */

   /// Get the XML-RPC response
   bool getResponse(UtlContainable*& value); ///< value for the param

   /// Get the fault code and fault string from the XML-RPC response
   void getFault(int* faultCode, UtlString& faultString);

   /// Parse the XML-RPC response
   bool parseXmlRpcResponse(UtlString& responseContent); ///< response content from XML-RPC request
   ///< @returns true for a good response, false for a fault (including unparsable response)

   /// Get the content of the response
   XmlRpcBody* getBody();

/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   /// The name of the method this is a response to
   UtlString   mMethod;

   /// XML-RPC body
   XmlRpcBody* mpResponseBody;

   /// Value for the XML-RPC response
   UtlContainable* mResponseValue;

   /// Fault code
   int mFaultCode;

   /// Fault string
   UtlString mFaultString;

   /// Disabled copy constructor
   XmlRpcResponse(const XmlRpcResponse& rXmlRpcResponse);

   /// Disabled assignment operator
   XmlRpcResponse& operator=(const XmlRpcResponse& rhs);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _XMLRPCRESPPONSE_H_
