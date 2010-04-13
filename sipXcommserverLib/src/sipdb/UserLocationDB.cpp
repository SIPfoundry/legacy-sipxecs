// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
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
#include "xmlparser/tinyxml.h"
#include "fastdb/fastdb.h"
#include "net/Url.h"

#include "sipXecsService/SipXecsService.h"

#include "sipdb/SIPDBManager.h"
#include "sipdb/ResultSet.h"
#include "sipdb/UserLocationRow.h"
#include "sipdb/UserLocationDB.h"

REGISTER( UserLocationRow );

// Static Initializers
UserLocationDB* UserLocationDB::spInstance = NULL;
OsMutex  UserLocationDB::sLockMutex (OsMutex::Q_FIFO);
UtlString UserLocationDB::gIdentityKey("identity");
UtlString UserLocationDB::gLocationKey("location");

// The 'type' attribute of the top-level 'items' element.
const UtlString UserLocationDB::sType("userlocation");

// The XML namespace of the top-level 'items' element.
const UtlString UserLocationDB::sXmlNamespace("http://www.sipfoundry.org/sipX/schema/xml/userlocation-00-00");

/* ============================ CREATORS ================================== */

UserLocationDB::UserLocationDB( const UtlString& name )
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
                  "UserLocationDB::_ users = %d, mTableLoaded = %d",
                  users, mTableLoaded);
    if ( users == 1 || ( users > 1 && mTableLoaded == false ) )
    {
        OsSysLog::add(FAC_DB, PRI_DEBUG, "UserLocationDB::_ about to load");
        mTableLoaded = false;
        // Load the file implicitly
        if (this->load() == OS_SUCCESS)
        {
           mTableLoaded = true;
           OsSysLog::add(FAC_DB, PRI_DEBUG, "UserLocationDB::_ table successfully loaded");
        }
    }
    OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                  "UserLocationDB::_ rows in table = %d",
                  getRowCount());
}

UserLocationDB::~UserLocationDB() 
{
    OsSysLog::add(FAC_DB, PRI_DEBUG, "<><>## UserLocationDB:: DESTRUCTOR");
}

/* ============================ MANIPULATORS ============================== */

void
UserLocationDB::releaseInstance()
{
    OsSysLog::add(FAC_DB, PRI_DEBUG, "<><>## UserLocationDB:: releaseInstance() spInstance=%p", spInstance);

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
UserLocationDB::load()
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
        UtlString pathName = SipXecsService::Path(SipXecsService::DatabaseDirType,
                                                  fileName.data());

        OsSysLog::add(FAC_DB, PRI_DEBUG, "UserLocationDB::load loading \"%s\"",
                      pathName.data());

        TiXmlDocument doc ( pathName );

        // Verify that we can load the file (i.e it must exist)
        if( doc.LoadFile() )
        {
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
                    insertRow ( nvPairs );
                }
            }
        } else 
        {
            OsSysLog::add(FAC_SIP, PRI_WARNING, "UserLocationDB::load failed to load \"%s\"",
                          pathName.data());
            result = OS_FAILED;
        }
    } else 
    {
        OsSysLog::add(FAC_DB, PRI_ERR, "UserLocationDB::load failed - no DB");
        result = OS_FAILED;
    }
    return result;
}

int
UserLocationDB::getRowCount () const
{
   int count = 0;

   // Thread Local Storage
   m_pFastDB->attach();

   dbCursor< UserLocationRow > cursor;

   dbQuery query;
   query="";
   count = cursor.select( query );

   // Commit rows to memory - multiprocess workaround
   m_pFastDB->detach(0);

   return count;
}

OsStatus
UserLocationDB::store()
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
        dbCursor< UserLocationRow > cursor;

        // Select everything in the IMDB and add as item elements if present
        if ( cursor.select() > 0 )
        {
            // metadata contains column names
            dbTableDescriptor* pTableMetaData = &UserLocationRow::dbDescriptor;

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

UtlBoolean
UserLocationDB::insertRow (const UtlHashMap& nvPairs) 
{
    // Note we do not need the identity object here
    // as it is inferred from the uri
    return insertRow (
        Url( *(dynamic_cast <UtlString*> (nvPairs.findValue(&gIdentityKey))) ),
        *(dynamic_cast <UtlString*> (nvPairs.findValue(&gLocationKey))));
}

UtlBoolean
UserLocationDB::insertRow (
    const Url& identityUri,
    const UtlString& location )
{
    UtlBoolean result = FALSE;

    UtlString identityStr;
    identityUri.getIdentity( identityStr );

    // both the pk and fk's are stored in the db as identities
    // making for consistency, this is how they are stored in 
    // the credentials database and the uriIdentity is a FK to that db
    // so this step of extracting identies is crucial
    if ( !identityStr.isNull() && !location.isNull() && (m_pFastDB != NULL) )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        // Search for a matching row before deciding to update or insert
        dbCursor< UserLocationRow > cursor(dbCursorForUpdate);
        dbQuery query;

        // If the row already exists do nothing
        query="identity=",identityStr,"and location=",location;

        if ( cursor.select( query ) == 0 )
        {
            // Non existent row so insert
            UserLocationRow row;
            // Fill out the row columns
            row.identity = identityStr;
            row.location = location;
            insert( row );
        }

        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);

        // Successful return code
        result = TRUE;
    }
    // Most likely an arg problem
    return result;
}

UtlBoolean
UserLocationDB::removeRow ( const Url& identityUri, const UtlString& location )
{
    UtlBoolean rc = FALSE;
    UtlString identityStr;
    identityUri.getIdentity(identityStr);

    if ( !identityStr.isNull() && (m_pFastDB != NULL) )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        dbCursor< UserLocationRow > cursor(dbCursorForUpdate);

        dbQuery query;
        query="identity=",identityStr,"and location=",location;
        if ( cursor.select( query ) > 0 )
        {
            cursor.removeAllSelected();
            rc = TRUE;
        }
        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }
    return rc;
}

void
UserLocationDB::removeRows ( const Url& identityUri )
{
    UtlString identityStr;
    identityUri.getIdentity(identityStr);

    if ( !identityStr.isNull() && ( m_pFastDB != NULL ) )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        dbCursor< UserLocationRow > cursor(dbCursorForUpdate);
        dbQuery query;
        query="identity=",identityStr;

        if ( cursor.select( query ) > 0 )
        {
            cursor.removeAllSelected();
        }
        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }
    return;
}

void
UserLocationDB::removeAllRows ()
{
    if ( m_pFastDB != NULL ) 
    {
        // Thread Local Storage
        m_pFastDB->attach();
        dbCursor< UserLocationRow > cursor( dbCursorForUpdate );

        if (cursor.select() > 0)
        {
            cursor.removeAllSelected();
        }
        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    } 
}

void
UserLocationDB::getAllRows(ResultSet& rResultSet) const
{
    // Clear the results
    rResultSet.destroyAll();

    if ( m_pFastDB != NULL )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        dbCursor< UserLocationRow > cursor;
        if ( cursor.select() > 0 )
        {
            do {
                UtlHashMap record;
                UtlString* identityValue = 
                    new UtlString ( cursor->identity );
                UtlString* locationValue = 
                    new UtlString ( cursor->location );

                // Memory Leak fixes, make shallow copies of static keys
                UtlString* identityKey = new UtlString( gIdentityKey );
                UtlString* locationKey = new UtlString( gLocationKey );

                record.insertKeyAndValue ( 
                    identityKey, identityValue );
                record.insertKeyAndValue ( 
                      locationKey, locationValue );
                rResultSet.addValue(record);
            } while (cursor.next());
        }
        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }
}

void 
UserLocationDB::getIdentities (
    const UtlString& location,
    ResultSet& rResultSet ) const
{
    // This should erase the contents of the existing resultset
    rResultSet.destroyAll();

    if ( !location.isNull() && (m_pFastDB != NULL) )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        dbQuery query;
        query="location=", location;

        // Search to see if we have a Credential Row
        dbCursor< UserLocationRow > cursor;

        if ( cursor.select(query) > 0 )
        {
            do {
                UtlHashMap record;
                UtlString* identityValue = 
                    new UtlString ( cursor->identity );
                UtlString* locationValue = 
                    new UtlString ( cursor->location );

                // Memory Leak fixes, make shallow copies of static keys
                UtlString* identityKey = new UtlString( gIdentityKey );
                UtlString* locationKey = new UtlString( gLocationKey );

                record.insertKeyAndValue ( 
                    identityKey, identityValue );
                record.insertKeyAndValue ( 
                      locationKey, locationValue );

                rResultSet.addValue(record);
            } while ( cursor.next() );
        }
        // Commit the rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }
}

void 
UserLocationDB::getLocations (
    const UtlString& identityString,
    ResultSet& rResultSet ) const
{
    // This should erase the contents of the existing resultset
    rResultSet.clear();

    if ( !identityString.isNull() && ( m_pFastDB != NULL) )
    {
        // Thread Local Storage
        m_pFastDB->attach();
        dbQuery query;
        query="identity=",identityString;

        // Search to see if we have a Credential Row
        dbCursor< UserLocationRow > cursor;

        if ( cursor.select(query) > 0 )
        {
            do {
                UtlHashMap record;
                UtlString* identityValue = 
                    new UtlString ( cursor->identity );
                UtlString* locationValue = 
                    new UtlString ( cursor->location );

                // Memory Leak fixes, make shallow copies of static keys
                UtlString* identityKey = new UtlString( gIdentityKey );
                UtlString* locationKey = new UtlString( gLocationKey );

                record.insertKeyAndValue ( 
                    identityKey, identityValue );
                record.insertKeyAndValue ( 
                    locationKey, locationValue );

                rResultSet.addValue(record);
            } while ( cursor.next() );
        }
        // Commit the rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }
}

UtlBoolean 
UserLocationDB::hasLocation( const UtlString& identityString ) const
{
   ResultSet resultSet;
   getLocations( identityString, resultSet );
   return ( resultSet.getSize() > 0 );
}

bool
UserLocationDB::isLoaded()
{
   return mTableLoaded;
}

UserLocationDB*
UserLocationDB::getInstance( const UtlString& name )
{
    // Critical Section here
    OsLock lock( sLockMutex );

    // See if this is the first time through for this process
    // Note that this being null => pgDatabase is also null
    if ( spInstance == NULL )
    {
        // Create the singleton class for clients to use
        spInstance = new UserLocationDB( name );
    }
    return spInstance;
}
