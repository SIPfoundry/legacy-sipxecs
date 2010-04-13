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
// NOTE: If you need a new facility, please first look for an enum value
//       that is available for re-use.  (e.g. FAC_AVAILABLE_FOR_REUSE)
//       If no enum values are available for re-use, then add a new one,
//       but please:
//        1) Insert it before the FAC_MAX_FACILITY.
//        2) Update OsSysLogFacilities.cpp to include the
//           string name.
//        3) Update the !enum comments below.
//
//       If removing a facility, do NOT delete the enum value.
//       Instead, please:
//        1) Rename the enum value to flag it as available for re-use.
//           (e.g. FAC_AVAILABLE_FOR_REUSE)
//        2) Update OsSysLogFacilities.cpp to change the string name
//           to "UNUSED".
//        3) Update the !enum comments below.
//
// *** READ THIS ***
//
//

enum tagOsSysLogFacility
{
   FAC_PERF=0,             ///< performance related
   FAC_KERNEL,             ///< kernel/os related
   FAC_AUTH,               ///< authentication/security related
   FAC_NET,                ///< networking related
   FAC_RTP,                ///< RTP/RTCP related
   FAC_PHONESET,           ///< phoneset related
   FAC_HTTP,               ///< http sever related
   FAC_SIP,                ///< sip related
   FAC_CP,                 ///< call processing related
   FAC_MP,                 ///< media processing related
   FAC_TAO,                ///< TAO related
   FAC_JNI,                ///< JNI Layer related
   FAC_JAVA,               ///< Java related
   FAC_LOG,                ///< OsSysLog related
   FAC_SUPERVISOR,         ///< sipXsupervisor
   FAC_SIP_OUTGOING,       ///< Outgoing SIP messages
   FAC_SIP_INCOMING,       ///< Incoming SIP messages
   FAC_SIP_INCOMING_PARSED,///< Incoming SIP messages after being parsed
   FAC_MEDIASERVER_CGI,    ///< Mediaserver CGIs
   FAC_MEDIASERVER_VXI,    ///< Mediaserver VXI engine
   FAC_ACD,                ///< ACD related
   FAC_PARK,               ///< Park Server related
   FAC_APACHE_AUTH,        ///< Apache Authentication Module
   FAC_UPGRADE,            ///< Update/Upgrade related
   FAC_LINE_MGR,           ///< SIP line manager related
   FAC_REFRESH_MGR,        ///< SIP refresh manager related
   FAC_UNIT_TEST,          ///< Available for re-use.
   FAC_STREAMING,          ///< Stream Media related message
   FAC_REPLICATION_CGI,    ///< replication cgi( replicates databases across components )
   FAC_DB,                 ///< Database related (sipdb)
   FAC_PROCESSMGR,         ///< OsProcessMgr
   FAC_PROCESS,            ///< process related
   FAC_SIPXTAPI,           ///< sipXtapi related
   FAC_AUDIO,              ///< audio related
   FAC_CONFERENCE,         ///< Conference bridge
   FAC_ODBC,               ///< ODBC related
   FAC_CDR,                ///< CDR generating related
   FAC_RLS,                ///< Resource list server
   FAC_XMLRPC,             ///< XML RPC related
   FAC_FSM,                ///< Finite State Machine tracking
   FAC_NAT,                ///< NAT Traversal related
   FAC_ALARM,              ///< Alarms
   FAC_SAA,                ///< Shared Appearance Agent
   FAC_MAX_FACILITY        ///< Last Facility (used to for length)

   //
   // *** READ THIS ***
   //
   // NOTE:  If adding a facility, please:
   //        1) Insert it before FAC_MAX_FACILITY.
   //        2) Update OsSysLogFacilities.cpp to include the
   //           string name.
   //        3) Update the !enum comments above.
   //
   // *** READ THIS ***
   //
   //
} ;

// TYPEDEFS
typedef enum tagOsSysLogFacility OsSysLogFacility ;

// FORWARD DECLARATIONS

#endif  /* _OsSysLogFacilities_h_ ] */
