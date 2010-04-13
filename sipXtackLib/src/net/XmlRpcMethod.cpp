//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <net/XmlRpcMethod.h>
#include <net/XmlRpcDispatch.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

XmlRpcMethod* XmlRpcMethod::get()
{
   OsSysLog::add(FAC_XMLRPC, PRI_CRIT, "XmlRpcMethod base class get factory called");
   assert(0);
   return NULL;
}

// Destructor
XmlRpcMethod::~XmlRpcMethod()
{
}


const char* XmlRpcMethod::ExecutionStatusString(ExecutionStatus value)
{
   const char* returnString;

   const char* StatusString[] =
   {
      "OK",
      "FAILED",
      "REQUIRE_AUTHENTICATION"
   };

   const char* OutOfRange = "<Invalid ExecutionStatus>";

   switch (value)
   {
   case OK:
   case FAILED:
   case REQUIRE_AUTHENTICATION:
      returnString = StatusString[value];
      break;

   default:
      returnString = OutOfRange;
      break;
   }

   return returnString;
}


// Constructor
XmlRpcMethod::XmlRpcMethod()
{
}

/* ============================ FUNCTIONS ================================= */
