//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _MprToneGen_h_
#define _MprToneGen_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "mp/dtmflib.h"
#include "mp/MpFlowGraphMsg.h"
#include "mp/MpResource.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:The "Tone Generator" media processing resource
class MprToneGen : public MpResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   MprToneGen(const UtlString& rName, int samplesPerFrame, int samplesPerSec,
       const char* locale);
     //:Constructor

   virtual
   ~MprToneGen();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

#ifdef LATER
Later (soon) this will be incorporated, but this is not quite the right
implementation.  At least these changes are needed:
(1) this should be an overriding virtual function, named
    handleSetSamplesPerSec.
(2) MpResource (the base class) needs to be enhanced so that the base
    virtual function exists to be overridden.
   virtual UtlBoolean setSamplesPerSec(int samplesPerSec);
     //:Sets the number of samples expected per second.
     // Returns FALSE if the specified rate is not supported, TRUE otherwise.
#endif

   OsStatus startTone(int toneId);
     //:Sends a START_TONE message to this resource to begin generating
     //:an audio tone.
     // Returns the result of attempting to queue the message to this
     // resource.

   OsStatus stopTone(void);
     //:Sends a STOP_TONE message to this resource to stop generating
     //:an audio tone.
     // Returns the result of attempting to queue the message to this
     // resource.

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   enum AddlMsgTypes
   {
      START_TONE = MpFlowGraphMsg::RESOURCE_SPECIFIC_START,
      STOP_TONE
   };

   static const int MIN_SAMPLE_RATE;
   static const int MAX_SAMPLE_RATE;

   MpToneGenPtr mpToneGenState;

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame,
                                    int samplesPerSecond);

   virtual UtlBoolean handleMessage(MpFlowGraphMsg& rMsg);
     //:Handle messages for this resource.

   MprToneGen(const MprToneGen& rMprToneGen);
     //:Copy constructor (not implemented for this class)

   MprToneGen& operator=(const MprToneGen& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprToneGen_h_
