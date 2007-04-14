// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#define OSBTEL_EXPORTS
#include "OSBtel.h"

#include <cstdio>
#include <string>
#include <cstring>
#define VXIstrcmp wcscmp

#include "VXIvalue.h"
#include "VXI/VXML.h"
#include "VXI/CommonExceptions.hpp"
#include "os/OsDefs.h"
#include "os/OsDateTime.h"

typedef std::basic_string<VXIchar> vxistring;

// ------*---------*---------*---------*---------*---------*---------*---------


// Global for the base diagnostic tag ID
//
static VXIunsigned gblDiagLogBase = 0;

// Constants for diagnostic logging tags
//
static const VXIunsigned DIAG_TAG_SIGNALING = 0;


// OSBtel implementation of the VXItel interface
//
extern "C" {
   struct OSBtelImpl {
      // Base interface, must be first
      OSBtelInterface osbIntf;

      // Log interface for this resource
      VXIlogInterface *log;

      IvrTelListener     *pListener;
      CallManager          *pCallMgr;
      OsBSem    *pExitGuard;
      VXIchar              *callId;
      VXIchar              *from;
      VXIchar              *to;
      int          transferred;
      int          waitingForTransferState;
      int          live;
      VXIunsigned  channel;
   };
}


// A few conversion functions...

static inline OSBtelImpl * ToOSBtelImpl(VXItelInterface * i)
{ return reinterpret_cast<OSBtelImpl *>(i); }

static inline OSBtelImpl * ToOSBtelImpl(OSBtelInterface * i)
{ return reinterpret_cast<OSBtelImpl *>(i); }


/*******************************************************
 *
 * Utility functions
 *
 *******************************************************/ 

/**
 * Log an error
 */
/// static VXIlogResult Error(OSBtelImpl *impl, VXIunsigned errorID,
///       const VXIchar *format, ...)
/// {
///    VXIlogResult rc;
///    va_list arguments;
/// 
///    if ((! impl) || (! impl->log))
///       return VXIlog_RESULT_NON_FATAL_ERROR;
/// 
///    if (format) {
///       va_start(arguments, format);
///       rc = (*impl->log->VError)(impl->log, COMPANY_DOMAIN L".OSBtel", 
///         errorID, format, arguments);
///       va_end(arguments);
///    } else {
///       rc = (*impl->log->Error)(impl->log, COMPANY_DOMAIN L".OSBtel",
///         errorID, NULL);
///    }
/// 
///    return rc;
/// }


/**
 * Log a diagnostic message
 */
static VXIlogResult Diag(OSBtelImpl *impl, VXIunsigned tag, 
      const VXIchar *subtag, const VXIchar *format, ...)
{
   VXIlogResult rc;
   va_list arguments;

   if ((! impl) || (! impl->log))
      return VXIlog_RESULT_NON_FATAL_ERROR;

   if (format) {
      va_start(arguments, format);
      rc = (*impl->log->VDiagnostic)(impl->log, tag + gblDiagLogBase, subtag,
            format, arguments);
      va_end(arguments);
   } else {
      rc = (*impl->log->Diagnostic)(impl->log, tag + gblDiagLogBase, subtag, 
            NULL);
   }

   return rc;
}


/*******************************************************
 *
 * Method routines for OSBtelInterface structure
 *
 *******************************************************/ 

// Get the VXItel interface version supported
//
static VXIint32 OSBtelGetVersion(void)
{
   return VXI_CURRENT_VERSION;
}



// Get the implementation name
//
static const VXIchar* OSBtelGetImplementationName(void)
{
   static const VXIchar IMPLEMENTATION_NAME[] = COMPANY_DOMAIN L".OSBtel";
   return IMPLEMENTATION_NAME;
}


// Begin a session
//
   static 
VXItelResult OSBtelBeginSession(VXItelInterface * pThis, VXIMap *sessionArgs)
{
   OSBtelImpl *impl = ToOSBtelImpl(pThis);
   impl->live = -1; // not quite live yet, other threads shouldn't its access resources

   Diag(impl, DIAG_TAG_SIGNALING, NULL, L"tel BeginSession");
   const VXIchar *callId = NULL;
   const VXIValue *val;
   val = VXIMapGetProperty(sessionArgs, L"callid");

   impl->pExitGuard = new OsBSem(OsBSem::Q_PRIORITY, OsBSem::FULL);
   impl->transferred = 0;
   impl->callId = NULL;
   if ((val) && (VXIValueGetType(val) == VALUE_STRING))
   {
      callId = VXIStringCStr((const VXIString *) val);
      int len = strlen((char*)callId) + 1;

      VXIchar *vxiCallid = (VXIchar *) calloc(len, sizeof(VXIchar));
      if (vxiCallid)
      {
         strncpy((char*)vxiCallid, (char*)callId, len);
         impl->callId = vxiCallid;
      }
      else
         return VXItel_RESULT_OUT_OF_MEMORY;
   }
   else
      return VXItel_RESULT_INVALID_ARGUMENT;


   impl->live = 1; // live
   return VXItel_RESULT_SUCCESS;
}


// End a session
//
   static
VXItelResult OSBtelEndSession(VXItelInterface *pThis, VXIMap *)
{
   OSBtelImpl *impl = ToOSBtelImpl(pThis);
   if (!impl) return VXItel_RESULT_INVALID_ARGUMENT;

   if (impl->pExitGuard)
   {
      impl->pExitGuard->acquire();
      impl->live = 0;

      Diag(impl, DIAG_TAG_SIGNALING, NULL, L"tel EndSession");

      if (impl->callId && impl->pCallMgr && !impl->transferred)
      {
         impl->pCallMgr->drop((char*)impl->callId);
         free(impl->callId);
         impl->callId = NULL;
      }
      impl->pExitGuard->release();
   }
   return VXItel_RESULT_SUCCESS;
}

OSBTEL_API VXItelResult OSBtelExiting(VXIlogInterface  *log,
      VXItelInterface **tel)
{ 
   if ( ! log ) return VXItel_RESULT_INVALID_ARGUMENT;

   OSBtelImpl *impl = ToOSBtelImpl(*tel);
   if ( !impl ) return VXItel_RESULT_INVALID_ARGUMENT;

   if ((impl->live == 1) && impl->pExitGuard)
   {
      impl->pExitGuard->acquire();
      if (impl->live == 1)
      {   
         impl->live = 0; // exited

         Diag(impl, DIAG_TAG_SIGNALING, NULL, L"tel Exiting");

         if (impl->waitingForTransferState && impl->pListener)
         {
           Diag(impl, DIAG_TAG_SIGNALING, NULL, L"tel Exiting - stopWaitForFinalState");
        if (FALSE == impl->pListener->stopWaitForFinalState((char*)impl->callId))
                {
                        OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR, "stopWaitForFinalState failed for %s\n", (char*)impl->callId);
                Diag(impl, DIAG_TAG_SIGNALING, NULL, L"stopWaitForFinalState failed");
                }       
         }
         else
            Diag(impl, DIAG_TAG_SIGNALING, NULL, L"tel Exiting - not in transferring mode");
      }
      impl->pExitGuard->release();
   }
   return VXItel_RESULT_SUCCESS; 
}

   static
VXItelResult OSBtelGetStatus(VXItelInterface * pThis, VXItelStatus *status)
{
   *status = VXItel_STATUS_ACTIVE;
   return VXItel_RESULT_SUCCESS;
}

/**
 * Disconnect caller (ie hang up).
 *
 * @param  ch   A handle to the channel resource manager
 */
   static
VXItelResult OSBtelDisconnect(VXItelInterface * vxip)
{
   OSBtelImpl *impl = ToOSBtelImpl(vxip);
   Diag(impl, DIAG_TAG_SIGNALING, NULL, L"Disconnect");

   if (impl->callId && impl->pCallMgr)
   {
      impl->pCallMgr->drop((char*)impl->callId);
      free(impl->callId);
      impl->callId = NULL;
   }

   return VXItel_RESULT_SUCCESS;
}

static
VXItelResult setTransferState(OSBtelImpl *impl, 
      const VXIMap * prop,
      const VXIchar * transferDestination,
      VXIMap ** resp,
      int* xstate,
      int* status)
{
   char remoteAddress[256];
   memset(remoteAddress, 0, 256);
   impl->waitingForTransferState = 1;

   int state = impl->pListener->waitForFinalState((char*)impl->callId, (char*)&remoteAddress);
   impl->waitingForTransferState = 0;

   Diag(impl, DIAG_TAG_SIGNALING, NULL, L"Transfer state: %d %d", (state&0x0000ffff), (state >> 16));
   if (impl->live == 0 || state == 0)
      return VXItel_RESULT_TIMEOUT;

   switch ((state & 0x0000ffff))
   {
      case PtEvent::CONNECTION_DISCONNECTED:
         {
            impl->transferred = 1;
            *status = VXItel_TRANSFER_FAR_END_DISCONNECT;
            int len = strlen(remoteAddress);
            if (len > 0)
            {
               char* dest = new char [len + 1];
               wcstombs(dest, transferDestination, len);
               dest[len] = 0;

               if (strstr(remoteAddress, dest) == 0 && strstr(dest, remoteAddress) == 0)
                  *status = VXItel_TRANSFER_NEAR_END_DISCONNECT;
               delete[] dest;
               dest = NULL;
            }
            break;
         }

      case PtEvent::CONNECTION_OFFERED:
      case PtEvent::CONNECTION_ESTABLISHED:
      case PtEvent::CONNECTION_CONNECTED:
         impl->transferred = 1;
         break;

      case PtEvent::CONNECTION_FAILED:
         {
            switch (state >> 16)
            {
               case PtEvent::CAUSE_DESTINATION_NOT_OBTAINABLE:
                  *status = VXItel_TRANSFER_NOANSWER;
                  break;

               case PtEvent::CAUSE_BUSY:
                  *status = VXItel_TRANSFER_BUSY;
                  break;

               case PtEvent::CAUSE_CALL_NOT_ANSWERED:
                  *status = VXItel_TRANSFER_NOANSWER;
                  break;

               case PtEvent::CAUSE_CALL_CANCELLED:
                  *status = VXItel_TRANSFER_NEAR_END_DISCONNECT;
                  break;

               default:
                  *status = VXItel_TRANSFER_UNKNOWN;
                  break;
            }
         }
         break;
      default:
         impl->transferred = 1;
         *status = VXItel_TRANSFER_UNKNOWN;
         break;
   }

   *xstate = (state & 0x0000ffff);
   return VXItel_RESULT_SUCCESS;
}

/**
 * Blind Transfer.
 */
static
VXItelResult OSBtelTransferBlind(VXItelInterface * vxip, 
      const VXIMap * prop,
      const VXIchar * transferDestination,
      const VXIMap * data,
      VXIMap ** resp)
{
   OSBtelImpl *impl = ToOSBtelImpl(vxip);
   Diag(impl, DIAG_TAG_SIGNALING, NULL, L"TransferBlind: %s", 
         transferDestination);

   VXIchar* best = (VXIchar *) calloc(128, sizeof(VXIchar));
   if (prop != NULL) {
      VXIString* vect =(VXIString*)VXIMapGetProperty(prop, L"Destination");
      if (vect != NULL) best = (VXIchar*) VXIStringCStr(vect);
   }

   *resp = VXIMapCreate();
   if (impl->callId && transferDestination)
   {
      vxistring dest = transferDestination;
      if (! dest.empty()) 
      {
         char* str = 0;
         int len = dest.length() + 1;
         if (len > 0)
         {
            str = new char [len + 1];
            wcstombs(str, transferDestination, len);
            str[len] = 0;
         }

         if (str)
         {
            UtlString from(str);
            HttpMessage::unescape( from );
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG, "OSBtelTransferBlind from = '%s'", from.data());
            if (impl->live == 1) 
            {
                if (PT_SUCCESS == impl->pCallMgr->transfer_blind((char*)impl->callId,
                                                                 from.data(), // to url
                                                                 0, // target call id
                                                                 0, // target connect addr
                                                                 false // no remote hold first
                                                                 )
                    )
                {
                        impl->transferred = 0;
                        int state = 0;
                        int status = VXItel_TRANSFER_UNKNOWN;
                        int duration = 0;
                        OsTime   startTime;
                        OsTime   endTime;
                        OsDateTimeBase::getCurTime(startTime);

                        setTransferState(impl, prop, transferDestination, resp, &state, &status);

                        OsDateTimeBase::getCurTime(endTime);
                        duration = (endTime.cvtToMsecs() - startTime.cvtToMsecs())/1000;
                        VXIMapSetProperty(*resp, TEL_TRANSFER_STATUS, (VXIValue *) VXIIntegerCreate(status));
                        VXIMapSetProperty(*resp, TEL_TRANSFER_DURATION, (VXIValue *)VXIIntegerCreate(duration));
                }
            }
            else
            {
                Diag(impl, DIAG_TAG_SIGNALING, NULL, L"Exiting called, live=%d, cancel TransferBlind: %s", 
                        impl->live, transferDestination);
            }
         }
      }
   }

   return VXItel_RESULT_SUCCESS;
}

/**
 * Bridging Transfer.
 *
 */
static
VXItelResult OSBtelTransferBridge(VXItelInterface * vxip, 
      const VXIMap * prop,
      const VXIchar * transferDestination,
      const VXIMap * data,
      VXIMap **resp)
{
   OSBtelImpl *impl = ToOSBtelImpl(vxip);
   Diag(impl, DIAG_TAG_SIGNALING, NULL, L"TransferBridge: %s", 
         transferDestination);

   *resp = VXIMapCreate();

   VXIchar* dest = 0;
   VXIchar* xaudio = 0;
   if (prop != NULL) {
      VXIString* vect =(VXIString*)VXIMapGetProperty(prop, L"Destination");
      if (vect != NULL) 
         dest = (VXIchar*) VXIStringCStr(vect);

      vect =(VXIString*)VXIMapGetProperty(prop, TEL_TRANSFER_AUDIO);
      if (vect != NULL) 
         xaudio = (VXIchar*) VXIStringCStr(vect);
   }

   *resp = VXIMapCreate();
   if (impl->callId && transferDestination)
   {
      vxistring dest = transferDestination;
      if (! dest.empty()) 
      {
         char* str = 0;
         int len = dest.length() + 1;
         if (len > 0)
         {
            str = new char [len + 1];
            wcstombs(str, transferDestination, len);
            str[len] = 0;
         }

         if (str)
         {
            UtlString from(str);
            HttpMessage::unescape( from );

            if (impl->live == 1)
            {
                impl->pCallMgr->connect((char*)impl->callId, from.data()); 

                impl->transferred = 0;
                int state = 0;
                int status = VXItel_TRANSFER_UNKNOWN;
                int duration = 0;
                OsTime   startTime;
                OsTime   endTime;
                OsDateTimeBase::getCurTime(startTime);

                VXItelResult ret;
                do 
                {
                        ret = setTransferState(impl, prop, transferDestination, resp, &state, &status);
                }
                while (state != PtEvent::CONNECTION_DISCONNECTED &&
                          state != PtEvent::CONNECTION_FAILED &&
                          ret != VXItel_RESULT_TIMEOUT);

                OsDateTimeBase::getCurTime(endTime);
                duration = (endTime.cvtToMsecs() - startTime.cvtToMsecs())/1000;
                VXIMapSetProperty(*resp, TEL_TRANSFER_STATUS, (VXIValue *) VXIIntegerCreate(status));
                VXIMapSetProperty(*resp, TEL_TRANSFER_DURATION, (VXIValue *)VXIIntegerCreate(duration));
            }
            else
            {
                Diag(impl, DIAG_TAG_SIGNALING, NULL, L"Exiting called, live=%d, cancel TransferBridge: %s",
                        impl->live, transferDestination);
            }
         }
      }
   }

   return VXItel_RESULT_SUCCESS;
}


/**
 * Enable calls on the channel
 *
 */
   static VXItelResult
OSBtelEnableCall(struct OSBtelInterface  *pThis)
{
   if (! pThis) return VXItel_RESULT_INVALID_ARGUMENT;

   return VXItel_RESULT_SUCCESS;
}


/**
 * Wait for and answer a call on the channel
 *
 */
static VXItelResult
OSBtelWaitForCall(struct OSBtelInterface  *vxip,
      VXIMap                 **telephonyProps)
{
   if ((! vxip) || (! telephonyProps))
      return VXItel_RESULT_INVALID_ARGUMENT;

   OSBtelImpl *impl = ToOSBtelImpl(vxip);
   Diag(impl, DIAG_TAG_SIGNALING, NULL, L"New call");

   return VXItel_RESULT_SUCCESS;
}


/*******************************************************
 * Factory and init routines
 *******************************************************/ 

/**
 * Global initialization of Telephony platform.
 */
OSBTEL_API VXItelResult OSBtelInit (VXIlogInterface  *log,
      VXIunsigned       diagLogBase)
{
   if (! log) return VXItel_RESULT_INVALID_ARGUMENT;

   gblDiagLogBase = diagLogBase;
   return VXItel_RESULT_SUCCESS;
}


/**
 * Global shutdown of Telephony platform.
 */
OSBTEL_API VXItelResult OSBtelShutDown (VXIlogInterface  *log)
{
   if (! log) return VXItel_RESULT_INVALID_ARGUMENT;

   return VXItel_RESULT_SUCCESS;
}


/**
 * Creates an OSBtel implementation of the VXItel interface
 */
OSBTEL_API VXItelResult OSBtelCreateResource(VXIunsigned channelNum, 
      VXIlogInterface *log,
      VXItelInterface **tel)
{
   if (! log) return VXItel_RESULT_INVALID_ARGUMENT;
   OSBtelImpl* pp = new OSBtelImpl();
   if (pp == NULL) return VXItel_RESULT_OUT_OF_MEMORY;

   pp->log = log;
   pp->channel = channelNum;
   pp->osbIntf.intf.GetVersion     = OSBtelGetVersion;
   pp->osbIntf.intf.GetImplementationName = OSBtelGetImplementationName;
   pp->osbIntf.intf.BeginSession   = OSBtelBeginSession;
   pp->osbIntf.intf.EndSession     = OSBtelEndSession;
   pp->osbIntf.intf.GetStatus      = OSBtelGetStatus;
   pp->osbIntf.intf.Disconnect     = OSBtelDisconnect;
   pp->osbIntf.intf.TransferBlind  = OSBtelTransferBlind;
   pp->osbIntf.intf.TransferBridge = OSBtelTransferBridge;
   pp->osbIntf.EnableCall          = OSBtelEnableCall;
   pp->osbIntf.WaitForCall         = OSBtelWaitForCall;

   *tel = &pp->osbIntf.intf;
   return VXItel_RESULT_SUCCESS;
}

/**
 * Add call manager handle to the VXItel interface
 */
OSBTEL_API VXItelResult OSBtelAddResource (VXItelInterface **tel,
      CallManager *pCallMgr,
      IvrTelListener *pListener)
{
   if (tel == NULL || *tel == NULL) return VXItel_RESULT_INVALID_ARGUMENT;
   OSBtelImpl* telImpl = reinterpret_cast<OSBtelImpl*>(*tel);

   telImpl->pCallMgr = pCallMgr;
   telImpl->pListener = pListener;
   telImpl->live = -1;

   return VXItel_RESULT_SUCCESS;
}


/**
 * Destroys the specified OSBtel implementation
 */
OSBTEL_API VXItelResult OSBtelDestroyResource(VXItelInterface **tel)
{
   if (tel == NULL || *tel == NULL) return VXItel_RESULT_INVALID_ARGUMENT;
   OSBtelImpl* telImpl = reinterpret_cast<OSBtelImpl*>(*tel);

   Diag(telImpl, DIAG_TAG_SIGNALING, NULL, L"tel DestroyResource");
   if (telImpl)
   {
      if (telImpl->pExitGuard)
      {
         telImpl->pExitGuard->acquire();
         telImpl->live = 0;
         telImpl->pExitGuard->release();
              delete telImpl->pExitGuard;       
              telImpl->pExitGuard = NULL;
      } 

      delete telImpl;
      telImpl = NULL;
      *tel = NULL;
   }

   return VXItel_RESULT_SUCCESS;
}

