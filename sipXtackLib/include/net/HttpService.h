//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _HTTPSERVICE_H_
#define _HTTPSERVICE_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlContainableAtomic.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class HttpMessage;
class HttpRequestContext;

/**
 * A HttpService is a dynamically loaded object that is invoked by the HttpServer
 * during the runtime.
 *
 * This class is the abstract base from which all Http services must inherit. One
 * method must be implemented by the subclasses:
 * - processRequest() is for HttpServer to process the request and send back
 * the response to the client side
 *
 */

class HttpService : public UtlContainableAtomic
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   /// Constructor
   HttpService();

   /// Destructor
   virtual ~HttpService();

   /// Process request. Subclasses must provide a definition for this method.
   virtual void processRequest(const HttpRequestContext& requestContext,
                               const HttpMessage& request,
                               HttpMessage*& response
                               ) = 0;

   static const UtlContainableType TYPE;

   /// Get the ContainableType as a UtlContainable-derived class.
   virtual UtlContainableType getContainableType() const;

protected:

private:

   /// Disabled copy constructor
   HttpService(const HttpService& rHttpService);

   /// Disabled assignment operator
   HttpService& operator=(const HttpService& rhs);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _HTTPSERVICE_H_
