//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PsTaoComponentGroup_h_
#define _PsTaoComponentGroup_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "utl/UtlDefs.h"
#include "utl/UtlString.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PsTaoComponent;

//:A grouping of PsTaoComponent objects.
// The phone terminal is composed of multiple PsTaoComponentGroups.
// Applications query the PsPhoneTask for the available PsTaoComponentGroups.
// Then they query this class for the PsTaoComponents that make up the group.
class PsTaoComponentGroup
{
   friend class PsPhoneTask;
     // The PsPhoneTask is responsible for creating and destroying
     // PsTaoComponent and PsTaoComponentGroup objects.  No other entity
     // should invoke the constructors or destructors for these classes.

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum PsTaoComponentGroupType
   {
      HEAD_SET,
      HAND_SET,
      SPEAKER_PHONE,
      PHONE_SET,
          EXTERNAL_SPEAKER,
          OTHER
   };

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */
        OsStatus setHandsetVolume(int& level);
        OsStatus setHeadsetVolume(int& level);
        OsStatus setSpeakerVolume(int& level);
        OsStatus setExtSpeakerVolume(int& level);
        OsStatus setRingerVolume(int& level);

        OsStatus setMicGain(int group, int& level);

   UtlBoolean activate(void);
     //:Enables the audio apparatus associated with the component group.
     // Returns TRUE if successful, FALSE if unsuccessful

   UtlBoolean deactivate(void);
     //:Disables the audio apparatus associated with the component group.
     // Returns TRUE if successful, FALSE if unsuccessful

/* ============================ ACCESSORS ================================= */
   OsStatus getComponents(PsTaoComponent* pComponents[], int size,
                          int& nItems);
     //:Returns pointers to the components in this group.
     // The caller provides an array that can hold up to <i>size</i>
     // PsTaoComponent pointers.  This method fills in the <i>pComponents</i>
     // array with up to <i>size</i> pointers.  The actual number of items
     // filled in is passed back via the <i>nItems</i> argument.
     // Returns OS_LIMIT_REACHED if there are more than <i>nItems</i>
     // components in the group.  Otherwise, returns OS_SUCCESS.

   void getDescription(UtlString& rDescription);
     //:Returns a string describing the component group.

   int getType(void);
     //:Returns the type of the component group, either HEAD_SET, HAND_SET,
     //:SPEAKER_PHONE, PHONE_SET or OTHER.

        OsStatus getHandsetVolume(int& level, int isNominal = 0);
        OsStatus getHeadsetVolume(int& level, int isNominal = 0);
        OsStatus getSpeakerVolume(int& level, int isNominal = 0);
        OsStatus getExtSpeakerVolume(int& level, int isNominal = 0);
        OsStatus getRingerVolume(int& level, int isNominal = 0);

        OsStatus getMicGain(int group, int& level);     // 0 <= level <= 10
        OsStatus getMicGainValue(int group, int& value); // actual value

/* ============================ INQUIRY =================================== */

   UtlBoolean isActivated(void);
     //:Determine whether the audio apparatus associated with the component
     //:group is enabled.
     // Returns TRUE if activated, FALSE if deactivated.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   PsTaoComponentGroup(int groupType, const UtlString& rDescription,
                       PsTaoComponent* pComponents[], int nItems);
     //:Constructor

   virtual
   ~PsTaoComponentGroup();
     //:Destructor

        OsStatus setVolumeRange(int low,         // lowest value
                      int high,        // highest value
                      int nominal,     // low <= nominal <= high
                      int stepsize,    // in .1 dB
                      int mute);       // input value to mute

        OsStatus setGainRange(int low,         // lowest value
                      int high,        // highest value
                      int nominal,     // low <= nominal <= high
                      int stepsize,    // in .1 dB
                      int mute);       // input value to mute

        OsStatus getVolume(int groupType, int& level);

        int mHandsetVolume;
        int mHeadsetVolume;
        int mRingerVolume;
        int mSpeakerVolume;
        int mExtSpeakerVolume;

        int mLow;
        int mHigh;
        int mNominal;     // low <= nominal <= high
        int mStepsize;    // in .1 dB
        int mMute;

        int mMicGain;

        int mMicLow;
        int mMicHigh;
        int mMicNominal;     // low <= nominal <= high
        int mMicStepsize;    // in .1 dB
        int mMicMute;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   PsTaoComponent** mpComponents;
   UtlString        mDescription;
   int             mGroupType;
   UtlBoolean       mIsActivated;
   int             mNumItems;

   PsTaoComponentGroup();
     //:Default constructor (not implemented for this class)

   PsTaoComponentGroup(const PsTaoComponentGroup& rPsTaoComponentGroup);
     //:Copy constructor (not implemented for this class)

   PsTaoComponentGroup& operator=(const PsTaoComponentGroup& rhs);
     //:Assignment operator (not implemented for this class)

        int normalize(int& level);

        int gainNormalize(int& level);

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PsTaoComponentGroup_h_
