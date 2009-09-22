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
// APPLICATION INCLUDES
#include "net/TapiMgr.h"
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
TapiMgr* TapiMgr::spTapiMgr = 0;

// Constructor - private, this is a singleton
TapiMgr::TapiMgr() :
    sipxCallEventCallbackPtr(0),
    sipxLineEventCallbackPtr(0)
{
}

// destructor - private
TapiMgr::~TapiMgr()
{
}

// static accessor for the singleton instance
TapiMgr& TapiMgr::getInstance()
{
   if (TapiMgr::spTapiMgr == 0)
   {
      TapiMgr::spTapiMgr = new TapiMgr();
   }
   return *(TapiMgr::spTapiMgr);
}

// setting the Call event callback pointer
void TapiMgr::setTapiCallCallback(sipxCallEventCallbackFn fp)
{
    sipxCallEventCallbackPtr = fp;
}

// setting the Line event callback pointer
void TapiMgr::setTapiLineCallback(sipxLineEventCallbackFn fp)
{
    sipxLineEventCallbackPtr = fp;
}

void TapiMgr::setTapiCallback(sipxEventCallbackFn fp)
{
    sipxEventCallbackPtr = fp;
}

void TapiMgr::fireCallEvent(const void*          pSrc,
                       const char*		    szCallId,
                       SipSession*          pSession,
				       const char*          szRemoteAddress,
				       SIPX_CALLSTATE_EVENT eMajorState,
				       SIPX_CALLSTATE_CAUSE eMinorState,
                       void*                pEventData)
{
    if (sipxCallEventCallbackPtr)
    {
        (*sipxCallEventCallbackPtr)(pSrc, szCallId, pSession, szRemoteAddress,
                                    (SIPX_CALLSTATE_MAJOR)(int)eMajorState, (SIPX_CALLSTATE_MINOR)(int)eMinorState, pEventData);
    }
    return;
}

void TapiMgr::fireLineEvent(const void* pSrc,
                        const char* szLineIdentifier,
                        SIPX_LINESTATE_EVENT event,
                        SIPX_LINESTATE_CAUSE cause)
{
    if (sipxLineEventCallbackPtr)
    {
        (*sipxLineEventCallbackPtr)(pSrc, szLineIdentifier,
                                    (SIPX_LINE_EVENT_TYPE_MAJOR)(int)event,
                                    (SIPX_LINE_EVENT_TYPE_MINOR)(int)cause);
    }
}

void TapiMgr::fireEvent(const void* pSrc, const SIPX_EVENT_CATEGORY event, void *pInfo)
{
    if (sipxEventCallbackPtr)
    {
        (*sipxEventCallbackPtr)(pSrc, event, pInfo);
    }
}
