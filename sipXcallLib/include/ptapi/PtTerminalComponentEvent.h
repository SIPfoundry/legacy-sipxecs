//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _PtTerminalComponentEvent_h_
#define _PtTerminalComponentEvent_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "ptapi/PtTerminalEvent.h"
#include "ptapi/PtComponent.h"
#include "ptapi/PtPhoneRinger.h"
#include "ptapi/PtPhoneSpeaker.h"
#include "ptapi/PtPhoneMicrophone.h"
#include "ptapi/PtPhoneLamp.h"
#include "ptapi/PtPhoneButton.h"
#include "ptapi/PtPhoneHookswitch.h"
#include "ptapi/PtPhoneDisplay.h"
#include "os/OsDefs.h"
#include "os/OsBSem.h"
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class TaoClientTask;
class TaoObjectMap;

//:PtTerminalComponentEvent contains PtComponent-associated event data

class PtTerminalComponentEvent : public PtTerminalEvent
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   PtTerminalComponentEvent(PtEvent::PtEventId eventId = EVENT_INVALID,
                                                   const char* termName = NULL,
                                                   TaoClientTask *pClient = NULL);
     //:Default constructor

   PtTerminalComponentEvent(const PtTerminalComponentEvent& rPtTerminalComponentEvent);
     //:Copy constructor

   virtual
   ~PtTerminalComponentEvent();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PtTerminalComponentEvent& operator=(const PtTerminalComponentEvent& rhs);
     //:Assignment operator

   PtStatus getComponent(PtComponent*& rpComponent);
     //:Returns the component object associated with this event.
     //!param: (out) prComponent - The reference used to return the component pointer
     //!retcode: PT_SUCCESS - Success
     //!retcode: PT_PROVIDER_UNAVAILABLE - The provider is not available

   void setStringData1(const char* stringData);
   void setStringData2(const char* stringData);
   void setStringData3(const char* stringData);

   void setIntData1(int intData);
   void setIntData2(int intData);
   void setIntData3(int intData);

/* ============================ ACCESSORS ================================= */
   void getStringData1(char* stringData);
   void getStringData2(char* stringData);
   void getStringData3(char* stringData);

   int getIntData1();
   int getIntData2();
   int getIntData3();

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

        int             mIntData1;
        int             mIntData2;
        int             mIntData3;

    UtlString mStringData1;
    UtlString mStringData2;
    UtlString mStringData3;

        PtPhoneRinger           *mpRinger;
        PtPhoneSpeaker          *mpSpeaker;
        PtPhoneMicrophone       *mpMic;
        PtPhoneLamp                     *mpLamp;
        PtPhoneButton           *mpButton;
        PtPhoneHookswitch       *mpHooksw;
        PtPhoneDisplay          *mpDisplay;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtTerminalComponentEvent_h_
