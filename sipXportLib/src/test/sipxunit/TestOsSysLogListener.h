//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _TESTOSSYSLOGLISTENER_H_
#define _TESTOSSYSLOGLISTENER_H_

// SYSTEM INCLUDES
#include <cppunit/Portability.h>
#include <cppunit/TestListener.h>

// APPLICATION INCLUDES

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class Test;

/// One line description
/**
 * Long description
 */
class TestOsSysLogListener : public CPPUNIT_NS::TestListener
{
  public:
   /*! Constructs a TextTestProgressListener object.
    */
   TestOsSysLogListener();

   /// Destructor.
   virtual ~TestOsSysLogListener();

   virtual void startTest( CPPUNIT_NS::Test *test );

   virtual void endTest( CPPUNIT_NS::Test *test );

  private:

   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   TestOsSysLogListener(const TestOsSysLogListener& nocopyconstructor);

   /// There is no assignment operator.
   TestOsSysLogListener& operator=(const TestOsSysLogListener& noassignmentoperator);
   // @endcond
};

#endif // _TESTOSSYSLOGLISTENER_H_
