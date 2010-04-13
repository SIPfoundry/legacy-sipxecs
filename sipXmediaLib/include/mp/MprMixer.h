//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MprMixer_h_
#define _MprMixer_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
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

//:The "Mixer" media processing resource
class MprMixer : public MpResource
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   MprMixer(const UtlString& rName, int numWeights,
                               int samplesPerFrame, int samplesPerSec);
     //:Constructor

   virtual
   ~MprMixer();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   UtlBoolean setWeights(int *newWeights, int numWeights);
     //:Sets the weighting factors for the first "numWeights" inputs.
     // For now, this method always returns TRUE.

   UtlBoolean setWeight(int newWeight, int weightIndex);
     //:Sets the weighting factor for the "weightIndex" input.
     // For now, this method always returns TRUE.

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   enum AddlMsgTypes
   {
      SET_WEIGHT  = MpFlowGraphMsg::RESOURCE_SPECIFIC_START,
      SET_WEIGHTS
   };

   enum { MAX_MIXER_INPUTS = 10 };

   int mWeights[MAX_MIXER_INPUTS];
   int mNumWeights;
   int mScale;

   virtual UtlBoolean doProcessFrame(MpBufPtr inBufs[],
                                    MpBufPtr outBufs[],
                                    int inBufsSize,
                                    int outBufsSize,
                                    UtlBoolean isEnabled,
                                    int samplesPerFrame=80,
                                    int samplesPerSecond=8000);

   virtual UtlBoolean handleMessage(MpFlowGraphMsg& rMsg);
     //:Handle messages for this resource.

   UtlBoolean handleSetWeight(int newWeight, int weightIndex);
     //:Handle the SET_WEIGHT message.

   UtlBoolean handleSetWeights(int *newWeights, int numWeights);
     //:Handle the SET_WEIGHTS message.

   MprMixer(const MprMixer& rMprMixer);
     //:Copy constructor (not implemented for this class)

   MprMixer& operator=(const MprMixer& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _MprMixer_h_
