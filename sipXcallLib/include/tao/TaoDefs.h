//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _TaoDefs_h_
#define _TaoDefs_h_

#include <stdint.h>

// Constants
#define MAXIMUM_INTEGER_STRING_LENGTH 20
#define DEF_TAO_VERY_BIG_NUMBER 2147483647
#define DEF_TAO_MSG_MAX_AGR_NUM 128     // default maximum number of argument in a TaoClientMsg
#define DEF_TAO_MAX_SOCKET_SIZE 1024
#define DEF_TAO_MAX_BUFFER_SIZE 128
#define DEF_TAO_MAX_CONNECTION  5       // max number of connection sockets on the
                                                                        // TaoServer transport
#define DEF_TAO_LISTEN_PORT             9000
#define DEF_TAO_EVENT_PORT              9001
#define HTTP_READ_TIMEOUT_MSECS  30000

#define TAOMESSAGE_DELIMITER    UtlString("$d$")

// TYPEDEFS
typedef unsigned long TaoEventId;
typedef unsigned long TaoObjHandle;
//:A 32-bit or 64-bit integer that has local meaning for a given TaoServer and represents
//:a PTAPI object or a TaoObject.

typedef unsigned int  IPAddress;
//:A 32-bit value that contains an IP address. To convert this value to the a.b.c.d string
//:form of an IP address, map the high-order byte to a, the low-order byte to d, and so on.

//:Status codes returned by Pingtel API methods
enum TaoStatus
{
        TAO_INVALID=0,              // invalid status (not yet set)
        TAO_SUCCESS,                // success
        TAO_FAILURE,                // failure

        // Name database
        TAO_IN_USE,                             // requested object name is already in use
        TAO_MORE_DATA,
        TAO_NO_MORE_DATA,           // no more data exists

        // Resource management
        TAO_DELETED,                // object has been deleted
        TAO_NOT_FOUND,              // object not found
        TAO_NOT_OWNER,              // not the owner of the resource
        TAO_LIMIT_REACHED,          // resource limit reached
        TAO_UNAVAILABLE,
        TAO_EXISTS,                 // resource exists

        // Synchronization
        TAO_BUSY,                   // requested object is locked
        TAO_IN_PROGRESS,

        // Message queues
        TAO_INVALID_LENGTH,         // message is too long for this message queue
        TAO_INVALID_STATE,

        // Event Management
        TAO_ALREADY_SIGNALED,       // attempt to signal an already signaled event
        TAO_NOT_SIGNALED,           // attempt to clear a not-yet-signaled event

        // Scheduling
        TAO_INVALID_PRIORITY,       // you requested an invalid priority level
        TAO_NO_TASK_READY,          // no task ready (when attempting to yield
                                                          //  the remainder of a time slice)
        // Task Management
        TAO_TASK_NOT_STARTED,       // task has not yet been started (or has been
                                                          //  shut down)
        // Memory Management
        TAO_NO_MEMORY,              // memory allocation error

        // Other
        TAO_INVALID_ARGUMENT,       // invalid argument to subroutine
        TAO_NOT_YET_IMPLEMENTED,    // coming soon ...
        TAO_AUTH_FAILED,
        TAO_UNSPECIFIED             // unspecified error
};

// TaoObjTypes defined for Tao objects or PTAPI objects
enum TaoObjTypes
{
  UNSPECIFIED = 0,
  TAO_OBJECT,           // the Tao objects
  PTAPI_OBJECT                  // PTAPI objects
};

// TaoObjSubTypes categories defined for use by the TaoObjectMap object
enum TaoObjSubTypes
{
  TAO_SERVER,           // the TaoServer object
  TAO_CLIENT,           // the TaoClient object
  TAO_MESSAGE,          // the TaoMessage object
  TAO_MSGQ,                             // the TaoMsgQ object
  TAO_TRANSPORT,        // the TaoTransport object
  TAO_CONNECTLISTENER   // the TaoConnectionListener object
};

// PtObjSubTypes categories defined for use by the TaoObjectMap object
enum PtObjSubTypes
{
  PTAPI_PROVIDER,       // PtProvider
  PTAPI_ADDRESS,        // PtAddress
  PTAPI_TERMINAL,       // PtTerminal
  PTAPI_CALL,                   // PtCall
  PTAPI_CALLLISTENER,   // PtCallListener
  PTAPI_CONNECT                 // PtConnection
};


#endif // _TaoDefs_h_
