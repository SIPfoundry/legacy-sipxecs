//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PtComponent_h_
#define _PtComponent_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "PtDefs.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:The PtComponent class is the base class for all individual components used
//:to model telephone hardware. Each distinct component type is derived from
//:this class.
class PtComponent
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum ComponentType
   {
      BUTTON,
      DISPLAY,
      GRAPHIC_DISPLAY,
      HOOKSWITCH,
      LAMP,
      MICROPHONE,
      RINGER,
      SPEAKER,
          TEXT_DISPLAY,
          EXTERNAL_SPEAKER,
          UNKNOWN
   };

/* ============================ CREATORS ================================== */
        PtComponent();

        PtComponent(const PtComponent& rPtComponent);
         //:Copy constructor (not implemented for this class)

        PtComponent& operator=(const PtComponent& rhs);
         //:Assignment operator (not implemented for this class)

        PtComponent(int componentType);
     //:Constructor
     //!param: componentType - The type of telephone hardware modeled by this component

        PtComponent(const char*& rName);
     //:Constructor
     //!param: rName - The name of telephone hardware modeled by this component
          //!name: button
          //!name: hookswitch
          //!name: display
          //!name: graphic_display
          //!name: text_display
          //!name: lamp
          //!name: microphone
          //!name: ringer
          //!name: speaker
          //!name: unknown

   virtual
   ~PtComponent();
     //:Destructor

/* ============================ MANIPULATORS ============================== */
        void    setGroupType(int groupType);

/* ============================ ACCESSORS ================================= */

  virtual PtStatus getName(char* rpName, int maxLen);
     //:Returns the name associated with this component.
     //!param: (out) rpName - The reference used to return the name
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

  virtual PtStatus getType(int& rType);
     //:Returns the type associated with this component.
     //!param: (out) rType - The reference used to return the component type
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

/* ============================ INQUIRY =================================== */

friend class PtTerminal;
friend class PtPhoneTerminal;
friend class PtComponentGroup;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
        int             mType;
        char    mpName[21];
public:
        int             mGroupType;


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtComponent_h_
