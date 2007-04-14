// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <iostream>

// APPLICATION INCLUDES
#include "utl/UtlInt.h"
#include "net/Url.h"
#include "net/NetMd5Codec.h"
#include "xmlparser/tinyxml.h"
#include "os/OsDateTime.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/ResultSet.h"
#include "sipdb/AliasDB.h"
#include "sipdb/HuntgroupDB.h"
#include "sipdb/ExtensionDB.h"
#include "sipdb/PermissionDB.h"
#include "sipdb/RegistrationDB.h"
#include "sipdb/SubscriptionDB.h"
#include "sipdb/DialByNameDB.h"
#include "sipdb/CredentialDB.h"
#include "sipdb/RegistrationDB.h"
#include "sipdb/SIPXAuthHelper.h"
#include "IMDBTaskMonitor.h"
#include "ImportTask.h"

extern UtlString componentKey;
extern UtlString uriKey;
extern UtlString extensionKey;
extern UtlString callidKey;
extern UtlString contactKey;
extern UtlString realmKey;
extern UtlString useridKey;
extern UtlString passtokenKey;
extern UtlString pintokenKey;
extern UtlString authtypeKey;
extern UtlString identityKey;
extern UtlString permissionKey;
extern UtlString qvalueKey;
extern UtlString expiresKey;
extern UtlString timenowKey;
extern UtlString subscribecseqKey;
extern UtlString eventtypeKey;
extern UtlString idKey;
extern UtlString toKey;
extern UtlString cseqKey;
extern UtlString fromKey;
extern UtlString keyKey;
extern UtlString recordrouteKey;
extern UtlString notifycseqKey;
extern UtlString acceptKey;
extern UtlString versionKey;
extern UtlString np_identityKey;
extern UtlString np_contactKey;
extern UtlString np_digitsKey;

using namespace std; 

ImportTask::ImportTask (
    const UtlString& rArgument, OsMsgQ& rMsgQ, OsEvent& rCommandEvent) :
    IMDBWorkerTask( rArgument, rMsgQ, rCommandEvent )
{}

ImportTask::~ImportTask()
{}

int
ImportTask::run( void* runArg )
{
    OsSysLog::add(LOG_FACILITY, PRI_DEBUG, "Starting Import Thread %s\n", mArgument.data());
    // Indicate that we're finished, the monitor thread
    // reads this flag and if it's still set
    setBusy (TRUE);

    importTableRows ( mArgument );

    UtlString databaseInfo;
    getDatabaseInfo ( databaseInfo );

    OsSysLog::add(LOG_FACILITY, PRI_DEBUG, databaseInfo.data());

    setBusy (FALSE);

    cleanupIMDBResources();

    OsSysLog::add(LOG_FACILITY, PRI_DEBUG, "Stopping Thread\n");

    // send a success message to the sleeping monitor
    notifyMonitor( USER_IMPORT_SUCCESS_EVENT );

    return( TRUE );
}

int
ImportTask::importTableRows ( const UtlString& rImportFilename ) const
{
    int exitCode = EXIT_SUCCESS;

    // Load all rows from an external XML Script
    if ( loadDB( rImportFilename ) == OS_SUCCESS )
    {
        cout << "Database loaded" << endl;

        // Checkpoint each of the DBs
        if ( CredentialDB::getInstance()->store() != OS_SUCCESS )
        {
            cout << "Problem storing CredentialDB to local XML file" << endl;
        }

        // Checkpoint each of the DBs
        if ( HuntgroupDB::getInstance()->store() != OS_SUCCESS )
        {
            cout << "Problem storing HuntgroupDB to local XML file" << endl;
        }

        if ( RegistrationDB::getInstance()->cleanAndPersist(0) != OS_SUCCESS )
        {
           cout << "Problem storing RegistrationDB to local XML file" << endl;
        }

        if ( AliasDB::getInstance()->store() != OS_SUCCESS )
        {
            cout << "Problem storing AliasDB to local XML file" << endl;
        }

        if ( PermissionDB::getInstance()->store() != OS_SUCCESS )
        {
            cout << "Problem storing AliasDB to local XML file" << endl;
        }

        if ( DialByNameDB::getInstance()->store() != OS_SUCCESS )
        {
            cout << "Problem storing DialByNameDB to local XML file" << endl;
        }

        if ( SubscriptionDB::getInstance()->store() != OS_SUCCESS )
        {
            cout << "Problem storing SubscriptionDB to local XML file" << endl;
        }

        if ( ExtensionDB::getInstance()->store() != OS_SUCCESS )
        {
            cout << "Problem storing ExtensionDB to local XML file" << endl;
        }
    } else
    {
        exitCode = EXIT_FILENOTFOUND;
        cout << "Could not import the XML file" << endl;
    }
    return exitCode;
}

// Populate the DB from the XML sections
OsStatus
ImportTask::loadDB( const UtlString& rFileName ) const
{
    // Critical Section here
    OsStatus result = OS_SUCCESS;

    TiXmlDocument doc ( rFileName );

    // Verify that we can load the file (i.e it must exist)
    if( doc.LoadFile() )
    {
        TiXmlNode *dbNode = doc.FirstChild( "items" );

        if ( dbNode )
          {
            UtlString tableName = dbNode->ToElement()->Attribute( "type" );

            for( ;
                 dbNode;
                 dbNode = dbNode->NextSibling( "items" ) )
            {
                // Clear out this table as we've got a new version in this file
                // Note dialbyname is an implied database
                if ( tableName.compareTo("alias" , UtlString::ignoreCase)==0 ) {
                    AliasDB::getInstance()->removeAllRows();
                } else if ( tableName.compareTo("credential"   , UtlString::ignoreCase)==0 ) {
                    CredentialDB::getInstance()->removeAllRows();
                } else if ( tableName.compareTo("extension"    , UtlString::ignoreCase)==0 ) {
                    ExtensionDB::getInstance()->removeAllRows();
                } else if ( tableName.compareTo("huntgroup"    , UtlString::ignoreCase)==0 ) {
                    HuntgroupDB::getInstance()->removeAllRows();
                } else if ( tableName.compareTo("permission"   , UtlString::ignoreCase)==0 ) {
                    PermissionDB::getInstance()->removeAllRows();
                } else if ( tableName.compareTo("registration" , UtlString::ignoreCase)==0 ) {
                    RegistrationDB::getInstance()->removeAllRows();
                } else if ( tableName.compareTo("subscription" , UtlString::ignoreCase)==0 ) {
                    SubscriptionDB::getInstance()->removeAllRows();
                }

                // the folder node contains at least the name/displayname/
                // and autodelete elements, it may contain others
                for( TiXmlNode *itemNode = dbNode->FirstChild( "item" );
                     itemNode;
                     itemNode = itemNode->NextSibling( "item" ) )
                {
                    // Create a hash dictionary for element attributes
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
                    insertRow ( nvPairs, tableName );
                }
            }
          }
        else
          {
            osPrintf( "No items element found\n");
            result = OS_FAILED;
          }
    }
    else
      {
        osPrintf( "Import load failed for '%s'\n", rFileName.data() );
        result = OS_FAILED;
      }

    return result;
}

OsStatus
ImportTask::getAttributeValue (
    const TiXmlNode& rNode,
    const UtlString& rKey,
    UtlString& value ) const
{
    OsStatus result = OS_SUCCESS;
    TiXmlNode* configNode = (TiXmlNode*)rNode.FirstChild( rKey );

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

void
ImportTask::insertRow (
    const UtlHashMap& rNVPairs,
    const UtlString& rTableName ) const
{
    OsSysLog::add(LOG_FACILITY, PRI_DEBUG, "ImportTask::insertRow %s"
                  ,rTableName.data()
                  );

    // check the table
    if ( rTableName.compareTo("credential" , UtlString::ignoreCase)==0 )
    {
        Url uri(*((UtlString*)rNVPairs.findValue(&uriKey)));
        UtlString realm     = *((UtlString*)rNVPairs.findValue(&realmKey));
        UtlString userid    = *((UtlString*)rNVPairs.findValue(&useridKey));
        UtlString authtype  = *((UtlString*)rNVPairs.findValue(&authtypeKey));
        UtlString passtoken = *((UtlString*)rNVPairs.findValue(&passtokenKey));

        if ( authtype.compareTo("NONE", UtlString::ignoreCase) == 0 )
        {
            UtlString passtokenPlain = passtoken;
            passtoken.remove(0);
            UtlString textToEncode = userid + ":" + realm + ":" + passtokenPlain;
            NetMd5Codec::encode(textToEncode, passtoken);
        }

        // if this is an old credentials file, there may not be a pintoken
        UtlString pintoken;
        if (rNVPairs.contains(&pintokenKey))
        {
           pintoken = *(UtlString*)rNVPairs.findValue(&pintokenKey);
        }
        else
        {
           // old file - duplicate the passtoken value for the pintoken
           pintoken = *(UtlString*)rNVPairs.findValue(&passtokenKey);
        }

        // Always create digest passtokens
        CredentialDB::getInstance()->
           insertRow ( uri, realm, userid, passtoken, pintoken, "DIGEST" );

    } else if ( rTableName.compareTo("huntgroup" , UtlString::ignoreCase)==0 )
    {
        HuntgroupDB::getInstance()->
            insertRow (
                Url( *((UtlString*)rNVPairs.findValue(&identityKey))));
    } else if ( rTableName.compareTo("registration" , UtlString::ignoreCase)==0 )
    {
        RegistrationDB::getInstance()->insertRow ( rNVPairs );
    } else if ( rTableName.compareTo("alias" , UtlString::ignoreCase)==0 )
    {
        AliasDB::getInstance()->
            insertRow (
                Url( *((UtlString*)rNVPairs.findValue(&identityKey))),
                Url( *((UtlString*)rNVPairs.findValue(&contactKey))));
    } else if ( rTableName.compareTo("permission" , UtlString::ignoreCase)==0 )
    {
        PermissionDB::getInstance()->
            insertRow (
                Url( *((UtlString*)rNVPairs.findValue(&identityKey))),
                *((UtlString*)rNVPairs.findValue(&permissionKey)));
    } else if ( rTableName.compareTo("subscription" , UtlString::ignoreCase)==0 )
    {
        int iNotifycseq = ((UtlInt*)rNVPairs.findValue(&notifycseqKey))->getValue();
        int iExpires = ((UtlInt*)rNVPairs.findValue(&expiresKey))->getValue();
        int iSubscribe = ((UtlInt*)rNVPairs.findValue(&subscribecseqKey))->getValue();
        int iVersion = ((UtlInt*)rNVPairs.findValue(&versionKey))->getValue();

        SubscriptionDB::getInstance()->
            insertRow(
                *((UtlString*)rNVPairs.findValue(&componentKey)),
                *((UtlString*)rNVPairs.findValue(&uriKey)),
                *((UtlString*)rNVPairs.findValue(&callidKey)),
                *((UtlString*)rNVPairs.findValue(&contactKey)),
                iExpires,
                iSubscribe,
                *((UtlString*)rNVPairs.findValue(&eventtypeKey)),
                *((UtlString*)rNVPairs.findValue(&idKey)),
                *((UtlString*)rNVPairs.findValue(&toKey)),
                *((UtlString*)rNVPairs.findValue(&fromKey)),
                *((UtlString*)rNVPairs.findValue(&keyKey)),
                *((UtlString*)rNVPairs.findValue(&recordrouteKey)),
                iNotifycseq,
                *((UtlString*)rNVPairs.findValue(&acceptKey)),
                iVersion) ;

    
    } else if ( rTableName.compareTo("extension" , UtlString::ignoreCase)==0 )
    {
        ExtensionDB::getInstance()->
            insertRow(
                Url( *((UtlString*)rNVPairs.findValue(&uriKey))),
                *((UtlString*)rNVPairs.findValue(&extensionKey)));
    } else
    {
        cout << "unknown database type: " << rTableName.data() << endl;
    }
}

