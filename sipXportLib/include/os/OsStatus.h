//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsStatus_h_
#define _OsStatus_h_

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

//:Status codes returned by OS abstraction layer methods
enum OsStatus
{
   OS_INVALID=0,              // invalid status (not yet set)
   OS_SUCCESS,                // success
   OS_FAILED,                 // general purpose failure message

   // Name database
   OS_NAME_IN_USE,            // requested object name is already in use
   OS_NO_MORE_DATA,           // no more data exists

   // Resource management
   OS_DELETED,                // object has been deleted
   OS_NOT_FOUND,              // object not found
   OS_NOT_OWNER,              // not the owner of the resource
   OS_LIMIT_REACHED,          // resource limit reached

   // Synchronization
   OS_BUSY,                   // requested object is locked
   OS_NOT_ISR_CALLABLE,       // operation not available from an ISR

   // Timers
   OS_NO_TIMER_SUPPORT,       // timers not supported
   OS_WAIT_TIMEOUT,           // wait operation timed out
   OS_WAIT_ABANDONED,         // synchronization object was abandoned

   // Message queues
   OS_INVALID_LENGTH,         // message is too long for this message queue

   // Event Management
   OS_ALREADY_SIGNALED,       // attempt to signal an already signaled event
   OS_NOT_SIGNALED,           // attempt to clear a not-yet-signaled event

   // Scheduling
   OS_INVALID_PRIORITY,       // you requested an invalid priority level
   OS_NO_TASK_READY,          // no task ready (when attempting to yield
                              //  the remainder of a time slice)

   // Task Management
   OS_TASK_NOT_STARTED,       // task has not yet been started (or has been
                              //  shut down)

   OS_PORT_IN_USE,            // task failed while trying to bind to port

   // Network status
   OS_DESTINATION_UNREACHABLE,// no route to destination
   OS_DESTINATION_NOT_RESPONDING, // destination not responding
   OS_DHCP_UNAVAILABLE,       // DHCP lease renewal failed or timed out
   OS_DNS_UNAVAILABLE,        // no DNS server available
   OS_NETWORK_UNAVAILABLE,    // Network unusable, e.g. LINK is inactive

   // Memory Management
   OS_NO_MEMORY,              // memory allocation error

   // Directed Graph Management
   OS_LOOP_DETECTED,          // loop detected in flow graph

   // Version Check
   OS_VERSIONCHECK_NO_PLATFORMTYPE,             // platform type not provided while calling findLatestVersion
   OS_VERSIONCHECK_NO_LATESTURL,                // latest url not provided while calling findLatestVersion
   OS_VERSIONCHECK_NO_SCRIPTURL,                // script url not found in the catalog file
   OS_VERSIONCHECK_NO_CATALOGURL,               // catalog_url not found in the latest file
   OS_VERSIONCHECK_NO_LATESTVERSION,    // version not found in the latest file
   OS_VERSIONCHECK_FAILURE_CONNECT_TO_SERVER,   // failed to connect to the web server for the latest or catalog url
   OS_VERSIONCHECK_FAILURE_GET_LATESTFILE,              // failed to get the specified latest file
   OS_VERSIONCHECK_FAILURE_GET_CATALOGFILE,             // failed to get the specified catalog file
   OS_VERSIONCHECK_FAILURE_GET_UPGRADESCRIPTS,  // failed to get the specified upgrade scripts file
   OS_VERSIONCHECK_FAILURE_OUT_OF_MEMORY,               // failed to get the required memory

   // cmd results
        OS_COMMAND_NOT_FOUND,   // if the specified cmd is not supported
        OS_COMMAND_AMBIGUOUS,   // if the specified cmd is ambiguous, i.e., more than one cmd can be assigned to it
        OS_COMMAND_BAD_SYNTAX,  // if the arguments do not match specified the cmd

        OS_HTTP_MOVED_PERMANENTLY_CODE  = 301,  // Moved Permanently
        OS_HTTP_MOVED_TEMPORARILY_CODE  = 302,  // Moved Temporatily
        OS_HTTP_UNAUTHORIZED_CODE       = 401,  // Unauthorized
        OS_HTTP_FILE_NOT_FOUND_CODE     = 404,  // File Not Found
        OS_HTTP_PROXY_UNAUTHORIZED_CODE = 407,  // Proxy Authentication Required
        OS_HTTP_UNSUPPORTED_METHOD_CODE = 501,  // Not Implemented

        //File System Error Messages
        OS_FILE_DIRECTORY_ALREADY_EXISTS,
        OS_FILE_SAMENAME,
        OS_FILE_ACCESS_DENIED,
        OS_FILE_DISKFULL,
        OS_FILE_INVALID_HANDLE,
        OS_FILE_READONLY,
        OS_FILE_DIR_NOT_EMPTY,
        OS_FILE_PATH_NOT_FOUND,
        OS_FILE_NOT_FOUND,
        OS_FILE_WRITE_FAILED,
        OS_FILE_READ_FAILED,
        OS_FILE_EOF,
        OS_FILE_SEEK_ERROR,


   // Other
   OS_INTERRUPTED,            // operation was interrupted from completion
   OS_INVALID_ARGUMENT,       // invalid argument to subroutine
   OS_INVALID_STATE,          // invalid state, unable to perform operation
   OS_NOT_SUPPORTED,          // Not supported at this time
   OS_NOT_YET_IMPLEMENTED,    // coming soon ...
   OS_UNSPECIFIED,            // unspecified error
   OS_UNAUTHORIZED            // "unauthorized" error
};
     //!enumcode: OS_INVALID=0 - invalid status (not yet set)
     //!enumcode: OS_SUCCESS - success
     //!enumcode: OS_NAME_IN_USE - requested object name is already in use
     //!enumcode: OS_NO_MORE_DATA - no more data exists
     //!enumcode: OS_DELETED - object has been deleted
     //!enumcode: OS_NOT_FOUND - object not found
     //!enumcode: OS_NOT_OWNER - not the owner of the resource
     //!enumcode: OS_LIMIT_REACHED - resource limit reached
     //!enumcode: OS_BUSY - requested object is locked
     //!enumcode: OS_NOT_ISR_CALLABLE - operation not available from an ISR
     //!enumcode: OS_NO_TIMER_SUPPORT - timers not supported
     //!enumcode: OS_WAIT_TIMEOUT - wait operation timed out
     //!enumcode: OS_WAIT_ABANDONED - synchronization object was abandoned
     //!enumcode: OS_INVALID_LENGTH - message is too long for this message queue
     //!enumcode: OS_ALREADY_SIGNALED - attempt to signal an already signaled event
     //!enumcode: OS_NOT_SIGNALED - attempt to clear a not-yet-signaled event
     //!enumcode: OS_INVALID_PRIORITY - you requested an invalid priority level
     //!enumcode: OS_NO_TASK_READY - no task ready (when attempting to yield the remainder of a time slice)
     //!enumcode: OS_TASK_NOT_STARTED - task has not yet been started (or has been shut down)
     //!enumcode: OS_NO_MEMORY - memory allocation error
     //!enumcode: OS_LOOP_DETECTED - loop detected in flow graph
     //!enumcode: OS_DESTINATION_UNREACHABLE - TBS
     //!enumcode: OS_DESTINATION_NOTRESPONDING - TBS
     //!enumcode: OS_DHCP_UNAVAILABLE - DHCP lease renewal timed out or failed
     //!enumcode: OS_DNS_UNAVAILABLE - No DNS server available
     //!enumcode: OS_NETWORK_UNAVAILABLE - Network unusable (e.g. no LINK)
     //!enumcode: OS_INVALID_ARGUMENT - invalid argument to subroutine
     //!enumcode: OS_INVALID_STATE - invalid state, unable to perform operation
     //!enumcode: OS_NOT_SUPPORTED - Not supported at this time
     //!enumcode: OS_NOT_YET_IMPLEMENTED - coming soon...
     //!enumcode: OS_UNSPECIFIED - unspecified error

/* ============================ INLINE METHODS ============================ */

#endif  // _OsStatus_h_
