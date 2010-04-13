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
#include "sipdb/PermissionRow.h"
#include "sipdb/PermissionDB.h"

REGISTER( PermissionRow );

// Static Initializers
PermissionDB* PermissionDB::spInstance = NULL;
OsMutex  PermissionDB::sLockMutex (OsMutex::Q_FIFO);
UtlString PermissionDB::gIdentityKey("identity");
UtlString PermissionDB::gPermissionKey("permission");

// The 'type' attribute of the top-level 'items' element.
const UtlString PermissionDB::sType("permission");

// The XML namespace of the top-level 'items' element.
const UtlString PermissionDB::sXmlNamespace("http://www.sipfoundry.org/sipX/schema/xml/permission-00-00");

/* ============================ CREATORS ================================== */

PermissionDB::PermissionDB( const UtlString& name )
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
                  "PermissionDB::_ users = %d, mTableLoaded = %d",
                  users, mTableLoaded);
    if ( users == 1 || ( users > 1 && mTableLoaded == false ) )
    {
        OsSysLog::add(FAC_DB, PRI_DEBUG, "PermissionDB::_ about to load");
        mTableLoaded = false;
        // Load the file implicitly
        if (this->load() == OS_SUCCESS)
        {
           mTableLoaded = true;
           OsSysLog::add(FAC_DB, PRI_DEBUG, "PermissionDB::_ table successfully loaded");
        }
    }
    OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                  "PermissionDB::_ rows in table = %d",
                  getRowCount());
}

PermissionDB::~PermissionDB()
{
    OsSysLog::add(FAC_DB, PRI_DEBUG, "<><>## PermissionDB:: DESTRUCTOR");
}

/* ============================ MANIPULATORS ============================== */

void
PermissionDB::releaseInstance()
{
    OsSysLog::add(FAC_DB, PRI_DEBUG, "<><>## PermissionDB:: releaseInstance() spInstance=%p", spInstance);

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
PermissionDB::load()
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

        OsSysLog::add(FAC_DB, PRI_DEBUG, "PermissionDB::load loading \"%s\"",
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
            OsSysLog::add(FAC_SIP, PRI_WARNING, "PermissionDB::load failed to load \"%s\"",
                          pathName.data());
            result = OS_FAILED;
        }
    } else
    {
        OsSysLog::add(FAC_DB, PRI_ERR, "PermissionDB::load failed - no DB");
        result = OS_FAILED;
    }
    return result;
}

int
PermissionDB::getRowCount () const
{
   int count = 0;

   // Thread Local Storage
   m_pFastDB->attach();

   dbCursor< PermissionRow > cursor;

   dbQuery query;
   query="";
   count = cursor.select( query );

   // Commit rows to memory - multiprocess workaround
   m_pFastDB->detach(0);

   return count;
}

OsStatus
PermissionDB::store()
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
        dbCursor< PermissionRow > cursor;

        // Select everything in the IMDB and add as item elements if present
        if ( cursor.select() > 0 )
        {
            // metadata contains column names
            dbTableDescriptor* pTableMetaData = &PermissionRow::dbDescriptor;

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
PermissionDB::insertRow (const UtlHashMap& nvPairs)
{
    // Note we do not need the identity object here
    // as it is inferred from the uri
    return insertRow (
        Url( *(dynamic_cast <UtlString*> (nvPairs.findValue(&gIdentityKey))) ),
        *(dynamic_cast <UtlString*> (nvPairs.findValue(&gPermissionKey))));
}

UtlBoolean
PermissionDB::insertRow (
    const Url& identity,
    const UtlString& permission )
{
    UtlBoolean result = FALSE;

    UtlString identityStr;
    identity.getIdentity( identityStr );

    // both the pk and fk's are stored in the db as identities
    // making for consistency, this is how they are stored in
    // the credentials database and the uriIdentity is a FK to that db
    // so this step of extracting identies is crucial
    if ( !identityStr.isNull() && !permission.isNull() && (m_pFastDB != NULL) )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        // Search for a matching row before deciding to update or insert
        dbCursor< PermissionRow > cursor(dbCursorForUpdate);
        dbQuery query;

        // If the row already exists do nothing
        query="identity=",identityStr,"and permission=",permission;

        if ( cursor.select( query ) == 0 )
        {
            // Non existent row so insert
            PermissionRow row;
            // Fill out the row columns
            row.identity = identityStr;
            row.permission = permission;
            insert (row);
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
PermissionDB::removeRow ( const Url& identity, const UtlString& permission )
{
    UtlBoolean rc = FALSE;
    UtlString identityStr;
    identity.getIdentity(identityStr);

    if ( !identityStr.isNull() && (m_pFastDB != NULL) )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        dbCursor< PermissionRow > cursor(dbCursorForUpdate);

        dbQuery query;
        query="identity=",identityStr,"and permission=",permission;
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
PermissionDB::removeRows ( const Url& identity )
{
    UtlString identityStr;
    identity.getIdentity(identityStr);

    if ( !identityStr.isNull() && ( m_pFastDB != NULL ) )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        dbCursor< PermissionRow > cursor(dbCursorForUpdate);
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
PermissionDB::removeAllRows ()
{
    if ( m_pFastDB != NULL )
    {
        // Thread Local Storage
        m_pFastDB->attach();
        dbCursor< PermissionRow > cursor( dbCursorForUpdate );

        if (cursor.select() > 0)
        {
            cursor.removeAllSelected();
        }
        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }
}

void
PermissionDB::getAllRows(ResultSet& rResultSet) const
{
    // Clear the results
    rResultSet.destroyAll();

    if ( m_pFastDB != NULL )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        dbCursor< PermissionRow > cursor;
        if ( cursor.select() > 0 )
        {
            do {
                UtlHashMap record;
                UtlString* identityValue =
                    new UtlString ( cursor->identity );
                UtlString* permissionValue =
                    new UtlString ( cursor->permission );

                // Memory Leak fixes, make shallow copies of static keys
                UtlString* identityKey = new UtlString( gIdentityKey );
                UtlString* permissionKey = new UtlString( gPermissionKey );

                record.insertKeyAndValue (
                    identityKey, identityValue );
                record.insertKeyAndValue (
                    permissionKey, permissionValue );
                rResultSet.addValue(record);
            } while (cursor.next());
        }
        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }
}

void
PermissionDB::getIdentities (
    const UtlString& permission,
    ResultSet& rResultSet ) const
{
    // This should erase the contents of the existing resultset
    rResultSet.destroyAll();

    if ( !permission.isNull() && (m_pFastDB != NULL) )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        dbQuery query;

        // Primary Key is the uriPermission's identity
        query="permission=", permission;

        // Search to see if we have a Credential Row
        dbCursor< PermissionRow > cursor;

        if ( cursor.select(query) > 0 )
        {
            do {
                UtlHashMap record;
                UtlString* identityValue =
                    new UtlString ( cursor->identity );
                UtlString* permissionValue =
                    new UtlString ( cursor->permission );

                // Memory Leak fixes, make shallow copies of static keys
                UtlString* identityKey = new UtlString( gIdentityKey );
                UtlString* permissionKey = new UtlString( gPermissionKey );

                record.insertKeyAndValue (
                    identityKey, identityValue );
                record.insertKeyAndValue (
                    permissionKey, permissionValue );

                rResultSet.addValue(record);
            } while ( cursor.next() );
        }
        // Commit the rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }
}

void
PermissionDB::getPermissions (
    const Url& identity,
    ResultSet& rResultSet ) const
{
    UtlString identityStr;
    identity.getIdentity(identityStr);

    // This should erase the contents of the existing resultset
    rResultSet.clear();

    if ( !identityStr.isNull() && ( m_pFastDB != NULL) )
    {
        // Thread Local Storage
        m_pFastDB->attach();
        dbQuery query;

        // Primary Key is the uriPermission's identity
        query="identity=",identityStr;

        // Search to see if we have a Credential Row
        dbCursor< PermissionRow > cursor;

        if ( cursor.select(query) > 0 )
        {
            do {
                UtlHashMap record;
                UtlString* identityValue =
                    new UtlString ( cursor->identity );
                UtlString* permissionValue =
                    new UtlString ( cursor->permission );

                // Memory Leak fixes, make shallow copies of static keys
                UtlString* identityKey = new UtlString( gIdentityKey );
                UtlString* permissionKey = new UtlString( gPermissionKey );

                record.insertKeyAndValue (
                    identityKey, identityValue );
                record.insertKeyAndValue (
                    permissionKey, permissionValue );

                rResultSet.addValue(record);
            } while ( cursor.next() );
        }
        // Commit the rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }
}

UtlBoolean
PermissionDB::hasPermission (
    const Url& identity,
    const UtlString& permission ) const
{
    UtlBoolean hasPermission = FALSE;
    UtlString identityStr;
    identity.getIdentity(identityStr);

    if ( !permission.isNull() && (m_pFastDB != NULL) )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        dbQuery query;

        // Primary Key is the uriPermission's identity
        query="identity=",identityStr,"and permission=", permission;


        // Search to see if we have a Credential Row
        dbCursor< PermissionRow > cursor;

        if ( cursor.select(query) > 0 )
        {
            hasPermission = TRUE;
        }
        // Commit the rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }
    return hasPermission;
}

bool
PermissionDB::isLoaded()
{
   return mTableLoaded;
}

PermissionDB*
PermissionDB::getInstance( const UtlString& name )
{
    // Critical Section here
    OsLock lock( sLockMutex );

    // See if this is the first time through for this process
    // Note that this being null => pgDatabase is also null
    if ( spInstance == NULL )
    {
        // Create the singleton class for clients to use
        spInstance = new PermissionDB( name );
    }
    return spInstance;
}
