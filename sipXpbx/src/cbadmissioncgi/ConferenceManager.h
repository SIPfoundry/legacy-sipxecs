//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef CONFERENCEMANAGER_H
#define CONFERENCEMANAGER_H

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsStatus.h"
#include "os/OsMutex.h"
#include "os/OsSysLog.h"
#include "utl/UtlSortedList.h"
#include "utl/UtlHashMap.h"
#include "utl/UtlHashBag.h"

// DEFINES
/* Digest Authenticaton helper */
#define MD5_DIGEST_LENGTH               32

/* Greeting files */
#define STANDARD_GREETING_FILE          "cb_welcome.wav"

/* Used in dynamically generating the URLs */
#define URL_SEPARATOR                   "/"
#define PARAMETER_SEPARATOR             ";"
#define ORGANIZER_ROLE                  "role=organizer"
#define PARTICIPANT_ROLE                "role=participant"
#define ADMIT_SIGNATURE                 "admitSignature="

/* Apache Aliases */
#define PROMPT_ALIAS                    "stdprompts"
#define CB_SCRIPTS_ALIAS                "cb_vxml"

/** Name of certain directories of interest */
#define MAILBOX_DIR                     "mailstore"
#define STD_PROMPTS_DIR                 "stdprompts"
#define CUSTOM_PROMPTS_DIR              "prompts"

/** Log levels */
#define LOG_LEVEL_DEBUG                 "DEBUG"
#define LOG_LEVEL_INFO                  "INFO"
#define LOG_LEVEL_NOTICE                "NOTICE"
#define LOG_LEVEL_WARNING               "WARNING"
#define LOG_LEVEL_ERROR                 "ERR"
#define LOG_LEVEL_CRITICAL              "CRIT"
#define LOG_LEVEL_ALERT                 "ALERT"
#define LOG_LEVEL_EMERGENCY             "EMERG"


/** Customizable parameters defined in cbadmission.xml file */
#define PARAM_LOG_LEVEL                 "cbadmission-cgi-log-level"
#define PARAM_MEDIASERVER_ROOT          "mediaserver-root"
#define PARAM_MEDIASERVER_URL           "mediaserver-url"
#define PARAM_SECURE_MEDIASERVER_URL    "mediaserver-url-secure"


// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class Url;
class UtlHashMap;
class TiXmlNode;


/**
 * Conference Manager class.  This class is responsible for
 * admitting the callers to a given conference.
 *
 */
class ConferenceManager
{

public:

    /**
     * Singleton Accessor
     *
     * @return
     */
    static ConferenceManager* getInstance();

    /**
     * Virtual Destructor
     */
    virtual ~ConferenceManager();

    /** Method to get the base URL of media server (like http://mediaserver:8090/) */
    OsStatus getMediaserverURL( UtlString& rMediaserverUrl ) const;

    /** Method to get the base URL of secure media server (like https://mediaserver:8091/) */
    OsStatus getMediaserverSecureURL ( UtlString& rMediaserverSecureUrl ) const ;

    /** Method to get the base file URL of IVR prompts (like file:///usr/share/www/doc) */
    OsStatus getIvrPromptURL( UtlString& rIvrPromptUrl ) const;

    /**
     * Method to validate the accesscode and to login the user.
     *
     * @param confid
     * @param password
     * @param rConferenceUrl
     * 
     * @return
     */
    OsStatus doLogin ( const UtlString& contact,
                       const UtlString& confid,
                       const UtlString& password,
                       UtlString& rConferenceUrl ) ;

    /** Helper method to return a url from an identity or extension */
    OsStatus getWellFormedURL (
        const UtlString& inputString,
        Url&  rUrl ) const;

    /**
     * Utility functiona to convert between persistent form of a url
     * and a form suitable for constructing a url
     *
     * @return OS_SUCCESS or OS_FAILED
     */
    static OsStatus convertUrlStringToXML ( UtlString& value );
    static OsStatus convertXMLStringToURL ( UtlString& value );


    /** Utility function for creating the vxml headers, including content-length. **/
    static OsStatus getResponseHeaders(int contentLength, UtlString& responseHeaders);

    /** Common method for retrieving the value of parameters defined in
     *  voicemail.xml.in file.
     *  TBD: Change the return parameter type from UtlString to
     *  something like an object so that it can be used for all parameters.
     */
    OsStatus getCustomParameter( const UtlString& paramName,
                                 UtlString& rStrValue) const;

protected:
    /**
     * Singleton Protected Ctor
     */
    ConferenceManager();

private:
    /**
     * Parses an XML file containing the settings for the ConferenceManager
     *
     * @param configFile
     *
     * @return
     */
    OsStatus parseConfigFile (const UtlString& configFileName );


    /**
     *
     * @param docxcv
     * @param key
     * @param value - output
     *
     * @return OS_SUCCESS or OS_FAILURE
     */
    OsStatus getConfigValue (
        const TiXmlNode& node,
        const UtlString& key,
        UtlString& value ) const;

    /**
     *
     * @param node
     * @param key
     * @param newValue
     *
     * @return
     */
    OsStatus setConfigValue (
        const TiXmlNode& node,
        const UtlString& key,
        const UtlString& newValue ) const;

    OsStatus parseConferenceFile (
        const UtlString& confId,
        UtlString& organizerCode,
        UtlString& participantCode,
        UtlString& secretValue,
        UtlString& bridgeUrl ) const;
        
    OsStatus getAdmitSignature (
        const UtlString& secretValue,
        const UtlString& conferenceAOR,
        const UtlString& contact,
        const UtlString& role,
        UtlString& signature) const;


    // Singleton instance
    static ConferenceManager* spInstance;

    // Exclusive binary lock
    static OsMutex sLockMutex;

    // these are read in from the configuraton file
    UtlString m_defaultDomain;
    UtlString m_defaultRealm;
    UtlString m_mailstoreRoot;
    UtlString m_mediaserverRoot;
    UtlString m_mediaserverUrl;
    UtlString m_mediaserverSecureUrl;
    UtlString m_fullMediaserverUrl;
    UtlString m_fullMediaserverSecureUrl;
    UtlString m_ivrPromptUrl;
    UtlString m_defaultMessageSubject;
    UtlString m_defaultUserPassword;
    UtlString m_logLevel;
};

#endif //CONFERENCEMANAGER_H
