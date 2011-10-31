//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
#ifndef _FILETESTCONTEXT_H_
#define _FILETESTCONTEXT_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "os/OsFS.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Utilities for redirecting file resources during unit tests.
/**
 * This class and its subclasses abstract the locations of files that would normally
 * be hardcoded so that tests can use locations in the build tree and provide different
 * input sets for each test.
 *
 * A typical test that reads and writes files can declare a FileTestContext so that the input
 * files are copied from the source tree to the build tree, possibly modified along the way
 * by a TemplateConverter supplied by the test or some subclass of FileTestContext.  Typically
 * the paths to the source and build tree will be provided from the build environment.
 *
 * @note
 * <strong>The working directory will be deleted and recreated</strong>, so it must not
 * be in the source tree.
 */
class FileTestContext
{
  public:

   /// Define a context for a test or set of tests.
   FileTestContext( const char* testInputDir   ///< directory for test input & template files
                   ,const char* testWorkingDir ///< directory for test working files.
                   );
   /**<
    * Deletes all files and directories in the testWorkingDir.
    *
    * The directories are:
    *
    * - testInputDir holds all the input files for the test.  Each input file is copied to
    *   the working directory using inputFile or inputFileFromTemplate.  Normally a path
    *   to somewhere in the src tree.  In the autotools build, most tests provided this from
    *   the cpp command line as TEST_DATA_DIR.
    *
    * - testWorkingDir holds the files for a single FileTestContext.
    *   Normally set to a directory in the build tree or some tmp location unique to the test.
    *   This is set to be the directory where sipdb expects to find database .xml files
    *   and the sipdb working files.  In the autotools build, most tests provided this from
    *   the cpp command line as TEST_WORK_DIR.
    */

   /// Get a file from the testInputDir, copy into testWorkingDir.
   virtual void inputFile(const char* filename);

   /// Helper function provided by a test for converting a template to a working file.
   typedef void TemplateConverter( OsFile* templateFile ///< input
                                  ,OsFile* workingFile  ///< output
                                  );
   /**<
    * This is provided by a test and called by workingFileFromTemplate.
    * When called, the templateFile is already opened for reading, and
    * the workingFile is already opened for writing.
    */

   virtual void workingFileFromTemplate( const char* templateName     ///< testInputDir file
                                        ,TemplateConverter* converter ///< conversion function
                                        ,const char* filename = NULL  ///< testWorkingDir file
                                        );
   /**<
    * The template in the testInputDir and the filename in testWorkingDir are opened,
    * and then both files are passed to the converter.
    *
    * The converter is responsible for reading the template and writing the filename.
    *
    * If no filename is supplied, its name is the same as the templateName.
    */

   /// Delete any existing copy of the file in the testWorkingDir
   virtual void removeWorkingFile(const char* filename);

   /// Convert a file name to a full path name in the input directory
   void inputFilePath(const char* filename, UtlString& path);

   /// Convert a file name to a full path name in the working directory
   void workingFilePath(const char* filename, UtlString& path);

   /// Set the environment so that the SipXecsService paths are resolved to the context.
   void setSipxDir(DirectoryType dirType,
                   const char* subDir = NULL ///< a subdirectory of the working directory.
                   );

   /// Destructor
   virtual ~FileTestContext();

  protected:

   virtual void makeCleanWorkingDir();

   UtlString mTestInputDir;

   UtlString mTestWorkingDir;

  private:

   /// There is no copy constructor.
   FileTestContext(const FileTestContext&);

   /// There is no assignment operator.
   FileTestContext& operator=(const FileTestContext&);

};

#endif // _FILETESTCONTEXT_H_
