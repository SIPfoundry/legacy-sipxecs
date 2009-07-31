//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PsMsg_h_
#define _PsMsg_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsMsg.h"
// DEFINES
#define PSMSG_MAX_STRINGPARAM_LENGTH    1024
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlString;

//:Message object used to communicate phone set information
class PsMsg : public OsMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   // Phone set message types
   enum PsMsgType
   {
/* ----------------------------- LOWLEVEL --------------------------------- */
                BUTTON_DOWN,
                BUTTON_REPEAT,
                BUTTON_UP,
                KEY_DOWN,  //for true keyboard support on NT
                KEY_UP,
                HOOKSW_STATE,
      KEYPAD_STATE,
      SCROLL_CHANGE,
      SCROLL_STATE,
      TOUCHSCREEN_CHANGE,
      TOUCHSCREEN_STATE,
/* ----------------------------- PHONEBUTTON ------------------------------ */
                BUTTON_PRESS,
                BUTTON_GET_INFO,
                BUTTON_SET_INFO,
                BUTTON_GET_PHONELAMP,
/* ----------------------------- PHONEHOOKSWITCH -------------------------- */
                HOOKSW_GET_STATE,
                HOOKSW_SET_STATE,
                HOOKSW_GET_CALL,
/* ----------------------------- PHONELAMP -------------------------------- */
                LAMP_GET_MODE,
                LAMP_GET_SUPPORTED_MODES,
                LAMP_GET_BUTTON,
                LAMP_SET_MODE,
/* ----------------------------- PHONEDISPLAY ----------------------------- */
                DISPLAY_GET_DISPLAY,
                DISPLAY_GET_ROWS,
                DISPLAY_GET_COLS,
                DISPLAY_SET_DISPLAY,
/* ----------------------------- PHONEMIC --------------------------------- */
                MIC_GET_GAIN,
                MIC_SET_GAIN,
/* ----------------------------- PHONERINGER ------------------------------ */
                RINGER_SET_INFO,
                RINGER_SET_PATTERN,
                RINGER_SET_VOLUME,
                RINGER_GET_INFO,
                RINGER_GET_PATTERN,
                RINGER_GET_VOLUME,
                RINGER_GET_MAX_PATTERN_INDEX,
                RINGER_GET_NUMBER_OF_RINGS,
                RINGER_IS_ON,
/* ----------------------------- PHONESPEAKER ----------------------------- */
                SPEAKER_SET_VOLUME,
                SPEAKER_GET_VOLUME,
                EXTSPEAKER_CONNECT,
                EXTSPEAKER_DISCONNECT,
/* ----------------------------- PHONECOMPONENT --------------------------- */
                PHONECOMPONENT_GET_TYPE,
                PHONECOMPONENT_GET_NAME,
/* ----------------------------- PHONECOMPONENT --------------------------- */
                HANDSET_SET_VOLUME,
                HANDSET_SET_GAIN,
/* ----------------------------- PHONEGROUP ------------------------------- */
                PHONEGROUP_ACTIVATE,
                PHONEGROUP_DEACTIVATE,
                PHONEGROUP_GET_COMPONENTS,
                PHONEGROUP_GET_DESCRIPTION,
                PHONEGROUP_GET_TYPE,
                PHONEGROUP_IS_ACTIVATED,
/* ----------------------------- TERMINAL --------------------------------- */
                TERMINAL_GET_COMPONENT,
                TERMINAL_GET_COMPONENTS,
                TERMINAL_GET_COMPONENTGROUPS,
        };

/* ============================ CREATORS ================================== */

   PsMsg(int msg, void* source, const int param1, const int param2);
     //:Constructor

   PsMsg(const PsMsg& rPsMsg);
     //:Copy constructor

   virtual OsMsg* createCopy(void) const;
     //:Create a copy of this msg object (which may be of a derived type)

   virtual
   ~PsMsg();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PsMsg& operator=(const PsMsg& rhs);
     //:Assignment operator

   void setMsgSource(void* source);
     //:Set the message source

   void setParam1(int param1);
     //:Set parameter1 of the phone set message

   void setParam2(int param2);
     //:Set parameter2 of the phone set message

   void setStringParam1(const char* str);
     //:Set string parameter1 of the phone set message

   void setStringParam2(const char* str);
     //:Set string parameter2 of the phone set message


   void setInUse(UtlBoolean isInUse);
     //:Set the InUse flag for the message.
     // For messages sent from an ISR, TRUE indicates that the receiver is
     // not done with the message yet.  The InUse flag is ignored for
     // messages that were not sent from an ISR.

/* ============================ ACCESSORS ================================= */

   virtual int getMsg(void) const;
     //:Return the type of the phone set message

   virtual void* getMsgSource(void) const;
     //:Return the message source

   virtual int getParam1(void) const;
     //:Return parameter1 of the message

   virtual int getParam2(void) const;
     //:Return parameter2 of the message

   void getStringParam1(UtlString& stringData);
     //:Return string parameter1 of the message

   void getStringParam2(UtlString& stringData);
     //:Return string parameter2 of the message

/* ============================ INQUIRY =================================== */

   UtlBoolean isInUse(void);
     //:Returns the value of the InUse flag for the message.
     // For messages sent from an ISR, TRUE indicates that the receiver is
     // not done with the message yet.  The InUse flag is ignored for
     // messages that were not sent from an ISR.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   UtlBoolean mInUse;       // For messages sent from an ISR, indicates that
   void*     mMsgSource;   // Message source
   int       mParam1;      // Message parameter 1
   int       mParam2;      // Message parameter 2
   char         mStringParam1[PSMSG_MAX_STRINGPARAM_LENGTH + 1];      // String parameter 1
   char         mStringParam2[PSMSG_MAX_STRINGPARAM_LENGTH + 1];      // String parameter 2
                           //  the receiver is not done with the message yet

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PsMsg_h_
