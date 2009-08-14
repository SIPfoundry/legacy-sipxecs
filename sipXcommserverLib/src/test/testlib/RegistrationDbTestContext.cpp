//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <memory>
#include <stdlib.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

// APPLICATION INCLUDES
#include "testlib/RegistrationDbTestContext.h"

// DEFINES
// CONSTANTS
const char* RegistrationDbTestContext::REGDB_FILENAME = "registration.xml";

// TYPEDEFS
// FORWARD DECLARATIONS

/// constructor
RegistrationDbTestContext::RegistrationDbTestContext( const char* testInputDir   ///< directory for test input & template files
                                               ,const char* testWorkingDir ///< directory for test working files.
                                               )
   : SipDbTestContext(testInputDir, testWorkingDir)
{
};

void RegistrationDbTestContext::inputFile(const char* filename)
{
   workingFileFromTemplate(filename, ConvertRelativeExpirations, REGDB_FILENAME);
}


void RegistrationDbTestContext::ConvertRelativeExpirations(
   OsFile* templateFile, ///< input
   OsFile* workingFile   ///< output
                                                           )
{
   // the files are already open
   UtlString line;
   long timeNow = OsDateTime::getSecsSinceEpoch();
   while (templateFile->readLine(line) == OS_SUCCESS)
   {
      timeShiftExpiresLine(line, timeNow);
      line.append("\n");
      size_t bytesWritten = 0;
      CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, workingFile->write(line.data(), line.length(), bytesWritten));
      CPPUNIT_ASSERT_EQUAL(line.length(), bytesWritten);
   }
   // the files will be closed by the wrapper
}

// If this is an expires line, then modify it by adding timeNow to the expiration time value.
// Otherwise don't mess with it.
void RegistrationDbTestContext::timeShiftExpiresLine(UtlString& line, long timeNow)
{
   const char* EXPIRES_BEGIN = "<expires>";
   const int EXPIRES_TAG_LENGTH = 9;
   const char* EXPIRES_END = "</expires>";
   ssize_t pos1, pos2;
   // If the line has an expiration value, then time-shift it
   if (((pos1 = line.index(EXPIRES_BEGIN)) != UTL_NOT_FOUND) &&
       ((pos2 = line.index(EXPIRES_END)) != UTL_NOT_FOUND))
   {
      pos1 += EXPIRES_TAG_LENGTH;    // skip past the tag, to the expires value
      CPPUNIT_ASSERT(pos2 > pos1);   // expires value cannot be empty
      UtlString timeText(line(pos1, pos2 - pos1));
      char* endptr = NULL;
      long timeNumber = strtol(timeText, &endptr, 10);
      CPPUNIT_ASSERT_EQUAL(*endptr, '\0');
      char newTime[20];          // room for up to a 64-bit number, may have minus sign
      int newTimeLen = sprintf(newTime, "%ld", timeNow + timeNumber);
      CPPUNIT_ASSERT(newTimeLen > 0);

      // Replace the old expiration value with the new shifted value
      UtlString lineEnd(line(pos2, line.length() - pos2));
      line.replace(pos1, newTimeLen, newTime);
      line.replace(pos1 + newTimeLen, lineEnd.length(), lineEnd);
   }
}

/// destructor
RegistrationDbTestContext::~RegistrationDbTestContext()
{
};
