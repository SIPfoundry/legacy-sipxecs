//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsFS_h_
#define _OsFS_h_

#include "os/OsFileBase.h"
#include "os/OsFileIteratorBase.h"
#include "os/OsDirBase.h"
#include "os/OsPathBase.h"
#include "os/OsFileInfoBase.h"

typedef const char* const DirectoryType;

// Depending on the native OS that we are running on, we include the class
// declaration for the appropriate lower level implementation and use a
// "typedef" statement to associate the OS-independent class name (OsTask)
// with the OS-dependent realization of that type (e.g., OsTaskWnt).
#if defined(_WIN32)
#  include "os/Wnt/OsFileWnt.h"
#  include "os/Wnt/OsDirWnt.h"
#  include "os/Wnt/OsPathWnt.h"
#  include "os/Wnt/OsFileInfoWnt.h"
#  include "os/Wnt/OsFileIteratorWnt.h"
#  include "os/Wnt/OsFileSystemWnt.h"
   typedef class OsFileWnt OsFile;
   typedef class OsDirWnt OsDir;
   typedef class OsPathWnt OsPath;
   typedef class OsFileInfoWnt OsFileInfo;
   typedef class OsFileIteratorWnt OsFileIterator;
#elif defined(_VXWORKS)
#  include "os/Vxw/OsFileVxw.h"
#  include "os/Vxw/OsDirVxw.h"
#  include "os/Vxw/OsPathVxw.h"
#  include "os/Vxw/OsFileInfoVxw.h"
#  include "os/Vxw/OsFileIteratorVxw.h"
   typedef class OsPathVxw OsPath;
   typedef class OsDirVxw OsDir;
   typedef class OsFileVxw OsFile;
   typedef class OsFileInfoVxw OsFileInfo;
   typedef class OsFileIteratorVxw OsFileIterator;
#elif defined(__pingtel_on_posix__)
#  include "os/linux/OsFileLinux.h"
#  include "os/linux/OsDirLinux.h"
#  include "os/linux/OsPathLinux.h"
#  include "os/linux/OsFileInfoLinux.h"
#  include "os/linux/OsFileIteratorLinux.h"
   typedef class OsPathLinux OsPath;
   typedef class OsDirLinux OsDir;
   typedef class OsFileLinux OsFile;
   typedef class OsFileInfoLinux OsFileInfo;
   typedef class OsFileIteratorLinux OsFileIterator;
#else
#  error Unsupported target platform.
#endif

#include "os/OsFileSystem.h"


#endif /* ifdef _OsFS_h_ */
