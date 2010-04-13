// 
// 
// Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "os/OsLock.h"
#include "os/OsDateTime.h"
#include "os/OsFS.h"
#include "os/OsSysLog.h"
#include "net/Url.h"

#include "fastdb/fastdb.h"
#include "xmlparser/tinyxml.h"
#include "sipXecsService/SipXecsService.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/ResultSet.h"
#include "sipdb/UserForwardRow.h"
#include "sipdb/UserForwardDB.h"

REGISTER( UserForwardRow );

// STATIC INITIALIZERS
UserForwardDB* UserForwardDB::spInstance = NULL;
OsMutex  UserForwardDB::sLockMutex (OsMutex::Q_FIFO);
UtlString UserForwardDB::gIdentityKey("identity");
UtlString UserForwardDB::gCfwdtimeKey("cfwdtime");

// The 'type' attribute of the top-level 'items' element.
const UtlString UserForwardDB::sType("userforward");

// The XML namespace of the top-level 'items' element.
const UtlString UserForwardDB::sXmlNamespace("http://www.sipfoundry.org/sipX/schema/xml/userforward-00-00");

/* ============================ CREATORS ================================== */

UserForwardDB::UserForwardDB( const UtlString& name )
: mDatabaseName( name )
, mTableLoaded ( true )
{
    // Access the shared table databse
    SIPDBManager* pSIPDBManager = SIPDBManager::getInstance();
    m_pFastDB = pSIPDBManager->getDatabase(name);

    // If we are the first process to attach
    // then we need to load the DB
    int users = pSIPDBManager->getNumDatabaseProcesses(name);
    OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                  "UserForwardDB::_ users = %d, mTableLoaded = %d",
                  users, mTableLoaded);
    if ( users == 1 || ( users > 1 && mTableLoaded == false ) )
    {
        OsSysLog::add(FAC_DB, PRI_DEBUG, "UserForwardDB::_ about to load");
        mTableLoaded = false;
        // Load the file implicitly
        if ( this->load() == OS_SUCCESS )
        {
          mTableLoaded = true;
          OsSysLog::add(FAC_DB, PRI_DEBUG, "UserForwardDB::_ table successfully loaded");
        }
    }
    OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                  "UserForwardDB::_ rows in table = %d",
                  getRowCount());
}

UserForwardDB::~UserForwardDB()
{
    OsSysLog::add(FAC_DB, PRI_DEBUG, "<><>## UserForwardDB:: DESTRUCTOR");
}

/* ============================ MANIPULATORS ============================== */

void
UserForwardDB::releaseInstance()
{
    OsSysLog::add(FAC_DB, PRI_DEBUG, "<><>## UserForwardDB:: releaseInstance() spInstance=%p", spInstance);

    // Critical Section here
    OsLock lock( sLockMutex );

    // if it exists, delete the object and NULL out the pointer
    if (spInstance != NULL) {

        // unregister this table/process from the IMDB
        SIPDBManager::getInstance()->removeDatabase ( spInstance->mDatabaseName );

        // NULL out the fastDB pointer also
        spInstance->m_pFastDB = NULL;

        delete spInstance;
        spInstance = NULL;
    }
}

OsStatus
UserForwardDB::load()
{
    // Critical Section here
    OsLock lock( sLockMutex );
    OsStatus result = OS_SUCCESS;

    if ( m_pFastDB != NULL ) 
    {
        // Clean out the existing DB rows before loading
        // a new set from persistent storage
        removeAllRows ();

        UtlString fileName = mDatabaseName + ".xml";
        OsPath pathName = SipXecsService::Path(SipXecsService::DatabaseDirType,fileName.data());

        OsSysLog::add(FAC_DB, PRI_DEBUG, "UserForwardDB::load loading \"%s\"",
                    pathName.data());

        TiXmlDocument doc ( pathName );

        // Verify that we can load the file (i.e it must exist)
        if( doc.LoadFile() )
        {
            // the checksum is used to determine if the db changed between reloads
            int loadChecksum = 0;
            TiXmlNode * rootNode = doc.FirstChild ("items");
            if (rootNode != NULL)
            {
                // the folder node contains at least the name/displayname/
                // and autodelete elements, it may contain others
                for( TiXmlNode *itemNode = rootNode->FirstChild( "item" );
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
                            result = SIPDBManager::getAttributeValue (
                                *itemNode, elementName, elementValue);
                            // update the load checksum
                            loadChecksum += ( elementName.hash() + elementValue.hash() );
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
                                // NULL Element value create a special 
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
                    insertRow ( nvPairs );
                }
            }
        } else 
        {
            OsSysLog::add(FAC_DB, PRI_WARNING, "UserForwardDB::load failed to load \"%s\"",
                    pathName.data());
            result = OS_FAILED;
        }
    } else 
    {
        OsSysLog::add(FAC_DB, PRI_ERR, "UserForwardDB::load failed - no DB");
        result = OS_FAILED;
    }
    return result;
}

OsStatus
UserForwardDB::store()
{
    // Critical Section here
    OsLock lock( sLockMutex );
    OsStatus result = OS_SUCCESS;

    if ( m_pFastDB != NULL ) 
    {
        UtlString fileName = mDatabaseName + ".xml";
        UtlString pathName = SipXecsService::Path(SipXecsService::DatabaseDirType,
                                                  fileName.data());

        // Create an empty document
        TiXmlDocument document;

        // Create a hard coded standalone declaration section
        document.Parse ("<?xml version=\"1.0\" standalone=\"yes\"?>");

        // Create the root node container
        TiXmlElement itemsElement ( "items" );
        itemsElement.SetAttribute( "type", sType.data() );
        itemsElement.SetAttribute( "xmlns", sXmlNamespace.data() );

        // Thread Local Storage
        m_pFastDB->attach();

        // Search our memory for rows
        dbCursor< UserForwardRow > cursor;

        // Select everything in the IMDB and add as item elements if present
        if ( cursor.select() > 0 )
        {
            // metadata contains column names
            dbTableDescriptor* pTableMetaData = &UserForwardRow::dbDescriptor;

            do {
                // Create an item container
                TiXmlElement itemElement ("item");

                byte* base = (byte*)cursor.get();

                // Add the column name value pairs
                for ( dbFieldDescriptor* fd = pTableMetaData->getFirstField();
                      fd != NULL; fd = fd->nextField ) 
                {
                    // if the column name does not contain the 
                    // np_prefix we must_presist it
                    if ( strstr( fd->name, "np_" ) == NULL )
                    {
                        // Create the a column element named after the IMDB column name
                        TiXmlElement element (fd->name );

                        // See if the IMDB has the predefined SPECIAL_NULL_VALUE
                        UtlString textValue;
                        SIPDBManager::getFieldValue(base, fd, textValue);

                        // If the value is not null append a text child element
                        if ( textValue != SPECIAL_IMDB_NULL_VALUE ) 
                        {
                            // Text type assumed here... @todo change this
                            TiXmlText value ( textValue.data() );
                            // Store the column value in the element making this
                            // <colname>coltextvalue</colname>
                            element.InsertEndChild  ( value );
                        }

                        // Store this in the item tag as follows
                        // <item>
                        // .. <col1name>col1textvalue</col1name>
                        // .. <col2name>col2textvalue</col2name>
                        // .... etc
                        itemElement.InsertEndChild  ( element );
                    }
                }
                // add the line to the element
                itemsElement.InsertEndChild ( itemElement );
            } while ( cursor.next() );
        }  

        // Attach the root node to the document
        document.InsertEndChild ( itemsElement );
        document.SaveFile ( pathName );

        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
        mTableLoaded = true;
    } else 
    {
         result = OS_FAILED;
    }
    return result;
}

int
UserForwardDB::getRowCount () const
{
   int count = 0;

   // Thread Local Storage
   m_pFastDB->attach();

   dbCursor< UserForwardRow > cursor;

   dbQuery query;
   query="";
   count = cursor.select( query );

   // Commit rows to memory - multiprocess workaround
   m_pFastDB->detach(0);

   return count;
}

UtlBoolean
UserForwardDB::insertRow (const UtlHashMap& nvPairs) 
{
    // Note we do not need the identity object here
    // as it is inferred from the uri
    UtlString identity, cfwdtime;
    identity = *(dynamic_cast <UtlString*> (nvPairs.findValue(&gIdentityKey)));
    cfwdtime = *(dynamic_cast <UtlString*> (nvPairs.findValue(&gCfwdtimeKey)));
    return insertRow ( Url( identity ), cfwdtime );
}

UtlBoolean
UserForwardDB::insertRow (
    const Url& identity,
    const UtlString& cfwdTime )
{
    UtlBoolean result = FALSE;
    UtlString identityStr;
    identity.getIdentity(identityStr);

    // both the pk and fk's are stored in the db as identities
    // making for consistency, this is how they are stored in 
    // the user forward  database and the uriIdentity is a FK to that db
    // so this step of extracting identies is crucial
    if ( !identityStr.isNull() && !cfwdTime.isNull() && (m_pFastDB != NULL) )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        // Search for a matching row before deciding to update or insert
        dbCursor< UserForwardRow > cursor(dbCursorForUpdate);

        UserForwardRow row;
        {
            // Fill out the row columns
            row.identity = identityStr;
            row.cfwdtime = cfwdTime;
            insert (row);
        }

        result = TRUE;
        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);

        // Table Data changed
        SIPDBManager::getInstance()->
            setDatabaseChangedFlag(mDatabaseName, TRUE);
    }
    return result;
}

UtlBoolean
UserForwardDB::removeRow ( const Url& identity )
{
    UtlBoolean removed = FALSE;
    UtlString identityStr;
    identity.getIdentity(identityStr);

    if ( !identityStr.isNull() && ( m_pFastDB != NULL ) )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        dbCursor< UserForwardRow > cursor(dbCursorForUpdate);

        dbQuery query;
        query="identity=",identityStr;
        if ( cursor.select( query ) > 0 )
        {
            cursor.removeAllSelected();
            removed = TRUE;
        }
        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);

        // Table Data changed
        SIPDBManager::getInstance()->
            setDatabaseChangedFlag(mDatabaseName, TRUE);
    }
    return removed;
}

void
UserForwardDB::removeAllRows ()
{
    // Thread Local Storage
    if (m_pFastDB != NULL) 
    {
        m_pFastDB->attach();

        dbCursor< UserForwardRow > cursor(dbCursorForUpdate);

        if (cursor.select() > 0)
        {
            cursor.removeAllSelected();
        }
        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);

        // Table Data changed
        SIPDBManager::getInstance()->
            setDatabaseChangedFlag(mDatabaseName, TRUE);
    }
}

void
UserForwardDB::getAllRows(ResultSet& rResultSet) const
{
    // Clear the out any previous records
    rResultSet.destroyAll();

    if (m_pFastDB != NULL)
    {
        // must do this first to ensure process/tls integrity
        m_pFastDB->attach();

        dbCursor< UserForwardRow > cursor;
        if ( cursor.select() > 0 )
        {
            do {
                UtlHashMap record;
                UtlString* identityValue = 
                    new UtlString ( cursor->identity );
                UtlString* cfwdtimeValue = 
                    new UtlString ( cursor->cfwdtime );

                // Memory Leak fixes, make shallow copies of static keys
                UtlString* identityKey = new UtlString( gIdentityKey );
                UtlString* cfwdtimeKey = new UtlString( gCfwdtimeKey );

                record.insertKeyAndValue ( 
                    identityKey, identityValue );
                record.insertKeyAndValue ( 
                    cfwdtimeKey, cfwdtimeValue );

                rResultSet.addValue(record);
            } while (cursor.next());
        }
        // commit rows and also ensure process/tls integrity
        m_pFastDB->detach(0);
    }
}

UtlBoolean
UserForwardDB::getCfwdTime (
    const Url& identity,
    UtlString& cfwdtime ) const
{
    UtlString identityStr;
    identity.getIdentity(identityStr);

    UtlBoolean found = FALSE;

    if ( !identityStr.isNull() && (m_pFastDB != NULL) )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        // Match a all rows where the contact identity matches
        dbQuery query;
        query="identity=",identityStr;

        // Search to see if we have a User Forward Row
        dbCursor< UserForwardRow > cursor;

        if ( cursor.select(query) > 0 )
        {
           cfwdtime = cursor->cfwdtime;
           found = TRUE;
        }

        m_pFastDB->detach(0);
    }

    return found;
}

bool 
UserForwardDB::isLoaded()
{
  return mTableLoaded;
}

UserForwardDB*
UserForwardDB::getInstance( const UtlString& name )
{
    // Critical Section here
    OsLock lock( sLockMutex );

    // See if this is the first time through for this process
    // Note that this being null => pgDatabase is also null
    if ( spInstance == NULL )
    {
        // Create the singleton class for clients to use
        spInstance = new UserForwardDB( name );
    }
    return spInstance;
}


