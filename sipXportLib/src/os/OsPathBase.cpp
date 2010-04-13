//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsFS.h"
#include "utl/UtlDefs.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

#if defined(_VXWORKS)
    UtlString OsPathBase::separator = "/";
#elif defined(WIN32)
    UtlString OsPathBase::separator = "\\";
#else
    UtlString OsPathBase::separator = "/";
#endif

//DEFINES
#define Extension_ 1
#define Filename_  2
#define Directory_ 4
#define Drive_     8
#define Wildname_  16
#define Wildpath_  32

/******************************************************************/
/*                                                                */
/*  has_wild()                                                    */
/*                                                                */
/*  Checks a string for wildcard characters ('?' and '*')         */
/*                                                                */
/*  Arguments 1 - String to check                                 */
/*                                                                */
/*  Returns: True_ if string contains wildcards, else False_      */
/*                                                                */
/*  Side Effects: None                                            */
/*                                                                */
/******************************************************************/

UtlBoolean has_wild(char *pname)
{
      if (NULL != strchr(pname, '*') || NULL != strchr(pname, '?'))
            return TRUE;
      else  return FALSE;
}

/******************************************************************/
/*                                                                */
/*  fnSplit()                                                     */
/*                                                                */
/*  Splits file specifications into component parts. Similar to   */
/*  compiler-specific fnsplit() or _splitpath().                  */
/*                                                                */
/*  Arguments 1 - Original file specification                     */
/*            2 - Buffer to receive drive spec                    */
/*            3 - Buffer to receive drive/path spec               */
/*            4 - Buffer to receive path spec                     */
/*            5 - Buffer to receive name.ext spec                 */
/*            6 - Buffer to receive name spec                     */
/*            7 - Buffer to receive ext spec                      */
/*                                                                */
/*  Returns: Bit map as follows (see defines in RBS.H):           */
/*           Extension_ - File spec included extension            */
/*           Filename_  - File spec did not end in '\'            */
/*           Directory_ - File spec included a path               */
/*           Drive_     - File spec included a drive spec         */
/*           Wildname_  - File name included wildcards (*.?)      */
/*           Wildpath_  - File path included wildcards (*.?)      */
/*                                                                */
/*  Side Effects: Calls dos2unix() to convert '\' to '/'          */
/*                                                                */
/*  Notes: Passing NULL in arguments 2-7 causes fnsplit() to      */
/*         not save the corresponding portion of the path.        */
/*                                                                */
/******************************************************************/

int fnSplit(char *spec,             /* Original file spec         */
            char *drive,            /* Drive spec                 */
            char *pname,            /* Path w/ drive spec         */
            char *path,             /* Path spec                  */
            char *fname,            /* File name + extension      */
            char *name,             /* File name                  */
            char *ext)              /* File extension             */
{
      int ret_code = 0;
      char *d = spec, *p, *e;


      if ('\0' != spec[0] && ':' == spec[1])
      {
            if (drive)
                  strncpy(drive, spec, 2);
            drive[2] = 0;
            d += 2;
            ret_code |= Drive_;
      }
      else
      {
            if (drive)
                  *drive = 0;
      }

      if (NULL != (p = strrchr(d, *(OsPathBase::separator.data()))))
      {
            char ch;

            ch = *(++p);
            *p = 0;
            if (path)
                  strcpy(path, d);
            if (pname)
                  strcpy(pname, spec);
            if (has_wild(d))
                  ret_code |= Wildpath_;
            *p = ch;
            ret_code |= Directory_;
      }
      else
      {
            if (path)
                  *path = 0;
            if (pname)
            {
                  if (drive)
                        strcpy(pname, drive);
                  else  *pname = 0;
            }
            p = d;

            if ('.' == *p)
            {
                  size_t dot_length;

                  ret_code |= Directory_;
                  for (dot_length = 0; '.' == p[dot_length]; ++dot_length)
                        ;
                  if (path)
                  {
                        strncat(path, p, dot_length);
                        strcat(path, OsPathBase::separator.data());
                  }
                  if (pname)
                  {
                        strncat(pname, p, dot_length);
                        strcat(pname, OsPathBase::separator.data());
                  }
                  if (fname)
                        *fname = 0;
                  if (name)
                        *name  = 0;
                  if (ext)
                        *ext   = 0;

                  return ret_code;
            }
      }
      if (fname)
            strcpy (fname, p);
      if (has_wild(p))
            ret_code |= Wildname_;
      if (*p)
            ret_code |= Filename_;

      if (NULL != (e = strrchr(p, '.')))
      {
            *e = 0;
            if (name)
                  strcpy(name, p);
            *e = '.';
            if (ext)
                  strcpy(ext, e);
            ret_code |= Extension_;
      }
      else
      {
            if (name)
                  strcpy(name,p);
            if (ext)
                  *ext = 0;
      }
      return ret_code;
}

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsPathBase::OsPathBase()
{
}

// Make one from a char string
OsPathBase::OsPathBase(const char *pathname)
: UtlString(pathname)
{
    massage();
}
// Make one from a UtlString character string
OsPathBase::OsPathBase(const UtlString &pathname)
: UtlString(pathname)
{
    massage();
}

// Copy constructor
OsPathBase::OsPathBase(const OsPathBase& rOsPath)
: UtlString(rOsPath)
{
    *this = rOsPath.data();
    massage();
}

// Destructor
OsPathBase::~OsPathBase()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsPathBase&
OsPathBase::operator=(const OsPathBase& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   UtlString::operator=(rhs.data());

   massage();

   return *this;
}

OsPathBase&
OsPathBase::operator+=(const OsPathBase& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   UtlString::operator+=(rhs.data());

   massage();

   return *this;
}

void OsPathBase::setSeparator(UtlString &rSeparator)
{
    separator = rSeparator;
}

/* ============================ ACCESSORS ================================= */

UtlString OsPathBase::getSeparator()
{
    return separator;
}

//: returns just the volme of this object (eg. for DOS c:,d: etc...)
UtlString OsPathBase::getVolume() const
{
    return mVolume;
}
//: returns just the path of this object (without volume or filename)
UtlString OsPathBase::getDirName() const
{
    return mDirName;
}

//: retrieves just the filename part of this object
UtlString OsPathBase::getFilename() const
{
    return mFilename;
}

//: returns just the extension part of this object
UtlString OsPathBase::getExt() const
{
    return mExtension;
}

//: returns the full pathname
OsStatus OsPathBase::getNativePath(OsPathBase &rFullPath) const
{
    OsStatus retval = OS_FAILED;
    OsPath origdir;
    OsFileSystem::getWorkingDirectory(origdir);

    OsPath path = *this;

    //just return
    if (!OsFileSystem::exists(path))
    {
        rFullPath = *this;
        return OS_SUCCESS;
    }


    //check if this is a dir
    OsPath temp;
    OsPath vol = mVolume;
    OsFileInfo fileInfo;
    OsFileSystem::getFileInfo(path,fileInfo);
    if (!fileInfo.isDir())
    {

        if (*vol.data() == '\0')
             vol = origdir.getVolume();
        temp = vol+mDirName;
    }
    else
        temp = *this;


    if (OsFileSystem::change(temp) == OS_SUCCESS)
    {
        OsPath newdir;
        OsFileSystem::getWorkingDirectory(newdir);
        rFullPath = newdir;
        if(!fileInfo.isDir() && !mFilename.isNull())
        {
            rFullPath = rFullPath + OsPath::separator + mFilename + mExtension;
        }

        if (OsFileSystem::change(origdir) == OS_SUCCESS)
            retval = OS_SUCCESS;
    }

    return retval;
}

/* ============================ INQUIRY =================================== */
//: returns true is specified path is valid for the platform
UtlBoolean OsPathBase::isValid()
{
    UtlBoolean retval = TRUE;
    retval = OsFileSystem::exists(*this);
    return retval;
}

void OsPathBase::Split()
{
    char drive[10];
    char pname[256];
    char path[256];
    char fname[256];
    char name[256];
    char ext[32];

    fnSplit((char *)data(),             /* Original file spec         */
            drive,            /* Drive spec                 */
            pname,            /* Path w/ drive spec         */
            path,             /* Path spec                  */
            fname,            /* File name + extension      */
            name,             /* File name                  */
            ext);              /* File extension             */

     mVolume = drive;
     mFilename = name;
     mDirName = path;
     mExtension = ext;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

// this function will form the path into the correct path for the given
// platform.
// It also breaks the path into its pieces and stores them in the correct member variables
void OsPathBase::massage()
{
    UtlString char_to_replace;

    //due to there not being a command api to do this, I've decided to do this myself.'
    // (goodluck to me)

    //now lets figure out which separators to replace
    if (separator.compareTo("/") == 0)
        char_to_replace = "\\";
    else if (separator.compareTo("\\") == 0)
        char_to_replace = "/";

    ssize_t pos = index(char_to_replace);
    while (pos != UTLSTRING_NOT_FOUND)
    {
        replace(pos,1,separator);
        pos = index(char_to_replace,pos+1);
    }

    Split();
}

/* ============================ FUNCTIONS ================================= */
