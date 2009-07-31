//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _TaoPhoneComponentAdaptor_h_
#define _TaoPhoneComponentAdaptor_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "tao/TaoAdaptor.h"
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PsPhoneTask;
class PsButtonTask;
class PsHookswTask;
class TaoPhoneLamp;
class TaoTransportTask;
class TaoMessage;

class TaoPhoneComponentAdaptor : public TaoAdaptor
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
        TaoPhoneComponentAdaptor(TaoTransportTask*& rpSvrTransport,
                                           TaoMessage& rMsg,
                                           const UtlString& name = "TaoPhoneComponentAdaptor",
                                           const int maxRequestQMsgs=DEF_MAX_MSGS);

     //:Default constructor

   virtual
   ~TaoPhoneComponentAdaptor();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

        virtual UtlBoolean handleMessage(OsMsg& rMsg);
         //:Handle an incoming message.
         // If the message is not one that the object is prepared to process,
         // the handleMessage() method in the derived class should return FALSE
         // which will cause the OsServerTask::handleMessage() method to be
         // invoked on the message.

        TaoStatus buttonUp(TaoMessage& rMsg);

        TaoStatus buttonDown(TaoMessage& rMsg);

        TaoStatus buttonPress(TaoMessage& rMsg);
         //:Press this button.
         //!retcode: PT_SUCCESS - Success
         //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

        TaoStatus setButtonInfo(TaoMessage& rMsg);
         //:Set the information associated with this button.
         //!param: (in) buttonInfo - The string to associate with this button
         //!retcode: PT_SUCCESS - Success
         //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

        TaoStatus setHookswState(TaoMessage& rMsg);

   TaoStatus setRingerInfo(TaoMessage& rMsg);
     //:Specifies the information string to associate with the indicated
     //:ringer pattern.
     // The <i>info</i> text string is used to provide additional
     // ringer-related information to the phone system (for example, the
     // sound file to associate with this ringer pattern).
     //!param: patternIndex - Identifies the pattern whose <i>info</i> string will be modified.
     //!param: info - The text string to associate with the specified ringer pattern.
     //!retcode: TAO_SUCCESS - Success
     //!retcode: TAO_INVALID_ARGUMENT - Invalid pattern index
     //!retcode: TAO_PROVIDER_UNAVAILABLE - The provider is not available

   TaoStatus setRingerPattern(TaoMessage& rMsg);
     //:Sets the ringer pattern given a valid index number.
     // The pattern index should be a number between 0 and the value returned
     // by getMaxRingPatternIndex().
     //!retcode: TAO_SUCCESS - Success
     //!retcode: TAO_INVALID_ARGUMENT - Invalid pattern index
     //!retcode: TAO_PROVIDER_UNAVAILABLE - The provider is not available

   TaoStatus setRingerVolume(TaoMessage& rMsg);
     //:Sets the ringer volume to a value between OFF and FULL (inclusive).
     //!param: volume - The ringer volume level
     //!retcode: TAO_SUCCESS - Success
     //!retcode: TAO_INVALID_ARGUMENT - Invalid volume level
     //!retcode: TAO_PROVIDER_UNAVAILABLE - The provider is not available

   TaoStatus setMicGain(TaoMessage& rMsg);
     //:Sets the microphone gain (volume) to a value between OFF and
     //:FULL (inclusive).
     //!param: gain - The microphone gain level
     //!retcode: TAO_SUCCESS - Success
     //!retcode: TAO_INVALID_ARGUMENT - invalid gain level
     //!retcode: TAO_PROVIDER_UNAVAILABLE - The provider is not available

   TaoStatus setSpeakerVolume(TaoMessage& rMsg);
     //:Sets the speaker volume to a value between OFF and FULL (inclusive).
     //!param: volume - The speaker volume level
     //!retcode: TAO_SUCCESS - Success
     //!retcode: TAO_INVALID_ARGUMENT - Invalid volume level
     //!retcode: TAO_PROVIDER_UNAVAILABLE - The provider is not available

   TaoStatus setExtSpeakerVolume(TaoMessage& rMsg);
     //:Sets the external speaker volume to a value between OFF and FULL (inclusive).
     //!param: volume - The speaker volume level
     //!retcode: TAO_SUCCESS - Success
     //!retcode: TAO_INVALID_ARGUMENT - Invalid volume level
     //!retcode: TAO_PROVIDER_UNAVAILABLE - The provider is not available

   TaoStatus setLampMode(TaoMessage& rMsg);
     //:Sets the indicator to one of its supported modes.
     //!retcode: TAO_SUCCESS - Success
     //!retcode: TAO_INVALID_ARGUMENT - The requested mode is not supported by this indicator
     //!retcode: TAO_PROVIDER_UNAVAILABLE - The provider is not available

        TaoStatus setDisplay(TaoMessage& rMsg);
         //:Sets the display string at position (x, y).

        TaoStatus setDisplayContrast(TaoMessage& rMsg);
         //:Sets the display contrast.


   TaoStatus activateGroup(TaoMessage& rMsg);
     //:Enables the audio apparatus associated with the component group.
     // Returns TRUE if successful, FALSE if unsuccessful

   TaoStatus deactivateGroup(TaoMessage& rMsg);
     //:Disables the audio apparatus associated with the component group.
     // Returns TRUE if successful, FALSE if unsuccessful

        TaoStatus returnResult(TaoMessage& rMsg);

/* ============================ ACCESSORS ================================= */

        TaoStatus getAssociatedPhoneLamp(TaoMessage& rMsg);
         //:Returns a pointer to the PtPhoneLamp object associated with this button.
         //!param: (out) rpLamp - The pointer to the associated lamp object
         //!retcode: PT_SUCCESS - Success
         //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

        TaoStatus getHookswState(TaoMessage& rMsg);

        TaoStatus getHookswCall(TaoMessage& rMsg);

        TaoStatus getButtonInfo(TaoMessage& rMsg);
         //:Returns the information associated with this button.
         //!param: (out) rpInfo - A pointer to the string associated with this button


   TaoStatus getMaxRingPatternIndex(TaoMessage& rMsg);
     //:Sets <i>rMaxIndex</i> to the maximum valid ringer pattern index.
     //!retcode: TAO_SUCCESS - Success
     //!retcode: TAO_PROVIDER_UNAVAILABLE - The provider is not available

   TaoStatus getNumberOfRings(TaoMessage& rMsg);
     //:Sets <i>rNumRingCycles</i> to the number of complete ring cycles that the ringer has been ringing.
     //!retcode: TAO_SUCCESS - Success
     //!retcode: TAO_PROVIDER_UNAVAILABLE - The provider is not available

   TaoStatus getRingerInfo(TaoMessage& rMsg);
     //:Sets <i>rpInfo</i> to point to the information text string that is
     //:associated with the specified ringer pattern.
     //!param: patternIndex - identifies the pattern whose <i>info</i> string will be modified.
     //!param: rpInfo - set to point to the text string associated with the specified ringer pattern.
     //!retcode: TAO_SUCCESS - Success
     //!retcode: TAO_INVALID_ARGUMENT - Invalid pattern index
     //!retcode: TAO_PROVIDER_UNAVAILABLE - The provider is not available

   TaoStatus getRingerPattern(TaoMessage& rMsg);
     //:Sets <i>rPatternIndex</i> to the index of the current ringer pattern.
     //!retcode: TAO_SUCCESS - Success
     //!retcode: TAO_PROVIDER_UNAVAILABLE - The provider is not available

   TaoStatus getRingerVolume(TaoMessage& rMsg);
     //:Sets <i>rVolume</i> to the current ringer volume level.
     //!retcode: TAO_SUCCESS - Success
     //!retcode: TAO_PROVIDER_UNAVAILABLE - The provider is not available

   TaoStatus getMicGain(TaoMessage& rMsg);
     //:Sets <i>rGain</i> to the current microphone gain level.
     //!retcode: TAO_SUCCESS - Success
     //!retcode: TAO_PROVIDER_UNAVAILABLE - The provider is not available

   TaoStatus getSpeakerVolume(TaoMessage& rMsg);
     //:Sets <i>rVolume</i> to the current speaker volume level.
     //!retcode: TAO_SUCCESS - Success
     //!retcode: TAO_PROVIDER_UNAVAILABLE - The provider is not available

   TaoStatus getSpeakerNominalVolume(TaoMessage& rMsg);
     //:Sets <i>rVolume</i> to the nominal speaker volume level.
     //!retcode: TAO_SUCCESS - Success
     //!retcode: TAO_PROVIDER_UNAVAILABLE - The provider is not available

   TaoStatus getExtSpeakerVolume(TaoMessage& rMsg);
     //:Sets <i>rVolume</i> to the current external speaker volume level.
     //!retcode: TAO_SUCCESS - Success
     //!retcode: TAO_PROVIDER_UNAVAILABLE - The provider is not available

   TaoStatus getExtSpeakerNominalVolume(TaoMessage& rMsg);
     //:Sets <i>rVolume</i> to the nominal external speaker volume level.
     //!retcode: TAO_SUCCESS - Success
     //!retcode: TAO_PROVIDER_UNAVAILABLE - The provider is not available

   TaoStatus getAssociatedPhoneButton(TaoMessage& rMsg);
     //:Returns a pointer to the PsTaoButton object associated with this indicator.
     //!param: (out) rpButton - The pointer to the associated button object
     //!retcode: TAO_SUCCESS - Success
     //!retcode: TAO_PROVIDER_UNAVAILABLE - The provider is not available

   TaoStatus getLampMode(TaoMessage& rMsg);
     //:Sets <i>rMode</i> to the current mode for this indicator,
     //!retcode: TAO_SUCCESS - Success
     //!retcode: TAO_PROVIDER_UNAVAILABLE - The provider is not available

   TaoStatus getSupportedLampModes(TaoMessage& rMsg);
     //:Sets <i>rModeMask</i> to all of the modes that are supported for this indicator.

        TaoStatus getDisplayRows(TaoMessage& rMsg);
         //:Returns the display rows.

        TaoStatus getDisplayColumns(TaoMessage& rMsg);
        //:Returns the display columns.

        TaoStatus getDisplay(TaoMessage& rMsg);
        // returns the string displayed at (x, y)

        TaoStatus getDisplayContrast(TaoMessage& rMsg);
         //:Gets the display contrast.

   TaoStatus getGroupComponents(TaoMessage& rMsg);
     //:Returns pointers to the components in this group.
     // The caller provides an array that can hold up to <i>size</i>
     // PsTaoComponent pointers.  This method fills in the <i>pComponents</i>
     // array with up to <i>size</i> pointers.  The actual number of items
     // filled in is passed back via the <i>nItems</i> argument.
     // Returns OS_LIMIT_REACHED if there are more than <i>nItems</i>
     // components in the group.  Otherwise, returns OS_SUCCESS.

   TaoStatus getGroupDescription(TaoMessage& rMsg);
     //:Returns a string describing the component group.

   TaoStatus getGroupType(TaoMessage& rMsg);
     //:Returns the type of the component group, either HEAD_SET, HAND_SET,
     //:SPEAKER_PHONE, PHONE_SET or OTHER.

/* ============================ INQUIRY =================================== */

   TaoStatus isRingerOn(TaoMessage& rMsg);
     //:Sets <i>rIsOn</i> to FALSE if the ringer is OFF and FALSE otherwise.
     //!param: (out) rIsOn - TRUE ==> ringer is ON, FALSE ==> ringer is OFF
     //!retcode: TAO_SUCCESS - Success
     //!retcode: TAO_PROVIDER_UNAVAILABLE - The provider is not available


   TaoStatus isGroupActivated(TaoMessage& rMsg);
     //:Determine whether the audio apparatus associated with the component
     //:group is enabled.
     // Returns TRUE if activated, FALSE if deactivated.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
        PsButtonTask*   mpButtonTask;
        PsPhoneTask*    mpPhoneTask;
        PsHookswTask*   mpHookswTask;
        TaoTransportTask*       mpSvrTransport;

        int mLCDContrast;       // current LCD level, for LCD contrast level adjustment
        int mLCDLow;            // minimum LCD level, for LCD adjustment
        int mLCDHigh;           // maximum LCD level, for LCD adjustment
        int mLCDOffset;         // LCD offset, for LCD adjustment

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

        TaoPhoneComponentAdaptor(const TaoPhoneComponentAdaptor& rTaoPhoneComponentAdaptor);
         //:Copy constructor (not implemented for this class)

        TaoPhoneComponentAdaptor& operator=(const TaoPhoneComponentAdaptor& rhs);
         //:Assignment operator (not implemented for this class)

        TaoPhoneLamp*   mpLamp;
        UtlString               mInfo;
        int                             mHookswState;


};

/* ============================ INLINE METHODS ============================ */

#endif  // _TaoPhoneComponentAdaptor_h_
