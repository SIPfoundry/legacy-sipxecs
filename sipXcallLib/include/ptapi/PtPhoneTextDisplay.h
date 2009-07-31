//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtPhoneTextDisplay_h_
#define _PtPhoneTextDisplay_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "ptapi/PtPhoneDisplay.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:The PtPhoneTextDisplay class models a character display.
class PtPhoneTextDisplay : public PtPhoneDisplay
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

   virtual PtStatus setDisplay(char* displayInfo, int x, int y);
     //:Display the indicated information starting at coordinates (x, y).
     //!param: (in) displayInfo - the string to be displayed
     //!param: (in) x,y - The starting coordinates for displaying the string
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

/* ============================ ACCESSORS ================================= */

   virtual PtStatus getDisplay(char*& rpContents, int maxLen, int x, int y);
     //:Returns a copy of the displayed string starting at coordinates (x, y).
     //!param: (in) rpContents - A pointer to the copy of the displayed string
     //!param: (in) maxLen - Size of the <i>rpContents</i> character array
     //!param: (in) x,y - The starting coordinates for displaying the string
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getDisplayColumns(int& rNumColumns);
     //:Returns the number of display columns.
     //!param: (out) rNumColumns - The number of display columns
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getDisplayRows(int& rNumRows);
     //:Returns the number of display rows.
     //!param: (out) rNumRows - The number of display rows
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

//   virtual PtStatus getName(char*& rpName);
     //:Returns the name associated with this component.
     //!param: (out) rpName - The reference used to return the name
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

//   virtual PtStatus getType(int& rType);
     //:Returns the type associated with this component.
     //!param: (out) rType - The reference used to return the component type
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
        int             mType;
        char*   mpName;

   PtPhoneTextDisplay();
     //:Default constructor

   virtual
   ~PtPhoneTextDisplay();
     //:Destructor

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   PtPhoneTextDisplay(const PtPhoneTextDisplay& rPtPhoneTextDisplay);
     //:Copy constructor (not implemented for this class)

   PtPhoneTextDisplay& operator=(const PtPhoneTextDisplay& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtPhoneTextDisplay_h_
