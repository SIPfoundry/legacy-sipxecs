//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PtMediaCapabilities_h_
#define _PtMediaCapabilities_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <ptapi/PtDefs.h>
// DEFINES
#define CODEC_DELTA             10
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PtAudioCodec;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class PtMediaCapabilities
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   PtMediaCapabilities(PtAudioCodec aAudioCodecs[] = NULL, int numAudioCodecs = 0);
     //:Default constructor
     // This constructor takes an optional array of audio codecs which define
     // the audio capabilities.  Note: the order implies the preference (i.e.
     // the first is prefered over the second, etc.).
     //! param: (in) aAudioCodecs - the array of codecs to be copied into this PtMediaCapabilities object.
     //! param: (in) numAudioCodecs - the number of codecs in the <I>aAudioCodecs</> array.

   PtMediaCapabilities(const PtMediaCapabilities& rPtMediaCapabilities);
     //:Copy constructor

   virtual
   ~PtMediaCapabilities();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PtMediaCapabilities& operator=(const PtMediaCapabilities& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */

   int getNumAudioCodecs() const;
   //: Get the number of codecs contained in this PtMediaCapabilities object.
   //! returns: the number of codecs

   PtBoolean getAudioCodec(int index, PtAudioCodec& codec);
   //: Get the condec contained at the given index
   //! param: (in) index - index indicating which codec is requested.
   //! param: (out) codec - reference to code to which the codec parameters are copied.
   //! retcode: TRUE - if a codec exists at the given index.
   //! retcode: FALSE - if index is invalid.

   void addAudioCodec(PtAudioCodec& codec);
   //: Add an audio codec to this PtMediaCapabilities object.
   // Note: the order in which the codecs are added implies the preference
   // (i.e. The first is prefered over the second and so on.).
   //! param: (in) codec - the codec to be copied and added to this PtMediaCapabilities object.

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    int mSizeAudioCodecs;
    int mNumAudioCodecs;
    PtAudioCodec* mAudioCodecs;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtMediaCapabilities_h_
