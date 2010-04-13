//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _REGISTRATIONDBTESTCONTEXT_H_
#define _REGISTRATIONDBTESTCONTEXT_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "testlib/SipDbTestContext.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS


/// Provides test input data for registration database tests.
/**
 * The registration database test input is in the form of templates for the
 * registration.xml file.  These are normal files, except that the expires
 * values are relative numbers.  The ConvertRelativeExpirations TemplateConverter
 * turns these into absolute times relative to the time of conversion.
 */
class RegistrationDbTestContext : public SipDbTestContext
{
  public:

   /// constructor
   RegistrationDbTestContext( const char* testInputDir   ///< directory for test input & template files
                             ,const char* testWorkingDir ///< directory for test working files.
                             );

   virtual void inputFile(const char* filename);

   static void ConvertRelativeExpirations( OsFile* templateFile ///< input
                                          ,OsFile* workingFile  ///< output
                                          );

   /// destructor
   virtual ~RegistrationDbTestContext();

   static const char* REGDB_FILENAME;

  protected:

   static void timeShiftExpiresLine(UtlString& line, long timeNow);

  private:

   /// There is no copy constructor.
   RegistrationDbTestContext(const RegistrationDbTestContext&);

   /// There is no assignment operator.
   RegistrationDbTestContext& operator=(const RegistrationDbTestContext&);

};

#endif // _REGISTRATIONDBTESTCONTEXT_H_
