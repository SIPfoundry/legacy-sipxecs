#define GRUU_WORKAROUND
//
//
// Copyright (C) 2007, 2010 Avaya, Inc., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdlib.h>
#include <string.h>

// APPLICATION INCLUDES
#include "os/OsDateTime.h"
#include "os/OsQueuedEvent.h"
#include "os/OsTimer.h"
#include "os/OsEventMsg.h"
#include "utl/UtlRegex.h"
#include "utl/PluginHooks.h"
#include "utl/UtlSList.h"
#include "utl/UtlSListIterator.h"
#include "net/CallId.h"
#include "net/SipUserAgent.h"
#include "net/NetMd5Codec.h"
#include "net/NameValueTokenizer.h"
#include "sipdb/ResultSet.h"
#include "registry/SipRegistrar.h"
#include "SipRegistrarServer.h"
#include "RegisterEventServer.h"
#include "registry/RegisterPlugin.h"
#include "sipXecsService/SipXecsService.h"

#include <assert.h>

// DEFINES

/*
 * GRUUs are constructed by hashing the AOR, the IID, and the primary
 * SIP domain.  The SIP domain is included so that GRUUs constructed by
 * different systems will be different, but that any registrars that
 * form a redundant set will generate the same GRUU for an AOR/IID pair.
 * (Strictly, we use the entire value of the +sip.instance field parameter
 * on the Contact: header, which should be <...> around the IID, but there
 * is no point enforcing or parsing that rule.)
 */

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// syntax for a valid q parameter value
const RegEx RegQValue("^(0(\\.\\d{0,3})?|1(\\.0{0,3})?)$");
// syntax of a +sip.instance value containing a URN based on a UUID
// based on a MAC/EUI address
// The format of the URN/UUID is detailed in RFC 4122.  Two positions
// have limited possibilities in order to specify format variant 01
// and version 0001.
// $1 matches the MAC/EUI address.
const RegEx InstanceUrnUuidMac(
   "^(?i:<urn:uuid:"
   "[0-9a-f]{8}-"
   "[0-9a-f]{4}-"
   "1[0-9a-f]{3}-"
   "[89ab][0-9a-f]{3}-"
   "([0-9a-f]{12})>)$"
   );

#define MAX_RETENTION_TIME 600
#define HARD_MINIMUM_EXPIRATION 60
#define MIN_EXPIRES_TIME_NORMAL  300
#define MAX_EXPIRES_TIME_NORMAL 7200
#define MIN_EXPIRES_TIME_NATED   180
#define MAX_EXPIRES_TIME_NATED   300

// STRUCTS
// TYPEDEFS

// FORWARD DECLARATIONS
// GLOBAL VARIABLES
// Static Initializers


#if 0
// This is no longer needed because all mongo operations are thread-safe
OsMutex         SipRegistrarServer::sLockMutex(OsMutex::Q_FIFO);
#endif

SipRegistrarServer::SipRegistrarServer(SipRegistrar& registrar) :
    OsServerTask("SipRegistrarServer", NULL, SIPUA_DEFAULT_SERVER_OSMSG_QUEUE_SIZE),
    mRegistrar(registrar),
    mIsStarted(FALSE),
    mSipUserAgent(NULL),
    mSendExpiresInResponse(TRUE),
    mSendAllContactsInResponse(FALSE),
    mNonceExpiration(5*60)
{
}

void
SipRegistrarServer::initialize(
   OsConfigDb*   pOsConfigDb,      ///< Configuration parameters
   SipUserAgent* pSipUserAgent)    ///< User Agent to use when sending responses
{
    mSipUserAgent = pSipUserAgent;

    UtlString localAddress;
    int localPort;
    mSipUserAgent->getLocalAddress(&localAddress, &localPort);
    if (!localAddress.isNull())
    {
        std::string serverId = localAddress.data();
        serverId += "/";
        serverId += "RegDB::_bindingsNameSpace";
        SipRegistrar::getInstance(NULL)->getRegDB()->setLocalAddress(serverId);
    }

    // Initialize the normal (non-NATed) minimum and maximum expiry values
    UtlString tempExpiresString;
    pOsConfigDb->get("SIP_REGISTRAR_MIN_EXPIRES_NORMAL", tempExpiresString);
    if ( tempExpiresString.isNull() )
    {
        mNormalExpiryIntervals.mMinExpiresTime = MIN_EXPIRES_TIME_NORMAL;
    }
    else
    {
        mNormalExpiryIntervals.mMinExpiresTime = atoi(tempExpiresString.data());
        if ( mNormalExpiryIntervals.mMinExpiresTime < HARD_MINIMUM_EXPIRATION )
        {
           Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                         "SipRegistrarServer "
                         "configured minimum for SIP_REGISTRAR_MIN_EXPIRES_NORMAL (%d) < hard minimum (%d); set to hard minimum",
                         mNormalExpiryIntervals.mMinExpiresTime, HARD_MINIMUM_EXPIRATION);
           mNormalExpiryIntervals.mMinExpiresTime = HARD_MINIMUM_EXPIRATION;
        }
    }

    tempExpiresString.remove(0);
    pOsConfigDb->get("SIP_REGISTRAR_MAX_EXPIRES_NORMAL", tempExpiresString);
    if ( tempExpiresString.isNull() )
    {
       mNormalExpiryIntervals.mMaxExpiresTime = MAX_EXPIRES_TIME_NORMAL;
    }
    else
    {
       mNormalExpiryIntervals.mMaxExpiresTime = atoi(tempExpiresString.data());
       if ( mNormalExpiryIntervals.mMaxExpiresTime < mNormalExpiryIntervals.mMinExpiresTime )
       {
          Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                        "SipRegistrarServer "
                        "configured maximum for SIP_REGISTRAR_MAX_EXPIRES_NORMAL (%d) < minimum (%d); set to minimum",
                        mNormalExpiryIntervals.mMaxExpiresTime, mNormalExpiryIntervals.mMinExpiresTime
                        );
          mNormalExpiryIntervals.mMaxExpiresTime = mNormalExpiryIntervals.mMinExpiresTime;
       }
    }

    // Initialize the NATed minimum and maximum expiry values
    tempExpiresString.remove(0);
    pOsConfigDb->get("SIP_REGISTRAR_MIN_EXPIRES_NATED", tempExpiresString);
    if ( tempExpiresString.isNull() )
    {
        mNatedExpiryIntervals.mMinExpiresTime = MIN_EXPIRES_TIME_NATED;
    }
    else
    {
        mNatedExpiryIntervals.mMinExpiresTime = atoi(tempExpiresString.data());
        if ( mNatedExpiryIntervals.mMinExpiresTime < HARD_MINIMUM_EXPIRATION )
        {
           Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                         "SipRegistrarServer "
                         "configured minimum for SIP_REGISTRAR_MIN_EXPIRES_NATED (%d) < hard minimum (%d); set to hard minimum",
                         mNatedExpiryIntervals.mMinExpiresTime, HARD_MINIMUM_EXPIRATION);
           mNatedExpiryIntervals.mMinExpiresTime = HARD_MINIMUM_EXPIRATION;
        }
    }

    tempExpiresString.remove(0);
    pOsConfigDb->get("SIP_REGISTRAR_MAX_EXPIRES_NATED", tempExpiresString);
    if ( tempExpiresString.isNull() )
    {
       mNatedExpiryIntervals.mMaxExpiresTime = MAX_EXPIRES_TIME_NATED;
    }
    else
    {
       mNatedExpiryIntervals.mMaxExpiresTime = atoi(tempExpiresString.data());
       if ( mNatedExpiryIntervals.mMaxExpiresTime < mNatedExpiryIntervals.mMinExpiresTime )
       {
          Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                        "SipRegistrarServer "
                        "configured maximum for SIP_REGISTRAR_MAX_EXPIRES_NATED (%d) < minimum (%d); set to minimum",
                        mNatedExpiryIntervals.mMaxExpiresTime, mNatedExpiryIntervals.mMinExpiresTime
                        );
          mNatedExpiryIntervals.mMaxExpiresTime = mNatedExpiryIntervals.mMinExpiresTime;
       }
    }

    // Authentication Realm Name
    pOsConfigDb->get("SIP_REGISTRAR_AUTHENTICATE_REALM", mRealm);


    // Authentication Scheme:  NONE | DIGEST
    UtlString authenticateScheme;
    pOsConfigDb->get("SIP_REGISTRAR_AUTHENTICATE_SCHEME", authenticateScheme);
    mUseCredentialDB = (authenticateScheme.compareTo("NONE" , UtlString::ignoreCase) != 0);


    UtlString hostAliases;
    OsConfigDb domainConfig;
    domainConfig.loadFromFile(SipXecsService::domainConfigPath());

    // get SIP_DOMAIN_ALIASES from domain-config
    domainConfig.get(SipXecsService::DomainDbKey::SIP_DOMAIN_ALIASES, hostAliases);

    if(!hostAliases.isNull())
    {
        Os::Logger::instance().log(FAC_SIP, PRI_INFO, "SIP_DOMAIN_ALIASES : %s",
                      hostAliases.data());
        mSipUserAgent->setHostAliases(hostAliases);
    }

    // Get the additional contact setting.
    pOsConfigDb->get("SIP_REGISTRAR_ADDITIONAL_CONTACT", mAdditionalContact);
    if (!mAdditionalContact.isNull())
    {
       // Validate mAdditionalContact:  parses as a name-addr, there is no
       // trailing stuff, and it has an Expires header-parameter composed
       // of digits with value >= 1.
       Url url;
       UtlString remainder, param;
       if (   url.fromString(mAdditionalContact, Url::NameAddr, &remainder)
           && remainder.isNull()
           && url.getFieldParameter("expires", param)
           && !param.isNull()
           && strspn(param.data(), "0123456789") == param.length()
           && atoi(param.data()) >= 1)
       {
          Os::Logger::instance().log(FAC_SIP, PRI_INFO,
                        "SipRegistrarServer::initialize adding SIP_REGISTRAR_ADDITIONAL_CONTACT: '%s'",
                        mAdditionalContact.data());
       }
       else
       {
          Os::Logger::instance().log(FAC_SIP, PRI_ERR,
                        "SipRegistrarServer::initialize Invalid value for SIP_REGISTRAR_ADDITIONAL_CONTACT: '%s', ignoring",
                        mAdditionalContact.data());
          // If value is invalid, make it null.
          mAdditionalContact.remove(0);
       }
    }

    // This is a developer-only configuration parameter
    // to prevent sending an Expires header in REGISTER responses
    mSendExpiresInResponse = pOsConfigDb->getBoolean("SIP_REGISTRAR_EXP_HDR_RSP", TRUE);
    Os::Logger::instance().log(FAC_SIP, PRI_INFO,
                  "SipRegistrarServer::initialize SIP_REGISTRAR_EXP_HDR_RSP is %s",
                  mSendExpiresInResponse ? "Enabled" : "Disabled");

    mSendAllContactsInResponse = pOsConfigDb->getBoolean("SIPX_SEND_ALL_CONTACTS", FALSE);
    Os::Logger::instance().log(FAC_SIP, PRI_INFO,
                  "SipRegistrarServer::initialize SIPX_SEND_ALL_CONTACTS is %s",
                  mSendAllContactsInResponse ? "Enabled" : "Disabled");

    /*
     * Unused Authentication Directives
     *
     * These directives are in the configuration files but are not used:
     *
     *   pOsConfigDb->get("SIP_REGISTRAR_AUTHENTICATE_ALGORITHM", authAlgorithm);
     *     there may someday be a reason to use that one, since MD5 is aging.
     *
     *   pOsConfigDb->get("SIP_REGISTRAR_AUTHENTICATE_QOP", authQop);
     *     the qop will probably never be used - removed from current config file
     */

    // Registration Plugins
    mpSipRegisterPlugins = new PluginHooks( RegisterPlugin::Factory
                                           ,RegisterPlugin::Prefix
                                           );
    mpSipRegisterPlugins->readConfig(*pOsConfigDb);

    _expireThread.run(SipRegistrar::getInstance(NULL)->getRegDB());
}




/// Apply valid changes to the database
///
/// Checks the message against the database, and if it is allowed by
/// those checks, applies the requested changes.
SipRegistrarServer::RegisterStatus
SipRegistrarServer::applyRegisterToDirectory( const Url& toUrl
                                             ,const UtlString& instrument
                                              //< the instrument identification
                                              // value from the authentication
                                              // user name, if any
                                             ,const int timeNow
                                             ,const SipMessage& registerMessage
                                             ,RegistrationExpiryIntervals*& pExpiryIntervals
                                             )
{
#if 0
    // This is no longer needed because all mongo operations is thread-safe
    OsLock lock(sLockMutex);
#endif

    RegisterStatus returnStatus = REGISTER_SUCCESS;
    UtlBoolean removeAll = FALSE;
    UtlBoolean isExpiresheader = FALSE;
    int longestRequested = -1; // for calculating the spread expiration time
    int commonExpires = -1;
    UtlString registerToStr;
    toUrl.getIdentity(registerToStr);
    std::string identity = registerToStr.str();

    // check if we are dealing with a registering UA that is located
    // behind a remote NAT.  If that is the case then we want to
    // use shorter registration expiry values to ensure that pinholes
    // get quickly re-established after network outages or system/component
    // reboots (XX-5986).
    if( isRegistrantBehindNat( registerMessage ) )
    {
       pExpiryIntervals = &mNatedExpiryIntervals;
    }
    else
    {
       pExpiryIntervals = &mNormalExpiryIntervals;
    }

    // get the expires header from the register message
    // this may be overridden by the expires parameter on each contact
    if ( registerMessage.getExpiresField( &commonExpires ) )
    {
        isExpiresheader = TRUE; // only for use in testing '*'
    }
    else
    {
        commonExpires = pExpiryIntervals->mMaxExpiresTime;
    }

    // get the header 'callid' from the register message
    UtlString registerCallidStr;
    registerMessage.getCallIdField( &registerCallidStr );

    // get the cseq and the method (should be REGISTER)
    // from the register message
    UtlString method;
    int registerCseqInt = 0;
    registerMessage.getCSeqField( &registerCseqInt, &method );

    // Check that this call-id and cseq are newer than what we have in the db
    RegDB* regDb = SipRegistrar::getInstance(NULL)->getRegDB();
    //if (! imdb->isOutOfSequence(toUrl, registerCallidStr, registerCseqInt))
    if (!regDb->isOutOfSequence(registerToStr.str(), registerCallidStr.str(), registerCseqInt))
    {
        // ****************************************************************
        // We now make two passes over all the contacts - the first pass
        // checks each contact for validity, and if valid, stores it in the
        // ResultSet registrations.
        // We act on the stored registrations only if all checks pass, in
        // a second iteration over the ResultSet below.
        // ****************************************************************

        std::vector<RegBinding::Ptr> registrations;

        int contactIndexCount;
        UtlString registerContactStr;
        for ( contactIndexCount = 0;
              (   REGISTER_SUCCESS == returnStatus
               && registerMessage.getContactEntry ( contactIndexCount
                                                   ,&registerContactStr
                                                   )
               );
              contactIndexCount++
             )
        {
           Os::Logger::instance().log( FAC_SIP, PRI_DEBUG,
                          "SipRegistrarServer::applyRegisterToDirectory - processing '%s'",
                          registerContactStr.data()
                               );
            if ( registerContactStr.compareTo("*") != 0 ) // is contact == "*" ?
            {
                // contact != "*"; a normal contact
                int expires;
                UtlString qStr, expireStr;
                Url registerContactURL( registerContactStr );

                // Check the schme of the parsed URI.
                Url::Scheme scheme = registerContactURL.getScheme();
                switch (scheme)
                {
                case Url::UnknownUrlScheme:
                   // Unknown scheme or parse failed.
                   Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                                 "Attempt to register Contact '%s' that is not a valid URI or has an unknown scheme",
                                 registerContactStr.data());
                   break;

                default:
                {
                   // Not sip: or sips:.
                   Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                                 "Attempt to register Contact '%s' with a scheme '%s' that is not 'sip' or 'sips'",
                                 registerContactStr.data(),
                                 Url::schemeName(scheme));
                   break;
                }
                case Url::SipUrlScheme:
                case Url::SipsUrlScheme:
                {
                   // sip: or sips:.
                   // Check for an (optional) Expires field parameter in the Contact
                   registerContactURL.getFieldParameter( SIP_EXPIRES_FIELD, expireStr );
                   if ( expireStr.isNull() )
                   {
                      // no expires parameter on the contact
                      // use the default established above
                      expires = commonExpires;
                   }
                   else
                   {
                      // contact has its own expires parameter, which takes precedence
                      char  expireVal[12]; // more digits than we need...
                      char* end;
                      strncpy( expireVal, expireStr.data(), 10 );
                      expireVal[11] = '\0';// ensure it is null terminated.
                      expires = strtol(expireVal,&end,/*base*/10);
                      if ( '\0' != *end )
                      {
                         // expires value not a valid base 10 number
                         returnStatus = REGISTER_INVALID_REQUEST;
                         Os::Logger::instance().log( FAC_SIP, PRI_WARNING,
                                        "SipRegistrarServer::applyRegisterToDirectory"
                                        " invalid expires parameter value '%s'",
                                        expireStr.data()
                            );
                      }
                   }

                   if ( REGISTER_SUCCESS == returnStatus )
                   {
                      // Ensure that the expires value is within allowed limits
                      if ( 0 == expires )
                      {
                         // unbind this mapping; ok
                      }
                      else if ( expires < pExpiryIntervals->mMinExpiresTime) // lower bound
                      {
                         returnStatus = REGISTER_LESS_THAN_MINEXPIRES;
                      }
                      else if ( expires > pExpiryIntervals->mMaxExpiresTime ) // upper bound
                      {
                         // our default is also the maximum we'll allow
                         expires = pExpiryIntervals->mMaxExpiresTime;
                      }

                      if ( REGISTER_SUCCESS == returnStatus )
                      {
                         // Get the qValue from the register message.
                         UtlString registerQvalueStr;
                         registerContactURL.getFieldParameter( SIP_Q_FIELD, registerQvalueStr );

                         // Get the Instance ID (if any) from the REGISTER message Contact field.
                         UtlString instanceId;
                         registerContactURL.getFieldParameter( "+sip.instance", instanceId );

                         // Get the instrument identification which applies to this contact:
                         // If 'instrument' is not provided by the Authorization header,
                         // and if +sip.instance is provided, use it.
                         UtlString contactInstrument(instrument);
                         if (contactInstrument.isNull() && !instanceId.isNull())
                         {
                            // Check that the instanceId value has the proper string format,
                            // i.e., it starts with '<' and ends with '>'.
                            if (instanceId(0) == '<' && instanceId(instanceId.length()-1) == '>')
                            {
                               // Check whether this instanceId value can be reduced to a MAC/EUI.
                               RegEx instanceUrnUuidMac(InstanceUrnUuidMac);
                               if (   instanceUrnUuidMac.Search(instanceId)
                                   && instanceUrnUuidMac.MatchStart(0) == 0)
                               {
                                  // Copy the MAC/EUI into contactInstrument.
                                  instanceUrnUuidMac.MatchString(&contactInstrument, 1);
                               }
                               else
                               {
                                  // There is a +sip.instance string value, but we can't find
                                  // a MAC/EUI in it, so we use the whole URN.
                                  contactInstrument.append(instanceId, 1, instanceId.length()-2);
                               }
                            }
                         }

                         Os::Logger::instance().log( FAC_SIP, PRI_DEBUG,
                                        "SipRegistrarServer::applyRegisterToDirectory"
                                        " instance ID = '%s', instrument ID = '%s'",
                                        instanceId.data(), contactInstrument.data());

                         // track longest expiration requested in the set
                         if ( expires > longestRequested )
                         {
                            longestRequested = expires;
                         }

                         // remove the parameter fields - they are not part of the contact itself
                         registerContactURL.removeFieldParameters();
                         UtlString contactWithoutExpires = registerContactURL.toString();

                         // Concatenate all path addresses in a single string.
                         UtlString pathStr;
                         UtlString tempPathUriStr;
                         int tmpPathIndex;
                         for( tmpPathIndex = 0;  registerMessage.getPathUri(tmpPathIndex, &tempPathUriStr); tmpPathIndex++ )
                         {
                            if( tmpPathIndex != 0 )
                            {
                               pathStr.append( SIP_MULTIFIELD_SEPARATOR );
                            }
                            pathStr.append( tempPathUriStr );
                         }

                         // Build the row for the validated contacts hash
                         UtlHashMap registrationRow;

                       

                         // Calculate GRUU if +sip.instance is provided.
                         // Note a GRUU is constructed even if the UA does not
                         // support GRUU itself -- other UAs can discover the
                         // GRUU via the reg event package.
                         UtlString gruuValue;
                         if (!instanceId.isNull())
                         {
                            // Hash the GRUU base, the AOR, and IID to
                            // get the variable part of the GRUU.
                            NetMd5Codec encoder;
                            // Use the trick that the MD5 of a series
                            // of uniquely parsable strings is
                            // effectively a unique function of all of
                            // the strings.  Include "sipX" as the
                            // signature of this software.
                            encoder.hash("sipX", sizeof ("sipX")-1);
                            /* The identifier of this domain,
                             * to ensure GRUUs aren't duplicated between domains. */
                            encoder.hash(mRegistrar.defaultDomain());
                            encoder.hash(toUrl.toString());
                            encoder.hash(instanceId);
                            // The preceding three items may not be uniquely parsable in theory,
                            // but they are in practice.
                            UtlString hash;
                            encoder.appendBase64Sig(hash,
                                                    NetBase64Codec::RFC4648UrlSafeAlphabet);
                            /* Use >=64 bits (11 chars), to avoid collisions
                             * when there are less than 2^32 registrations. */
                            hash.remove(11);
                            // Now construct the GRUU URI,
                            // "~~gr~XXXXXXXXXXXXXXXX@[principal SIP domain]".
                            // That is what we store in IMDB, so it can be
                            // searched for by the redirector, since it searches
                            // for the "identity" part of the URI, which does
                            // not contain the scheme.
                            gruuValue = GRUU_PREFIX;
                            gruuValue.append(hash);
                            gruuValue.append('@');
                            gruuValue.append(mRegistrar.defaultDomain());
                            gruuValue.append(";" SIP_GRUU_URI_PARAM);
                            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                                          "SipRegistrarServer::applyRegisterToDirectory "
                                          "gruu = '%s'",
                                          gruuValue.data());
                         }

                        RegBinding::Ptr pRegBinding(new RegBinding());
                         pRegBinding->setContact(contactWithoutExpires.str());
                         pRegBinding->setExpirationTime(expires);
                         pRegBinding->setQvalue(registerQvalueStr.str());
                         pRegBinding->setInstanceId(instanceId.str());
                         pRegBinding->setGruu(gruuValue.str());
                         pRegBinding->setPath(pathStr.str());
                         pRegBinding->setInstrument(contactInstrument.str());
                         pRegBinding->setCallId(registerCallidStr.str());
                         pRegBinding->setCseq(registerCseqInt);
                         pRegBinding->setIdentity(registerToStr.str());
                         pRegBinding->setUri(toUrl.toString().str());
                         registrations.push_back(pRegBinding);
                      }
                   }
                   break;
                }
                }
            }
            else
            {
                // Asterisk ('*') requests that we unregister all contacts for the AOR
                removeAll = TRUE;
            }
        } // iteration over Contact entries

        // Now that we've gone over all the Contacts
        // act on them only if all was kosher
        if ( REGISTER_SUCCESS == returnStatus )
        {
            if ( 0 == contactIndexCount )
            {   // no contact entries were found - this is just a query
                returnStatus = REGISTER_QUERY;
            }
            else
            {
               unsigned int spreadExpirationTime = 0;

                if ( removeAll )
                {
                    // Contact: * special case
                    //  - request to remove all bindings for toUrl
                    if (   isExpiresheader
                        && 0 == commonExpires
                        && 1 == contactIndexCount
                        )
                    {
                        regDb->expireAllBindings(
                            identity,
                            registerCallidStr.str(),
                            registerCseqInt,
                            timeNow);
                    }
                    else
                    {
                        // not allowed per rfc 3261
                        //  - must use Expires header of zero with Contact: *
                        //  - must not combine this with other contacts
                        returnStatus = REGISTER_INVALID_REQUEST;
                    }
                }
                else
                {
                    // Normal REGISTER request, no Contact: * entry
                    size_t numRegistrations = registrations.size();

                    // act on each valid contact
                    if ( numRegistrations > 0 )
                    {
                        /*
                         * Calculate the maximum expiration time for this set.
                         * The idea is to spread registrations by choosing a random
                         * expiration time.
                         */
                       int spreadFloor = pExpiryIntervals->mMinExpiresTime * 2;
                       if ( longestRequested > spreadFloor )
                        {
                           // a normal (long) registration
                           // - spread it between twice the min and the longest they asked for
                           spreadExpirationTime = (  (rand() % (longestRequested - spreadFloor))
                                                   + spreadFloor);
                        }
                        else if ( longestRequested > pExpiryIntervals->mMinExpiresTime )
                        {
                           // a short but not minimum registration
                           // - spread it between the min and the longest they asked for
                           spreadExpirationTime = (  (rand()
                                                      % (longestRequested - pExpiryIntervals->mMinExpiresTime)
                                                      )
                                                   + pExpiryIntervals->mMinExpiresTime
                                                   );
                        }
                        else // longestExpiration == mMinExpiresTimeint
                        {
                           // minimum - can't randomize because we can't shorten or lengthen it
                           spreadExpirationTime = pExpiryIntervals->mMinExpiresTime;
                        }

                        for ( std::vector<RegBinding::Ptr>::iterator iter = registrations.begin();
                                iter != registrations.end(); iter++)
                        {
                            RegBinding::Ptr pRecord = *iter;

                            unsigned int expires = pRecord->getExpirationTime();

                            int expirationTime;
                            if ( expires == 0 )
                            {
                                // Unbind this binding
                                //
                             
                                expirationTime = 0;

                                Os::Logger::instance().log( FAC_SIP, PRI_DEBUG,
                                              "SipRegistrarServer::applyRegisterToDirectory "
                                              "- Expiring map '%s'->'%s'",
                                              registerToStr.data(), pRecord->getContact().c_str()
                                              );
                            }
                            else
                            {
                                // expires > 0, so add the registration
                                expirationTime = (  expires < spreadExpirationTime
                                                  ? expires
                                                  : spreadExpirationTime
                                                  ) + timeNow;

                                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                                              "SipRegistrarServer::applyRegisterToDirectory - "
                                              "Adding map '%s'->'%s' "
                                              "expires %d (now+%d)",
                                              registerToStr.data(), pRecord->getContact().c_str(),
                                              expirationTime, expires);
                            }

                            pRecord->setExpirationTime(expirationTime);

                           regDb->updateBinding(pRecord);

                        } // iterate over good contact entries

                        // If there were any bindings not dealt with explicitly in this
                        // message that used the same callid, then expire them.
                        regDb->expireOldBindings(
                            identity,
                            registerCallidStr.str(),
                            registerCseqInt,
                            timeNow);
                    }
                    else
                    {
                        Os::Logger::instance().log( FAC_SIP, PRI_ERR
                                      ,"SipRegistrarServer::applyRegisterToDirectory "
                                      "contact count mismatch %d != %d"
                                      ,contactIndexCount, (int)numRegistrations
                                      );
                        returnStatus = REGISTER_QUERY;
                    }
                }

                // Only if this was a good registration:
                // update reg event content, persist the xml, and do registration hooks
                if ( REGISTER_SUCCESS == returnStatus )
                {
                    // Now that we have revised the bindings for
                    // toUrl, update the reg event content for it.
                    RegisterEventServer* s = mRegistrar.getRegisterEventServer();
                    if (s)
                    {
                       // Use getUri to extract the AOR as a string, because
                       // registerToStr is only the identity part.
                       UtlString aorString;
                       toUrl.getUri(aorString);
                       s->generateAndPublishContent(aorString, toUrl, instrument);
                    }

                    // give each RegisterPlugin a chance to do its thing
                    PluginIterator plugins(*mpSipRegisterPlugins);
                    RegisterPlugin* plugin;
                    while ((plugin = static_cast<RegisterPlugin*>(plugins.next())))
                    {
                       plugin->takeAction(registerMessage, spreadExpirationTime, mSipUserAgent );
                    }
                }
            }
        }
    }
    else
    {
        Os::Logger::instance().log( FAC_SIP, PRI_WARNING,
                      "SipRegistrarServer::applyRegisterToDirectory request out of order"
                      "  To: '%s'\n"
                      "  Call-Id: '%s'\n"
                      "  Cseq: %d",
                      registerToStr.data(), registerCallidStr.data(), registerCseqInt
                     );

        returnStatus = REGISTER_OUT_OF_ORDER;
    }

    return returnStatus;
}



//functions
UtlBoolean
SipRegistrarServer::handleMessage( OsMsg& eventMessage )
{

  int msgType = eventMessage.getMsgType();
  int msgSubType = eventMessage.getMsgSubType();
  UtlBoolean handled = FALSE;

  std::string errorString;

  try
  {
    // Timer event
    if(   msgType    == OsMsg::OS_EVENT
       && msgSubType == OsEventMsg::NOTIFY
       )
    {
       Os::Logger::instance().log( FAC_SIP, PRI_DEBUG, "SipRegistrarServer::handleMessage() - "
                     "unknown timer expiration" );

        handled = TRUE;
    }
    // SIP message event
    else if (msgType == OsMsg::PHONE_APP)
    {
        Os::Logger::instance().log( FAC_SIP, PRI_DEBUG, "SipRegistrarServer::handleMessage() - "
                "Start processing REGISTER Message" );

        const SipMessage& message = *((SipMessageEvent&)eventMessage).getMessage();
        UtlString userKey, uri;
        SipMessage finalResponse;

        // Fetch the domain and port from the request URI
        UtlString lookupDomain, requestUri;
        message.getRequestUri( &requestUri );
        Url reqUri(requestUri);

        if ( mRegistrar.isValidDomain(reqUri) )
        {
           // Get the full To name-addr.
           Url toUri;
           message.getToUrl(toUri);

           /*
            * Normalize the port in the To URI.
            *   This is not strictly kosher, but it solves interoperability problems.
            *   Technically, user@foo:5060 != user@foo , but many implementations
            *   insist on including the explicit port even when they should not, and
            *   it causes registration mismatches, so we normalize the URI when inserting
            *   and looking up in the database so that if explicit port is the same as
            *   the proxy listening port, then we remove it.
            *   (Since our proxy has mProxyNormalPort open, no other SIP entity
            *   can use sip:user@domain:mProxyNormalPort, so this normalization
            *   cannot interfere with valid addresses.)
            *
            * For the strict rules, set the configuration parameter
            *   SIP_REGISTRAR_PROXY_PORT : PORT_NONE
            */
           int proxyPort = mRegistrar.domainProxyPort();
           if (   proxyPort != PORT_NONE
               && toUri.getHostPort() == proxyPort
               )
           {
              toUri.setHostPort(PORT_NONE);
           }

           /*
            * This is to support registration where the To URI uses a domain alias.
            * Replace domain part of the ToURI with the default domain
            */
           if (mSipUserAgent->isMyHostAlias(toUri))
           {
              if (Os::Logger::instance().willLog(FAC_SIP, PRI_INFO))
              {
                 UtlString alias;
                 toUri.getHostWithPort(alias);
                 Os::Logger::instance().log( FAC_SIP, PRI_INFO, "SipRegistrarServer::handleMessage() "
                               "Allowing use of domain alias; To domain '%s' -> '%s'",
                               alias.data(), mRegistrar.defaultDomain());
              }
              toUri.setHostAddress(mRegistrar.defaultDomain() );
           }

           // check in credential database if authentication needed
           UtlString instrument; // the instrument value from the authentication user name
           if ( isAuthorized( toUri, instrument, message, finalResponse ) )
            {
                int port;
                UtlString address, protocol, tag;
                message.getToAddress( &address, &port, &protocol, NULL, NULL, &tag );

                // Add new contact values - update or insert.
                int timeNow = (int) OsDateTime::getSecsSinceEpoch();
                RegistrationExpiryIntervals* pExpiryIntervalsUsed = 0;
                RegisterStatus applyStatus
                   = applyRegisterToDirectory( toUri, instrument,
                                               timeNow, message,
                                               pExpiryIntervalsUsed );

                switch (applyStatus)
                {
                    case REGISTER_SUCCESS:
                    case REGISTER_QUERY:
                    {
                        Os::Logger::instance().log( FAC_SIP, PRI_DEBUG, "SipRegistrarServer::handleMessage() - "
                               "contact successfully added");

                        //create response - 200 ok reseponse
                        finalResponse.setOkResponseData(&message);

                        // get the call-id from the register message for context test below
                        UtlString registerCallId;
                        message.getCallIdField(&registerCallId);

                        //get all current contacts now for the response

                        UtlString identity_;
                        toUri.getIdentity(identity_);
                        RegDB::Bindings registrations;
                        SipRegistrar::getInstance(NULL)->getRegDB()->getUnexpiredContactsUser(identity_.str(),
                            timeNow, registrations);
                        bool requestSupportsGruu =
                           message.isInSupportedField("gruu");
#ifdef GRUU_WORKAROUND
                        // Workaround causes GRUU to be sent even if
                        // "Supported: gruu" was not in request.
                        requestSupportsGruu = true;
#endif
                        bool allExpirationsEqual = false;
                        bool firstConsideredContact = true;
                        // commonExpirationTime is used only if allExpirationsEqual
                        // is true, and when the latter is set, commonExpirationTime
                        // is set.  But we must initialize it to stop the compiler
                        // from complaining.
                        unsigned int commonExpirationTime = 0;

                        int contactCounter = 0;
                        for (RegDB::Bindings::const_iterator iter = registrations.begin(); iter != registrations.end(); iter++)
                        {
                          const RegBinding& record = *iter;

                          // Check if this contact should be returned in this context
                          // for REGISTER_QUERY, return all contacts
                          // for REGISTER_SUCCESS, return only the contacts with the same
                          //     call-id.  This is because we've seen too many phones that
                          //     don't expect to see contacts other than the one they sent,
                          //     and get upset by contact parameters meant for others
                          //     In particular, when some other contact is about to expire,
                          //     they think that they got a short time and try again - which
                          //     loops until the other contact expires or is refreshed by
                          //     someone else - not good.



                          if (mSendAllContactsInResponse || REGISTER_QUERY == applyStatus || registerCallId.str() == record.getCallId())
                             
                          {


                            unsigned int expires = record.getExpirationTime();
                            expires = expires - timeNow;

                            Os::Logger::instance().log( FAC_SIP, PRI_DEBUG,
                                          "SipRegistrarServer::handleMessage - "
                                          "processing contact '%s'", record.getContact().c_str());
                            Url contactUri( record.getContact().c_str() );

                            UtlString expiresStr;
                            expiresStr.appendNumber((int)expires);

                            if ( firstConsideredContact ) // first considered contact
                            {
                               firstConsideredContact = false;
                               allExpirationsEqual = true;
                               commonExpirationTime = expires;
                            }
                            else if (expires != commonExpirationTime)
                            {
                               allExpirationsEqual = false;
                            }

                            contactUri.setFieldParameter(SIP_EXPIRES_FIELD, expiresStr.data());

                            if ( !record.getQvalue().empty() )
                            {
                               Os::Logger::instance().log( FAC_SIP, PRI_DEBUG,
                                             "SipRegistrarServer::handleMessage - "
                                             "adding q '%s'", record.getQvalue().c_str());

                               //check if q value is numeric and between the range 0.0 and 1.0
                               RegEx qValueValid(RegQValue);
                               if (qValueValid.Search(record.getQvalue().c_str()))
                               {
                                  contactUri.setFieldParameter(SIP_Q_FIELD, record.getQvalue().c_str());
                               }
                            }

                            // Add the +sip.instance and gruu
                            // parameters if an instance ID is recorded.


                            Os::Logger::instance().log( FAC_SIP, PRI_DEBUG,
                                          "SipRegistrarServer::handleMessage"
                                          " - instanceIdKey = '%s'",
                                           record.getInstanceId().c_str());

                            if (!record.getInstanceId().empty())
                            {
                               Os::Logger::instance().log( FAC_SIP, PRI_DEBUG,
                                             "SipRegistrarServer::handleMessage"
                                             " - add instance '%s'",
                                             record.getInstanceId().c_str());

                               contactUri.setFieldParameter("+sip.instance", record.getInstanceId().c_str());


                               // Only add the "gruu" parameter if the GRUU is
                               // non-null and the request includes "Supported:
                               // gruu".
                               if (requestSupportsGruu && !record.getGruu().empty())
                               {
                                  // Prepend "sip:" to the GRUU, since it is stored
                                  // in the database in identity form.
                                  UtlString temp("sip:");
                                  temp.append(record.getGruu().c_str());
                                  contactUri.setFieldParameter("pub-gruu", temp);
#ifdef GRUU_WORKAROUND
                                  // Workaround causes GRUU to be sent in the
                                  // 'gruu' parameter as well, for
                                  // backward compatibility.
                                  contactUri.setFieldParameter("gruu", temp);
#endif
                               }
                            }

                            // Undo the transformations made to the Contact by the
                            // sipXproxy for NAT traversal.  These transformations
                            // effectively create a Contact that will cause a UAC compliant
                            // with section 19.1.4 of RFC 3261 to fail to match the
                            // Contact returned in the 200 OK with the one it sent.
                            // We have seen that this causes interop problems
                            // with some phones.  As a results, steps are taken here
                            // to undo the transformations that sipXproxy applied to the
                            // Contact.  See XX-5926 for details.
                            UtlString privateContact;
                            if( contactUri.getUrlParameter( SIPX_PRIVATE_CONTACT_URI_PARAM, privateContact, 0 ) )
                            {
                               Url privateContactAsUrl;
                               UtlString hostAddressString;

                               privateContactAsUrl.fromString( privateContact );
                               privateContactAsUrl.getHostAddress( hostAddressString );
                               contactUri.setHostAddress( hostAddressString );
                               contactUri.setHostPort( privateContactAsUrl.getHostPort() );
                               contactUri.removeUrlParameter( SIPX_PRIVATE_CONTACT_URI_PARAM );
                            }
                            else
                            {
                               // if we do not have a SIPX_PRIVATE_CONTACT_URI_PARAM,
                               // we almost certainly have a SIPX_NO_NAT_URI_PARAM - just
                               // remove it without testing for its presence - if it is not
                               // present then the remove operation just ends up being a no-op
                               contactUri.removeUrlParameter( SIPX_NO_NAT_URI_PARAM );
                            }

                            finalResponse.setContactField(contactUri.toString(), contactCounter++);
                          }
                        }

                        if (allExpirationsEqual && mSendExpiresInResponse)
                        {
                           /*
                            * Some clients are not good at picking the expiration out of the contact
                            * field parameter, so if all the expiration times are the same (usually
                            * true in a REGISTER response because we only send the contacts for that call-id),
                            * copy the value into an Expires header too.
                            */
                           UtlString expiresString;
                           expiresString.appendNumber((int)commonExpirationTime);
                           finalResponse.setHeaderValue(SIP_EXPIRES_FIELD, expiresString, 0);
                        }

                        // Add the testing contact, if it is set.
                        if (!mAdditionalContact.isNull())
                        {
                           finalResponse.setContactField(mAdditionalContact,
                                                         registrations.size());
                        }
                    }
                    break;

                    case REGISTER_OUT_OF_ORDER:
                        finalResponse.setResponseData(&message,SIP_5XX_CLASS_CODE,"Out Of Order");
                        break;

                    case REGISTER_LESS_THAN_MINEXPIRES:
                    {
                        //send 423 Registration Too Brief response
                        //must contain Min-Expires header field
                        finalResponse.setResponseData(&message,
                                                      SIP_TOO_BRIEF_CODE,SIP_TOO_BRIEF_TEXT);
                        UtlString minExpiresAsString;
                        if( pExpiryIntervalsUsed )
                        {
                           minExpiresAsString.appendNumber( pExpiryIntervalsUsed->mMinExpiresTime );
                        }
                        else
                        {
                           // safeguard - should never happen
                           minExpiresAsString.appendNumber( mNormalExpiryIntervals.mMinExpiresTime );
                        }
                        finalResponse.setHeaderValue(SIP_MIN_EXPIRES_FIELD, minExpiresAsString, 0);
                    }
                    break;

                    case REGISTER_INVALID_REQUEST:
                        finalResponse.setResponseData(&message,
                                                      SIP_BAD_REQUEST_CODE, SIP_BAD_REQUEST_TEXT);
                        break;

                    case REGISTER_FORBIDDEN:
                        finalResponse.setResponseData(&message,
                                                      SIP_FORBIDDEN_CODE, SIP_FORBIDDEN_TEXT);
                        break;

                    case REGISTER_NOT_FOUND:
                        finalResponse.setResponseData(&message,
                                                      SIP_NOT_FOUND_CODE, SIP_NOT_FOUND_TEXT);
                        break;

                    default:
                       Os::Logger::instance().log( FAC_SIP, PRI_ERR,
                                     "Invalid result %d from applyRegisterToDirectory",
                                     applyStatus
                                     );
                        finalResponse.setResponseData(&message,
                                                      SIP_SERVER_INTERNAL_ERROR_CODE,
                                                      SIP_SERVER_INTERNAL_ERROR_TEXT);
                        break;
                }

                // Add tag to response if none is present in the request (which
                // there shouldn't be).
                if ( tag.isNull() )
                {
                   UtlString newTag;
                   CallId::getNewTag(newTag);
                   finalResponse.setToFieldTag(newTag.data());
                }
            }
           else
            {
               // authentication error - response data was set in isAuthorized
            }
        }
        else
        {
           // Invalid domain for registration
           UtlString requestedDomain;
           reqUri.getHostAddress(requestedDomain);

           Os::Logger::instance().log(FAC_AUTH, PRI_WARNING,
                         "SipRegistrarServer::handleMessage('%s' == '%s') Invalid",
                         requestedDomain.data(), lookupDomain.data()) ;

           UtlString responseText;
           responseText.append("Domain '");
           responseText.append(requestedDomain);
           responseText.append("' is not valid at this registrar");
           finalResponse.setResponseData(&message, SIP_NOT_FOUND_CODE, responseText.data() );
        }

        mSipUserAgent->setUserAgentHeader(finalResponse);

        if (Os::Logger::instance().willLog(FAC_SIP, PRI_DEBUG))
        {
           UtlString finalMessageStr;
           ssize_t finalMessageLen;
           finalResponse.getBytes(&finalMessageStr, &finalMessageLen);
           Os::Logger::instance().log( FAC_SIP, PRI_DEBUG, "\n----------------------------------\n"
                         "Sending final response\n%s", finalMessageStr.data());
        }

        mSipUserAgent->send(finalResponse);

        handled = TRUE;
    }
    else if ( msgType == OsMsg::OS_SHUTDOWN )
    {
       Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                    "SipRegistrarServer::handleMessage received shutdown request"
                     );
       OsTask::requestShutdown(); // tell OsServerTask::run to exit
       handled = TRUE;
    }
    else
    {
       Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                     "SipRegistrarServer::handleMessage unexpected message type %d/%d",
                     msgType, msgSubType
                     ) ;
    }

    return handled;
  }
#ifdef MONGO_assert
  catch (mongo::DBException& e)
  {
    errorString = "Registry - Mongo DB Exception";
    OS_LOG_ERROR( FAC_SIP, "SipRegistrarServer::handleMessage() Exception: "
             << e.what() );
  }
#endif
  catch (boost::exception& e)
  {
    errorString = "Registry - Boost Library Exception";
    OS_LOG_ERROR( FAC_SIP, "SipRegistrarServer::handleMessage() Exception: "
             << boost::diagnostic_information(e));
  }
  catch (std::exception& e)
  {
    errorString = "Registry - Standard Library Exception";
    OS_LOG_ERROR( FAC_SIP, "SipRegistrarServer::handleMessage() Exception: "
             << e.what() );
  }
  catch (...)
  {
    errorString = "Registry - Unknown Exception";
    OS_LOG_ERROR( FAC_SIP, "SipRegistrarServer::handleMessage() Exception: Unknown Exception");
  }

  //
  // If it ever get here, that means we caught an exception
  //
  if (msgType == OsMsg::PHONE_APP)
  {
    const SipMessage& message = *((SipMessageEvent&)eventMessage).getMessage();
    SipMessage finalResponse;
    finalResponse.setResponseData(&message, SIP_5XX_CLASS_CODE, errorString.c_str());
    mSipUserAgent->send(finalResponse);
    handled = TRUE;
  }

  return handled;
}


UtlBoolean
SipRegistrarServer::isAuthorized(const Url& toUri,
                                 UtlString& instrument,
                                 const SipMessage& message,
                                 SipMessage& responseMessage )
{
    UtlBoolean isAuthorized = FALSE;

    Url fromNameAddr;
    message.getFromUrl(fromNameAddr);

    UtlString identity;
    toUri.getIdentity(identity);

    if ( !mUseCredentialDB )
    {
        Os::Logger::instance().log( FAC_AUTH, PRI_DEBUG, "SipRegistrarServer::isAuthorized "
                      "No Credential DB - request is always AUTHORIZED" );
        isAuthorized = TRUE;
    }
    else
    {
        // Realm and auth type should be default for server.
        // check if we requested authentication and this is the req with
        // authorization,validate the authorization
        Os::Logger::instance().log( FAC_AUTH, PRI_DEBUG,
                      "SipRegistrarServer::isAuthorized "
                      "fromNameAddr='%s', toUri='%s', realm='%s' ",
                       fromNameAddr.toString().data(), toUri.toString().data(),
                       mRealm.data() );

        UtlString requestNonce;
        UtlString requestRealm;
        UtlString requestUser;
        UtlString requestUserBase;
        UtlString uriParam;
        UtlString requestCnonce;
        UtlString requestNonceCount;
        UtlString requestQop;
        int requestAuthIndex = 0;
        UtlString callId;
        UtlString fromTag;
        // 'instrument' is a parameter of this method.

        message.getCallIdField(&callId);
        fromNameAddr.getFieldParameter("tag", fromTag);

        while ( ! isAuthorized
               && message.getDigestAuthorizationData(&requestUser,
                                                     &requestRealm, 
                                                     &requestNonce,
                                                     NULL, NULL, 
                                                     &uriParam,
                                                     &requestCnonce,  // cnonce
                                                     &requestNonceCount,  // nonceCount
                                                     &requestQop,  // qop
                                                     HttpMessage::SERVER, 
                                                     requestAuthIndex,
                                                     &requestUserBase, 
                                                     &instrument)
               )
        {
           Os::Logger::instance().log( FAC_AUTH, PRI_DEBUG, "Message Authorization received: "
                    "reqRealm='%s', reqUser='%s', reqUserBase='%s', instrument='%s'",
                          requestRealm.data() , requestUser.data(),
                          requestUserBase.data(), instrument.data());

           UtlString qopType;

           if (mRealm.compareTo(requestRealm) ) // case sensitive check that realm is correct
           {
              Os::Logger::instance().log(FAC_AUTH, PRI_DEBUG,
                            "SipRegistrarServer::isAuthorized "
                            "Realm does not match");
           }

           // validate the nonce
           else if (!mNonceDb.isNonceValid(requestNonce, callId, fromTag, mRealm, mNonceExpiration))
           {
               Os::Logger::instance().log(FAC_AUTH, PRI_INFO,
                             "SipRegistrarServer::isAuthorized "
                             "Invalid nonce for '%s', nonce='%s', callId='%s'",
                             identity.data(), requestNonce.data(), callId.data());
           }

           // verify that qop,cnonce, nonceCount are compatible
           else if (message.verifyQopConsistency(requestCnonce.data(),
                                                 requestNonceCount.data(),
                                                 &requestQop,
                                                 qopType)
                    >= HttpMessage::AUTH_QOP_NOT_SUPPORTED)
           {
               Os::Logger::instance().log(FAC_AUTH, PRI_INFO,
                             "SipRegistrarServer::isAuthorized "
                             "Invalid combination of QOP('%s'), cnonce('%s') and nonceCount('%s')",
                             requestQop.data(), requestCnonce.data(), requestNonceCount.data());
           }

           else // realm, nonce and qop are all ok
           {
                // need the request URI to validate the nonce
                UtlString reqUri;
                message.getRequestUri(&reqUri);
                UtlString authTypeDB;
                UtlString passTokenDB;
                Url discardUriFromDB;

                // then get the credentials for this user & realm

                if (SipRegistrar::getInstance(NULL)->getEntityDB()->getCredential( toUri
                                   ,requestRealm
                                   ,requestUserBase
                                   ,passTokenDB
                                   ,authTypeDB
                                   ))
                {
                  // only DIGEST is used, so the authTypeDB above is ignored
                  if ((isAuthorized = message.verifyMd5Authorization(requestUser.data(),
                                                                     passTokenDB.data(),
                                                                     requestNonce,
                                                                     requestRealm.data(),
                                                                     requestCnonce.data(),
                                                                     requestNonceCount.data(),
                                                                     requestQop.data(),
                                                                     uriParam)
                       ))
                    {
                      Os::Logger::instance().log(FAC_AUTH, PRI_DEBUG,
                                    "SipRegistrarServer::isAuthorized "
                                    "response auth hash matches");
                    }
                  else
                    {
                      Os::Logger::instance().log(FAC_AUTH, PRI_ERR,
                                    "Response auth hash does not match (bad password?)"
                                    " toUri='%s' requestUser='%s' requestNonce='%s' uriParam='%s'"
                                    " passTokenDB='%s' authTypeDB='%s'",
                                    toUri.toString().data(),
                                    requestUser.data(),
                                    requestNonce.data(),
                                    uriParam.data(),
                                    passTokenDB.data(),
                                    authTypeDB.data());
                    }
                }
                else // failed to get credentials
                {
                    Os::Logger::instance().log(FAC_AUTH, PRI_ERR,
                                  "Unable to get credentials for '%s', realm='%s', user='%s'",
                                  identity.data(), mRealm.data(), requestUser.data());
                }
           }    // end check DB
           requestAuthIndex++;
        } //end while

        if ( !isAuthorized )
        {
           // Generate a new challenge
            UtlString newNonce;
            UtlString opaque;

            mNonceDb.createNewNonce(callId,
                                    fromTag,
                                    mRealm,
                                    newNonce);

            responseMessage.setRequestUnauthorized ( &message, HTTP_DIGEST_AUTHENTICATION, mRealm,
                                                    newNonce, NULL // opaque
                                                    );

            // Clear instrument, which may have been set.
            instrument.remove(0);
        }
    }   // end DB exists
    return isAuthorized;
}







// inspect the first contact of the request and look for the presence of
// a x-sipX-privcontact URL parameter which indicates that the request comes
// from a UA that is located behind a NAT.  Note that this routine only
// tests for the first of possibly many contacts - this is good enough
// given that every contact will carry the x-sipX-privcontact parameter
// when the UA is behind a NAT.
bool SipRegistrarServer::isRegistrantBehindNat( const SipMessage& registerRequest ) const
{
   bool bIsBehindNat = false;
   UtlString registerContactStr;
   if( registerRequest.getContactEntry( 0, &registerContactStr ) )
   {
      Url contactUrl( registerContactStr );
      UtlString privateContact;
      if( contactUrl.getUrlParameter( SIPX_PRIVATE_CONTACT_URI_PARAM, privateContact, 0 ) )
      {
         bIsBehindNat = true;
      }
   }
   return bIsBehindNat;
}

SipRegistrarServer::~SipRegistrarServer()
{
}

void RegisterPlugin::takeAction( const SipMessage&   registerMessage
                                ,const unsigned int  registrationDuration
                                ,SipUserAgent*       sipUserAgent
                                )
{
   assert(false);

   Os::Logger::instance().log(FAC_SIP, PRI_ERR,
                 "RegisterPlugin::takeAction not resolved by configured hook"
                 );
}



