//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _OsSysLogFacilities_h_
#define _OsSysLogFacilities_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// ENUMS

//
// *** READ THIS ***
//
// NOTE:  If adding a facility, please:
//        1) Insert it before the FAC_MAX_FACILITY.
//        2) Update OsSysLogFacilties.cpp to include the
//           string name.
//        3) Update the !enum comments below.
//
// *** READ THIS ***
//
//

enum tagOsSysLogFacility
{
   FAC_PERF=0,             // performance related
   FAC_KERNEL,             // kernel/os related
   FAC_AUTH,               // authentication/security related
   FAC_NET,                // networking related
   FAC_RTP,                // RTP/RTCP related
   FAC_PHONESET,           // phoneset related
   FAC_HTTP,               // http sever related
   FAC_SIP,                // sip related
   FAC_CP,                 // call processing related
   FAC_MP,                 // media processing related
   FAC_TAO,                // TAO related
   FAC_JNI,                // JNI Layer related
   FAC_JAVA,               // Java related
   FAC_LOG,                // OsSysLog related
   FAC_WATCHDOG,           // WatchDog related
   FAC_SIP_OUTGOING,       // Outgoing SIP messages
   FAC_SIP_INCOMING,       // Incoming SIP messages
   FAC_SIP_INCOMING_PARSED,// Incoming SIP messages after being parsed
   FAC_MEDIASERVER_CGI,    // Mediaserver CGIs
   FAC_MEDIASERVER_VXI,    // Mediaserver VXI engine
   FAC_ACD,                // ACD related
   FAC_PARK,               // Park Server related
   FAC_APACHE_AUTH,        // Apache Authentication Module
   FAC_UPGRADE,            // Update/Upgrade related
   FAC_LINE_MGR,           // SIP line manager related
   FAC_REFRESH_MGR,        // SIP refresh manager related
   FAC_PROCESSCGI,         // Process Management CGI (process.cgi)
   FAC_STREAMING,          // Stream Media related message
   FAC_REPLICATION_CGI,    // replication cgi( replicates databases across components )
   FAC_DB,                 // Database related (sipdb)
   FAC_PROCESSMGR,         // OsProcessMgr
   FAC_PROCESS,            // process related
   FAC_SIPXTAPI,           // sipXtapi related
   FAC_AUDIO,              // audio related
   FAC_CONFERENCE,         // Conference bridge
   FAC_ODBC,               // ODBC related
   FAC_CDR,                // CDR generating related
   FAC_RLS,                // Resource list server
   FAC_MAX_FACILITY        // Last Facility (used to for length)

   //
   // *** READ THIS ***
   //
   // NOTE:  If adding a facility, please:
   //        1) Insert it before FAC_MAX_FACILITY.
   //        2) Updatd OsSysLogFacilties.cpp to include the
   //           string name.
   //        3) Update the !enum comments below.
   //
   // *** READ THIS ***
   //
   //
} ;
  //: Defines the various facilities available for platforms.
  //
  //
  //!enumcode: FAC_PERF - performance related
  //!enumcode: FAC_KERNEL - kernel/os related
  //!enumcode: FAC_AUTH - authentication/security related
  //!enumcode: FAC_NET - networking related
  //!enumcode: FAC_RTP - RTP/RTCP related
  //!enumcode: FAC_PHONESET - phoneset related
  //!enumcode: FAC_HTTP - http sever related
  //!enumcode: FAC_SIP - sip related
  //!enumcode: FAC_CP - call processing related
  //!enumcode: FAC_TAO - TAO related
  //!enumcode: FAC_JNI - JNI Layer related
  //!enumcode: FAC_JAVA - Java related
  //!enumcode: FAC_LOG - OsSysLog related
  //!enumcode: FAC_SIP_OUTGOING - Outgoing SIP messages
  //!enumcode: FAC_SIP_INCOMING - Incoming SIP messages
  //!enumcode: FAC_SIP_INCOMING_PARSED - Incoming SIP messages after being parsed
  //!enumcode: FAC_MEDIASERVER_CGI - Mediaserver CGIs
  //!enumcode: FAC_MEDIASERVER_VXI - Mediaserver VXI engine
  //!enumcode: FAC_ACD - ACD related
  //!enumcode: FAC_PARK - Park Server related
  //!enumcode: FAC_APACHE_AUTH - Apache Authentication Module
  //!enumcode: FAC_UPGRADE - Update/Upgrade related
  //!enumcode: FAC_PROCESSCGI - Process Management CGI (process.cgi)
  //!enumcode: FAC_DB - Database related (sipdb)
  //!enumcode: FAC_REPLICATION_CGI - replication cgi (replicates databases across components)
  //!enumcode: FAC_PROCESSMGR - os processmanager related
  //!enumcode: FAC_PROCESS - process related
  //!enumcode: FAC_SIPXTAPI - sipXtapi related
  //!enumcode: FAC_AUDIO - audio related
  //!enumcode: FAC_CONFERENCE - Conference bridge
  //!enumcode: FAC_ODBC - ODBC related
  //!enumcode: FAC_CDR - CDR generating related
  //!enumcode: FAC_RLS - Resource list server

// TYPEDEFS
typedef enum tagOsSysLogFacility OsSysLogFacility ;

// FORWARD DECLARATIONS

#endif  /* _OsSysLogFacilities_h_ ] */
