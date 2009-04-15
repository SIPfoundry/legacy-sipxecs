// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#define OSBREC_EXPORTS
#include "OSBrec.h"
#include "osbrec_utils.h"
#include "OSBprompt.h"
#include "VXI/PromptManager.hpp"

#include <assert.h>
#include <stdlib.h>
#include <time.h>
#include <cstdio>
#include <cstring>

#include "VXIvalue.h"
#include "VXI/VXML.h"
#include "VXI/CommonExceptions.hpp"
#include "ivr/clientMain.h"
#include "os/OsFS.h"
#include "os/OsLock.h"
#include "os/OsDateTime.h"
#include "os/OsQueuedEvent.h"
#include "os/OsEventMsg.h"
#include "mp/MprRecorder.h"
#ifdef DMALLOC
#include <dmalloc.h>
#endif

// ------*---------*---------*---------*---------*---------*---------*---------

class VXIVectorHolder
{
public:
    VXIVectorHolder() :
        _vector(NULL)
    {
        _vector = VXIVectorCreate();
    }
    VXIVectorHolder(VXIVector * m) :
        _vector(m)
    {
    }
    ~VXIVectorHolder()
    {
        if (_vector != NULL)
            VXIVectorDestroy(&_vector);
    }

    VXIVectorHolder & operator=(const VXIVectorHolder & x)
    {
        if (this != &x)
        {
            if (_vector != NULL)
                VXIVectorDestroy(&_vector);
            _vector = VXIVectorClone(x._vector);
        }
        return *this;
    }

    // GetValue returns the internal vector.  The standard vector manipulation
    // functions may then be used.

    VXIVector * GetValue() const
    {
        return _vector;
    }

    // These functions allow the holder to take ownership of an existing vector
    // or to release the internal one.

    VXIVector * Release()
    {
        VXIVector * m = _vector;
        _vector = NULL;
        return m;
    }

    void Acquire(VXIVector * m)
    {
        if (_vector != NULL)
            VXIVectorDestroy(&_vector);
        else
            _vector = m;
    }

private:
    VXIVectorHolder(const VXIVectorHolder &); // intentionally not defined.

    VXIVector * _vector;
};

// ------*---------*---------*---------*---------*---------*---------*---------

// Global for the base diagnostic tag ID, see osbrec_utils.h for tags
//
static VXIunsigned gblDiagLogBase = 0;
static const VXIunsigned DIAG_TAG_REC = 0;

char DTMF_CHAR[17] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '*',
        '#', 'A', 'B', 'C', 'D', 'F' };

// OSBrec implementation of the VXIrec interface
//
extern "C"
{
struct OSBrecImpl
{
    // Base interface, must be first
    VXIrecInterface intf;
    // Class for managing grammars
    OSBrecData *recData;
    OsMsgQ mIncomingQ;
    // Log interface for this resource
    VXIlogInterface *log;
    CallManager *pCallMgr;
    OsBSem *pExitGuard;
    VXIpromptInterface *prompt;
    VXIchar *callId;
    int live; // session state 1 is alive, 0 exiting, -1 not_initiated.
    int recording;
    int hungup; // caller has hung up.
    VXIunsigned channel;
    OsQueuedEvent* dtmfEvent; // this event is used throughout the session.
};
}

// A few conversion functions...

static inline VXIrecGrammar * ToVXIrecGrammar(OSBrecGrammar * i)
{
    return reinterpret_cast<VXIrecGrammar *>(i);
}

static inline OSBrecGrammar * FromVXIrecGrammar(VXIrecGrammar * i)
{
    return reinterpret_cast<OSBrecGrammar *>(i);
}

static inline OSBrecData * GetRecData(VXIrecInterface * i)
{
    if (i == NULL)
        return NULL;
    return reinterpret_cast<OSBrecImpl *>(i)->recData;
}

/*******************************************************
 * Method routines for VXIrecInterface structure
 *******************************************************/

// Get the VXIrec interface version supported
//
static VXIint32 OSBrecGetVersion(void)
{
    return VXI_CURRENT_VERSION;
}

// Get the implementation name
//
static const VXIchar* OSBrecGetImplementationName(void)
{
    static const VXIchar IMPLEMENTATION_NAME[]= COMPANY_DOMAIN L".OSBrec";
    return IMPLEMENTATION_NAME;
}

static VXIrecResult OSBrecBeginSession(VXIrecInterface * pThis,
        VXIMap *sessionArgs)
{
    OSBrecData *tp = GetRecData(pThis);
    if (tp == NULL)
        return VXIrec_RESULT_INVALID_ARGUMENT;

    OSBrecImpl* impl = reinterpret_cast<OSBrecImpl*>(pThis);
    impl->live = -1; // not quite live yet, other threads shouldn't access its resources

    impl->recording = 0;

    tp->Diag(DIAG_TAG_REC, NULL,L"rec BeginSession" );

    impl->pExitGuard = new OsBSem(OsBSem::Q_PRIORITY, OsBSem::FULL);
    if (impl->pExitGuard)
    {
        OsLock lock(*(impl->pExitGuard));
        const VXIValue *val;
        impl->callId = NULL;
        val = VXIMapGetProperty(sessionArgs, L"callid");
        if ((val) && (VXIValueGetType(val) == VALUE_STRING))
        {
            const VXIchar *callId = NULL;
            callId = VXIStringCStr((const VXIString *) val);
            int len = strlen((char*)callId) + 1;

            VXIchar *vxiCallid = (VXIchar *) calloc(len, sizeof(VXIchar));
            if (vxiCallid)
            {
                strncpy((char*)vxiCallid, (char*)callId, len);
                impl->callId = vxiCallid;
            }
            else
                return VXIrec_RESULT_OUT_OF_MEMORY;
        }
        else
            return VXIrec_RESULT_INVALID_ARGUMENT;

        if (impl->hungup != 1 && impl->pCallMgr)
        {
            OsQueuedEvent* dtmfEvent = new OsQueuedEvent(impl->mIncomingQ, (void*)1);
            if (!dtmfEvent)
               return VXIrec_RESULT_OUT_OF_MEMORY;

            VXIint interdigitTime = VXI_TIMEOUT_DEFAULT;
            impl->pCallMgr->enableDtmfEvent((const char*)impl->callId,
                    interdigitTime/1000,
                    dtmfEvent,
                    1);
            impl->dtmfEvent = dtmfEvent;
            impl->live = 1; // live
        }
        else
        {
            if (impl->callId)
            {
                free(impl->callId);
                impl->callId = NULL;
            }
            impl->live = 0; // don't set live flag since caller has hung up.
            tp->Diag(DIAG_TAG_REC, NULL, L"rec BeginSession live=%d hungup=%d", impl->live, impl->hungup);
            return VXIrec_RESULT_DISCONNECT;
        }
    }

    return VXIrec_RESULT_SUCCESS;
}

static VXIrecResult OSBrecEndSession(VXIrecInterface *pThis, VXIMap *)
{
    OSBrecData* tp = GetRecData(pThis);
    if (tp == NULL)
        return VXIrec_RESULT_INVALID_ARGUMENT;

    tp->Clear();

    OSBrecImpl* impl = reinterpret_cast<OSBrecImpl*>(pThis);
    if (!impl)
        return VXIrec_RESULT_INVALID_ARGUMENT;

    if (impl->pExitGuard)
    {
        impl->pExitGuard->acquire();
        impl->live = 0;
        tp->Diag(DIAG_TAG_REC, NULL,L"Rec end session" );

        if (impl->callId)
        {
            if (impl->dtmfEvent)
            {
                impl->pCallMgr->removeDtmfEvent((const char*)impl->callId, (void*)impl->dtmfEvent);
                // 08/19/03 (rschaaf):
                // Since this event may still be in use, it must be the responsibility of
                // the recipient of the removeDtmf message to delete the event.
                // delete dtmfEvent;
                impl->dtmfEvent = NULL;
            }

            free(impl->callId);
            impl->callId = NULL;
        }

        impl->pExitGuard->release();
    }
    return VXIrec_RESULT_SUCCESS;
}

static VXIrecResult OSBrecLoadGrammarFromURI(struct VXIrecInterface *pThis,
        const VXIMap *properties, const VXIchar *type, const VXIchar *uri,
        const VXIMap *uriArgs, VXIrecGrammar **gram)
{
    OSBrecData* tp = GetRecData(pThis);
    if (tp == NULL)
        return VXIrec_RESULT_INVALID_ARGUMENT;

    if (!properties && wcsncmp(uri,L"builtin:" , 8) != 0)
        return VXIrec_RESULT_FATAL_ERROR;

    OSBrecGrammar * gp = NULL;
    if (wcsncmp(uri, L"builtin:dtmf", 12) == 0)
    {
        // Create a wordlist.
        const char NEXT = ';';
        vxistring gm(uri);
        vxistring min;
        vxistring max;
        size_t idx = 0;

        if ((idx = gm.find(L"digits?")) != vxistring::npos)
        {
            idx += 7;
            size_t pos = gm.length();
            vxistring tmpstr = gm.substr(idx, pos - idx);
            if ((pos = tmpstr.find(NEXT)) != vxistring::npos)
            {
                gm = tmpstr.substr(0, pos);
                tmpstr = tmpstr.substr((pos + 1), tmpstr.length() - pos);
                if (((pos = gm.find(L"minlength=")) != vxistring::npos) ||
                        ((pos = gm.find(L"mindigits=")) != vxistring::npos))
                {
                    min = gm.substr(pos + 10, gm.length() - pos);
                }
                else if (((pos = gm.find(L"maxlength=")) != vxistring::npos) ||
                        ((pos = gm.find(L"maxdigits=")) != vxistring::npos))
                {
                    max = gm.substr(pos + 10, gm.length() - pos);
                }

                if (((pos = tmpstr.find(L"minlength=")) != vxistring::npos) ||
                        ((pos = tmpstr.find(L"mindigits=")) != vxistring::npos))
                {
                    min = tmpstr.substr(pos + 10, tmpstr.length() - pos);
                }
                else if (((pos = tmpstr.find(L"maxlength=")) != vxistring::npos) ||
                        ((pos = tmpstr.find(L"maxdigits=")) != vxistring::npos))
                {
                    max = tmpstr.substr(pos + 10, tmpstr.length() - pos);
                }
                vxistring str = L"(0|1|2|3|4|5|6|7|8|9|*|#){";
                str.append(min);
                str.append(L"-");
                str.append(max);
                str.append(L"}");
                gp = tp->CreateWordListFromString(str.c_str());
                if (gp == NULL) return VXIrec_RESULT_FAILURE;
                tp->AddGrammar(gp);
                *gram = ToVXIrecGrammar(gp);
            }
            else if (((pos = tmpstr.find(L"minlength=")) != vxistring::npos) ||
                    ((pos = tmpstr.find(L"mindigits=")) != vxistring::npos))
            {
                tmpstr = tmpstr.substr(pos + 10, tmpstr.length() - pos);
                vxistring str = L"(0|1|2|3|4|5|6|7|8|9|*|#){";
                str.append(tmpstr);
                str.append(L"-}");
                gp = tp->CreateWordListFromString(str.c_str());
                if (gp == NULL) return VXIrec_RESULT_FAILURE;
                tp->AddGrammar(gp);
                *gram = ToVXIrecGrammar(gp);
            }
            else if (((pos = tmpstr.find(L"maxlength=")) != vxistring::npos) ||
                    ((pos = tmpstr.find(L"maxdigits=")) != vxistring::npos))
            {
                tmpstr = tmpstr.substr(pos + 10, tmpstr.length() - pos);
                vxistring str = L"(0|1|2|3|4|5|6|7|8|9|*|#){";
                str.append(L"-");
                str.append(tmpstr);
                str.append(L"}");
                gp = tp->CreateWordListFromString(str.c_str());
                if (gp == NULL) return VXIrec_RESULT_FAILURE;
                tp->AddGrammar(gp);
                *gram = ToVXIrecGrammar(gp);
            }
            else if (((pos = tmpstr.find(L"length=")) != vxistring::npos) ||
                    ((pos = tmpstr.find(L"digits=")) != vxistring::npos))
            {
                tmpstr = tmpstr.substr(pos + 7, tmpstr.length() - pos);
                vxistring str = L"(0|1|2|3|4|5|6|7|8|9|*|#){";
                str.append(tmpstr);
                str.append(L"}");
                gp = tp->CreateWordListFromString(str.c_str());
                if (gp == NULL) return VXIrec_RESULT_FAILURE;
                tp->AddGrammar(gp);
                *gram = ToVXIrecGrammar(gp);
            }
        }
    }

    return VXIrec_RESULT_SUCCESS;
}

static VXIrecResult OSBrecLoadGrammarFromString(VXIrecInterface *pThis,
        const VXIMap *prop, const VXIchar *type, const VXIchar *str,
        VXIrecGrammar **gram)
{
    // Check the arguments
    OSBrecData* tp = GetRecData(pThis);
    if (tp == NULL)
        return VXIrec_RESULT_INVALID_ARGUMENT;
    if (str == NULL)
        return VXIrec_RESULT_INVALID_ARGUMENT;

    OSBrecGrammar * gp= NULL;

    // We ignore the type field, but different types could be created in future.
    if (1)
    {
        // Create a wordlist.
        gp = tp->CreateWordListFromString(str);
        if (gp == NULL)
            return VXIrec_RESULT_FAILURE;
    }

    tp->AddGrammar(gp);
    *gram = ToVXIrecGrammar(gp);
    return VXIrec_RESULT_SUCCESS;
}

static VXIrecResult OSBrecFreeGrammar(VXIrecInterface *pThis,
        VXIrecGrammar **gram)
{
    // Check the arguments
    OSBrecData* tp = GetRecData(pThis);
    if (tp == NULL)
        return VXIrec_RESULT_INVALID_ARGUMENT;

    // If the grammar pointer is null, there is nothing to free.
    if (gram == NULL || *gram == NULL)
        return VXIrec_RESULT_SUCCESS;

    tp->Diag(DIAG_TAG_REC, NULL,L"Rec OSBrecFreeGrammar 0x%p 0x%p" , gram, (*gram));
    tp->FreeGrammar(FromVXIrecGrammar(*gram));
    *gram = NULL;
    return VXIrec_RESULT_SUCCESS;
}

static VXIrecResult OSBrecActivateGrammar(VXIrecInterface *pThis,
        const VXIMap *properties, VXIrecGrammar *gram)
{
    OSBrecData* tp = GetRecData(pThis);

    if (tp == NULL)
        return VXIrec_RESULT_INVALID_ARGUMENT;
    if (gram == NULL)
        return VXIrec_RESULT_INVALID_ARGUMENT;

    FromVXIrecGrammar(gram)->SetEnabled(true);
    return VXIrec_RESULT_SUCCESS;
}

static VXIrecResult OSBrecDeactivateGrammar(VXIrecInterface *pThis,
        VXIrecGrammar *gram)
{
    OSBrecData* tp = GetRecData(pThis);

    if (tp == NULL)
        return VXIrec_RESULT_INVALID_ARGUMENT;
    if (gram == NULL)
        return VXIrec_RESULT_INVALID_ARGUMENT;

    FromVXIrecGrammar(gram)->SetEnabled(false);
    return VXIrec_RESULT_SUCCESS;
}

static void RecognitionResultDestroy(VXIrecRecognitionResult **Result)
{
    if (Result == NULL || *Result == NULL)
        return;

    VXIrecRecognitionResult * result = *Result;

    if (result->waveform != NULL)
    {
        VXIContentDestroy(&(result->waveform));
        result->waveform = NULL;
    }

    if (result->results != NULL)
    {
        VXIVectorDestroy(&(result->results));
        result->results = NULL;
    }

    delete result;
    result = NULL;
    *Result = NULL;
}

static void ContentDestroy(VXIbyte **content, void *userData)
{
    if ((content) && (*content))
    {
        delete [] *content;
        *content = NULL;
    }
}

static void RecordResultDestroy(VXIrecRecordResult **Result)
{
    if (Result == NULL || *Result == NULL)
        return;

    VXIrecRecordResult * result = *Result;

    if (result->waveform != NULL)
        VXIContentDestroy(&result->waveform);

    if (result->timestamp)
        delete[] result->timestamp;
    result->timestamp = NULL;
    delete result;
    result = NULL;
    *Result = NULL;
}

static VXIrecResult waitForResponse(VXIrecInterface *pThis,
        OSBPlayerListener* pListener, int timeoutMilliSecs, int& dtmfInfo,
        int& eventInfo, int isFirstKey)
{
    VXIrecResult ret = VXIrec_RESULT_SUCCESS;
    OSBrecData* tp = GetRecData(pThis);
    if (tp == NULL)
        return VXIrec_RESULT_INVALID_ARGUMENT;

    OSBrecImpl* impl = reinterpret_cast<OSBrecImpl*>(pThis);
    tp->Diag(DIAG_TAG_REC, NULL,L"Rec waitForResponse" );

    OsQueuedEvent* promptEvent = NULL;

    if (pListener)
    {
        promptEvent = new OsQueuedEvent(impl->mIncomingQ, 0);
        if (promptEvent)
        pListener->addListeningEvent(promptEvent);
        else
        {
            eventInfo = -1;
            return VXIrec_RESULT_OUT_OF_MEMORY;
        }
    }
    else
    {
        return VXIrec_RESULT_OUT_OF_MEMORY; // ???
    }

    if (timeoutMilliSecs <= 0) timeoutMilliSecs = 1000*CP_MAX_EVENT_WAIT_SECONDS;
    OsTime maxEventTime(900, 0); // 15 minutes
    if (!isFirstKey && pListener->isState(PlayerStopped))
    {
        tp->Diag(DIAG_TAG_REC, NULL, L"Rec waitForResponse setting timeout %d (%d)", timeoutMilliSecs, isFirstKey);
        maxEventTime = OsTime(0, timeoutMilliSecs*1000); // prompt stopped, start timer
    }

    // Wait until key press or time out - timeout count starts from last prompt stops.
    OsEventMsg* pMsg;
    OsStatus waitRet = OS_BUSY;

    do
    {
        waitRet = impl->mIncomingQ.receive((OsMsg*&) pMsg, maxEventTime); // wait for a message

        if (waitRet == OS_SUCCESS)
        {
            // got event
        intptr_t temp_info = dtmfInfo;
        pMsg->getEventData(temp_info);
        dtmfInfo = temp_info;
            eventInfo = ((dtmfInfo & 0xffff0000) >> 16);
            if (!pMsg->getSentFromISR())
            pMsg->releaseMsg(); // free the message

            if (dtmfInfo & 0x00000001)
            {
                // event from the prompt listener
                if ((impl->live == 1) && eventInfo == PlayerStopped)
                {
                    tp->Diag(DIAG_TAG_REC, NULL, L"Rec waitForResponse setting timeout-player stopped  %d (%d)", timeoutMilliSecs, isFirstKey);
                    maxEventTime = OsTime(0, timeoutMilliSecs*1000); // prompt just stopped, start timer
                }
                else if ((impl->live == 1) && eventInfo != PlayerStopped)
                {
                    tp->Diag(DIAG_TAG_REC, NULL, L"Rec waitForResponse player state  %d (%d)", (dtmfInfo & 0x0000fffe), eventInfo);
                }
                else if (impl->live != 1)
                {
                    eventInfo = -1;
                    break;
                }
            }
            else
            {
                // dtmf event, key pressed
                tp->Diag(DIAG_TAG_REC, NULL, L"Rec waitForResponse dtmf event, key pressed=%d, isFirstKey=%d", eventInfo, isFirstKey);
                break;
            }
        }
        else
        {
            ret = VXIrec_RESULT_FETCH_TIMEOUT;
            tp->Diag(DIAG_TAG_REC, NULL, L"Rec waitForResponse timed out (%d)", isFirstKey);
            break; // timed out
        }
    }
    while (TRUE);

    if (pListener)
    {
        if (promptEvent)
        {
            pListener->removeListeningEvent(promptEvent);
            delete promptEvent;
            promptEvent = NULL;
        }
    }

    return ret;
}

static VXIrecResult waitForDtmfTone(VXIrecInterface *pThis,
        OSBPlayerListener* pListener, int timeoutMilliSecs,
        int interDigitMilliSecs, int& dtmfInfo, char& dtmfName, char* strBuf,
        int bufLen, int isFirstKey)
{
    VXIrecResult ret = VXIrec_RESULT_SUCCESS;
    OSBrecData* tp = GetRecData(pThis);
    if (tp == NULL)
        return VXIrec_RESULT_INVALID_ARGUMENT;

    tp->Diag(DIAG_TAG_REC, NULL,L"Rec waitForDtmfTone" );

    int eventInfo = 0;
    ret = waitForResponse(pThis, pListener, timeoutMilliSecs, dtmfInfo, eventInfo, isFirstKey);

    if (ret == VXIrec_RESULT_SUCCESS)
    {
        if (eventInfo == -1)
        {
            tp->Diag(DIAG_TAG_REC, NULL, L"Rec waitForDtmfTone call DISCONNECTED");
            ret = VXIrec_RESULT_DISCONNECT; // call disconnected
        }
        else
        {
            dtmfName = DTMF_CHAR[eventInfo];
        }
    }
    else
    {
        tp->Diag(DIAG_TAG_REC, NULL, L"Rec waitForDtmfTone TIMED OUT");

        ret = VXIrec_RESULT_FETCH_TIMEOUT;
    }

    return ret;
}

static VXIrecResult OSBrecRecognize(VXIrecInterface *pThis,
        const VXIMap *properties, VXIrecRecognitionResult **recogResult)
{
    OSBrecData* tp = GetRecData(pThis);
    if (tp == NULL)
        return VXIrec_RESULT_INVALID_ARGUMENT;

    OSBrecImpl* impl = reinterpret_cast<OSBrecImpl*>(pThis);
    tp->Diag(DIAG_TAG_REC, NULL,L"Rec recognizing" );

    if (impl->hungup)
    {
        tp->Diag(DIAG_TAG_REC, NULL, L"rec Recognize live=%d hungup=%d", impl->live, impl->hungup);
        return VXIrec_RESULT_DISCONNECT;
    }

    int dtmfInfo;
    char dtmfName;
    char strInfo[128];
    int len = 0;
    VXIchar* digit = new VXIchar[10];
    VXIchar* best = new VXIchar[VXI_RECOGNIZE_MAX_INPUT_LENGTH];

    memset(digit, 0, 10*sizeof(VXIchar));
    memset(best, 0, VXI_RECOGNIZE_MAX_INPUT_LENGTH*sizeof(VXIchar));

    VXIMap* res = NULL;
    const VXIchar *termChar = NULL;
    VXIint completeTime = VXI_TIMEOUT_DEFAULT;
    VXIint incompleteTime = VXI_TIMEOUT_DEFAULT;
    VXIint interdigitTime = VXI_TIMEOUT_DEFAULT;
    VXIint termTime = VXI_TIMEOUT_DEFAULT;
    VXIint timeout = VXI_TIMEOUT_DEFAULT;
    VXIint dtmfTime = VXI_TIMEOUT_DEFAULT;
    VXIflt32 confidenceLevel;
    VXIflt32 sensitivity;
    VXIflt32 speedvsacc;

    if (properties != NULL)
    {
        const VXIValue* v;
        v = VXIMapGetProperty(properties, REC_TIMEOUT_COMPLETE);
        if (v) completeTime = VXIIntegerValue(reinterpret_cast<const VXIInteger *>(v));
        v = VXIMapGetProperty(properties, REC_TIMEOUT_INCOMPLETE);
        if (v) incompleteTime = VXIIntegerValue(reinterpret_cast<const VXIInteger *>(v));
        v = VXIMapGetProperty(properties, REC_DTMF_TIMEOUT_INTERDIGIT);
        if (v) interdigitTime = VXIIntegerValue(reinterpret_cast<const VXIInteger *>(v));
        v = VXIMapGetProperty(properties, REC_DTMF_TIMEOUT_TERMINATOR);
        if (v) termTime = VXIIntegerValue(reinterpret_cast<const VXIInteger *>(v));

        v = VXIMapGetProperty(properties, REC_TIMEOUT);
        if (v) timeout = VXIIntegerValue(reinterpret_cast<const VXIInteger *>(v));
        v = VXIMapGetProperty(properties, REC_DTMF_TIMEOUT);
        if (v) dtmfTime = VXIIntegerValue(reinterpret_cast<const VXIInteger *>(v));

        v = VXIMapGetProperty(properties, REC_CONFIDENCE_LEVEL);
        if (v) confidenceLevel = VXIFloatValue(reinterpret_cast<const VXIFloat*>(v));
        v = VXIMapGetProperty(properties, REC_SENSITIVITY);
        if (v) sensitivity = VXIFloatValue(reinterpret_cast<const VXIFloat*>(v));
        v = VXIMapGetProperty(properties, REC_SPEED_VS_ACCURACY);
        if (v) speedvsacc = VXIFloatValue(reinterpret_cast<const VXIFloat*>(v));

        v = VXIMapGetProperty(properties, REC_DTMF_TERMINATOR_CHAR);
        if (v) termChar = (VXIchar*) VXIStringCStr(reinterpret_cast<const VXIString*>(v));
    }

    VXIrecRecognitionResult * result = new VXIrecRecognitionResult();
    if (result == NULL)
    {
        delete[] digit;
        digit = NULL;
        delete[] best;
        best = NULL;
        return VXIrec_RESULT_OUT_OF_MEMORY;
    }

    result->results = VXIVectorCreate();
    if (result->results == NULL)
    {
        delete[] digit; digit = NULL;
        delete[] best; best = NULL;
        delete result; result = NULL;
        return VXIrec_RESULT_OUT_OF_MEMORY;
    }

    *recogResult = result;

    result->Destroy = RecognitionResultDestroy;
    result->status = REC_STATUS_SUCCESS;
    result->mode = REC_INPUT_MODE_DTMF;
    result->waveform = NULL;

    res = VXIMapCreate();

    OSBPlayerListener* pListener = VXIGetPlayerListener((const char*)impl->callId, (unsigned)impl->channel);
    tp->Diag(DIAG_TAG_REC, NULL,
    L"Recognition  VXIGetPlayerListener got pListener=%p, channel=%u", pListener, impl->channel);

    VXIrecResult ret = VXIrec_RESULT_DISCONNECT;
    if ((impl->live == 1) && ((ret = waitForDtmfTone(pThis,
                            pListener,
                            timeout, // timeout ms
                            interdigitTime, // interdigittimeout ms
                            dtmfInfo,
                            dtmfName,
                            strInfo,
                            127,
                            1)) == VXIrec_RESULT_SUCCESS))
    {
        if (impl->prompt)
        {
            impl->prompt->Stop(impl->prompt);
            OsTask::delay(300);
        }

        mbstowcs(digit, &dtmfName, 1);
        memcpy(best, digit, 10*sizeof(VXIchar));
        len++;

        const VXIchar* val = best;

        do
        {

            tp->Diag(DIAG_TAG_RECOGNITION_RESULT, NULL, L"best = %s", best);
            val = best;
            int maxlen = 0;
            OSBrecGrammar * gp = tp->FindGrammarForPhrase(best, &val, &maxlen);
            memset(digit, 0, 10*sizeof(VXIchar));
            if (gp == NULL)
            {
                // no match yet
                if (best[0] == '#')
                {
                    gp = tp->FindGrammarForPhraseAfterPound(best, &val, &maxlen);
                    if (gp)
                    {
                        result->status = REC_STATUS_DTMFSUCCESS;
                        VXIMapSetProperty(res, REC_GRAMMAR, (VXIValue*)VXIPtrCreate(gp));
                    }
                    else if (maxlen != -1 && maxlen <= len)
                    {
                        result->status = REC_STATUS_FAILURE;
                        VXIVectorAddElement(result->results, (VXIValue*) res);
                    }
                }
                else if (maxlen != -1 && maxlen <= len)
                {
                    result->status = REC_STATUS_FAILURE;
                    VXIVectorAddElement(result->results, (VXIValue*) res);
                }
                else
                {
                    if ((impl->live == 1) && ((ret = waitForDtmfTone(pThis,
                                            pListener,
                                            interdigitTime, // timeout ms
                                            interdigitTime, // interdigittimeout ms
                                            dtmfInfo,
                                            dtmfName,
                                            strInfo,
                                            127,
                                            0)) == VXIrec_RESULT_SUCCESS))
                    {
                        mbstowcs(digit, &dtmfName, 1);
                        if (termChar && termChar[0] == digit[0])
                        {
                            val = best;
                            gp = tp->FindGrammarForPhraseAfterPound(best, &val, &maxlen);
                            result->status = REC_STATUS_DTMFSUCCESS;
                            VXIMapSetProperty(res, REC_GRAMMAR, (VXIValue*)VXIPtrCreate(gp));
                        }
                        else
                        {
                            memcpy((VXIchar*)(best + len), digit, sizeof(digit));
                            len++;
                            result->status = REC_STATUS_CONTINUE;
                        }
                    }
                    else
                    {
                        val = best;
                        gp = tp->FindGrammarForPhrase(best, &val, &maxlen);
                        if (!gp)
                            gp = tp->FindGrammarForPhraseAfterPound(best, &val, &maxlen);
                        if (gp)
                        {
                            result->status = REC_STATUS_DTMFSUCCESS;
                            VXIMapSetProperty(res, REC_GRAMMAR, (VXIValue*)VXIPtrCreate(gp));
                        }
                        else if (ret == VXIrec_RESULT_DISCONNECT)
                            result->status = REC_STATUS_DISCONNECT;
                        else if (best[0] == 0)
                            result->status = REC_STATUS_TIMEOUT;
                        else
                            result->status = REC_STATUS_FAILURE;
                    }
                }
            }
            else
            {
                result->status = REC_STATUS_SUCCESS;
                VXIMapSetProperty(res, REC_GRAMMAR, (VXIValue*)VXIPtrCreate(gp));
            }

        } while ((impl->live == 1) &&
                result->status != REC_STATUS_TIMEOUT &&
                result->status != REC_STATUS_SUCCESS &&
                result->status != REC_STATUS_FAILURE &&
                result->status != REC_STATUS_DISCONNECT &&
                result->status != REC_STATUS_DTMFSUCCESS);

        if (result->status != REC_STATUS_FAILURE)
        {
            // Create answer vectors.
            VXIVectorHolder raws, keys, vals, confs;
            if (raws.GetValue() == NULL || keys.GetValue() == NULL ||
                    vals.GetValue() == NULL || confs.GetValue() == NULL)
                return VXIrec_RESULT_OUT_OF_MEMORY;

            // Q: Is the recognition returning a single answer?
            if (val[0] != '?')
            {
                // A: Yes, we'll call it 'best_answer'.
                const VXIchar * const KEY = L"best_answer";
                VXIVectorAddElement(raws.GetValue(), (VXIValue*)VXIStringCreate(best));
                VXIVectorAddElement(keys.GetValue(), (VXIValue*)VXIStringCreate(KEY));
                VXIVectorAddElement(vals.GetValue(), (VXIValue*)VXIStringCreate(val));
                VXIVectorAddElement(confs.GetValue(), (VXIValue*)VXIFloatCreate(0.95F));
            }
            else
            {
                // A: No, we must extract the keys and values.
                vxistring::size_type pos = 1; // skip leading '?'
                vxistring data(val);

                do
                {
                    vxistring::size_type itembreak = data.find(';', pos);
                    vxistring::size_type seperator = data.find('=', pos);
                    if (itembreak == vxistring::npos) itembreak = data.length();
                    if (seperator == vxistring::npos) seperator = data.length();

                    if (pos == itembreak || pos == seperator || itembreak <= seperator)
                    {
                        result->status = REC_STATUS_FAILURE;
                        break;
                    }

                    VXIString * key = VXIStringCreateN(data.c_str() + pos,
                    seperator - pos);
                    VXIString * value = VXIStringCreateN(data.c_str() + seperator + 1,
                    itembreak - seperator - 1);

                    VXIVectorAddElement(keys.GetValue(), (VXIValue*) key);
                    VXIVectorAddElement(vals.GetValue(), (VXIValue*) value);
                    VXIVectorAddElement(raws.GetValue(), (VXIValue*)VXIStringCreate(best));
                    VXIVectorAddElement(confs.GetValue(),(VXIValue*)VXIFloatCreate(0.95F));

                    pos = itembreak + 1;
                }while (pos < data.length());
            }

            // Add vectors to result map.
            VXIMapSetProperty(res, REC_RAW, (VXIValue *) raws.Release());
            VXIMapSetProperty(res, REC_KEYS, (VXIValue *) keys.Release());
            VXIMapSetProperty(res, REC_VALUES, (VXIValue *) vals.Release());
            VXIMapSetProperty(res, REC_CONF, (VXIValue *) confs.Release());

            VXIVectorAddElement(result->results, (VXIValue*) res);
        }
    }
    else
    {
        if (impl->prompt)
        {
            impl->prompt->Stop(impl->prompt);
            OsTask::delay(300);
        }
        result->status = REC_STATUS_TIMEOUT;
        VXIVectorAddElement(result->results, (VXIValue*) res);
    }

    int ref = VXIReleasePlayerListener((const char*)impl->callId, impl->channel);
    tp->Diag(DIAG_TAG_REC, NULL,
    L"Recognition VXIReleasePlayerListener %p, ref=%d", pListener, ref);
    pListener = NULL;

    if (impl->live == 0 || ret == VXIrec_RESULT_DISCONNECT)
    {
        //throw VXIException::InterpreterEvent(EV_TELEPHONE_HANGUP);
        result->status = REC_STATUS_DISCONNECT;
    }

    if (digit)
    {
        delete[] digit;
        digit = 0;
    }
    tp->Diag(DIAG_TAG_RECOGNITION_RESULT, NULL, L"Final recognition result best = %s", best);
    if (best)
    {
        delete[] best;
        best = 0;
    }

    return VXIrec_RESULT_SUCCESS;
}

static int getRecordedData(OsProtectedEvent* recordEvent, int& dtmfterm,
        double& duration)
{
    int ret = MprRecorder::RECORDING;
    void* info = 0;
    recordEvent->getUserData(info);
    if (info)
    {
        MprRecorderStats rs = *((MprRecorderStats *)info);
        duration = rs.mDuration;
        if (rs.mDtmfTerm >= 0)
            dtmfterm = DTMF_CHAR[rs.mDtmfTerm];
        else if (rs.mFinalStatus == MprRecorder::RECORD_FINISHED)
            dtmfterm = 0;
        else
            dtmfterm = -1;

        ret = rs.mFinalStatus;
    }

    return ret;
}

static VXIrecResult OSBrecRecord(VXIrecInterface *pThis, const VXIMap *props,
        VXIrecRecordResult **recordResult)
{
    OSBrecData* tp = GetRecData(pThis);
    if (tp == NULL)
        return VXIrec_RESULT_INVALID_ARGUMENT;
    if (recordResult == NULL)
        return VXIrec_RESULT_INVALID_ARGUMENT;

    OSBrecImpl* impl = reinterpret_cast<OSBrecImpl*>(pThis);

    if (impl->hungup)
    {
        tp->Diag(DIAG_TAG_REC, NULL,L"rec Record live=%d hungup=%d" , impl->live, impl->hungup);
        return VXIrec_RESULT_DISCONNECT;
    }

    if (props != NULL)
    {
        VXIint finalsilence = 5;
        VXIint maxtime = 300000;
        VXIint timeout = 10;
        VXIint dtmfterm = 1;
        vxistring beepURL;

        const VXIValue* v;
        v = VXIMapGetProperty(props, REC_MAX_RECORDING_TIME);
        if (v) maxtime = VXIIntegerValue(reinterpret_cast<const VXIInteger *>(v));
        v = VXIMapGetProperty(props, REC_TIMEOUT_COMPLETE);
        if (v) finalsilence = VXIIntegerValue(reinterpret_cast<const VXIInteger *>(v));
        v = VXIMapGetProperty(props, REC_TERMINATED_ON_DTMF);
        if (v) dtmfterm = VXIIntegerValue(reinterpret_cast<const VXIInteger *>(v));
        v = VXIMapGetProperty(props, REC_TIMEOUT);
        if (v) timeout = VXIIntegerValue(reinterpret_cast<const VXIInteger *>(v));

        v = VXIMapGetProperty(props, REC_RECORD_BEEP_URL);
        if (v) beepURL = VXIStringCStr((const VXIString *) v);

        time_t cur;
        char fileName[128];
        double duration;

        time(&cur);
        cur += rand();
        sprintf(fileName, "temp_recording_file%ld%s", cur, ".wav");
        tp->Diag(DIAG_TAG_RECORDING, NULL, L"Recording AUDIO: %s maxtime %dms", (char*)fileName, maxtime);

        MprRecorderStats rs;
        memset(&rs, 0, sizeof(MprRecorderStats));
        rs.mFinalStatus = MprRecorder::RECORDING;
        rs.mDuration = 0;
        rs.mDtmfTerm = -1;
        rs.mTotalBytesWritten = 0;

        OsProtectEventMgr* eventMgr = OsProtectEventMgr::getEventMgr();
        OsProtectedEvent* recordEvent = eventMgr->alloc();
        recordEvent->setUserData(&rs);

        int eventInfo = 0;
        int dtmfInfo;
        char strInfo[128];

        OSBPlayerListener* pListener = VXIGetPlayerListener((const char*)impl->callId, (unsigned)impl->channel);
        tp->Diag(DIAG_TAG_REC, NULL,
        L"Recording VXIGetPlayerListener got pListener=%p, channel=%u", pListener, impl->channel);

        if ((impl->live == 1) && VXIrec_RESULT_SUCCESS == waitForDtmfTone(pThis, pListener, 500, 500, dtmfInfo, (char&)eventInfo, strInfo, 127, 1))
        {
            if (impl->prompt)
            {
                if ( impl->prompt)
                {
                    impl->prompt->Stop(impl->prompt);
                    OsTask::delay(300);
                }
                if (!beepURL.empty())
                {
                    impl->prompt->Queue(impl->prompt, NULL, beepURL.c_str(), NULL, props, PromptManager::ENABLED);
                    impl->prompt->Play(impl->prompt);
                    waitForResponse(pThis, pListener, 500, dtmfInfo, eventInfo, 1);
                }
            }
        }

        if (impl->dtmfEvent)
        {
            impl->pCallMgr->disableDtmfEvent((const char*)impl->callId, (void*)impl->dtmfEvent);
            // All key presses during recording would have been queued, confusing
            // recognition following it, so disable the dtmf listening during recording.
            // All key presses during recording will be ignored. 
            tp->Diag(DIAG_TAG_REC, NULL,
            L"Recording AUDIO: disable dtmf listening.");
        }

        tp->Diag(DIAG_TAG_REC, NULL, L"Rec recording");
        if ( impl->live == 1 && OS_SUCCESS == impl->pCallMgr->ezRecord((const char*)impl->callId,
                maxtime,
                finalsilence/1000, /* in seconds */
                duration,
                (const char*)fileName,
                dtmfterm,
                recordEvent))
        {
            impl->recording = 1;
            int timeoutSeconds = CP_MAX_EVENT_WAIT_SECONDS;
            int maxRecordingSeconds = maxtime/1000;
            if (maxRecordingSeconds > CP_MAX_EVENT_WAIT_SECONDS)
            timeoutSeconds = maxRecordingSeconds + 10;
            OsTime maxEventTime(timeoutSeconds, 0);

            OsStatus ret;
            // Wait until the recording ends
            int state = MprRecorder::RECORD_IDLE;
            while((ret = recordEvent->wait(0, maxEventTime)) == OS_SUCCESS)
            {
                state = getRecordedData(recordEvent, dtmfterm, duration);
                if (MprRecorder::RECORDING == state)
                {
                    ///////////////////////////////////   debug info   /////////////////////////////////
                    tp->Diag(DIAG_TAG_RECORDING, NULL, L"Recording: state %d event 0x%08x, to reset.",
                        state, recordEvent);
                    ///////////////////////////////////   debug info   /////////////////////////////////

                    recordEvent->reset();

                    ///////////////////////////////////   debug info   /////////////////////////////////
                    tp->Diag(DIAG_TAG_RECORDING, NULL, L"Recording: state %d event 0x%08x, reset.",
                        state, recordEvent);
                    ///////////////////////////////////   debug info   /////////////////////////////////
                }
                else
                {
                    if ((impl->live == 0) && impl->recording)
                    {
                        OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                        "osbrec::OSBrecRecord calling stopRecording() for %s",
                        (const char*)impl->callId);
                        impl->pCallMgr->stopRecording((const char*)impl->callId);
                        impl->recording = 0;

                        ///////////////////////////////////   debug info   /////////////////////////////////
                        tp->Diag(DIAG_TAG_RECORDING, NULL, L"Recording: stop recording live=0, recording state %d event 0x%08x ", state, recordEvent);
                        ///////////////////////////////////   debug info   /////////////////////////////////
                    }
                    ///////////////////////////////////   debug info   /////////////////////////////////
                    tp->Diag(DIAG_TAG_RECORDING, NULL, L"Recording: recording stopped, releasing event 0x%08x ", recordEvent);
                    ///////////////////////////////////   debug info   /////////////////////////////////

                    recordEvent->setUserData(0);
                    eventMgr->release(recordEvent);

                    ///////////////////////////////////   debug info   /////////////////////////////////
                    tp->Diag(DIAG_TAG_RECORDING, NULL, L"Recording: recording stopped, state=%d released event 0x%08x ", state, recordEvent);
                    ///////////////////////////////////   debug info   /////////////////////////////////

                    break;
                }
            }

            assert(ret == OS_SUCCESS);

            if (ret != OS_SUCCESS)
            {
                ///////////////////////////////////   debug info   /////////////////////////////////
                tp->Diag(DIAG_TAG_RECORDING, NULL, L"Recording: event 0x%08x timed out ret=%d", recordEvent, ret);
                ///////////////////////////////////   debug info   /////////////////////////////////

                if (impl->recording)
                {
                    OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                    "osbrec::OSBrecRecord calling stopRecording() for %s",
                    (const char*)impl->callId);
                    impl->pCallMgr->stopRecording((const char*)impl->callId);
                    impl->recording = 0;
                }
                int state = getRecordedData(recordEvent, dtmfterm, duration);
                // If the event has already been signalled, clean up
                if(OS_ALREADY_SIGNALED == recordEvent->signal(0))
                {
                    ///////////////////////////////////   debug info   /////////////////////////////////
                    tp->Diag(DIAG_TAG_RECORDING, NULL, L"Recording: timed out state %d, event 0x%08x already signaled, releasing", state, recordEvent);
                    ///////////////////////////////////   debug info   /////////////////////////////////

                    recordEvent->setUserData(0);
                    eventMgr->release(recordEvent);

                    ///////////////////////////////////   debug info   /////////////////////////////////
                    tp->Diag(DIAG_TAG_RECORDING, NULL, L"Recording: timed out, event 0x%08x already signaled, released", recordEvent);
                    ///////////////////////////////////   debug info   /////////////////////////////////
                }
            }

            OsFile file(fileName);
            if (OS_SUCCESS == file.open(OsFile::READ_ONLY))
            {
                VXIrecRecordResult * result = new VXIrecRecordResult();
                if (result == NULL) return VXIrec_RESULT_OUT_OF_MEMORY;
                *recordResult = result;

                VXIContent *content = NULL;

                if ((MprRecorder::RECORD_FINISHED == state) && (duration < finalsilence + 1000))
                {
                    tp->Diag(DIAG_TAG_REC, NULL, L"Recording AUDIO ignored: duration %dms is too short, finalsilence %d", (int)duration, (int)finalsilence);
                    result->Destroy = RecordResultDestroy;
                    result->status = REC_STATUS_TIMEOUT;
                }
                else
                {
                    size_t len = (size_t)rs.mTotalBytesWritten + 44; // header length = 44
                    VXIbyte *data = new VXIbyte [len + 1];
                    size_t numread = 0;

                    if ((OS_SUCCESS == file.read(data, len, numread)) && (numread > 0))
                    {
                        if (data)
                        {
                            content = VXIContentCreate(VXI_MIME_WAV, data, numread,
                            ContentDestroy, NULL);
                        }

                        UtlString strTime;
                        OsDateTime::getLocalTimeString(strTime);
                        const char* tmstr = strTime.data();
                        size_t len = strlen(tmstr);
                        VXIchar *tm = new VXIchar[len + 1];
                        for (size_t i = 0; i <= len; i++)
                           tm[i] = (VXIchar)tmstr[i];
                        tm[len] = 0;
                        // Return the content
                        if (! content)
                        {
                            delete [] data;
                            data = NULL;
                        }
                        if (duration <= 0 )
                        {
                            tp->Diag(DIAG_TAG_REC, NULL, L"Recording AUDIO: invalid duration %d set to 1", duration);
                            duration = 1;
                        }
                        result->Destroy = RecordResultDestroy;
                        result->status = REC_STATUS_SUCCESS;
                        result->duration = (int)duration;
                        result->size = numread;
                        result->waveform = content;
                        result->timestamp = tm;
                        // If dtmfterm == -1, result->termchar will be
                        // corrected to 0 below.
                        result->termchar = dtmfterm;

                        if (dtmfterm < 0) // user hang up

                        {
                            result->status = REC_STATUS_DISCONNECT;
                            // dtmfterm can be -1, indicating recording ended due
                            // to hang-up (and perhaps other causes), but
                            // result->termchar must be either 0 or the terminating
                            // DTMF character.
                            result->termchar = 0;
                        }
                    }

                    if (dtmfterm < 0) // user hang up

                    {
                        impl->live = 0;
                        tp->Diag(DIAG_TAG_REC, NULL,
                        L"Recording AUDIO: duration %dms bytes %d, termKey=%d - caller hang up.",
                        (int)duration, (int)numread, dtmfterm);
                    }
                    else
                    {
                        tp->Diag(DIAG_TAG_REC, NULL,
                        L"Recording AUDIO: duration %dms bytes %d, termKey=%c",
                        (int)duration, (int)numread, dtmfterm);
                    }
                }

                file.close();
                file.remove();
            }
            else
            tp->Diag(DIAG_TAG_REC, NULL, L"Recording for %s: cannot open file: %s ", impl->callId, (char*)fileName);
        }

        // All key presses during recording have been ignored. Now re-enable dtmf listening. 
        VXIint interdigitTime = VXI_TIMEOUT_DEFAULT;
        impl->pCallMgr->enableDtmfEvent((const char*)impl->callId,
        interdigitTime/1000,
        impl->dtmfEvent,
        1);
        tp->Diag(DIAG_TAG_REC, NULL, L"Recording AUDIO: re-enable dtmf listening.");

        int ref = VXIReleasePlayerListener((const char*)impl->callId, impl->channel);
        tp->Diag(DIAG_TAG_REC, NULL,
        L"Recording VXIReleasePlayerListener %p, ref=%d", pListener, ref);
        pListener = NULL;

    }
    return VXIrec_RESULT_SUCCESS;
}

/*******************************************************
 * Init and factory routines
 *******************************************************/

static inline OSBrecImpl * ToOSBrecImpl(VXIrecInterface * i)
{
    return reinterpret_cast<OSBrecImpl *>(i);
}

// Global init - Don't need to do much here
//
OSBREC_API VXIrecResult OSBrecInit(VXIlogInterface *log,
        VXIunsigned diagLogBase)
{
    if (!log) return VXIrec_RESULT_INVALID_ARGUMENT;

    gblDiagLogBase = diagLogBase;
    return VXIrec_RESULT_SUCCESS;
}

// Global shutdown
//
OSBREC_API VXIrecResult OSBrecShutDown(VXIlogInterface *log)
{
    if (!log) return VXIrec_RESULT_INVALID_ARGUMENT;

    return VXIrec_RESULT_SUCCESS;
}

OSBREC_API VXIrecResult OSBrecExiting (VXIlogInterface *log,
        VXIrecInterface **rec)
{
    if (! log) return VXIrec_RESULT_INVALID_ARGUMENT;

    if (rec == NULL || *rec == NULL) return VXIrec_RESULT_INVALID_ARGUMENT;
    OSBrecImpl* impl = reinterpret_cast<OSBrecImpl*>(*rec);

    if (!impl) return VXIrec_RESULT_INVALID_ARGUMENT;

    impl->hungup = 1;

    OSBrecData * tp = impl->recData;
    tp->Diag(DIAG_TAG_REC, NULL, L"Rec Exiting");

    if ((impl->live == 1) && impl->pExitGuard)
    {
        impl->pExitGuard->acquire();
        if (impl->live == 1)
        {
            impl->live = 0;

            if ((impl->recording == 1) && impl->pCallMgr && impl->callId)
            {
                OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                        "osbrec::OSBrecExiting calling stopRecording() for %s",
                        (const char*)impl->callId);
                impl->pCallMgr->stopRecording((const char*)impl->callId);
                impl->recording = 0;
                tp->Diag(DIAG_TAG_REC, NULL, L"stopRecording called");
            }
        }
        impl->pExitGuard->release();
    }

    return VXIrec_RESULT_SUCCESS;
}

// Create an VXIrecInterface object and return.
//
OSBREC_API VXIrecResult OSBrecCreateResource(VXIunsigned channelNum,
        VXIlogInterface *log,
        VXIrecInterface **rec)
{
    if (!log) return VXIrec_RESULT_INVALID_ARGUMENT;

    OSBrecImpl* pp = new OSBrecImpl();
    if (pp == NULL) return VXIrec_RESULT_OUT_OF_MEMORY;

    OSBrecData* tp = new OSBrecData(gblDiagLogBase, log);
    if (tp == NULL) return VXIrec_RESULT_OUT_OF_MEMORY;

    pp->log = log;
    pp->channel = channelNum;
    pp->recData = tp;
    pp->pExitGuard = NULL;
    pp->intf.GetVersion = OSBrecGetVersion;
    pp->intf.GetImplementationName = OSBrecGetImplementationName;
    pp->intf.BeginSession = OSBrecBeginSession;
    pp->intf.EndSession = OSBrecEndSession;
    pp->intf.LoadGrammarURI = OSBrecLoadGrammarFromURI;
    pp->intf.LoadGrammarString = OSBrecLoadGrammarFromString;
    pp->intf.ActivateGrammar = OSBrecActivateGrammar;
    pp->intf.DeactivateGrammar = OSBrecDeactivateGrammar;
    pp->intf.FreeGrammar = OSBrecFreeGrammar;
    pp->intf.Recognize = OSBrecRecognize;
    pp->intf.Record = OSBrecRecord;

    *rec = &pp->intf;
    return VXIrec_RESULT_SUCCESS;
}

OSBREC_API VXIrecResult OSBrecAddResource (VXIrecInterface **rec,
        CallManager *pCallMgr,
        VXIpromptInterface *prompt)
{
    if (rec == NULL || *rec == NULL) return VXIrec_RESULT_INVALID_ARGUMENT;
    OSBrecImpl* impl = reinterpret_cast<OSBrecImpl*>(*rec);

    impl->pCallMgr = pCallMgr;
    impl->prompt = prompt;
    impl->live = -1;
    impl->hungup = 0;

    return VXIrec_RESULT_SUCCESS;
}

// Free OSBrec structure allocated in OSBrecCreateResource.
//
OSBREC_API VXIrecResult OSBrecDestroyResource(VXIrecInterface **rec)
{
    if (rec == NULL || *rec == NULL) return VXIrec_RESULT_INVALID_ARGUMENT;
    OSBrecImpl* impl = reinterpret_cast<OSBrecImpl*>(*rec);

    if (!impl) return VXIrec_RESULT_INVALID_ARGUMENT;

    if (impl->pExitGuard)
    {
        impl->pExitGuard->acquire();
        impl->live = 0;
        impl->mIncomingQ.flush(); // dispose of any messages in the request queue

        OSBrecData * tp = impl->recData;
        tp->Diag(DIAG_TAG_REC, NULL, L"Rec DestroyResource - mIncomingQ flushed");

        impl->pExitGuard->release();
        delete impl->pExitGuard;
        impl->pExitGuard = NULL;

        if (tp != NULL)
        {   
            delete tp; 
            tp = NULL;
        }
        delete impl;
        impl = NULL;
        *rec = NULL;
    }
    return VXIrec_RESULT_SUCCESS;
}

