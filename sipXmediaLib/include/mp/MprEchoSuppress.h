//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _MprEchoSuppress_h_
#define _MprEchoSuppress_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsStatus.h"
#include "mp/MpResource.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

class DspResampling;
class MprToSpkr;
class FilterBank;
class HandsetFilterBank;

//:The "From Microphone" media processing resource
class MprEchoSuppress : public MpResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
   MprEchoSuppress(const UtlString& rName,
                           int samplesPerFrame, int samplesPerSec);
     //:Constructor

   virtual
   ~MprEchoSuppress();
     //:Destructor



/* ============================ MANIPULATORS ============================== */

void setSpkrPal(MprToSpkr* pal);

int startSpeech();
int endSpeech();

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   short                mState;
   MprToSpkr*           mpSpkrPal;
   MpBufPtr             mpPrev;
   int                  mTicksPerFrame;
   int                  mLastSpkrAtten;
   int                  mSpeechFake;
   short                mshDelay;
   FilterBank*          mpFilterBank;
   HandsetFilterBank*   mpHandsetFilterBank;

   DspResampling*       mpDspResampSpk;

   MpBufPtr LoudspeakerFade(MpBufPtr in, short& shSpkState, int iFreezeFlag);

   void frame_match(MpBufPtr in);

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame=80,
                                    int samplesPerSecond=8000);

   void control_logic(unsigned long       ulSigIn,
                      unsigned long       ulSigOut,
                              short&      shSpkState,
                                int       iFreezeFlag);

   MprEchoSuppress(const MprEchoSuppress& rMprEchoSuppress);
     //:Copy constructor (not implemented for this class)

   MprEchoSuppress& operator=(const MprEchoSuppress& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprEchoSuppress_h_
