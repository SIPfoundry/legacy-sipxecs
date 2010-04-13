//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _SipXtapiEventDispatcher_h_
#define _SipXtapiEventDispatcher_h_

// SYSTEM INCLUDES
#include <os/OsServerTask.h>
#include <tapi/sipXtapi.h>
#include <tapi/sipXtapiEvents.h>
#include <tapi/sipXtapiInternal.h>

// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//
//! Task to collect, process and dispatch call events to the sipXtapi event callbacks
/*! This task is for events that need to be queue to a OsServerTask
 *  that are to be fired through the sipXtapi callbacks.
 *  Initially this is DTMF tones
 *
 */

class SipXtapiEventDispatcher : public OsServerTask
{
/* ============================= P U B L I C ============================== */
public:


/* ============================ C R E A T O R S =========================== */

    //! Constructor
    /*! description
     */
    SipXtapiEventDispatcher(SIPX_INSTANCE_DATA& instance);

    //! Destructor
    virtual
    ~SipXtapiEventDispatcher();

/* ======================== M A N I P U L A T O R S ======================= */

    UtlBoolean handleMessage(OsMsg& message);

    void dtmfToSipXtapiMinorEvent(int dtmfButton, SIPX_CALLSTATE_MINOR& sipxTapiMinorEvent);

/* ========================== A C C E S S O R S =========================== */

/* ============================ I N Q U I R Y ============================= */

//__________________________________________________________________________//
/* ========================== P R O T E C T E D =========================== */
protected:


//__________________________________________________________________________//
/* ============================ P R I V A T E ============================= */
private:


    //! Disabled copy constructor
    SipXtapiEventDispatcher(const SipXtapiEventDispatcher& rSipXtapiEventDispatcher);

    //! Disabled assignment operator
    SipXtapiEventDispatcher& operator=(const SipXtapiEventDispatcher& rhs);

    SIPX_INSTANCE_DATA* mpInstance;

};


#endif  // _SipXtapiEventDispatcher_h_
