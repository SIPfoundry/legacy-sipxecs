//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MprFromMic_h_
#define _MprFromMic_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "mp/MpResource.h"
#include "mp/MpCodec.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef void (*MICDATAHOOK)(const int nLength, Sample* samples) ;

// FORWARD DECLARATIONS

class DspResampling;

//:The "From Microphone" media processing resource
class MprFromMic : public MpResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
   enum { MAX_MIC_BUFFERS = 10 };

/* ============================ CREATORS ================================== */

   MprFromMic(const UtlString& rName, int samplesPerFrame, int samplesPerSec);
     //:Constructor

   virtual
   ~MprFromMic();
     //:Destructor

/* ============================ MANIPULATORS ============================== */


/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   enum{EqFilterLen = 24}; // Brant, 11 May 2001; was 13, allow for experiments

   short        shpFilterBuf[80 + 10];
   DspResampling* mpDspResamp;

   int               mNumEmpties;
   int               mNumFrames;

   void  Init_highpass_filter800();
   void  highpass_filter800(short *, short *, short);
   short speech_detected(Sample*, int);


   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame=80,
                                    int samplesPerSecond=8000);

   MprFromMic(const MprFromMic& rMprFromMic);
     //:Copy constructor (not implemented for this class)

   MprFromMic& operator=(const MprFromMic& rhs);
     //:Assignment operator (not implemented for this class)

public:
   static MICDATAHOOK s_fnMicDataHook ;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprFromMic_h_
