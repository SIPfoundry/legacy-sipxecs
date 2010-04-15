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
#include "sipdb/RegistrationBinding.h"
#include "sipdb/RegistrationDB.h"
#include "sipdb/ResultSet.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/CredentialDB.h"
#include "sipdb/RegistrationDB.h"
#include "RegistrarPersist.h"
#include "RegistrarSync.h"
#include "registry/SipRegistrar.h"
#include "SipRegistrarServer.h"
#include "RegisterEventServer.h"
#include "SyncRpc.h"
#include "registry/RegisterPlugin.h"
#include "sipXecsService/SipXecsService.h"

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

static UtlString gUriKey("uri");
static UtlString gCallidKey("callid");
static UtlString gContactKey("contact");
static UtlString gExpiresKey("expires");
static UtlString gCseqKey("cseq");
static UtlString gQvalueKey("qvalue");
static UtlString gInstanceIdKey("instance_id");
static UtlString gGruuKey("gruu");
static UtlString gPathKey("path");
static UtlString gContactInstrumentKey("contact_instrument");

OsMutex         SipRegistrarServer::sLockMutex(OsMutex::Q_FIFO);

SipRegistrarServer::SipRegistrarServer(SipRegistrar& registrar) :
    OsServerTask("SipRegistrarServer", NULL, SIPUA_DEFAULT_SERVER_OSMSG_QUEUE_SIZE),
    mRegistrar(registrar),
    mIsStarted(FALSE),
    mSipUserAgent(NULL),
    mSendExpiresInResponse(TRUE),
    mNonceExpiration(5*60)
{
}

void
SipRegistrarServer::initialize(
   OsConfigDb*   pOsConfigDb,      ///< Configuration parameters
   SipUserAgent* pSipUserAgent)    ///< User Agent to use when sending responses
{
    mSipUserAgent = pSipUserAgent;

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
           OsSysLog::add(FAC_SIP, PRI_WARNING,
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
          OsSysLog::add(FAC_SIP, PRI_WARNING,
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
           OsSysLog::add(FAC_SIP, PRI_WARNING,
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
          OsSysLog::add(FAC_SIP, PRI_WARNING,
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
        OsSysLog::add(FAC_SIP, PRI_INFO, "SIP_DOMAIN_ALIASES : %s",
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
          OsSysLog::add(FAC_SIP, PRI_INFO,
                        "SipRegistrarServer::initialize adding SIP_REGISTRAR_ADDITIONAL_CONTACT: '%s'",
                        mAdditionalContact.data());
       }
       else
       {
          OsSysLog::add(FAC_SIP, PRI_ERR,
                        "SipRegistrarServer::initialize Invalid value for SIP_REGISTRAR_ADDITIONAL_CONTACT: '%s', ignoring",
                        mAdditionalContact.data());
          // If value is invalid, make it null.
          mAdditionalContact.remove(0);
       }
    }

    // This is a developer-only configuration parameter
    // to prevent sending an Expires header in REGISTER responses
    mSendExpiresInResponse = pOsConfigDb->getBoolean("SIP_REGISTRAR_EXP_HDR_RSP", TRUE);
    OsSysLog::add(FAC_SIP, PRI_INFO,
                  "SipRegistrarServer::initialize SIP_REGISTRAR_EXP_HDR_RSP is %s",
                  mSendExpiresInResponse ? "Enabled" : "Disabled");

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
}

int SipRegistrarServer::pullUpdates(
   const UtlString& registrarName,
   Int64            updateNumber,
   UtlSList&        updates)
{
   // Critical Section here
   OsLock lock(sLockMutex);

   RegistrationDB* imdb = mRegistrar.getRegistrationDB();
   int numUpdates = imdb->getNewUpdatesForRegistrar(registrarName, updateNumber, updates);
   return numUpdates;
}

/// Set the largest update number in the local database for this registrar as primary
void SipRegistrarServer::setDbUpdateNumber(Int64 dbUpdateNumber)
{
   mDbUpdateNumber = dbUpdateNumber;

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipRegistrarServer::setDbUpdateNumber to %0#16" FORMAT_INTLL "x", mDbUpdateNumber.getValue());
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
    // Critical Section here
    OsLock lock(sLockMutex);

    RegisterStatus returnStatus = REGISTER_SUCCESS;
    UtlBoolean removeAll = FALSE;
    UtlBoolean isExpiresheader = FALSE;
    int longestRequested = -1; // for calculating the spread expiration time
    int commonExpires = -1;
    UtlString registerToStr;
    toUrl.getIdentity(registerToStr);

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

    RegistrationDB* imdb = mRegistrar.getRegistrationDB();

    // Check that this call-id and cseq are newer than what we have in the db
    if (! imdb->isOutOfSequence(toUrl, registerCallidStr, registerCseqInt))
    {
        // ****************************************************************
        // We now make two passes over all the contacts - the first pass
        // checks each contact for validity, and if valid, stores it in the
        // ResultSet registrations.
        // We act on the stored registrations only if all checks pass, in
        // a second iteration over the ResultSet below.
        // ****************************************************************

       ResultSet registrations; // built up during validation, acted on if all is well

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
           OsSysLog::add( FAC_SIP, PRI_DEBUG,
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
                   OsSysLog::add(FAC_SIP, PRI_WARNING,
                                 "Attempt to register Contact '%s' that is not a valid URI or has an unknown scheme",
                                 registerContactStr.data());
                   break;

                default:
                {
                   // Not sip: or sips:.
                   OsSysLog::add(FAC_SIP, PRI_WARNING,
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
                         OsSysLog::add( FAC_SIP, PRI_WARNING,
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

                         OsSysLog::add( FAC_SIP, PRI_DEBUG,
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

                         // value strings
                         UtlString* contactValue =
                            new UtlString ( contactWithoutExpires );
                         UtlInt* expiresValue =
                            new UtlInt ( expires );
                         UtlString* qvalueValue =
                            new UtlString ( registerQvalueStr );
                         UtlString* instanceIdValue = new UtlString ( instanceId );
                         UtlString* gruuValue;
                         UtlString* pathValue = new UtlString( pathStr );
                         UtlString* contactInstrumentValue = new UtlString( contactInstrument );

                         // key strings - make shallow copies of static keys
                         UtlString* contactKey = new UtlString( gContactKey );
                         UtlString* expiresKey = new UtlString( gExpiresKey );
                         UtlString* qvalueKey = new UtlString( gQvalueKey );
                         UtlString* instanceIdKey = new UtlString( gInstanceIdKey );
                         UtlString* gruuKey = new UtlString( gGruuKey );
                         UtlString* pathKey = new UtlString( gPathKey );
                         UtlString* contactInstrumentKey = new UtlString( gContactInstrumentKey );

                         // Calculate GRUU if +sip.instance is provided.
                         // Note a GRUU is constructed even if the UA does not
                         // support GRUU itself -- other UAs can discover the
                         // GRUU via the reg event package.
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
                            encoder.appendBase64Sig(hash);
                            /* Use 8 bytes, to avoid collisions
                             * when there are less than 2^32 registrations. */
                            hash.remove(16);
                            // Now construct the GRUU URI,
                            // "~~gr~XXXXXXXXXXXXXXXX@[principal SIP domain]".
                            // That is what we store in IMDB, so it can be
                            // searched for by the redirector, since it searches
                            // for the "identity" part of the URI, which does
                            // not contain the scheme.
                            gruuValue = new UtlString(GRUU_PREFIX);
                            gruuValue->append(hash);
                            gruuValue->append('@');
                            gruuValue->append(mRegistrar.defaultDomain());
                            gruuValue->append(";" SIP_GRUU_URI_PARAM);
                            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                          "SipRegistrarServer::applyRegisterToDirectory "
                                          "gruu = '%s'",
                                          gruuValue->data());
                         }
                         else
                         {
                            gruuValue = new UtlString( "" );
                         }

                         registrationRow.insertKeyAndValue( contactKey, contactValue );
                         registrationRow.insertKeyAndValue( expiresKey, expiresValue );
                         registrationRow.insertKeyAndValue( qvalueKey, qvalueValue );
                         registrationRow.insertKeyAndValue( instanceIdKey, instanceIdValue );
                         registrationRow.insertKeyAndValue( gruuKey, gruuValue );
                         registrationRow.insertKeyAndValue( pathKey, pathValue );
                         registrationRow.insertKeyAndValue( contactInstrumentKey, contactInstrumentValue );

                         registrations.addValue( registrationRow );
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
               int spreadExpirationTime = 0;

               // Increment the update number so as to assign a single fresh update
               // number to all the database changes we're about to make.  In some
               // cases we might not actually make any changes.  That's OK, having
               // an update number with no associated changes won't break anything.
               setDbUpdateNumber(mDbUpdateNumber + 1);

                if ( removeAll )
                {
                    // Contact: * special case
                    //  - request to remove all bindings for toUrl
                    if (   isExpiresheader
                        && 0 == commonExpires
                        && 1 == contactIndexCount
                        )
                    {
                        // Expires: 0 && Contact: * - clear all contacts
                        imdb->expireAllBindings( toUrl
                                                ,registerCallidStr
                                                ,registerCseqInt
                                                ,timeNow
                                                ,primaryName()
                                                ,mDbUpdateNumber
                                                );
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
                    int numRegistrations = registrations.getSize();

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

                        for ( int i = 0; i<numRegistrations; i++ )
                        {
                            UtlHashMap record;
                            registrations.getIndex( i, record );

                            int expires = ((UtlInt*)record.findValue(&gExpiresKey))->getValue();
                            UtlString contact(*((UtlString*)record.findValue(&gContactKey)));

                            int expirationTime;
                            if ( expires == 0 )
                            {
                                // Unbind this binding
                                //
                                // To cancel a contact, we expire it one second ago.
                                // This allows it to stay in the database until the
                                // explicit cleanAndPersist method cleans it out (which
                                // will be when it is more than one maximum registration
                                // time in the past).
                                // This prevents the problem of an expired registration
                                // being recreated by an old REGISTER request coming in
                                // whose cseq is lower than the one that unregistered it,
                                // which, if we had actually removed the entry would not
                                // be there to compare the out-of-order message to.
                                expirationTime = timeNow - 1;

                                OsSysLog::add( FAC_SIP, PRI_DEBUG,
                                              "SipRegistrarServer::applyRegisterToDirectory "
                                              "- Expiring map '%s'->'%s'",
                                              registerToStr.data(), contact.data()
                                              );
                            }
                            else
                            {
                                // expires > 0, so add the registration
                                expirationTime = (  expires < spreadExpirationTime
                                                  ? expires
                                                  : spreadExpirationTime
                                                  ) + timeNow;

                                OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                              "SipRegistrarServer::applyRegisterToDirectory - "
                                              "Adding map '%s'->'%s' "
                                              "expires %d (now+%d)",
                                              registerToStr.data(), contact.data(),
                                              expirationTime, expires);
                            }

                            UtlString qvalue(*(UtlString*)record.findValue(&gQvalueKey));
                            UtlString instance_id(*(UtlString*)record.findValue(&gInstanceIdKey));
                            UtlString gruu(*(UtlString*)record.findValue(&gGruuKey));
                            UtlString pathValue(*(UtlString*)record.findValue(&gPathKey));
                            UtlString contact_instrument(*(UtlString*)record.findValue(&gContactInstrumentKey));

                            imdb->updateBinding( toUrl, contact, qvalue
                                                ,registerCallidStr, registerCseqInt
                                                ,expirationTime
                                                ,instance_id
                                                ,gruu
                                                ,pathValue
                                                ,primaryName()
                                                ,mDbUpdateNumber
                                                ,contact_instrument
                                                );

                        } // iterate over good contact entries

                        // If there were any bindings not dealt with explicitly in this
                        // message that used the same callid, then expire them.
                        imdb->expireOldBindings( toUrl, registerCallidStr, registerCseqInt,
                                                 timeNow, primaryName(),
                                                 mDbUpdateNumber );
                    }
                    else
                    {
                        OsSysLog::add( FAC_SIP, PRI_ERR
                                      ,"SipRegistrarServer::applyRegisterToDirectory "
                                      "contact count mismatch %d != %d"
                                      ,contactIndexCount, numRegistrations
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

                    // something changed - garbage collect and persist the database
                    scheduleCleanAndPersist();

                    // give each RegisterPlugin a chance to do its thing
                    PluginIterator plugins(*mpSipRegisterPlugins);
                    RegisterPlugin* plugin;
                    while ((plugin = static_cast<RegisterPlugin*>(plugins.next())))
                    {
                       plugin->takeAction(registerMessage, spreadExpirationTime, mSipUserAgent );
                    }

                    // if replication is configured, then trigger replication
                    if (mRegistrar.isReplicationConfigured())
                    {
                       RegistrarSync* sync = mRegistrar.getRegistrarSync();
                       assert(sync);
                       sync->sendUpdates();
                    }
                }
            }
        }
    }
    else
    {
        OsSysLog::add( FAC_SIP, PRI_WARNING,
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


Int64
SipRegistrarServer::applyUpdatesToDirectory(
   const UtlSList& updates,       ///< list of updates to apply
   UtlString* errorMsg)           ///< fill in the error message on failure
{
   Int64 maxUpdateNumber; // to be returned

   // We need an error string buffer even if the caller didn't provide one.
   UtlString altErrorMsg;
   errorMsg = errorMsg ? errorMsg : &altErrorMsg;

   // Critical Section here
   OsLock lock(sLockMutex);

   // Pointer to the registration DB.
   RegistrationDB* imdb = mRegistrar.getRegistrationDB();
   // Set to true if an error is found in the updates list.
   bool error_found = false;
   // Record the peer that is the primary for these updates.
   // (The initialization is to keep the compiler from complaining.
   // The only way that 'peer' can not be assigned a value in the following
   // loop is if 'updates' is empty, in which case, the value of 'peer'
   // will not be used.)
   RegistrarPeer* peer = NULL;

   {
      // Loop over the updates and check that they are all for the same primary registrar.
      UtlSListIterator updateIter(updates);
      RegistrationBinding* reg;
      UtlString primary;
      UtlString emptyPrimary;
      const UtlString myPrimary(mRegistrar.primaryName());

      bool first_time = true;
      while (   !error_found
             && (reg = dynamic_cast<RegistrationBinding*>(updateIter())))
      {
         if (first_time)
         {
            // If this is the first element of the list...
            first_time = false;
            // Get the primary registrar name (or NULL) from the first update and save it
            // to compare with the remaining updates.
            if (reg->getPrimary() != NULL)
            {
               primary = *(reg->getPrimary());
            }
            // Check that the primary is known -- either this registrar, or a peer registrar.
            if (!primary.isNull() && primary.compareTo(myPrimary) != 0)
            {
               peer = mRegistrar.getPeer(primary);
               if (peer == NULL)
               {
                  errorMsg->append("update with unknown primary: updateNumber=");
                  errorMsg->appendNumber(reg->getUpdateNumber(), "%0#16" FORMAT_INTLL "x");
                  errorMsg->append(", primary='");
                  errorMsg->append(primary);
                  errorMsg->append("'");
                  OsSysLog::add(FAC_SIP, PRI_ERR, "SipRegistrarServer::applyUpdatesToDirectory %s",
                                errorMsg->data());

                  error_found = true;
               }
            }
            else
            {
               // The primary is this registrar, so set 'peer' to NULL to indicate that.
               peer = NULL;
            }
         }
         else
         {
            // If this is a later element of the list...
            // Make sure that all updates are for a single primary.
            const UtlString* nextPrimary = reg->getPrimary() ? reg->getPrimary() : &emptyPrimary;
            if (primary.compareTo(*nextPrimary) != 0)
            {
               errorMsg->append("updates with mixed primaries: updateNumber=");
               errorMsg->appendNumber(reg->getUpdateNumber(), "%0#16" FORMAT_INTLL "x");
               errorMsg->append(", primary of element 0='");
               errorMsg->append(primary);
               errorMsg->append("', primary of element ");
               errorMsg->appendNumber((int) updates.index(reg));
               errorMsg->append("='");
               errorMsg->append(nextPrimary->data());
               errorMsg->append("'");

               OsSysLog::add(FAC_SIP, PRI_ERR, "SipRegistrarServer::applyUpdatesToDirectory %s",
                             errorMsg->data());

               error_found = true;
            }
         }
      }
   }

   // If the updates pass the checks, apply them.
   if (error_found)
   {
      // Set the return value to -1.
      maxUpdateNumber = -1;
   }
   else
   {
      maxUpdateNumber = peer->receivedFrom(); // in case there are no updates (should not happen)

      UtlSListIterator updateIter(updates);
      RegistrationBinding* reg;

      // Loop through the list of updates.
      while ((reg = dynamic_cast<RegistrationBinding*>(updateIter())))
      {
         /* Apply one update.  Record the return value (the current
          * max known update number for the registrar in question).
          * The last return value will be the final max known update
          * number and will be the return value of this function.
          */
         maxUpdateNumber = updateOneBinding(reg, peer, imdb);
      }

      // Garbage-collect and persist the database.
      scheduleCleanAndPersist();
   }

   /* Return the final max known update for the peer registrar,
    * or -1 if an error was found.
    */
   return maxUpdateNumber;
}


Int64 SipRegistrarServer::updateOneBinding(
   RegistrationBinding* reg,
   RegistrarPeer* peer,    // NULL if it's the local registrar
   RegistrationDB* imdb)
{
   assert(reg);
   assert(imdb);

   // Don't require updateNumbers to be in order because when pulling updates,
   // updateNumber order is not guaranteed.

   // Update the registrar state and the binding
   Int64 updateNumber = reg->getUpdateNumber();

   // The return value is the max update number that we have seen for this registrar.
   Int64 maxUpdateNumber = updateNumber;

   if (peer != NULL)
   {
      // This update is for a peer.  If the updateNumber is bigger than previous
      // updateNumbers, then increase peerReceivedDbUpdateNumber accordingly.
      Int64 receivedFrom = peer->receivedFrom();
      if (updateNumber > receivedFrom)
      {
         peer->setReceivedFrom(updateNumber);
      }
      else
      {
         maxUpdateNumber = receivedFrom;
      }
   }
   else
   {
      // This is a local update pulled from a peer.  If the updateNumber is bigger
      // than previous updateNumbers, then increase mDbUpdateNumber accordingly.
      if (updateNumber > mDbUpdateNumber)
      {
         setDbUpdateNumber(updateNumber);
      }
      else
      {
         maxUpdateNumber = mDbUpdateNumber;
      }
   }

   // Check callId/cseq and accept only updates that are in sequence.
   if (imdb->isOutOfSequence(*reg->getUri(), *reg->getCallId(), reg->getCseq()))
   {
      // this can happen in an HA configuration when the peer didn't get the
      // update and tries to sync an old registration.
      OsSysLog::add(FAC_SIP, PRI_WARNING,
                    "SipRegistrarServer::updateOneBinding request out of order\n"
                    "  To: '%s'\n"
                    "  Contact: '%s'\n"
                    "  Call-Id: '%s'\n"
                    "  Cseq: %d",
                    reg->getUri()->toString().data(),
                    reg->getContact()->data(),
                    reg->getCallId()->data(),
                    reg->getCseq());
   }
   else // registration is newer than what we have, so store it
   {
      imdb->updateBinding(*reg);

      // Regenerate reg event information.
      RegisterEventServer* s = mRegistrar.getRegisterEventServer();
      if (s)
      {
         const Url* aor = reg->getUri();
         UtlString aor_string;
         aor->getUri(aor_string);
         s->generateAndPublishContent(aor_string, *aor, *reg->getInstrument());
      }
   }

   return maxUpdateNumber;
}


//functions
UtlBoolean
SipRegistrarServer::handleMessage( OsMsg& eventMessage )
{
    int msgType = eventMessage.getMsgType();
    int msgSubType = eventMessage.getMsgSubType();

    UtlBoolean handled = FALSE;

    // Timer event
    if(   msgType    == OsMsg::OS_EVENT
       && msgSubType == OsEventMsg::NOTIFY
       )
    {
       OsSysLog::add( FAC_SIP, PRI_DEBUG, "SipRegistrarServer::handleMessage() - "
                     "unknown timer expiration" );

        handled = TRUE;
    }
    // SIP message event
    else if (msgType == OsMsg::PHONE_APP)
    {
        OsSysLog::add( FAC_SIP, PRI_DEBUG, "SipRegistrarServer::handleMessage() - "
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
              if (OsSysLog::willLog(FAC_SIP, PRI_INFO))
              {
                 UtlString alias;
                 toUri.getHostWithPort(alias);
                 OsSysLog::add( FAC_SIP, PRI_INFO, "SipRegistrarServer::handleMessage() "
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
                        OsSysLog::add( FAC_SIP, PRI_DEBUG, "SipRegistrarServer::handleMessage() - "
                               "contact successfully added");

                        //create response - 200 ok reseponse
                        finalResponse.setOkResponseData(&message);

                        // get the call-id from the register message for context test below
                        UtlString registerCallId;
                        message.getCallIdField(&registerCallId);

                        //get all current contacts now for the response
                        ResultSet registrations;

                        mRegistrar.getRegistrationDB()->
                           getUnexpiredContactsUser(toUri,
                                                    timeNow,
                                                    registrations);

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
                        int commonExpirationTime = 0;
                        int numRegistrations = registrations.getSize();
                        for ( int i = 0 ; i<numRegistrations; i++ )
                        {
                          UtlHashMap record;
                          registrations.getIndex( i, record );

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
                          UtlString* contactCallId
                             = dynamic_cast<UtlString*>(record.findValue(&gCallidKey));
                          if (   REGISTER_QUERY == applyStatus
                              || (   contactCallId
                                  && registerCallId.isEqual(contactCallId)
                                  )
                              )
                          {
                            UtlString contactKey("contact");
                            UtlString expiresKey("expires");
                            UtlString qvalueKey("qvalue");
                            UtlString instanceIdKey("instance_id");
                            UtlString gruuKey("gruu");
                            UtlString contact = *((UtlString*)record.findValue(&contactKey));
                            UtlString qvalue = *((UtlString*)record.findValue(&qvalueKey));
                            int expires = ((UtlInt*)record.findValue(&expiresKey))->getValue();
                            expires = expires - timeNow;

                            OsSysLog::add( FAC_SIP, PRI_DEBUG,
                                          "SipRegistrarServer::handleMessage - "
                                          "processing contact '%s'", contact.data());
                            Url contactUri( contact );

                            UtlString expiresStr;
                            expiresStr.appendNumber(expires);

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
                            if ( !qvalue.isNull() && qvalue.compareTo(SPECIAL_IMDB_NULL_VALUE)!=0 )
                            {
                               OsSysLog::add( FAC_SIP, PRI_DEBUG,
                                             "SipRegistrarServer::handleMessage - "
                                             "adding q '%s'", qvalue.data());

                               //check if q value is numeric and between the range 0.0 and 1.0
                               RegEx qValueValid(RegQValue);
                               if (qValueValid.Search(qvalue.data()))
                               {
                                  contactUri.setFieldParameter(SIP_Q_FIELD, qvalue);
                               }
                            }

                            // Add the +sip.instance and gruu
                            // parameters if an instance ID is recorded.
                            UtlString* instance_id =
                               dynamic_cast<UtlString*> (record.findValue(&instanceIdKey));

                            OsSysLog::add( FAC_SIP, PRI_DEBUG,
                                          "SipRegistrarServer::handleMessage"
                                          " - value %p, instance_id %p, instanceIdKey = '%s'",
                                           record.findValue(&instanceIdKey),
                                           instance_id, instanceIdKey.data());
                            if (instance_id && !instance_id->isNull() &&
                                instance_id->compareTo(SPECIAL_IMDB_NULL_VALUE) !=0 )
                            {
                               OsSysLog::add( FAC_SIP, PRI_DEBUG,
                                             "SipRegistrarServer::handleMessage"
                                             " - add instance '%s'",
                                             instance_id->data());
                               contactUri.setFieldParameter("+sip.instance",
                                                            *instance_id);

                               UtlString* gruu =
                                  dynamic_cast<UtlString*> (record.findValue(&gruuKey));
                               // Only add the "gruu" parameter if the GRUU is
                               // non-null and the request includes "Supported:
                               // gruu".
                               if (requestSupportsGruu && !gruu->isNull())
                               {
                                  // Prepend "sip:" to the GRUU, since it is stored
                                  // in the database in identity form.
                                  UtlString temp("sip:");
                                  temp.append(*gruu);
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

                            finalResponse.setContactField(contactUri.toString(), i);
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
                           expiresString.appendNumber(commonExpirationTime);
                           finalResponse.setHeaderValue(SIP_EXPIRES_FIELD, expiresString, 0);
                        }

                        // Add the testing contact, if it is set.
                        if (!mAdditionalContact.isNull())
                        {
                           finalResponse.setContactField(mAdditionalContact,
                                                         numRegistrations);
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
                       OsSysLog::add( FAC_SIP, PRI_ERR,
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

           OsSysLog::add(FAC_AUTH, PRI_WARNING,
                         "SipRegistrarServer::handleMessage('%s' == '%s') Invalid",
                         requestedDomain.data(), lookupDomain.data()) ;

           UtlString responseText;
           responseText.append("Domain '");
           responseText.append(requestedDomain);
           responseText.append("' is not valid at this registrar");
           finalResponse.setResponseData(&message, SIP_NOT_FOUND_CODE, responseText.data() );
        }

        mSipUserAgent->setUserAgentHeader(finalResponse);

        if (OsSysLog::willLog(FAC_SIP, PRI_DEBUG))
        {
           UtlString finalMessageStr;
           ssize_t finalMessageLen;
           finalResponse.getBytes(&finalMessageStr, &finalMessageLen);
           OsSysLog::add( FAC_SIP, PRI_DEBUG, "\n----------------------------------\n"
                         "Sending final response\n%s", finalMessageStr.data());
        }

        mSipUserAgent->send(finalResponse);

        handled = TRUE;
    }
    else if ( msgType == OsMsg::OS_SHUTDOWN )
    {
       OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "SipRegistrarServer::handleMessage received shutdown request"
                     );
       OsTask::requestShutdown(); // tell OsServerTask::run to exit
       handled = TRUE;
    }
    else
    {
       OsSysLog::add(FAC_SIP, PRI_DEBUG,
                     "SipRegistrarServer::handleMessage unexpected message type %d/%d",
                     msgType, msgSubType
                     ) ;
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
        OsSysLog::add( FAC_AUTH, PRI_DEBUG, "SipRegistrarServer::isAuthorized "
                      "No Credential DB - request is always AUTHORIZED" );
        isAuthorized = TRUE;
    }
    else
    {
        // Realm and auth type should be default for server.
        // check if we requested authentication and this is the req with
        // authorization,validate the authorization
        OsSysLog::add( FAC_AUTH, PRI_DEBUG,
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
           OsSysLog::add( FAC_AUTH, PRI_DEBUG, "Message Authorization received: "
                    "reqRealm='%s', reqUser='%s', reqUserBase='%s', instrument='%s'",
                          requestRealm.data() , requestUser.data(),
                          requestUserBase.data(), instrument.data());

           UtlString qopType;

           if (mRealm.compareTo(requestRealm) ) // case sensitive check that realm is correct
           {
              OsSysLog::add(FAC_AUTH, PRI_DEBUG,
                            "SipRegistrarServer::isAuthorized "
                            "Realm does not match");
           }

           // validate the nonce
           else if (!mNonceDb.isNonceValid(requestNonce, callId, fromTag, mRealm, mNonceExpiration))
           {
               OsSysLog::add(FAC_AUTH, PRI_INFO,
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
               OsSysLog::add(FAC_AUTH, PRI_INFO,
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
                if (CredentialDB::getInstance()->getCredential( toUri
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
                      OsSysLog::add(FAC_AUTH, PRI_DEBUG,
                                    "SipRegistrarServer::isAuthorized "
                                    "response auth hash matches");
                    }
                  else
                    {
                      OsSysLog::add(FAC_AUTH, PRI_ERR,
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
                    OsSysLog::add(FAC_AUTH, PRI_ERR,
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


const UtlString& SipRegistrarServer::primaryName() const
{
   return mRegistrar.primaryName();
}

Int64 SipRegistrarServer::getMaxUpdateNumberForRegistrar(const char* primaryName) const
{
   // If replication is not configured, then the primaryName will be empty, but it
   // should never be null.
   assert(primaryName != NULL);
   assert(!mRegistrar.isReplicationConfigured() || strlen(primaryName) > 0);

   // Critical Section here
   OsLock lock(sLockMutex);

   RegistrationDB* imdb = mRegistrar.getRegistrationDB();
   return imdb->getMaxUpdateNumberForRegistrar(primaryName);
}

/// Return true if there is a new update to send to the registrar, and return the update
bool SipRegistrarServer::getNextUpdateToSend(RegistrarPeer *peer,
                                             UtlSList&      bindings)
{
   assert(peer != NULL);
   bool isNewUpdate = false;

   // Critical Section here
   OsLock lock(sLockMutex);

   Int64 peerSentDbUpdateNumber = peer->sentTo();

   // This method must not be called until the peer's sentTo value has been initialized
   assert(peerSentDbUpdateNumber >= 0);

   if (mDbUpdateNumber > peerSentDbUpdateNumber)    // if there might be updates to send
   {
      // Get the next update belonging to us (we're primary) that we haven't sent to
      // registrarName yet.
      RegistrationDB* imdb = mRegistrar.getRegistrationDB();
      int numBindings = imdb->getNextUpdateForRegistrar(primaryName(),
                                                        peerSentDbUpdateNumber,
                                                        bindings);
      assert(static_cast<int>(bindings.entries()) == numBindings);
      if (numBindings > 0)
      {
         isNewUpdate = true;
      }
   }

   return isNewUpdate;
}

/// Schedule garbage collection and persistence of the registration database
void SipRegistrarServer::scheduleCleanAndPersist()
{
   RegistrarPersist* persistThread = mRegistrar.getRegistrarPersist();
   assert(persistThread);
   persistThread->scheduleCleanAndPersist();
}

/// Garbage-collect and persist the registration database
void SipRegistrarServer::cleanAndPersist()
{
   RegistrationDB* regDb = mRegistrar.getRegistrationDB();
   int timeNow = (int)OsDateTime::getSecsSinceEpoch();
   int oldestTimeToKeep = timeNow - (  mNormalExpiryIntervals.mMaxExpiresTime < MAX_RETENTION_TIME
                                     ? mNormalExpiryIntervals.mMaxExpiresTime : MAX_RETENTION_TIME );

   // Send reg event notices for any expired rows, including the rows we
   // are about to delete.  It's possible that there hasn't been a
   // notify telling that they're terminated yet, and we have to make
   // sure to generate one before the row is deleted.
   RegisterEventServer* s = mRegistrar.getRegisterEventServer();
   if (s)
   {
      UtlSList aors;          // AOR name-addrs of the expired bindings.
      regDb->getAllOldBindings(timeNow, aors);
      UtlSListIterator itor(aors);
      UtlString* aor;
      UtlString* instrument;
      while ((aor = dynamic_cast <UtlString*> (itor())))
      {
         Url aor_uri(*aor, FALSE); // Parse name-addr format.
         UtlString aor_addr;
         aor_uri.getUri(aor_addr); // Generate addr-spec (URI) format.
         instrument = dynamic_cast <UtlString*> (itor());
         s->generateAndPublishContent(aor_addr, aor_uri, *instrument);
      }

      // Free the strings.
      aors.destroyAll();
   }

   // Critical section here
   OsLock lock(sLockMutex);

   regDb->cleanAndPersist(oldestTimeToKeep);
}

/// Get the largest update number in the local database for this registrar as primary
Int64 SipRegistrarServer::getDbUpdateNumber() const
{
   // Critical Section here
   OsLock lock(sLockMutex);

   return mDbUpdateNumber.getValue();
}

/// Reset the upper half of the DbUpdateNumber to the epoch time.
void SipRegistrarServer::resetDbUpdateNumberEpoch()
{
   unsigned long timeNow = OsDateTime::getSecsSinceEpoch();
   Int64 newEpoch;
   newEpoch = timeNow;
   newEpoch <<= 32;

   // Check that the first update number for the new epoch is greater than
   // the currently reported update number.  It always will be, absent
   // severe clock skew, but if it is not, the system will behave worse
   // if we set the update number downward.  So if the new epoch is greater
   // that what has already been used, reset the update number.
   Int64 current = getDbUpdateNumber();
   if (newEpoch >= current)
   {
      { // lock before changing the epoch update number
         OsLock lock(sLockMutex);

         setDbUpdateNumber(newEpoch);
      } // release lock before logging

      OsSysLog::add(FAC_SIP, PRI_INFO,
                    "SipRegistrarServer::resetDbUpdateNumberEpoch to %" FORMAT_INTLL "x",
                    newEpoch
         );
   }
   else if (newEpoch + (((Int64) (15 * 60)) << 32) /* 15 minutes */ < current)
   {
      // If the new epoch number is more than 15 minutes less than the
      // highest update number that has already been used,
      // warn the user that there may be problems -- the update mechanism will
      // work OK, but it's likely that there are future-dated registrations
      // in the database, and they will take a long time to time out.
      OsSysLog::add(FAC_SIP, PRI_CRIT,
                    "SipRegistrarServer::resetDbUpdateNumberEpoch "
                    "the current epoch is %" FORMAT_INTLL "x, "
                    "but the last update was %" FORMAT_INTLL "x, "
                    "which is %" FORMAT_INTLL "d seconds in the future.  "
                    "Obsolete registrations could persist until that time.",
                    newEpoch, current, (current - newEpoch) >> 32);
   }
}

/// Recover the DbUpdateNumber from the local database
void SipRegistrarServer::restoreDbUpdateNumber()
{
   // Recover and return the largest update number from the database for this primary.
   // Call this method after reading the configuration, otherwise the primaryName will
   // be empty.
   setDbUpdateNumber(getMaxUpdateNumberForRegistrar(mRegistrar.primaryName()));
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

   OsSysLog::add(FAC_SIP, PRI_ERR,
                 "RegisterPlugin::takeAction not resolved by configured hook"
                 );
}
