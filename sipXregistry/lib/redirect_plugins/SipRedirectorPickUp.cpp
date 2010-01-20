//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdlib.h>
#include <limits.h>

// APPLICATION INCLUDES
#include "os/OsSysLog.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/ResultSet.h"
#include "sipdb/CredentialDB.h"
#include "SipRedirectorPickUp.h"
#include "os/OsProcess.h"
#include "net/NetMd5Codec.h"
#include "net/Url.h"
#include "net/CallId.h"
#include "net/SipDialogEvent.h"
#include "net/SipLine.h"
#include "net/SipLineMgr.h"
#include "registry/SipRedirectServer.h"
#include "xmlparser/ExtractContent.h"
#include "net/SipMessage.h"

// DEFINES

// The parameter giving the directed call pick-up feature code.
#define CONFIG_SETTING_DIRECTED_CODE \
    "DIRECTED_CALL_PICKUP_CODE"
// The parameter giving the global call pick-up feature code.
#define CONFIG_SETTING_GLOBAL_CODE \
    "GLOBAL_CALL_PICKUP_CODE"
// The parameter giving the call retrieve feature code.
#define CONFIG_SETTING_RETRIEVE_CODE \
    "CALL_RETRIEVE_CODE"
// The parameter giving the SIP address of the park server.
#define CONFIG_SETTING_PARK_SERVER \
    "PARK_SERVER"
// The parameter giving the call pick-up wait time.
#define CONFIG_SETTING_WAIT \
    "CALL_PICKUP_WAIT"
// The parameter for activating the "no early-only" workaround.
#define CONFIG_SETTING_NEO \
    "PICKUP_NO_EARLY_ONLY"
// The parameter for activating the "reversed Replaces" workaround.
#define CONFIG_SETTING_RR \
    "PICKUP_REVERSED_REPLACES"
// The parameter for activating the "1 second subscription" workaround.
#define CONFIG_SETTING_1_SEC \
    "PICKUP_1_SEC_SUBSCRIBE"
// The parameter that specifies the file containing the parking orbits.
#define CONFIG_SETTING_ORBIT_FILENAME \
    "ORBIT_FILENAME"
// The default call pick-up wait time, in seconds and microseconds.
#define DEFAULT_WAIT_TIME_SECS        1
#define DEFAULT_WAIT_TIME_USECS       0
// The minimum and maximum call pick-up wait time allowed (floating-point).
#define MIN_WAIT_TIME            0.001
#define MAX_WAIT_TIME            100.0
// The parameter setting which IP address/interface to bind on
#define CONFIG_SETTING_BIND_IP \
   "SIP_REGISTRAR_BIND_IP"

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const int SipRedirectorPrivateStoragePickUp::TargetDialogDurationAbsent = -1;

const UtlContainableType SipRedirectorPrivateStoragePickUp::TYPE =
    "SipRedirectorPrivateStoragePickUp";

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

// Static factory function.
extern "C" RedirectPlugin* getRedirectPlugin(const UtlString& instanceName)
{
   return new SipRedirectorPickUp(instanceName);
}

// Constructor
SipRedirectorPickUp::SipRedirectorPickUp(const UtlString& instanceName) :
   RedirectPlugin(instanceName),
   mpSipUserAgent(NULL),
   mTask(NULL)
{
   mLogName.append("[");
   mLogName.append(instanceName);
   mLogName.append("] SipRedirectorPickUp");

   mLogNameGlobalPickUp = mLogName;
   mLogNameGlobalPickUp.append(" global call pick-up");
   mLogNameOrbit = mLogName;
   mLogNameOrbit.append(" orbit number");
   mLogNamePickUp = mLogName;
   mLogNamePickUp.append(" call pick-up");
   mLogNameRetrieve = mLogName;
   mLogNameRetrieve.append(" call retrieve");
}

// Destructor
SipRedirectorPickUp::~SipRedirectorPickUp()
{
}

// Read config information.
void SipRedirectorPickUp::readConfig(OsConfigDb& configDb)
{
   // The return status.
   // It will be OS_SUCCESS if this redirector is configured to do any work,
   // and OS_FAILED if not.
   mRedirectorActive = OS_FAILED;

   // Fetch the configuration parameters for the workaround features.
   // Defaults are set to match the previous behavior of the code.

   // No early-only.
   mNoEarlyOnly = configDb.getBoolean(CONFIG_SETTING_NEO, TRUE);
   OsSysLog::add(FAC_SIP, PRI_INFO,
                 "%s::readConfig mNoEarlyOnly = %d",
                 mLogName.data(), mNoEarlyOnly);
   // Reversed Replaces.
   mReversedReplaces = configDb.getBoolean(CONFIG_SETTING_RR, FALSE);
   OsSysLog::add(FAC_SIP, PRI_INFO,
                 "%s::readConfig mReversedReplaces = %d",
                 mLogName.data(), mReversedReplaces);
   // One-second subscriptions.
   mOneSecondSubscription = configDb.getBoolean(CONFIG_SETTING_1_SEC, TRUE);
   OsSysLog::add(FAC_SIP, PRI_INFO,
                 "%s::readConfig mOneSecondSubscription = %d",
                 mLogName.data(), mOneSecondSubscription);

   // Fetch the call pick-up festure code from the config file.
   // If it is null, it doesn't count.
   if ((configDb.get(CONFIG_SETTING_DIRECTED_CODE, mCallPickUpCode) !=
        OS_SUCCESS) ||
       mCallPickUpCode.isNull())
   {
      OsSysLog::add(FAC_SIP, PRI_INFO,
                    "%s::readConfig No call pick-up feature code specified",
                    mLogName.data());
   }
   else
   {
      // Call pick-up feature code is configured.
      // Initialize the system.
      OsSysLog::add(FAC_SIP, PRI_INFO,
                    "%s::readConfig Call pick-up feature code is '%s'",
                    mLogName.data(), mCallPickUpCode.data());
      mRedirectorActive = OS_SUCCESS;

      // Record the two user-names that are excluded as being pick-up requests.
      mExcludedUser1 = mCallPickUpCode;
      mExcludedUser1.append("*");
      mExcludedUser2 = mCallPickUpCode;
      mExcludedUser2.append("#");
   }

   // Fetch the global call pick-up username from the config file.
   // If it is null, it doesn't count.
   if ((configDb.get(CONFIG_SETTING_GLOBAL_CODE, mGlobalPickUpCode) !=
        OS_SUCCESS) ||
       mGlobalPickUpCode.isNull())
   {
      OsSysLog::add(FAC_SIP, PRI_INFO,
                    "%s::readConfig No global call pick-up code specified",
                    mLogName.data());
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_INFO,
                    "%s::readConfig Global call pick-up code is '%s'",
                    mLogName.data(), mGlobalPickUpCode.data());
      mRedirectorActive = OS_SUCCESS;
   }

   // Fetch the call retrieve prefix from the config file.
   // If it is null, it doesn't count.
   UtlString callRetrieveCode;
   if ((configDb.get(CONFIG_SETTING_RETRIEVE_CODE, callRetrieveCode) !=
        OS_SUCCESS) ||
       callRetrieveCode.isNull())
   {
      OsSysLog::add(FAC_SIP, PRI_INFO,
                    "%s::readConfig No call retrieve code specified",
                    mLogName.data());
   }
   else
   {
      // Check that an orbit description file exists.
      UtlString orbitFileName;
      if ((configDb.get(CONFIG_SETTING_ORBIT_FILENAME, orbitFileName) !=
           OS_SUCCESS) ||
          orbitFileName.length() == 0)
      {
         OsSysLog::add(FAC_SIP, PRI_INFO,
                       "%s::readConfig No orbit file name specified",
                       mLogName.data());
      }
      else
      {
         if (
            // Get the park server's SIP domain so we can forward its
            // addresses to it.
            (configDb.get(CONFIG_SETTING_PARK_SERVER, mParkServerDomain) !=
             OS_SUCCESS) ||
            mParkServerDomain.isNull())
         {
            OsSysLog::add(FAC_SIP, PRI_CRIT,
                          "%s::readConfig No park server address specified.",
                          mLogName.data());
         }
         else
         {
            mOrbitFileReader.setFileName(&orbitFileName);
            OsSysLog::add(FAC_SIP, PRI_INFO, "%s::readConfig "
                          "Call retrieve code is '%s', orbit file is '%s', "
                          "park server domain is '%s'",
                          mLogName.data(),
                          callRetrieveCode.data(), orbitFileName.data(),
                          mParkServerDomain.data());
            mRedirectorActive = OS_SUCCESS;
            // All needed information for call retrieval is present,
            // so set mCallRetrieveCode to activate it.
            mCallRetrieveCode = callRetrieveCode;
         }
      }
   }

   // Get the wait time for NOTIFYs in response to our SUBSCRIBEs.
   // Set the default value, to be overridden if the user specifies a valid
   // value.
   mWaitSecs = DEFAULT_WAIT_TIME_SECS;
   mWaitUSecs = DEFAULT_WAIT_TIME_USECS;
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "%s::readConfig Default wait time is %d.%06d",
                 mLogName.data(), mWaitSecs, mWaitUSecs);
   // Fetch the parameter value.
   UtlString waitUS;
   float waitf;
   if (configDb.get(CONFIG_SETTING_WAIT, waitUS) == OS_SUCCESS)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "%s::readConfig " CONFIG_SETTING_WAIT " is '%s'",
                    mLogName.data(), waitUS.data());
      // Parse the value, checking for errors.
      unsigned int char_count;
      sscanf(waitUS.data(), " %f %n", &waitf, &char_count);
      if (char_count != waitUS.length())
      {
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "%s::readConfig Invalid format for "
                       CONFIG_SETTING_WAIT " '%s'",
                       mLogName.data(), waitUS.data());
      }
      else if (
         // Check that the value is in range.
         !(waitf >= MIN_WAIT_TIME && waitf <= MAX_WAIT_TIME))
      {
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "%s::readConfig " CONFIG_SETTING_WAIT
                       " (%f) outside allowed range (%f to %f)",
                       mLogName.data(), waitf, MIN_WAIT_TIME, MAX_WAIT_TIME);
      }
      else
      {
         // Extract the seconds and microseconds, being careful to round
         // because the conversion from character data may have
         // been inexact.
         // Since waitf <= 100, usecs <= 100,000,000.
         int usecs = (int)((waitf * 1000000) + 0.0000005);
         mWaitSecs = usecs / 1000000;
         mWaitUSecs = usecs % 1000000;
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "%s::readConfig Wait time is %d.%06d",
                       mLogName.data(), mWaitSecs, mWaitUSecs);
      }
   }
}

// Initializer
OsStatus
SipRedirectorPickUp::initialize(OsConfigDb& configDb,
                                int redirectorNo,
                                const UtlString& localDomainHost)
{
   // If any of the pick-up redirections are active, set up the machinery
   // to execute them.
   if (mRedirectorActive == OS_SUCCESS)
   {
      // Get and save our domain name.
      mDomain = localDomainHost;

      UtlString bindIp;
      if (configDb.get(CONFIG_SETTING_BIND_IP, bindIp) != OS_SUCCESS ||
            !OsSocket::isIp4Address(bindIp))
      {
         bindIp = "0.0.0.0";
      }

      // Authentication Realm Name
      UtlString realm;
      configDb.get("SIP_REGISTRAR_AUTHENTICATE_REALM", realm);
      // Get SipLineMgr containing the credentials for REGISTRAR_ID_TOKEN.
      SipLineMgr* lineMgr = addCredentials(mDomain, realm);

      // Create a SIP user agent to generate SUBSCRIBEs and receive NOTIFYs,
      // and save a pointer to it.
      // Having a separate user agent ensures that the NOTIFYs are not
      // processed for redirection, but rather we can act as a UAS to
      // process them.
      mpSipUserAgent = new SipUserAgent(
         // Let the system choose the port numbers.
         PORT_DEFAULT, // sipTcpPort
         PORT_DEFAULT, // sipUdpPort
         PORT_DEFAULT, // sipTlsPort
         NULL, // publicAddress
         NULL, // defaultUser
         bindIp, // defaultSipAddress
         NULL, // sipProxyServers
         NULL, // sipDirectoryServers
         NULL, // sipRegistryServers
         NULL, // authenticationScheme
         NULL, // authenicateRealm
         NULL, // authenticateDb
         NULL, // authorizeUserIds
         NULL, // authorizePasswords
         lineMgr, // lineMgr
         SIP_DEFAULT_RTT, // sipFirstResendTimeout
         TRUE, // defaultToUaTransactions
         -1, // readBufferSize
         OsServerTask::DEF_MAX_MSGS, // queueSize
         FALSE // bUseNextAvailablePort
         );
      mpSipUserAgent->setUserAgentHeaderProperty("sipXecs/redirectorPickup");
      mpSipUserAgent->start();

      // Initialize the CSeq counter to an arbitrary acceptable value.
      mCSeq = 4711;

      // Create and start the task to receive NOTIFYs.
      mTask = new SipRedirectorPickUpTask(mpSipUserAgent, redirectorNo);
      mTask->start();
   }

   return mRedirectorActive;
}

// Finalizer
void
SipRedirectorPickUp::finalize()
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "%s::finalize entered", mLogName.data());

   // Close down the SipUserAgent.
   if (mpSipUserAgent)
   {
      // Do not block, as any incomplete work is useless anyway.
      mpSipUserAgent->shutdown(FALSE);
      delete mpSipUserAgent;
      mpSipUserAgent = NULL;
   }

   // Terminate the task to receive NOTIFYs.
   if (mTask)
   {
      mTask->requestShutdown();
      delete mTask;
      mTask = NULL;
   }
}

RedirectPlugin::LookUpStatus
SipRedirectorPickUp::lookUp(
   const SipMessage& message,
   const UtlString& requestString,
   const Url& requestUri,
   const UtlString& method,
   ContactList& contactList,
   RedirectPlugin::RequestSeqNo requestSeqNo,
   int redirectorNo,
   SipRedirectorPrivateStorage*& privateStorage,
   ErrorDescriptor& errorDescriptor)
{
   UtlString userId;
   bool bSupportsReplaces;
   UtlString incomingCallId;

   requestUri.getUserId(userId);
   message.getCallIdField(&incomingCallId);

   if (!mCallPickUpCode.isNull() &&
       !mCallRetrieveCode.isNull() &&
       userId.length() > mCallPickUpCode.length() &&
       userId.index(mCallPickUpCode.data()) == 0 )
   {
      // We have both a Call Pickup and Retrieve codes defined and the userid contains the
      // Pickup code at the beginning.  Check if the remainder of the userid is a park orbit.
      // If it is, replace the Pickup code with the Retrieve code in the userid.
      // Extract the orbit number.
      UtlString orbit(userId.data() + mCallPickUpCode.length());
      if (mOrbitFileReader.findInOrbitList(orbit) != NULL)
      {
         // userid contains a park orbit.  replace the pickup code with the retrieve code.
         userId.replace(0, mCallPickUpCode.length(), mCallRetrieveCode);
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "SipRedirectorPickup::lookUp replace Pickup for Retrieve = %s and requestString = %s",
                       userId.data(), requestString.data());
      }
   }

   if (!mCallPickUpCode.isNull() &&
       userId.length() > mCallPickUpCode.length() &&
       userId.index(mCallPickUpCode.data()) == 0 &&
       userId.compareTo(mExcludedUser1) != 0 &&
       userId.compareTo(mExcludedUser2) != 0)
   {
      // Check if directed call pick-up is active, and this is a
      // request for directed call pick-up.
      // Because the default directed pick-up feature code is "*78" and
      // the default global pick-up feature code is "*78*", we can't just
      // match all strings with the directed pick-up feature code as a
      // prefix, we also require that the suffix not be "*" or "#".
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipRedirectorPickup::lookUp callpickupcode is present, userId = '%s'",
                       userId.data());
      return lookUpDialog(requestString,
                          incomingCallId,
                          contactList,
                          requestSeqNo,
                          redirectorNo,
                          privateStorage,
                          // The suffix of the request URI after the
                          // directed call pick-up code.
                          userId.data() + mCallPickUpCode.length(),
                          // Only examine early dialogs.
                          stateEarly);
   }
   else if (!mGlobalPickUpCode.isNull() &&
            userId.compareTo(mGlobalPickUpCode) == 0)
   {
      // Process the global call pick-up code.
      return lookUpDialog(requestString,
                          incomingCallId,
                          contactList,
                          requestSeqNo,
                          redirectorNo,
                          privateStorage,
                          // The all-exetnsions user.
                          ALL_CREDENTIALS_USER,
                          // Only examine early dialogs.
                          stateEarly);
   }
   else if (!mGlobalPickUpCode.isNull() &&
            userId.compareTo(ALL_CREDENTIALS_USER) == 0)
   {
      // Process the "~~sp~allcredentials" user for global call pick-up.
      if (method.compareTo("SUBSCRIBE", UtlString::ignoreCase) == 0)
      {
         // Only the SUBSCRIBE method is acceptable for
         // ~~sp~allcredentials, to prevent "INVITE ~~sp~allcredentials@..."
         // from ringing every phone!
         ResultSet credentials;
         CredentialDB::getInstance()->getAllRows(credentials);

         // Loop through the result set, looking at each credentials
         // entry in turn.
         int numGlobalPickUpContacts = credentials.getSize();
         for (int i = 0; i < numGlobalPickUpContacts; i++)
         {
            static UtlString uriKey("uri");

            UtlHashMap record;
            if (credentials.getIndex(i, record))
            {
               // Extract the "uri" element from the credential.
               UtlString contactStr;
               UtlString uri = *((UtlString*)record.findValue(&uriKey));
               Url contactUri(uri);

               // Add the contact to the contact list.
               contactList.add( contactUri, *this );
            }
         }
         return RedirectPlugin::SUCCESS;
      }
      else
      {
         UtlString reasonPhrase;
         reasonPhrase = "Subscribe Method Not Allowed For address ";
         reasonPhrase.append( ALL_CREDENTIALS_USER );
         errorDescriptor.setStatusLineData( SIP_BAD_METHOD_CODE, reasonPhrase );
         errorDescriptor.setAllowFieldValue( SIP_SUBSCRIBE_METHOD );
         return RedirectPlugin::ERROR;
      }
   }
   else if (!mCallRetrieveCode.isNull() &&
            mOrbitFileReader.findInOrbitList(userId) != NULL)
   {
      // Check if call retrieve is active, and this is a request for
      // an extension that is an orbit number.

      // Add the contact to the contact list.
      UtlString contactStr = "sip:" + userId + "@" + mParkServerDomain;
      Url contactUri(contactStr, Url::AddrSpec);
      contactUri.setUrlParameter(SIP_SIPX_CALL_DEST_FIELD, "PARK");
      contactList.add( contactUri, *this );

      return RedirectPlugin::SUCCESS;
   }
   else if (!mCallRetrieveCode.isNull() &&
            userId.length() > mCallRetrieveCode.length() &&
            userId.index(mCallRetrieveCode.data()) == 0)
   {
      // Check if call retrieve is active, and this is a request for
      // call retrieve.
      // Test for supports: replaces
      // sipXtapi now sends "Supported: replaces", so we can test for its
      // presence and act on it, without causing a calling sipXtapi to fail.
      bSupportsReplaces = message.isInSupportedField("replaces");

      // Extract the putative orbit number.
      UtlString orbit(userId.data() + mCallRetrieveCode.length());

      if (bSupportsReplaces)
      {
         // Look it up in the orbit list.
         if (mOrbitFileReader.findInOrbitList(orbit) != NULL)
         {

           UtlString  retrieveRequestString(requestString);
           ssize_t    pickupCodeIndex;

           // If the requestString contains the call pickup code, replace it with call retrieve code.  Used for one button
           // parked call pickup.
           pickupCodeIndex = retrieveRequestString.index(mCallPickUpCode.data());
           if ( pickupCodeIndex != UTL_NOT_FOUND )
           {
             retrieveRequestString.replace( pickupCodeIndex, mCallPickUpCode.length(), mCallRetrieveCode);
           }
            return lookUpDialog(retrieveRequestString,

                                incomingCallId,
                                contactList,
                                requestSeqNo,
                                redirectorNo,
                                privateStorage,
                                // The orbit number.
                                orbit.data(),
                                // Only examine confirmed dialogs.
                                stateConfirmed);
         }
         else
         {
            // It appears to be a call retrieve, but the orbit number is invalid.
            // Return ERROR.
            UtlString reasonPhrase;
            reasonPhrase = "Park Orbit " + orbit + " Not Found";
            errorDescriptor.setStatusLineData( SIP_NOT_FOUND_CODE, reasonPhrase );
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "%s::lookUp Invalid orbit number '%s'",
                          mLogName.data(), orbit.data());
            return RedirectPlugin::ERROR;
         }
      }
      else
      {
         // The park retrieve failed because the UA does not support INVITE/Replaces
         UtlString reasonPhrase;
         reasonPhrase = "Replaces Extension Required";
         errorDescriptor.setStatusLineData( SIP_EXTENSION_REQUIRED_CODE, reasonPhrase );
         errorDescriptor.setRequireFieldValue( SIP_REPLACES_EXTENSION );

         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "%s::lookUp Executor does not support INVITE/Replaces",
                       mLogName.data());
         return RedirectPlugin::ERROR;
      }
   }
   else
   {
      // We do not recognize the user, so we do nothing.
      return RedirectPlugin::SUCCESS;
   }
}

RedirectPlugin::LookUpStatus
SipRedirectorPickUp::lookUpDialog(
   const UtlString& requestString,
   const UtlString& incomingCallId,
   ContactList& contactList,
   RedirectPlugin::RequestSeqNo requestSeqNo,
   int redirectorNo,
   SipRedirectorPrivateStorage*& privateStorage,
   const char* subscribeUser,
   State stateFilter)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "%s::lookUpDialog requestString = '%s', "
                 "requestSeqNo = %d, redirectorNo = %d, privateStorage = %p, "
                 "subscribeUser = '%s', stateFilter = %d",
                 mLogName.data(),
                 requestString.data(), requestSeqNo, redirectorNo,
                 privateStorage, subscribeUser, stateFilter);

   // If the private storage is already allocated, then this is a
   // reprocessing cycle, and the dialog to pick up (if any) is
   // recorded in private storage.
   if (privateStorage)
   {
      // Cast the private storage pointer to the right type so we can
      // access the needed dialog information.
      SipRedirectorPrivateStoragePickUp* dialog_info =
         dynamic_cast<SipRedirectorPrivateStoragePickUp*> (privateStorage);

      if (dialog_info->mTargetDialogDuration !=
          SipRedirectorPrivateStoragePickUp::TargetDialogDurationAbsent)
      {
         // For call pickup execute the original 302 with a Replaces parameter
         if (dialog_info->mStateFilter == stateEarly)
         {
            // A dialog has been recorded.  Construct a contact for it.
            // Beware that as recorded in the dialog event notice, the
            // target URI is in addr-spec format; any parameters are URI
            // parameters.  (Field parameters have been broken out in
            // param elements.)
            Url contact_URI(dialog_info->mTargetDialogRemoteURI, TRUE);

            // Construct the Replaces: header value the caller should use.
            UtlString header_value(dialog_info->mTargetDialogCallId);
            // Note that according to RFC 3891, the to-tag parameter is
            // the local tag at the destination of the INVITE/Replaces.
            // But the INVITE/Replaces goes to the other end of the call from
            // the one we queried with SUBSCRIBE, so the to-tag in the
            // Replaces: header is the *remote* tag from the NOTIFY.
            header_value.append(";to-tag=");
            header_value.append(dialog_info->mTargetDialogRemoteTag);
            header_value.append(";from-tag=");
            header_value.append(dialog_info->mTargetDialogLocalTag);
            // If the state filtering is "early", add "early-only", so we
            // don't pick up a call that has just been answered.
            if (dialog_info->mStateFilter == stateEarly &&
                !mNoEarlyOnly)
            {
               header_value.append(";early-only");
            }
            // Add a header parameter to specify the Replaces: header.
            contact_URI.setHeaderParameter("Replaces", header_value.data());

            // We add a header parameter to cause the redirection to
            // include a "Require: replaces" header.  Then if the caller
            // phone does not support INVITE/Replaces:, the pick-up will
            // fail entirely.  Without it, if the caller phone does not
            // support INVITE/Replaces, the caller will get a
            // simultaneous incoming call from the executing phone.
            // Previously, we thought the latter behavior was better, but
            // it is not -- Consider if the device is a gateway from the
            // PSTN.  Then the INVITE/Replaces will generate an outgoing
            // call to the calling phone.
            contact_URI.setHeaderParameter(SIP_REQUIRE_FIELD,
                                           SIP_REPLACES_EXTENSION);

            contact_URI.setUrlParameter(SIP_SIPX_CALL_DEST_FIELD, "DPUP");
            // Record the URI as a contact.
            contactList.add( contact_URI, *this );

            // If "reversed Replaces" is configured, also add a Replaces: with
            // the to-tag and from-tag reversed.
            if (mReversedReplaces)
            {
               Url c(dialog_info->mTargetDialogRemoteURI);

               UtlString header_value(dialog_info->mTargetDialogCallId);
               header_value.append(";to-tag=");
               header_value.append(dialog_info->mTargetDialogLocalTag);
               header_value.append(";from-tag=");
               header_value.append(dialog_info->mTargetDialogRemoteTag);
               if (dialog_info->mStateFilter == stateEarly &&
                   !mNoEarlyOnly)
               {
                  header_value.append(";early-only");
               }

               c.setHeaderParameter("Replaces", header_value.data());
               c.setHeaderParameter(SIP_REQUIRE_FIELD,
                                    SIP_REPLACES_EXTENSION);
               // Put this contact at a lower priority than the one in
               // the correct order.
               c.setFieldParameter("q", "0.9");

               contactList.add( c,  *this );
            }
         }
      }

      // We do not need to suspend this time.
      return RedirectPlugin::SUCCESS;
   }
   else
   {
      UtlString userId;
      Url requestUri(requestString);
      requestUri.getUserId(userId);
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "%s::lookUpDialog userId '%s'",
                    mLogName.data(), userId.data());
      // Test to see if this is a call retrieval
      if (!mCallRetrieveCode.isNull() &&
          userId.length() > mCallRetrieveCode.length() &&
          userId.index(mCallRetrieveCode.data()) == 0)
      {
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "%s::lookUpDialog doing call retrieval",
                       mLogName.data());

         // Construct the contact address for the call retrieval.
         UtlString contactString("sip:");
         // The user of the request URI is our subscribeUser parameter.
         contactString.append(subscribeUser);
         contactString.append("@");
         contactString.append(mParkServerDomain);

         Url contact_URI(contactString, Url::AddrSpec);

         contact_URI.setUrlParameter("operation", "retrieve");
         contact_URI.setUrlParameter(SIP_SIPX_CALL_DEST_FIELD, "RPARK");

         contactList.add( contact_URI, *this );
         // We do not need to suspend this time.*/
         return RedirectPlugin::SUCCESS;
      }
      else
      {
         // Construct the SUBSCRIBE for the call pickup.
         SipMessage subscribe;
         UtlString subscribeRequestUri("sip:");
         // The user of the request URI is our subscribeUser parameter.
         subscribeRequestUri.append(subscribeUser);
         subscribeRequestUri.append("@");
         subscribeRequestUri.append(mDomain);

         // Construct a Call-Id for the SUBSCRIBE.
         UtlString callId;
         CallId::getNewCallId(callId);

         // Construct the From: value.
         UtlString fromUri;
         {
            // Get the local address and port.
            UtlString address;
            int port;
            mpSipUserAgent->getLocalAddress(&address, &port);
            // Use the first 8 chars of the MD5 of the Call-Id as the from-tag.
            NetMd5Codec encoder;
            UtlString tag;
            encoder.encode(callId.data(), tag);
            tag.remove(8);
            // Assemble the URI.
            SipMessage::buildSipUri(&fromUri,
                                    address.data(),
                                    port,
                                    NULL, // protocol
                                    NULL, // user
                                    NULL, // userLabel,
                                    tag.data());
         }

         // Set the standard request headers.
         // Allow the SipUserAgent to fill in Contact:.
         subscribe.setRequestData(
            SIP_SUBSCRIBE_METHOD,
            subscribeRequestUri.data(), // request URI
            fromUri, // From:
            subscribeRequestUri.data(), // To:
            callId,
            mCSeq);
         // Increment CSeq and roll it over if necessary.
         mCSeq++;
         mCSeq &= 0x0FFFFFFF;
         // Set the Expires header.
         // If "1 second subscriptions" is set (needed for some versions
         // of Snom phones), use a 1-second subscription.  Otherwise, use
         // a 0-second subscription, so we get just one NOTIFY.
         subscribe.setExpiresField(mOneSecondSubscription ? 1 : 0);
         // Set the "Event: dialog" header.
         subscribe.setEventField("dialog");
         // Set the "Accept: application/dialog-info+xml" header.
         // Not strictly necessary (per the I-D), but it makes the SUBSCRIBE
         // more strictly compliant.
         subscribe.setHeaderValue(SIP_ACCEPT_FIELD,
	                          DIALOG_EVENT_CONTENT_TYPE);
         // Set the References header for tracing dialog associations.
         {
            UtlString referencesValue(incomingCallId);
            referencesValue.append(";rel=inquiry");
            subscribe.setHeaderValue(SIP_REFERENCES_FIELD,
                                     referencesValue);
         }

         // Send the SUBSCRIBE.
         mpSipUserAgent->send(subscribe);

         // Allocate private storage.
         SipRedirectorPrivateStoragePickUp *storage =
            new SipRedirectorPrivateStoragePickUp(requestSeqNo,
                                                  redirectorNo);
         privateStorage = storage;

         // Record the Call-Id of the SUBSCRIBE, so we can correlated the
         // NOTIFYs with it.
         storage->mSubscribeCallId = callId;
         // Record the state filtering criterion.
         storage->mStateFilter = stateFilter;

         // If we are printing debug messages, record when the SUBSCRIBE
         // was sent, so we can report how long it took to get the NOTIFYs.
         if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
         {
            OsDateTime::getCurTime(storage->mSubscribeSendTime);
         }

         // Set the timer to resume.
         storage->mTimer.oneshotAfter(OsTime(mWaitSecs, mWaitUSecs));

         // Suspend processing the request.
         return RedirectPlugin::SEARCH_PENDING;
      }
   }
}


SipRedirectorPrivateStoragePickUp::SipRedirectorPrivateStoragePickUp(
   RedirectPlugin::RequestSeqNo requestSeqNo,
   int redirectorNo) :
   mNotification(requestSeqNo, redirectorNo),
   mTimer(mNotification),
   // Set special value to show no dialog has been recorded.
   mTargetDialogDuration(TargetDialogDurationAbsent)
{
}

SipRedirectorPrivateStoragePickUp::~SipRedirectorPrivateStoragePickUp()
{
}

const char* const
SipRedirectorPrivateStoragePickUp::getContainableType() const
{
   return SipRedirectorPrivateStoragePickUp::TYPE;
}

void SipRedirectorPrivateStoragePickUp::processNotify(const char* body)
{
   // Initialize Tiny XML document object.
   TiXmlDocument document;
   TiXmlNode* dialog_info;
   if (
      // Load the XML into it.
      document.Parse(body) &&
      // Find the top element, which should be a <dialog-info>.
      (dialog_info = document.FirstChild("dialog-info")) != NULL &&
      dialog_info->Type() == TiXmlNode::ELEMENT)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipRedirectorPrivateStoragePickUp::processNotify "
                    "Body parsed, <dialog-info> found");
      // Find all the <dialog> elements.
      for (TiXmlNode* dialog = 0;
           (dialog = dialog_info->IterateChildren("dialog", dialog)); )
      {
         // Process each <dialog> element.
         processNotifyDialogElement(dialog->ToElement());
      }
   }
   else
   {
      // Report error.
      OsSysLog::add(FAC_SIP, PRI_ERR,
                    "SipRedirectorPrivateStoragePickUp::processNotify "
                    "NOTIFY body invalid: '%s'", body);
   }
}

void SipRedirectorPrivateStoragePickUp::processNotifyDialogElement(
   TiXmlElement* dialog)
{
   // Variables to record the data items that we find in the dialog.
   const char* call_id = NULL;
   const char* local_tag = NULL;
   const char* remote_tag = NULL;
   const char* incoming_string;
   UtlBoolean incoming = FALSE;
   // Duration defaults to 0 if none is given.
   int duration = 0;
   SipRedirectorPickUp::State state = SipRedirectorPickUp::stateUnknown;
   UtlString local_identity;
   UtlString local_target;
   UtlString remote_identity;
   UtlString remote_target;

   // Get attribute values.
   call_id = dialog->Attribute("call-id");
   local_tag = dialog->Attribute("local-tag");
   remote_tag = dialog->Attribute("remote-tag");
   incoming_string = dialog->Attribute("direction");
   // Beware that the direction attribute might be missing.
   UtlString incomingString(incoming_string);
   incoming =
      incoming_string != NULL &&
      incomingString.compareTo("recipient", UtlString::ignoreCase) == 0;

   // Get values from children.
   for (TiXmlNode* child = dialog->FirstChild(); child;
        child = child->NextSibling())
   {
      if (child->Type() == TiXmlNode::ELEMENT &&
          strcmp(child->Value(), "duration") == 0)
      {
         // duration element
         // Get the content and convert to an integer.
         UtlString duration_s;
         textContentShallow(duration_s, child->ToElement());
         const char* startptr = duration_s.data();
         char* endptr = NULL;
         long int temp = strtol(startptr, &endptr, 10);
         // If the duration value passes sanity checks, use it.
         if (((const char*) endptr) == startptr + duration_s.length() &&
             temp >= 0 &&
             temp <= INT_MAX)
         {
            duration = temp;
         }
         else
         {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                          "SipRedirectorPrivateStoragePickUp::"
                          "processNotifyDialogElement "
                          "Invalid <duration> '%s'",
                          duration_s.data());
         }
      }
      else if (child->Type() == TiXmlNode::ELEMENT &&
          strcmp(child->Value(), "state") == 0)
      {
         // state element
         UtlString state_string;
         textContentShallow(state_string, child->ToElement());
         if (state_string.compareTo("early", UtlString::ignoreCase) == 0)
         {
            state = SipRedirectorPickUp::stateEarly;
         }
         else if (state_string.compareTo("confirmed",
                                         UtlString::ignoreCase) == 0)
         {
            state = SipRedirectorPickUp::stateConfirmed;
         }
         else
         {
            /* Other values leave 'state' set to stateUnknown. */ ;
         }
      }
      else if (child->Type() == TiXmlNode::ELEMENT &&
          strcmp(child->Value(), "local") == 0)
      {
         // local element
         processNotifyLocalRemoteElement(child->ToElement(),
                                         local_identity, local_target);
      }
      else if (child->Type() == TiXmlNode::ELEMENT &&
          strcmp(child->Value(), "remote") == 0)
      {
         // remote element
         processNotifyLocalRemoteElement(child->ToElement(),
                                         remote_identity, remote_target);
      }
      else
      {
         /* Ignore unknown elements. */ ;
      }
   }
   // Report all the information we have on the dialog.
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipRedirectorPrivateStoragePickUp::"
                 "processNotifyDialogElement "
                 "Dialog values: call_id = '%s', "
                 "local_tag = '%s', remote_tag = '%s', "
                 "incoming = %d, duration = %d, state = %d, "
                 "local_identity = '%s', local_target = '%s', "
                 "remote_identity = '%s', remote_target = '%s'",
                 call_id ? call_id : "[NULL]",
                 local_tag ? local_tag : "[NULL]",
                 remote_tag ? remote_tag : "[NULL]",
                 incoming,
                 duration,
                 state,
                 local_identity.data() ? local_identity.data() : "[NULL]",
                 local_target.data() ? local_target.data() : "[NULL]",
                 remote_identity.data() ? remote_identity.data() : "[NULL]",
                 remote_target.data() ? remote_target.data() : "[NULL]");

   // Check whether the element has enough information to be usable, and
   // ignore it if not.
   if (!(call_id && call_id[0] != '\0' &&
         local_tag && local_tag[0] != '\0' &&
         remote_tag && remote_tag[0] != '\0' &&
         // Must have at least one remote URI, so we can contact the caller.
         (!remote_identity.isNull() || !remote_target.isNull())))
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipRedirectorPrivateStoragePickUp::"
                    "processNotifyDialogElement "
                    "Dialog element unusable");
      return;
   }

   // Check whether this is a dialog that qualifies for our consideration:
   //   direction is incoming, and
   //   state matches mStateFilter, and
   //   its duration is > the duration of the currently recorded dialog
   if (!(incoming &&
         // Currently, for the state to match, it must be the same as
         // mStateFilter, or mStateFilter is stateDontCare.
         (mStateFilter == SipRedirectorPickUp::stateDontCare ||
          state == mStateFilter) &&
         duration > mTargetDialogDuration))
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipRedirectorPrivateStoragePickUp::"
                    "processNotifyDialogElement "
                    "Dialog does not qualify");
      return;
   }

   // Save information about this dialog.
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipRedirectorPrivateStoragePickUp::"
                 "processNotifyDialogElement "
                 "Dialog element saved");
   // Note that the members here that are strings are UtlStrings, so
   // these assignments copy the contents of these char*'s into the
   // UtlStrings, and thus will save the values when we discard the TiXml
   // document tree.
   mTargetDialogCallId = call_id;
   mTargetDialogLocalTag = local_tag;
   mTargetDialogRemoteTag = remote_tag;
   mTargetDialogDuration = duration;
   // Record the URI at which to contact the calling phone.
   // Use the remote target if it exists, otherwise the remote identity.
   mTargetDialogRemoteURI =
      !remote_target.isNull() ? remote_target : remote_identity;
   mTargetDialogLocalURI =
      !local_target.isNull() ? local_target : local_identity;
   mTargetDialogLocalIdentity = local_identity;
   mTargetDialogRemoteIdentity = remote_identity;
}

void SipRedirectorPrivateStoragePickUp::processNotifyLocalRemoteElement(
   TiXmlElement* element,
   UtlString& identity,
   UtlString& target)
{
   // Get values from children.
   for (TiXmlNode* child = element->FirstChild(); child;
        child = child->NextSibling())
   {
      if (child->Type() == TiXmlNode::ELEMENT &&
          strcmp(child->Value(), "target") == 0)
      {
         // target element
         const char* target_string = child->ToElement()->Attribute("uri");
         target = target_string != NULL ? target_string : "";
         // Note that we can ignore the <param> children of <target>, because
         // they carry field-parameters from the Contact: header, not
         // URI-parameters.
      }
      else if (child->Type() == TiXmlNode::ELEMENT &&
               strcmp(child->Value(), "identity") == 0)
      {
         // identity element
         textContentShallow(identity, child->ToElement());
      }
   }
}

SipRedirectorPickUpNotification::SipRedirectorPickUpNotification(
   RedirectPlugin::RequestSeqNo requestSeqNo,
   int redirectorNo) :
   mRequestSeqNo(requestSeqNo),
   mRedirectorNo(redirectorNo)
{
}

OsStatus SipRedirectorPickUpNotification::signal(const intptr_t eventData)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipRedirectorPickUpNotification::signal "
                 "Fired mRequestSeqNo %d, mRedirectorNo %d",
                 mRequestSeqNo, mRedirectorNo);
   SipRedirectServer::getInstance()->
      resumeRequest(mRequestSeqNo, mRedirectorNo);
   return OS_SUCCESS;
}

SipRedirectorPickUpTask::SipRedirectorPickUpTask(SipUserAgent* pSipUserAgent,
                                                 int redirectorNo) :
   OsServerTask("SipRedirectorPickUpTask-%d"),
   mpSipUserAgent(pSipUserAgent),
   mRedirectorNo(redirectorNo)
{
   // Set up listening for NOTIFYs.
   pSipUserAgent->addMessageObserver(*(this->getMessageQueue()),
                                     SIP_NOTIFY_METHOD,
                                     TRUE, // want to get requests
                                     FALSE, // no responses
                                     TRUE, // Incoming messages
                                     FALSE); // No outgoing messages
}

SipRedirectorPickUpTask::~SipRedirectorPickUpTask()
{
   // we don't need to remove the observer here; the SipUserAgent cleans it up
}

UtlBoolean
SipRedirectorPickUpTask::handleMessage(OsMsg& eventMessage)
{
   // Only flag SIP messages as handled, so that OS_SHUTDOWN and anything else
   // is passed to OsServerTask::handleMessage.
   // Note that we only provide responses to NOTIFY requests, others are simply
   // absorbed.
   UtlBoolean handled = FALSE;

   int msgType = eventMessage.getMsgType();

   switch (msgType)
   {
   case OsMsg::PHONE_APP:
   {
      // Get a pointer to the message.
      const SipMessage* message =
         ((SipMessageEvent&)eventMessage).getMessage();

      // If it is a request, check its method.  If it is a response, absorb it.
      if (!message->isResponse())
      {
         // Extract the request method.
         UtlString method;
         message->getRequestMethod(&method);

         if (method.compareTo(SIP_NOTIFY_METHOD, UtlString::ignoreCase) == 0)
         {
            // Get the Call-Id.
            UtlString callId;
            message->getCallIdField(&callId);
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipRedirectorPickUpTask::handleMessage "
                          "Start processing NOTIFY CallID '%s'", callId.data());

            {
               // This block holds SipRedirectServer::mRedirectorMutex.
               OsLock lock(SipRedirectServer::getInstance()->mRedirectorMutex);

               // Look for a suspended request whose SUBSCRIBE had this Call-Id.
               SipRedirectServerPrivateStorageIterator itor(mRedirectorNo);
               // Fetch a pointer to each element of myContentSource into
               // pStorage.
               SipRedirectorPrivateStoragePickUp* pStorage;
               while ((pStorage =
                       dynamic_cast<SipRedirectorPrivateStoragePickUp*> (itor())))
               {
                  // Does this request have the same Call-Id?
                  if (callId.compareTo(pStorage->mSubscribeCallId) == 0)
                  {
                     // This is the request to which this NOTIFY is a response.
                     // Process the NOTIFY and store its information in
                     // *pStorage.
                     const char* body;
                     ssize_t length;
                     // Be careful getting the body, as any of the pointers
                     // may be null.
                     const HttpBody* http_body;
                     if (!(http_body = message->getBody()))
                     {
                        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                      "SipRedirectorPickUpTask::handleMessage "
                                      "getBody returns NULL, ignoring");
                     }
                     else if (http_body->getBytes(&body, &length),
                              !(body && length > 0))
                     {
                        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                      "SipRedirectorPickUpTask::handleMessage "
                                      "getBytes returns no body, ignoring");
                     }
                     else
                     {
                        if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
                        {
                           // Calculate the response delay.
                           OsTime now;
                           OsDateTime::getCurTime(now);
                           OsTime delta;
                           delta = now - (pStorage->mSubscribeSendTime);

                           OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                         "SipRedirectorPickUpTask::handleMessage "
                                         "NOTIFY for request %d, delay %d.%06d, "
                                         "body '%s'",
                                         itor.requestSeqNo(),
                                         (int) delta.seconds(),
                                         (int) delta.usecs(),
                                         body);
                        }
                        // Parse this NOTICE and store the needed
                        // information in private storage.
                        pStorage->processNotify(body);
                     }

                     // Don't bother checking for a match with any other request.
                     break;
                  }
               }
            }

            // Return a 200 response.
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipRedirectorPickUpTask::handleMessage "
                          "Sending 200 OK response to NOTIFY");
            SipMessage response;
            response.setOkResponseData(message);
            mpSipUserAgent->setServerHeader(response);
            mpSipUserAgent->send(response);
         }
      }
   }

   handled = TRUE;
   break;
   }

   return handled;
}

const UtlString& SipRedirectorPickUp::name( void ) const
{
   return mLogName;
}


// Get and add the credentials for sipXregistrar
SipLineMgr*
SipRedirectorPickUp::addCredentials (UtlString domain, UtlString realm)
{
   SipLine* line = NULL;
   SipLineMgr* lineMgr = NULL;
   UtlString user;

   CredentialDB* credentialDb;
   if ((credentialDb = CredentialDB::getInstance()))
   {
      Url identity;

      identity.setUserId(REGISTRAR_ID_TOKEN);
      identity.setHostAddress(domain);
      UtlString ha1_authenticator;
      UtlString authtype;
      bool bSuccess = false;

      if (credentialDb->getCredential(identity, realm, user, ha1_authenticator, authtype))
      {
         if ((line = new SipLine( identity // user entered url
                                 ,identity // identity url
                                 ,user     // user
                                 ,TRUE     // visible
                                 ,SipLine::LINE_STATE_PROVISIONED
                                 ,TRUE     // auto enable
                                 ,FALSE    // use call handling
                                 )))
         {
            if ((lineMgr = new SipLineMgr()))
            {
               if (lineMgr->addLine(*line))
               {
                  if (lineMgr->addCredentialForLine( identity, realm, user, ha1_authenticator
                                                    ,HTTP_DIGEST_AUTHENTICATION
                                                    )
                      )
                  {
                     lineMgr->setDefaultOutboundLine(identity);
                     bSuccess = true;

                     OsSysLog::add(FAC_SIP, PRI_INFO,
                                   "Added identity '%s': user='%s' realm='%s'"
                                   ,identity.toString().data(), user.data(), realm.data()
                                   );
                  }
                  else
                  {
                     OsSysLog::add(FAC_SIP, PRI_ERR,
                                   "Error adding identity '%s': user='%s' realm='%s'\n"
                                   "Call Pickup will not work!",
                                   identity.toString().data(), user.data(), realm.data()
                                   );
                  }
               }
               else
               {
                  OsSysLog::add(FAC_SIP, PRI_ERR, "addLine failed. Call Pickup will not work!" );
               }
            }
            else
            {
               OsSysLog::add(FAC_SIP, PRI_ERR,
                             "Constructing SipLineMgr failed. Call Pickup will not work!" );
            }
         }
         else
         {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                          "Constructing SipLine failed. Call Pickup will not work!" );
         }
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "No credential found for '%s' in realm '%s'. "
                       "Call Pickup will not work!"
                       ,identity.toString().data(), realm.data()
                       );
      }

      if( !bSuccess )
      {
         delete line;
         line = NULL;

         delete lineMgr;
         lineMgr = NULL;
      }
   }

   credentialDb->releaseInstance();

   return lineMgr;
}
