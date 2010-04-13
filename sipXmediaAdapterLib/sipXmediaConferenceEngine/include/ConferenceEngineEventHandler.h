//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////


#ifndef _ConferenceEngineEventHandler_h_
#define _ConferenceEngineEventHandler_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "ConferenceEngine.h"

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
class ConferenceEngineEventHandler : public GIPS_callback_handler
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    /**
     * Default constructor
     */
    ConferenceEngineEventHandler();


    /**
     * Destructor
     */
    virtual ~ConferenceEngineEventHandler();

/* ============================ MANIPULATORS ============================== */

    /**
     * A user sent an AVT event (see RFC 2833 for details)
     * Event 0-15 are DTMF tones.
     */
    virtual void ReceivedOutbandDTMF(void* anyPtr, int channel, int value);

    virtual void ReceivedInbandDTMF(void* anyPtr, int channel, int tone);

   /**
    * A file played to a channel is ended, channel -1 eqauls a file that
    * was played to the whole meeting
    */
    virtual void PlayedFileEnded(void* anyPtr, int channel, const char* fileName);

    virtual void EventMessage(void* anyPtr, GIPS_EVENT_MESSAGE type, const char* str);

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:


};

/* ============================ INLINE METHODS ============================ */

#endif  // _ConferenceEngineEventHandler_h_
