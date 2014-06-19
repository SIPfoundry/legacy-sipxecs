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
#include "os/OsFileInfoBase.h"


// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
OsFileInfoBase::OsFileInfoBase() :
  mbIsReadOnly(0),
  mbIsDirectory(0),
  mSize(0)
{
}

// Copy constructor
OsFileInfoBase::OsFileInfoBase(const OsFileInfoBase& rOsFileInfoBase) :
  mbIsReadOnly(0),
  mbIsDirectory(0),
  mSize(0)
{
}

// Destructor
OsFileInfoBase::~OsFileInfoBase()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OsFileInfoBase&
OsFileInfoBase::operator=(const OsFileInfoBase& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/* ============================ ACCESSORS ================================= */
OsStatus OsFileInfoBase::getCreateTime(OsTime& time) const
{
    OsStatus stat = OS_SUCCESS;
    time = mCreateTime;
    return stat;

}

OsStatus OsFileInfoBase::getModifiedTime(OsTime& time) const
{
    OsStatus stat = OS_SUCCESS;
    time = mModifiedTime;
    return stat;

}

OsStatus OsFileInfoBase::getSize(unsigned long& size) const
{
    OsStatus stat = OS_SUCCESS;
    size = mSize;
    return stat;
}

/* ============================ INQUIRY =================================== */

UtlBoolean  OsFileInfoBase::isReadOnly() const
{

    return mbIsReadOnly;
}

UtlBoolean  OsFileInfoBase::isDir() const
{

    return mbIsDirectory;
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
