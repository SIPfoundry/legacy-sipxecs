//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtTerminalComponentListener_h_
#define _PtTerminalComponentListener_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "ptapi/PtTerminalListener.h"
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PtComponentIntChangeEvent;
class PtComponentStringChangeEvent;
class PtTerminalComponentEvent;
class PtEventMask;

//:The PtTerminalComponentListener is used to register with and receive
//:events from PtTerminal objects.

class PtTerminalComponentListener : public PtTerminalListener
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   PT_CLASS_INFO_MEMBERS

/* ============================ CREATORS ================================== */

   PtTerminalComponentListener(const char* name = NULL, PtEventMask* pMask = NULL);
     //:Default constructor
     //!param: (in) pMask - Event mask defining events the listener is interested in.  This must be a subset of the events that the listener supports.  The mask may be NULL where it is assumed that all events applicable to the derived listener are of interest.

   virtual
   ~PtTerminalComponentListener();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

  virtual void phoneRingerVolumeChanged(const PtComponentIntChangeEvent& rEvent);
     //:Method invoked on listener for event id =
     //:PHONE_RINGER_VOLUME_CHANGED
     //:indicating that the PtPhoneRinger volume has changed.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void phoneRingerPatternChanged(const PtComponentIntChangeEvent& rEvent);
     //:Method invoked on listener for event id =
     //:PHONE_RINGER_PATTERN_CHANGED
     //:indicating that the PtPhoneRinger audio pattern to be played when ringing has changed.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void phoneRingerInfoChanged(const PtComponentStringChangeEvent& rEvent);
     //:Method invoked on listener for event id =
     //:PHONE_RINGER_INFO_CHANGED
     //:indicating that the PtPhoneRinger info string has changed.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void phoneSpeakerVolumeChanged(const PtComponentIntChangeEvent& rEvent);
     //:Method invoked on listener for event id =
     //:PHONE_SPEAKER_VOLUME_CHANGED
     //:indicating that the associated PtPhoneSpeaker volume has changed.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void phoneMicrophoneGainChanged(const PtComponentIntChangeEvent& rEvent);
     //:Method invoked on listener for event id =
     //:PHONE_MICROPHONE_GAIN_CHANGED
     //:indicating that the associated PtPhoneMicrophone gain has changed.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void phoneLampModeChanged(const PtComponentIntChangeEvent& rEvent);
     //:Method invoked on listener for event id =
     //:PHONE_LAMP_MODE_CHANGED
     //:indicating that the associated PtPhoneLamp mode has changed.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void phoneButtonInfoChanged(const PtComponentStringChangeEvent& rEvent);
     //:Method invoked on listener for event id =
     //:PHONE_BUTTON_INFO_CHANGED
     //:indicating that the associated PtPhoneButton info string has changed.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void phoneButtonUp(const PtTerminalComponentEvent& rEvent);
     //:Method invoked on listener for event id =
     //:PHONE_BUTTON_UP
     //:indicating that the associated PtPhoneButton has changed to the up
     //:(released) position.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void phoneButtonDown(const PtTerminalComponentEvent& rEvent);
     //:Method invoked on listener for event id =
     //:PHONE_BUTTON_DOWN
     //:indicating that the associated PtPhoneButton has changed to the down
     //:(pressed) position.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void phoneButtonRepeat(const PtTerminalComponentEvent& rEvent);
     //:Method invoked on listener for event id =
     //:PHONE_BUTTON_DOWN
     //:indicating that the associated PtPhoneButton has changed to the down
     //:(pressed) position.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void phoneHookswitchOffhook(const PtTerminalComponentEvent& rEvent);
     //:Method invoked on listener for event id =
     //:PHONE_HOOKSWITCH_OFFHOOK
     //:indicating that the PtPhoneHookswitch has changed to the offhook state.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void phoneHookswitchOnhook(const PtTerminalComponentEvent& rEvent);
     //:Method invoked on listener for event id =
     //:PHONE_HOOKSWITCH_ONHOOK
     //:indicating that the PtPhoneHookswitch has changed to the onhook state.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void phoneDisplayChanged(const PtTerminalComponentEvent& rEvent);
     //:Method invoked on listener for event id =
     //:PHONE_DISPLAY_CHANGED
     //:indicates that the PtPhoneDisplay has changed.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void phoneHandsetVolumeChanged(const PtComponentIntChangeEvent& rEvent);
     //:Method invoked on listener for event id =
     //:PHONE_HANDSET_VOLUME_CHANGED
     //:indicating that the phone handset speaker volume has changed.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void phoneExtSpeakerConnected(const PtTerminalComponentEvent& rEvent);
     //:Method invoked on listener for event id =
     //:PHONE_EXTSPEAKER_CONNECTED
     //:indicating that the phone external speaker has been plugged in.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

  virtual void phoneExtSpeakerDisconnected(const PtTerminalComponentEvent& rEvent);
     //:Method invoked on listener for event id =
     //:PHONE_EXTSPEAKER_CONNECTED
     //:indicating that the phone external speaker has been plugged in.
     // The event parameter is valid only within
     // this method.  The implementation must copy the event if
     // it is needed beyond the scope of an invocation.  The implementation
     // of this method should not block as it may prevent other listeners
     // from processing events in a timely fashion.
     //!param: (in) rEvent - Reference to the PtEvent containing the specific event information.

/* ============================ ACCESSORS ================================= */

   static const char* className();
     //:Returns the name of this class
     //!returns: Returns the string representation of the name of this class

/* ============================ INQUIRY =================================== */

   virtual PtBoolean isClass(const char* pClassName);
     //:Determines if this object if of the specified type.
     //!param: (in) pClassName - the string to compare with the name of this class.
     //!retcode: TRUE - if the given string contains the class name of this class.
     //!retcode: FALSE - if the given string does not match that of this class

   virtual PtBoolean isInstanceOf(const char* pClassName);
     //:Determines if this object is either an instance of or is derived from
     //:the specified type.
     //!param: (in) pClassName - the string to compare with the name of this class.
     //!retcode: TRUE - if this object is either an instance of or is derived from the specified class.
     //!retcode: FALSE - if this object is not an instance of the specified class.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
   PtTerminalComponentListener(const PtTerminalComponentListener& rPtTerminalComponentListener);
     //:Copy constructor

   PtTerminalComponentListener& operator=(const PtTerminalComponentListener& rhs);
     //:Assignment operator

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtTerminalComponentListener_h_
