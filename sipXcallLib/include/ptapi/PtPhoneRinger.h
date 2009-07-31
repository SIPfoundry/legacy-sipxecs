//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtPhoneRinger_h_
#define _PtPhoneRinger_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "ptapi/PtComponent.h"
#include "os/OsTime.h"
#include "os/OsProtectEventMgr.h"
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class TaoClientTask;

//:The PtPhoneRinger class models a phone ringer.

class PtPhoneRinger : public PtComponent
{
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
   PtPhoneRinger();
     //:Default constructor

  PtPhoneRinger(TaoClientTask *pClient);

   PtPhoneRinger(const PtPhoneRinger& rPtPhoneRinger);
     //:Copy constructor (not implemented for this class)

   PtPhoneRinger& operator=(const PtPhoneRinger& rhs);
     //:Assignment operator (not implemented for this class)

   virtual
   ~PtPhoneRinger();
     //:Destructor


/* ============================ MANIPULATORS ============================== */

   virtual PtStatus setRingerInfo(int patternIndex, char* info);
     //:Specifies the information string to associate with the indicated
     //:ringer pattern.
     // The <i>info</i> text string is used to provide additional
     // ringer-related information to the phone system (for example, the
     // sound file to associate with this ringer pattern).
     //!param: patternIndex - Identifies the pattern whose <i>info</i> string will be modified.
     //!param: info - The text string to associate with the specified ringer pattern.
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_INVALID_ARGUMENT - Invalid pattern index
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus setRingerPattern(int patternIndex);
     //:Sets the ringer pattern given a valid index number.
     // The pattern index should be a number between 0 and the value returned
     // by getMaxRingPatternIndex().
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_INVALID_ARGUMENT - Invalid pattern index
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus setRingerVolume(int volume);
     //:Sets the ringer volume to a value between OFF and FULL (inclusive).
     //!param: volume - The ringer volume level
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_INVALID_ARGUMENT - Invalid volume level
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

/* ============================ ACCESSORS ================================= */

   virtual PtStatus getMaxRingPatternIndex(int& rMaxIndex);
     //:Sets <i>rMaxIndex</i> to the maximum valid ringer pattern index.
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getNumberOfRings(int& rNumRingCycles);
     //:Sets <i>rNumRingCycles</i> to the number of complete ring cycles that the ringer has been ringing.
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getRingerInfo(int patternIndex, char*& rpInfo);
     //:Sets <i>rpInfo</i> to point to the information text string that is
     //:associated with the specified ringer pattern.
     //!param: patternIndex - identifies the pattern whose <i>info</i> string will be modified.
     //!param: rpInfo - set to point to the text string associated with the specified ringer pattern.
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_INVALID_ARGUMENT - Invalid pattern index
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getRingerPattern(int& rPatternIndex);
     //:Sets <i>rPatternIndex</i> to the index of the current ringer pattern.
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   virtual PtStatus getRingerVolume(int& rVolume);
     //:Sets <i>rVolume</i> to the current ringer volume level.
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

   virtual PtStatus isRingerOn(PtBoolean& rIsOn);
     //:Sets <i>rIsOn</i> to FALSE if the ringer is OFF and FALSE otherwise.
     //!param: (out) rIsOn - TRUE ==> ringer is ON, FALSE ==> ringer is OFF
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

        TaoClientTask   *mpClient;

        OsTime          mTimeOut;
/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
        OsProtectEventMgr *mpEventMgr;


};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtPhoneRinger_h_
