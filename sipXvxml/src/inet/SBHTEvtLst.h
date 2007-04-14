/*****************************************************************************
 *****************************************************************************
 *
 * 
 *
 * Enhanced W3C Sample Code Library libwww Persistent Cache Manager
 *
 * Enhanced to add locks in the event loop for multi-threaded SBinet.
 *
 * Original W3C Libwww notes:
 *
 * Updated HTEvent module 
 * This new module combines the functions of the old HTEvent module and 
 * the HTThread module. We retain the old HTThread module, but it
 * consists of calls to the HTEvent interfaces
 *
 * Authors:
 *	HFN	Henrik Frystyk <frystyk@w3.org>
 *	CLB    	Charlie Brooks <cbrooks@osf.org>
 *   WSAAsyncSelect and windows app stuff need the following definitions:
 *   WWW_WIN_ASYNC - enable WSAAsyncSelect instead of select
 *   _WIN23 - win32 libararies - may be window or console app
 *   _WINSOCKAPI_ - using WINSOCK.DLL - not necessarily the async routines.
 *   _CONSOLE - the console app for NT
 *
 * first pass: EGP - 10/26/95
 *
 *****************************************************************************
 ****************************************************************************/


/****************License************************************************
 *
 * (c) COPYRIGHT MIT 1995.
 * Please first read the full copyright statement in the file COPYRIGH.
 *
 * Copyright 2000-2001.  SpeechWorks International, Inc.  
 *
 * Use of this software is subject to notices and obligations set forth
 * in the SpeechWorks Public License - Software Version 1.1 which is
 * included with this software.
 *
 * SpeechWorks is a registered trademark, and SpeechWorks Here, 
 * DialogModules and the SpeechWorks logo are trademarks of SpeechWorks 
 * International, Inc. in the United States and other countries. 
 * 
 ************************************************************************
 */

#ifndef SBHTEVTLST_H
#define SBHTEVTLST_H

#include <time.h>
#include "wwwsys.h"
#include "HTEvent.h"
#include "HTReq.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
(
  Windows Specific Handles
)
*/

#if defined(WWW_WIN_ASYNC) || defined(WWW_WIN_DLL)
extern BOOL SBinetHTEventList_winHandle (HTRequest * request);
extern BOOL SBinetHTEventList_setWinHandle (HWND window, unsigned long message);
extern HWND SBinetHTEventList_getWinHandle (unsigned long * pMessage);
extern LRESULT CALLBACK SBinetAsyncWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#endif

/*
.
  Event Registry
.

The libwww event registry binds a socket and operation (FD_READ,
FD_WRITE, ...) to a callback function. Events are registered,
unregistered, and dispatched as they come in.
(
  Register an Event Handler
)

For a given socket, reqister a request structure, a set of operations, a
HTEventCallback function, and a priority. For this implementation, we allow
only a single HTEventCallback function for all operations. and the priority
field is ignored.
*/

extern HTEvent_registerCallback SBinetHTEventList_register;

/*
(
  Unregister an Event Handler
)

Remove the registered information for the specified socket for the actions
specified in ops. if no actions remain after the unregister, the registered
info is deleted, and, if the socket has been registered for notification,
the HTEventCallback will be invoked.
*/

extern HTEvent_unregisterCallback SBinetHTEventList_unregister;

/*
(
  Unregister ALL Event Handlers
)

Unregister all sockets. N.B. we just remove them for our internal data
structures: it is up to the application to actually close the socket.
*/

extern int SBinetHTEventList_unregisterAll (void);

/*
(
  Lookup and Dispatch Event Handlers
)

Callbacks can be looked up or dispatched based on the socket and operation
(read/write/oob)
*/

extern int SBinetHTEventList_dispatch (SOCKET s, HTEventType type, ms_t now);
extern HTEvent * SBinetHTEventList_lookup (SOCKET s, HTEventType type);

/*
.
  Libwww Default EventLoop
.

The libwww default eventloop dispatches events to the event
registry.
(
  Start and Stop the Event Manager
)
*/

extern BOOL SBinetHTEventInit (void);
extern BOOL SBinetHTEventTerminate (void);

/*
(
  Start the Eventloop
)

That is, we wait for activity from one of our registered channels, and dispatch
on that. Under Windows/NT, we must treat the console and sockets as distinct.
That means we can't avoid a busy wait, but we do our best.
*/

extern int SBinetHTEventList_newLoop (void);

/*

The next version is an old version of the eventloop start. The request is
not used for anything and can be NULL.
*/

extern int SBinetHTEventList_loop (HTRequest * request);

/*
(
  Stop the Eventloop
)

Stops the (select based) eventloop immediately. The function does not guarantee
that all requests have terminated so it is important that the application
does this before this function is called. This can be done using the
HTNet_isIdle() function in the HTNet Module
*/

extern void SBinetHTEventList_stopLoop (void);

/*
*/

#ifdef __cplusplus
}
#endif

#endif /* SBHTEVTLST_H */
