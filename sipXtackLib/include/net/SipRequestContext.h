//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#ifndef _SipRequestContext_h_
#define _SipRequestContext_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <utl/UtlDList.h>


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//! Class Container for context variables for SIP request API
/*! The class is passed as an additional argument to contain
 *  context information that is not contained in the SIP request
 *  (which is also passed with this container).  Effectively this
 *  is supplimental informatiion to the SIP Request.
 *
 * The intention is for this to be used as part of standard
 * APIs that process SIP requests and generate SIP responses.
 * In those scenarios (e.g. the status server plugins, the
 * redirect server plugins) there is a need for a general
 * container that provides more information than is available
 * or perhaps easily accessable in the SIP request itself.
 * The specific API will need to provide specifics such as
 * is the request context for consumption only (i.e. read only)
 * or can the application add content to the request context
 * that may be used by subsequent applications consuming the
 * same SipRequestContext.
 */

class SipRequestContext
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   static const char* sAUTH_USER;
   static const char* sAUTH_REALM;
   static const char* sREQUEST_METHOD;
   static const char* sSERVER_DOMAIN;

/* ============================ CREATORS ================================== */

   //! Constructor
   /*! Construct a request context for an incoming request of
    * the given request method.
    * \par requestMethod - the request method for the given request.
    */
   SipRequestContext(const char* requestMethod = NULL);
     //:Default constructor


   virtual
   ~SipRequestContext();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

   //! Get context variables provided in this request context.
   /*! As it is possible to have multiple occurances of a named value
    *  the occurance argument indicates which occurance.  The default is
    *  the first.
    * \param name - the name of the variable to be retrieved from the
    * request context.  It is legal to have multiple values for the
    * same name.  Hense the optional argument occurance.  In applications
    * where the consumer of the SipRequestContext may also set values,
    * the application should use a names space convention to avoid
    * accedental collisions (e.g. for an ENUM pluging, it might use
    * a prefix of "ENUM." in front of all the variable names it adds
    * to the request context.
    * \param value - the value of the given variable name.
    * \param occurance - if there are more than one instance of the
    * same variable name in the SipRequestContext, occurance is an
    * integer that can be used to walk through and retrieve all of the
    * values.  Returns FALSE if the given occurance number does not exist.
    */
   UtlBoolean getVariable(const char* name,
                          UtlString& value,
                          int occurance = 0) const;


   //! Add a variable to the context.
   /*! If there are other instances of the same variable in the
    * context, the new one is added after the last occurance.
    */
   void addVariable( const char* name, const char* value);

   //! Get a string of all the values and names in the SipRequestContext
   /*! This is a debugging tool to dump the context.
    *  Returns the number of variables found.
    */
   int toString(UtlString& dumpString);

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   //! Get the name and value of the variable at the given index
   UtlBoolean getVariable(int index,
                             UtlString& name,
                             UtlString& value) const;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   SipRequestContext(const SipRequestContext& rSipRequestContext);
     //:Copy constructor
   SipRequestContext& operator=(const SipRequestContext& rhs);
     //:Assignment operator

   UtlDList mVariableList;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipRequestContext_h_
