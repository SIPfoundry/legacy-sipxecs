//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _SipConfigServerAgent_h_
#define _SipConfigServerAgent_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsServerTask.h>
#include <net/SipMessage.h>
#include <os/OsBSem.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef int (*EnrollmentCallbackFunc)(const SipMessage& subscribeRequest);
// Note this function will need to parse out the following:
//  from the From header field URL: 
//      fromUser, fromAddress, fromPort
//  from the From header field parameters (Url:getHeaderFieldParameter):
//      vender, model, version,  mac,  serial
// from the Config-Requires header field:
//      numProfiles, profiles[]
// from the Config-Allow  header field
//      numProcols, protocols[]
// from the Config-Expires header field:
//      expirationSeconds
// Note: #defines exist in SipMessage.h for these header field names:
// SIP_FROM_FIELD, SIP_TO_FIELD,
// SIP_CONFIG_ALLOW_FIELD, SIP_CONFIG_REQUIRE_FIELD

typedef void (*NotifyResponseCallbackFunc)(const SipMessage& notifyResponse);
// Note: this function will need to parse out the
// following data from the SipMessage:
// from the header response status code: notifyChangeResponseCode
// from the header response status text: notifyChangeResponseText


// FORWARD DECLARATIONS
class SipUserAgent;
class OsConfigDb;


//:This class is the SIP configuation agent for the deployment serve
// From the SIP messaging perspective there are two types of messages
// related to configuration: 
// 1) Enrollment from the SIP UA/device to the server in the form of SUBSCRIBE
// 2) Configuration change notification from the config server to the device
//    in the form of a NOTIFY request
// <BR>
// The following message flow illustrates the communications
// between this object, the config. server and the managed
// SIP UA device.
//
// config.          SipConfigServerAgent       UA device              
// server
//                        <------------------SUBSCRIBE (enrollment)
//   <--------------EnrollmentCallbackFunc
//   return code --------->
//                  SUBSCRIBE response ---------->
// ...
// sendChangeNotification->
//                  NOTIFY (config change) ------>
//                        <------------------NOTIFY response
//   <--------------NotifyResponseCallbackFunc
//
// <BR>
// Enrollment is initiated by the device sending the SUBSCRIBE to the config
// server.  When the SipConfigServerAgent receives the SUBSCRIBE it calls
// the callback: EnrollmentCallbackFunc to provide the entollement data
// to the config. server.  The config. server must return the response
// code to be used for the SUBSCRIBE response.  Typically this will
// be: 202 Acepted for success, 400 Bad Request, etc. see SipMessage.h
// Included is a subscribe context that is needed if the config. server
// is to send change notifications.
// <BR>
// Later when the config. server wishes to send the config profile
// URLs, it calls the sendChangeNotification method.  This method
// returns a boolean indicating whether the NOTIFY was sent or
// not.  This does not indicate the request was received or not
// by the SIP UA device.  The final outcome is indicated to the
// config. server via the callaback: NotifyResponseCallbackFunc
// Note: I have not come up with a convenient means of matching
// the invocations of this call back with the sendChangeNotification
// invocation (i.e. same transaction).  So for now I won't implement
// this callback.

class SipConfigServerAgent : public OsServerTask
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   SipConfigServerAgent(SipUserAgent* userAgent);
     //:Default constructor

   virtual
   ~SipConfigServerAgent();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual UtlBoolean handleMessage(OsMsg& eventMessage);

   UtlBoolean sendChangeNotification(SipMessage& notifyRequest);
   //: Sends an config. change notification (NOTIFY) to the SIP UA device indicated in subscriberContext
   //! param: subscribeContext - text serialization of the enrollment SUBSCRIBE
   //! param: notifyCSeqence - the Csequence number for the NOTIFY request.  This can be random for the very first NOTIFY sent to a UA and should be incremented by one for each NOTIFY afterwards.
   //! param: configLicenseSeconds - the period of time after which the UA should consider the configuration to be stale.
   //! param: notifyBody - the contents for the NOTIFY request body.  zero or more lines of the format: <profile-name>: Sequence=<serial-config-id>;Url=<profile-config-url>

   // Note the application is responsible for keeping a copy of the
   // original request as it will need to copy and reverse the
   // To & From fields..  It should construct the proper route
   // field if there was a record-route field in the subscribe
   // request.  Each notify request should have an incremented
   // CSeq header field.  The NOTIFY body should contain the
   // profile urls and sequence numbers.

   // Note: we need a asychronous meand of letting the
   // caller of this method know that the message failed to send.

/* ============================ ACCESSORS ================================= */

   void setEnrollmentCallback(EnrollmentCallbackFunc callback);
   //: Sets the function which gets called when an enrollment SUBSCRIBE comes in

   void setChangeNotifyStatusCallback(NotifyResponseCallbackFunc callback);
   //: Sets the function which gets called when a response is received for a change notification

   static SipConfigServerAgent* getSipConfigServerAgent();
    //: Returns a singleton reference to the Sip Config Server Agent

   SipUserAgent* getSipUserAgent();
    //: Returns a reference to the Sip User Agent

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    static SipConfigServerAgent* startAgents(const char* configFileName) ;
    //: Starts up the SipUserAgent and the SipConfigServerAgent

    SipConfigServerAgent(const SipConfigServerAgent& rSipConfigServerAgent);
    //:Copy constructor diabled

    SipConfigServerAgent& operator=(const SipConfigServerAgent& rhs);
    //:Assignment operator diabled

    static SipConfigServerAgent* spInstance ;
    //: Singleton instance to ourself
    static OsBSem sLock;
    //: Semaphore guarding start up
    SipUserAgent* mpSipUserAgent;
    //: Reference to the Sip User Agent
    EnrollmentCallbackFunc mfpEnrollmentCallbackFunc;
    NotifyResponseCallbackFunc mfpNotifyResponseCallbackFunc;

    void static initializeLog(OsConfigDb* pConfigDb) ;
      //: Initialize the OsSysLog 

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipConfigServerAgent_h_
