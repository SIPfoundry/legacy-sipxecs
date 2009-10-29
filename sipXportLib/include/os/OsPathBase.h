//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _OsPathBase_h_
#define _OsPathBase_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "os/OsFS.h"
#include "utl/UtlDefs.h"
#include "utl/UtlString.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:OS generic path class.  Will massage any input string so separators are correct.
//:Also provided functions to
// Base UtlString is the path as a character string.
class OsPathBase : public UtlString
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    static UtlString separator;
      //: platform specific separator (eg. '/' or '\')

/* ============================ CREATORS ================================== */

   OsPathBase();
     //:Default constructor

   OsPathBase(const OsPathBase& rOsPath);
     //:Copy constructor

   virtual
   ~OsPathBase();
     //:Destructor

   OsPathBase(const UtlString& rPath);
     //: Copy contructor

   OsPathBase(const char* pPath);
     //: Construct OsPath from char*

   OsPathBase(const UtlString& rVolume, const UtlString& rDirName, const UtlString& rFileName,
           const UtlString& rExtension);
     //: Forms a OsPath from discrete parts

/* ============================ MANIPULATORS ============================== */

    OsPathBase& operator=(const OsPathBase& rhs);
      //:Assignment operator

    OsPathBase&
    operator+=(const OsPathBase& rhs);
      //:+= operator

    static void setSeparator(UtlString &rSeparator);
      //: Returns the path separator for this object

    void Split();
    //: breaks path into its parts

/* ============================ ACCESSORS ================================= */
    UtlString getVolume() const;
      //: Returns just the volume of this object (eg. for DOS c:,d: etc...)

    UtlString getDirName() const;
      //: Returns just the path of this object (without volume or filename)

    UtlString getFilename() const;
      //: Retrieves just the filename part of this object

    UtlString getExt() const;
      //: Returns just the extension part of this object

    OsStatus getNativePath(OsPathBase &rFullPath) const;
      //: Returns TRUE if the full path for the specified platform was
      //: found to be valid. Returns the full path in rFullPath.

    static UtlString getSeparator() ;
      //: Returns the path separator for this object

/* ============================ INQUIRY =================================== */

    UtlBoolean isValid();
      //:Return TRUE if pathname represented by object is valid for the platform.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    UtlString mDirName;
    //: Directory name.  Ends with backslash
    UtlString mVolume;
    //: Volume (eg. c: d: /sda1)
    UtlString mFilename;
    //: Returns the filename (without extension).
    //  If the OsPath object contains just a path, then filename and ext will blank.
    UtlString mExtension;
    //: Returns the extension of the file.
    //  If the OsPath object contains just a path, then filename and ext will blank.
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    void massage();
      //: Based on the platform in use, this function manipulates the string
      //  so it reprsents a valid platform path

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsPathBase_h_
