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
#include "utl/UtlTokenizer.h"
#include "xmlparser/tinyxml.h"
#include "fastdb/fastdb.h"
#include "net/Url.h"

#include "sipXecsService/SipXecsService.h"

#include "sipdb/SIPDBManager.h"
#include "sipdb/ResultSet.h"
#include "sipdb/LocationRow.h"
#include "sipdb/LocationDB.h"

REGISTER( LocationRow );

// Static Initializers
LocationDB* LocationDB::spInstance = NULL;
OsMutex  LocationDB::sLockMutex (OsMutex::Q_FIFO);
UtlString LocationDB::gNameKey("name");
UtlString LocationDB::gDescriptionKey("description");
UtlString LocationDB::gLocationCodeKey("locationcode");
UtlString LocationDB::gSubnetsKey("subnets");

// The 'type' attribute of the top-level 'items' element.
const UtlString LocationDB::sType("location");

// The XML namespace of the top-level 'items' element.
const UtlString LocationDB::sXmlNamespace("http://www.sipfoundry.org/sipX/schema/xml/location-00-00");

/* ============================ CREATORS ================================== */

LocationDB::LocationDB( const UtlString& name )
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
                  "LocationDB::_ users = %d, mTableLoaded = %d",
                  users, mTableLoaded);
    if ( users == 1 || ( users > 1 && mTableLoaded == false ) )
    {
        OsSysLog::add(FAC_DB, PRI_DEBUG, "LocationDB::_ about to load");
        mTableLoaded = false;
        // Load the file implicitly
        if (this->load() == OS_SUCCESS)
        {
           mTableLoaded = true;
           OsSysLog::add(FAC_DB, PRI_DEBUG, "LocationDB::_ table successfully loaded");
        }
    }
    OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                  "LocationDB::_ rows in table = %d",
                  getRowCount());
}

LocationDB::~LocationDB() 
{
    OsSysLog::add(FAC_DB, PRI_DEBUG, "<><>## LocationDB:: DESTRUCTOR");
}

/* ============================ MANIPULATORS ============================== */

void
LocationDB::releaseInstance()
{
    OsSysLog::add(FAC_DB, PRI_DEBUG, "<><>## LocationDB:: releaseInstance() spInstance=%p", spInstance);

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
LocationDB::load()
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

        OsSysLog::add(FAC_DB, PRI_DEBUG, "LocationDB::load loading \"%s\"",
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
            OsSysLog::add(FAC_SIP, PRI_WARNING, "LocationDB::load failed to load \"%s\"",
                          pathName.data());
            result = OS_FAILED;
        }
    } else 
    {
        OsSysLog::add(FAC_DB, PRI_ERR, "LocationDB::load failed - no DB");
        result = OS_FAILED;
    }
    return result;
}

OsStatus
LocationDB::store()
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
        dbCursor< LocationRow > cursor;

        // Select everything in the IMDB and add as item elements if present
        if ( cursor.select() > 0 )
        {
            // metadata contains column names
            dbTableDescriptor* pTableMetaData = &LocationRow::dbDescriptor;

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
LocationDB::getRowCount () const
{
   int count = 0;

   // Thread Local Storage
   m_pFastDB->attach();

   dbCursor< LocationRow > cursor;

   dbQuery query;
   query="";
   count = cursor.select( query );

   // Commit rows to memory - multiprocess workaround
   m_pFastDB->detach(0);

   return count;
}

UtlBoolean
LocationDB::insertRow (const UtlHashMap& nvPairs) 
{

    return insertRow (
        *(dynamic_cast <UtlString*> (nvPairs.findValue(&gNameKey))),
        *(dynamic_cast <UtlString*> (nvPairs.findValue(&gDescriptionKey))),
        *(dynamic_cast <UtlString*> (nvPairs.findValue(&gLocationCodeKey))),
        *(dynamic_cast <UtlString*> (nvPairs.findValue(&gSubnetsKey)))
       );
}

UtlBoolean
LocationDB::insertRow (
      const UtlString& locationName,
      const UtlString& locationDescription,
      const UtlString& locationCode,
      const UtlString& locationSubnetsString )
{
    UtlBoolean result = FALSE;

    if ( !locationName.isNull() && (m_pFastDB != NULL) )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        // Search for a matching row before deciding to update or insert
        dbCursor< LocationRow > cursor(dbCursorForUpdate);
        dbQuery query;

        // If the row already exists do nothing
        query="name=",locationName;

        if ( cursor.select( query ) == 0 )
        {
            // Non existent row so insert
            LocationRow row;
            // Fill out the row columns
            row.name = locationName;
            row.description = locationDescription;
            row.locationcode = locationCode;
            row.subnets = locationSubnetsString;
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
LocationDB::removeRow ( const UtlString& locationName )
{
    UtlBoolean rc = FALSE;

    if ( !locationName.isNull() && (m_pFastDB != NULL) )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        dbCursor< LocationRow > cursor(dbCursorForUpdate);

        dbQuery query;
        query="name=",locationName;
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

UtlBoolean
LocationDB::removeRows ( const UtlString& locationName )
{
   return removeRow( locationName );
}

void
LocationDB::removeAllRows ()
{
    if ( m_pFastDB != NULL ) 
    {
        // Thread Local Storage
        m_pFastDB->attach();
        dbCursor< LocationRow > cursor( dbCursorForUpdate );

        if (cursor.select() > 0)
        {
            cursor.removeAllSelected();
        }
        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    } 
}

UtlBoolean 
LocationDB::getRowByName( const UtlString& locationName, UtlHashMap& nvPairs ) const
{
   dbQuery query;
   query="name=",locationName;
   return getRowByQuery( query, nvPairs );
}

UtlBoolean 
LocationDB::getRowByLocationCode( const UtlString& locationCode, UtlHashMap& nvPairs ) const
{
   dbQuery query;
   query="locationcode=",locationCode;
   return getRowByQuery( query, nvPairs );
}

UtlBoolean 
LocationDB::getRowByIpAddress( const UtlString& ipAddress, UtlHashMap& nvPairs ) const
{
   bool bEntryFound = false;
   nvPairs.destroyAll();
   ResultSet resultSet;
   UtlString ipSubnet;
   int index = 0;

   getAllRows( resultSet );
   for( index = 0; !bEntryFound && index < resultSet.getSize(); index++ )
   {
      UtlHashMap tempNvPairs;
      if( resultSet.getIndex( index, tempNvPairs ) == OS_SUCCESS )
      {
         UtlString* pSubnetsString;
         
         pSubnetsString = dynamic_cast<UtlString*>( tempNvPairs.findValue( &gSubnetsKey ) );
         if( pSubnetsString && !pSubnetsString->isNull() )
         {
            UtlTokenizer tokenizer( *pSubnetsString );
            while( tokenizer.next( ipSubnet, "," ) )
            {
               if( subnetPatternMatcher.IPv4subnet( ipAddress, ipSubnet ) )
               {
                  bEntryFound = true;
                  // entry is found.  Allocate new strings to create name-value pairs that will be 
                  // returned to the caller.
                  UtlString* nameValue         = new UtlString ( *(UtlString*)tempNvPairs.findValue( &gNameKey ) );
                  UtlString* descriptionValue  = new UtlString ( *(UtlString*)tempNvPairs.findValue( &gDescriptionKey ) );
                  UtlString* locationCodeValue = new UtlString ( *(UtlString*)tempNvPairs.findValue( &gLocationCodeKey ) );
                  UtlString* subnetsValue      = new UtlString ( *(UtlString*)tempNvPairs.findValue( &gSubnetsKey ) );

                  UtlString* nameKey         = new UtlString( gNameKey );
                  UtlString* descriptionKey  = new UtlString( gDescriptionKey );
                  UtlString* locationCodeKey = new UtlString( gLocationCodeKey );
                  UtlString* subnetsKey      = new UtlString( gSubnetsKey );

                  nvPairs.insertKeyAndValue ( nameKey,         nameValue ); 
                  nvPairs.insertKeyAndValue ( descriptionKey,  descriptionValue ); 
                  nvPairs.insertKeyAndValue ( locationCodeKey, locationCodeValue ); 
                  nvPairs.insertKeyAndValue ( subnetsKey,      subnetsValue );    
               }
            }
         }
      }
   }
   return bEntryFound;
}


UtlBoolean 
LocationDB::getRowByQuery( dbQuery& query, UtlHashMap& nvPairs ) const
{
   UtlBoolean success = false;
   
   // Clear the results
   nvPairs.destroyAll();
   
   if ( m_pFastDB != NULL )
   {
      // Thread Local Storage
      m_pFastDB->attach();

      dbCursor< LocationRow > cursor;
      if ( cursor.select( query ) > 0 )
      {
         success = true;
         UtlString* nameValue         = new UtlString ( cursor->name );
         UtlString* descriptionValue  = new UtlString ( cursor->description );
         UtlString* locationCodeValue = new UtlString ( cursor->locationcode );
         UtlString* subnetsValue      = new UtlString ( cursor->subnets );

         UtlString* nameKey         = new UtlString( gNameKey );
         UtlString* descriptionKey  = new UtlString( gDescriptionKey );
         UtlString* locationCodeKey = new UtlString( gLocationCodeKey );
         UtlString* subnetsKey      = new UtlString( gSubnetsKey );

         nvPairs.insertKeyAndValue ( nameKey,         nameValue ); 
         nvPairs.insertKeyAndValue ( descriptionKey,  descriptionValue ); 
         nvPairs.insertKeyAndValue ( locationCodeKey, locationCodeValue ); 
         nvPairs.insertKeyAndValue ( subnetsKey,      subnetsValue ); 
       }
       m_pFastDB->detach(0);
   }
   return success;
}

void
LocationDB::getAllRows(ResultSet& rResultSet) const
{
    // Clear the results
    rResultSet.destroyAll();

    if ( m_pFastDB != NULL )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        dbCursor< LocationRow > cursor;
        if ( cursor.select() > 0 )
        {
            do {
                UtlHashMap record;
                UtlString* nameValue         = new UtlString ( cursor->name );
                UtlString* descriptionValue  = new UtlString ( cursor->description );
                UtlString* locationCodeValue = new UtlString ( cursor->locationcode );
                UtlString* subnetsValue      = new UtlString ( cursor->subnets );

                // Memory Leak fixes, make shallow copies of static keys
                UtlString* nameKey         = new UtlString( gNameKey );
                UtlString* descriptionKey  = new UtlString( gDescriptionKey );
                UtlString* locationCodeKey = new UtlString( gLocationCodeKey );
                UtlString* subnetsKey      = new UtlString( gSubnetsKey );

                record.insertKeyAndValue ( nameKey,         nameValue ); 
                record.insertKeyAndValue ( descriptionKey,  descriptionValue ); 
                record.insertKeyAndValue ( locationCodeKey, locationCodeValue ); 
                record.insertKeyAndValue ( subnetsKey,      subnetsValue ); 

                rResultSet.addValue(record);
            } while (cursor.next());
        }
        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }
}

bool
LocationDB::isLoaded()
{
   return mTableLoaded;
}

LocationDB*
LocationDB::getInstance( const UtlString& name )
{
    // Critical Section here
   OsLock lock( sLockMutex );

    // See if this is the first time through for this process
    // Note that this being null => pgDatabase is also null
    if ( spInstance == NULL )
    {
        // Create the singleton class for clients to use
        spInstance = new LocationDB( name );
    }
    return spInstance;
}
