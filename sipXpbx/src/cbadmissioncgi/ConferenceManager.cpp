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
#include "cgicc/Cgicc.h"
#include "cgicc/CgiEnvironment.h"
#include "sipxcgi/CgiValues.h"
#include "xmlparser/tinyxml.h"
#include "mailboxmgr/VXMLDefs.h"
#include "cbadmissioncgi/ConferenceManager.h"
#include "utl/UtlTokenizer.h"
#include "utl/UtlHashMap.h"
#include "utl/UtlHashMapIterator.h"
#include "os/OsDateTime.h"

// DEFINES
#ifndef O_BINARY        // needed for WIN32
    #define O_BINARY 0
#endif
#define CONFERENCE_CONFIG_FILE     SIPX_CONFDIR "/cbadmission.xml"
#define CONFERENCE_REGISTER_FILE   SIPX_CONFDIR "/conferences.xml"

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
ConferenceManager* ConferenceManager::spInstance = NULL;
OsMutex ConferenceManager::sLockMutex (OsMutex::Q_FIFO);


/* ============================ CREATORS ================================== */
ConferenceManager::ConferenceManager() :
    m_logLevel(LOG_LEVEL_ERROR)
{
    // Parse the configuration file voicemail.xml
    parseConfigFile ( UtlString(CONFERENCE_CONFIG_FILE) );
}

ConferenceManager::~ConferenceManager()
{
}

ConferenceManager*
ConferenceManager::getInstance()
{
    // Critical Section here
    OsLock lock( sLockMutex );

    // See if this is the first time through for this process
    // Note that this being null => pgDatabase is also null
    if ( spInstance == NULL )
    {   // Create the singleton class for clients to use
        spInstance = new ConferenceManager();
    }
    return spInstance;
}


OsStatus
ConferenceManager::parseConfigFile ( const UtlString& configFileName )
{
    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                  "Entering ConferenceManager::parseConfigFile('%s')\n",
                  configFileName.data());
    OsStatus result = OS_FAILED;

    TiXmlDocument doc ( configFileName );

    // Verify that we can load the file (i.e it must exist)
    if( doc.LoadFile() )
    {
       OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                     "ConferenceManager::parseConfigFile: doc.LoadFile() returned TRUE");
        TiXmlNode * rootNode = doc.FirstChild ("settings");
        if (rootNode != NULL)
        {
            TiXmlNode * voicemailNode = rootNode->FirstChild("cbadmission");
            if (voicemailNode != NULL)
            {
                getConfigValue ( *voicemailNode, "default-domain", m_defaultDomain );
                getConfigValue ( *voicemailNode, "default-realm", m_defaultRealm );
                getConfigValue ( *voicemailNode, "mailstore-root", m_mailstoreRoot );
                getConfigValue ( *voicemailNode, "mediaserver-root", m_mediaserverRoot );
                getConfigValue ( *voicemailNode, "mediaserver-url", m_mediaserverUrl );
                getConfigValue ( *voicemailNode, "mediaserver-url-secure", m_mediaserverSecureUrl );
                getConfigValue ( *voicemailNode, "full-mediaserver-url", m_fullMediaserverUrl );
                getConfigValue ( *voicemailNode, "full-mediaserver-url-secure", m_fullMediaserverSecureUrl );
                getConfigValue ( *voicemailNode, "ivr-prompt-url", m_ivrPromptUrl );
                getConfigValue ( *voicemailNode, "cbadmission-cgi-log-level", m_logLevel);
            }
        }
    }
   else
   {
      // We are unable to parse the configuration file.
      // Log an error with a high severity to make sure that it makes it
      // into the log.
      OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_CRIT,
                    "ConferenceManager::parseConfigFile: doc.LoadFile() returned FALSE while attempting to process configuration file '%s'",
                    configFileName.data());
   }

   OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                 "ConferenceManager::parseConfigFile: returning %d", result);
   return result;
}


OsStatus
ConferenceManager::getConfigValue (
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
                  "ConferenceManager::getConfigValue('%s') returns result = %d value = '%s'",
                  key.data(), result,
                 (result == OS_SUCCESS ? value.data() : "[unknown]"));
    return result;
}

OsStatus
ConferenceManager::setConfigValue (
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
ConferenceManager::doLogin (
    const UtlString& contact,
    const UtlString& confId,
    const UtlString& accessCode,
    UtlString& conferenceUrl )
{
    OsStatus result = OS_FAILED;
    
    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                 "ConferenceManager::doLogin conference Id %s from contact %s",
                 confId.data(), contact.data());
    
    // Validate the access code
    UtlString organizerCode;
    UtlString participantCode;
    UtlString urlString;
    UtlString value;
    result = parseConferenceFile(confId, organizerCode, participantCode, value, urlString);
    
    if (result == OS_SUCCESS)
    {
        Url bridgeUrl;
        getWellFormedURL(urlString, bridgeUrl);
        
        UtlString signature;
        
        if (accessCode.compareTo(organizerCode) == 0)
        {
            getAdmitSignature(value, bridgeUrl.toString(), contact, "organizer", signature);
            conferenceUrl = bridgeUrl.toString() + PARAMETER_SEPARATOR
                          + ORGANIZER_ROLE + PARAMETER_SEPARATOR
                          + ADMIT_SIGNATURE + signature;
            result = OS_SUCCESS;
        }
        else
        {
            if (accessCode.compareTo(participantCode) == 0)
            {
                getAdmitSignature(value, bridgeUrl.toString(), contact, "participant", signature);
                conferenceUrl = bridgeUrl.toString() + PARAMETER_SEPARATOR
                              + PARTICIPANT_ROLE + PARAMETER_SEPARATOR
                              + ADMIT_SIGNATURE + signature;
                result = OS_SUCCESS;
            }
            else
            {
                OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_WARNING,
                             "ConferenceManager::doLogin invalid access code %s for conference Id %s",
                             accessCode.data(), confId.data());
                result = OS_FAILED;
            }
        }
    }
                    
    return result;
}


OsStatus
ConferenceManager::getWellFormedURL (
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
ConferenceManager::convertUrlStringToXML ( UtlString& value )
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
ConferenceManager::convertXMLStringToURL ( UtlString& value )
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
ConferenceManager::getMediaserverURL (
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
        OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_ERR,
                  "ConferenceManager::getMediaserverURL Failed to get the mediaserver-url defined in cbadmission.xml");
    }

    return result;
}

OsStatus
ConferenceManager::getMediaserverSecureURL (
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
        OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_ERR,
                  "ConferenceManager::getMediaserverSecureURL Failed to get the mediaserver-url-secure defined in cbadmission.xml");
    }
    return result;
}


OsStatus
ConferenceManager::getIvrPromptURL (
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
        OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_ERR,
                  "ConferenceManager::getIvrPromptURL Failed to get the ivr-prompt-url defined in cbadmission.xml");
    }

    return result;
}


OsStatus
ConferenceManager::getResponseHeaders(int contentLength, UtlString& responseHeaders)
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
ConferenceManager::getCustomParameter( const UtlString& paramName,
                                       UtlString& rStrValue) const
{

    if( paramName == PARAM_LOG_LEVEL )
    {
            rStrValue = m_logLevel ;
    }

    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                  "ConferenceManager::getCustomParameter('%s') returns %d, rStrValue = '%s'",
                  paramName.data(), OS_SUCCESS, rStrValue.data());

    return OS_SUCCESS ;

}


OsStatus
ConferenceManager::parseConferenceFile (
        const UtlString& confId,
        UtlString& organizerCode,
        UtlString& participantCode,
        UtlString& secretValue,
        UtlString& bridgeUrl ) const
{
    OsStatus result = OS_FAILED;
    UtlString confFile(CONFERENCE_REGISTER_FILE);
    
    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                  "ConferenceManager::parseConferenceFile start to parse the file = %s\n", confFile.data());
    TiXmlDocument doc(confFile);

    // Verify that we can load the file (i.e it must exist)
    if(doc.LoadFile())
    {
        TiXmlNode * rootNode = doc.FirstChild ("conferences");
        if (rootNode != NULL)
        {
            // Search in each list
            for (TiXmlNode *conferenceNode = rootNode->FirstChild("conference");
                 conferenceNode; 
                 conferenceNode = conferenceNode->NextSibling("conference"))
            {
                // Compare the value of confId with the id in this conference
                if (confId.compareTo(((conferenceNode->FirstChild("id"))->FirstChild())->Value()) == 0 )
                {
                    secretValue = conferenceNode->FirstChild("value")->FirstChild()->Value();
                    organizerCode = conferenceNode->FirstChild("organizer-access-code")->FirstChild()->Value();
                    participantCode = conferenceNode->FirstChild("participant-access-code")->FirstChild()->Value();
                    bridgeUrl = conferenceNode->FirstChild("bridge-url")->FirstChild()->Value();
                    
                    OsSysLog::add(FAC_MEDIASERVER_CGI, PRI_DEBUG,
                                  "ConferenceManager::parseConferenceFile confId %s, organizer code %s, participant code %s, bridge url %s",
                                  confId.data(), organizerCode.data(), participantCode.data(), bridgeUrl.data());

                    return OS_SUCCESS;
                
                 }
            }
        }
    }

    return result;
}


OsStatus
ConferenceManager::getAdmitSignature (
        const UtlString& secretValue,
        const UtlString& conferenceAOR,
        const UtlString& contact,
        const UtlString& role,
        UtlString& signature) const
{
    OsStatus result = OS_SUCCESS;
    
    UtlString signatureSeed(secretValue);
    signatureSeed.append(conferenceAOR);
    signatureSeed.append(contact);
    signatureSeed.append(role);
    
    NetMd5Codec::encode(signatureSeed, signature);
    
    return result;

}
