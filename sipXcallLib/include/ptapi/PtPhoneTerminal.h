//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtPhoneTerminal_h_
#define _PtPhoneTerminal_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "ptapi/PtTerminal.h"
#include "ptapi/PtDefs.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PtComponentGroup;

//:The PtPhoneTerminal interface extends the PtTerminal interface to provide
// functionality for the Phone package. It allows applications to obtain arrays of
// telephony Components (each group is called a ComponentGroup) which
// represents the physical components of telephones. <br>
// <br>
// <H3>Do Not Disturb</H3>
// The PtPhoneTerminal class defines the <i>do-not-disturb</i> attribute. The
// <i>do-not-disturb</i> attribute indicates to the telephony platform that
// this terminal does not want to be bothered with incoming telephone calls.
// That is, if this feature is activated, the underlying telephone platform
// will not ring this terminal for incoming telephone calls. Applications use
// the PtPhoneTerminal.setDoNotDisturb() method to activate and deactivate this
// feature and the PtPhoneTerminal.getDoNotDisturb() method to return the current
// state of this attribute.<br>
// <br>
// Note that the PtAddress class also carries the <i>do-not-disturb</i>
// attribute. The attributes associated with each class are maintained
// independently. Maintaining a separate <i>do-not-disturb</i> attribute at
// both the terminal and address allows for control over the
// <i>do-not-disturb</i> feature at either the terminal or address level.
class PtPhoneTerminal : public PtTerminal
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

        PtPhoneTerminal();
         //:Default constructor

        virtual
        ~PtPhoneTerminal();
         //:Destructor

        PtPhoneTerminal(const PtPhoneTerminal& rPtPhoneTerminal);
         //:Copy constructor

        PtPhoneTerminal(const char* terminalName);


/* ============================ MANIPULATORS ============================== */

        PtPhoneTerminal& operator=(const PtPhoneTerminal& rhs);
         //:Assignment operator

/* ============================ ACCESSORS ================================= */

        virtual PtStatus getComponentGroups(PtComponentGroup* pComponentGroup[], int size, int& nItems);

     //:Returns an array of ComponentGroup objects available on the Terminal. A
     //:ComponentGroup object is composed of a number of Components.
     //:Examples of Component objects include headsets, handsets,
     // speakerphones, and buttons. ComponentGroup objects group Components
     // together.
     //!param: (out) pComponentGroup - The array of PtComponetGroups
     //!param: (in) size - The number of elements in the <i>components</i> array
     //!param: (out) nItems - The number of items assigned
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_MORE_DATA - There are more than <i>size</i> components
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtPhoneTerminal_h_
