//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _TapiMgr_h
#define _TapiMgr_h

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "tapi/sipXtapiEvents.h"
#include "tapi/sipXtapiInternal.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/**
 * The TapiMgr singleton class allows
 * callback funtion-pointers for tapi to be set,
 * indicating the function that is to be called for
 * the firing of sipXtapi events.
 * This obviates the need for sipXcallLib and
 * sipXtackLib to link-in the sipXtapi library.
 */
class TapiMgr
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
    /**
    * Accessor for the single class instance.
    */
    static TapiMgr& getInstance();


   /**
    * Sets the callback function pointer for all Events (to be fired to the sipXtapi layer)
    */
    void setTapiCallback(sipxEventCallbackFn fp);

   /**
    * Sets the callback function pointer for Call Events (to be fired to the sipXtapi layer)
    */
    void setTapiCallCallback(sipxCallEventCallbackFn fp);

   /**
    * Sets the callback function pointer for Line Events (to be fired to the sipXtapi layer)
    */
    void setTapiLineCallback(sipxLineEventCallbackFn fp);


    /**
     * This method calls the Call event callback using the function pointer.
     * It calls the deprecated "Call Callback" in sipXtapi.
     */
    void fireCallEvent(const void*          pSrc,
                       const char*		    szCallId,
                       SipSession*          pSession,
				       const char*          szRemoteAddress,
				       SIPX_CALLSTATE_EVENT eMajorState,
				       SIPX_CALLSTATE_CAUSE eMinorState,
                       void*                pEventData) ;

    /**
     * This method calls the Line event callback using the function pointer.
     * It calls the deprecated "Line Callback" in sipXtapi.
     */
    void fireLineEvent(const void* pSrc,
                        const char* szLineIdentifier,
                        SIPX_LINESTATE_EVENT event,
                        SIPX_LINESTATE_CAUSE cause);

    /**
     * This method calls the new "unified callback" procedure in sipXtapi.
     */
    void fireEvent(const void* pSrc,
                   const SIPX_EVENT_CATEGORY event,
                   void* pInfo);


/* ============================ MANIPULATORS ============================== */
/* ============================ ACCESSORS ================================= */
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
/* ============================ CREATORS ================================== */
    /**
    * TapiMgr contructor. singleton class.
    */
    TapiMgr();

    /**
    * Copy constructor - should never be used.
    */
    TapiMgr(const TapiMgr& src);


    /**
    * TapiMgr destructor.
    */
    virtual ~TapiMgr();

    /**
     * Private, static pointer, holding on to the singleton class instance.
     */
    static TapiMgr* spTapiMgr;

    /**
     * function pointer for the Unified Event callback proc in sipXtapi.
     */
    sipxEventCallbackFn sipxEventCallbackPtr;

    /**
     * function pointer for the Call Event callback proc in sipXtapi.
     */
    sipxCallEventCallbackFn sipxCallEventCallbackPtr;

    /**
     * function pointer for the Line Event callback proc in sipXtapi.
     */
    sipxLineEventCallbackFn sipxLineEventCallbackPtr;

};

#endif /* ifndef _TapiMgr_h_ */
