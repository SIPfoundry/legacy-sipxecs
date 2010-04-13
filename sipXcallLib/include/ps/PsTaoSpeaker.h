//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PsTaoSpeaker_h_
#define _PsTaoSpeaker_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "ps/PsTaoComponent.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:The PsTaoSpeaker class models the keypad and feature buttons.
class PsTaoSpeaker : public PsTaoComponent
{
   friend class PsPhoneTask;
     // The PsPhoneTask is responsible for creating and destroying
     // all objects derived from the PsTaoComponent class.  No other entity
     // should invoke the constructors or destructors for these classes.

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum VolumeLevel
   {
      OFF    = 0,
      MIDDLE = 5,
      FULL   = 10
   };
   //!enumcode: OFF - The speaker is turned off
   //!enumcode: MIDDLE  - The speaker volume level is set to the middle of its range
   //!enumcode: FULL - The speaker volume is set to its maximum level

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */
   OsStatus setVolume(int volume);
     //:Sets the speaker volume to a value between OFF and FULL (inclusive).
     //!param: volume - The speaker volume level
     //!retcode: OS_SUCCESS - Success
     //!retcode: OS_INVALID_ARGUMENT - Invalid volume level
     //!retcode: OS_PROVIDER_UNAVAILABLE - The provider is not available


/* ============================ ACCESSORS ================================= */
   OsStatus getVolume(int& rVolume);
     //:Sets <i>rVolume</i> to the current speaker volume level.
     //!retcode: OS_SUCCESS - Success
     //!retcode: OS_PROVIDER_UNAVAILABLE - The provider is not available


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
        PsTaoSpeaker(const UtlString& rComponentName, int componentType);

   PsTaoSpeaker();
     //:Default constructor

   virtual
   ~PsTaoSpeaker();
     //:Destructor

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   int      mVolume;     // the volume

   PsTaoSpeaker(const PsTaoSpeaker& rPsTaoSpeaker);
     //:Copy constructor (not implemented for this class)

   PsTaoSpeaker& operator=(const PsTaoSpeaker& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PsTaoSpeaker_h_
