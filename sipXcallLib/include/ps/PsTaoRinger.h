//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PsTaoRinger_h_
#define _PsTaoRinger_h_

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

//:The PsTaoRinger class models the keypad and feature buttons.
class PsTaoRinger : public PsTaoComponent
{
   friend class PsPhoneTask;
     // The PsPhoneTask is responsible for creating and destroying
     // all objects derived from the PsTaoComponent class.  No other entity
     // should invoke the constructors or destructors for these classes.

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum RingerLevel
   {
      OFF    = 0,
      MIDDLE = 5,
      FULL   = 10
   };
   //!enumcode: OFF - The ringer is turned off
   //!enumcode: MIDDLE  - The ringer volume is set to the middle of its range
   //!enumcode: FULL - The ringer volume is set to its maximum level

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

   OsStatus setRingerInfo(int patternIndex, char* info);
     //:Specifies the information string to associate with the indicated
     //:ringer pattern.
     // The <i>info</i> text string is used to provide additional
     // ringer-related information to the phone system (for example, the
     // sound file to associate with this ringer pattern).
     //!param: patternIndex - Identifies the pattern whose <i>info</i> string will be modified.
     //!param: info - The text string to associate with the specified ringer pattern.
     //!retcode: OS_SUCCESS - Success
     //!retcode: OS_INVALID_ARGUMENT - Invalid pattern index
     //!retcode: OS_PROVIDER_UNAVAILABLE - The provider is not available

   OsStatus setRingerPattern(int patternIndex);
     //:Sets the ringer pattern given a valid index number.
     // The pattern index should be a number between 0 and the value returned
     // by getMaxRingPatternIndex().
     //!retcode: OS_SUCCESS - Success
     //!retcode: OS_INVALID_ARGUMENT - Invalid pattern index
     //!retcode: OS_PROVIDER_UNAVAILABLE - The provider is not available

   OsStatus setRingerVolume(int volume);
     //:Sets the ringer volume to a value between OFF and FULL (inclusive).
     //!param: volume - The ringer volume level
     //!retcode: OS_SUCCESS - Success
     //!retcode: OS_INVALID_ARGUMENT - Invalid volume level
     //!retcode: OS_PROVIDER_UNAVAILABLE - The provider is not available

/* ============================ ACCESSORS ================================= */

   OsStatus getMaxRingPatternIndex(int& rMaxIndex);
     //:Sets <i>rMaxIndex</i> to the maximum valid ringer pattern index.
     //!retcode: OS_SUCCESS - Success
     //!retcode: OS_PROVIDER_UNAVAILABLE - The provider is not available

   OsStatus getNumberOfRings(int& rNumRingCycles);
     //:Sets <i>rNumRingCycles</i> to the number of complete ring cycles that the ringer has been ringing.
     //!retcode: OS_SUCCESS - Success
     //!retcode: OS_PROVIDER_UNAVAILABLE - The provider is not available

   OsStatus getRingerInfo(int patternIndex, char*& rpInfo);
     //:Sets <i>rpInfo</i> to point to the information text string that is
     //:associated with the specified ringer pattern.
     //!param: patternIndex - identifies the pattern whose <i>info</i> string will be modified.
     //!param: rpInfo - set to point to the text string associated with the specified ringer pattern.
     //!retcode: OS_SUCCESS - Success
     //!retcode: OS_INVALID_ARGUMENT - Invalid pattern index
     //!retcode: OS_PROVIDER_UNAVAILABLE - The provider is not available

   OsStatus getRingerPattern(int& rPatternIndex);
     //:Sets <i>rPatternIndex</i> to the index of the current ringer pattern.
     //!retcode: OS_SUCCESS - Success
     //!retcode: OS_PROVIDER_UNAVAILABLE - The provider is not available

   OsStatus getRingerVolume(int& rVolume);
     //:Sets <i>rVolume</i> to the current ringer volume level.
     //!retcode: OS_SUCCESS - Success
     //!retcode: OS_PROVIDER_UNAVAILABLE - The provider is not available

/* ============================ INQUIRY =================================== */

   OsStatus isRingerOn(UtlBoolean& rIsOn);
     //:Sets <i>rIsOn</i> to FALSE if the ringer is OFF and FALSE otherwise.
     //!param: (out) rIsOn - TRUE ==> ringer is ON, FALSE ==> ringer is OFF
     //!retcode: OS_SUCCESS - Success
     //!retcode: OS_PROVIDER_UNAVAILABLE - The provider is not available

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
        PsTaoRinger(const UtlString& rComponentName, int componentType);

   PsTaoRinger();
     //:Default constructor

   virtual
   ~PsTaoRinger();
     //:Destructor

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   int      mVolume;     // the volume
   bool         mIsRingerOn;

   PsTaoRinger(const PsTaoRinger& rPsTaoRinger);
     //:Copy constructor (not implemented for this class)

   PsTaoRinger& operator=(const PsTaoRinger& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PsTaoRinger_h_
