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
#include "os/OsLock.h"
#include "os/OsDateTime.h"
#include "os/OsFS.h"
#include "os/OsSysLog.h"
#include "net/Url.h"

#include "xmlparser/tinyxml.h"
#include "fastdb/fastdb.h"
#include "sipdb/ResultSet.h"
#include "sipXecsService/SipXecsService.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/CredentialRow.h"
#include "sipdb/CredentialDB.h"

// DEFINES
REGISTER( CredentialRow );

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

// STATIC INITIALIZERS
CredentialDB* CredentialDB::spInstance = NULL;
OsMutex       CredentialDB::sLockMutex (OsMutex::Q_FIFO);

// GLOBAL VARIABLES
UtlString CredentialDB::gUriKey("uri");
UtlString CredentialDB::gRealmKey("realm");
UtlString CredentialDB::gUseridKey("userid");
UtlString CredentialDB::gPasstokenKey("passtoken");
UtlString CredentialDB::gPintokenKey("pintoken");
UtlString CredentialDB::gAuthtypeKey("authtype");

// The 'type' attribute of the top-level 'items' element.
const UtlString CredentialDB::sType("credential");

// The XML namespace of the top-level 'items' element.
const UtlString CredentialDB::sXmlNamespace("http://www.sipfoundry.org/sipX/schema/xml/credential-01-00");

/* ============================ CREATORS ================================== */

CredentialDB::CredentialDB( const UtlString& name )
: mDatabaseName( name )
, mTableLoaded ( true )
{
    // Access the shared table database
    SIPDBManager* pSIPDBManager = SIPDBManager::getInstance();
    m_pFastDB = pSIPDBManager->getDatabase( name );

    // If we are the first process to attach
    // then we need to load the DB
    int numusers = pSIPDBManager->getNumDatabaseProcesses (name);
    OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                  "CredentialDB::_ numusers = %d, mTableLoaded = %d",
                  numusers, mTableLoaded);
    if ( numusers == 1 || ( numusers > 1 && mTableLoaded == false ) )
    {
        OsSysLog::add(FAC_DB, PRI_DEBUG, "CredentialDB::_ about to load");
        mTableLoaded = false;
        // Load the file implicitly
        if (this->load() == OS_SUCCESS)
        {
           mTableLoaded = true;
           OsSysLog::add(FAC_DB, PRI_DEBUG, "CredentialDB::_ table successfully loaded");
        }
    }
    OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                  "CredentialDB::_ rows in table = %d",
                  getRowCount());
}

CredentialDB::~CredentialDB()
{
    OsSysLog::add(FAC_DB, PRI_DEBUG, "<><>## CredentialDB:: DESTRUCTOR");
}

/* ============================ MANIPULATORS ============================== */

void
CredentialDB::releaseInstance()
{
    OsSysLog::add(FAC_DB, PRI_DEBUG, "<><>## CredentialDB:: releaseInstance() spInstance=%p", spInstance);

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
CredentialDB::load()
{
    // Critical Section here
    OsLock lock( sLockMutex );
    OsStatus result = OS_SUCCESS;

    if ( m_pFastDB != NULL )
    {
        // Clean out the existing DB rows before loading
        // a new set from persistent storage
        removeAllRows ();

        UtlString fileName = OsPath::separator + mDatabaseName + ".xml";
        UtlString pathName = SipXecsService::Path(SipXecsService::DatabaseDirType,
                                                  fileName.data());

        OsSysLog::add(FAC_DB, PRI_DEBUG, "CredentialDB::load loading \"%s\"",
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

                    // If this is an old credentials file, it may not contain
                    //    the pintoken element - if not, duplicate the passtoken
                    //    to create it.
                    if (!nvPairs.contains(&gPintokenKey))
                    {
                       UtlString* pintokenKey = new UtlString(gPintokenKey);
                       UtlString* pintokenValue
                          = new UtlString(*(dynamic_cast <UtlString*> (nvPairs.findValue(&gPasstokenKey))));
                       nvPairs.insertKeyAndValue(pintokenKey, pintokenValue);
                    }

                    // Insert the item row into the IMDB
                    insertRow ( nvPairs );
                }
            }
            // Update the tableInfo table and determine if the db has
            // changed as a result of the reload (setting the
            // changed tableInfo field
            SIPDBManager::getInstance()->
                updateDatabaseInfo(
                    mDatabaseName, loadChecksum);
        } else
        {
            OsSysLog::add(FAC_DB, PRI_WARNING, "CredentialDB::load TiXmlDocument::LoadFile() failed for file '%s'",
                    pathName.data());
            result = OS_FAILED;
        }
    } else
    {
        OsSysLog::add(FAC_DB, PRI_ERR, "CredentialDB::load failed - no DB");
        result = OS_FAILED;
    }
    return result;
}

OsStatus
CredentialDB::store()
{
    // Critical Section here
    OsLock lock( sLockMutex );
    OsStatus result = OS_SUCCESS;

    if ( m_pFastDB != NULL )
    {
        UtlString fileName = OsPath::separator + mDatabaseName + ".xml";
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
        dbCursor< CredentialRow > cursor;

        // Select everything in the IMDB and add as item elements if present
        if ( cursor.select() > 0 )
        {
            // metadata contains column names
            dbTableDescriptor* pTableMetaData = &CredentialRow::dbDescriptor;

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
CredentialDB::insertRow (const UtlHashMap& nvPairs)
{
    // Note: identity inferred from the uri
    return insertRow (
        Url (*(dynamic_cast <UtlString*> (nvPairs.findValue(&gUriKey)))),
        *(dynamic_cast <UtlString*> (nvPairs.findValue(&gRealmKey))),
        *(dynamic_cast <UtlString*> (nvPairs.findValue(&gUseridKey))),
        *(dynamic_cast <UtlString*> (nvPairs.findValue(&gPasstokenKey))),
        *(dynamic_cast <UtlString*> (nvPairs.findValue(&gPintokenKey))),
        *(dynamic_cast <UtlString*> (nvPairs.findValue(&gAuthtypeKey))));
}

UtlBoolean
CredentialDB::insertRow (
    const Url& uri,
    const UtlString& realm,
    const UtlString& userid,
    const UtlString& passToken,
    const UtlString& pinToken,
    const UtlString& authType )
{
    UtlBoolean result = FALSE;
    UtlString identity;
    uri.getIdentity(identity);
    if ( !identity.isNull() && (m_pFastDB != NULL) )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        // Search for a matching row before
        // deciding to update or insert
        dbCursor< CredentialRow > cursor(dbCursorForUpdate);

        // The URI field should be unique.
        dbQuery query;
        query="np_identity=",identity,"and realm=",realm;
        UtlString fullUri = uri.toString();
        if ( cursor.select( query ) > 0 )
        {
            do {
                // Update row (Identiy and realm already match so ignore
                cursor->uri = fullUri;
                cursor->userid = userid;
                cursor->passtoken = passToken;
                cursor->pintoken = pinToken;
                cursor->authtype = authType;
                cursor.update();
            } while (cursor.next());
        } else
        {
            // Insert new row
            CredentialRow row;
            row.np_identity = identity;
            row.realm = realm;
            row.uri = fullUri;
            row.userid = userid;
            row.passtoken = passToken;
            row.pintoken = pinToken;
            row.authtype = authType;
            insert (row);
        }
        // Either did an insert or an update
        result = TRUE;

        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);

        // Table Data changed
        SIPDBManager::getInstance()->
            setDatabaseChangedFlag(mDatabaseName, TRUE);
    }
    // Most likely an arg problem
    return result;
}

void CredentialDB::removeRows (
    const Url& uri,
    const UtlString& realm )
{
    UtlString identity;
    uri.getIdentity(identity);
    if ( !identity.isNull() && (m_pFastDB != NULL))
    {
        // Thread Local Storage
        m_pFastDB->attach();

        dbCursor< CredentialRow > cursor(dbCursorForUpdate);

        dbQuery query;
        query="np_identity=",identity,"and realm=",realm;

        if (cursor.select(query) > 0)
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
CredentialDB::removeRows ( const Url& uri )
{
    UtlString identity;
    uri.getIdentity(identity);

    if ( !identity.isNull() && (m_pFastDB != NULL))
    {
        // Thread Local Storage
        m_pFastDB->attach();

        dbCursor< CredentialRow > cursor(dbCursorForUpdate);

        dbQuery query;
        query="np_identity=",identity;
        if (cursor.select(query) > 0)
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
CredentialDB::removeAllRows ()
{
    if ( m_pFastDB != NULL )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        dbCursor< CredentialRow > cursor(dbCursorForUpdate);

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
CredentialDB::getAllRows( ResultSet& rResultSet ) const
{
    // Clear the out any previous records
    rResultSet.destroyAll();

    if (m_pFastDB != NULL)
    {
        // must do this first to ensure process/tls integrity
        m_pFastDB->attach();

        dbCursor< CredentialRow > cursor;
        if ( cursor.select() > 0 )
        {
            do {
                UtlHashMap record;
                UtlString* uriValue =
                    new UtlString ( cursor->uri );
                UtlString* realmValue =
                    new UtlString ( cursor->realm );
                UtlString* useridValue =
                    new UtlString ( cursor->userid );
                UtlString* passtokenValue =
                   new UtlString ( cursor->passtoken );
                UtlString* pintokenValue =
                   new UtlString ( cursor->pintoken );
                UtlString* authtypeValue =
                    new UtlString ( cursor->authtype );

                // Memory Leak fixes, make shallow copies of static keys
                UtlString* uriKey = new UtlString( gUriKey );
                UtlString* realmKey = new UtlString( gRealmKey );
                UtlString* useridKey = new UtlString( gUseridKey );
                UtlString* passtokenKey = new UtlString( gPasstokenKey );
                UtlString* pintokenKey = new UtlString( gPintokenKey );
                UtlString* authtypeKey = new UtlString( gAuthtypeKey );

                record.insertKeyAndValue ( uriKey, uriValue );
                record.insertKeyAndValue ( realmKey, realmValue );
                record.insertKeyAndValue ( useridKey, useridValue );
                record.insertKeyAndValue ( passtokenKey, passtokenValue );
                record.insertKeyAndValue ( pintokenKey, pintokenValue );
                record.insertKeyAndValue ( authtypeKey, authtypeValue );

                rResultSet.addValue(record);
            } while (cursor.next());
        }
        // commit rows and also ensure process/tls integrity
        m_pFastDB->detach(0);
    }
}

int
CredentialDB::getRowCount () const
{
   int count = 0;

   // Thread Local Storage
   m_pFastDB->attach();

   dbCursor< CredentialRow > cursor;

   dbQuery query;
   query="";
   count = cursor.select( query );

   // Commit rows to memory - multiprocess workaround
   m_pFastDB->detach(0);

   return count;
}

UtlBoolean
CredentialDB::getCredentialByUserid (
    const Url& uri,
    const UtlString& realm,
    const UtlString& userid,
    UtlString& passtoken,
    UtlString& authType ) const
{
    UtlBoolean found = FALSE;

    UtlString identity;
    uri.getIdentity(identity);

    // Match the row and return the passtoken and authtype
    if ( !identity.isNull() && (m_pFastDB != NULL) )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        dbCursor< CredentialRow > cursor;

        dbQuery query;
        query="np_identity=",identity, \
              "and realm=",realm, \
              "and userid=",userid, \
              "order by np_identity asc, realm asc";

        if ( cursor.select( query ) > 0 )
        {
            do {
                passtoken = cursor->passtoken;
                authType = cursor->authtype;
                found = TRUE;
            } while ( cursor.next() );
        }
        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }
    return found;
}

UtlBoolean
CredentialDB::getCredential (
    const UtlString& userid,
    const UtlString& realm,
    Url& uri,
    UtlString& passtoken,
    UtlString& authType ) const
{
    UtlBoolean found = FALSE;

    if ( !userid.isNull() && !realm.isNull() && (m_pFastDB != NULL) )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        // Search to see if we have a Credential Row
        dbCursor< CredentialRow > cursor;

        dbQuery query;

        query="userid=",userid,"and realm=",realm;
        if ( cursor.select(query) > 0 ) {
            do {
                uri = cursor->uri;
                passtoken = cursor->passtoken;
                authType = cursor->authtype;
            } while ( cursor.next() );
            found = TRUE;
        }
        // Commit the rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }
    return found;
}

UtlBoolean CredentialDB::getCredential (const Url& uri,
                                        const UtlString& realm,
                                        UtlString& userid,
                                        UtlString& passtoken,
                                        UtlString& authType ) const
{
    UtlBoolean found = FALSE;

    UtlString identity;
    uri.getIdentity(identity);

    // Match the row and return the passtoken and authtype
    if ( !identity.isNull() && (m_pFastDB != NULL) )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        dbCursor< CredentialRow > cursor;

        dbQuery query;
        query="np_identity=",identity, \
              "and realm=",realm, \
              "order by np_identity asc, realm asc";

        if ( cursor.select( query ) > 0 )
        {
            do {
                userid = cursor->userid;
                passtoken = cursor->passtoken;
                authType = cursor->authtype;
                found = TRUE;
            } while ( cursor.next() );
        }
        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }
    return found;
}

/// Retrieve the User PIN check values for a given identity and realm
UtlBoolean CredentialDB::getUserPin (
   const Url& uri,
   const UtlString& realm,
   UtlString& userid,
   UtlString& pintoken,
   UtlString& authType
                                     ) const
{
   UtlBoolean found = FALSE;

   UtlString identity;
   uri.getIdentity(identity);

   // Match the row and return the passtoken and authtype
   if ( !identity.isNull() && (m_pFastDB != NULL) )
   {
      // Thread Local Storage
      m_pFastDB->attach();

      dbCursor< CredentialRow > cursor;

      dbQuery query;
      query="np_identity=",identity, \
         "and realm=",realm, \
         "order by np_identity asc, realm asc";

      if ( cursor.select( query ) > 0 )
      {
         do {
            userid = cursor->userid;
            pintoken = cursor->pintoken;
            authType = cursor->authtype;
            found = TRUE;
         } while ( cursor.next() );
      }
      // Commit rows to memory - multiprocess workaround
      m_pFastDB->detach(0);
   }
   return found;
}


/// Retrieve the User PIN check values for a given userid and realm
UtlBoolean CredentialDB::getUserPin (
   const UtlString& userid,
   const UtlString& realm,
   Url& uri,
   UtlString& pintoken,
   UtlString& authType
                       ) const
{
   UtlBoolean found = FALSE;

   if ( !userid.isNull() && !realm.isNull() && (m_pFastDB != NULL) )
   {
      // Thread Local Storage
      m_pFastDB->attach();

      // Search to see if we have a Credential Row
      dbCursor< CredentialRow > cursor;

      dbQuery query;

      query="userid=",userid,"and realm=",realm;
      if ( cursor.select(query) > 0 ) {
         do {
            uri = cursor->uri;
            pintoken = cursor->pintoken;
            authType = cursor->authtype;
         } while ( cursor.next() );
         found = TRUE;
      }
      // Commit the rows to memory - multiprocess workaround
      m_pFastDB->detach(0);
   }
   return found;
}


void
CredentialDB::getAllCredentials (
    const Url& uri,
    ResultSet& rResultSet ) const
{
    UtlString identity;
    uri.getIdentity(identity);

    // This should erase the contents of the existing resultset
    rResultSet.clear();

    if ( !identity.isNull() && (m_pFastDB != NULL) )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        // Search to see if we have a Credential Row
        dbCursor< CredentialRow > cursor;

        dbQuery query;
        query="np_identity=",identity;

        if ( cursor.select( query ) > 0 )
        {
            do {
                UtlHashMap record;
                UtlString* uriValue =
                    new UtlString ( cursor->uri );
                UtlString* realmValue =
                    new UtlString ( cursor->realm );
                UtlString* useridValue =
                    new UtlString ( cursor->userid );
                UtlString* passtokenValue =
                   new UtlString ( cursor->passtoken );
                UtlString* pintokenValue =
                   new UtlString ( cursor->pintoken );
                UtlString* authtypeValue =
                    new UtlString ( cursor->authtype );

                // Memory Leak fixes, make shallow copies of static keys
                UtlString* uriKey = new UtlString( gUriKey );
                UtlString* realmKey = new UtlString( gRealmKey );
                UtlString* useridKey = new UtlString( gUseridKey );
                UtlString* passtokenKey = new UtlString( gPasstokenKey );
                UtlString* pintokenKey = new UtlString( gPintokenKey );
                UtlString* authtypeKey = new UtlString( gAuthtypeKey );

                record.insertKeyAndValue ( uriKey, uriValue );
                record.insertKeyAndValue ( realmKey, realmValue );
                record.insertKeyAndValue ( useridKey, useridValue );
                record.insertKeyAndValue ( passtokenKey, passtokenValue );
                record.insertKeyAndValue ( pintokenKey, pintokenValue );
                record.insertKeyAndValue ( authtypeKey, authtypeValue );

                rResultSet.addValue(record);
            } while ( cursor.next() );
        }
        // Commit the rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }
}

UtlBoolean
CredentialDB::isUriDefined (
    const Url& uri,
    UtlString& realm,
    UtlString& authType ) const
{
    UtlBoolean found = FALSE;

    UtlString identity;
    uri.getIdentity(identity);

    OsSysLog::add(FAC_DB, PRI_DEBUG, "CredentialDB::isUriDefined identity %s, m_pFastDB=%p ",
                    identity.data(), m_pFastDB);

    if ( !identity.isNull() && (m_pFastDB != NULL) )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        // Search to see if we have a Credential Row
        dbCursor< CredentialRow > cursor;

        dbQuery query;
        // This is equivalent to select * from credentials
        // where identity=uri.getIdentity() and realm=realm
        // and authType = authType
        query="np_identity=",identity;
        if ( cursor.select(query) > 0 ) {
    	    OsSysLog::add(FAC_DB, PRI_DEBUG, "CredentialDB::isUriDefined cursor selected ");
            do {
                realm = cursor->realm;
                authType = cursor->authtype;
            } while ( cursor.next() );
            found = TRUE;
        }
        // Commit the rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }
    OsSysLog::add(FAC_DB, PRI_DEBUG, "CredentialDB::isUriDefined found=%d ", (int)found);
    return found;
}

bool
CredentialDB::isLoaded()
{
   return mTableLoaded;
}

CredentialDB*
CredentialDB::getInstance( const UtlString& name )
{
    // Critical Section here
    OsLock lock( sLockMutex );

    // See if this is the first time through for this process
    // Note that this being null => pgDatabase is also null
    if ( spInstance == NULL )
    {
        // Create the singleton class for clients to use
        spInstance = new CredentialDB( name );
    }
    return spInstance;
}
