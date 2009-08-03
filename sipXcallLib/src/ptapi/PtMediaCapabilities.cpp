//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "utl/UtlDefs.h"

#include "ptapi/PtMediaCapabilities.h"
#include "ptapi/PtAudioCodec.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
PtMediaCapabilities::PtMediaCapabilities(PtAudioCodec aAudioCodecs[], int numAudioCodecs)
{
        mNumAudioCodecs = numAudioCodecs;

        mSizeAudioCodecs = (numAudioCodecs / CODEC_DELTA + 1 )*CODEC_DELTA;

        mAudioCodecs = new PtAudioCodec[mSizeAudioCodecs];

        if (aAudioCodecs != NULL)
                for (int i=0; i<numAudioCodecs; i++)
                {
                        mAudioCodecs[i] = aAudioCodecs[i];
                }

}

// Copy constructor
PtMediaCapabilities::PtMediaCapabilities(const PtMediaCapabilities& rPtMediaCapabilities)
{
        mSizeAudioCodecs = rPtMediaCapabilities.mSizeAudioCodecs;
        mNumAudioCodecs = rPtMediaCapabilities.mNumAudioCodecs;

        mAudioCodecs = new PtAudioCodec[mSizeAudioCodecs];

        for (int i=0; i<mNumAudioCodecs; i++)
        {
                mAudioCodecs[i] = rPtMediaCapabilities.mAudioCodecs[i];
        }

}

// Destructor
PtMediaCapabilities::~PtMediaCapabilities()
{
        if (mAudioCodecs)
        {
                delete[] mAudioCodecs;
                mAudioCodecs = 0;
        }
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
PtMediaCapabilities&
PtMediaCapabilities::operator=(const PtMediaCapabilities& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   delete[] mAudioCodecs;

   mSizeAudioCodecs = rhs.mSizeAudioCodecs;
   mNumAudioCodecs = rhs.mNumAudioCodecs;

   mAudioCodecs = new PtAudioCodec[mSizeAudioCodecs];

   for (int i=0; i<mNumAudioCodecs; i++)
   {
           mAudioCodecs[i] = rhs.mAudioCodecs[i];
   }

   return *this;
}

/* ============================ ACCESSORS ================================= */
// Get the number of codecs contained in this object
int PtMediaCapabilities::getNumAudioCodecs() const
{
        return mNumAudioCodecs;
}

// get the codec contained at the given index
PtBoolean PtMediaCapabilities
::getAudioCodec(int index, PtAudioCodec& codec)
{
        if (index >= mNumAudioCodecs || index <= 0)
                return FALSE;
        else
        {
                codec = mAudioCodecs[index];
                return TRUE;
        }
}

// add audio codec to the object
void PtMediaCapabilities::addAudioCodec(PtAudioCodec& codec)
{
        if (mNumAudioCodecs + 1 <= mSizeAudioCodecs)
                mAudioCodecs[mNumAudioCodecs] = codec;
        else
        {
                int i;
                mSizeAudioCodecs = mSizeAudioCodecs + CODEC_DELTA;
                PtAudioCodec*   pTempCodecs;

                pTempCodecs = new PtAudioCodec[mSizeAudioCodecs];
                for (i=0; i<mNumAudioCodecs; i++)
                {
                        pTempCodecs[i] = mAudioCodecs[i];
                }
                pTempCodecs[mNumAudioCodecs+1] = codec;

                delete[] mAudioCodecs;

                mAudioCodecs = new PtAudioCodec[mSizeAudioCodecs];
                for (i=0; i<=mNumAudioCodecs; i++)
                {
                        mAudioCodecs[i] = pTempCodecs[i];
                }

                delete[] pTempCodecs;
        }
        mNumAudioCodecs++;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
