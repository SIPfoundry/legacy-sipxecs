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
#include "os/OsFS.h"
#include "sipXecsService/SipXecsService.h"
#include "sipxunit/TestUtilities.h"
#include "testlib/FileTestContext.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// constructor
FileTestContext::FileTestContext( const char* testInputDir
                                 ,const char* testWorkingDir
                                 )
   : mTestInputDir(testInputDir),
     mTestWorkingDir(testWorkingDir)
{
   makeCleanWorkingDir();
};

void FileTestContext::inputFile(const char* filename)
{
   UtlString inputPath;
   inputFilePath(filename, inputPath);

   UtlString workingPath;
   workingFilePath(filename, workingPath);

   OsPath input(inputPath);
   OsPath working(workingPath);

   UtlString msg;
   msg.append("copy failed from '");
   msg.append(inputPath);
   msg.append("' -> '");
   msg.append(workingPath);
   msg.append("'");
   CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(),
                                OS_SUCCESS,
                                OsFileSystem::copy(input, working)
                                );
}


void FileTestContext::workingFileFromTemplate(
   const char* templateName,     ///< testInputDir file
   TemplateConverter* converter, ///< conversion function
   const char* filename          ///< testWorkingDir file
                                               )
{
   /*
    * The template in the testInputDir and the filename in testWorkingDir are opened,
    * and then both files are passed to the converter.
    *
    * The converter is responsible for reading the template and writing the filename.
    *
    * If no filename is supplied, its name is the same as the templateName.
    */
   CPPUNIT_ASSERT(templateName != NULL);
   CPPUNIT_ASSERT(converter != NULL);

   // Open the template file
   UtlString templatePath;
   inputFilePath(templateName, templatePath);
   OsPath input(templatePath);
   OsFile inputFile(input);
   
   CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, inputFile.open(OsFile::READ_ONLY));

   // Create/open the registration DB file.
   UtlString workingPath;
   workingFilePath(filename ? filename : templateName, workingPath);
   OsPath working(workingPath);
   OsFile workingFile(working);
   CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, workingFile.open(OsFile::CREATE));
         
   // Let the converter convert the files
   converter(&inputFile, &workingFile);

   CPPUNIT_ASSERT_EQUAL(OS_SUCCESS, workingFile.flush());

   inputFile.close();
   workingFile.close();
}


void FileTestContext::removeWorkingFile(const char* filename)
{
   UtlString working;
   workingFilePath(filename, working);
   OsPath workingPath(working);
   OsFile testFile(workingPath);
   
   if (testFile.exists())
   {
      OsStatus removeStatus = OsFileSystem::remove( workingPath
                                                   ,TRUE // Recursive
                                                   ,TRUE // Force                                                   
                                                   );
      UtlString msg("failed to remove working file '");
      msg.append(working);
      msg.append("'");
      CPPUNIT_ASSERT_EQUAL_MESSAGE(msg.data(),OS_SUCCESS,removeStatus);
   }
}

void FileTestContext::makeCleanWorkingDir()
{
   UtlString makefilePath = mTestWorkingDir+OsPath::separator+"Makefile";
   OsPath makefileInWorkingDir(makefilePath);

   UtlString message;
   message.append("FileTestContext::makeCleanWorkingDir - the test working dir must be a new diretory\n"
                  "directory\n    ");
   message.append(mTestWorkingDir);
   message.append("\n  contains a Makefile, so it is not a good working directory");
   
   CPPUNIT_ASSERT_MESSAGE(message.data(), ! OsFileSystem::exists(makefilePath));
   
   OsPath         testWorkingDir(mTestWorkingDir);

   OsFileSystem::remove(testWorkingDir, TRUE /* Recursive */, TRUE /* Force */ );
   OsFileSystem::createDir(testWorkingDir, TRUE);
}

void FileTestContext::workingFilePath(const char* filename, UtlString& path)
{
   path = mTestWorkingDir+OsPath::separator+filename;
}

void FileTestContext::inputFilePath(const char* filename, UtlString& path)
{
   path = mTestInputDir+OsPath::separator+filename;
}

/// destructor
FileTestContext::~FileTestContext()
{
};
