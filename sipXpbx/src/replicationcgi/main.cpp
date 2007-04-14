// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <signal.h>
#include <iostream>

// APPLICATION INCLUDES
#include "os/OsDateTime.h"
#include "os/OsFS.h"
#include "os/OsFileSystem.h"
#include "os/OsStatus.h"
#include "os/OsServerTask.h"
#include "os/OsSysLog.h"
#include "net/Url.h"
#include "net/NetBase64Codec.h"

#include "xmlparser/tinyxml.h"
#include "sipdb/ResultSet.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/AliasDB.h"
#include "sipdb/CallerAliasDB.h"
#include "sipdb/PermissionDB.h"
#include "sipdb/CredentialDB.h"
#include "sipdb/RegistrationDB.h"
#include "sipdb/ExtensionDB.h"
#include "sipdb/AuthexceptionDB.h"


// DEFINES
#define WORKING_DIR     "."

#define REPLICATION_LOG_DIR         SIPX_LOGDIR
#define REPLICATION_LOG_FILENAME    "replicationcgi.log"
#define REPLICATION_DEFAULT_LOG_DIR SIPX_LOGDIR
#define LOG_FACILITY                FAC_REPLICATION_CGI

#define  URI            "uri"
#define  CONTACT        "contact"
#define  REALM          "realm"
#define  USERID         "userid"
#define  PASSTOKEN      "passtoken"
#define  PINTOKEN       "pintoken"
#define  AUTHTYPE       "authtype"
#define  IDENTITY       "identity"
#define  USER           "user"
#define  DOMAIN         "domain"

#define  CREDENTIAL     "credential"
#define  ALIAS          "alias"
#define  CALLER_ALIAS   "caller-alias"
#define  PERMISSION     "permission"
#define  EXTENSION      "extension"
#define  AUTHEXCEPTION  "authexception"

using namespace std ;

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef void (*sighandler_t)(int);

// FUNCTIONS
extern "C" {
    void  sigHandler( int sig_num );
    sighandler_t pt_signal( int sig_num, sighandler_t handler );
}

// FORWARD DECLARATIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
// GLOBAL VARIABLE INITIALIZATIONS
OsMutex*     gpLockMutex = new OsMutex(OsMutex::Q_FIFO);
UtlBoolean   gClosingIMDB = FALSE;

UtlString gstrError("");

/* ============================ FUNCTIONS ================================= */

/**
 * Description:
 * closes any open connections to the IMDB safely using a mutex lock
 * this procedure is slightly different for CGI's where there is 
 * no server task
 */
void
closeIMDBConnectionsFromCGI ()
{
    // Critical Section here
    OsLock lock( *gpLockMutex );

    if (!gClosingIMDB)
    {
        // prevent deadlock recursion from multiple
        // signals closing the IMDB
        gClosingIMDB = TRUE;
        // Ensure that this process calls close on the IMDB
        // this will only access the FastDB if it was opened 
        // by reference and tables were registers (It checks for 
        // pFastDB in its destructor and pFastDB is only created
        // or opened if a user requests a table
        delete SIPDBManager::getInstance();
    }
}

/**
 * Description:
 * This is a replacement for signal() which registers a signal handler but sets
 * a flag causing system calls ( namely read() or getchar() ) not to bail out
 * upon recepit of that signal. We need this behavior, so we must call
 * sigaction() manually.
 */
sighandler_t
pt_signal( int sig_num, sighandler_t handler)
{
#if defined(__pingtel_on_posix__)
    struct sigaction action[2];
    action[0].sa_handler = handler;
    sigemptyset(&action[0].sa_mask);
    action[0].sa_flags = 0;
    sigaction ( sig_num, &action[0], &action[1] );
    return action[1].sa_handler;
#else
    return signal( sig_num, handler );
#endif
}

/** 
 * Description: 
 * This is the signal handler, When called this sets the 
 * global gShutdownFlag allowing the main processing
 * loop to exit cleanly.
 */
void 
sigHandler( int sig_num )
{
    // set a global shutdown flag
//    gShutdownFlag = TRUE;

    // Unregister interest in the signal to prevent recursive callbacks
    pt_signal( sig_num, SIG_DFL );

    // Minimize the chance that we loose log data
    OsSysLog::flush();
    if (SIGTERM == sig_num)
    {
       OsSysLog::add( LOG_FACILITY, PRI_INFO, "sigHandler: terminate signal received.");
    }
    else
    {
       OsSysLog::add( LOG_FACILITY, PRI_CRIT, "sigHandler: caught signal: %d", sig_num );
    }
    OsSysLog::add( LOG_FACILITY, PRI_DEBUG, "sigHandler: closing IMDB connections" );
    OsSysLog::flush();

    closeIMDBConnectionsFromCGI(  );
}

OsStatus
getLogFilePath(UtlString& logFilePath)
{
   OsStatus result = OS_SUCCESS;
   OsPath path;
   UtlBoolean found = FALSE;
   UtlString candidate;

   // If the environment variable configured via SIPX_DB_VAR_PATH has a value,
   // try it.
   char* varpath = getenv(SIPX_DB_VAR_PATH);
   if (varpath != NULL && varpath[0] != '\0')
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "getLogFilePath env variable '%s' set to '%s'"
                    ,SIPX_DB_CFG_PATH, varpath);
      candidate = varpath;

      // If the last character is a separator, strip it.
      if (candidate(candidate.length() -1) == OsPath::separator)
      {
         candidate = candidate(0, candidate.length()-1);
      }
        
      if (OsFileSystem::exists(candidate + REPLICATION_LOG_DIR))
      {
         path = candidate + REPLICATION_LOG_DIR;
         found = TRUE;
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_ERR, "getLogFilePath env variable '%s' has value '%s' but is not valid"
                       ,SIPX_DB_CFG_PATH, varpath);
      }
   }

   // If that doesn't work, try the default location.
   if (!found)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG, "getLogFilePath trying default '%s'"
                    ,REPLICATION_DEFAULT_LOG_DIR);
      candidate = REPLICATION_DEFAULT_LOG_DIR;

      if (OsFileSystem::exists(candidate))
      {
         path = REPLICATION_DEFAULT_LOG_DIR;
         found = TRUE;
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_ERR, "getLogFilePath default '%s' is not valid"
                    ,REPLICATION_DEFAULT_LOG_DIR);
      }
   }

   // Otherwise, use the working directory.
   if (!found)
   {
      OsFileSystem::getWorkingDirectory(path);
   }

   // Now that we have the path to the logfile directory,
   // append the name of the log file.
   path += OsPath::separator + REPLICATION_LOG_FILENAME;

   // Finally translate path to native path,
   // this will ensure the volume is set correctly
   OsPath nativePath;
   path.getNativePath(nativePath);
   logFilePath = nativePath;

   return result;
}


void
getWorkingDirectory( UtlString& strWorkingDir ) {

   OsSysLog::add(FAC_REPLICATION_CGI,PRI_DEBUG, "before parsing encoded input char buffer" );

    char *etcEnv = getenv (SIPX_DB_CFG_PATH);

    if ( etcEnv != NULL )
    {
        if ( OsFileSystem::exists( (UtlString) etcEnv) )
        {
            strWorkingDir = (UtlString) etcEnv;
        }
    }
    else // Environment Variable is not defined check for ETC_DIR1
    {
       if ( OsFileSystem::exists( SIPX_DBDIR ) )
        {
           strWorkingDir = SIPX_CONFDIR;
        }
        else
        {
           strWorkingDir = WORKING_DIR;
        }
    }
}





OsStatus getAttributeValue (
            const TiXmlNode& node,
            const UtlString& key,
            UtlString& value )
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
            if( childNode && childNode->Type() == TiXmlNode::TEXT )
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
    return result;
}



/**
* inserts a row to a database of type "type".
*/
void
insertRow (const UtlHashMap& nvPairs, const UtlString& type)
{
    static UtlString uriKey           (URI);
    static UtlString extensionKey     (EXTENSION);
    static UtlString contactKey       (CONTACT);
    static UtlString realmKey         (REALM);
    static UtlString useridKey        (USERID);
    static UtlString passtokenKey     (PASSTOKEN);
    static UtlString pintokenKey      (PINTOKEN);
    static UtlString authtypeKey      (AUTHTYPE);
    static UtlString identityKey      (IDENTITY);
    static UtlString permissionKey    (PERMISSION);
    static UtlString userKey          (USER);
    static UtlString domainKey        (DOMAIN);
    static UtlString aliasKey         (ALIAS);

    if ( type.compareTo(CREDENTIAL , UtlString::ignoreCase)==0 ) {
       CredentialDB::getInstance()->
            insertRow (
                Url (*((UtlString*)nvPairs.findValue(&uriKey))),
                *((UtlString*)nvPairs.findValue(&realmKey)),
                *((UtlString*)nvPairs.findValue(&useridKey)),
                *((UtlString*)nvPairs.findValue(&passtokenKey)),
                *((UtlString*)nvPairs.findValue(&pintokenKey)),
                *((UtlString*)nvPairs.findValue(&authtypeKey)));
    } 
    
    else if ( type.compareTo(ALIAS , UtlString::ignoreCase)==0 ) {
        AliasDB::getInstance()->
            insertRow (
                Url( *((UtlString*)nvPairs.findValue(&identityKey))),
                Url( *((UtlString*)nvPairs.findValue(&contactKey))));
    } 
    
    else if ( type.compareTo(PERMISSION , UtlString::ignoreCase)==0 ) {
        PermissionDB::getInstance()->
            insertRow (
                Url( *((UtlString*)nvPairs.findValue(&identityKey))),
                *((UtlString*)nvPairs.findValue(&permissionKey)));
    }
    
    else if ( type.compareTo(EXTENSION , UtlString::ignoreCase)==0 )
    {
        ExtensionDB::getInstance()->
            insertRow(
                Url( *((UtlString*)nvPairs.findValue(&uriKey))),
                *((UtlString*)nvPairs.findValue(&extensionKey)));
    }
    
    else if ( type.compareTo(AUTHEXCEPTION , UtlString::ignoreCase)==0 ) {
        AuthexceptionDB::getInstance()->
            insertRow (
                 *((UtlString*)nvPairs.findValue(&userKey)));
    } 
    
    else if ( type.compareTo(CALLER_ALIAS , UtlString::ignoreCase)==0 ) {
       UtlString* optionalIdentity = dynamic_cast<UtlString*>(nvPairs.findValue(&identityKey));
       UtlString nullIdentity;
       OsSysLog::add(FAC_REPLICATION_CGI,PRI_DEBUG,"replication.cgi::insertRow caller-alias"
                     " identity '%s' domain '%s' alias '%s'",
                     optionalIdentity ? optionalIdentity->data() : nullIdentity.data(),
                     dynamic_cast<UtlString*>(nvPairs.findValue(&domainKey))->data(),
                     dynamic_cast<UtlString*>(nvPairs.findValue(&aliasKey))->data()
                     );

       CallerAliasDB::getInstance()->
          insertRow(optionalIdentity ? *optionalIdentity : nullIdentity,
                    *dynamic_cast<UtlString*>(nvPairs.findValue(&domainKey)),
                    *dynamic_cast<UtlString*>(nvPairs.findValue(&aliasKey))
                    );
    } 
    
    else {
       OsSysLog::add(FAC_REPLICATION_CGI,PRI_ERR,"replication.cgi::insertRow"
                     " unknown type '%s'",type.data());
    }
}

/**
* updates the database specified with the rows imported from the inputsource of
* xml paylaod file.
*/
OsStatus updateDB ( const char* pBuf, const UtlString& databaseName  )
{
   OsStatus result = OS_SUCCESS;
   TiXmlDocument doc;
   doc.Parse(pBuf);
   if( doc.Error())
   {
      result = OS_FAILED;
      gstrError.append(" couldn't load decoded xml file. Perhaps it is corrupted.");
   }

   if( result == OS_SUCCESS )
   {
      int iDataToUpdateDatabaseFound = 0;

      for( TiXmlNode *dbNode = doc.FirstChild( "items" );
           dbNode;
           dbNode = dbNode->NextSibling( "items" ) )
      {
          UtlString type = dbNode->ToElement()->Attribute( "type" );
          if( type == databaseName )
          {
             iDataToUpdateDatabaseFound = 1;

             for( TiXmlNode *itemNode = dbNode->FirstChild( "item" );
                  itemNode;
                  itemNode = itemNode->NextSibling( "item" ) )
             {
                 UtlHashMap nvPairs;

                 for( TiXmlNode *elementNode = itemNode->FirstChild();
                         elementNode;
                         elementNode = elementNode->NextSibling() )
                 {
                     // Bypass comments and other element types only interested
                     // in parsing element attributes
                     if ( elementNode->Type() == TiXmlNode::ELEMENT )
                     {
                         UtlString elementName = elementNode->Value();
                         UtlString elementValue;
                         result = getAttributeValue (*itemNode, elementName, elementValue);

                         if (result == OS_SUCCESS)
                         {
                             UtlString* collectableKey =
                                 new UtlString( elementName );
                             UtlString* collectableValue =
                                 new UtlString( elementValue );
                             nvPairs.insertKeyAndValue (
                                 collectableKey, collectableValue );

                         } else if ( elementNode->FirstChild() == NULL )
                         {
                             // Null Element value creaete a special
                             // char string we have key and value so insert
                             UtlString* collectableKey =
                                 new UtlString( elementName );
                             UtlString* collectableValue =
                                 new UtlString( SPECIAL_IMDB_NULL_VALUE );
                             nvPairs.insertKeyAndValue (
                                 collectableKey, collectableValue );

                         }
                     }
                 }
                 // Insert the item row into the IMDB
                 insertRow ( nvPairs, type );
            }
          }else
          {
             gstrError.append(" unexpected type '");
             gstrError.append(type);
             gstrError.append("' expected '");
             gstrError.append(databaseName);
             gstrError.append("'");
             result = OS_FAILED;
          }
          

         if( iDataToUpdateDatabaseFound == 0 )
         {
            gstrError.append(" payload row didn't contain data for  ");
             gstrError.append( databaseName.data() );
             gstrError.append(".\n");
             result = OS_FAILED;
         }
      }
   }
   return result;

}



/**
* updates the database specified with the rows imported from the xml paylaod file.
*/
void
updateDB ( const UtlString& importFileName, const UtlString& databaseName  )
{
    // Only parse the file if it already exists
    if ( OsFileSystem::exists( importFileName ) )
    {
       OsStatus rc;
       OsFile importFile (importFileName);
       importFile.open( OsFile::READ_ONLY );
       unsigned long length;

       if ( (rc = importFile.getLength( length )) == OS_SUCCESS )
       {
           char* pBuf =
               new char[length];
           unsigned long bytesRead;
           if ( (rc = importFile.read(pBuf, length, bytesRead)) == OS_SUCCESS )
           {
              updateDB( pBuf, databaseName );
           }
           delete pBuf;
       }
    }
}


/**
* Loads the resource map file and puts the name value pairs in a UtlHashMap specified.
* Make sure, there is a file called "resourcemap.xml" in the location specified by
* SIPXCHANGE_HOME. It should have resource mappings for the files and databases you are
* going to update. This is the format of "resourcemap.xml file.
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>

<!-- This file is located on the replication targets and maps
     between arbitary resource names and a physical location on
     the machine -->

<!DOCTYPE identifer_lookup_table [
   <!ELEMENT identifer_lookup_table (host*, component*) >

   <!ELEMENT host( filemap*, databasemap*)>
   <!ELEMENT component( filemap*, databasemap*)>

   <!ELEMENT filemap (EMPTY)>
   <!ATTLIST filemap identifier CDATA #REQUIRED>
   <!ATTLIST filemap location CDATA #REQUIRED>
   <!ELEMENT databasemap (EMPTY)>
   <!ATTLIST databasemap identifier CDATA #REQUIRED>
   <!ATTLIST databasemap location  CDATA #REQUIRED>

]>

<identifer_lookup_table>
   <host>
       <filemap identifier="user-config"      location="/var/local/user-config" />
       <filemap identifier="pinger-config"    location="/var/local/pinger-config" />
       <databasemap identifier="credentials"  location="credentials" />
   </host>

   <component id="MediaServer1">
       <filemap identifier="user-config"      location="/usr/local/user-config" />
       <filemap identifier="pinger-config"    location="/usr/local/pinger-config" />
       <databasemap identifier="credentials"  location="credentials" />
   </component>

   <component id="MediaServer2">
       <filemap identifier="user-config"   location="/usr/local/media_server2/user-config" />
       <filemap identifier="pinger-config" location="/usr/local/media_server2/pinger-config" />
   </component>


</identifer_lookup_table>

*/

void  loadResourceMap ( const UtlString& resourcemapFileName, UtlHashMap& rResourceMaps )
{
   // Only parse the file if it already exists
   UtlString strWorkingDirectory;
   getWorkingDirectory( strWorkingDirectory );
   /*
   OsPath workingPath("");
   OsFileSystem::getWorkingDirectory(workingPath);
   */
   UtlString fileToRead("");
   fileToRead.append( strWorkingDirectory );
   fileToRead.append( OsPathBase::separator );
   fileToRead.append( resourcemapFileName ) ;

   if ( OsFileSystem::exists(fileToRead) )
   {

    OsStatus rc;
    OsFile importFile (fileToRead);
    importFile.open( OsFile::READ_ONLY );
    unsigned long length;

    if ( (rc = importFile.getLength( length )) == OS_SUCCESS )
    {
        unsigned char* pBuf =
            new unsigned char[length];
        unsigned long bytesRead;
        if ( (rc = importFile.read(pBuf, length, bytesRead)) == OS_SUCCESS )
        {

           TiXmlDocument doc;
           doc.Parse( (const char*)pBuf );
           /////////////////
           TiXmlNode* pRootNode = doc.FirstChild ( "identifier_lookup_table" );
           if (pRootNode != NULL)
           {

               for( TiXmlNode *pComponentNode = pRootNode->FirstChild( "component" );
                    pComponentNode;
                    pComponentNode = pComponentNode->NextSibling( "component" ) )
               {
                  UtlString strComponentID = pComponentNode->ToElement()->Attribute( "id" );


                  UtlHashMap* pFilemapPairs = new UtlHashMap();

                  for( TiXmlNode *pFileMapNode = pComponentNode->FirstChild( "filemap" );
                     pFileMapNode;
                     pFileMapNode = pFileMapNode->NextSibling( "filemap" ) )
                  {
                     UtlString* pStrIdentifier =
                             new UtlString (pFileMapNode->ToElement()->Attribute("identifier"));

                     UtlString* pStrLocation =
                             new UtlString (pFileMapNode->ToElement()->Attribute("location"));

                     pFilemapPairs->insertKeyAndValue ( pStrIdentifier, pStrLocation );
                  }


                  UtlHashMap* pDatabaseMapPairs = new UtlHashMap();

                  for( TiXmlNode *pDatabaseMapNode = pComponentNode->FirstChild( "databasemap" );
                     pDatabaseMapNode;
                     pDatabaseMapNode = pDatabaseMapNode->NextSibling( "databasemap" ) )
                  {
                     UtlString* pStrIdentifier =
                             new UtlString (pDatabaseMapNode->ToElement()->Attribute("identifier"));

                     UtlString* pStrLocation =
                             new UtlString (pDatabaseMapNode->ToElement()->Attribute("location"));

                     pDatabaseMapPairs->insertKeyAndValue ( pStrIdentifier, pStrLocation );
                  }


                  rResourceMaps.insertKeyAndValue
                     (new UtlString (strComponentID+"_filemap"),  pFilemapPairs );
                  rResourceMaps.insertKeyAndValue
                     (new UtlString (strComponentID+"_databasemap"), pDatabaseMapPairs );
              }


              //now repeat the process for "host" elements
              ///////////////////////////////////////////////////////////

              for( TiXmlNode *pHostNode = pRootNode->FirstChild( "host" );
                    pHostNode;
                    pHostNode = pHostNode->NextSibling( "host" ) )
              {

                  UtlHashMap* pFilemapPairs = new UtlHashMap();

                  for( TiXmlNode *pFileMapNode = pHostNode->FirstChild( "filemap" );
                     pFileMapNode;
                     pFileMapNode = pFileMapNode->NextSibling( "filemap" ) )
                  {
                     UtlString* pStrIdentifier =
                             new UtlString (pFileMapNode->ToElement()->Attribute("identifier"));

                     UtlString* pStrLocation =
                             new UtlString (pFileMapNode->ToElement()->Attribute("location"));

                     pFilemapPairs->insertKeyAndValue ( pStrIdentifier, pStrLocation );
                  }


                  UtlHashMap* pDatabaseMapPairs = new UtlHashMap();

                  for( TiXmlNode *pDatabaseMapNode = pHostNode->FirstChild( "databasemap" );
                     pDatabaseMapNode;
                     pDatabaseMapNode = pDatabaseMapNode->NextSibling( "databasemap" ) )
                  {
                     UtlString* pStrIdentifier =
                             new UtlString (pDatabaseMapNode->ToElement()->Attribute("identifier"));

                     UtlString* pStrLocation =
                             new UtlString (pDatabaseMapNode->ToElement()->Attribute("location"));

                     pDatabaseMapPairs->insertKeyAndValue ( pStrIdentifier, pStrLocation );
                  }


                  rResourceMaps.insertKeyAndValue
                     (new UtlString ("host_filemap"),  pFilemapPairs );
                  rResourceMaps.insertKeyAndValue
                     (new UtlString ("host_databasemap"), pDatabaseMapPairs );
              }
           }
         }else
         {
               gstrError ="Error reading resourcemap.xml in the path specified by SIPXCHANGE_HOME \n";
         }
         delete []pBuf;

      }else
      {
           gstrError = "Error getting length of resourcemap.xml in the path specified by SIPXCHANGE_HOME \n";
      }

   }else
   {
        gstrError = "resourcemap.xml does not exist in the path [";
        gstrError.append(fileToRead.data());
        gstrError.append("] \n");

   }

}

              /////////////////



/**
* remove all rows from  the database specified.
*/

void cleanDatabase(const UtlString& databaseName){
    
   if ( databaseName == CREDENTIAL ){
        CredentialDB::getInstance()->removeAllRows();
    }
    
    else if ( databaseName == "registration" ){
        RegistrationDB::getInstance()->removeAllRows();
    }
    
    else if ( databaseName == ALIAS ){
        AliasDB::getInstance()->removeAllRows();
    }
    
    else if ( databaseName == PERMISSION ){
        PermissionDB::getInstance()->removeAllRows();
    }
    
    else if ( databaseName == EXTENSION ){
       ExtensionDB::getInstance()->removeAllRows();
    }
    
    else if ( databaseName == AUTHEXCEPTION ){
       AuthexceptionDB::getInstance()->removeAllRows();
    }

    else if ( databaseName == CALLER_ALIAS ){
       CallerAliasDB::getInstance()->removeAllRows();
    }
    
}

/*write the database's content to xml file
*/
void storeDatabase(const UtlString& databaseName){
    if ( databaseName == CREDENTIAL ){
        if ( CredentialDB::getInstance()->store() != OS_SUCCESS ){
             gstrError.append("Problem storing CredentialDB to local XML file");
        }
    }
    
    else if ( databaseName == ALIAS ){
        if ( AliasDB::getInstance()->store() != OS_SUCCESS ){
             gstrError.append("Problem storing AliasDB to local XML file");
        }
    }
    
    else if ( databaseName == PERMISSION ){
        if ( PermissionDB::getInstance()->store() != OS_SUCCESS ){
             gstrError.append("Problem storing permissionDB to local XML file");
        }
    }
    
    else if ( databaseName == EXTENSION ){
        if ( ExtensionDB::getInstance()->store() != OS_SUCCESS ){
             gstrError.append("Problem storing ExtensionDB to local XML file");
        }
    }
    
    else if ( databaseName == AUTHEXCEPTION ){
        if ( AuthexceptionDB::getInstance()->store() != OS_SUCCESS ){
             gstrError.append("Problem storing AuthexceptionDB to local XML file");
        }
    }

    else if ( databaseName == CALLER_ALIAS ){
       if ( CallerAliasDB::getInstance()->store() != OS_SUCCESS ){
          gstrError.append("Problem storing CallerAliasDB to local XML file");
       }
    }
    

}







// Print out some command line interface help
void usage()
{
    cout << "Uage: replication.cgi test <encodedfilename>" << endl;
}



/**
* handles input data  extracted from the inputsource of posted xml file.
* The payload can be of any format, from a plian txt file, xml file
* or a binary file. It basically updates the databases if the payload
* type is of database. Otherwise, if the payload type is file, it updates
* the files.  The xml file data should be in this format:
--------------------------------------------------------------------------------

<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
 <!-- This file proposes a XML format (envelope, really) for sending
 between the configuration server and replication target components.  Here,
 the XML allows you to send one or more different bundles. -->
 <!DOCTYPE replicationdata [
     <!ELEMENT replicationdata (data+) >

     <!-- since payload element is of type #CDATA, the value will not be
     parsed by the xml parser.  So, that value can be anything,
     from xml data to any other format of data( like user-config type data) -->
     <!ELEMENT data ( payload )>

     <!ATTLIST data type       ( database | file   ) #REQUIRED>
     <!ATTLIST data action     ( replace  | update ) #REQUIRED>
     <!ATTLIST data target_data_name CDATA #REQUIRED>

     <!-- if both component_id and component_type are not present,
          send it to all the components residing in the host -->

     <!--if component id is present, send this data to that
                                  particular component only -->
     <!ATTLIST data target_component_id CDATA #IMPLIED>

     <!--if component type is present but component_id is not present,
           send this data to all the components of this type -->
     <!ATTLIST data target_component_type (media-server | config-server | comm-server)  #IMPLIED>

     <!ELEMENT payload (#PCDATA) >
]>

<replicationdata>
    <!-- text clob data example -->
    <data type="database" action="replace" target_data_name="credentials"
          target_component_type="comm_server" target_component_id="CommServer1"  >

       <payload>PD94bWwgdmVyc2lvbj0iMS4wIiBlbmNvZGl</payload>

     </data>
</replicationdata>
--------------------------------------------------------------------------------
The payload data is Base64 encoded. Make sure you have no space in between payload
opening and closing tages.

*/
void handleInput(const char* pBuf )
{

   TiXmlDocument doc;
   OsSysLog::add(FAC_REPLICATION_CGI,PRI_DEBUG, "before parsing encoded input char buffer" );
   doc.Parse( pBuf );
   if( doc.Error())
   {
      gstrError.append("error occured while parsing encoded input char buffer.");
      return;
   }
   OsSysLog::add(FAC_REPLICATION_CGI,PRI_DEBUG, "after parsing encoded input char buffer" );
   

   UtlHashMap  resourceMaps;
   loadResourceMap("resourcemap.xml", resourceMaps);
   if( gstrError.length() > 0 )
   {
     //error while loading resourcemap, so exit.
     return;
   }

   UtlString strHostFileMap("host_filemap");
   UtlString strHostDatabaseMap("host_filemap");
   UtlHashMap*  pHostFileMaps =
     (UtlHashMap*)(resourceMaps.findValue(&strHostFileMap));
   UtlHashMap*  pHostDatabaseMaps =
     (UtlHashMap*)(resourceMaps.findValue(&strHostDatabaseMap));

   TiXmlNode* pRootNode = doc.FirstChild ( "replicationdata" );
   if (pRootNode != NULL)
   {
      for( TiXmlNode *pDataNode = pRootNode->FirstChild( "data" );
           pDataNode;
           pDataNode = pDataNode->NextSibling( "data" ) )
      {
         UtlString strTargetDataName = pDataNode->ToElement()->Attribute("target_data_name");
         UtlString strTargetDataType = pDataNode->ToElement()->Attribute("type");
         UtlString strComponentID    = pDataNode->ToElement()->Attribute("target_component_id");
         UtlString strComponentType  = pDataNode->ToElement()->Attribute("target_component_type");

         UtlString*  pStrTargetDataResourceMap ;
         UtlHashMap* pDictMapsToLookAt = NULL;

         if( strTargetDataType == "file" )
         {
             //if we need to use "host" resource map
             if( strComponentID.isNull() && strComponentType.isNull() )
             {
                 pDictMapsToLookAt = pHostFileMaps ;
             } else
             {
                 //only if component id is present
                 if( ! strComponentID.isNull() )
                 {
                     UtlString componentFileKey( strComponentID+"_filemap");
                     pDictMapsToLookAt =
                         (UtlHashMap*)(resourceMaps.findValue(&componentFileKey));
                 }
             }
         }else if( strTargetDataType == "database" )
         {
             if( strComponentID.isNull() && strComponentType.isNull() )
             {
                 pDictMapsToLookAt = pHostDatabaseMaps ;
             }else
             {
                 //only if component id is present
                 if( ! strComponentID.isNull() )
                 {
                     UtlString componentDatabaseKey( strComponentID+"_databasemap");
                     pDictMapsToLookAt  =
                         (UtlHashMap*)(resourceMaps.findValue(&componentDatabaseKey));
                 }
             }
         }

         if( pDictMapsToLookAt != NULL )
         {
             UtlString rwTargetDataName(strTargetDataName);
             pStrTargetDataResourceMap =
                (UtlString*)(pDictMapsToLookAt->findValue(&rwTargetDataName));
         }else{
             gstrError =  (" any resources for the passed component id not found in resourcemap.xml\n " );
             return;
         }

         TiXmlNode* pPayLoadNode = pDataNode->FirstChild("payload");
         if( pPayLoadNode != NULL )
         {

            TiXmlNode* pTextChildNode = pPayLoadNode->FirstChild();
            if( pTextChildNode != NULL )
            {
               const char*  pEncodedPayLoadData = pTextChildNode->Value();
               UtlString strPayLoadData(pEncodedPayLoadData);

               int iEncodedLength = strPayLoadData.length();

               int iDecodedLength =
                  NetBase64Codec::decodedSize(iEncodedLength, pEncodedPayLoadData);

               char* pDecodedPayLoadData = new char[iDecodedLength + 1];

               NetBase64Codec::decode(
                 iEncodedLength, pEncodedPayLoadData,
                 iDecodedLength, (char*)pDecodedPayLoadData);
                 
               // Added the termination character at the end of the string
               pDecodedPayLoadData[iDecodedLength] = '\0';

               if( pStrTargetDataResourceMap != NULL )
               {
                 UtlString mappedLocation(pStrTargetDataResourceMap->data());

                 if( strTargetDataType == "file" ){
                     //write to a file
                     OsStatus rc;
                     OsFile exportFile(mappedLocation);
                     if( exportFile.open( OsFile::CREATE ) == OS_SUCCESS){

                         unsigned long bytesRead;
                         if ( (rc = exportFile.write
                                     (pDecodedPayLoadData, iDecodedLength, bytesRead)) == OS_SUCCESS )
                         {
                             //good.
                             //UtlString errorMessageSubstr("successfully write to file ");


                         } else
                         {
                             UtlString errorMessageSubstr("couldn't write to file  ");
                             gstrError = (errorMessageSubstr + mappedLocation +"\n");
                         }
                     } else
                     {
                         UtlString errorMessageSubstr("couldn't open file ");
                         gstrError = (errorMessageSubstr + mappedLocation +"\n");
                     }
                 }else if( (strTargetDataType == "database") && (iDecodedLength > 0) )
                 {
                     //update databases.

                     OsSysLog::add(FAC_REPLICATION_CGI,PRI_DEBUG,
                           "before cleaning %s database", mappedLocation.data());

                     cleanDatabase(mappedLocation);

                     OsSysLog::add(FAC_REPLICATION_CGI,PRI_DEBUG,
                           "after cleaning %s database", mappedLocation.data());

                     // If the payload data exceeds 500 characters, only print
                     // the first 5,000 characters and follow them with "...".
                     OsSysLog::add(FAC_REPLICATION_CGI,PRI_DEBUG,
                                   "before updating %s database with '%.500s%s'",
                                   mappedLocation.data(), pDecodedPayLoadData,
                                   (iDecodedLength <= 500 ? "" : "..."));

                     // Load all rows from an external XML Script
                     OsStatus status = updateDB( pDecodedPayLoadData, mappedLocation );

                     OsSysLog::add(FAC_REPLICATION_CGI,PRI_DEBUG,
                           "after updating %s database", mappedLocation.data());


                     if( status== OS_SUCCESS ){
                         // Checkpoint each of the DBs
                        OsSysLog::add(FAC_REPLICATION_CGI,PRI_DEBUG,
                              "before storing %s database to an xml file", mappedLocation.data());

                        storeDatabase(mappedLocation);

                        OsSysLog::add(FAC_REPLICATION_CGI,PRI_INFO,
                                      "updated database: %s", mappedLocation.data());
                     }else
                     {
                        OsSysLog::add(FAC_REPLICATION_CGI,PRI_ERR,
                                      "failed to update database: %s", mappedLocation.data());
                     }
                 }else
                 {
                     gstrError = (" target data type is not understood\n");
                 }
               }else
               {
                  gstrError = (" target resource is not mapped for the component\n");
               }
         //delete[] decodedPayLoadData;
         }else
         {
            UtlString errorMessageSubstr("payload data was not found to replicate\n");
            gstrError = (errorMessageSubstr );
         }


        }
    }

  }

}

/**
* handles input data  extracted from the posted xml file.
* The payload can be of any format, from a plian txt file, xml file
* or a binary file. It basically updates the databases if the payload
* type is of database. Otherwise, if the payload type is file, it updates
* the files.
*/
void handleInput(const UtlString& importFileName)
{
   if ( OsFileSystem::exists( importFileName ) )
   {
      OsStatus rc;
      OsFile importFile (importFileName);
      importFile.open( OsFile::READ_ONLY );
      unsigned long length;

      if ( (rc = importFile.getLength( length )) == OS_SUCCESS )
      {
         char* pBuf = new char[length];
         unsigned long bytesRead;
         if ( (rc = importFile.read(pBuf, length, bytesRead)) == OS_SUCCESS )
         {
            handleInput(pBuf ) ;
         }
         delete []pBuf;
      }
   }
}




/**
* Main Entry point for CGI process. Rememeber, if you are
* trying to get an environment variable, it needs to be
* set in Apache ( or whatever http server you are using ).
* In Apache, in htpd.conf, you need to uncomment the line
*      LoadModule env_module modules/mod_env.so
* and set a environment variable like this:
*      SetEnv envkey envvalue
* Replication process depends on the in-memory databases.
* Those IMDBs look for a location to put persistent files.
* Hence you need to  setup environment varibale called
* SIPXCHANGE_HOME and set it to a directory where you like
* your persistent files to be.
*/
int 
main( int argc, char *argv[] )
{
   UtlString logFilePath;

   gstrError.remove(0);
   
   if ( getLogFilePath ( logFilePath ) == OS_SUCCESS )
   {
      // Initialize the logger.
      OsSysLog::initialize(1024, "replicationcgi" );
      OsSysLog::setOutputFile(0, logFilePath );
      OsSysLog::setLoggingPriorityForFacility(FAC_REPLICATION_CGI, PRI_DEBUG);
      OsSysLog::add(FAC_REPLICATION_CGI,PRI_INFO, "replication.cgi invoked");
      OsSysLog::flush();

      // Register Signal handlers to close IMDB
      pt_signal(SIGINT,   sigHandler);    // Trap Ctrl-C on NT
      pt_signal(SIGILL,   sigHandler); 
      pt_signal(SIGABRT,  sigHandler);    // Abort signal 6
      pt_signal(SIGFPE,   sigHandler);    // Floading Point Exception
      pt_signal(SIGSEGV,  sigHandler);    // Address access violations signal 11 
      pt_signal(SIGTERM,  sigHandler);    // Trap kill -15 on UNIX
#if defined(__pingtel_on_posix__)
      pt_signal(SIGHUP,   sigHandler);    // Hangup
      pt_signal(SIGQUIT,  sigHandler); 
      pt_signal(SIGPIPE,  SIG_IGN);    // Handle TCP Failure
      pt_signal(SIGBUS,   sigHandler); 
      pt_signal(SIGSYS,   sigHandler); 
      pt_signal(SIGXCPU,  sigHandler); 
      pt_signal(SIGXFSZ,  sigHandler);
      pt_signal(SIGUSR1,  sigHandler); 
      pt_signal(SIGUSR2,  sigHandler); 
#endif
      printf("Content-type: text/html \n\n");

      const char *pUserAgent = getenv("HTTP_USER_AGENT");
      const char *pRequestMethod = getenv("REQUEST_METHOD");

      if( pUserAgent != NULL)
      {
         const char *pContentLength = getenv("CONTENT_LENGTH");

         if( ( pRequestMethod != NULL ) && ( UtlString(pRequestMethod) != "POST" ) )
         {
            printf( "This cgi only works with POST data.\n" );
         } else
         {
            if( pContentLength != NULL )
            {
               int iContentlength = atoi(pContentLength);
               char* buff =  new  char[iContentlength];
               fread(buff, iContentlength, 1, stdin);


               UtlString responseData("");


               if( gstrError.length() == 0 )
               {
                  handleInput( buff );
               }
               if( gstrError.length() > 0 )
               {
                  //adding header called "ErrorInReplication"
                  //if there is any error.
                  responseData.append( gstrError);
                  OsSysLog::add(FAC_REPLICATION_CGI,PRI_ERR, gstrError.data());

               }else
               {
                   responseData.append( "replication was successful");
                   OsSysLog::add(FAC_REPLICATION_CGI,PRI_DEBUG, "replication was successful");

               }
               printf("%s", responseData.data());


               delete []buff;
            }
         }
      }
      else
      {

         //executing standalone needs encodedFile to be passed as second argument.
         printf(" This cgi is being executed from command line, not by a Http Server.\n\n " );
         // See if we have a specific configuration file to use
         if( argc > 2 )
         {
            if ( UtlString(argv[1]) == "test" )
            {
               UtlString responseData("");
               handleInput( UtlString(argv[2]));

               if( gstrError.length() > 0 )
               {
                  responseData.append( gstrError);
                  OsSysLog::add(FAC_REPLICATION_CGI,PRI_ERR, gstrError.data());

               }else
               {
                  responseData.append( "replication was successful");
                   OsSysLog::add(FAC_REPLICATION_CGI,PRI_DEBUG, "replication was successful");
               }
               printf("%s\n", responseData.data());
            }
         }else
         {
             usage();
         }
      }
      // now deregister this process's database references from the IMDB
      closeIMDBConnectionsFromCGI ();

      if (!gstrError.isNull())
      {
         OsSysLog::add(FAC_REPLICATION_CGI,PRI_ERR, "%s", gstrError.data());
      }else
      {
         OsSysLog::add(FAC_REPLICATION_CGI,PRI_DEBUG, "exited main() of replication.cgi");
      }
      OsSysLog::flush();
   }
   return 0;
}
