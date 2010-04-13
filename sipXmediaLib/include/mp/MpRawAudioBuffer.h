//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _MpRawAudioBuffer_h_
#define _MpRawAudioBuffer_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
*
*/
class MpRawAudioBuffer {
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

/**
* Default constructor
*/
MpRawAudioBuffer(const char* pFilePath);

/**
* Destructor
*/
~MpRawAudioBuffer();

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

char* getAudio(char*& prAudio, unsigned long& rLength);

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
char          *mpAudioBuffer;            // Pointer to the raw audio data
unsigned long mAudioBufferLength;        // Length, in bytes, of the audio data
};

#endif  // _MpRawAudioBuffer_h_
