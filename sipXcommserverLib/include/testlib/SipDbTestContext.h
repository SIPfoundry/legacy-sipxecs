//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _SIPDBTESTCONTEXT_H_
#define _SIPDBTESTCONTEXT_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "os/OsFS.h"
#include "testlib/FileTestContext.h"
// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Utilities for redirecting sipdb resources during unit tests.
/**
 * This class and its subclasses extend the test file abstraction provided by
 * FileTestContext to include the sipdb databases.
 */
class SipDbTestContext : public FileTestContext
{
  public:

   /// Define a context for a test or set of tests.
   SipDbTestContext( const char* testInputDir   ///< directory for test input & template files
                    ,const char* testWorkingDir ///< directory for test working files.
                    );
   /**<
    * In addition to setting the FileTestContext, this sets the testWorkingDir to be
    * the location of the files for the SIPDBManager and the persistent storage for all databases.
    */

   /// Destructor
   virtual ~SipDbTestContext();

  protected:

   virtual void setFastDbEnvironment();

  private:

   /// There is no copy constructor.
   SipDbTestContext(const SipDbTestContext&);

   /// There is no assignment operator.
   SipDbTestContext& operator=(const SipDbTestContext&);

};

#endif // _SIPDBTESTCONTEXT_H_
