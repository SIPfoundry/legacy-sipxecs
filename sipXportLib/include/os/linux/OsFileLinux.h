//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#ifndef _OsFileLinux_h_
#define _OsFileLinux_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsFileInfoBase;
class OsFileInfoLinux;
class OsPathLinux;

//:OS class for creating,reading, writing, manipulating files.
class OsFileLinux : public OsFileBase
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:


/* ============================ CREATORS ================================== */

   OsFileLinux(const OsPathBase& filename);
     //:Default constructor

   virtual ~OsFileLinux();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsStatus filelock(int mode);
     //: Sets the locking mode of the file.  The file must be open.
     //: Use SHARED_READ, SHARED_WRITE, SHARED_NONE
     //: Returns OS_SUCCESS on success
     //:         or OS_FAILED on failure

   OsStatus fileunlock();
     //: Unlocks the file locked by the function above

   OsStatus setLength(size_t newLength);
     //: Sets the length of the file specified by the object to the new size
     //: Sets the length of the file specified by the object to the new size
     //: Shrinking or Growing the file as needed.


   OsStatus setReadOnly(UtlBoolean isReadOnly);
     //: Sets the file to the new state
     //: Returns:
     //:        OS_SUCCESS if successful
     //:         OS_INVALID if failed


   OsStatus touch();
     //: Updates the date and time on the file.  Creates if needed.

/* ============================ ACCESSORS ================================= */



/* ============================ INQUIRY =================================== */

    UtlBoolean isReadonly() const;
    //: Returns TRUE if file is readonly


    virtual OsStatus getFileInfo(OsFileInfoBase& rFileinfo) const;
    //: Returns all the relevant info on this file

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   OsFileLinux(const OsFileLinux& rOsFileLinux);
     //:Copy constructor

   OsFileLinux& operator=(const OsFileLinux& rhs);
     //:Assignment operator

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsFileLinux_h_
