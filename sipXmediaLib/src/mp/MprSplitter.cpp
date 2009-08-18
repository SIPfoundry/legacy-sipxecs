//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsDefs.h"
#include "mp/MpMisc.h"
#include "mp/MpBuf.h"
#include "mp/MprSplitter.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES

// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
MprSplitter::MprSplitter(const UtlString& rName, int numOutputs,
                           int samplesPerFrame, int samplesPerSec)
:  MpResource(rName, 1, 1, 1, numOutputs, samplesPerFrame, samplesPerSec)
{
}

// Destructor
MprSplitter::~MprSplitter()
{
}

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

UtlBoolean MprSplitter::doProcessFrame(MpBufPtr inBufs[],
                                      MpBufPtr outBufs[],
                                      int inBufsSize,
                                      int outBufsSize,
                                      UtlBoolean isEnabled,
                                      int samplesPerFrame,
                                      int samplesPerSecond)
#ifdef OLD_WAY /* [ */
{
    MpBufPtr in = NULL;
    int      i;

    if (0 == outBufsSize) return TRUE;

    if (0 < inBufsSize) in = *inBufs;

    for (i=0; i<outBufsSize; i++) outBufs[i] = NULL;

    if (NULL == in) in = MpMisc.silence;

    // Copy input to each connected output, and
    // rely on caller to release input
    for (i=0; i<outBufsSize; i++) {
        if (isOutputConnected(i)) {
            MpBuf_addRef(in);
            outBufs[i] = in;
            if (!isEnabled) {
                in = MpMisc.silence; // the rest is silence.
            }
        }
    }
    return TRUE;
}

#else /* OLD_WAY ] [ */

{
    MpBufPtr in = NULL;
    int      i;

    if (0 == outBufsSize) return TRUE;

    if (0 < inBufsSize) {
        in = *inBufs;
        *inBufs = NULL;
    }

    for (i=0; i<outBufsSize; i++) outBufs[i] = NULL;

    if (NULL == in) {
        in = MpBuf_getFgSilence(); // Adds a reference!
    }

    if (isEnabled) {
        for (i=0; i<outBufsSize; i++) {
            if (isOutputConnected(i)) {
                MpBuf_addRef(in);
                outBufs[i] = in;
            }
        }
    } else {
        for (i=0; ((i<outBufsSize) && (NULL != in)); i++) {
            if (isOutputConnected(i)) {
                outBufs[i] = in;
                in = NULL;
            }
        }
        if (NULL == in) {
            in = MpBuf_getFgSilence(); // the rest is silence.
            for (; i<outBufsSize; i++) {
                if (isOutputConnected(i)) {
                    MpBuf_addRef(in);
                    outBufs[i] = in;
                }
            }
        }
    }
    MpBuf_delRef(in);
    return TRUE;
}

#endif /* OLD_WAY ] */
/* ============================ FUNCTIONS ================================= */
