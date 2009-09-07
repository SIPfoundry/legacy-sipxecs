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
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

// APPLICATION INCLUDES
#include "os/OsSysLogFacilities.h"
#include "os/OsSysLog.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

// sFacilityName provides string based representations of the various
// facilities within the system.  Indexed by enum tagOsSysLogFacility values.
const char* OsSysLog::sFacilityNames[] =
{
   "PERF",
   "KERNEL",
   "AUTH",
   "NET",
   "RTP",
   "PHONESET",
   "HTTP",
   "SIP",
   "CP",
   "MP",
   "TAO",
   "JNI",
   "JAVA",
   "LOG",
   "SUPERVISOR",
   "OUTGOING",
   "INCOMING",
   "INCOMING_PARSED",
   "MEDIASERVER_CGI",
   "MEDIASERVER_VXI",
   "ACD",
   "PARK",
   "APACHE_AUTH",
   "UPGRADE",
   "LINE_MGR",
   "REFRESH_MGR",
   "UNIT_TEST",
   "STREAMING",
   "REPLICATION_CGI",
   "SIPDB",
   "PROCESSMGR",
   "OS",
   "SIPXTAPI",
   "AUDIO",
   "CONFERENCE",
   "ODBC",
   "CDR",
   "RLS",
   "XMLRPC",
   "FSM",
   "NAT",
   "ALARM",
   "SAA",
   "UNKNOWN",
} ;



/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
