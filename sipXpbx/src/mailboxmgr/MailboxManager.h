//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef MAILBOXMANAGER_H
#define MAILBOXMANAGER_H

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
#define STANDARD_GREETING_FILE              "standard.wav"
#define OUTOFOFFICE_GREETING_FILE           "outofoffice.wav"
#define EXTENDED_ABSENCE_GREETING_FILE      "extendedabs.wav"
#define DEFAULT_STANDARD_GREETING_FILE      "standard-default.wav"
#define DEFAULT_OUTOFOFFICE_GREETING_FILE   "outofoffice-default.wav"
#define DEFAULT_EXTENDED_ABS_GREETING_FILE  "extendedabsence-default.wav"
#define RECORDED_NAME_FILE                  "name.wav"

/* Greeting Types */
#define STANDARD_GREETING               "standard"
#define OUTOFOFFICE_GREETING            "outofoffice"
#define EXTENDED_ABSENCE_GREETING       "extendedabsence"
#define RECORDED_NAME                   "name"
#define DEFAULT_STANDARD_GREETING       "defaultstandard"
#define DEFAULT_OUTOFOFFICE_GREETING    "defaultoutofoffice"
#define DEFAULT_EXTENDED_ABS_GREETING   "defaultextendedabsence"


/**
 * Indicator for no active greeting.
 * Stored in the mailbox preferences file.
 */
#define ACTIVE_GREETING_NOT_SET         "none"

/* Mailbox folder summary file */
#define FOLDER_SUMMARY_FILE             "summary.xml"

/* Used in dynamically generating the URLs */
#define URL_SEPARATOR   "/"

/* Apache Aliases */
#define PROMPT_ALIAS                    "stdprompts"
#define MEDIASERVER_ROOT_ALIAS          "mailboxes"
#define VOICEMAIL_SCRIPTS_ALIAS         "vm_vxml"
#define AUTOATTENDANT_SCRIPTS_ALIAS     "aa_vxml"

/** Name of certain directories of interest */
#define MAILBOX_DIR                     "mailstore"
#define STD_PROMPTS_DIR                 "stdprompts"
#define CUSTOM_PROMPTS_DIR              "prompts"
#define ORGANIZATION_PREFS_FILE         "organizationprefs.xml"
#define MAILBOX_PREFS_FILE              "mailboxprefs.xml"


/** Standard folder display names */
#define INBOX_DISPLAY_NAME              "Inbox";
#define SAVED_DISPLAY_NAME              "Saved";
#define DELETED_DISPLAY_NAME            "Deleted" ;

/**
 * Flag to indicate that the state of all the messages in the folder
 * should be updated.
 * Used in updateMessageStates() method.
 */
#define UPDATE_ALL_MSG_STATES           "-1"
#define REFRESH_ALL_MSG_STATES           "-2"
#define MESSAGE_DELIMITER               " "
#define DEFAULT_MESSAGE_SUBJECT         "Voice Message"

/** Log levels */
#define LOG_LEVEL_DEBUG                 "DEBUG"
#define LOG_LEVEL_INFO                  "INFO"
#define LOG_LEVEL_NOTICE                "NOTICE"
#define LOG_LEVEL_WARNING               "WARNING"
#define LOG_LEVEL_ERROR                 "ERR"
#define LOG_LEVEL_CRITICAL              "CRIT"
#define LOG_LEVEL_ALERT                 "ALERT"
#define LOG_LEVEL_EMERGENCY             "EMERG"


/** Customizable parameters defined in voicemail.xml file */
#define PARAM_LOG_LEVEL                 "voicemail-cgi-log-level"
#define PARAM_DEFAULT_MSG_SUBJECT       "default-message-subject"
#define PARAM_REFRESH_INTERVAL          "webpage-refresh-interval"
#define PARAM_MIN_MESSAGE_LENGTH        "min-message-length"
#define PARAM_MSG_BLOCK_SIZE            "web-message-block-size"
#define PARAM_DEFAULT_DOMAIN            "default-domain"
#define PARAM_DEFAULT_REALM             "default-realm"
#define PARAM_MAILSTORE_ROOT            "mailstore-root"
#define PARAM_MEDIASERVER_ROOT          "mediaserver-root"
#define PARAM_MEDIASERVER_URL           "mediaserver-url"
#define PARAM_SECURE_MEDIASERVER_URL    "mediaserver-url-secure"
#define PARAM_VOICEMAIL_INFO_PLAYBACK   "voicemail-info-playback"
#define PARAM_IVR_PROMPT_URL            "ivr-prompt-url"

/** System administration permissions */
#define RECORD_SYSTEM_PROMPTS_PERMISSION    "RecordSystemPrompts"


/** Types of system prompts */
#define STANDARD_SYSTEM_GREETING            "standard"
#define AFTER_HOURS_SYSTEM_GREETING         "afterhours"
#define SPECIAL_OCCASION_SYSTEM_GREETING    "special"
#define GENERIC_SYSTEM_GREETING             "genericsystem"
#define GENERIC_AUTOATTENDANT_PROMPT        "genericautoattendant"
#define RECORDED_AUTOATTENDANT_PROMPT       "customautoattendant"
#define DISABLE_AUTOATTENDANT_PROMPT        "none"

/** System prompt filenames */
#define STANDARD_SYSTEM_GREETING_FILE           "standard_system_greeting.wav"
#define AFTER_HOURS_SYSTEM_GREETING_FILE        "afterhours_system_greeting.wav"
#define SPECIAL_OCCASION_SYSTEM_GREETING_FILE   "special_system_greeting.wav"
#define RECORDED_AUTOATTENDANT_PROMPT_FILE      "customautoattendant"
#define GENERIC_AUTOATTENDANT_PROMPT_FILE       "autoattendant.wav"
#define GENERIC_SYSTEM_GREETING_FILE            "welcome.wav"


/** Distribution filename */
#define DIST_LIST_NAME "distribution.xml"


/** Autoattendant schedule **/
#define AA_SCHEDULE_DIR "aa_vxml/"
#define HOLIDAY_MENU    "afterhour"
#define REGULAR_MENU    "operator"
#define AFTERHOUR_MENU  "afterhour"

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
class UtlDList;
class UtlHashMap;
class MessageIDGenerator;
class NotificationHelper;
class SIPXAuthHelper;

// Private class to contain organization pref info
class AutoAttendantSchedule
{
public:
    AutoAttendantSchedule();

    virtual ~AutoAttendantSchedule();
    
    UtlHashBag mHolidays;
    UtlString mHolidayMenu;

    UtlHashMap mRegularFromHour;
    UtlHashMap mRegularToHour;
    UtlString mRegularMenu;
    
    UtlString mAfterHourMenu;
};


/**
 * Mailbox Manager class.  This class is responsible for
 * loading and storing all of the mailboxes in the media server.
 *
 * @author John P. Coffey
 * @version 1.0
 */
class MailboxManager
{

friend class AutoAttendantSchedule;

public:
    // Used in GetMessageBlock
    // No need for an enum when it can take just two values.
    // A boolean flag should do the trick.
        enum DisplayOrder
        {
                DESCENDING,
        ASCENDING
        };

    /**
     * Singleton Accessor
     *
     * @return
     */
    static MailboxManager* getInstance();

    /**
     * Virtual Destructor
     */
    virtual ~MailboxManager();

    /** Delegated SaveMessage CGI/Helper implementation */
    OsStatus saveMessage (
        const Url& fromUrl,
        const UtlString& loginString,
        const UtlString& duration,
        const UtlString& timestamp,
        const char* data,
        const int& datasize,
        const UtlString& nextMessageID = "-1",
        const UtlBoolean& saveIfDataIsEmpty=FALSE,
        const UtlBoolean& sendEmail=TRUE) ;

    /** Method to get the base URL of media server (like http://mediaserver:8090/) */
    OsStatus getMediaserverURL( UtlString& rMediaserverUrl ) const;

    /** Method to get the base URL of secure media server (like https://mediaserver:8091/) */
    OsStatus getMediaserverSecureURL ( UtlString& rMediaserverSecureUrl ) const ;

    /** Method to get the base URL of media server (like http://mediaserver:6000/) */
    OsStatus getMediaserverURLForWeb( UtlString& rMediaserverUrl ) const;

    /** Method to get the base URL of media server (like http://mediaserver:6000/) */
    OsStatus getNonLocalhostMediaserverURL ( UtlString& rMediaserverUrl ) const;

    /** Method to get the base file URL of IVR prompts (like file:///usr/share/www/doc) */
    OsStatus getIvrPromptURL( UtlString& rIvrPromptUrl ) const;

    /** Method to get the name of AA menu based on current local time */
    OsStatus getTimeBasedAAName( UtlString& rScheduleName, UtlString& rLocalTime, UtlString& rAAName ) const;

    /**
     * Method to validate the extn and pin and to login the user.
     *
     * @param userid
     * @param password
     * @param rMailboxIdentity
     * @param rExtension
     *
     * @return
     */
    OsStatus doLogin ( const UtlString& userid,
                       const UtlString& password,
                       UtlString& rMailboxIdentity,
                       UtlString& rExtension ) ;

    /**
     * Reads the summary.xml files in user's inbox and saved folders.
     *
     * @param mailboxIdentity
     * @param folderName
     * @param rUnheardCount
     * @param rTotalCount
     *
     * @return
     */
    OsStatus getMailboxStatus(  const UtlString& mailboxIdentity,
                                const UtlString& folderName,
                                UtlString& rUnheardCount,
                                UtlString& rTotalCount ) const;

    /**
     * This method does many things, firstly is attempts to resolve whether the
     * identityOrExtension input parameter is a true identity or an Extension. To
     * resolve this, the quueries the credential IMDB. If a match is found here,
     * the identityOrExtension is really an identity and not an extension.
     *
     * Once we resolve to a credential identity we match this in permissions database
     * to ensure that the identity has the 'Voicemail' permission turned on.  Optionaly
     * we search the AliasDB for the Extension corresponding to this identity.  This
     * is becuase higher level VXML scripts need to indicate user at extension XYZ is
     * not available, please leave a message... (if they have not recorded a name or greeting)
     *
     * Finally as part of a lazy creation algorithm, if the mailbox folders have not been
     * created on disk, they are created using the template structure defined
     * in voicemail.xml.
     *
     * If the poassed in parameter identityOrExtension is an extension. The algorithm
     * to determine the
     * In order for an unambiguious identity result, this alias must correspond to
     * a single credential identity.  If there are multiple identities associated
     * with the alias the alias is invalid for voicemail and OS_FAILED is returned
     * If we have a single identity associated with it, we return to the algorithm in first paragraph
     * (it's actually a do while (!failed and !resolved ) loop).
     *
     * @param identityOrExtension   this is either an identity or an extension
     * @param resolveAliasFlag      optionally resolve the best alias (preferably numeric)
     * @param rMailboxIdentity      the resolved maibox identity (adfter searching the DBs)
     *                              this parameter is only returned if the resolveAliasFlag
     *                              is set to true
     * @param rExtension            the resolved best alias (preferably digits)
     *
     * @return OsStatus         OS_SUCCESS or OS_FAILED
     */
    OsStatus validateMailbox (
        const UtlString& identityOrExtension,
        const UtlBoolean& resolveExtensionFlag,
        const UtlBoolean& checkPermissions,
        UtlString& rMailboxIdentity,
        UtlString& rExtension ) ;

    /**
     * Retrieves messages in chunks.
         *
     * @param mailboxIdentity           Fully qualified mailboxIdentity id from where the
         *                                                              messages are retrieved.
     * @param msgBlock                          List of message ids and their descriptions.
     * @param category                          Type of messages to be retrieved.
         *                                                              Eg. unheard, heard, saved, etc.
     * @param blocksize                         number of messages retrieved (size of the chunk).
         *                                                              To retrieve all messages in the category, blocksize is set to -1
     * @param nextBlockHandle           ID of the last message in the previous block.
         *                                                              For the first block, nextBlockHandle will be set to -1.
     * @param endOfMessages                     Flag indicating if all the messages in this category
     *                                                          have been retrieved.
         * @param isFromWeb                 Indicates whether the user is loging in over the web or TUI
         * @param msgOrderDescending    Specifies the order in which messages will be returned.
         *                                                              If true, order is oldest to latest (ascending message ids).
         *
     *
     * @return
     */
    OsStatus getMessageBlock(
        const UtlString& mailboxIdentity,
        UtlDList& msgBlock,
        const UtlString& category,
        const int blocksize,
        const int nextBlockHandle,
        UtlBoolean& endOfMessages,
        const UtlBoolean& isFromWeb,
        const UtlBoolean& msgOrderDescending = FALSE ) const;

    OsStatus getMessageInfo (
        const UtlString& mailboxIdentity,
        const UtlString& folderName,
        const UtlString& messageId,
        UtlHashMap* msgInfoHashDictionary,
        const UtlBoolean& isFromWeb = FALSE
        ) const ;


    /** Gets the block handles for displaying appropriate links on the WebUI.
     *  When the contents of the folder is listed on the WebUI, if the number of messages
     *  is greater than the block size (set in voicemail.xml.in), then they are
     *  displayed on multiple pages and links are provided in the format "More Messages: 1 2 3"

     *  The block handle indicates the message id of the last message on the previous page.
     *  For example, if 00000010.wav was the last message on page 1, the link for page 2 will
     *  have 'nextblockhandle' parameter set to 0000010.wav and page 2 starts with 0000009.wav
     *
     *  @param  mailboxIdentity     Fully qualified mailbox identity
     *  @param  rNextBlockHandles   Sorted list of block handles. Filled on return.
     *  @param  category            Folder that is being viewed
     *  @param  blocksize           Number of messages displayed on one webpage.
     */
    OsStatus getMessageBlockHandles (
        const UtlString& mailboxIdentity,
        UtlSortedList& rNextBlockHandles,
        const UtlString& category,
        const int blocksize ) const;

    /**
     * Changes the state of the messages from 'unheard' to 'heard'.
     *
     * @param mailboxIdentity   Fully qualified mailboxIdentity id
         *                                                      from where the messages are retrieved.
     * @param category                  Folder name. Eg. inbox, saved, etc.
     * @param messageids                Space separated list of message ids whose state
         *                                                      has to be changed. To update all the messages in
         *                                                      the folder, messageids is set to -1.
     *
     * @return
     */
    OsStatus updateMessageStates(
        const UtlString& mailboxIdentity,
        const UtlString& category,
        const UtlString& messageids );

    /**
     * Moves messages from one folder to another.
     *
     * @param mailboxIdentity
     * @param fromFolder
     * @param toFolder
     * @param messageIds                space separated list of messages to be moved.
     * @param maintainstatus    flag to indicate if the 'unheard' status of the message
         *                                                      should be maintained. That is, if the .sta file should be moved too.
     *                                                  For the TUI, messages can be moved only after being heard.
     *                                                  Hence there is no need to maintain the 'unheard' status.
     *                                                  However, on the Web UI, messages can be moved without being heard
     *
     * @return
     */
    OsStatus moveMessages(
        const UtlString& mailboxIdentity,
        const UtlString& fromFolder,
        const UtlString& toFolder,
        const UtlString& messageIds,
        const UtlString& maintainstatus) ;

    /** Permanently deletes the messages in the 'Deleted' folder */
    OsStatus recycleDeletedMessages(const UtlString& mailboxIdentity,
                                    const UtlString& messageIds) ;

    /**
     * Delegated ForwardMessage CGI/Helper implementation.
     *
     * @param comments   Comments recorded by the mailboxIdentity owner.
     * @param commentsDuration
     *                   Length of comments in seconds.
     * @param commentsTimestamp
     *                   Date and time of recording the comments.
     * @param commentsSize
     *                   Size of comments in bytes.
     * @param fromMailboxIdentity
     *                   fully qualified mailboxIdentity id of the user forwarding the message.
     * @param fromFolder original location of the message.
     * @param messageId  id of the message being forwarded.
     * @param toExtension
     *
     * @return
     */
    OsStatus forwardMessages (
        const char* comments,
        const UtlString& commentsDuration,
        const UtlString& commentsTimestamp,
        const int& commentsSize,
        const Url& fromMailboxIdentity,
        const UtlString& fromFolder,
        const UtlString& messageId,
        const UtlString& toExtension );

        /**
     * Gets the URL of the active greeting.
     *
     * @param mailboxIdentity   Fully qualified mailbox identity.
     * @param rGreeting                 Contains the URL of the active greeting.
     *
     * @return  if greeting file was found, returns OS_SUCCESS.
         *                      else, returns OS_FAILED
     */
        OsStatus getActiveGreeting(
        const UtlString& mailboxIdentity,
        UtlString& rGreeting,
        const UtlBoolean& isFromWeb = FALSE ) ;


    /**
     * Gets the type of the active greeting.
     *
     * @param mailboxIdentity   Fully qualified mailbox identity.
     * @param rGreetingType     Contains the type of the active greeting.
     *
     * @return  if active greeting was found, returns OS_SUCCESS.
         *                      else, returns OS_FAILED
     */
        OsStatus getActiveGreetingType(
        const UtlString& mailboxIdentity,
        UtlString& rGreetingType ) const ;


        /**
     * Gets the URL of the recorded name.
     *
     * @param mailboxIdentity   Fully qualified mailbox identity.
     * @param rRecordedName             Contains the URL of the recorded name.
     *
     * @return  if recorded name file was found, returns OS_SUCCESS.
         *                      else, returns OS_FAILED
     */
        OsStatus getRecordedName(
        const UtlString& mailboxIdentity,
        UtlString& rRecordedName,
        const UtlBoolean& isFromWeb = FALSE ) const ;


    /**
     * Sets a greeting as active by updating the activegreeting.vxml file.
     *
     * @param mailboxIdentity
     * @param greetingType              greeting that has to be set as active.
     *                                                  This can be standard, outofoffice, extendedabsence or default.
     *
     * @return
     */
    OsStatus setActiveGreeting(
        const UtlString& mailboxIdentity,
        const UtlString& greetingType ) ;

    /**
     *  Returns the fully qualified URL of the specified greeting type.
     *  @param mailboxIdentity      Fully qualified mailbox id
     *  @param greetingType         Greeting type - "standard", "outofoffice", "extendedabsence".
     *  @param rGreetingUrl         Holds the URL - filled up on return
     *  @param isFromWeb            Flag indicating the source of request.
     *                              This is used to determine the host of the mediaserver.
     *  @param returnDefaultFileUrl Flag to indicate if the URL of the default system greeting
     *                              for the specified greeting type can be returned if
     *                              user recorded greeting is not available.
     *
     *  @return OS_SUCCESS      if URL was retrieved successfully.
     *          OS_FAILED       if there was an error retrieving the URL or
     *                          if the user recorded and system greeting files were not available.
     */
    OsStatus getGreetingUrl(
        const UtlString& mailboxIdentity,
        const UtlString& greetingType,
        UtlString& rGreetingUrl,
        const UtlBoolean& isFromWeb = FALSE,
        const UtlBoolean& returnDefaultFileUrl = TRUE) ;

    /**
     * Saves the recorded greeting / name to appropriate files in owner's mailbox.
     *
     * @param mailboxIdentity   fully qualified mailbox id.
     * @param greetingType      indicates the greeting name and is helpful in
     *                          determining the filename in which the recorded
     *                          data should be stored.
     *                          Possible values: standard, outofoffice,
     *                          extendedabsence, name.
     * @param data              actual recorded binary data.
     * @param datasize          length of the data.
     *
     * @return
     */
    OsStatus saveGreetingOrName (
        const UtlString& mailboxIdentity,
        const UtlString& greetingType,
        const char* data,
        const int& datasize ) ;


    /**
     * Saves the recorded greeting / name to appropriate files in owner's mailbox.
     *
     * @param mailboxIdentity   fully qualified mailbox id.
     * @param greetingType      indicates the greeting name and is helpful in
     *                          determining the filename in which the recorded
     *                          data should be stored.
     *                          Possible values: standard, outofoffice,
     *                          extendedabsence, name.
     * @param isActiveGreeting  Flag to indicate if the greeting to be deleted is the
     *                          active greeting.
     *                          If TRUE, mailbox prefs file is also updated.
     *
     * @return
     */
    OsStatus deleteGreeting(
        const UtlString& mailboxIdentity,
        const UtlString& greetingType,
        const UtlBoolean& isActiveGreeting ) ;

    /**
     * Moves the mailbox to a backup location the mailbox is not deleted
     *
     * @param mailboxIdentity
     *
     * @return
     */
    OsStatus deleteMailbox ( const UtlString& mailboxIdentity ) const;

    /**
     * gets the sorted list of mailbox folders for this user.
     *
     * @param mailboxIdentity   Fully qualified identity of the user.
     * @param folderList        RW sorted vector of folder names
     *
     * @return
     */
    OsStatus getMailboxFolders(
        const UtlString& mailboxIdentity,
        UtlSortedList& folderList ) const;

    /**
     *
     * @param mailboxIdentity
     * @param rNotifyText
     *
     * @return
     */
    OsStatus getMWINotifyText (
        const UtlString& mailboxIdentity, ///< identity of mailbox owner
        const UtlString* iMailboxPath,     ///< if specified, the path to the inbox 
        UtlString& rNotifyText            ///< application/message-summary body
        ) const;

    /**
     *
     * @param mailboxIdentity
     *
     * @return
     */
    OsStatus getMailboxURL(
        const UtlString& mailboxIdentity,
        UtlString& mailboxUrl,
        const UtlBoolean& isFromWeb = FALSE) const;

    /**
     *
     * @param mailboxIdentity
     *
     * @return
     */
    OsStatus editMessage(
        const UtlString& mailboxIdentity,
        const UtlString& folderName,
        const UtlString& messageId,
        const UtlString& subject ) const;

    OsStatus addFolder(
        const UtlString& mailboxIdentity,
        const UtlString& foldername ) const;

    OsStatus deleteFolder(
        const UtlString& mailboxIdentity,
        const UtlString& foldername ) const;

    OsStatus editFolder(
        const UtlString& mailboxIdentity,
        const UtlString& oldFoldername,
        const UtlString& newFoldername ) const;

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

    /**
     *  Gets the list of address to which email notification has to be sent.
     *
     *  @param  mailboxIdentity         Fully qualified mailbox identity
     *  @param  contactsHashDictionary  Stores the list of addresses -
     *                                  one sorted vector for email addresses and
     *                                  one sorted vector for sms addresses
     *
     *  @return OS_SUCCESS or OS_FAILED
     */
    OsStatus getNotificationContactList(
                                const UtlString&     mailboxIdentity,
                                UtlHashMap*   contactsHashDictionary ) const ;

    /**
     *  Utility method for adding / deleting / editing contacts.
     *
     *  @param  mailboxIdentity     Fully qualified mailbox identity
     *  @param  action              Action to be performed
     *  @param  contactAddress      Old contact address. This acts as the identifier
     *                              for editing / deleting. Does not apply for adding.
     *  @param  newContactAddress   New contact address. Does not apply for deleting.
     *  @param  newContactType      New contact type. Does not apply for deleting.
     *
     *  @return OS_SUCCESS          Successfully performed the requested action
     *          OS_FAILED           Failed to perform the requested action
     *          OS_NAME_IN_USE      New contact address specified for adding / editing
     *                              is not unique.
     */
    OsStatus addEditDeleteNotification(
                        const UtlString& mailboxIdentity,
                        const UtlString& action,
                        const UtlString& contactAddress,
                        const UtlString& newContactAddress,
                        const UtlString& newContactType,
                        const UtlString& sendAttachments) const ;

    /** Rate at which the "list of messages in a folder" webpage will be refreshed. */
    OsStatus getPageRefreshInterval( UtlString& rPageRefreshInterval ) const ;

    /** Returns the minimum required length of the incoming message. */
    OsStatus getMinMessageLength( int& rMinMessageLength ) const ;

    /** Utility function for creating the vxml headers, including content-length. **/
    static OsStatus getResponseHeaders(int contentLength, UtlString& responseHeaders);

    /** Returns the blocksize for displaying messages on the web */
    OsStatus getMessageBlockSizeForWeb( int& rMessageBlockSize ) const;

    /** Common method for retrieving the value of parameters defined in
     *  voicemail.xml.in file.
     *  TBD: Change the return parameter type from UtlString to
     *  something like an object so that it can be used for all parameters.
     */
    OsStatus getCustomParameter( const UtlString& paramName,
                                 UtlString& rStrValue) const;


    /** Saves the recorded system prompts (system-wide greetings and auto attendant prompt).
     *
     *  @param  promptType  Type of prompt recorded. It can be "standard", "afterhours",
     *                      "special" or "autoattendant"
     *  @param  data        Binary data for the recorded prompt
     *  @param  datasize    Size of recorded data.
     *
     *  @return OS_SUCCESS  if save was successful.
     */
    OsStatus saveSystemPrompts (const UtlString& promptType,
                                const char* data,
                                const int&  datasize ) const ;


    /** Sets a system prompt to be the active system prompt
     *  by putting right entries into the organization prefs xml.
     *
     *  @param  promptType          Type of prompt to be set as the active prompt.
     *
     *  @return OS_SUCCESS          If the greeting as set as active.
     *          OS_FILE_NOT_FOUND   If the WAV file for the specified prompt type
     *                              is unavailable.
     */
    OsStatus setActiveSystemPrompt(  const UtlString& promptType ) const ;



    /** Gets the fully qualified URL of the specified system prompt type.
     *
     *  @param  promptType          Prompt we are interested in.
     *  @param  rPromptUrl          Filled on return.
     *  @param  isFromWeb           Flag indicating if the request was from the web.
     *                              if yes, the base URL will be https://servername
     *                              if no, the base URL will be https://localhost
     *
     *  @return OS_SUCCESS          URL was obtained successfully.
     *          OS_FILE_NOT_FOUND   WAV file corresponding to the specified prompt type
     *                              does not exist.
     */
    OsStatus getSystemPromptUrl(    const UtlString& promptType,
                                    UtlString& rPromptUrl,
                                    const UtlBoolean& isFromWeb) const ;


    /** Parses the organization preferences file.
     *
     *  @param  rOrgPrefsHashDict   Hash dictionary filled on return.
     *                              It contains the name-value pairs of different
     *                              elements of the prefs file.
     */
    OsStatus parseOrganizationPrefsFile ( UtlHashMap* rOrgPrefsHashDict) const;


    /** Sets a special AA menu to be the active overal system AA menu
     *  by putting right entries into the organization prefs xml.
     *
     *  @param  specialType         The flag to enable the special AA menu.
     *
     */
    OsStatus setSystemOverallAAMenu(  bool specialType ) const ;


    /**
     * Utility method for parsing the mailbox id and
     *  deriving the physical location of the mailbox.
     *
     * @param mailbox
     * @param mailboxPath
     *
     * @return
     */
    OsStatus getMailboxPath (
        const UtlString& mailboxIdentity,
        UtlString& mailboxPath ) const;

    /**
     * Utility method for parsing the distribution list
     *
     * @param fileName
     * @param destList
     *
     * @return
     */
    OsStatus parseDistributionFile (
        const UtlString& fileName,
        const UtlString& distributionId,
        UtlSortedList& destList ) const;

    /**
     * Sets the new password in IMDB
     *
     * @param userid
     * @param password
     *
     * @return
     */
    OsStatus setPassword ( const UtlString& userid,
                           const UtlString& password ) ;

    /** Parses the autoattendant schedule file.
     *
     *  @param  rAASchedule  Autoattendant schdule filled on return.
    */
    OsStatus parseAutoAttendantSchduleFile ( UtlString& fileLocation,
                                             AutoAttendantSchedule* rSchedule ) const;

protected:
    /**
     * Singleton Protected Ctor
     */
    MailboxManager();

private:
    /**
     *
     * @param mailboxIdentity
     *
     * @return
     */
    OsStatus postMWIStatus( const UtlString& mailboxIdentity, const UtlString& inboxPath ) const;

    /**
     * create the folder from scratch
     *
     * @param mailboxIdentity
     *
     * @return
     */
    OsStatus createMailbox ( const UtlString& mailboxIdentity ) ;

    /**
     * Reconst mailboxIdentity
     *
     * @param mailboxIdentity
     *
     * @return
     */
    OsStatus restoreMailbox ( const UtlString& mailboxIdentity ) const;

    
    /// Count the messages in a mailbox
    void getMailboxCounts( const UtlString& mailboxPath,  ///< full path to the mailbox
                          int&              rUnheardCount,///< output count of unheard messages
                          int&              rTotalCount   ///< output count of all messages
                          ) const;
    
    /// If there are any subscribers to this mailbox, send them an update
    void updateMailboxStatus (
       const UtlString& mailboxIdentity, ///< SIP identity of mailbox owner
       const UtlString& mailboxPath      ///< path to the mailbox directory
                              ) const;
    /**<
     * This may be called for any mailbox; only an inbox path actually sends any notification,
     * so this determines whether or not the mailbox is an inbox before actually counting
     * messages and generating a notice.
     */
    
    /**
     * Parses an XML file containing the settings for the mailboxmanager
     *
     * @param configFile
     *
     * @return
     */
    OsStatus parseConfigFile (const UtlString& configFileName );

    /**
     * Parses the message descriptor XML file associated with a message.
     *
     * @param descriptorFileName
     * @param msgDesc
     *
     * @return SUCCESS or FAILED
     */
    OsStatus parseMessageDescriptor(
        const UtlString& descriptorFileName,
        UtlHashMap* msgDesc ) const;

    OsStatus updateMessageDescriptor (
        const UtlString& msgDescriptorFilePath,
        const UtlString& subject ) const ;

        /**
     * Creates the mailbox preferences file.
     *
         * @param prefsFileLocation             Location of the mailbox preferences file
         *
     * @return SUCCESS or FAILED
     */
        OsStatus createMailboxPrefsFile( UtlString& prefsFileLocation ) const;


        /**
     * Parses the mailbox preferences file.
     *
         * @param prefsFileLocation             Location of the mailbox preferences file
     * @param mailboxPrefsHashDict      Hash dictionary containing all the contents of the
         *                                                              XML file.
     *
     * @return SUCCESS or FAILED
     */
        OsStatus parseMailboxPrefsFile(
        const UtlString& prefsFileLocation,
        const UtlString& elementToBeRead,
                UtlHashMap* mailboxPrefsHashDict) const;

        /**
     * Updates the mailbox preferences file one element at a time.
     *  For example, consider: <contact type="email">j@j.com</contact>
     *
         * @param prefsFileLocation             Location of the mailbox preferences file
     * @param elementName           Name of the element to be updated.
     *                              For the example above, it will be "contact".
     * @param elementValue          New value for the element.
     *                              For the above example, it will be "j@j.com"
     * @param elementAttributeName  Name of the element's attribute - "type"
     * @param elementAttributeValue Value for the element's attribute - "email"
     * @param elementId             Some value for uniquely finding the element.
     *                              For example, there could be more than one contacts
     *                              and the unique value will be the contact's value - j@j.com
     * @param action                Action to be performed like 'editactivegreeting',
     *                              'editnotification', 'deletenotification', etc.
     *
     * @return  OS_SUCCESS          If the requested operation was successful
     *          OS_FAILED           If the requested operation failed.
     *          OS_NAME_IN_USE      For action='editnotification' and 'addnotification' only.
     *                              Returned if the contact to be added is a duplicate.
     */
        OsStatus updateMailboxPrefsFile (
        const UtlString& prefsFileLocation,
        const UtlString& elementName,
        const UtlString& elementValue,
        const UtlString& elementAttributeName,
        const UtlString& elementAttributeValue,
        const UtlString& elementId,
        const UtlString& action) const;

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

        /**
     * Utility method for logging errors
     *
         * @param methodName    Method calling this function
     * @param contents          Contents to the system log file
     *
     * @return
     */
        void writeToLog(
        const UtlString& methodName,
        const UtlString& contents,
        const OsSysLogPriority& priority = PRI_INFO) const;

        /**
         *      Utility method for finding the folder name based on the category.
         *      that is, if category = "unheard", foldername is 'inbox'.
         *
         *      @return foldername
         */
        UtlString getFolderName(        const UtlString& category ) const;

    /**
     *  Utility method for generating the default greetings
     *  by merging together appropriate WAV files.
     *
     *  @param  mailboxIdentity     Mailbox id for which the greeting should be created
     *  @param  greetingType        Type of default greeting to be created
     *
     */
    OsStatus generateDefaultGreetings (
        const UtlString& mailboxIdentity,
        const UtlString& greetingType) ;


    /** Gets the location of system prompts.
     *
     *  @param  isGenericPrompts    Flag to indicate if we are interested in generic prompts
     *                              TRUE - Location of generic prompts
     *                              FALSE - Location of custom prompts
     *  @param  rPromptsDir         Filled on return.
     *  @return OsStatus            OS_SUCCESS or OS_FAILED.
     */
    OsStatus getSystemPromptsDir(   const UtlBoolean& isGenericPrompts,
                                    UtlString& rPromptsDir ) const;


    /** Gets the base URL for the system prompts.
     *
     *  @param  rPromptsUrl         Filled on return.
     *  @param  isGenericPrompts    Flag to indicate if we are interested in generic prompts
     *                              TRUE - Base URL for generic prompts
     *                              FALSE - Base URL for custom prompts
     *  @param  isFromWeb           Flag indicating if the request is from the web.
     *                              If yes, the base url is https://<servername>
     *                              Else, the base url is https://localhost
     *
     *  @return OsStatus            OS_SUCCESS or OS_FAILED.
     */
    OsStatus getSystemPromptsUrl(   UtlString& rPromptsUrl,
                                    const UtlBoolean& isGenericPrompts,
                                    const UtlBoolean& isFromWeb) const;


    /** Gets the location of the organization preferences XML file.
     *  This file is usually present at data/<domain> directory.
     *  For backward compatibility, if the file does not exist,
     *  we try to create it.
     *
     *  @param  rOrgPrefsFileLocation   Filled on return.
     *  @return OsStatus                OS_SUCCESS or OS_FAILED.
     */
    OsStatus getOrgPrefsFileLocation( UtlString& rOrgPrefsFileLocation ) const ;


    /** Creates the organization preferences XML file.
     *
     *  @param  orgPrefsFileLocation    Fully qualified org prefs file name.
     *  @return OsStatus                OS_SUCCESS or OS_FAILED
     */
    OsStatus createOrganizationPrefsFile( const UtlString& orgPrefsFileLocation ) const;


    /** Updates the organization preferences XML file.
     *
     *  @param  prefsFileLocation   Fully qualified org prefs file name
     *  @param  elementName         Name of the XML element to be updated.
     *  @param  elementValue        Value for the element.
     */
    OsStatus updateOrganizationPrefsFile (  const UtlString& prefsFileLocation,
                                            const UtlString& elementName,
                                            const UtlString& elementValue ) const ;


    OsStatus sendEmailNotification(     const UtlString& mailboxIdentity,
                                        const UtlString& from,
                                        const UtlString& timestamp,
                                        const UtlString& duration,
                                        const UtlString& wavFileName,
                                        const char* data,
                                        const int& datasize) const;

    /** Gets the location of the AA schedule XML file.
     *
     *  @param  rScheduleFileLocation   Filled on return.
     *  @return OsStatus                OS_SUCCESS or OS_FAILED.
     */
    OsStatus getScheduleFileLocation( UtlString& rFileName, UtlString& rScheduleFileLocation ) const ;

    int timeValue(const char* time) const;

    // Singleton instance
    static MailboxManager* spInstance;

    // Exclusive binary lock
    static OsMutex sLockMutex;

    // these are read in from the configuraton file
    UtlString m_mode;
    UtlString m_password;
    UtlString m_authType;
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
    UtlString m_mwiUrl;
    UtlString m_inboxFolder;
    UtlString m_savedFolder;
    UtlString m_deletedFolder;
    UtlString m_pageRefreshInterval;
    UtlString m_logLevel;
    UtlString m_smtpServer;
    UtlString m_emailNotificationAddr;
    UtlString m_configServerSecureUrl;
    UtlString m_mailboxUrl;

    // Variable to hold the value of <voicemail-info-playback> element
    UtlString m_voicemailInfoPlayback;

    UtlHashMap* m_templateFolderInfo;
    int      m_maxMessageLength;
    int      m_minMessageLength;
    int      m_minUserPasswordLength;
    int      m_maxUserPasswordLength;
    int      m_maxMailboxMessages;
    int      m_housekeepingTimerMinutes;
    int      m_webMessageBlockSize ;

    /**
     * The following are pointers to the sipdb classes that to be used in the mailbox manager.
     * The reason for adding them is that they need to be deleted in the destuctor, and doing
     * delete ::getInstance() sometimes unnecessarily creates the db object - deleing without
     * anyone actually used it.
     * @JC No longer need to store pointers to the individual tables, sipdbmanager takes care
     * of reference counting and destruction
     */
    MessageIDGenerator* m_pMsgIDGenerator;    
};

#endif //MAILBOXMANAGER_H
