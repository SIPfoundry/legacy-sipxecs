//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsFS.h"
#include "os/OsLock.h"
#include "os/OsSysLog.h"
#include "net/Url.h"
#include "net/HttpBody.h"
#include "net/HttpMessage.h"
#include "net/SipMessage.h"
#include "net/NetMd5Codec.h"
#include "net/MailMessage.h"
#include "net/Url.h"
#include "mp/MpAudioUtils.h"
#include "sipdb/ResultSet.h"
#include "sipdb/AliasDB.h"
#include "sipdb/ExtensionDB.h"
#include "sipdb/CredentialDB.h"
#include "sipdb/PermissionDB.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/SIPXAuthHelper.h"
#include "cgicc/Cgicc.h"
#include "cgicc/CgiEnvironment.h"
#include "sipxcgi/CgiValues.h"
#include "xmlparser/tinyxml.h"
#include "mailboxmgr/CategorizedString.h"
#include "mailboxmgr/VXMLDefs.h"
#include "mailboxmgr/NotificationHelper.h"
#include "mailboxmgr/MailboxManager.h"
#include "mailboxmgr/MessageIDGenerator.h"
#include "utl/UtlTokenizer.h"
#include "utl/UtlHashMap.h"
#include "utl/UtlHashMapIterator.h"
#include "utl/UtlRegex.h"
#include "os/OsDateTime.h"

// DEFINES
#ifndef O_BINARY        // needed for WIN32
    #define O_BINARY 0
#endif
#define VOICEMAIL_CONFIG_FILE     SIPX_CONFDIR "/voicemail.xml"

// @TODO@ add new pin as part of the message
#define CONTENT_TYPE_TEXT_XML "text/xml; charset=utf-8"

#ifdef USE_SOAP
#define _SOAP_MSG_NO_ARG_TEMPLATE_ \
	"<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"%s\" SOAP-ENV:encoding=\"%s\"" \
	" xmlns:xsi=\"%s\"" \
	" xmlns:xsd=\"%s\">" \
	"<SOAP-ENV:Body>"\
	"<m:%s xmlns:m=\"%s\">"\
	"</m:%s>" \
	"</SOAP-ENV:Body>"\
	"</SOAP-ENV:Envelope>"

#define _SOAP_MSG_TEMPLATE_ \
   "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"%s\" SOAP-ENV:encoding=\"%s\"" \
   " xmlns:xsi=\"%s\"" \
   " xmlns:xsd=\"%s\">" \
   "<SOAP-ENV:Body>"\
   "<m:%s xmlns:m=\"%s\">"\
   "<m:c-gensym3 xsi:type=\"xsd:string\">"\
   "%s"\
   "</m:c-gensym3>"\
   "<m:c-gensym5 xsi:type=\"xsd:string\">"\
   "%s"\
   "</m:c-gensym5>"\
   "</m:%s>" \
   "</SOAP-ENV:Body>"\
   "</SOAP-ENV:Envelope>"

static UtlString soap_env_ns("http://schemas.xmlsoap.org/soap/envelope/");
static UtlString soap_env_enc("http://schemas.xmlsoap.org/soap/encoding/");
static UtlString soap_xsi_ns("http://www.w3.org/1999/XMLSchema-instance");
static UtlString soap_xsd_ns("http://www.w3.org/1999/XMLSchema");
#endif

// MACROS
// EXTERNAL FUNCTIONS

// EXTERNAL VARIABLES
extern CgiValues *gValues;
extern cgicc::Cgicc *gCgi;

// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
// STATIC INITIALIZERS
MailboxManager* MailboxManager::spInstance = NULL;
OsMutex         MailboxManager::sLockMutex (OsMutex::Q_FIFO);


AutoAttendantSchedule::AutoAttendantSchedule()
{    
    mHolidayMenu = HOLIDAY_MENU;   
    mRegularMenu = REGULAR_MENU;
    mAfterHourMenu = AFTERHOUR_MENU;
}

AutoAttendantSchedule::~AutoAttendantSchedule()
{
    mHolidays.destroyAll();    
    mRegularFromHour.destroyAll();
    mRegularToHour.destroyAll();
}


/* ============================ CREATORS ================================== */
MailboxManager::MailboxManager() :
    m_inboxFolder("inbox"),
    m_savedFolder("saved"),
    m_deletedFolder("deleted"),
    m_pageRefreshInterval("1"),
    m_logLevel(LOG_LEVEL_ERROR),
    m_smtpServer("localhost"),
    m_templateFolderInfo (new UtlHashMap()),
    m_maxMessageLength (-1),
    m_minMessageLength (3),
    m_minUserPasswordLength (4),
    m_maxUserPasswordLength (10),
    m_maxMailboxMessages (-1),
    m_housekeepingTimerMinutes (-1),
    m_webMessageBlockSize(-1),
    m_pMsgIDGenerator(NULL)
{
    // Parse the configuration file voicemail.xml
    parseConfigFile ( UtlString(VOICEMAIL_CONFIG_FILE) );
}

MailboxManager::~MailboxManager()
{
    if ( m_templateFolderInfo != NULL )
        delete m_templateFolderInfo;

    if (m_pMsgIDGenerator)
    {
        delete m_pMsgIDGenerator;
        m_pMsgIDGenerator = NULL;
    }

    // Clean up IMDB resources
    delete SIPDBManager::getInstance();
}

MailboxManager*
MailboxManager::getInstance()
{
    // Critical Section here
    OsLock lock( sLockMutex );

    // See if this is the first time through for this process
    // Note that this being null => pgDatabase is also null
    if ( spInstance == NULL )
    {   // Create the singleton class for clients to use
        spInstance = new MailboxManager();
    }
    return spInstance;
}

OsStatus
MailboxManager::deleteMailbox ( const UtlString& mailboxIdentity ) const
{
    OsStatus result = OS_FAILED;
    UtlString mailboxPath;
    result = getMailboxPath( mailboxIdentity, mailboxPath );
    if( result == OS_SUCCESS )
    {
        OsPath path = mailboxPath ;
        OsDir mailboxDir ( path );

        if ( mailboxDir.exists() )
        {
            result = OsFileSystem::remove(mailboxPath, TRUE, TRUE);
        }
    }
    return result;
}

OsStatus
MailboxManager::createMailbox ( const UtlString& mailboxIdentity )
{
    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                  "MailboxManager::createMailbox('%s') called",
                  mailboxIdentity.data());
    OsStatus result = OS_FAILED;
    UtlString mailboxPath;
    result = getMailboxPath( mailboxIdentity, mailboxPath );
    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                  "MailboxManager::createMailbox: getMailboxPath(mailboxIdentity = '%s', mailboxPath = '%s') = %d",
                  mailboxIdentity.data(), mailboxPath.data(), result);
    if( result == OS_SUCCESS )
    {
        if ( !OsFileSystem::exists( mailboxPath ) )
	{
          result = OsFileSystem::createDir( mailboxPath );
          OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                      "MailboxManager::createMailbox: createDir('%s') = %d",
                      mailboxPath.data(), result);
        }

        if ( result == OS_SUCCESS )
        {
            // Use the template mailbox definitions stored
            // in the xml file to create the subfolders
            // for the mailbox, each folder is defined
            // in the XML as:
            //
            // <folder>
            //   <name>inbox</name>
            //   <displayname>In box</displayname>
            //   <autodelete>Never</autodelete>
            // </folder>
            // the m_templateFolderInfo is a HashDictionary. its key will
            // be the 'name' value above, it's value will be another
            // HashDictionary with the keys displayname & autodelete
            UtlHashMapIterator iterator(*m_templateFolderInfo);
            UtlString* nextFolderName;

            while ((nextFolderName = (UtlString*)iterator()))
            {
                OsPath folderPath =
                    mailboxPath + OsPathBase::separator +
                    *nextFolderName;

                result = OsFileSystem::createDir ( folderPath );
                OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                              "MailboxManager::createMailbox: createDir('%s') = %d",
                              folderPath.data(), result);
            }

            // Create the default greetings
            generateDefaultGreetings ( mailboxIdentity, DEFAULT_STANDARD_GREETING );
            generateDefaultGreetings ( mailboxIdentity, DEFAULT_OUTOFOFFICE_GREETING );
            generateDefaultGreetings ( mailboxIdentity, DEFAULT_EXTENDED_ABS_GREETING );

            // set the active greeting to unset, this will also create the prefs file
            setActiveGreeting( mailboxIdentity, ACTIVE_GREETING_NOT_SET );
        }
        else
        {
           OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_ERR,
                         "MailboxManager::createMailbox: unable to create mailbox directory '%s'",
                         mailboxPath.data());
        }
    }
    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                  "MailboxManager::createMailbox: result = %d",
                  result);
    return result;
}

OsStatus
MailboxManager::restoreMailbox ( const UtlString& mailbox ) const
{
    OsFileIterator fi("");

    // find all files starting with C or c
    // you can use full regular expression
    // matching when looking for files or directories
    OsPath entry;
    OsStatus result = fi.findFirst(entry,"*",OsFileIterator::FILES );

    if ( result == OS_SUCCESS )
    {
        while ( result == OS_SUCCESS )
        {
            result = fi.findNext(entry);
        }
    }
    return result;
}

void
MailboxManager::getMailboxCounts(
    const UtlString& mailboxPath,
    int& rUnheardCount,
    int& rTotalCount
                                 ) const
{
    OsPath path( mailboxPath );
    OsFileIterator fi( path );
    OsPath entry;
    rUnheardCount = 0;
    rTotalCount = 0;

    // Get the count of all *-00.xml files (gives the total count )
    OsStatus result;
    for ( result =  fi.findFirst( entry, "[0-9]-00.xml", OsFileIterator::FILES );
          result == OS_SUCCESS;
          result =  fi.findNext(entry)
         )
    {
       rTotalCount ++;
    }

    // Get the count of all *-00.sta files (gives the unheard count)
    for ( result =  fi.findFirst( entry, "[0-9]-00.sta", OsFileIterator::FILES );
          result == OS_SUCCESS;
          result =  fi.findNext(entry)
         )
    {
       rUnheardCount ++;
    }

    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                  "MailboxManager::getMailboxCounts( '%s', %d, %d )",
                  mailboxPath.data(), rUnheardCount, rTotalCount);
}

OsStatus
MailboxManager::parseConfigFile ( const UtlString& configFileName )
{
    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                  "Entering MailboxManager::parseConfigFile('%s')",
                  configFileName.data());
    OsStatus result = OS_FAILED;

    TiXmlDocument doc ( configFileName );

    // Verify that we can load the file (i.e it must exist)
    if( doc.LoadFile() )
    {
       OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                     "MailboxManager::parseConfigFile: doc.LoadFile() returned TRUE");
        TiXmlNode * rootNode = doc.FirstChild ("settings");
        if (rootNode != NULL)
        {
            TiXmlNode * voicemailNode = rootNode->FirstChild("voicemail");
            if (voicemailNode != NULL)
            {
                // Hari 01/30/2003
                // Commented out parameters that are never used in this release.
                // Removed these parameters from voicemail.xml.in file too.

                // getConfigValue ( *voicemailNode, "mode", m_mode );
                // getConfigValue ( *voicemailNode, "password", m_password );
                // getConfigValue ( *voicemailNode, "auth-type", m_authType );
                // getConfigValue ( *voicemailNode, "default-user-password", m_defaultUserPassword );

                getConfigValue ( *voicemailNode, "default-domain", m_defaultDomain );
                getConfigValue ( *voicemailNode, "default-realm", m_defaultRealm );
                getConfigValue ( *voicemailNode, "config-server-url-secure", m_configServerSecureUrl );
                getConfigValue ( *voicemailNode, "email-mailbox-url", m_mailboxUrl );
                getConfigValue ( *voicemailNode, "email-notification-addr", m_emailNotificationAddr );
                getConfigValue ( *voicemailNode, "mailstore-root", m_mailstoreRoot );
                getConfigValue ( *voicemailNode, "mediaserver-root", m_mediaserverRoot );
                getConfigValue ( *voicemailNode, "mediaserver-url", m_mediaserverUrl );
                getConfigValue ( *voicemailNode, "mediaserver-url-secure", m_mediaserverSecureUrl );
                getConfigValue ( *voicemailNode, "full-mediaserver-url", m_fullMediaserverUrl );
                getConfigValue ( *voicemailNode, "full-mediaserver-url-secure", m_fullMediaserverSecureUrl );
                getConfigValue ( *voicemailNode, "ivr-prompt-url", m_ivrPromptUrl );
                getConfigValue ( *voicemailNode, "default-message-subject", m_defaultMessageSubject );
                getConfigValue ( *voicemailNode, "webpage-refresh-interval", m_pageRefreshInterval);
                getConfigValue ( *voicemailNode, "voicemail-cgi-log-level", m_logLevel);
                getConfigValue ( *voicemailNode, "voicemail-info-playback", m_voicemailInfoPlayback);

                // These are the integer values there is the extra step of convert
                // ing the string to a member integer
                UtlString intValueString;

                // getConfigValue (*voicemailNode, "max-message-length", intValueString);
                // m_maxMessageLength = atoi (intValueString);

                getConfigValue (*voicemailNode, "min-message-length", intValueString);
                m_minMessageLength = atoi (intValueString);

                // getConfigValue (*voicemailNode, "min-user-password-length", intValueString);
                // m_minUserPasswordLength = atoi (intValueString);

                // getConfigValue (*voicemailNode, "max-user-password-length", intValueString);
                // m_maxUserPasswordLength = atoi (intValueString);

                // getConfigValue (*voicemailNode, "max-mailbox-messages", intValueString);
                // m_maxMailboxMessages = atoi (intValueString);

                // getConfigValue (*voicemailNode, "housekeeping-timer-minutes", intValueString);
                // m_housekeepingTimerMinutes = atoi (intValueString);

                getConfigValue ( *voicemailNode, "web-message-block-size", intValueString);
                m_webMessageBlockSize = atoi (intValueString);

                // Now find the folder name settings
                TiXmlNode * allFolders = voicemailNode->FirstChild("folders");

                // the folder node contains at least the name/displayname/
                // and autodelete elements, it may contain others
                for( TiXmlNode *folderNode = allFolders->FirstChild( "folder" );
                     folderNode;
                     folderNode = folderNode->NextSibling( "folder" ) )
                {
                    UtlHashMap* folderMetaData = new UtlHashMap();

                    for( TiXmlNode *elementNode = folderNode->FirstChild();
                         elementNode;
                         elementNode = elementNode->NextSibling() )
                    {
                        // Bypass comments and other element types only interested
                        // in parsing element attributes
                        if ( elementNode->Type() == TiXmlNode::ELEMENT )
                        {
                            UtlString elementName = elementNode->Value();
                            UtlString elementValue;
                            result = getConfigValue (*folderNode, elementName, elementValue);

                            if (result == OS_SUCCESS)
                            {
                                UtlString* collectableKey = new UtlString( elementName );
                                UtlString* collectableValue = new UtlString( elementValue );
                                folderMetaData->insertKeyAndValue ( collectableKey, collectableValue );
                            }
                        }
                    }

                    // After creating the metadata for each folder extract the folder name
                    // and store this along with the metadata on the folderInfo hashtable
                    UtlString* nameKey = new UtlString("name");

                    UtlString* folderName =
                        (UtlString*)folderMetaData->findValue(nameKey);

                    if ( folderName != NULL )
                    {
                        m_templateFolderInfo->
                            insertKeyAndValue (
                                folderName,
                                folderMetaData );
                    } else
                    {
                        // there is no name element so do not bother
                        // to store the folder meta data or its
                        // collectable key
                        delete folderMetaData;
                        delete nameKey;
                    }
                }

                // NOTE : The names of the standard folders has been hard coded.
                // Trying to change this will break many functionality like
                // managing folders, play messages etc (all places where we look for "inbox", etc).

                // TBD : Remove the hard coding and read it from voicemail.xml.in file.
                // This is already done in createMailbox method. Update all other CGIs to
                // make use of this.
                m_inboxFolder = "inbox";
                m_savedFolder = "saved";
                m_deletedFolder = "deleted";
                result = OS_SUCCESS;
            }

            TiXmlNode * mwiNode = rootNode->FirstChild("mwi");
            if (mwiNode != NULL)
            {
                getConfigValue (*mwiNode, "mwiurl", m_mwiUrl );
            }
        }
    }
   else
   {
      // We are unable to parse the configuration file.
      // Log an error with a high severity to make sure that it makes it
      // into the log.
      OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_CRIT,
                    "MailboxManager::parseConfigFile: doc.LoadFile() returned FALSE while attempting to process configuration file '%s'",
                    configFileName.data());
   }

   OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                 "MailboxManager::parseConfigFile: returning %d", result);
    return result;
}

OsStatus
MailboxManager::parseMessageDescriptor (
    const UtlString& descriptorFileName,
    UtlHashMap* msgDesc) const
{
    OsStatus result = OS_FAILED;
    UtlBoolean fileNeedsRepair = FALSE;
    TiXmlDocument doc ( descriptorFileName );

    // Verify that we can load the file (i.e it must exist)
    if( doc.LoadFile() )
    {
        TiXmlNode * rootNode = doc.FirstChild ("messagedescriptor");
        if ( rootNode != NULL )
        {
            // Parse the XML file looking for various elements
            // fail if we encounter a single error
            UtlString value;
            result = getConfigValue (*rootNode, "id", value);
            if ( result == OS_SUCCESS )
            {
                msgDesc->insertKeyAndValue(
                    new UtlString("id"),
                    new UtlString(value));

                // the encoding to and from XML form for the < > and quote
                // symbols is completely transparent to us for the from field.
                // even though the values went into the XML file with the
                // correctly &lt; and $gt; they come out here correctly
                // unescaped into < and > signs so be careful
                result = getConfigValue (*rootNode, "from", value);

                // if we didn't get XML formatting errors
                if ( result == OS_SUCCESS )
                {
                    // This is a temporary fixup patch that should be removed
                    // once everybody has reviewed all their messages
                    if ( value.isNull() )
                    {
                        value = "sip:unknown@unknown-domain";
                        setConfigValue (*rootNode, "from", value);
                        fileNeedsRepair = TRUE;
                    } else if ( value.index ("%40") != UTL_NOT_FOUND )
                    {   // found an escaped @ sign so unescape the string
                        // and it should be a good format
                        HttpMessage::unescape ( value );

                        // replace the < and > signs and update the
                        // from field.
                        MailboxManager::convertUrlStringToXML( value );
                        setConfigValue (*rootNode, "from", value);
                        fileNeedsRepair = TRUE;
                    }

                    msgDesc->insertKeyAndValue(
                        new UtlString("from"),
                        new UtlString(value));

                    result = getConfigValue (
                        *rootNode, "subject", value);

                    if ( result == OS_SUCCESS )
                    {
                        msgDesc->insertKeyAndValue(
                            new UtlString("subject"),
                            new UtlString(value));

                        result = getConfigValue (
                            *rootNode, "timestamp", value);

                        if ( result == OS_SUCCESS )
                        {
                            msgDesc->insertKeyAndValue(
                                new UtlString("timestamp"),
                                new UtlString(value));

                            result = getConfigValue (
                                *rootNode, "durationsecs", value);

                            if ( result == OS_SUCCESS )
                            {
                                msgDesc->insertKeyAndValue(
                                    new UtlString("durationsecs"),
                                    new UtlString(value));

                                result = getConfigValue (
                                    *rootNode, "priority", value);
                                if ( result == OS_SUCCESS )
                                {
                                    msgDesc->insertKeyAndValue(
                                        new UtlString("priority"),
                                        new UtlString(value));
                                } else
                                {
                                    writeToLog(
                                        "parseMessageDescriptor",
                                        "cannot find 'durationsecs' for - " +
                                         descriptorFileName,
                                        PRI_ERR);
                                }
                            } else
                            {
                                writeToLog(
                                    "parseMessageDescriptor",
                                    "cannot find 'durationsecs' for - " + descriptorFileName,
                                    PRI_ERR);
                            }
                        } else
                        {
                            writeToLog(
                                "parseMessageDescriptor",
                                "cannot find 'timestamp' for - " + descriptorFileName,
                                PRI_ERR);
                        }
                    } else
                    {
                        writeToLog(
                            "parseMessageDescriptor",
                            "cannot find 'subject' for - " + descriptorFileName,
                            PRI_ERR);
                    }
                } else
                {
                    writeToLog(
                        "parseMessageDescriptor",
                        "cannot find 'from' for - " + descriptorFileName,
                        PRI_ERR);
                }
            } else
            {
                writeToLog(
                    "parseMessageDescriptor",
                    "cannot find 'id' for - " + descriptorFileName,
                    PRI_ERR);
            }
        } else
        {
            writeToLog( "parseMessageDescriptor",
                "Message descriptor does not have root element - " + descriptorFileName,
                PRI_ERR);
        }

        // fix up the old from fields and persist them
        if ( fileNeedsRepair )
        {
            // Save the updated document
            if ( doc.SaveFile( descriptorFileName ) == true )
            {
                result = OS_SUCCESS;
            }
        }
    } else
    {
        writeToLog( "parseMessageDescriptor",
            "Unable to open open/parse file  - " + descriptorFileName,
            PRI_ERR);
    }

    // If there is an error parsing the file, dump the contents into the log.
    // :TODO: Rewrite this using the C++ I/O functions.
    if (result != OS_SUCCESS)
    {
       OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_ERR,
                     "parseMessageDescriptor: Dumping contents of descriptor file '%s'",
                     descriptorFileName.data());
       FILE *f = fopen(descriptorFileName.data(), "r");
       if (f == NULL)
       {
          OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_ERR,
                        "parseMessageDescriptor: Received errno = %d '%s' while opening file",
                        errno, strerror(errno));
       }
       else
       {
          char buffer[100];
          while (fgets(buffer, sizeof (buffer), f) != NULL)
          {
             OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_ERR,
                           "parseMessageDescriptor: Contents: %s",
                           buffer);
          }
          OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_ERR,
                        "parseMessageDescriptor: EOF");
          fclose(f);
       }
    }

    return result;
}

OsStatus
MailboxManager::updateMessageDescriptor (
    const UtlString& msgDescriptorFilePath,
    const UtlString& subject ) const
{
    OsStatus result = OS_FAILED;

    TiXmlDocument doc ( msgDescriptorFilePath );

    // Verify that we can load the file (i.e it must exist)
    if( doc.LoadFile() )
    {
        TiXmlNode * rootNode = doc.FirstChild ("messagedescriptor");
        if ( rootNode != NULL )
        {
            TiXmlNode* node = rootNode->FirstChild("subject");
            if ( (node != NULL) && (node->Type() == TiXmlNode::ELEMENT) )
            {
                // If the content of elem is null, or (for some
                // reason) all whitespace, it has no children, and
                // childNode is NULL.  In that case, we have to construct
                // a new child node.
                if ( node->FirstChild() == NULL )
                {
                   TiXmlText newChildNode(subject);
                   node->InsertEndChild(newChildNode);
                   result = OS_SUCCESS;
                }
                else if ( node->FirstChild() && node->FirstChild()->Type() == TiXmlNode::TEXT )
                {
                   node->FirstChild()->SetValue( subject );
                   result = OS_SUCCESS;
                }
            }

            // Save the updated document
            if ( result == OS_SUCCESS &&
                 doc.SaveFile( msgDescriptorFilePath ) != true )
            {
               // Rescind the success indication.
               result = OS_FAILED;
            }
        }
    }
    return result;
}

OsStatus
MailboxManager::createMailboxPrefsFile( UtlString& prefsFileLocation ) const
{
    OsFile prefsFile ( prefsFileLocation );

    OsStatus result = prefsFile.open( OsFile::CREATE );
    if ( result == OS_SUCCESS )
    {
        UtlString defaultPrefsData =
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
            "<prefs>\n"
            "   <activegreeting>" + UtlString( ACTIVE_GREETING_NOT_SET ) + "</activegreeting>\n"
            "   <notification></notification>\n"
            "</prefs>";

        unsigned long bytes_written = 0;
        result = prefsFile.write(
            defaultPrefsData.data(),
            defaultPrefsData.length(),
            bytes_written );
        prefsFile.close();
    }

    return result;
}

/* Get information from the mailbox preference file.
 *
 * If elementtoBeRead is "notification", then the returned value is a hash.
 * The "sortedcontacts" element of the hash is a UtlSortedList* containing all
 * the e-mail addresses.
 * Each e-mail address is also a key, whose value is another hash, containing
 * two elements:  The "type" element is the type attribute, and the
 * "attachments" element is the attachments attribute.
 */
OsStatus
MailboxManager::parseMailboxPrefsFile(
    const UtlString& prefsFileLocation,
    const UtlString& elementToBeRead,
    UtlHashMap* mailboxPrefsHashDict ) const
{
    OsStatus result = OS_FAILED;
    TiXmlDocument doc ( prefsFileLocation );

    // Verify that we can load the file (i.e it must exist)
    if( doc.LoadFile() )
    {
        TiXmlNode * rootNode = doc.FirstChild ("prefs");
        if (rootNode != NULL)
        {
            if( elementToBeRead.compareTo("activegreeting") == 0 )
            {
                // Retrieve the active greeting
                UtlString value;
                result = getConfigValue (*rootNode, "activegreeting", value);
                if( result == OS_SUCCESS )
                {
                    mailboxPrefsHashDict->insertKeyAndValue(
                        new UtlString("activegreeting"),
                        new UtlString(value));
                }
                else
                {
                    writeToLog("ParseMailboxPrefsFile",
                        "File corrupted. Failed to find element '<activegreeting>' in " + prefsFileLocation, PRI_ERR);
                }
            } else if( elementToBeRead.compareTo("notification") == 0 )
            {
                // this pertains to the email forwarding settings
                TiXmlNode *notificationNode = rootNode->FirstChild( "notification" ) ;

                if( notificationNode != NULL )
                {
                    UtlSortedList* sortedContacts = new UtlSortedList();

                    // Get the list of emails for new voicemail notification
                    for( TiXmlNode *contactNode = notificationNode->FirstChild("contact");
                         contactNode;
                         contactNode = contactNode->NextSibling("contact") )
                    {

                        // Bypass comments and other element types only interested
                        // in parsing element attributes
                        if ( contactNode->Type() == TiXmlNode::ELEMENT )
                        {
                            // convert the node to an element
                            TiXmlElement* elem = contactNode->ToElement();
                            if ( elem != NULL )
                            {
                                // Get the text in the element
                                TiXmlNode* childNode = elem->FirstChild();
                                if( childNode && childNode->Type() == TiXmlNode::TEXT )
                                {
                                    TiXmlText* value = childNode->ToText();
                                    if (value)
                                    {
                                        UtlString* collectableValue = new UtlString( value->Value() );
                                        if( !mailboxPrefsHashDict->contains(collectableValue) )
                                        {
                                            // Get information about the contact
                                            UtlString contactType = "email" ;
                                            if( elem->Attribute("type") != NULL )
                                                contactType = elem->Attribute("type") ;

                                            UtlString sendAttachments = "no";
                                            if( elem->Attribute("attachments") != NULL )
                                                sendAttachments = elem->Attribute("attachments") ;

                                            // Add the details of the contact to a hash dictionary
                                            UtlHashMap* contactHashDict = new UtlHashMap();
                                            contactHashDict->insertKeyAndValue( new UtlString("type"), new UtlString(contactType) );
                                            contactHashDict->insertKeyAndValue( new UtlString("attachments"), new UtlString(sendAttachments) );

                                            // Add the contact and its details to the hash dictionary that will get returned.
                                            mailboxPrefsHashDict->insertKeyAndValue( collectableValue, contactHashDict );

                                            OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                                                          "MailboxManager::parseMailboxPrefsFile: addr = '%s', type = '%s', attachments = '%s', contactHashDict = %p",
                                                          collectableValue->data(), contactType.data(), sendAttachments.data(), (void*) contactHashDict);

                                            // Add the contact to a sorted vector.
                                            sortedContacts->insert( collectableValue );
                                        }
                                    }
                                }
                            }
                        }
                    }
                    mailboxPrefsHashDict->insertKeyAndValue( new UtlString("sortedcontacts"), sortedContacts );
                }
            }
        }
        else
        {
            writeToLog("ParseMailboxPrefsFile", "File corrupted. Failed to find root node '<prefs>' in " + prefsFileLocation, PRI_ERR);
        }
    }
    else
    {
        writeToLog("ParseMailboxPrefsFile", "TinyXML failed to load the file " + prefsFileLocation, PRI_ERR);
    }
    return result;
}

OsStatus
MailboxManager::updateMailboxPrefsFile(
    const UtlString& prefsFileLocation,
    const UtlString& elementName,
    const UtlString& elementValue,
    const UtlString& elementAttributeName,
    const UtlString& elementAttributeValue,
    const UtlString& elementId,
    const UtlString& action
    ) const
{
    OsStatus result = OS_FAILED ;

    TiXmlDocument doc ( prefsFileLocation );

    // Verify that we can load the file (i.e it must exist)
    if( doc.LoadFile() )
    {
        TiXmlNode * rootNode = doc.FirstChild ("prefs");
        if ( rootNode != NULL )
        {
            if( action.compareTo("editactivegreeting") == 0 )
            {
                TiXmlElement* element = rootNode->FirstChildElement( elementName );
                if ( element )
                {
                    if ( element->FirstChild() && element->FirstChild()->Type() == TiXmlNode::TEXT )
                    {
                        element->FirstChild()->SetValue( elementValue );
                        result = OS_SUCCESS ;
                    }
                }
            } else
            {
                TiXmlNode *notificationNode = rootNode->FirstChild( "notification" ) ;
                if( notificationNode == NULL )
                {
                    // For backward compatibility, create the <notification> elememt.
                    TiXmlElement* notificationElement = new TiXmlElement("notification") ;
                    if( notificationElement != NULL )
                    {
                        rootNode->InsertEndChild( *notificationElement );
                        notificationNode = rootNode->FirstChild( "notification" ) ;
                    }
                }

                if( notificationNode != NULL )
                {
                    // Flag used for 'addnotification'.
                    // Indicates if the contact to be added is already present.
                    UtlBoolean duplicatesFound = FALSE ;

                    // Get the list of emails for new voicemail notification
                    for( TiXmlElement *contactElement = notificationNode->FirstChildElement("contact");
                         contactElement;
                         contactElement = contactElement->NextSiblingElement("contact") )
                    {
                        // check if old and new contact addresses are different.
                        // If they are the same for edit notification,
                        // do not check for duplicates (as you will definitely find one).
                        // Change must be in someother parameter like the type.
                        if( action.compareTo("editnotification") == 0 &&
                            elementValue.compareTo(elementId) == 0 )
                            break ;

                        if( contactElement->FirstChild() && contactElement->FirstChild()->Type() == TiXmlNode::TEXT )
                        {
                            UtlString contactAddress = contactElement->FirstChild()->Value() ;

                            if( action.compareTo("editnotification") == 0 ||
                                action.compareTo("addnotification") == 0 )
                            {
                                // Check for duplicates :
                                // Check if the new value to be added already exists
                                if( contactAddress.compareTo(elementValue, UtlString::ignoreCase) == 0 )
                                {
                                    duplicatesFound = TRUE ;

                                    // set the return status to something different
                                    // from OS_FAILED
                                    // This helps to display appropriate message
                                    // to indicate to the user that there is a duplicate
                                    result = OS_NAME_IN_USE ;
                                    break ;
                                }
                            } else if( action.compareTo("deletenotification") == 0 )
                            {
                                if( contactAddress.compareTo(elementId, UtlString::ignoreCase) == 0 )
                                {
                                    if( notificationNode->RemoveChild( contactElement ) == true )
                                        result = OS_SUCCESS ;
                                    break ;
                                }
                            }
                        }
                    }

                    // Check if duplicatesFound flag is TRUE.

                    if( !duplicatesFound )
                    {
                        // If not, add/edit the entry.
                        if( action.compareTo("addnotification") == 0)
                        {
                            // TO DO : Check if the user has added the maximum number of contacts
                            TiXmlElement* notificationElement =
                                rootNode->FirstChildElement( "notification" );

                            if ( notificationElement != NULL )
                            {
                                // Create the element <contact>
                                TiXmlElement* contactElement = new TiXmlElement("contact") ;

                                // Create the text element for the contact value
                                TiXmlText* textElement = new TiXmlText( elementValue ) ;

                                if( contactElement != NULL && textElement != NULL)
                                {
                                    contactElement->SetAttribute( "type", "email" );

                                    // Add attribute 'attachments' -- <contact type="..">
                                    contactElement->SetAttribute( elementAttributeName, elementAttributeValue );

                                    // Add text to the contact.
                                    // Results in -- <contact type="email">j@j.com</contact>
                                    contactElement->InsertEndChild( *textElement );

                                    // Add the new contact to <notification>
                                    notificationElement->InsertEndChild(*contactElement);

                                    result = OS_SUCCESS ;
                                }
                            }
                        } else if( action.compareTo("editnotification") == 0 )
                        {
                            // Get the list of emails for new voicemail notification
                            for( TiXmlElement *contactElement = notificationNode->FirstChildElement("contact");
                                contactElement;
                                contactElement = contactElement->NextSiblingElement("contact") )
                            {
                                // Get the text in the element
                                if( contactElement->FirstChild() && contactElement->FirstChild()->Type() == TiXmlNode::TEXT )
                                {
                                    UtlString contactAddress = contactElement->FirstChild()->Value() ;

                                    if( contactAddress.compareTo(elementId, UtlString::ignoreCase) == 0 )
                                    {
                                        contactElement->FirstChild()->SetValue( elementValue );
                                        contactElement->SetAttribute( elementAttributeName, elementAttributeValue );
                                        result = OS_SUCCESS ;
                                        break ;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Save the updated document
            if( result == OS_SUCCESS )
            {
                if ( doc.SaveFile( prefsFileLocation ) == false )
                    result = OS_FAILED;
            }
        }
    }
    return result;
}


OsStatus
MailboxManager::addFolder(
    const UtlString& mailboxIdentity,
    const UtlString& foldername ) const
{
    UtlString mailboxPath, logContents ;
    OsStatus result = getMailboxPath( mailboxIdentity, mailboxPath ) ;

    if( result == OS_SUCCESS )
    {
        UtlString folderPath = mailboxPath + OsPathBase::separator + foldername ;

        // Check whether the foldername has already existed or not (ignore the case sensitivity)
        UtlSortedList folderList;
        getMailboxFolders( mailboxIdentity, folderList );
        UtlBoolean foundExistingOne = false;

        while( folderList.entries() > 0 )
        {
            UtlString* rwFolderName = (UtlString*) folderList.removeAt(0);
            UtlString existingFolder = rwFolderName->data();
            delete rwFolderName ;

            if(existingFolder.compareTo(foldername, UtlString::ignoreCase) == 0)
            {
                foundExistingOne = true;
                break;
            }
        }

        if (!foundExistingOne)
        {
            result = OsFileSystem::createDir ( folderPath );
            if ( result != OS_SUCCESS )
            {
                logContents = "Failed to create folder: " + folderPath ;
            }
        }
        else
        {
            result = OS_FAILED;
            logContents = "Found an existing mailbox " + mailboxIdentity ;
        }
    }
    else
    {
        logContents = "Unable to get mailbox path for " + mailboxIdentity ;
    }

    if( result != OS_SUCCESS )
        writeToLog( "AddFolder" , logContents, PRI_ERR) ;

    return result ;
}

OsStatus
MailboxManager::deleteFolder(
    const UtlString& mailboxIdentity,
    const UtlString& foldername ) const
{
    UtlString mailboxPath, logContents ;
    OsStatus result = getMailboxPath( mailboxIdentity, mailboxPath ) ;

    if( result == OS_SUCCESS )
    {
        OsPath folderPath = mailboxPath + OsPathBase::separator + foldername ;
        OsDir folderDir ( folderPath ) ;

        UtlBoolean isRecursive = TRUE ;
        UtlBoolean force = TRUE ;

        result = folderDir.remove( isRecursive, force );
        if( result != OS_SUCCESS )
        {
            result = OS_FAILED ;
            logContents = "Unable to delete dir as path is not found - " + folderPath ;
        }
    } else
    {
        logContents = "Unable to get mailbox path for " + mailboxIdentity ;
    }

    if( result != OS_SUCCESS )
        writeToLog( "DeleteFolder" , logContents, PRI_ERR) ;

    return result ;
}

OsStatus
MailboxManager::editFolder(
    const UtlString& mailboxIdentity,
    const UtlString& oldFoldername,
    const UtlString& newFoldername ) const
{
    UtlString mailboxPath, logContents ;
    OsStatus result = getMailboxPath( mailboxIdentity, mailboxPath ) ;

    if( result == OS_SUCCESS )
    {
        OsPath folderPath = mailboxPath + OsPathBase::separator + oldFoldername ;
        OsPath renamedPath = mailboxPath + OsPathBase::separator + newFoldername ;
        OsDir folderDir ( folderPath ) ;

        result = folderDir.rename( renamedPath );
        if( result != OS_SUCCESS )
        {
            logContents = "Unable to rename dir as path is not found - " + folderPath ;
        }
    }
    else
    {
        logContents = "Unable to get mailbox path for " + mailboxIdentity ;
    }

    if( result != OS_SUCCESS )
        writeToLog( "EditFolder" , logContents, PRI_ERR) ;

    return result ;
}

OsStatus
MailboxManager::saveMessage (
    const Url& fromUrl,
    const UtlString& mailboxIdentity,
    const UtlString& duration,
    const UtlString& timestamp,
    const char* data,
    const int& datasize,
    const UtlString& nextMessageID,
    const UtlBoolean& saveIfDataIsEmpty,
    const UtlBoolean& sendEmail)
{
   OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                 "MailboxManager::saveMessage(fromUrl = '%s', mailboxIdentity = '%s', duration = '%s', timestamp = '%s', datasize = %d, nextMessageId = '%s', saveIfDataIsEmpty = %d)",
                 fromUrl.toString().data(), mailboxIdentity.data(),
                 duration.data(), timestamp.data(), datasize,
                 nextMessageID.data(), saveIfDataIsEmpty);
   OsStatus result = OS_FAILED ;
   UtlString mailboxPath, logContent;

   result = getMailboxPath( mailboxIdentity, mailboxPath);
   if ( result == OS_SUCCESS )
   {
      // Create a new message identifier to name the files
      if (!m_pMsgIDGenerator)
      {
         OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                       "MailboxManager::saveMessage: calling MessageIDGenerator::getInstance, m_mailstoreRoot = '%s'",
                       m_mailstoreRoot.data());
         m_pMsgIDGenerator = MessageIDGenerator::getInstance(m_mailstoreRoot);
      }
      UtlString saveFolder = mailboxPath + OsPathBase::separator + m_inboxFolder;
      OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                    "MailboxManager::saveMessage: saveFolder = '%s'",
                    saveFolder.data());

      // check if message id was passed in the parameter (forwarded messages)
      UtlString messageid;
      if ( nextMessageID == "-1" )
      {
         // Generate the message id.
         result = m_pMsgIDGenerator->getNextMessageID( messageid );
      }
      else
      {
         messageid = nextMessageID;
         result = OS_SUCCESS;
      }

      if ( result == OS_SUCCESS )
      {
         UtlString nameWithoutExtension (
            saveFolder + OsPathBase::separator +
            messageid + "-00");

         // Create the wave file
         int file = open( nameWithoutExtension + ".wav", O_BINARY | O_CREAT | O_RDWR, 0644);
         unsigned long bytes_written = 0;
         if ( file != -1 )
         {
            bytes_written = write( file, data, datasize );
            if ( bytes_written > 0 || saveIfDataIsEmpty)
            {
               // create the metadata nextMessageID-00.xml
               OsFile metaDataFile ( nameWithoutExtension + ".xml");
               if ( metaDataFile.open( OsFile::CREATE ) == OS_SUCCESS )
               {
                  UtlString xmlFriendlyFrom = fromUrl.toString();
                  MailboxManager::convertUrlStringToXML( xmlFriendlyFrom );
                  char xmlText[1024];
                  sprintf (
                     xmlText,
                     "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                     "<messagedescriptor>\n"
                     "  <id>%s</id>\n"
                     "  <from>%s</from>\n"
                     "  <durationsecs>%d</durationsecs>\n"
                     "  <timestamp>%s</timestamp>\n"
                     "  <subject>" + UtlString( DEFAULT_MESSAGE_SUBJECT ) +
                     " " + messageid + "</subject>\n"
                     "  <priority>normal</priority>\n"
                     "</messagedescriptor>",
                     mailboxIdentity.data(),
                     xmlFriendlyFrom.data(),
                     (atoi(duration.data())/1000),
                     timestamp.data());

                  result = metaDataFile.write(
                     xmlText, strlen (xmlText), bytes_written );

                  if ( result == OS_SUCCESS )
                  {
                     // close the file handle before calling the status server
                     metaDataFile.close();

                     // create an empty nextMessageID-00.sta
                     // the presence of this file indicates that
                     // the file has not been heard
                     OsFile heardStatusFile ( nameWithoutExtension + ".sta");
                     heardStatusFile.touch();
                     heardStatusFile.close();

                     // Update any subscribers on the status of this mailbox
                     updateMailboxStatus(mailboxIdentity, saveFolder);

                     UtlString from;
                     fromUrl.getDisplayName(from);

                     UtlString userId;
                     fromUrl.getUserId(userId);

                     if (!from.isNull() && from.length() > 0)
                        from += " - " + userId;
                     else
                        from = userId;

                     UtlString wavFileName;
                           
                     if (nextMessageID == "-1")
                     {
                        // Message from direct deposit
                        wavFileName = messageid + "-00.wav";
                     }
                     else
                     {
                        // Message from forwarding
                        wavFileName = messageid + "-FW.wav";
                     }

                     if (sendEmail)
                     {
                         sendEmailNotification ( mailboxIdentity ,
                                            from,
                                            timestamp,
                                            duration,
                                            wavFileName,
                                            data,
                                            datasize);
                      }

                  } 
                  else
                  {
                     // Delete the WAV and XML file as we failed to
                     // write to the descriptor.
                     OsFile wavFile( nameWithoutExtension + ".wav" );
                     wavFile.remove();
                     metaDataFile.remove();
                     metaDataFile.close();
                     logContent = "Unable to write to message descriptor file : " + nameWithoutExtension + ".xml\n" ;
                     OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_ERR,
				     "MailboxManager::saveMessage: metaDataFile.write() failed, filename '%s.xml'",
                                   nameWithoutExtension.data());
                  }
               } else
               {
                  // Delete the WAV and XML file as we failed to
                  // write to the descriptor.
                  OsFile wavFile( nameWithoutExtension + ".wav" );
                  wavFile.remove();
                  logContent = "Unable to create message descriptor file : " + nameWithoutExtension + ".xml\n" ;
                  OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_ERR,
                                "MailboxManager::saveMessage: Error opening metadata file '%s.xml' for output, errno = %d '%s'",
                                nameWithoutExtension.data(),
                                errno, strerror(errno));
               }
            } else
            {
               OsFile wavFile( nameWithoutExtension + ".wav" );
               wavFile.remove();
               logContent = "Failed to write recorded voicemail to " + nameWithoutExtension + ".wav\n" ;
               OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_ERR,
                             "MailboxManager::saveMessage: write(fd = %d, size = %d) failed, fd opened on audio file '%s.wav', errno = %d '%s'",
                             file, datasize, nameWithoutExtension.data(),
                             errno, strerror(errno));
            }

            // succesfully saved wav file close file
            close(file);

         } else
         {
            logContent = "Unable to create file " + nameWithoutExtension + ".wav\n";
            OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_ERR,
                          "MailboxManager::saveMessage: Error opening audio file '%s.wav' for output: errno = %d '%s'",
                          nameWithoutExtension.data(), errno, strerror(errno));
         }
      } else
      {
         logContent = "Failed to get the next message id for " +
            mailboxIdentity;
         OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_ERR,
                       "MailboxManager::saveMessage: m_pMsgIDGenerator->getNextMessageID() failed");
      }
   } else
   {
      logContent = "Unable to get mailbox path for " + mailboxIdentity;
      OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_ERR,
                    "MailboxManager::saveMessage: getMailboxPath('%s') failed",
                    mailboxIdentity.data());
   }

   if( result != OS_SUCCESS )
   {
      writeToLog( "SaveMessage" , logContent, PRI_ERR);
   }

   OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                 "MailboxManager::saveMessage: returns %d", result);
   return result;
}

OsStatus
MailboxManager::getConfigValue (
    const TiXmlNode& node,
    const UtlString& key,
    UtlString& value ) const
{
    OsStatus result = OS_SUCCESS;
    TiXmlNode* configNode = (TiXmlNode*)node.FirstChild( key );

    if ( (configNode != NULL) && (configNode->Type() == TiXmlNode::ELEMENT) )
    {
        // convert the node to an element
        TiXmlElement* elem = configNode->ToElement();
        if ( elem != NULL )
        {
            TiXmlNode* childNode = elem->FirstChild();
            // If the content of elem is null, or (for some
            // reason) all whitespace, it has no children, and
            // childNode is NULL.  In that case, return "".
            if (childNode == NULL)
            {
               value = "";
            }
            else if( childNode && childNode->Type() == TiXmlNode::TEXT )
            {
                TiXmlText* elementValue = childNode->ToText();
                if (elementValue)
                {
                    value = elementValue->Value();
                } else
                {
                    result = OS_FAILED;
                }
            } else
            {
                result = OS_FAILED;
            }
        } else
        {
            result = OS_FAILED;
        }
    } else
    {
        result = OS_FAILED;
    }

    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                  "MailboxManager::getConfigValue('%s') returns result = %d value = '%s'",
                  key.data(), result,
                 (result == OS_SUCCESS ? value.data() : "[unknown]"));
    return result;
}

OsStatus
MailboxManager::setConfigValue (
    const TiXmlNode& node,
    const UtlString& key,
    const UtlString& newValue ) const
{
    OsStatus result = OS_SUCCESS;
    TiXmlNode* configNode = (TiXmlNode*)node.FirstChild( key );

    if ( (configNode != NULL) && (configNode->Type() == TiXmlNode::ELEMENT) )
    {
        // convert the node to an element
        TiXmlElement* elem = configNode->ToElement();
        if ( elem != NULL )
        {
            TiXmlNode* childNode = elem->FirstChild();
            // If the content of elem is null, or (for some
            // reason) all whitespace, it has no children, and
            // childNode is NULL.  In that case, we have to construct
            // a new child node.
            if (childNode == NULL)
            {
               TiXmlText newChildNode(newValue);
               elem->InsertEndChild(newChildNode);
            }
            else if( childNode && childNode->Type() == TiXmlNode::TEXT )
            {
                childNode->SetValue( newValue );
            } else
            {
                result = OS_FAILED;
            }
        } else
        {
            result = OS_FAILED;
        }
    } else
    {
        result = OS_FAILED;
    }
    return result;
}

OsStatus
MailboxManager::doLogin (
    const UtlString& loginString,
    const UtlString& loginPassToken,
    UtlString& rMailboxIdentity,
    UtlString& rExtension )
{
#ifdef WIN32
//  DebugBreak();
#endif

    UtlString logContent, dummyContactUserID, dummyContactDomain;

    // lazy create the Mailbox if the extension is valid
    OsStatus result = validateMailbox (
        loginString,       // userid/extension with/without domain
        TRUE,              // resolve extension
        TRUE,              // check permissions
        rMailboxIdentity,  // URL of actual mailbox found
        rExtension );

        if (result == OS_SUCCESS)
    {
                // Common security library algorithm used in AuthModule and also here
        if ( !SIPXAuthHelper::getInstance()->isAuthorizedUser (
                    rMailboxIdentity,   // URL of mailbox
                    loginPassToken,     // password (numeric PIN here)
                    m_defaultRealm,     // realm
                    m_defaultDomain,    // domain
                    TRUE,               // check permissions also
                    dummyContactUserID, // only used in auth module
                    dummyContactDomain, // only used in auth module
                    logContent) )       // error logging info
            {
                logContent =
                    "User : " + loginString +
                    " Mailbox : " + rMailboxIdentity +
                    " Password : " + loginPassToken +
                    " not authorized for realm : " + m_defaultRealm;
                result = OS_FAILED;
            }
        } else
        {
            logContent = "Could not validate/create mailbox: " + loginString;
        }

    if( result != OS_SUCCESS )
        writeToLog( "doLogin", logContent, PRI_INFO);

    return result;
}

OsStatus
MailboxManager::getMailboxStatus(
   const UtlString& mailboxIdentity,
   const UtlString& folderName,
   UtlString& rUnheardCount,
   UtlString& rTotalCount ) const
{
   OsStatus result = OS_SUCCESS;
   UtlString mailboxPath, logContents;
   result = getMailboxPath( mailboxIdentity, mailboxPath );
   if ( result == OS_SUCCESS )
   {
      // Get the message counts
      int unheardCount;
      int totalCount;

      UtlString folderPath;
      folderPath = mailboxPath + OsPathBase::separator + folderName;
      getMailboxCounts( folderPath, unheardCount, totalCount );

      char count[10];
      sprintf( count, "%d", unheardCount);
      rUnheardCount = count;
      sprintf( count, "%d", totalCount);
      rTotalCount = count;
   }
   else
   {
      rUnheardCount = "0";
      rTotalCount = "0";
      // Set the result to success anyway.
      result = OS_SUCCESS ;
   }
   return result;
}

OsStatus
MailboxManager::getWellFormedURL (
    const UtlString& inputString,
    Url&  rUrl ) const
{
    OsStatus result = OS_SUCCESS;

    // test for well formed non url encoded sip address
    // sip:user@domain or SIP:user@domain or user@domain (identity)
    if ( ( inputString.index ("sip:", 0, UtlString::ignoreCase ) != UTL_NOT_FOUND ) &&
         ( inputString.index ("@") != UTL_NOT_FOUND ) )
    {
        rUrl = inputString;
    } else if ( inputString.index ("@") != UTL_NOT_FOUND )
    {
        // this is more than likely an identity (without the sip: leader)
        rUrl = UtlString("sip:") + inputString;
    } else
    {
        // login over the web or just a {vdigits} string
        rUrl = UtlString("sip:") + inputString + "@" + m_defaultDomain;
    }
    return result;
}

OsStatus
MailboxManager::convertUrlStringToXML ( UtlString& value )
{
    OsStatus result = OS_SUCCESS;

    UtlString charsTobeEscaped("'&");
    HttpMessage::escapeChars(value, charsTobeEscaped);

    // If the displayname contains double quote symbols
    // replace them with &quot; html substitutions
    if ( value(0) == '"') {
        UtlString temp = value;
        temp.remove ( 0, 1);
        temp.prepend ("&quot;");
        int closeQuoteIndex = temp.index ('"');
        if (closeQuoteIndex > 0)
        {
            temp.remove ( closeQuoteIndex, 1 );
            temp.insert ( closeQuoteIndex, "&quot;" );
        }
        value = temp;
    }

    // Check to see if it's a sip address
    unsigned int index = 0;
    while ( (index = value.first ('<')) != UTL_NOT_FOUND )
    {
        UtlString temp = value;
        if ( index >= 0 )
        {
            temp.remove ( index, 1 );
            temp.insert ( index, "&lt;" );
        }
        value = temp;
    }

    while ( (index = value.first ('>')) != UTL_NOT_FOUND )
    {
        UtlString temp = value;
        if ( index != UTL_NOT_FOUND )
        {
            temp.remove ( index, 1 );
            temp.insert ( index, "&gt;" );
        }
        value = temp;
    }

    return result;
}

OsStatus
MailboxManager::convertXMLStringToURL ( UtlString& value )
{
    OsStatus result = OS_SUCCESS;
    // replace the &quot; &lt; and &gt; symbols with their ", < and >
    // symbols, this will make a string that is suitable for constructing
    // SIP Urls
    if ( value.index ("&quot;") == 0 )
    {
        UtlString temp = value;
        // Strip Leading "&quot;"
        temp.remove (0, 6);
        temp.prepend("\"");
        int closeQuoteIndex = temp.index ("&quot;");
        if (closeQuoteIndex > 0)
        {
            temp.remove ( closeQuoteIndex, 6 );
            temp.insert ( closeQuoteIndex, '"' );
        }
        value = temp;
    }

    if ( value.index ("&lt;") != UTL_NOT_FOUND )
    {
        UtlString temp = value;
        unsigned int indexLT = temp.index ("&lt;");
        temp.remove (indexLT, 4);
        temp.insert (indexLT, "<");
        unsigned int indexGT = temp.index ("&gt;");
        if ( indexGT != UTL_NOT_FOUND )
        {
            temp.remove (indexGT, 4);
            temp.insert (indexGT, ">");
        }
        value = temp;
    }
    return result;
}

OsStatus
MailboxManager::validateMailbox (
    const UtlString& loginString,
    const UtlBoolean& resolveExtensionFlag,
    const UtlBoolean& checkPermissions,
    UtlString& rMailboxIdentity,
    UtlString& rExtension )
{
    OsStatus result = OS_SUCCESS;
    UtlString logContent;

    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                  "MailboxManager::validateMailbox: loginString = '%s', resolveExtensionFlag = %d, checkPermissions = %d",
                  loginString.data(), resolveExtensionFlag, checkPermissions);

    // Hack debugging info
    logContent = "entering with arguments " + loginString;
    writeToLog( "validateMailbox", logContent, PRI_DEBUG );

    // Strip leading sip:'s as these do not work well when looking
    // up the extensions or alias tables but they work really well
    // when constructing URL's (as the default protocol is 'sip'
    UtlString userOrExtensionAtOptDomain(loginString);
    unsigned int sipIndex = userOrExtensionAtOptDomain.index("sip:");
    if ( sipIndex  != UTL_NOT_FOUND )
    {
        // see if we're being passed in a full URL in which
        // case strip it down to its identity, discarding the display name
        if ( sipIndex > 0 )
        {
            // Hack debugging info
            logContent = "detected full SIP uri, converting " +
                userOrExtensionAtOptDomain +
                " to sip identity\n";
            Url loginUrl ( userOrExtensionAtOptDomain );
            loginUrl.getIdentity( userOrExtensionAtOptDomain );
            logContent = "changed userOrExtensionAtOptDomain to " +
                userOrExtensionAtOptDomain + "\n";
            writeToLog( "validateMailbox", logContent, PRI_DEBUG );
        } else
        {
            // sip: is in first position
            userOrExtensionAtOptDomain =
                userOrExtensionAtOptDomain( 4, userOrExtensionAtOptDomain.length() - 4 );
        }
    }

    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                  "MailboxManager::validateMailbox: userOrExtensionAtOptDomain = '%s'",
                  userOrExtensionAtOptDomain.data());

    static UtlString identityKey ("identity");
    static UtlString contactKey ("contact");
    static UtlString permissionKey ("permission");

    // Make sure that the login string is valid
    if ( !userOrExtensionAtOptDomain.isNull() )
    {
        UtlString sipAddress;
        OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                      "MailboxManager::validateMailbox: userOrExtension = '%s'",
                      userOrExtensionAtOptDomain.data());

        // Look up credentials database to see if we can find a match
        UtlString realmName, authType;
        bool validated = FALSE;

        // Infinite loop detector
        int loopCount = 0;
        while ( !validated && ( result != OS_FAILED ) && (loopCount < 4) )
        {
           // this method should return a good URL from the input string
           // even if it is url encoded or if it comes directly from the web
           // where it may not be url encoded or may be missing the 'sip:' 
           // prefix
           Url mailboxUrl;
           getWellFormedURL ( userOrExtensionAtOptDomain, mailboxUrl );
           OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                         "MailboxManager::validateMailbox: top of loop, loopCount = %d mailboxUrl = '%s'",
                         loopCount, mailboxUrl.toString().data());

            // check the IMDB for a matching identity
            if (CredentialDB::getInstance()->isUriDefined(
                    mailboxUrl,  // IN
                    realmName,   // OUT
                    authType )) // OUT
            {
               OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                             "MailboxManager::validateMailbox: CredentialDB::...isUriDefined succeeded, mailboxUrl = '%s', realmName = '%s', authType = '%s'",
                             mailboxUrl.toString().data(), realmName.data(),
                             authType.data());
                writeToLog( "validateMailbox", "found mailboxUrl", PRI_DEBUG );
                // Flag to indicate if we found correct permission for this credential
                UtlBoolean permissionFound = FALSE;
                if( checkPermissions )
                {
                    writeToLog( "validateMailbox", "checking permissions", PRI_DEBUG );
                    // check for 'Voicemail' permission entry in IMDB
                    ResultSet permissions;
                    PermissionDB::getInstance()->
                        getPermissions( mailboxUrl, permissions );

                    if ( permissions.getSize() > 0 )
                    {
                        // Ensure user has 'Voicemail' permissions before proceeding
                        for ( int i=0; i<permissions.getSize(); i++ )
                        {
                            UtlHashMap record;
                            permissions.getIndex( i, record );
                            UtlString permission = *((UtlString*)record.findValue(&permissionKey));

                            if ( permission.compareTo( "Voicemail", UtlString::ignoreCase ) == 0 )
                            {
                                // fixup to prevent infinite loop where the
                                // uri does not have a permission set
                                permissionFound = TRUE;

                                // Exit the for loop with or without a good status
                                // as we found a good set of permissions,
                                validated = TRUE;
                                OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                                              "MailboxManager::validateMailbox: found voicemail permission");
                                break;
                            }
                        }
                    }
                } else
                {
                    permissionFound = TRUE;
                    validated = TRUE;
                    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                                  "MailboxManager::validateMailbox: did not check permissions");
            }

                // it is possible that even though there is an uri defined
                // for the user logging in, they may not have the persmission set
                if( permissionFound == FALSE )
                {
                    writeToLog( "validateMailbox",
                                (UtlString)"Voicemail Disabled for - " + mailboxUrl.toString(),
                                PRI_INFO);
                    result = OS_FAILED;
                } else
                {
                    writeToLog( "validateMailbox",
                                (UtlString)"Voicemail Enabled for - " + mailboxUrl.toString(),
                                PRI_DEBUG);
                    UtlString mailboxId;
                    mailboxUrl.getUserId(mailboxId);
                    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                                  "MailboxManager::validateMailbox: mailboxId = '%s'", mailboxId.data());
                    // Make sure we have a userid
                    if ( !mailboxId.isNull() )
                    {
                        UtlString inboxDirName =
                            m_mailstoreRoot + OsPathBase::separator +
                            MAILBOX_DIR + OsPathBase::separator +
                            mailboxId + OsPathBase::separator + 
			    m_inboxFolder;
                        OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                                      "MailboxManager::validateMailbox: inboxDirName = '%s'",
                                      inboxDirName.data());

                        OsDir inboxDir ( inboxDirName.data() );

                        // return the validated identity to the caller
                        mailboxUrl.getIdentity( rMailboxIdentity );

                        // If the mailbox does not exist, create it
                        // from scratch using the folder names
                        // defined in m_templateFolderNames
                        if ( !inboxDir.exists() )
                        {
                            result = createMailbox( rMailboxIdentity );
                            OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                                          "MailboxManager::validateMailbox: createMailbox('%s') = %d",
                                          rMailboxIdentity.data(), result);
                        }

                        if ( result == OS_SUCCESS )
                        {
                            // we don't always resolve a best extension
                            if ( resolveExtensionFlag )
                            {
                                logContent = "resolving extension for " + mailboxUrl.toString();
                                writeToLog( "validateMailbox", logContent, PRI_DEBUG);
                                // we must look up the aliases / extensions here also
                                rExtension.remove (0);

                                // Search using the alias identity and ensure that only
                                // one contact is associated with this alias
                                writeToLog( "validateMailbox",
                                            (UtlString)"Searching for optional extension associated with - " +
                                            mailboxUrl.toString(), PRI_DEBUG);

                                // do this and not the code below
                                if ( !ExtensionDB::getInstance()->getExtension( mailboxUrl, rExtension ) )
                                {
                                    writeToLog( "validateMailbox", "warning could not resolve an extension", PRI_DEBUG);
                                    // This is not fatal
                                    // result = OS_FAILED;
                                }
                            }
                        }
                    }
                }
            } else // Perhaps loginString is an alias (superset of extensions)
            {
                logContent =
                    "Warning - " +
                    userOrExtensionAtOptDomain +
                    " not found in CredentialDB, searching ExtensionDB\n";
                writeToLog( "validateMailbox", logContent, PRI_DEBUG);

                // search for a mailbox URL associated with the userOrExtensionAtOptDomain
                UtlBoolean mailboxUrlFound =
                    ExtensionDB::getInstance()->
                        getUri ( userOrExtensionAtOptDomain, mailboxUrl );
                OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                              "MailboxManager::validateMailbox: ExtensionDB::...getInstance called, userOrExtensionAtOptDomain = '%s', mailboxUrl = '%s', mailboxUrlFound = %d",
                              userOrExtensionAtOptDomain.data(),
                              mailboxUrl.toString().data(), mailboxUrlFound);
                UtlString dbPassToken, dbAuthType;
                if ( !mailboxUrlFound )
                {
                    // searching using the userOrExtensionAtOptDomain did
                    // not work, search again (making sure to add/remove
                    // the domain portion)
                    if ( userOrExtensionAtOptDomain.index("@") == UTL_NOT_FOUND )
                    {
                        userOrExtensionAtOptDomain =
                            userOrExtensionAtOptDomain + "@" + m_defaultDomain;
                    } else if ( loginString.index( m_defaultDomain) != UTL_NOT_FOUND )
                    {
                        userOrExtensionAtOptDomain =
                            userOrExtensionAtOptDomain(0,
                            userOrExtensionAtOptDomain.index( m_defaultDomain ) -1 );
                    }
                    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                                  "MailboxManager::validateMailbox: userOrExtensionAtOptDomain = '%s'",
                                  userOrExtensionAtOptDomain.data());

                    logContent =
                        "No Match, searching ExtensionDB with/(out) realm - " +
                        userOrExtensionAtOptDomain;
                    writeToLog( "validateMailbox", logContent, PRI_DEBUG);

                    // search again adding with a realm in the search
                    mailboxUrlFound  =
                        ExtensionDB::getInstance()->
                            getUri ( userOrExtensionAtOptDomain, mailboxUrl );
                    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                                  "MailboxManager::validateMailbox: ExtensionDB::...getUri called, userOrExtensionAtOptDomain = '%s', mailboxUrl = '%s', mailboxUrlFound = %d",
                                  userOrExtensionAtOptDomain.data(),
                                  mailboxUrl.toString().data(),
                                  mailboxUrlFound);

                    if ( !mailboxUrlFound )
                    {   // Write to the log and exit the loop.
                        logContent = "Warning couldn't find - " +
                            userOrExtensionAtOptDomain +
                            " in ExtensionsDB, trying Aliases for Unique Row";
                        writeToLog( "validateMailbox", logContent, PRI_DEBUG);

                        ResultSet aliasContacts;

                        // Ensure that we construct a valid Url to search the AliasDB
                        Url aliasIdentityUrl;
                        if ( userOrExtensionAtOptDomain.index("@") != UTL_NOT_FOUND)
                        {
                            aliasIdentityUrl = userOrExtensionAtOptDomain;
                        } else
                        {
                            aliasIdentityUrl = userOrExtensionAtOptDomain + "@" + m_defaultDomain;
                        }
                        OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                                      "MailboxManager::validateMailbox: aliasIdentityUrl = '%s'",
                                      aliasIdentityUrl.toString().data());

                        logContent = "Searching AliasDB for contact for alias - " +
                            aliasIdentityUrl.toString();
                        writeToLog( "validateMailbox", logContent, PRI_DEBUG);

                        AliasDB::getInstance()->getContacts( aliasIdentityUrl, aliasContacts );

                        int numRows = aliasContacts.getSize();

                        if ( numRows <= 0 )
                        {
                             logContent =
                                 "Failed, No alias found for : " +
                                 aliasIdentityUrl.toString();
                             writeToLog( "validateMailbox", logContent, PRI_DEBUG);
                             logContent = "FAILED- " +
                                mailboxUrl.toString();
                             writeToLog( "validateMailbox", logContent, PRI_DEBUG);
                             result = OS_FAILED;
                        } else // Successful match
                        {
                            // set the mailboxUrl to the first contact returned
                            // (ideally we want to loop thru them all till we
                            //  find a mailbox, but that's just asking too much
                            //  for now! --Woof!)
                            UtlHashMap record;
                            aliasContacts.getIndex(0, record) ;
                            UtlString *contact = (UtlString*)record.findValue(&contactKey);
                            if (contact != NULL)
                            {
                               mailboxUrl = *contact;
                               // return the validated identity to the caller
                               mailboxUrl.getIdentity( rMailboxIdentity );
                               logContent = "Success, found contact setting mailboxUrl to - " +
                                   rMailboxIdentity;
                               writeToLog( "validateMailbox", logContent, PRI_DEBUG);
                               // knowing the alias exists is enough.
                               // return that and be happy.
                               validated = TRUE ;
                            }
                            else
                            {
                               logContent = "FAILED- did not find contact for alias" + aliasIdentityUrl.toString();
                               writeToLog( "validateMailbox", logContent, PRI_DEBUG);
                               result = OS_FAILED;
                            }
                        }
                    } else  // mailboxUrl is now valid
                    {
                        // update the userOrExtensionAtOptDomain
                        CredentialDB::getInstance()->getUserPin (
                            mailboxUrl,                  // IN
                            m_defaultRealm,              // IN
                            userOrExtensionAtOptDomain,  // OUT
                            dbPassToken,                 // OUT
                            dbAuthType );                // OUT
                        OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                                      "MailboxManager::validateMailbox: CredentialDB::...getUserPin called, mailboxUrl = '%s', m_defaultRealm = '%s', userOrExtensionAtOptDomain = '%s', dbPassToken = '%s', dbAuthType = '%s'",
                                      mailboxUrl.toString().data(),
                                      m_defaultRealm.data(),
                                      userOrExtensionAtOptDomain.data(),
                                      dbPassToken.data(), dbAuthType.data());
                    }
                } else
                {
                    logContent = "ExtensionsDB found mailboxUrl - " +
                        mailboxUrl.toString() + " extension " + loginString + "looking up credentials";

                    writeToLog( "validateMailbox", logContent, PRI_DEBUG);
                    // found the mailbox url associated with the extension now
                    // we need to query the userid from the DB and try again
                    // update the userOrExtensionAtOptDomain
                    if (!CredentialDB::getInstance()->getUserPin (
                            mailboxUrl,                  // IN
                            m_defaultRealm,              // IN
                            userOrExtensionAtOptDomain,  // OUT (updated for next time round the loop)
                            dbPassToken,                 // OUT
                            dbAuthType ) )               // OUT
                    {
                        // Write to the log and exit the loop.
                        logContent = "FAILED = Could not find Credential for mailboxUrl - " + mailboxUrl.toString();
                        writeToLog( "validateMailbox", logContent, PRI_DEBUG);
                        result = OS_FAILED;
                    }
                    else
                    {
                       OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                                     "MailboxManager::validateMailbox: CredentialDB::...getUserPin called, mailboxUrl = '%s', m_defaultRealm = '%s', userOrExtensionAtOptDomain = '%s', dbPassToken = '%s', dbAuthType = '%s'",
                                     mailboxUrl.toString().data(),
                                     m_defaultRealm.data(),
                                     userOrExtensionAtOptDomain.data(),
                                     dbPassToken.data(), dbAuthType.data());
                    }
                }
            }
            loopCount += 1;
        } // while

        // See if we've exceeded the loop count, which could happen if the IMDB tables
        // have a recurcive relationship, extension's contact URI is not in contacts for ex.
        if ( !validated && ( result != OS_FAILED ) )
        {
            writeToLog( "validateMailbox", "ERROR ExtensionDB does not have a valid CredentialDB Contact!\n", PRI_DEBUG);;
            result = OS_FAILED;
        }
    } else // must pass in a non null argument
    {
        logContent = "ERROR: loginString argument must not be set to null";
        result = OS_FAILED;
    }

    if (result == OS_FAILED)
    {
        writeToLog( "validateMailbox" , logContent, PRI_INFO) ;
        writeToLog( "validateMailbox" , loginString + " is not a valid userid/extension/alias", PRI_INFO);
    } else
    {
        logContent = "exiting succesfully";
        writeToLog( "validateMailbox", logContent, PRI_DEBUG );
    }
    return result;
}

// Codes for different manners of sorting messags.
enum {
   SORT_HEARD,
   SORT_UNHEARD,
   SORT_INBOX,
   SORT_OTHER};

/* Get information for a group of messages.
 * mailboxIdentity is which mailbox's messages we are looking for.
 * category is the category of messages we are looking for.
 * blocksize is the maximum number of messages to retrieve.
 * nextBlockHandle is the number of the last message NOT to include in the set,
 *      or <= 0 to select all messages.
 * msgOrderDescending is TRUE if we want to list messages in descending order
 *      by message ID, and FALSE if we want ascending order.
 */
OsStatus
MailboxManager::getMessageBlock(
    const UtlString& mailboxIdentity,
    UtlDList& msgBlock,
    const UtlString& category,
    const int blocksize,
    const int nextBlockHandle,
    UtlBoolean& endOfMessages,
    const UtlBoolean& isFromWeb,
    const UtlBoolean& msgOrderDescending) const
{
   OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                 "MailboxManager::getMessageBlock(mailboxIdentity = '%s', category = '%s', blocksize = %d, nextBlockHandle = %d, isFromWeb = %d, msgOrderDescending = %d)",
                 mailboxIdentity.data(), category.data(), blocksize,
                 nextBlockHandle, isFromWeb, msgOrderDescending);

   // Status variable used in many different places.
   OsStatus result = OS_FAILED;
   UtlString mailboxPath, logContents, folderPath;

   result = getMailboxPath (mailboxIdentity, mailboxPath);
   OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                 "MailboxManager::getMessageBlock: getMailboxPath(mailboxIdentity = '%s') returns %d, mailboxPath = '%s'",
                 mailboxIdentity.data(), result, mailboxPath.data());
   if (result == OS_SUCCESS)
   {
      UtlString mailboxFolder = getFolderName(category) ;
      folderPath = mailboxPath + OsPathBase::separator + mailboxFolder;
      // Get the code for how we are going to select and sort messages.
      int sort_criterion = strcmp(category, "heard") == 0 ? SORT_HEARD :
         strcmp(category, "unheard") == 0 ? SORT_UNHEARD :
         strcmp(category, "inbox") == 0 ? SORT_INBOX :
         SORT_OTHER;
      OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                    "MailboxManager::getMessageBlock: mailboxFolder = '%s', folderPath = '%s'",
                    mailboxFolder.data(), folderPath.data());

#if defined(__pingtel_on_posix__)
      // Set up a lock in the mailbox so that multiple access can be prevented
      UtlString mailboxLock = folderPath + OsPathBase::separator + "retrieve.lck";
      OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                    "MailboxManager::getMessageBlock: set up the lock file %s",
                    mailboxLock.data());
      OsFileLinux lockFile(mailboxLock);
      lockFile.open(OsFileBase::CREATE);
      lockFile.filelock(OsFileBase::FSLOCK_WAIT);
#endif

      OsPath path(folderPath);
      OsFileIterator fi(path);
      OsPath entry;

      // Sorted list for storing the message ids.
      UtlSortedList messageids;

      // Regular expression used for searching for the messages
      UtlString regExp = "[0-9]-00.wav";

      // Search for the message files in this folder.
      // We search for message file names using regExp, testing whether they
      // meet the various selection criteria.
      OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                    "MailboxManager::getMessageBlock: regExp '%s'",
                    regExp.data());
      for (result = fi.findFirst(entry, regExp, OsFileIterator::FILES);
           result == OS_SUCCESS;
           result = fi.findNext(entry))
      {
         // "entry" is the next directory entry to be considered.

         // Filenames are in the format 'xxxxxxxx-yy'.
         // Extract the 'xxxxxxxx' part.
         UtlString messageId = entry.getFilename();
         OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                       "MailboxManager::getMessageBlock: found file '%s' in directory",
                       messageId.data());
         messageId = messageId(0, messageId.index('-'));

         // Add the message to the set if its message ID passes the
         // selection criteria.

         // message_rejected is TRUE if we have detected that this message
         // should not be listed.
         UtlBoolean message_rejected = FALSE;
         // This is the priority used to sort the messages.
         // Most cases do not use priority, so we default it to 0.
         int priority = 0;

         // If we are selecting on message number, do the appropriate test.
         if (nextBlockHandle > 0)
         {
            int iFileName = atoi(messageId.data());

            message_rejected =
               msgOrderDescending ?
               iFileName >= nextBlockHandle :
               iFileName < nextBlockHandle;
            OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                          "MailboxManager::getMessageBlock: set message_rejected = %d based on nextBlockHandle",
                          message_rejected);
         }

         // If we are selecting on message heard/unheard, do the appropriate
         // test.
         if (!message_rejected && sort_criterion != SORT_OTHER)
         {
            // Test if a .sta file for this message exists, which indicates
            // that the message is unheard.
            UtlBoolean unheard =
               OsFileSystem::exists(path + OsPathBase::separator +
                                    messageId + "-00.sta");

            switch (sort_criterion) {
            case SORT_HEARD:
               // Searching for heard messages; reject unheard messages.
               message_rejected = unheard;
               break;
            case SORT_UNHEARD:
               // Searching for unheard messages; reject heard messages.
               message_rejected = !unheard;
               break;
            case SORT_INBOX:
               // Searching for inbox messages; have unheard messages sort
               // to the beginning.
               // Note that low priority numbers sort to the beginning of
               // the list.
               // However, for web UI, we will display all the messages in
               // chronicle order regardless whether the messages have been
               // read or not.
               if (!isFromWeb)
               {
                  priority = !unheard;
               }

               break;
            case SORT_OTHER:
               // Not possible.
               ;
            }
            OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                          "MailboxManager::getMessageBlock: message_rejected = %d based on unheard = %d, sort_criterion = %d",
                          message_rejected, unheard, sort_criterion);
         }

         // If the message is acceptable, add it to the set.
         if (message_rejected)
         {
            OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                          "MailboxManager::getMessageBlock: not adding '%s' to set because it fails selection",
                          messageId.data());
         }
         else
         {
            // Construct the collectable message ID.
            OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                          "MailboxManager::getMessageBlock: building CategorizedString(%d, '%s')",
                          priority, messageId.data());
            CategorizedString* collectableMessageId =
               new CategorizedString(priority, messageId);
            // Skip this directory entry if the set of message IDs
            // already contains this message ID.
            if (!messageids.contains(collectableMessageId))
            {
               messageids.insert(collectableMessageId);
            }
            else
            {
               OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                             "MailboxManager::getMessageBlock: '%s' already in set",
                             messageId.data());
               // Delete the temp string.
               delete collectableMessageId;
            }
         }
      }

      // At this point, result should be OS_FILE_NOT_FOUND, since that is the
      // exit criterion for the previous loop.
      if (result == OS_FILE_NOT_FOUND)
      {
         // Get the information about the first/last N messages.
         // N is either the total number of messages, or
         // 'blocksize' messages, if that is fewer.
         // If we want the messages in descending order by number, we
         // will reverse the order of the list in this step.
         while (messageids.entries() > 0 &&
                (blocksize <= 0 || (int)(msgBlock.entries()) < blocksize))
         {
            // As we execute this loop, we remove messages from the list
            // messageids, so the following gets the information about the
            // first/last unprocessed message in the list.
            int index = msgOrderDescending ? messageids.entries() - 1 : 0;
            CategorizedString* rwfilename =
               (CategorizedString*) messageids.removeAt(index);
            UtlString filename = rwfilename->data();

            // Find all <messageid>-**.xml files for this message ID.
            regExp = filename + ".+\\.xml";
            OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                          "MailboxManager::getMessageBlock: regExp = '%s'",
                          regExp.data());
            UtlSortedList messages;
            for (result = fi.findFirst(entry, regExp,
                                       OsFileIterator::FILES);
                 result == OS_SUCCESS;
                 result = fi.findNext(entry))
            {
               // Get the filename.
               filename = entry.getFilename();
               OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                             "MailboxManager::getMessageBlock: filename = '%s'",
                             filename.data());

               UtlString collectableFileName (filename);

               // Store it in a sorted vector.
               if (!messages.contains(&collectableFileName))
               {
                  UtlString temp = filename + ".xml";

                  if (temp != FOLDER_SUMMARY_FILE)
                  {
                     OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                                   "MailboxManager::getMessageBlock: adding '%s' to messages list",
                                   filename.data());
                     messages.insert(new UtlString(filename));
                  }
               }
            }
            OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                          "MailboxManager::getMessageBlock: no more files for regExp = '%s'",
                          regExp.data());

            // Again this is a successful result so reset to SUCCESS
            if (result == OS_FILE_NOT_FOUND)
               result = OS_SUCCESS;

            // this is a list hashDictionaries, each has info on its correspinding
            // message in the form of name/value pairs
            UtlDList* messageList = new UtlDList() ;

            // Iterate through the xml filename getting info about each message.
            // Store the information about each message in a HashDictionary.
            while ((messages.entries() > 0) && (result == OS_SUCCESS))
            {
               UtlString* rwMessageId = (UtlString*) messages.get();
               OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                             "MailboxManager::getMessageBlock: rwMessageId = '%s'",
                             rwMessageId->data());

               filename = rwMessageId->data();
               // Construct a Hash dictionary for storing the attributes
               // of a message - message id, from, timestamp, etc.
               UtlHashMap* msgHashDictionary = new UtlHashMap();
               result = getMessageInfo (
                  mailboxIdentity,
                  mailboxFolder,
                  filename,
                  msgHashDictionary,
                  isFromWeb);
               OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                             "MailboxManager::getMessageBlock: getMessageInfo(mailboxIdentity = '%s', mailboxFolder = '%s', filename = '%s', isFromWeb = %d) returns %d",
                             mailboxIdentity.data(),
                             mailboxFolder.data(), filename.data(),
                             isFromWeb, result);
               if (result == OS_SUCCESS)
               {
                  messageList->insert(msgHashDictionary);
               }
               else
               {
                  delete msgHashDictionary;
               }
            }
            OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                          "MailboxManager::getMessageBlock: no more messages in the list");

            // Put the Hash dictionary into a UtlDListCollectable object.
            // This object stores the hash dictionaries for each message in
            // the block.
            // But if there is no information for the message, skip it.
            if (messageList->entries() > 0)
            {
               msgBlock.insert(messageList);
            }
         }
         // endOfMessages is true if there are no more messages that could
         // be fetched.
         endOfMessages = blocksize <= 0 || messageids.entries() == 0;
      }
      result = OS_SUCCESS;

#if defined(__pingtel_on_posix__)
      // Unlock the lock file in the mailbox
      OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                    "MailboxManager::getMessageBlock: unlock the lock file %s",
                    mailboxLock.data());
      lockFile.fileunlock();
      lockFile.close();
#endif
   }
   else
   {
      logContents = "Unable to obtain mailbox path for " + mailboxIdentity;
      writeToLog("GetMessageBlock", logContents, PRI_ERR);
   }
   OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                 "MailboxManager::getMessageBlock(mailboxIdentity = '%s') returns %d",
                 mailboxIdentity.data(), result);
   return result;
}


OsStatus
MailboxManager::getMessageInfo (
    const UtlString& mailboxIdentity,
    const UtlString& folderName,
    const UtlString& messageId,
    UtlHashMap* msgInfoHashDictionary,
    const UtlBoolean& isFromWeb) const
{
    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                  "MailboxManager::getMessageInfo(mailboxIdentity = '%s', folderName = '%s', messageId = '%s', msgInfoHashDictionary = %p, isFromWeb = %d) entered",
                  mailboxIdentity.data(), folderName.data(), messageId.data(),
                  msgInfoHashDictionary, isFromWeb);

    // Status variable used in many different places.
    OsStatus result = OS_FAILED;
    UtlString mailboxPath, logContents;

    result = getMailboxPath ( mailboxIdentity, mailboxPath );
    if ( result == OS_SUCCESS )
    {
        mailboxPath += OsPathBase::separator + folderName ;

        UtlString msgDescriptorPath  =
            mailboxPath + OsPathBase::separator + messageId + ".xml";

        UtlString msgWavFilePath =
            mailboxPath + OsPathBase::separator + messageId + ".wav";

        UtlString mailboxUrl;
        result = getMailboxURL( mailboxIdentity, mailboxUrl, isFromWeb );

        if ( result == OS_SUCCESS )
        {
            UtlString msgWavFileUrl = mailboxUrl + URL_SEPARATOR +
                                     folderName + URL_SEPARATOR +
                                     messageId + ".wav";
            UtlString msgHeardStatus;
            if( folderName == "unheard" )
            {
                msgHeardStatus = "unheard";
            } else if ( folderName == "heard" )
            {
                msgHeardStatus = "heard";
            } else
            {
                UtlString msgStatusFilePath =
                    mailboxPath + OsPathBase::separator + messageId + ".sta";
                if ( OsFileSystem::exists( msgStatusFilePath ) )
                    msgHeardStatus = "unheard";
                else
                    msgHeardStatus = "heard";
            }

            // Parse the message descriptor file and populate the hash dictionary.
            result = parseMessageDescriptor( msgDescriptorPath, msgInfoHashDictionary );

            if ( result == OS_SUCCESS )
            {
                // Insert the URL of the message file
                // This is not stored in the message descriptor XML file and
                // hence has to be added separately.
                msgInfoHashDictionary->insertKeyAndValue(
                    new UtlString("url"),
                    new UtlString(msgWavFileUrl));

                UtlString id = messageId(0, messageId.index('-'));

                msgInfoHashDictionary->insertKeyAndValue(
                    new UtlString("messageid"),
                    new UtlString(id));

                msgInfoHashDictionary->insertKeyAndValue(
                    new UtlString("status"),
                    new UtlString(msgHeardStatus));

                // Check if the combined WAV file is present -- <messageid>-FW.wav
                // This is only available for forwarded messages.
                UtlString combinedWavFileLocation =
                    mailboxPath + OsPathBase::separator + id + "-FW.wav";

                UtlString combinedWavUrl ;
                if( OsFileSystem::exists( combinedWavFileLocation ) )
                {
                    combinedWavUrl = mailboxUrl + URL_SEPARATOR +
                                     folderName + URL_SEPARATOR +
                                     id + "-FW.wav";
                }
                else
                {
                    combinedWavUrl = msgWavFileUrl ;
                }

                msgInfoHashDictionary->insertKeyAndValue(
                    new UtlString("playlisturl"),
                    new UtlString(combinedWavUrl));



            } else
            {
                logContents = "Unable to parseMessageDescriptor for " + msgDescriptorPath;
                writeToLog("GetMessageBlock", logContents, PRI_ERR );
            }
        } else
        {
            logContents = "Unable to obtain mailbox URL for " + mailboxIdentity ;
            writeToLog("GetMessageBlock", logContents, PRI_ERR );
        }
    }
    else
    {
        logContents = "Unable to obtain mailbox path for " + mailboxIdentity ;
        writeToLog("GetMessageBlock", logContents, PRI_ERR );
    }
   OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                 "MailboxManager::getMessageInfo: returns %d", result);
   return result;
}


OsStatus
MailboxManager::getMessageBlockHandles (
    const UtlString& mailboxIdentity,
    UtlSortedList& rNextBlockHandles,
    const UtlString& category,
    const int blocksize ) const
{
    // Status variable used in many different places.
    OsStatus result = OS_FAILED;
    UtlString mailboxPath, logContents, folderPath;

    result = getMailboxPath ( mailboxIdentity, mailboxPath );
    if ( result == OS_SUCCESS )
    {
        UtlString mailboxFolder = getFolderName( category ) ;
        folderPath = mailboxPath + OsPathBase::separator + mailboxFolder;

#if defined(__pingtel_on_posix__)
        // Set up a lock in the mailbox so that multiple access can be prevented
        UtlString mailboxLock = folderPath + OsPathBase::separator + "retrieve.lck";
        OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                      "MailboxManager::getMessageBlockHandles: set up the lock file %s",
                      mailboxLock.data());
        OsFileLinux lockFile(mailboxLock);
        lockFile.open(OsFileBase::CREATE);
        lockFile.filelock(OsFileBase::FSLOCK_WAIT);
#endif

        OsPath path( folderPath );
        OsFileIterator fi( path );
        OsPath entry;

        // Holds the list of messages in the folder.
        UtlSortedList svMessageId ;

        // Regular expression used for searching for the messages
        UtlString regExp = "[0-9]-00.wav";

        // Get all the message files in this folder.
        result = fi.findFirst( entry, regExp, OsFileIterator::FILES );

        int counter = 0;
        UtlString messageId ;

        // Apply a filter to the entire contents of the specified folder(category)
        while ( result == OS_SUCCESS )
        {
            // Filenames are in the format 'xxxxxxxx-yy'.
            // Extract the 'xxxxxxxx' part.
            messageId = entry.getFilename();
            messageId = messageId(0, messageId.index('-'));

            UtlString* collectableMessageId =
                new UtlString (messageId);

            // Add all the message ids to the sorted vector.
            // This is make sure that the message ids are in numeric order.
            if ( !svMessageId.contains( collectableMessageId ) )
                svMessageId.insert( collectableMessageId );
            else
                delete collectableMessageId;

            // Get the next message.
            result = fi.findNext(entry);
        }

        while( svMessageId.entries() > 0 )
        {
            // Retrieve the message ids from the end.
            // On the WebUI, messages are displayed in descending order (latest to oldest)
            UtlString* rwfilename =
                (UtlString*) svMessageId.removeAt(svMessageId.entries() -1);
            messageId = rwfilename->data();

            if ( !rNextBlockHandles.contains( rwfilename ) )
            {
                counter++ ;
                // Check if we have reached the id that is at the end of the block
                if( counter == blocksize )
                {
                    // Add the message id as a handle as it is the last message in the block.
                    // Reset the counter.
                    counter = 0;
                    rNextBlockHandles.insert(rwfilename);
                }
                else
                {
                    delete rwfilename;
                }
            }
            else
            {
                delete rwfilename;
            }
        }

        // This is to handle a special case when messages can be equally divided into blocks.
        // That is, if block size =5 and the number of messages = 10, then in that case,
        // there will be two entries - one for 5.wav and another for 10.wav (as counter was
        // equal to the block size when 10.wav was reached).
        // However, 10.wav is not really a handle as there are no more messages after it.
        // Remove it from the vector.
        if( rNextBlockHandles.entries() > 0 && counter == 0 )
        {
            // remove the last entry
            // In the PlayMessagesCGI, the block handles are read from the end
            // as messages have to be displayed in descending order.
            // Hence remove the first entry so that an unnecessary link will not be provided.
            rNextBlockHandles.remove(0);
        }

#if defined(__pingtel_on_posix__)
        // Unlock the lock file in the mailbox
        OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                      "MailboxManager::getMessageBlockHandles: unlock the lock file %s",
                      mailboxLock.data());
        lockFile.fileunlock();
        lockFile.close();
#endif
    }

    // This is questionable!
    // But does no harm as we only look at the number of entries
    // in the vector that is returned.
    return OS_SUCCESS ;

}


OsStatus
MailboxManager::updateMessageStates(
    const UtlString& mailboxIdentity,
    const UtlString& category,
    const UtlString& messageids)
{
   OsStatus result = OS_SUCCESS;
   UtlString mailboxPath, logContents;
   OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                 "MailboxManager::updateMessageStates(mailboxIdentity = '%s', category = '%s', messageids = '%s')",
                 mailboxIdentity.data(), category.data(), messageids.data());

   result = getMailboxPath(mailboxIdentity, mailboxPath);
   if (result == OS_SUCCESS)
   {
      mailboxPath += OsPathBase::separator + getFolderName(category);

#if defined(__pingtel_on_posix__)
      // Set up a lock in the mailbox so that multiple access can be prevented
      UtlString mailboxLock = mailboxPath + OsPathBase::separator + "retrieve.lck";
      OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                    "MailboxManager::updateMessageStates: set up the lock file %s",
                    mailboxLock.data());
      OsFileLinux lockFile(mailboxLock);
      lockFile.open(OsFileBase::CREATE);
      lockFile.filelock(OsFileBase::FSLOCK_WAIT);
#endif

      OsPath path(mailboxPath);
      OsFileIterator fi(path);
      OsPath entry;   // Object for storing the results of file system searches.
      int msgCount = 0;
      int errorCount = 0;

      if (messageids == REFRESH_ALL_MSG_STATES)
      {
	 // Refresh message states, trigger NOTIFY
         msgCount = 1;         
      } 
      else if (messageids == UPDATE_ALL_MSG_STATES)
      {
         // messageids == "-1" means to update all messages to "heard" status.
         // Delete all .sta files.
         for (result = fi.findFirst(entry, "^[-0-9]*.sta$",
                                    OsFileIterator::FILES);
              result == OS_SUCCESS;
              result = fi.findNext(entry))
         {
            // Delete the file.
            UtlString fileLocation =
               mailboxPath + OsPathBase::separator + entry;
            OsFile file(fileLocation);
            OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                          "MailboxManager::updateMessageStates fileLocation = '%s'",
                          fileLocation.data());
            file.remove();
            msgCount++;
         }
      }
      else if (messageids.contains(";"))
      {
         // messageids == "messageid;"
         // Indicates that all messages with ids <= messageid should
         // be updated.
         // Used by the TUI.

         // Remove the trailing semicolon
         UtlString sentid = messageids(0, messageids.length()-1);
         if (!sentid.isNull())
         {
            int messageid = atoi(sentid);
            for (result = fi.findFirst(entry, "^[-0-9]*.sta$",
                                       OsFileIterator::FILES);
                 result == OS_SUCCESS;
                 result = fi.findNext(entry))
            {
               int id = atoi(entry.getFilename());
               if (id <= messageid)
               {
                  // Delete the file.
                  UtlString fileLocation =
                     mailboxPath + OsPathBase::separator + entry;
                  OsFile file(fileLocation);
                  OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                                "MailboxManager::updateMessageStates fileLocation = '%s'",
                                fileLocation.data());
                  file.remove();
                  msgCount++;
               }
            }
         }
      }
      else if (messageids == "0")
      {
         // messageids == "0" means no action.
         logContents = "Messageids is '0', nothing to do.";
         writeToLog("UpdateMessageStates", logContents, PRI_DEBUG);
      }
      else
      {
         // The remaining case is a series of message numbers separated by
         // whitespace.
         // Retrieve individual message ids and update the status of each.
         // Use a tokenizer for retrieving the message ids.
         UtlTokenizer nameValuePair(messageids);
         UtlString id;
         while (nameValuePair.next(id, MESSAGE_DELIMITER))
         {
            id = id.strip(UtlString::both);
            // There is only one .sta file for a message.
            UtlString fromFileLocation =
               mailboxPath + OsPathBase::separator + id + "-00.sta";
            OsFile fromFile(fromFileLocation);
            fromFile.remove();
            OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                          "MailboxManager::updateMessageStates fromFileLocation = '%s'",
                          fromFileLocation.data());

            if (result == OS_SUCCESS)
            {
               msgCount++;
            }
            else
            {
               errorCount++;
               logContents = "message '" + id + "' does not exist at '" +
                  fromFileLocation + "'";
               writeToLog("MailboxManager::UpdateMessageStates", logContents,
                           PRI_DEBUG);
            }
         }
      }

      // Any error from here below sets result = OS_FAILURE and logContents.
      result = OS_SUCCESS;

      // If any messages were updated, update any subscribers
      if (msgCount > 0)
      {
         updateMailboxStatus(mailboxIdentity, mailboxPath);
      }

      // If any messages could not be found, produce an error message.
      if (errorCount > 0)
      {
         logContents = (const UtlString) ("Could not find " + errorCount) +
            " messages from list '" + messageids + 
            "' in " + category;
         result = OS_NOT_FOUND;
      }

#if defined(__pingtel_on_posix__)
      // Unlock the lock file in the mailbox
      OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                    "MailboxManager::updateMessageStates: unlock the lock file %s",
                    mailboxLock.data());
      lockFile.fileunlock();
      lockFile.close();
#endif
   }
   else
   {
      logContents = "Failed to obtain the mailbox path for " + mailboxIdentity;
   }

   if (result != OS_SUCCESS)
      writeToLog("MailboxManager::UpdateMessageStates", logContents, PRI_ERR);

   return result;
}

OsStatus
MailboxManager::moveMessages(
    const UtlString& mailboxIdentity,
    const UtlString& fromFolder,
    const UtlString& toFolder,
    const UtlString& messageIds,
    const UtlString& maintainstatus)
{
    OsStatus result = OS_FAILED;
    UtlString logContents ;

    if( fromFolder != toFolder )
    {
        UtlString mailboxPath, fromFolderPath, toFolderPath ;

        result = getMailboxPath( mailboxIdentity, mailboxPath );
        if( result == OS_SUCCESS )
        {
            fromFolderPath = mailboxPath + OsPathBase::separator + getFolderName( fromFolder ) ;
            toFolderPath = mailboxPath + OsPathBase::separator + getFolderName( toFolder ) ;

#if defined(__pingtel_on_posix__)
            // Set up a lock in the mailbox so that multiple access can be prevented
            UtlString mailboxLock = fromFolderPath + OsPathBase::separator + "retrieve.lck";
            OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                          "MailboxManager::moveMessages: set up the lock file %s",
                          mailboxLock.data());
            OsFileLinux lockFile(mailboxLock);
            lockFile.open(OsFileBase::CREATE);
            lockFile.filelock(OsFileBase::FSLOCK_WAIT);
#endif

            OsPath path(fromFolderPath);
            OsFileIterator fi(path);
            OsPath entry;

            UtlTokenizer nameValuePair(messageIds);
            UtlString id;
            int totalMsgsMoved = 0;
            int unheardMsgCount = 0;

            // Parse messageIds
            while ( nameValuePair.next(id, MESSAGE_DELIMITER) )
            {
                id = id.strip(UtlString::both);
                totalMsgsMoved++;

                // Find all files starting with the given message id.
                UtlString regExp = id + ".+\\.*";

                result = fi.findFirst(entry, regExp, OsFileIterator::FILES);

                while ( result == OS_SUCCESS )
                {
                    // Construct a file object.
                    UtlString fromFileLocation   = fromFolderPath + OsPathBase::separator + entry.getFilename() + entry.getExt();
                    OsFile fromFile(fromFileLocation);

                    // If maintainstatus is yes, copy all the files.
                    // If maintainstatus is no,  copy only the wav and xml files.
                    if( entry.getExt() == ".sta" )
                    {
                        if( maintainstatus == "yes" )
                        {
                            // Copy the file to destination folder.
                            UtlString toFileLocation =
                                toFolderPath + OsPathBase::separator +
                                entry.getFilename() + entry.getExt();

                            fromFile.copy(toFileLocation);
                        }
                        unheardMsgCount++;
                    } else
                    {
                        // Copy the file to destination folder.
                        UtlString toFileLocation =
                            toFolderPath + OsPathBase::separator +
                            entry.getFilename() + entry.getExt();

                        fromFile.copy(toFileLocation);
                    }

                    // Delete the file in the source folder.
                    fromFile.remove();

                    // Get the next message.
                    result = fi.findNext(entry);
                }

            }

            // Update any subscribers on the status of this mailbox
            updateMailboxStatus(mailboxIdentity, fromFolderPath);

            updateMailboxStatus(mailboxIdentity, toFolderPath);

            result = OS_SUCCESS ;

#if defined(__pingtel_on_posix__)
            // Unlock the lock file in the mailbox
            OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                          "MailboxManager::moveMessages: unlock the lock file %s",
                          mailboxLock.data());
            lockFile.fileunlock();
            lockFile.close();
#endif
        }
        else
        {
            logContents = "Failed to obtain mailbox path for " + mailboxIdentity ;
        }
    }
    else
    {
        // TODO: If we want to explicitly let the user know that the operation is invalid,
        // return a different status and add VXML script to MoveMessageCGI.cpp
        // to handle this.
        result = OS_SUCCESS ;
    }

    if( result != OS_SUCCESS )
        writeToLog( "MoveMessages", logContents, PRI_ERR );

    return result;
}

OsStatus
MailboxManager::recycleDeletedMessages( const UtlString& mailbox,
                                        const UtlString& messageIds )
{

    UtlString mailboxPath;
    OsStatus result = OS_FAILED;
    UtlString logContents ;

    result = getMailboxPath( mailbox, mailboxPath );
    if( result == OS_SUCCESS )
    {
        mailboxPath += OsPathBase::separator + m_deletedFolder;

#if defined(__pingtel_on_posix__)
        // Set up a lock in the mailbox so that multiple access can be prevented
        UtlString mailboxLock = mailboxPath + OsPathBase::separator + "retrieve.lck";
        OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                      "MailboxManager::recycleDeletedMessages: set up the lock file %s",
                      mailboxLock.data());
        OsFileLinux lockFile(mailboxLock);
        lockFile.open(OsFileBase::CREATE);
        lockFile.filelock(OsFileBase::FSLOCK_WAIT);
#endif

        OsPath path(mailboxPath);
        OsFileIterator fi(path);
        OsPath entry;   // Object for storing the results of file system searches.

        if ( messageIds == "-1" )
        {
           for ( result = fi.findFirst(entry, "[0-9a-zA-Z]*", OsFileIterator::FILES);
                 result == OS_SUCCESS;
                 result = fi.findNext(entry)
                )
           {
              // delete the file.
              UtlString fileLocation =
                 mailboxPath + OsPathBase::separator + entry.getFilename() + entry.getExt();
              OsFile file(fileLocation);
              file.remove();
           }
        }
        else
        {
            UtlTokenizer nameValuePair(messageIds);
            UtlString id;

            // Parse messageIds
            while ( nameValuePair.next(id, MESSAGE_DELIMITER) )
            {
                id = id.strip(UtlString::both);

                // Find all files starting with the given message id.
                UtlString regExp = id + ".+\\.*";

                for ( result = fi.findFirst(entry, regExp, OsFileIterator::FILES);
                      result == OS_SUCCESS;
                      result = fi.findNext(entry)
                     )
                {
                    // delete the file.
                    UtlString fileLocation =
                       mailboxPath + OsPathBase::separator + entry.getFilename() + entry.getExt();
                    OsFile file(fileLocation);
                    file.remove();
                }
            }
        }
        result = OS_SUCCESS;

#if defined(__pingtel_on_posix__)
        // Unlock the lock file in the mailbox
        OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                      "MailboxManager::recycleDeletedMessages: unlock the lock file %s",
                      mailboxLock.data());
        lockFile.fileunlock();
        lockFile.close();
#endif
    }
    else
    {
        logContents = "Unable to find the mailbox path of " + mailbox ;
    }

    if( result != OS_SUCCESS )
    {
       writeToLog( "Empty Deleted Folder", logContents, PRI_ERR );
    }

    return result;

}

OsStatus
MailboxManager::forwardMessages (
    const char* comments,
    const UtlString& commentsDuration,
    const UtlString& commentsTimestamp,
    const int& commentsSize,
    const Url& fromIdentityUrl,
    const UtlString& fromFolder,
    const UtlString& messageIds,
    const UtlString& toMailbox )
{
    OsStatus result = OS_FAILED;
    UtlString forwardedMsgsPlayListExtn = "FW" ;
    UtlString logContents ;
    UtlString fromMailboxPath, toMailboxPath;
    UtlString forwardedMsgSubjectPrefix = "Fwd:" ;

    // 0. Note that since we only have the fromMailboxIdentity passed in
    // we must query the CredentialDB for the full URI
    ResultSet credentials;

    CredentialDB::getInstance()->
        getAllCredentials( fromIdentityUrl, credentials );

    for(;;) // dummy loop so we can use "break" to jump out
    {
        if ( credentials.getSize() <= 0 )
        {
            logContents = "Unable to get credentials for " + 
                fromIdentityUrl.toString();
            break ;
        }

        UtlHashMap record;
        // only interested in the first row here
        credentials.getIndex( 0, record );
        UtlString uriKey ("uri");

        // note that this is the fully qualified URL complete with display name
        Url fromUrl (*((UtlString*)record.findValue(&uriKey)));

        // 1. Get the full path to both the mailboxes.
        UtlString fromIdentity;
        fromUrl.getIdentity( fromIdentity );
        result = getMailboxPath( fromIdentity, fromMailboxPath );
        if ( result != OS_SUCCESS )
        {
            logContents = "Unable to get mailbox path for " + fromIdentity;
            break ;
        }

        fromMailboxPath += OsPathBase::separator + getFolderName( fromFolder );

        result = getMailboxPath( toMailbox, toMailboxPath ) ;
        if ( result != OS_SUCCESS )
        {
            logContents = "Unable to get mailbox path for " + toMailbox;
            break ;
        }

        toMailboxPath += OsPathBase::separator + m_inboxFolder;

        // 2. Copy the original message to the destination folder.
        OsPath path(fromMailboxPath);
        OsFileIterator fi(path);
        OsPath entry;

        UtlTokenizer nameValuePair(messageIds);
        UtlString id;
        int msgCount = 0;

        // Parse messageIds
        while ( nameValuePair.next(id, MESSAGE_DELIMITER) )
        {
            id = id.strip(UtlString::both);
            msgCount++;

            // Find all files starting with the given message id.
            UtlString regExp = id + ".+\\.*";
            UtlString nextMessageID ;

            result = fi.findFirst(entry, regExp, OsFileIterator::FILES);

            if ( result != OS_SUCCESS )
            {
                logContents = "Failed to find files with regExp " + regExp ;
                break ;
            }

            // If files found, save the recorded comment first.
            // Get the id of the message to be stored in the destination folder.
            if (!m_pMsgIDGenerator)
            {
               m_pMsgIDGenerator = MessageIDGenerator::getInstance(m_mailstoreRoot);
            }

            result = m_pMsgIDGenerator->getNextMessageID( nextMessageID );
            if ( result != OS_SUCCESS )
            {
                logContents = "Failed to generate next message id for " 
                                 + toMailbox ;
                break ;
            }

            OsStatus    moreMessages = OS_SUCCESS ;
            UtlString    mergeFileInput2 ;
            UtlString    originalMsgSubject ;

            // Copy the forwarded messages to the new mailbox
            while ( moreMessages == OS_SUCCESS )
            {
                // Construct a file object.
                UtlString fromFileLocation = 
                    fromMailboxPath + OsPathBase::separator + 
                    entry.getFilename() + entry.getExt();
                OsFile fromFile( fromFileLocation );
                UtlString filename = entry.getFilename();
                UtlString fromMessageDepth = 
                    filename(filename.index('-')+1, filename.length());

                if( fromMessageDepth == forwardedMsgsPlayListExtn )
                {
                    mergeFileInput2 = fromFileLocation ;
                }
                else
                {
                    // increment the depth of the forwarded message
                    int iFromMessageDepth = atoi ( fromMessageDepth.data() );
                    iFromMessageDepth++;
                    char temp[5];
                    sprintf ( temp, "%02d", iFromMessageDepth );
                    UtlString toMessageDepth = temp;

                    if ( entry.getExt() != ".sta" )
                    {
                        UtlString toFileLocation =
                            toMailboxPath + OsPathBase::separator +
                            nextMessageID + "-" + toMessageDepth + 
                            entry.getExt();
                        fromFile.copy(toFileLocation);
                    }

                    if( entry.getExt() == ".xml" )
                    {
                        if( iFromMessageDepth == 1 )
                        {
                            // Get the message subject from the -00.xml file
                            UtlHashMap *msgInfoHash = new UtlHashMap();
                            parseMessageDescriptor(fromFileLocation, msgInfoHash );
                            UtlString *rwMsgSubject = 
                                (UtlString *)msgInfoHash->findValue( 
                                                   new UtlString("subject") ) ;
                            originalMsgSubject = rwMsgSubject->data() ;

                            delete rwMsgSubject ;
                            delete msgInfoHash ;
                        }
                    }

                    if( entry.getExt() == ".wav" && iFromMessageDepth == 1 )
                    {
                        mergeFileInput2 = fromFileLocation ;
                    }
                }

                // Get the next message.
                moreMessages = fi.findNext(entry);
            }

            // Call SaveMessage to save the comments.
            result = saveMessage(
                fromUrl,
                toMailbox,
                commentsDuration,
                commentsTimestamp,
                comments,
                commentsSize,
                nextMessageID,
                TRUE,
                FALSE); // Don't email this message!  We do that later.

            if( result != OS_SUCCESS )
            {
                logContents = "Failed to save the message to " + toMailbox ;
                break ;
            }

            UtlString mergedOutputFileLocation = 
                toMailboxPath +
                OsPathBase::separator +
                nextMessageID + "-" + forwardedMsgsPlayListExtn + ".wav";

            UtlString subject = forwardedMsgSubjectPrefix + originalMsgSubject ;
            UtlString messageDescriptorLoc = 
                toMailboxPath +
                OsPathBase::separator +
                nextMessageID + "-00.xml";

            // Update the message descriptor
            updateMessageDescriptor( messageDescriptorLoc, subject );

            if( commentsSize > 0 )
            {
                // merge the wav files.
                UtlString mergeFileInput1 =  toMailboxPath +
                                            OsPathBase::separator +
                                            nextMessageID + "-00.wav";
                UtlString inputFileArray[3] ;
                inputFileArray[0] = mergeFileInput1 ;
                inputFileArray[1] = mergeFileInput2 ;
                inputFileArray[2] = "" ;

                result = mergeWaveFiles(inputFileArray, mergedOutputFileLocation );
                logContents = "Unable to merge file " + mergeFileInput1 + 
                   "with " + mergeFileInput2 ;
            }
            else
            {
                // Use the original file
                OsFile originalMsg (mergeFileInput2) ;
                result = originalMsg.copy(mergedOutputFileLocation);
                logContents = "Unable to copy file " + mergeFileInput2 + 
                    "to " + mergedOutputFileLocation ;
            }

            if (result != OS_SUCCESS)
            {
                break ;
            }

            OsFile file(mergedOutputFileLocation.data());
            result = file.open() ;
            if (result != OS_SUCCESS)
            {
                logContents = "Unable to open merged file " + 
                   mergedOutputFileLocation ;
                break ;
            }

            // Save the file size
            unsigned long fileSize;
            file.getLength(fileSize);

            // Create a buffer for the file contents
            unsigned char *buffer = new unsigned char[fileSize];
            if (buffer != NULL)
            {
                // Read the file contents into the buffer
                unsigned long bytesRead;
                if ( file.read(buffer, fileSize,bytesRead) == OS_SUCCESS )
                {
                    if (bytesRead == fileSize)
                    {
                        UtlString from;
                        fromUrl.getDisplayName(from);

                        UtlString userId;
                        fromUrl.getUserId(userId);

                        if (!from.isNull() && from.length() > 0)
                           from += " - " + userId;
                        else
                           from = userId;

                        UtlString wavFileName = nextMessageID + "-" + 
                           forwardedMsgsPlayListExtn + ".wav";

                        const char* data = (const char*) buffer;
                        sendEmailNotification ( toMailbox,
                            from,
                            commentsTimestamp,
                            commentsDuration,
                            wavFileName,
                            data,
                            fileSize);
                    }
                }
                delete [] buffer;
            }
            file.close();
        }
        break ; // End dummy loop
    }

    if( result != OS_SUCCESS )
        writeToLog( "ForwardMessage", logContents, PRI_ERR );

    return result;
}


OsStatus
MailboxManager::setActiveGreeting(  const UtlString& mailboxIdentity,
                                    const UtlString& greetingType )
{
    UtlString mailboxPath, greetingFileName, defaultGreetingFileName, prefsEntry, logContents;
    OsStatus result = getMailboxPath( mailboxIdentity, mailboxPath );
    if ( result == OS_SUCCESS )
    {
        // Get the name of the greetings file.
        if ( greetingType == STANDARD_GREETING )
        {
            greetingFileName = STANDARD_GREETING_FILE;
            defaultGreetingFileName = DEFAULT_STANDARD_GREETING_FILE ;
            prefsEntry = STANDARD_GREETING ;
        }
        else if( greetingType == OUTOFOFFICE_GREETING )
        {
            greetingFileName = OUTOFOFFICE_GREETING_FILE ;
            defaultGreetingFileName = DEFAULT_OUTOFOFFICE_GREETING_FILE ;
            prefsEntry = OUTOFOFFICE_GREETING ;
        }
        else if ( greetingType == EXTENDED_ABSENCE_GREETING )
        {
            greetingFileName = EXTENDED_ABSENCE_GREETING_FILE ;
            defaultGreetingFileName = DEFAULT_EXTENDED_ABS_GREETING_FILE ;
            prefsEntry = EXTENDED_ABSENCE_GREETING ;
        }
        else if ( greetingType == ACTIVE_GREETING_NOT_SET)
        {
            prefsEntry = ACTIVE_GREETING_NOT_SET ;
        }
        else
        {
            result = OS_FAILED;
            logContents = "Mailbox - " + mailboxIdentity + " : Unknown greeting type - " + greetingType ;
        }

        // Update the mailbox preferences file
        if ( result == OS_SUCCESS )
        {
            if( greetingType != ACTIVE_GREETING_NOT_SET)
            {
                // Check if the user recorded greeting exists
                UtlString greetingLocation = mailboxPath + OsPathBase::separator + greetingFileName ;
                if( !OsFileSystem::exists( greetingLocation ) )
                {
                    // Check if the default greeting exists.
                    greetingLocation = mailboxPath + OsPathBase::separator + defaultGreetingFileName ;
                    if( !OsFileSystem::exists( greetingLocation ) )
                    {
                        logContents = "Mailbox - " + mailboxIdentity + " : Greeting file (user recorded and default) not found for type " + greetingType ;
                        writeToLog( "SetActiveGreeting", logContents, PRI_INFO );

                        // Try creating the default greeting file.
                        generateDefaultGreetings ( mailboxIdentity, greetingType );

                        // Check if the greeting file was generated successfully
                        if( !OsFileSystem::exists( greetingLocation ) )
                        {
                            // Give up. Return an error.
                            logContents = "Mailbox - " + mailboxIdentity + " : Attempt to generate the greeting file failed" + greetingType ;
                            result = OS_FILE_NOT_FOUND ;
                        }
                    }
                }
            }
        }

        if( result == OS_SUCCESS )
        {
            // Check if mailbox preferences XML file exists.
            // Active greeting type is stored in this file.
            UtlString prefFileLocation = mailboxPath + OsPathBase::separator + MAILBOX_PREFS_FILE;
            if ( !OsFileSystem::exists( prefFileLocation ) )
            {
                // Create the preferences file.
                result = createMailboxPrefsFile( prefFileLocation );
            }

            if( result == OS_SUCCESS )
            {
                // Name of the element in the prefs xml file to be updated
                UtlString elementName            = "activegreeting" ;

                // New value for the element
                UtlString elementValue           = prefsEntry ;

                // Name of the element's attribute to be updated
                UtlString elementAttributeName   =   "" ;

                // New value for the element's attribute
                UtlString elementAttributeValue  =   "" ;

                // Id to uniquely identify the element to be updated
                UtlString elementId              =   "" ;

                // Action to be taken by the update prefs file code
                UtlString action                 =   "editactivegreeting" ;

                result = updateMailboxPrefsFile(    prefFileLocation,
                                                    elementName,
                                                    elementValue,
                                                    elementAttributeName,
                                                    elementAttributeValue,
                                                    elementId,
                                                    action
                                                );

                if( result != OS_SUCCESS )
                    logContents = "Failed to update the mailbox prefs file " + prefFileLocation ;

            } else
            {
                logContents = "Unable to access mailbox prefs file " + prefFileLocation ;
            }
        }
    } else
    {
        logContents = "Failed to get the mailbox path for " + mailboxIdentity;
    }

    if( result != OS_SUCCESS )
        writeToLog( "SetActiveGreeting", logContents, PRI_ERR );

    return result;
}

OsStatus
MailboxManager::getActiveGreeting(
    const UtlString& mailboxIdentity,
    UtlString& rGreeting,
    const UtlBoolean& isFromWeb )
{
    // Clear the return variable.
    rGreeting = "";
    UtlString greetingType, logContents ;
    OsStatus result = getActiveGreetingType( mailboxIdentity , greetingType );

    if( result == OS_SUCCESS )
    {
        if( greetingType == ACTIVE_GREETING_NOT_SET )
        {
            result = OS_FAILED ;
            logContents = "Mailbox - " + mailboxIdentity + " : Active greeting not set" ;
            writeToLog("GetActiveGreeting", logContents, PRI_DEBUG);
        }
        else
        {
            result = getGreetingUrl(    mailboxIdentity,
                                        greetingType,
                                        rGreeting,
                                        isFromWeb,
                                        TRUE
                                    );
        }
    }
    else
    {
        writeToLog("GetActiveGreeting", "Failed to get the active greeting type for " + mailboxIdentity, PRI_INFO);
    }
    return result;
}

OsStatus
MailboxManager::getActiveGreetingType (
    const UtlString& mailboxIdentity,
    UtlString& rGreetingType ) const
{
    UtlString  mailboxPath, logContents;

    OsStatus result = getMailboxPath(mailboxIdentity, mailboxPath);
    if( result == OS_SUCCESS )
    {
        // Check if mailbox preferences XML file exists.
        // Active greeting type is stored in this file.
        UtlString fileLocation = mailboxPath + OsPathBase::separator + MAILBOX_PREFS_FILE;
        if ( OsFileSystem::exists( fileLocation ) )
        {
            // parse the preferences file.
            // Contents of the file are stored in the hash dictionary.
            UtlHashMap *prefsHashDictionary = new UtlHashMap();
            UtlString elementToBeRead = "activegreeting" ;
            result = parseMailboxPrefsFile(fileLocation, elementToBeRead, prefsHashDictionary);

            if( result == OS_SUCCESS && prefsHashDictionary != NULL )
            {
                UtlString* value =
                    (UtlString*) prefsHashDictionary->
                        findValue(new UtlString("activegreeting"));
                rGreetingType = value->data();
            }
            else
            {
                result = OS_FAILED;
            }
        }
        else
        {
            logContents = "Mailbox preferences file does not exist -- " +fileLocation ;
            writeToLog( "GetActiveGreetingType", logContents, PRI_INFO );
        }
    }
    return result;
}


OsStatus
MailboxManager::getRecordedName(
    const UtlString& mailboxIdentity,
    UtlString& rRecordedName,
    const UtlBoolean& isFromWeb ) const
{
    OsStatus    result = OS_FAILED;
    UtlString    mailboxPath, logContents;

    // clear the contents of rRecordedName.
    rRecordedName = "";

    result = getMailboxPath(mailboxIdentity, mailboxPath);
    if( result == OS_SUCCESS )
    {
        // Check if name.wav exists -- indicates that the user has recorded his name
        UtlString nameWAVLocation = mailboxPath + OsPathBase::separator + RECORDED_NAME_FILE;
        if ( OsFileSystem::exists( nameWAVLocation ) )
        {
            // Get the full url of the active greeting
            UtlString mailboxUrl;
            result = getMailboxURL(mailboxIdentity, mailboxUrl, isFromWeb);
            if( result == OS_SUCCESS )
            {
                rRecordedName = mailboxUrl + "/" + RECORDED_NAME_FILE ;
            }
        } else
        {
            writeToLog("GetRecordedName", "User has not recorded name. Mailbox " + mailboxIdentity, PRI_DEBUG);
            result = OS_FAILED ;
        }
    }

    return result;
}

OsStatus
MailboxManager::getGreetingUrl(
    const UtlString& mailboxIdentity,
    const UtlString& greetingType,
    UtlString& rGreetingUrl,
    const UtlBoolean& isFromWeb,
    const UtlBoolean& returnDefaultFileUrl)
{
    OsStatus result = OS_SUCCESS;
    UtlString mailboxPath, mailboxUrl, logContents;

    if ( (getMailboxPath( mailboxIdentity, mailboxPath ) == OS_SUCCESS) &&
         (getMailboxURL( mailboxIdentity, mailboxUrl, isFromWeb ) == OS_SUCCESS) )
    {
        UtlString greetingFileName, defaultGreetingFileName;

        if ( greetingType == STANDARD_GREETING )
        {
            greetingFileName = STANDARD_GREETING_FILE;
            defaultGreetingFileName = DEFAULT_STANDARD_GREETING_FILE ;
        }
        else if( greetingType == OUTOFOFFICE_GREETING )
        {
            greetingFileName = OUTOFOFFICE_GREETING_FILE ;
            defaultGreetingFileName = DEFAULT_OUTOFOFFICE_GREETING_FILE ;
        }
        else if ( greetingType == EXTENDED_ABSENCE_GREETING )
        {
            greetingFileName = EXTENDED_ABSENCE_GREETING_FILE ;
            defaultGreetingFileName = DEFAULT_EXTENDED_ABS_GREETING_FILE ;
        }
        else if ( greetingType == DEFAULT_STANDARD_GREETING || greetingType == ACTIVE_GREETING_NOT_SET)
        {
            // Set the greeting filename to be same as the default greeting filename
            defaultGreetingFileName = DEFAULT_STANDARD_GREETING_FILE ;
            greetingFileName = defaultGreetingFileName ;
        }
        else if( greetingType == DEFAULT_OUTOFOFFICE_GREETING )
        {
            defaultGreetingFileName = DEFAULT_OUTOFOFFICE_GREETING_FILE ;
            greetingFileName = defaultGreetingFileName ;
        }
        else if ( greetingType == DEFAULT_EXTENDED_ABS_GREETING )
        {
            defaultGreetingFileName = DEFAULT_EXTENDED_ABS_GREETING_FILE ;
            greetingFileName = defaultGreetingFileName ;
        }
        else
        {
            result = OS_FAILED;
            logContents = "Failed to get greeting URL. Unknown greeting type - " + greetingType ;
        }

        // Update the mailbox preferences file
        if ( result == OS_SUCCESS )
        {
            // Check if the user recorded greeting exists
            UtlString greetingLocation = mailboxPath + OsPathBase::separator + greetingFileName ;
            if( OsFileSystem::exists( greetingLocation ) )
            {
                rGreetingUrl = mailboxUrl + URL_SEPARATOR + greetingFileName ;
            }
            else if( returnDefaultFileUrl )
            {
                writeToLog("GetGreetingURL", "User recorded greeting not found - " + greetingLocation, PRI_DEBUG);

                // Check if the default greeting exists.
                greetingLocation = mailboxPath + OsPathBase::separator + defaultGreetingFileName ;
                if( OsFileSystem::exists( greetingLocation ) )
                {
                    rGreetingUrl = mailboxUrl + URL_SEPARATOR + defaultGreetingFileName ;
                }
                else
                {
                    logContents = "Mailbox - " + mailboxIdentity + " : Greeting file (user recorded and default) not found for type " + greetingType ;
                    writeToLog( "GetGreetingUrl", logContents, PRI_INFO) ;

                    // Try creating the default greeting
                    generateDefaultGreetings( mailboxIdentity, greetingType );

                    // Check if the greeting file was generated successfully
                    if( !OsFileSystem::exists( greetingLocation ) )
                    {
                        // Neither user recorded nor default system greeting was found
                        // for the specified greeting type.
                        logContents = "Mailbox - " + mailboxIdentity + " : Failed to create the default greeting file for type " + greetingType ;
                        writeToLog( "GetGreetingUrl", logContents, PRI_ERR) ;
                        result = OS_FAILED ;
                    }
                    else
                    {
                        rGreetingUrl = mailboxUrl + URL_SEPARATOR + defaultGreetingFileName ;
                        result = OS_SUCCESS ;
                    }

                }
            }
            else
            {
                // Only the URL of user recorded greeting was requested and file was not found
                logContents = "Mailbox - " + mailboxIdentity + " : Greeting file (user recorded) not found for type " + greetingType ;
                writeToLog( "GetGreetingUrl", logContents, PRI_INFO) ;
                result = OS_FAILED ;
            }
        }
    }

    return result;
}

OsStatus
MailboxManager::saveGreetingOrName (
    const UtlString& mailboxIdentity,
    const UtlString& greetingType,
    const char* data,
    const int&  datasize )
{
    OsStatus result = OS_SUCCESS;
    UtlString mailboxPath, logContents;

    result = getMailboxPath( mailboxIdentity, mailboxPath) ;
    if ( result == OS_SUCCESS )
    {
        UtlString filename = mailboxPath + OsPathBase::separator ;

        if ( greetingType == STANDARD_GREETING )
            filename += STANDARD_GREETING_FILE ;
        else if ( greetingType == OUTOFOFFICE_GREETING )
            filename += OUTOFOFFICE_GREETING_FILE ;
        else if ( greetingType == EXTENDED_ABSENCE_GREETING )
            filename += EXTENDED_ABSENCE_GREETING_FILE ;
        else if ( greetingType == RECORDED_NAME )
            filename += RECORDED_NAME_FILE ;
        else
            result = OS_FAILED;

        if ( result == OS_SUCCESS )
        {
            // Check if the wav file already exists
            if ( OsFileSystem::exists(filename) )
            {
                // Remove the existing wav file.
                OsFile osfile (filename) ;
                osfile.remove();
            }

            // Create a new file
            int file = open(filename, O_BINARY | O_CREAT | O_RDWR, 0644);
            unsigned long bytes_written = 0;
            if ( file != -1 )
            {
                // write the data to the file.
                bytes_written = write( file, data, datasize );
                if( bytes_written <= 0 )
                {
                    logContents = "Unable to write recorded greeting to " + filename ;
                    result = OS_FAILED ;
                }

                // close file
                close(file);

                // If the recorded file was user's name,
                // then update the default greetings.
                if( greetingType == RECORDED_NAME )
                {
                    generateDefaultGreetings ( mailboxIdentity, DEFAULT_STANDARD_GREETING );
                    generateDefaultGreetings ( mailboxIdentity, DEFAULT_OUTOFOFFICE_GREETING );
                    generateDefaultGreetings ( mailboxIdentity, DEFAULT_EXTENDED_ABS_GREETING );
                }
                else
                {
                    // Set this greeting to be the active greeting (as per defect #2701)
                    // This automatic setting may be changed in the future.
                    result = setActiveGreeting( mailboxIdentity, greetingType );
                    if( result != OS_SUCCESS )
                        logContents = "Failed to set " + greetingType + " as the active greeting for " + mailboxIdentity ;
                }

            } else
            {
                logContents = "Unable to create " + filename ;
                result = OS_FAILED;
            }
        } else
        {
            logContents = "Mailbox - " + mailboxIdentity + " -- Unknown greeting type " + greetingType ;
        }
    } else
    {
        logContents = "Failed to get the mailbox path for " + mailboxIdentity;
    }

    if( result != OS_SUCCESS )
        writeToLog( "SaveGreetingOrName", logContents, PRI_ERR );

    return result;
}

OsStatus
MailboxManager::deleteGreeting(
    const UtlString& mailboxIdentity,
    const UtlString& greetingType,
    const UtlBoolean& isActiveGreeting )
{
    UtlString    mailboxPath, logContents;
    OsStatus    result = getMailboxPath(mailboxIdentity, mailboxPath);

    if( result == OS_SUCCESS )
    {
        UtlString filename = mailboxPath + OsPathBase::separator ;
        UtlString defaultGreetingFilename = filename ;

        if ( greetingType == STANDARD_GREETING )
        {
            filename += STANDARD_GREETING_FILE ;
            defaultGreetingFilename += DEFAULT_STANDARD_GREETING_FILE ;
        }
        else if ( greetingType == OUTOFOFFICE_GREETING )
        {
            filename += OUTOFOFFICE_GREETING_FILE ;
            defaultGreetingFilename += DEFAULT_OUTOFOFFICE_GREETING_FILE ;
        }
        else if ( greetingType == EXTENDED_ABSENCE_GREETING )
        {
            filename += EXTENDED_ABSENCE_GREETING_FILE ;
            defaultGreetingFilename += DEFAULT_EXTENDED_ABS_GREETING_FILE ;
        }
        else if ( greetingType == RECORDED_NAME )
            filename += RECORDED_NAME_FILE ;
        else
            result = OS_FAILED;

        if( result == OS_SUCCESS )
        {
            if( isActiveGreeting )
            {
                // check if the default greeting is found.
                // If not found, reset the active greeting.
                if( !OsFileSystem::exists(defaultGreetingFilename) )
                {

                    // Update the mailbox preferences file.

                    // Check if mailbox preferences XML file exists.
                    // Active greeting url is stored in this file.
                    UtlString prefFileLocation = mailboxPath + OsPathBase::separator + MAILBOX_PREFS_FILE;
                    if ( !OsFileSystem::exists( prefFileLocation ) )
                    {
                        // Create the preferences file.
                        result = createMailboxPrefsFile( prefFileLocation );
                    }
                    else
                    {
                        // Name of the element in the prefs xml file to be updated
                        UtlString elementName            = "activegreeting" ;

                        // New value for the element
                        UtlString elementValue           = ACTIVE_GREETING_NOT_SET ;

                        // Name of the element's attribute to be updated
                        UtlString elementAttributeName   =   "" ;

                        // New value for the element's attribute
                        UtlString elementAttributeValue  =   "" ;

                        // Id to uniquely identify the element to be updated
                        UtlString elementId              =   "" ;

                        // Action to be taken by the update prefs file code
                        UtlString action                 =   "editactivegreeting" ;

                        result = updateMailboxPrefsFile(
                            prefFileLocation,
                            elementName,
                            elementValue,
                            elementAttributeName,
                            elementAttributeValue,
                            elementId,
                            action );

                        if( result != OS_FAILED )
                            logContents = "Failed to update the mailbox preferences file " + prefFileLocation ;
                    }
                }
            }

            if(  result == OS_SUCCESS )
            {
                // Check if the wav file exists
                if ( OsFileSystem::exists(filename) )
                {
                    // Remove the wav file.
                    OsFile osfile (filename) ;
                    osfile.remove();

                    // If the recorded file was user's name,
                    // then update the default greetings.
                    if( greetingType == RECORDED_NAME )
                    {
                        generateDefaultGreetings ( mailboxIdentity, DEFAULT_STANDARD_GREETING );
                        generateDefaultGreetings ( mailboxIdentity, DEFAULT_OUTOFOFFICE_GREETING );
                        generateDefaultGreetings ( mailboxIdentity, DEFAULT_EXTENDED_ABS_GREETING );
                    }
                }
            }
        }
        else
        {
            logContents = "Unknown greeting type -- " + greetingType ;
            result = OS_FAILED ;
        }
    }
    else
    {
        logContents = "Failed to get the mailbox path for " + mailboxIdentity;
    }

    if( result != OS_SUCCESS )
        writeToLog( "DeleteGreeting", logContents, PRI_ERR );

    return result;
}

OsStatus
MailboxManager::getMediaserverURL (
    UtlString& rMediaserverUrl ) const
{
    OsStatus result = OS_FAILED;

    if ( m_mediaserverUrl != "" )
    {
        rMediaserverUrl = m_mediaserverUrl;
        result = OS_SUCCESS;
    }
    else
    {
        writeToLog("GetMediaserverUrl", "Failed to get the mediaserver-url defined in voicemail.xml.in", PRI_ERR);
    }

    return result;
}

OsStatus
MailboxManager::getMediaserverSecureURL (
    UtlString& rMediaserverSecureUrl ) const
{
    OsStatus result = OS_FAILED;

    if ( m_mediaserverSecureUrl != "" )
    {
        rMediaserverSecureUrl = m_mediaserverSecureUrl;
        result = OS_SUCCESS;
    }
    else
    {
        writeToLog("GetMediaserverSecureUrl", "Failed to get the mediaserver-url-secure defined in voicemail.xml.in", PRI_ERR);
    }
    return result;
}

OsStatus
MailboxManager::getMediaserverURLForWeb (
    UtlString& rMediaserverUrl ) const
{
    OsStatus result = OS_SUCCESS ;

    if ( m_mediaserverSecureUrl != "" && m_mediaserverSecureUrl.index("localhost", UtlString::ignoreCase) == UTL_NOT_FOUND)
    {
        rMediaserverUrl = m_mediaserverSecureUrl;
    }
    else if ( m_mediaserverUrl != "" && m_mediaserverUrl.index("localhost", UtlString::ignoreCase) == UTL_NOT_FOUND)
    {
        rMediaserverUrl = m_mediaserverUrl;
    }
    else
    {
        // Get the Protocol from the HTTPS environment variable
        // note SERVER_PROTOCOL does not work for this
        UtlString protocol = "http";
        cgicc::CgiEnvironment env = gCgi->getEnvironment();
        if ( env.usingHTTPS())
            protocol = "https";

        if ( m_mediaserverSecureUrl.first( protocol.data() ) == UTL_NOT_FOUND )
        {
           OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_WARNING,
                         "getMediaserverURLForWeb: m_mediaserverSecureUrl = '%s', m_mediaserverUrl = '%s', protocol = '%s'",
                         (m_mediaserverSecureUrl == NULL ? "(null)" :
                          m_mediaserverSecureUrl.data()),
                         (m_mediaserverUrl == NULL ? "(null)" :
                          m_mediaserverUrl.data()),
                         protocol.data());
        }

        rMediaserverUrl = protocol + "://" + UtlString( getenv("SERVER_NAME") ) + ":" +
            UtlString( getenv("SERVER_PORT") );
    }
    return result;
}

OsStatus
MailboxManager::getNonLocalhostMediaserverURL (
    UtlString& rMediaserverUrl ) const
{
    OsStatus result = OS_SUCCESS ;

    if ( !m_fullMediaserverSecureUrl.isNull() && m_fullMediaserverSecureUrl != "" )
    {
        rMediaserverUrl = m_fullMediaserverSecureUrl ;
    }
    else if ( m_mediaserverSecureUrl != "" && m_mediaserverSecureUrl.index("localhost", UtlString::ignoreCase) == UTL_NOT_FOUND)
    {
        rMediaserverUrl = m_mediaserverSecureUrl;
    }
    else if ( m_mediaserverUrl != "" && m_mediaserverUrl.index("localhost", UtlString::ignoreCase) == UTL_NOT_FOUND)
    {
        rMediaserverUrl = m_mediaserverUrl;
    }
    else
    {
        // Get the Protocol from the HTTPS environment variable
        // note SERVER_PROTOCOL does not work for this
        UtlString protocol = "http";
        if (gCgi->getEnvironment().usingHTTPS())
            protocol = "https";

        // if the protocol is missing in voicemail.xml use the request one and store it
        if ( m_mediaserverUrl.first( protocol.data() ) == UTL_NOT_FOUND )
        {
            writeToLog( "getNonLocalhostMediaserverURL", "Protocol Mismatch", PRI_ERR );
            result = OS_FAILED ;
        }
        Url mediaserverUrl ( m_mediaserverSecureUrl != "" ? m_mediaserverSecureUrl : m_mediaserverUrl );
        int port = mediaserverUrl.getHostPort();

        char temp[10];
        sprintf(temp, "%d", port );
        UtlString portStr = temp ;

        // NOTE: Using the domain does not always work. You need to use the full hostname instead.
        rMediaserverUrl = protocol + "://" + m_defaultDomain + ":" + portStr;
    }
    return result;
}

OsStatus
MailboxManager::getIvrPromptURL (
    UtlString& rIvrPromptUrl ) const
{
    OsStatus result = OS_FAILED;

    if ( m_ivrPromptUrl != "" )
    {
        rIvrPromptUrl = m_ivrPromptUrl;
        result = OS_SUCCESS;
    }
    else
    {
        writeToLog("GetIvrPromptURL", "Failed to get the ivr-prompt-url defined in voicemail.xml.in", PRI_ERR);
    }

    return result;
}

OsStatus
MailboxManager::getMailboxPath(
    const UtlString& mailbox,
    UtlString& mailboxPath ) const
{
    OsStatus result = OS_FAILED;
    // "mailbox" is a fully qualified SIP URL.
    // For example, mailbox = sip:hari@pingtel.com.
    // Retrieve the userid from it.
    Url mailboxUrl ( mailbox );
    UtlString mailboxId ;
    mailboxUrl.getUserId(mailboxId);

    // Ensure the mailbox id parameter is valid
    if ( !mailboxId.isNull() )
    {
        if( !m_mailstoreRoot.isNull() && m_mailstoreRoot != "")
        {
            // Construct the physical path to the mailbox directory.
            mailboxPath = m_mailstoreRoot + OsPathBase::separator +
                          MAILBOX_DIR + OsPathBase::separator +
                          mailboxId ;
            result = OS_SUCCESS;
        }
        else
        {
            writeToLog("getMailboxPath", "Failed to get mailbox root '<mailstore-root>' from voicemail.xml.in", PRI_ERR);
        }

    }
    else
    {
        writeToLog("getMailboxPath", "Failed to get the mailbox path for " + mailbox, PRI_INFO);
    }
    return result;
}

OsStatus
MailboxManager::getMailboxURL (
    const UtlString& mailbox,
    UtlString& mailboxUrl,
    const UtlBoolean& isFromWeb ) const
{
   // the mailbox parameter is is a fully qualified SIP URL.
   // For example, mailbox = sip:hari@pingtel.com.
   // Retrieve the userid from it.
   Url url ( mailbox );
   UtlString mailboxId;
   url.getUserId( mailboxId );

   // Ensure the mailbox id parameter is valid
   if ( mailboxId.isNull() )
      return OS_FAILED;

   // Construct the base URL for the mailbox.
   // If mailbox = hari@pingtel.com,
   // mailboxUrl will be like http://mediaserver:8090/mailboxes/mailstore/hari
   if( isFromWeb )
   {
      getMediaserverURLForWeb( mailboxUrl );
      mailboxUrl += UtlString( URL_SEPARATOR ) +
         MEDIASERVER_ROOT_ALIAS + URL_SEPARATOR +
         MAILBOX_DIR + URL_SEPARATOR +
         mailboxId;
   }
   else
   {
      // If <mediaserver-url-secure> is set to localhost, then mailstore and
      // Media Server are running on the same machine and we should use
      // local file access to retrieve voicemail .wav files.  Otherwise use
      // secure HTTP access path as specified by <mediaserver-url-secure>.
      if (m_mediaserverSecureUrl.index("localhost", UtlString::ignoreCase) == UTL_NOT_FOUND)
      {
         mailboxUrl = m_mediaserverSecureUrl;
         mailboxUrl += UtlString( URL_SEPARATOR ) +
            MEDIASERVER_ROOT_ALIAS + URL_SEPARATOR +
            MAILBOX_DIR + URL_SEPARATOR +
            mailboxId;
      }
      else
      {
         mailboxUrl = UtlString( "file://" );
         mailboxUrl += m_mailstoreRoot;
         mailboxUrl += UtlString( URL_SEPARATOR ) +
            MAILBOX_DIR + URL_SEPARATOR +
            mailboxId;
      }
   }

   return OS_SUCCESS;
}

void
MailboxManager::writeToLog(
    const UtlString& methodName,
    const UtlString& contents,
    const OsSysLogPriority& priority) const
{
    UtlString logContents = methodName + " : " + contents ;
    OsSysLog::add(FAC_MEDIASERVER_CGI, priority, logContents.data());
    OsSysLog::flush();
    return;
}

/* Get the folder (directory) name corresponding to a category of messages.
 * "heard", "unheard", and "inbox" all are folder "inbox".
 */
UtlString
MailboxManager::getFolderName(  const UtlString& category ) const
{
    UtlString rFolderName ;
    if( category == "unheard" || category == "heard" || category == "inbox")
        rFolderName = m_inboxFolder ;
    else if( category == "saved" )
        rFolderName = m_savedFolder ;
    else if( category == "deleted" )
        rFolderName = m_deletedFolder ;
    else
        rFolderName = category ;
    return rFolderName;
}

OsStatus
MailboxManager::getMailboxFolders(
    const UtlString& mailboxIdentity,
    UtlSortedList& folderList ) const
{
    // retrieve all the folder names.
    UtlString mailboxPath;
    OsStatus result = getMailboxPath( mailboxIdentity, mailboxPath );
    if( result == OS_SUCCESS )
    {
        OsPath path(mailboxPath);
        OsFileIterator fi(path);
        OsPath entry;

        OsStatus directoriesFound = fi.findFirst(entry, "^[0-9a-zA-Z]+$", OsFileIterator::DIRECTORIES);
        while ( directoriesFound == OS_SUCCESS )
        {
            UtlString foldername = entry.data();
            folderList.insert( new UtlString (foldername) );
            directoriesFound = fi.findNext(entry) ;
        }
    }
    return result ;
}

OsStatus
MailboxManager::editMessage (
    const UtlString& mailboxIdentity,
    const UtlString& folderName,
    const UtlString& messageId,
    const UtlString& subject ) const
{
    UtlString mailboxPath ;
    OsStatus result = getMailboxPath( mailboxIdentity, mailboxPath );
    if( result == OS_SUCCESS )
    {
        UtlString fileLocation =
            mailboxPath +
            OsPath::separator + folderName +
            OsPath::separator + messageId + ".xml" ;

        result = updateMessageDescriptor( fileLocation, subject );
    }
    return result ;
}

/// If there are any subscribers to this mailbox, send them an update
void
MailboxManager::updateMailboxStatus (
   const UtlString& mailboxIdentity, ///< SIP identity of mailbox owner
   const UtlString& mailboxPath      ///< path to the mailbox directory
                                     ) const
{
   // Subscription is supported only for the inbox, so see if this mailox is the inbox
   UtlString inboxPath( OsPathBase::separator + m_inboxFolder );
   UtlString endPath
      = mailboxPath(mailboxPath.length() - inboxPath.length(),inboxPath.length());

   if ( endPath == inboxPath )
   {
      postMWIStatus( mailboxIdentity, mailboxPath );
   }
   else
   {
      OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                    "MailboxManager::updateMailboxStatus not an inbox: '%s'; no update",
                    mailboxPath.data()
                    );
   }
}



OsStatus
MailboxManager::postMWIStatus (const UtlString& mailboxIdentity,
                               const UtlString& inboxPath
                               ) const
{
    OsStatus result = OS_SUCCESS;

    // only call the MWI if the URL is specified
    if ( !m_mwiUrl.isNull() )
    {
        UtlString notifyBodyText;
        result = getMWINotifyText( mailboxIdentity, &inboxPath, notifyBodyText );
        if ( result == OS_SUCCESS )
        {
            // first, build a URL to send the message ids
            // to the status server (where it will be for
            // warded via the UserAgent to the phone
            Url statusServerUrl ( m_mwiUrl );
            statusServerUrl.setHeaderParameter (
                "eventtype", "message-summary" );
            statusServerUrl.setHeaderParameter(
                "identity",  mailboxIdentity );

            // get the url path up to the ? separator
            // (indicated via the TRUE parameter)
            UtlString uriString;
            statusServerUrl.getPath( uriString, TRUE );
            int statusServerPort = statusServerUrl.getHostPort();

            // get the hostname
            UtlString hostAddress;
            statusServerUrl.getHostAddress( hostAddress );

            char portString [32];
            sprintf ( portString, "%d", statusServerPort );
            HttpMessage *pRequest = new HttpMessage();
            pRequest->setFirstHeaderLine( "GET", uriString, HTTP_PROTOCOL_VERSION );
            pRequest->addHeaderField( "Accept", "*/*");
            pRequest->setUserAgentField("Voicemail-Server");

            HttpBody* body = new HttpBody(
                notifyBodyText.data(),
                notifyBodyText.length(),
                CONTENT_TYPE_SIMPLE_MESSAGE_SUMMARY );

            pRequest->setBody( body );
            pRequest->setContentType( CONTENT_TYPE_SIMPLE_MESSAGE_SUMMARY );
            pRequest->setContentLength( notifyBodyText.length() );

            // Create an empty response object and sent the built up request
            // via it to the embedded web server on statusserver:8100
            HttpMessage *pResponse = new HttpMessage();

            UtlString updateMsg;
            updateMsg.append("update mailbox '");
            updateMsg.append(mailboxIdentity);
            updateMsg.append("'\n");
            updateMsg.append(notifyBodyText.data());
            writeToLog("postMWIevent ", updateMsg.data(), PRI_INFO);
            pResponse->get( statusServerUrl, *pRequest,
                           2000 /* two seconds */, false /* not persistent */
                           );
            writeToLog("postMWIevent", "exiting ", PRI_DEBUG);

            UtlString status;
            pResponse->getResponseStatusText(&status);
            OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                          "postMWIEvent status = %s", status.data());

            if (status.compareTo("OK") == 0)
            {
              OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                            "postMWIEvent getResponseStatusText successful with status = %s",
                            status.data());
              result = OS_SUCCESS;
            }
            else
            {
              OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                            "postMWIEvent getResponseStatusText failed with status = %s",
                            status.data());
              result = OS_FAILED;
            }

            delete pResponse;
            delete pRequest;
        }
        else
        {
           OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_ERR,
                         "MailboxManager::postMWIEvent failed for '%s' folder '%s'",
                         mailboxIdentity.data(), inboxPath.data()
                         );

        }
    }
    return result;
}

OsStatus
MailboxManager::getMWINotifyText (
    const UtlString& mailboxIdentity,
    const UtlString* iMailboxPath,
    UtlString& rNotifyText
    ) const
{
    OsStatus result = OS_FAILED;

    // clear any existing text first
    rNotifyText.remove(0);

    // only call the MWI if the URL is specified
    int unheard = 0;
    int total = 0;
    UtlString inboxPathName;
    
    if ( iMailboxPath ) // caller provided the mailbox path
    {
       UtlString mailboxPath;
       inboxPathName = *iMailboxPath;
       result = OS_SUCCESS;
    }
    else
    {
       UtlString mailboxPath;
       
       result = getMailboxPath( mailboxIdentity, mailboxPath );
       if ( OS_SUCCESS == result )
       {
          inboxPathName = mailboxPath + OsPathBase::separator + m_inboxFolder;
       }
       else
       {
          OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_ERR,
                        "MailboxManager::getMWINotifyText no inbox path found for idenitiy '%s'",
                        mailboxIdentity.data()
                        );
       }
    }

    if (!inboxPathName.isNull())
    {
       // get the unheard and total counts from the inbox
       getMailboxCounts (inboxPathName, unheard, total );
    
       const char* messageSummaryFormat =
          "Messages-Waiting: %s\r\n"
          "Voice-Message: %d/%d (0/0)\r\n"
          "\r\n";
       char messageSummary[200];
          sprintf(messageSummary, messageSummaryFormat, 
                  unheard ? "yes" : "no", // messages-waiting value
                  unheard, total
                  );
          rNotifyText = messageSummary;
    }
    else
    {
       // simplest to just give it a dummy answer.
       rNotifyText = 
          "Messages-Waiting: no\r\n"
          "Voice-Message: 0/0 (0/0)\r\n"
          "\r\n";

       // could not find the inbox
       result = OS_FAILED;
    }
    
    return result;
}

OsStatus
MailboxManager::generateDefaultGreetings (
    const UtlString& mailboxIdentity,
    const UtlString& greetingType)
{
    UtlString mailboxPath, mailboxUrl ;
    OsStatus result = OS_FAILED ;

    if( (getMailboxPath( mailboxIdentity, mailboxPath ) == OS_SUCCESS) &&
        (getMailboxURL( mailboxIdentity, mailboxUrl ) == OS_SUCCESS) )
    {
        // Get the full path of the file to be created.
        UtlString greetingLocation, greetingSuffix ;

        writeToLog("GenerateDefaultGreetings", "Generating greetings for type " + greetingType + " and mailbox " + mailboxIdentity, PRI_DEBUG) ;

        if( greetingType == DEFAULT_STANDARD_GREETING || greetingType == STANDARD_GREETING || greetingType == ACTIVE_GREETING_NOT_SET)
        {
            greetingLocation = mailboxPath + OsPath::separator + DEFAULT_STANDARD_GREETING_FILE ;
            greetingSuffix   = m_mediaserverUrl + "/" + PROMPT_ALIAS + "/" + "is_not_available.wav" ;
        }
        else if( greetingType == DEFAULT_OUTOFOFFICE_GREETING || greetingType == OUTOFOFFICE_GREETING)
        {
            greetingLocation = mailboxPath + OsPath::separator + DEFAULT_OUTOFOFFICE_GREETING_FILE ;
            greetingSuffix   = m_mediaserverUrl + "/" + PROMPT_ALIAS + "/" + "is_out_of_office.wav" ;
        }
        else if( greetingType == DEFAULT_EXTENDED_ABS_GREETING || greetingType == EXTENDED_ABSENCE_GREETING)
        {
            greetingLocation = mailboxPath + OsPath::separator + DEFAULT_EXTENDED_ABS_GREETING_FILE ;
            greetingSuffix   = m_mediaserverUrl + "/" + PROMPT_ALIAS + "/" + "is_on_extended_leave.wav" ;
        }

        // Delete the default greeting file if it already exists.
        if ( OsFileSystem::exists(greetingLocation) )
        {
            // Remove the wav file.
            OsFile osfile (greetingLocation) ;
            osfile.remove();
        }

        // Check if the user has recorded their name.
        UtlString recordedNameFile =
            mailboxPath + OsPath::separator + RECORDED_NAME_FILE ;

        if( OsFileSystem::exists( recordedNameFile ) )
        {
            writeToLog("GenerateDefaultGreetings", "User has recorded name.", PRI_DEBUG);

            // Modified greetingSuffix to be file based
            if( greetingType == DEFAULT_STANDARD_GREETING || greetingType == STANDARD_GREETING || greetingType == ACTIVE_GREETING_NOT_SET)
            {
                greetingSuffix   = m_mediaserverRoot + "/" + PROMPT_ALIAS + "/" + "is_not_available.wav" ;
            }
            else if( greetingType == DEFAULT_OUTOFOFFICE_GREETING || greetingType == OUTOFOFFICE_GREETING)
            {
                greetingSuffix   = m_mediaserverRoot + "/" + PROMPT_ALIAS + "/" + "is_out_of_office.wav" ;
            }
            else if( greetingType == DEFAULT_EXTENDED_ABS_GREETING || greetingType == EXTENDED_ABSENCE_GREETING)
            {
                greetingSuffix   = m_mediaserverRoot + "/" + PROMPT_ALIAS + "/" + "is_on_extended_leave.wav" ;
            }

            UtlString infiles[3] ;
            infiles[0] = recordedNameFile ;
            infiles[1] = greetingSuffix ;
            infiles[2] = "" ;


            result = mergeWaveFiles(infiles, greetingLocation);
        } 
        else
        {
            UtlString identity, extension ;

            // Get the extension
            validateMailbox (
                mailboxIdentity,
                TRUE,           // RESOLVE EXTENSION
                FALSE,          // CHECK PERMISSIONS
                identity,
                extension );

            // For standard greetings, merge:
            // extension.wav, <extension digits>.wav, is_not_available.wav
            UtlString extensionUrl =
                m_mediaserverUrl + "/" + PROMPT_ALIAS + "/" + "default_greeting_prefix.wav" ;

            writeToLog("GenerateDefaultGreetings", "Extension: " + extension, PRI_DEBUG );

            // Get the number of digits in the extension
            int extensionLength = extension.length();

            // @JC Fixed a problem where the default greeting included non numeric
            // extension information (@ . etc) this could not be created
            if ( extension.index ("@") != UTL_NOT_FOUND )
                extensionLength = extension.index ("@");

            // If the extension is longer than 5 characters, or contains
            // anything other than digits, use a more generic greeting,
            bool generic = true ;
            if (extensionLength <= 5)
            {
                // Check if all of those characters are digits
                RegEx pattern("^[0-9]+$");
                if (pattern.Search(extension.data(), extensionLength))
                {
                   // Yes, they are.  Can use the more specific prompt
                   generic = false ;
                }
            }
            
            if (generic)
            {
               // "The owner of this extension" + {suffix}"
               UtlString infiles[3] ;
               infiles[0] =
                  m_mediaserverUrl + "/" + PROMPT_ALIAS + "/" + "owner.wav" ;
               infiles[1] = greetingSuffix ;
               infiles[2] = "" ;

            writeToLog("GenerateDefaultGreetings", "Extension: " + extension + "Using generic greeting", PRI_DEBUG );
               result = mergeWaveUrls(infiles, greetingLocation);
            }
            else
            {

               // WAV files to be merged is:
               // extension.wav + <each digit of the extension>.wav + "is_not_available.wav"

               // Hence array size is:
               // prefix wav file + suffix wav file + length of the extension +
               // 1 blank entry to indicate the end of WAV files
               int arrayLength = 3 + extensionLength ;

               // Create the array for storing the WAV files to be merged
               UtlString* infiles = new UtlString[ arrayLength ] ;

               if( infiles )
               {
                   // First element of the array is the prefix greeting URL
                   infiles[0] = extensionUrl ;
   
                   // Get individual digits of the extension
                   int arrayIndex = 1 ;
                   while( extensionLength > 0 )
                   {
                       infiles[ arrayIndex ] =
                           m_mediaserverUrl + "/" + PROMPT_ALIAS + "/" + extension( 0, 1 ) + ".wav";
                       extensionLength-- ;
                       extension = extension( 1, extensionLength) ;
                       arrayIndex ++ ;
                   }

                   infiles[arrayIndex] = greetingSuffix ;
                   infiles[arrayIndex + 1] = "" ;

                    writeToLog("GenerateDefaultGreetings", "Extension: " + extension + "Using digit greeting", PRI_DEBUG );
                   result = mergeWaveUrls(infiles, greetingLocation);
                   delete [] infiles ;
               }
            }
        }
    }

    if (result != OS_SUCCESS)
    {
       OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_ERR,
          "MailboxManager::generateDefaultGreetings failed to generate default %s prompt for %s",
          greetingType.data(), mailboxIdentity.data());
                  
    }
    return result ;
}

OsStatus
MailboxManager::getNotificationContactList (
    const UtlString&   mailboxIdentity,
    UtlHashMap* contactsHashDictionary ) const
{

    // retrieve all the folder names.
    UtlString mailboxPath;
    OsStatus result = getMailboxPath( mailboxIdentity, mailboxPath );
    if( result == OS_SUCCESS )
    {
        UtlString prefFileLocation = mailboxPath + OsPathBase::separator + MAILBOX_PREFS_FILE;

        // Flag indicating the element to be read from the preference file
        UtlString elementToBeRead = "notification" ;

        result = parseMailboxPrefsFile (
            prefFileLocation,
            elementToBeRead,
            contactsHashDictionary );

    }
    return result ;
}

OsStatus
MailboxManager::addEditDeleteNotification(
    const UtlString& mailboxIdentity,
    const UtlString& action,
    const UtlString& contactAddress,
    const UtlString& newContactAddress,
    const UtlString& newContactType,
    const UtlString& sendAttachments) const
{
    UtlString mailboxPath ;
    OsStatus result = getMailboxPath( mailboxIdentity, mailboxPath) ;
    if( result == OS_SUCCESS )
    {
        UtlString prefFileLocation = mailboxPath + OsPathBase::separator + MAILBOX_PREFS_FILE;
        if ( !OsFileSystem::exists( prefFileLocation ) )
        {   // Create the preferences file.
            result = createMailboxPrefsFile( prefFileLocation );
        }

        if( result == OS_SUCCESS )
        {
            // Name of the element in the prefs xml file to be updated
            UtlString elementName = "email" ;

            // Name of the element's attribute to be updated
            UtlString elementAttributeName = "attachments" ;

            result = updateMailboxPrefsFile(
                prefFileLocation,
                elementName,
                newContactAddress,
                elementAttributeName,
                sendAttachments,
                contactAddress,
                action );

            // Note: Feature request - for add notification, send a test mail
            // to the newly added contact for verification.
        }
    }
    return result ;
}

OsStatus
MailboxManager::getPageRefreshInterval( UtlString& rPageRefreshInterval ) const
{
    if( m_pageRefreshInterval != "" )
        rPageRefreshInterval = m_pageRefreshInterval ;
    else
        rPageRefreshInterval = "1" ;

    return OS_SUCCESS ;
}

OsStatus
MailboxManager::getMinMessageLength( int& rMinMessageLength ) const
{
    if( m_minMessageLength > 0 )
        rMinMessageLength = m_minMessageLength ;
    else
        rMinMessageLength = 1 ;

    return OS_SUCCESS ;
}

OsStatus
MailboxManager::getResponseHeaders(int contentLength, UtlString& responseHeaders)
{
   responseHeaders.remove(0);
#ifdef __pingtel_on_posix__ /* [ */
   char contentlenstr[128];
   memset(contentlenstr, 0, 128*sizeof(char));
   sprintf(contentlenstr, "Content-length: %d\n", contentLength);
   responseHeaders = UtlString(contentlenstr) + VXML_HEADER;
#else /* __pingtel_on_posix__ ] */
   /*
    * On Windows, the content-length field is calculated and inserted correctly
    * by the Apache server, so no need to add it. Besides, the contentLength will
    * be incorrect because for each \n in the vxml body, a \r is added.
    */
   responseHeaders = VXML_HEADER;
#endif /* __pingtel_on_posix__ ] */

   return OS_SUCCESS;
}

OsStatus
MailboxManager::getMessageBlockSizeForWeb( int& rMessageBlockSize ) const
{
    // If the block size is not set to a valid value, then set it to -1.
    // blocksize = -1 indicates that all messages should be displayed on a single page.

    if( m_webMessageBlockSize == -1 || m_webMessageBlockSize > 0)
        rMessageBlockSize = m_webMessageBlockSize ;
    else
        rMessageBlockSize = -1 ;

    return OS_SUCCESS ;
}

OsStatus
MailboxManager::getCustomParameter( const UtlString& paramName,
                                    UtlString& rStrValue) const
{

    if( paramName == PARAM_LOG_LEVEL )
    {
            rStrValue = m_logLevel ;
    }
    else if( paramName == PARAM_DEFAULT_MSG_SUBJECT )
    {
        // handle this later.
    }
    else if( paramName == PARAM_REFRESH_INTERVAL )
    {
        // handle this later
    }
    else if( paramName == PARAM_MIN_MESSAGE_LENGTH )
    {
        // handle this later
    }
    else if( paramName == PARAM_MSG_BLOCK_SIZE )
    {
        // handle this later
    }
    else if( paramName == PARAM_DEFAULT_DOMAIN )
    {
        // handle this later
    }
    else if( paramName == PARAM_MAILSTORE_ROOT )
    {
        // handle this later
    }
    else if( paramName == PARAM_MEDIASERVER_URL )
    {
        // handle this later
    }
    else if( paramName == PARAM_SECURE_MEDIASERVER_URL )
    {
        // handle this later
    }
    else if( paramName == PARAM_VOICEMAIL_INFO_PLAYBACK )
    {
        if( m_voicemailInfoPlayback.isNull() )
            rStrValue = "DISABLE" ;
        else
            rStrValue = m_voicemailInfoPlayback ;
    }

    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                  "MailboxManager::getCustomParameter('%s') returns %d, rStrValue = '%s'",
                  paramName.data(), OS_SUCCESS, rStrValue.data());

    return OS_SUCCESS ;

}

OsStatus
MailboxManager::getSystemPromptsDir(const UtlBoolean& isGenericPrompts,
                                    UtlString& rPromptsDir ) const
{
    OsStatus result = OS_FAILED;

    if( isGenericPrompts )
    {
        // Construct the physical path to the generic mediaserver prompts directory.
        if( !m_mediaserverRoot.isNull() && m_mediaserverRoot != "")
        {
            // Location of generic (pingtel supplied) prompts.
            rPromptsDir =   m_mediaserverRoot + OsPathBase::separator +
                            STD_PROMPTS_DIR ;

            result = OS_SUCCESS;
        }
        else
        {
            writeToLog("getSystemPromptsDir", "Failed to get mediaserver root '<mediaserver-root>' from voicemail.xml.in", PRI_ERR);
        }
    }
    else
    {
        // Construct the physical path to the custom mediaserver prompts directory.
        if( !m_mailstoreRoot.isNull() && m_mailstoreRoot != "")
        {
            // Location of custom (administrator recorded) system prompts
            rPromptsDir =   m_mailstoreRoot + OsPathBase::separator +
                            CUSTOM_PROMPTS_DIR ;

            result = OS_SUCCESS;
        }
        else
        {
            writeToLog("getSystemPromptsDir", "Failed to get mailbox root '<mailstore-root>' from voicemail.xml.in", PRI_ERR);
        }
    }

    return result;
}


OsStatus
MailboxManager::getSystemPromptsUrl( UtlString& rPromptsUrl,
                                     const UtlBoolean& isGenericPrompts,
                                     const UtlBoolean& isFromWeb) const
{
    OsStatus result = OS_FAILED;

    if( !m_mediaserverSecureUrl.isNull() && m_mediaserverSecureUrl != "")
    {
        if( isGenericPrompts )
        {
            // Base URL of generic (pingtel supplied) system prompts
            rPromptsUrl =   m_mediaserverSecureUrl + URL_SEPARATOR +
                            STD_PROMPTS_DIR ;
        }
        else
        {
            // Base URL of custom administrator recorded prompts
            rPromptsUrl =   m_mediaserverSecureUrl + URL_SEPARATOR +
                            MEDIASERVER_ROOT_ALIAS + URL_SEPARATOR +
                            CUSTOM_PROMPTS_DIR ;
        }

        result = OS_SUCCESS;
    }
    else
    {
        writeToLog("getSystemPromptsUrl", "Failed to get secure mediaserver url from voicemail.xml.in", PRI_ERR);
    }

    return result;
}



OsStatus
MailboxManager::getOrgPrefsFileLocation( UtlString& rOrgPrefsFileLocation ) const
{
    OsStatus result = OS_FAILED;

    if( !m_mailstoreRoot.isNull() && m_mailstoreRoot != "")
    {
        // Construct the physical path to the location of the organization prefs file.
        rOrgPrefsFileLocation = m_mailstoreRoot + OsPathBase::separator +
                                ORGANIZATION_PREFS_FILE ;

        if ( OsFileSystem::exists(rOrgPrefsFileLocation) )
        {
            result = OS_SUCCESS;
        }
        else
        {
            //try to create the organization prefs file.
            result = createOrganizationPrefsFile( rOrgPrefsFileLocation );
        }
    }
    else
    {
        writeToLog("getOrgPrefsFileLocation", "Failed to get mailbox root '<mailstore-root>' from voicemail.xml.in", PRI_ERR);
    }

    return result;
}


OsStatus
MailboxManager::saveSystemPrompts (
    const UtlString& promptType,
    const char* data,
    const int&  datasize ) const
{
    OsStatus result = OS_SUCCESS;
    UtlString systemPromptsDir, logContents;

    // 1. Get the location of the custom prompts directory.
    result = getSystemPromptsDir( FALSE, systemPromptsDir ) ;
    if ( result == OS_SUCCESS )
    {
        // 2. Based on the type of prompt to be recorded,
        // construct the full path of the destination file.
        UtlString filename = systemPromptsDir + OsPathBase::separator ;

        if ( promptType == STANDARD_SYSTEM_GREETING )
            filename += STANDARD_SYSTEM_GREETING_FILE ;
        else if ( promptType == AFTER_HOURS_SYSTEM_GREETING )
            filename += AFTER_HOURS_SYSTEM_GREETING_FILE ;
        else if ( promptType == SPECIAL_OCCASION_SYSTEM_GREETING )
            filename += SPECIAL_OCCASION_SYSTEM_GREETING_FILE ;
        else if ( promptType == RECORDED_AUTOATTENDANT_PROMPT )
        {
            char buffer[256];
            long epochTime = OsDateTime::getSecsSinceEpoch();
            sprintf(buffer, "-%ld.wav", epochTime);
            filename += RECORDED_AUTOATTENDANT_PROMPT_FILE + UtlString(buffer);
            OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                          "saveSystemPrompts filename = %s", filename.data());
        }
        else
            result = OS_FAILED;

        if ( result == OS_SUCCESS )
        {
            // 3. If file exists, delete it and start fresh.
            if ( OsFileSystem::exists(filename) )
            {
                OsFile osfile (filename) ;
                osfile.remove();
            }

            // 4. Create a new file
            int file = open(filename, O_BINARY | O_CREAT | O_RDWR, 0644);
            unsigned long bytes_written = 0;
            if ( file != -1 )
            {
                // 5. write the data to the file.
                bytes_written = write( file, data, datasize );
                if( bytes_written <= 0 )
                {
                    logContents = "Unable to write recorded system greeting to " + filename ;
                    result = OS_FAILED ;
                }
                close(file);

                // 6. Set this prompt as the active prompt.
                // We will not do it here anymore. The prompt will be set from configure server.
#if 0
                result = setActiveSystemPrompt( promptType );
                if( result != OS_SUCCESS )
                    logContents = "Failed to set " + promptType + " as the active system prompt" ;
#endif                    
            } else
            {
                logContents = "Unable to create " + filename ;
                result = OS_FAILED;
            }
        } else
        {
            logContents = "Unknown prompt type " + promptType ;
        }
    } else
    {
        logContents = "Failed to get the custom system prompts directory";
    }

    if( result != OS_SUCCESS )
        writeToLog( "SaveSystemPrompts", logContents, PRI_ERR );

    return result;
}


OsStatus
MailboxManager::setActiveSystemPrompt(  const UtlString& promptType ) const
{
    OsStatus result = OS_FAILED;
    UtlString promptsDir, logContents, filename;

    // 1. Get the base dir of the prompt, based on the prompt type
    if( promptType == GENERIC_SYSTEM_GREETING ||
        promptType == GENERIC_AUTOATTENDANT_PROMPT )
    {
        result = getSystemPromptsDir( TRUE, promptsDir );
    }
    else if( promptType == DISABLE_AUTOATTENDANT_PROMPT )
    {
        // Dont bother. User just wants to disable autoattendant.
        promptsDir = "" ;
        result = OS_SUCCESS ;
    }
    else
    {
        result = getSystemPromptsDir( FALSE, promptsDir ) ;
    }

    if ( result == OS_SUCCESS )
    {
        // 2. Construct the full path of the WAV file corresponding to the prompt type.
        UtlString filename = promptsDir + OsPathBase::separator ;

        if ( promptType == STANDARD_SYSTEM_GREETING )
            filename += STANDARD_SYSTEM_GREETING_FILE ;
        else if ( promptType == AFTER_HOURS_SYSTEM_GREETING )
            filename += AFTER_HOURS_SYSTEM_GREETING_FILE ;
        else if ( promptType == SPECIAL_OCCASION_SYSTEM_GREETING )
            filename += SPECIAL_OCCASION_SYSTEM_GREETING_FILE ;
        else if ( promptType == RECORDED_AUTOATTENDANT_PROMPT )
            filename += RECORDED_AUTOATTENDANT_PROMPT_FILE ;
        else if ( promptType == GENERIC_AUTOATTENDANT_PROMPT )
            filename += GENERIC_AUTOATTENDANT_PROMPT_FILE ;
        else if ( promptType == GENERIC_SYSTEM_GREETING )
            filename += GENERIC_SYSTEM_GREETING_FILE ;
        else if ( promptType == DISABLE_AUTOATTENDANT_PROMPT )
        {
            // do nothing
        }
        else
        {
            result = OS_FAILED;
            logContents = "Unknown prompt type - " + promptType ;
        }


        if ( result == OS_SUCCESS )
        {
            // 3. Check if the wav file exists. If yes, proceed with setting it as active.
            if (    promptType == DISABLE_AUTOATTENDANT_PROMPT ||
                    OsFileSystem::exists(filename) )
            {
                // 4. Get the location of the organization preferences file.
                UtlString prefFileLocation;
                result = getOrgPrefsFileLocation( prefFileLocation );
                if( result == OS_SUCCESS )
                {
                    // Based on prompt type, set the element and its value
                    // to be updated in the prefs file
                    UtlString elementName, elementValue ;
                    if ( promptType == RECORDED_AUTOATTENDANT_PROMPT ||
                         promptType == GENERIC_AUTOATTENDANT_PROMPT ||
                         promptType == DISABLE_AUTOATTENDANT_PROMPT
                       )
                    {
                        elementName     = "autoattendant";
                        elementValue    = promptType ;
                    }
                    else
                    {
                        elementName     = "systemgreeting";
                        elementValue    = promptType ;
                    }

                    // 5. Update the file
                    result = updateOrganizationPrefsFile(   prefFileLocation,
                                                            elementName,
                                                            elementValue
                                                        );

                    if( result != OS_SUCCESS )
                        logContents = "Failed to update the organization prefs file " + prefFileLocation ;

                } else
                {
                    logContents = "Failed to get the location of organization prefs XML file";
                }
            }
            else
            {
                logContents = "Cannot find file " + filename ;
                result = OS_FILE_NOT_FOUND ;
            }
        }
    } else
    {
        logContents = "Failed to get the location of system prompts.";
    }

    if( result != OS_SUCCESS )
        writeToLog( "SetActiveSystemPrompt", logContents, PRI_ERR );

    return result;
}


OsStatus
MailboxManager::getSystemPromptUrl( const UtlString& promptType,
                                    UtlString& promptUrl,
                                    const UtlBoolean& isFromWeb) const
{
    OsStatus result = OS_FAILED;
    UtlString promptsDir, baseUrl, logContents, filename;

    // 1. Get the base dir of the prompt, based on the prompt type
    if( promptType == GENERIC_SYSTEM_GREETING ||
        promptType == GENERIC_AUTOATTENDANT_PROMPT )
    {
        result = getSystemPromptsDir( TRUE, promptsDir ) ;
        if( result == OS_SUCCESS )
            result = getSystemPromptsUrl( baseUrl, TRUE, isFromWeb );
    }
    else
    {
        result = getSystemPromptsDir( FALSE, promptsDir ) ;
        if( result == OS_SUCCESS )
            result = getSystemPromptsUrl( baseUrl, FALSE, isFromWeb ) ;
    }

    if ( result == OS_SUCCESS )
    {
        // 2. Construct the full path of the WAV file corresponding to the prompt type.
        UtlString filename = promptsDir + OsPathBase::separator ;
        promptUrl  = baseUrl + "/" ;

        if ( promptType == STANDARD_SYSTEM_GREETING )
        {
            filename += STANDARD_SYSTEM_GREETING_FILE ;
            promptUrl += STANDARD_SYSTEM_GREETING_FILE ;
        }
        else if ( promptType == AFTER_HOURS_SYSTEM_GREETING )
        {
            filename += AFTER_HOURS_SYSTEM_GREETING_FILE ;
            promptUrl += AFTER_HOURS_SYSTEM_GREETING_FILE;
        }
        else if ( promptType == SPECIAL_OCCASION_SYSTEM_GREETING )
        {
            filename += SPECIAL_OCCASION_SYSTEM_GREETING_FILE ;
            promptUrl += SPECIAL_OCCASION_SYSTEM_GREETING_FILE;
        }
        else if ( promptType == RECORDED_AUTOATTENDANT_PROMPT )
        {
            filename += RECORDED_AUTOATTENDANT_PROMPT_FILE ;
            promptUrl += RECORDED_AUTOATTENDANT_PROMPT_FILE;
        }
        else if ( promptType == GENERIC_AUTOATTENDANT_PROMPT )
        {
            filename += GENERIC_AUTOATTENDANT_PROMPT_FILE ;
            promptUrl += GENERIC_AUTOATTENDANT_PROMPT_FILE;
        }
        else if ( promptType == GENERIC_SYSTEM_GREETING )
        {
            filename += GENERIC_SYSTEM_GREETING_FILE ;
            promptUrl += GENERIC_SYSTEM_GREETING_FILE;
        }
        else
        {
            result = OS_FAILED;
            logContents = "Unknown prompt type - " + promptType ;
        }


        if ( result == OS_SUCCESS )
        {
            // 3. Check if the wav file exists. If yes, return the URL.
            if ( !OsFileSystem::exists(filename) )
            {
                logContents = "File not found - " + filename ;
                result = OS_FILE_NOT_FOUND ;
            }
        }
    } else
    {
        logContents = "Failed to get the location of system prompts.";
    }

    if( result != OS_SUCCESS )
        writeToLog( "getSystemPromptUrl", logContents, PRI_ERR );

    return result;
}


OsStatus
MailboxManager::createOrganizationPrefsFile( const UtlString& orgPrefsFileLocation ) const
{

    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                  "createOrganizationPrefsFile creating a default file %s",
                  orgPrefsFileLocation.data());
                  
    OsFile prefsFile (orgPrefsFileLocation);
    OsStatus result = prefsFile.open(OsFile::CREATE);
    if (result == OS_SUCCESS)
    {
        UtlString defaultPrefsData =
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n"
            "<organizationprefs>\n"
            "   <specialoperation>" + UtlString( "false" ) + "</specialoperation>\n"
            "   <autoattendant>" + UtlString( "afterhour" ) + "</autoattendant>\n"
            "</organizationprefs>";
   
        unsigned long bytes_written = 0;
        result = prefsFile.write(defaultPrefsData.data(),
                                 defaultPrefsData.length(),
                                 bytes_written);
        prefsFile.close();
    }
   
    return result;
}


OsStatus
MailboxManager::updateOrganizationPrefsFile (
    const UtlString& prefsFileLocation,
    const UtlString& elementName,
    const UtlString& elementValue ) const
{
    OsStatus result = OS_FAILED;

    TiXmlDocument doc ( prefsFileLocation );

    // 1. Verify that we can load the file (i.e it must exist)
    if( doc.LoadFile() )
    {
        TiXmlNode * rootNode = doc.FirstChild ("organizationprefs");
        if ( rootNode != NULL )
        {
            // 2. Get a reference to the element to be updated
            TiXmlNode* node = rootNode->FirstChild( elementName );
            if ( (node != NULL) && (node->Type() == TiXmlNode::ELEMENT) )
            {
                if ( node->FirstChild() && node->FirstChild()->Type() == TiXmlNode::TEXT )
                {
                    // 3. set the new value for the element.
                    node->FirstChild()->SetValue( elementValue );
                }
            }

            // 4. Save the updated document
            if ( doc.SaveFile( prefsFileLocation ) == true )
            {
                result = OS_SUCCESS;
            }
        }
    } else
    {
        // File does not exist. Create it with default values.
        result = createOrganizationPrefsFile( prefsFileLocation );
    }

    return result;
}


OsStatus
MailboxManager::parseOrganizationPrefsFile ( UtlHashMap* rOrgPrefsHashDict) const
{
    OsStatus result = OS_FAILED;
    UtlString prefsFileName;

    // 1. Get the location of the organization prefs file.
    result = getOrgPrefsFileLocation(prefsFileName);
    if(result == OS_SUCCESS)
    {
        TiXmlDocument doc (prefsFileName);

        // 2. Verify that we can load the file (i.e it must exist)
        if(doc.LoadFile())
        {
            // 3. Get the root element of the XML file.
            TiXmlNode * rootNode = doc.FirstChild ("organizationprefs");
            if (rootNode != NULL)
            {
                // 4. Get individual element values.
                UtlString value;
                result = getConfigValue (*rootNode, "specialoperation", value);
                rOrgPrefsHashDict->insertKeyAndValue(
                        new UtlString("specialoperation"),
                        new UtlString(value));

                result = getConfigValue (*rootNode, "autoattendant", value);
                rOrgPrefsHashDict->insertKeyAndValue(
                        new UtlString("autoattendant"),
                        new UtlString(value));
            }
        }
    }
    
    return result;
}


OsStatus
MailboxManager::sendEmailNotification(  const UtlString& mailboxIdentity,
                                        const UtlString& from,
                                        const UtlString& timestamp,
                                        const UtlString& duration,
                                        const UtlString& wavFileName,
                                        const char* data,
                                        const int& datasize) const
{
   UtlString logContent;
   OsStatus result = OS_SUCCESS;

   OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                 "MailboxManager::sendEmailNotification: mailboxIdentity = '%s', from = '%s', timestamp = '%s', duration ='%s', wavFileName = '%s', data = '%.*s%s', datasize = %d",
                 mailboxIdentity.data(), from.data(), timestamp.data(),
                 duration.data(), wavFileName.data(),
                 (datasize > 20 ? 20 : datasize), data,
                 (datasize > 20 ? "..." : ""), datasize);

   UtlHashMap contactsHashDictionary;
   getNotificationContactList (
      mailboxIdentity,
      &contactsHashDictionary );

   if( contactsHashDictionary.entries() > 0 )
   {
      // Get the sorted list of contacts.
      UtlSortedList* contactList = (UtlSortedList*)
         contactsHashDictionary.findValue(new UtlString("sortedcontacts"));

      if (contactList != NULL)
      {
         if( contactList->entries() == 0 )
         {
            logContent = "No email addresses found for notifying new voicemail.";
            writeToLog( "sendEmailNotification" , logContent, PRI_DEBUG);
         }


         // is email notification enabled for this mailbox?
         while( contactList->entries() > 0 )
         {
            // Get the contact address from the vector
            UtlString* rwAddress = (UtlString*) contactList->removeAt(0);
            UtlString contactStr = rwAddress->data();

            // Get the hash dictionary containing contact details.
            UtlHashMap* contactDetailsHashDict = (UtlHashMap*) contactsHashDictionary.
               findValue( rwAddress );
            UtlString* rwContactType = (UtlString*) contactDetailsHashDict->
               findValue( new UtlString("type") );
            UtlString* rwSendAttachments = (UtlString*) contactDetailsHashDict->
               findValue( new UtlString("attachments") );

            UtlString contactType = rwContactType->data();
            UtlString sendAttachments = rwSendAttachments->data();
            UtlBoolean bAttachments = sendAttachments == "yes";

            if( !m_smtpServer.isNull())
            {
               // send an email (from name, from email, smtp server
               UtlString replyTo;
               if( !m_emailNotificationAddr.isNull() )
                  replyTo = m_emailNotificationAddr;
               else
               {
                  logContent = "Email address from which voicemail notification should be sent was not set (email-notification-addr in voicemail.xml is null). Defaulting to " +replyTo;
                  writeToLog( "sendEmailNotification" , logContent, PRI_ERR);
                  replyTo = "postmaster@" + m_smtpServer;
               }

               Url mailboxUrl(m_mailboxUrl);

               OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                             "MailboxManager::sendEmailNotification: m_smtpServer = '%s', mailboxUrl = '%s', contactStr = '%s', duration = '%s', bAttachments = %d",
                             m_smtpServer.data(),
                             mailboxUrl.toString().data(),
                             contactStr.data(), duration.data(), bAttachments);
               NotificationHelper::getInstance()->send(
                  mailboxIdentity,
                  m_smtpServer,     // smtp server
                  mailboxUrl,  // root address of mailbox ui and operations
                  contactStr,       // to
                  from,             // from
                  replyTo,          // replyto
                  timestamp,        // date
                  duration,        // duration in seconds
                  wavFileName,     // location of the WAV file URL
                  data,
                  datasize,
                  bAttachments );   // AttachmentEnabled
               writeToLog( "sendEmailNotification", "Email Notification - end", PRI_DEBUG);
            }
            else
            {
               result = OS_SUCCESS;
               logContent = "No smtp-server defined - email notices for voicemail disabled";
               writeToLog( "sendEmailNotification" , logContent, PRI_CRIT);
            }
         }
         delete contactList;
      }
      else
      {
         result = OS_SUCCESS;
         logContent = "No email addresses found for notifying new voicemail.";
         writeToLog( "sendEmailNotification" , logContent, PRI_DEBUG);
      }
   }
   else
   {
      result = OS_SUCCESS;
      logContent = "No email addresses found for notifying new voicemail.";
      writeToLog( "sendEmailNotification" , logContent, PRI_DEBUG);
   }

   writeToLog( "sendEmailNotification" , "End reached", PRI_DEBUG);

   return result;
}

OsStatus
MailboxManager::parseDistributionFile(const UtlString& distFile, const UtlString& distId, UtlSortedList& destinations) const
{
    OsStatus result = OS_FAILED;

    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                  "MailboxManager - parseDistributionFile:: start to parse the file = %s", distFile.data());
    TiXmlDocument doc(distFile);

    // Verify that we can load the file (i.e it must exist)
    if(doc.LoadFile())
    {
       OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                     "sendByDistListCGI - doc.LoadFile() returned TRUE");

       TiXmlNode * rootNode = doc.FirstChild ("distributions");

       if (rootNode != NULL)
       {
          // Search in each list
          for (TiXmlNode *groupNode = rootNode->FirstChild("list");
               groupNode; 
               groupNode = groupNode->NextSibling("list"))
          {
             // Compare the value of distId with the index in this list
             if (distId.compareTo(((groupNode->FirstChild("index"))->FirstChild())->Value()) == 0 )
             {
                // The requested distribution list is found
                for (TiXmlNode *destNode = groupNode->FirstChild("destination");
                     destNode; 
                     destNode = destNode->NextSibling("destination"))
                {
                   UtlString mailbox = (destNode->FirstChild())->Value();
                   destinations.insert( new UtlString (mailbox) );
                   OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                                 "MailboxManager - parseDistributionFile:: Use the destination = %s", mailbox.data());

                }

                return OS_SUCCESS;
             }
          }
       }
    }

    return result;
}

OsStatus
MailboxManager::setPassword (
    const UtlString& loginString,
    const UtlString& newPassToken )
{
   OsStatus result = OS_FAILED;
   
   // Get the user id from the loginString
   int substrLength = loginString.index("@");
   UtlString userId = loginString(0, substrLength);

#ifdef USE_SOAP
   // Get the superadmin passtoken
   Url mailboxUrl;
   UtlString dbPassToken, dbAuthType;
   CredentialDB::getInstance()->getUserPin ("superadmin",      // IN
                                            m_defaultRealm,    // IN
                                            mailboxUrl,        // OUT
                                            dbPassToken,       // OUT
                                            dbAuthType );      // OUT
#else
   // Get the old passtoken
   Url mailboxUrl;
   UtlString dbPassToken, dbAuthType;
   CredentialDB::getInstance()->getUserPin (userId,            // IN
                                            m_defaultRealm,    // IN
                                            mailboxUrl,        // OUT
                                            dbPassToken,       // OUT
                                            dbAuthType );      // OUT
#endif

   OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                 "Sending the new pin to Config server for user %s", userId.data());

   // Send to the config server
#ifdef USE_SOAP   
   Url userServiceUrl(m_configServerSecureUrl + "//pds/soap/services/urn:UserService");
   userServiceUrl.setUserId("superadmin");
   userServiceUrl.setPassword(dbPassToken.data());

   // Built a SOAP message and send it to the Config Server via https
   char buffer[1024];
   sprintf(buffer, _SOAP_MSG_TEMPLATE_,
           soap_env_ns.data(), soap_env_enc.data(),
           soap_xsi_ns.data(), soap_xsd_ns.data(),
           "setPintoken", "urn:UserService", userId.data(), newPassToken.data(), "setPintoken");

   UtlString soapMessage = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" + UtlString(buffer);    

   HttpMessage *pRequest = new HttpMessage();
   pRequest->setFirstHeaderLine(HTTP_POST_METHOD, "//pds/soap/services/urn:UserService", HTTP_PROTOCOL_VERSION_1_1);
   pRequest->addHeaderField("Connection", "close");
   pRequest->addHeaderField("Accept", "text/xml");
   pRequest->addHeaderField("Accept", "multipart/*");
   pRequest->setUserAgentField("Media-Server");

   HttpBody* body = new HttpBody(soapMessage.data(),
                                 soapMessage.length(),
                                 CONTENT_TYPE_TEXT_XML);

   pRequest->setBody(body);
   pRequest->setContentType(CONTENT_TYPE_TEXT_XML);
   pRequest->setContentLength(soapMessage.length());
   pRequest->addHeaderField("SOAPAction", "#setPintoken");

   // Create an empty response object and sent the built up request
   // via it to the config server
   HttpMessage *pResponse = new HttpMessage();

   pResponse->get(userServiceUrl, *pRequest, 5*1000, false);

   UtlString status;

   pResponse->getResponseStatusText(&status);
   OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                 "setPassword status = %s", status.data());


   if (status.compareTo("OK") == 0)
   {
      OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                    "setPassword successful with status = %s", status.data());

      Url dataSetServiceUrl(m_configServerSecureUrl + "//pds/soap/services/urn:DataSetService");
      dataSetServiceUrl.setUserId("superadmin");
      dataSetServiceUrl.setPassword(dbPassToken.data());
      
      // Send out resync command
      sprintf(buffer, _SOAP_MSG_NO_ARG_TEMPLATE_,
              soap_env_ns.data(), soap_env_enc.data(),
              soap_xsi_ns.data(), soap_xsd_ns.data(),
              "rebuildDataSets", "urn:DataSetService", "rebuildDataSets");

      soapMessage = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" + UtlString(buffer);
            
      delete pRequest;
      delete pResponse;
      
      pRequest = new HttpMessage();
      pRequest->setFirstHeaderLine(HTTP_POST_METHOD, "//pds/soap/services/urn:DataSetService", HTTP_PROTOCOL_VERSION_1_1);
      pRequest->addHeaderField("Connection", "close");
      pRequest->addHeaderField("Accept", "text/xml");
      pRequest->addHeaderField("Accept", "multipart/*");
      pRequest->setUserAgentField("Media-Server");
      
      body = new HttpBody(soapMessage.data(),
                          soapMessage.length(),
                          CONTENT_TYPE_TEXT_XML);

      pRequest->setBody(body);
      pRequest->setContentType(CONTENT_TYPE_TEXT_XML);
      pRequest->setContentLength(soapMessage.length());
      pRequest->addHeaderField("SOAPAction", "#rebuildDataSets");

      pResponse = new HttpMessage();
      pResponse->get(dataSetServiceUrl, *pRequest, 5*1000, false);
      
      pResponse->getResponseStatusText(&status);

      if (status.compareTo("OK") == 0)
      {
         result = OS_SUCCESS;
      }
      else
      {
         OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                      "resync dataset failed with status = %s", status.data());
         result = OS_FAILED;
      }
   }
   else
   {
      OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                    "setPassword failed with status = %s", status.data());
      result = OS_FAILED;
   }
   
   delete pRequest;
   delete pResponse;
#else
   Url userServiceUrl(m_configServerSecureUrl + "/sipxconfig/api/change-pintoken");

   // Built a http message and send it to the Config Server via https
   char buffer[1024];
   UtlString encodedToken;
   UtlString textToEncode = userId + ":" + m_defaultRealm + ":" + newPassToken;
   NetMd5Codec::encode(textToEncode, encodedToken);
   sprintf(buffer, "%s;%s;%s", userId.data(), dbPassToken.data(), encodedToken.data());

   UtlString httpMessage = UtlString(buffer);    

   HttpMessage *pRequest = new HttpMessage();
   pRequest->setFirstHeaderLine(HTTP_POST_METHOD, "/sipxconfig/api/change-pintoken", HTTP_PROTOCOL_VERSION_1_1);
   pRequest->addHeaderField("Connection", "close");
   pRequest->addHeaderField("Accept", "text/xml");
   pRequest->addHeaderField("Accept", "multipart/*");
   pRequest->setUserAgentField("Media-Server");

   HttpBody* body = new HttpBody(httpMessage.data(),
                                 httpMessage.length(),
                                 CONTENT_TYPE_TEXT_XML);

   pRequest->setBody(body);
   pRequest->setContentType(CONTENT_TYPE_TEXT_XML);
   pRequest->setContentLength(httpMessage.length());

   // Create an empty response object and sent the built up request
   // via it to the config server
   HttpMessage *pResponse = new HttpMessage();

   pResponse->get(userServiceUrl, *pRequest, 5*1000, false);

   UtlString status;

   pResponse->getResponseStatusText(&status);
   OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                 "setPassword status = %s", status.data());


   if (status.compareTo("OK") == 0)
   {
      OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                    "setPassword successful with status = %s", status.data());
      result = OS_SUCCESS;
   }
   else
   {
      OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                    "setPassword failed with status = %s", status.data());
      result = OS_FAILED;
   }
   
   delete pRequest;
   delete pResponse;

#endif
  
   return result;
}


OsStatus
MailboxManager::getTimeBasedAAName ( UtlString& rName,
                                     UtlString& rLocalTime,
                                     UtlString& rAAName ) const
{
    OsStatus result = OS_FAILED;
    
    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                  "getTimeBasedAAName local time = %s", rLocalTime.data());
                  
    // Mon, 25-Sep-2002 05:51:44 PM EST
    UtlString dayOfWeek;
    UtlString time;
    UtlString day;
    UtlString hour;
    UtlString minute;
    
    dayOfWeek = rLocalTime(0, 3);

    int dayLength = rLocalTime.index(" ", 5);
    day = rLocalTime(5, dayLength-5);
    if (dayLength == 15)
    {
       day = "0" + day;
    }
    
    int timeLength = rLocalTime.index(" ", dayLength+1);
    time = rLocalTime(dayLength+1, timeLength-dayLength-1);
    hour = time(0, 2);
    minute = time(3, 2);

    int hr = atoi(hour.data()) ;
    int currentTime = hr * 100 + atoi(minute);

    // Adjust for 12 AM (0 hours), and 1-11 PM (13-23 hours), but not 12 PM
    // (fixes XMR-72) --Woof!
    if (rLocalTime.index("PM") != UTL_NOT_FOUND)
    {
       if (hr < 12)
       {
          // 1-11 PM is 13-23 hours
          currentTime += 1200 ;
       }
       // 12 PM (noon) is 12 hours, no change needed
    }
    else
    {
       // 1-11 AM, no change needed.
       if (hr == 12) 
       {
          // 12 AM is midnight or 0 hours
          currentTime -= 1200 ;
       }
    }
    
    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                  "getTimeBasedAAName dayOfWeek = %s, day = %s, currentTime = %d",
                  dayOfWeek.data(), day.data(), currentTime);
                  
    UtlString scheduleFileLocation;
    result = getScheduleFileLocation( rName, scheduleFileLocation );
    if( result == OS_SUCCESS )
    {
        // Check whether the special case is on or not
        UtlHashMap rOrgPrefsHashDict;        
        result = parseOrganizationPrefsFile(&rOrgPrefsHashDict);
        UtlString key("specialoperation");
        UtlString* special = (UtlString *) rOrgPrefsHashDict.findValue(&key);
        OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                      "getTimeBasedAAName special operation = %s", special->data());
        
        if (special->compareTo("true", UtlString::ignoreCase) == 0)
        {
            UtlString name("autoattendant");
            UtlString* aaMenu = (UtlString *) rOrgPrefsHashDict.findValue(&name);
            rAAName = aaMenu->data();
        }
        else
        {
            AutoAttendantSchedule scheduleFile;
            result = parseAutoAttendantSchduleFile( scheduleFileLocation, &scheduleFile );
            
            if (result == OS_SUCCESS)
            {
                // Check whether it is on holiday
                if (scheduleFile.mHolidays.find(&day))
                {
                    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                                  "getTimeBasedAAName today is a holiday = %s", day.data());
                    rAAName = scheduleFile.mHolidayMenu;
                }
                else
                {
                    // Check whether it is in rgular hour
                    UtlString* fromHour = (UtlString *) scheduleFile.mRegularFromHour.findValue(&dayOfWeek);
                    UtlString* toHour = (UtlString *) scheduleFile.mRegularToHour.findValue(&dayOfWeek);
                    if (fromHour != NULL && toHour != NULL)
                    {
                        if ((timeValue(fromHour->data()) < currentTime) &&
                            (currentTime < timeValue(toHour->data())))
                        {
                            rAAName = scheduleFile.mRegularMenu;
                        }
                        else
                        {        
                            rAAName = scheduleFile.mAfterHourMenu;
                        }
                    }
                    else {
                        rAAName = scheduleFile.mAfterHourMenu;
                    }
                }
            }
            else
            {
                OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_ERR,
                              "getTimeBasedAAName failed to parse the schedule file %s", scheduleFileLocation.data());
                
            }
        }  
    }
    
    return result;
}


OsStatus
MailboxManager::parseAutoAttendantSchduleFile ( UtlString& fileLocation,
                                                AutoAttendantSchedule* rSchedule ) const
{
    OsStatus result = OS_FAILED;

    TiXmlDocument doc (fileLocation);

    // 1. Verify that we can load the file (i.e it must exist)
    if(doc.LoadFile())
    {
        // 2. Get the root element of the XML file.
        TiXmlNode * rootNode = doc.FirstChild ("schedules");
        if (rootNode != NULL)
        {
/*
            // 3. Get individual element values.              
            //
            //<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n" \
            //<schedules>\n" \
            //  <autoattendant>\n" \
            //    <holidays>\n" \
            //      <date>1-Jan-2005</date>\n" \
            //      <date>4-Jul-2005</date>\n" \
            //      <date>25-Dec-2005</date>\n" \
            //      <filename>%s</filename>\n" \
            //    </holidays>\n" \
            //    <regularhours>\n" \
            //      <monday>\n" \
            //        <from>08:00</from>\n" \
            //        <to>18:00</to>\n" \
            //      </monday>\n" \
            //      <tuesday>\n" \
            //        <from>08:00</from>\n" \
            //        <to>18:00</to>\n" \
            //      </tuesday>\n" \
            //      <wednesday>\n" \
            //        <from>08:00</from>\n" \
            //        <to>18:00</to>\n" \
            //      </wednesday>\n" \
            //      <thursday>\n" \
            //        <from>08:00</from>\n" \
            //        <to>18:00</to>\n" \
            //      </thursday>\n" \
            //      <friday>\n" \
            //        <from>08:00</from>\n" \
            //        <to>17:00</to>\n" \
            //      </friday>\n" \
            //      <filename>%s</filename>\n" \
            //    </regularhours>\n" \
            //    <afterhours>\n" \
            //      <filename>%s</filename>\n" \
            //    </afterhours>\n" \
            //  </autoattendant>\n" \
            //</schedules>";
*/
              
            TiXmlNode* aaNode = rootNode->FirstChild("autoattendant");
            if (aaNode != NULL)
            {
                // Get dates for holiday
                TiXmlNode* holidayNode = aaNode->FirstChild("holidays");
                if (holidayNode != NULL)
                {
                    for (TiXmlNode* dateNode = holidayNode->FirstChild("date");
                         dateNode; 
                         dateNode = dateNode->NextSibling("date"))
                    {
                        UtlString date = dateNode->FirstChild()->Value();
                        rSchedule->mHolidays.insert(new UtlString(date));
                    }
                      
                    TiXmlNode* holidayName = holidayNode->FirstChild("filename");
                    if (holidayName != NULL)
                    {
                        rSchedule->mHolidayMenu = holidayName->FirstChild()->Value();
                    }
                }

                // Get regular hours
                TiXmlNode* regularNode = aaNode->FirstChild("regularhours");
                if (regularNode != NULL)
                {
                    TiXmlNode* dayNode = regularNode->FirstChild("sunday");
                    if (dayNode != NULL)
                    {
                        TiXmlNode* fromNode = dayNode->FirstChild("from");
                        if (fromNode)
                        {
                            UtlString from = fromNode->FirstChild()->Value();
                            rSchedule->mRegularFromHour.insertKeyAndValue(new UtlString("Sun"),
                                                                          new UtlString(from));
                        }

                        TiXmlNode* toNode = dayNode->FirstChild("to");
                        if (toNode)
                        {
                            UtlString to = toNode->FirstChild()->Value();
                            rSchedule->mRegularToHour.insertKeyAndValue(new UtlString("Sun"),
                                                                        new UtlString(to));
                        }
                    }

                    dayNode = regularNode->FirstChild("monday");
                    if (dayNode != NULL)
                    {
                        TiXmlNode* fromNode = dayNode->FirstChild("from");
                        if (fromNode)
                        {
                            UtlString from = fromNode->FirstChild()->Value();
                            rSchedule->mRegularFromHour.insertKeyAndValue(new UtlString("Mon"),
                                                                          new UtlString(from));
                        }

                        TiXmlNode* toNode = dayNode->FirstChild("to");
                        if (toNode)
                        {
                            UtlString to = toNode->FirstChild()->Value();
                            rSchedule->mRegularToHour.insertKeyAndValue(new UtlString("Mon"),
                                                                        new UtlString(to));
                        }
                    }

                    dayNode = regularNode->FirstChild("tuesday");
                    if (dayNode != NULL)
                    {
                        TiXmlNode* fromNode = dayNode->FirstChild("from");
                        if (fromNode)
                        {
                            UtlString from = fromNode->FirstChild()->Value();
                            rSchedule->mRegularFromHour.insertKeyAndValue(new UtlString("Tue"),
                                                                          new UtlString(from));
                        }

                        TiXmlNode* toNode = dayNode->FirstChild("to");
                        if (toNode)
                        {
                            UtlString to = toNode->FirstChild()->Value();
                            rSchedule->mRegularToHour.insertKeyAndValue(new UtlString("Tue"),
                                                                        new UtlString(to));
                        }
                    }

                    dayNode = regularNode->FirstChild("wednesday");
                    if (dayNode != NULL)
                    {
                        TiXmlNode* fromNode = dayNode->FirstChild("from");
                        if (fromNode)
                        {
                            UtlString from = fromNode->FirstChild()->Value();
                            rSchedule->mRegularFromHour.insertKeyAndValue(new UtlString("Wed"),
                                                                          new UtlString(from));
                        }

                        TiXmlNode* toNode = dayNode->FirstChild("to");
                        if (toNode)
                        {
                            UtlString to = toNode->FirstChild()->Value();
                            rSchedule->mRegularToHour.insertKeyAndValue(new UtlString("Wed"),
                                                                        new UtlString(to));
                        }
                    }

                    dayNode = regularNode->FirstChild("thursday");
                    if (dayNode != NULL)
                    {
                        TiXmlNode* fromNode = dayNode->FirstChild("from");
                        if (fromNode)
                        {
                            UtlString from = fromNode->FirstChild()->Value();
                            rSchedule->mRegularFromHour.insertKeyAndValue(new UtlString("Thu"),
                                                                          new UtlString(from));
                        }

                        TiXmlNode* toNode = dayNode->FirstChild("to");
                        if (toNode)
                        {
                            UtlString to = toNode->FirstChild()->Value();
                            rSchedule->mRegularToHour.insertKeyAndValue(new UtlString("Thu"),
                                                                        new UtlString(to));
                        }
                    }

                    dayNode = regularNode->FirstChild("friday");
                    if (dayNode != NULL)
                    {
                        TiXmlNode* fromNode = dayNode->FirstChild("from");
                        if (fromNode)
                        {
                            UtlString from = fromNode->FirstChild()->Value();
                            rSchedule->mRegularFromHour.insertKeyAndValue(new UtlString("Fri"),
                                                                          new UtlString(from));
                        }

                        TiXmlNode* toNode = dayNode->FirstChild("to");
                        if (toNode)
                        {
                            UtlString to = toNode->FirstChild()->Value();
                            rSchedule->mRegularToHour.insertKeyAndValue(new UtlString("Fri"),
                                                                        new UtlString(to));
                        }
                    }

                    dayNode = regularNode->FirstChild("saturday");
                    if (dayNode != NULL)
                    {
                        TiXmlNode* fromNode = dayNode->FirstChild("from");
                        if (fromNode)
                        {
                            UtlString from = fromNode->FirstChild()->Value();
                            rSchedule->mRegularFromHour.insertKeyAndValue(new UtlString("Sat"),
                                                                          new UtlString(from));
                        }

                        TiXmlNode* toNode = dayNode->FirstChild("to");
                        if (toNode)
                        {
                            UtlString to = toNode->FirstChild()->Value();
                            rSchedule->mRegularToHour.insertKeyAndValue(new UtlString("Sat"),
                                                                        new UtlString(to));
                        }
                    }

                    TiXmlNode* regularName = regularNode->FirstChild("filename");
                    if (regularName != NULL)
                    {
                        rSchedule->mRegularMenu = regularName->FirstChild()->Value();
                    }
                }

                // Get after hours
                TiXmlNode* afterhourNode = aaNode->FirstChild("afterhours");
                if (afterhourNode != NULL)
                {
                    TiXmlNode* afterhourName = afterhourNode->FirstChild("filename");
                    if (afterhourName != NULL)
                    {
                        rSchedule->mAfterHourMenu = afterhourName->FirstChild()->Value();
                    }
                }
                
                result = OS_SUCCESS;
            }
        }
    }

    return result;
}


OsStatus
MailboxManager::getScheduleFileLocation( UtlString& rFileName, UtlString& rScheduleFileLocation ) const
{
    OsStatus result = OS_FAILED;

    // Construct the physical path to the location of the organization prefs file.
    if (!m_mediaserverRoot.isNull() && m_mediaserverRoot != "")
    {
        rScheduleFileLocation = m_mediaserverRoot + OsPathBase::separator + AA_SCHEDULE_DIR + rFileName + "-schedule.xml";
    
        if ( OsFileSystem::exists(rScheduleFileLocation) )
        {
            result = OS_SUCCESS;
        }
        else
        {
            OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_ERR,
                          "getScheduleFileLocation failed to find the schedule file %s",
                          rScheduleFileLocation.data());
        }
    }
    else
    {
        OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_ERR,
                      "getScheduleFileLocation failed to find mailserver root dir");
    }
        

    return result;
}


int MailboxManager::timeValue(const char* currentTime) const
{
    UtlString time(currentTime);  
    UtlString hour = time(0, 2);
    UtlString minute = time(3, 2);
    int timeInValue = atoi(hour)*100 + atoi(minute);
    
    return timeInValue;
}    


OsStatus
MailboxManager::setSystemOverallAAMenu(  bool special ) const
{
    OsStatus result = OS_FAILED;
    UtlString logContents;
                
    UtlString prefFileLocation;
    result = getOrgPrefsFileLocation( prefFileLocation );
    if( result == OS_SUCCESS )
    {
        UtlString elementName, elementValue ;
        if (special)
        {
            elementName = "specialoperation";
            elementValue = "true";
        }
        else
        {
            elementName = "specialoperation";
            elementValue = "false";
        }

        result = updateOrganizationPrefsFile(prefFileLocation, elementName, elementValue);

        if( result != OS_SUCCESS )
        {
            logContents = "Failed to update the organization prefs file " + prefFileLocation ;

        }
    }
    else
    {
        logContents = "Failed to get the location of organization prefs XML file";
    }

    if( result != OS_SUCCESS )
        writeToLog( "setSystemOverallAAMenu", logContents, PRI_ERR );

    return result;
}
