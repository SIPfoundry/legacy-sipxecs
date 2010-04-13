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
#include "net/Url.h"

#include "fastdb/fastdb.h"
#include "xmlparser/tinyxml.h"
#include "sipXecsService/SipXecsService.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/ResultSet.h"
#include "sipdb/CallerAliasRow.h"
#include "sipdb/CallerAliasDB.h"

REGISTER( CallerAliasRow );

// STATIC INITIALIZERS
CallerAliasDB* CallerAliasDB::spInstance = NULL;
OsMutex  CallerAliasDB::sLockMutex (OsMutex::Q_FIFO);

const UtlString CallerAliasDB::IdentityKey("identity");
const UtlString CallerAliasDB::DomainKey("domain");
const UtlString CallerAliasDB::AliasKey("alias");

const UtlString CallerAliasDB::DbName("caller-alias");

// The 'type' attribute of the top-level 'items' element.
const UtlString CallerAliasDB::sType("caller-alias");

// The XML namespace of the top-level 'items' element.
const UtlString CallerAliasDB::sXmlNamespace("http://www.sipfoundry.org/sipX/schema/xml/caller-alias-00-00");

/* ============================ CREATORS ================================== */

CallerAliasDB::CallerAliasDB( const UtlString& name )
: mDatabaseName( name )
, mTableLoaded ( true )
{
   // Access the shared table databse
   SIPDBManager* pSIPDBManager = SIPDBManager::getInstance();
   mpFastDB = pSIPDBManager->getDatabase(name);

   // If we are the first process to attach
   // then we need to load the DB
   int users = pSIPDBManager->getNumDatabaseProcesses(name);
   OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                 "CallerAliasDB::_ users = %d, mTableLoaded = %d",
                 users, mTableLoaded);
   if ( users == 1 || ( users > 1 && mTableLoaded == false ) )
   {
      OsSysLog::add(FAC_DB, PRI_DEBUG, "CallerAliasDB::_ about to load");
      mTableLoaded = false;
      // Load the file implicitly
      if (this->load() == OS_SUCCESS)
      {
         mTableLoaded = true;
         OsSysLog::add(FAC_DB, PRI_DEBUG, "CallerAliasDB::_ table successfully loaded");
      }
   }
   OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                 "CallerAliasDB::_ rows in table = %d",
                 getRowCount());
}

CallerAliasDB::~CallerAliasDB()
{
}

CallerAliasDB*
CallerAliasDB::getInstance( const UtlString& name )
{
   // Critical Section here
   OsLock lock( sLockMutex );

   // See if this is the first time through for this process
   // Note that this being null => pgDatabase is also null
   if ( spInstance == NULL )
   {
      // Create the singleton class for clients to use
      spInstance = new CallerAliasDB( name );
   }
   return spInstance;
}

void
CallerAliasDB::releaseInstance()
{
   // Critical Section here
   OsLock lock( sLockMutex );

   // if it exists, delete the object and NULL out the pointer
   if (spInstance != NULL) {

      // unregister this table/process from the IMDB
      SIPDBManager::getInstance()->removeDatabase ( DbName );

      // NULL out the fastDB pointer also
      spInstance->mpFastDB = NULL;

      delete spInstance;
      spInstance = NULL;
   }
}

int
CallerAliasDB::getRowCount () const
{
   int count = 0;

   // Thread Local Storage
   mpFastDB->attach();

   dbCursor< CallerAliasRow > cursor;

   dbQuery query;
   query="";
   count = cursor.select( query );

   // Commit rows to memory - multiprocess workaround
   mpFastDB->detach(0);

   return count;
}

// Add a single mapping to the database.
void CallerAliasDB::insertRow(
   const UtlString identity, ///< identity of caller in 'user@domain' form (no scheme)
   const UtlString domain,   /**< domain and optional port for target
                              *  ( 'example.com' or 'example.com:5099' ) */
   const UtlString alias     /// returned alias
                              )
{
   /*
    * The identity value may be the null string; this is a wildcard entry that matches
    * any caller to the given domain.
    */
   if ( !domain.isNull() && !alias.isNull() && (mpFastDB != NULL) )
   {
      // Thread Local Storage
      mpFastDB->attach();

      // Search for a matching row before deciding to update or insert
      dbCursor< CallerAliasRow > cursor(dbCursorForUpdate);

      CallerAliasRow row;
      dbQuery query;

      // Primary Key is the urialias's identity
      query="identity=",identity.data(), " and domain=", domain.data();

      if ( cursor.select( query ) > 0 )
      {
         // Should only be one row so update the contact
         do {
            cursor->alias = alias.data();
            cursor.update();
         } while ( cursor.next() );
      }
      else // Insert as the row does not exist
      {
         // Fill out the row columns
         row.identity = identity.data();
         row.domain = domain.data();
         row.alias = alias.data();
         insert (row);
      }
#     if VERBOSE_LOGGING
      OsSysLog::add(FAC_DB,PRI_DEBUG,
                    "CallerAliasDB::insertRow "
                    "identity='%s', domain='%s', alias='%s'",
                    identity.data(), domain.data(), alias.data()
                    );
#     endif

      // Commit rows to memory - multiprocess workaround
      mpFastDB->detach(0);

      // Table Data changed
      SIPDBManager::getInstance()->
         setDatabaseChangedFlag(DbName, TRUE);
   }
   else
   {
      OsSysLog::add(FAC_DB,PRI_CRIT,
                    "CallerAliasDB::insertRow failed "
                    "db=%p, domain='%s', alias='%s'",
                    mpFastDB, domain.data(), alias.data()
                    );
   }
}

OsStatus
CallerAliasDB::load()
{
   // Critical Section here
   OsLock lock( sLockMutex );
   OsStatus result = OS_SUCCESS;

   if ( mpFastDB != NULL )
   {
      // Clean out the existing DB rows before loading
      // a new set from persistent storage
      removeAllRows ();

      UtlString fileName = DbName + ".xml";
      UtlString pathName = SipXecsService::Path(SipXecsService::DatabaseDirType,
                                                fileName.data());

      OsSysLog::add(FAC_DB, PRI_DEBUG, "CallerAliasDB::load loading '%s'", pathName.data());

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
               UtlString identity;
               UtlString domain;
               UtlString alias;

               for( TiXmlNode *elementNode = itemNode->FirstChild();
                    elementNode;
                    elementNode = elementNode->NextSibling() )
               {
                  // Bypass comments and other element types only interested
                  // in parsing element attributes
                  if ( elementNode->Type() == TiXmlNode::ELEMENT )
                  {
                     UtlString column(elementNode->Value());

                     if (column.compareTo(IdentityKey) == 0)
                     {
                        SIPDBManager::getAttributeValue(*itemNode, column, identity);
                     }
                     else if (column.compareTo(DomainKey) == 0)
                     {
                        SIPDBManager::getAttributeValue(*itemNode, column, domain);
                     }
                     else if (column.compareTo(AliasKey) == 0)
                     {
                        SIPDBManager::getAttributeValue(*itemNode, column, alias);
                     }
                     else
                     {
                        OsSysLog::add(FAC_DB, PRI_ERR,
                                      "Unrecognized column '%s' in item: ignored",
                                      column.data()
                                      );
                     }
                  }
               }
               // Insert the item row into the IMDB
               insertRow (identity, domain, alias);
            }
         }
      }
      else
      {
         OsSysLog::add(FAC_DB, PRI_WARNING, "CallerAliasDB::load failed to load '%s'",
                       pathName.data());
         result = OS_FAILED;
      }
   }
   else
   {
      OsSysLog::add(FAC_DB, PRI_ERR, "CallerAliasDB::load failed - no DB");
      result = OS_FAILED;
   }
   return result;
}

OsStatus
CallerAliasDB::store()
{
   UtlString fileName = DbName + ".xml";
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

   // Critical Section while actually opening and using the database
   {
      OsLock lock( sLockMutex );

      if ( mpFastDB != NULL )
      {
         // Thread Local Storage
         mpFastDB->attach();

         // Search our memory for rows
         dbCursor< CallerAliasRow > cursor;

         // Select everything in the IMDB and add as item elements if present
         int rowNumber;
         int rows;
         for (rowNumber = 0, rows = cursor.select();
              rowNumber < rows;
              rowNumber++, cursor.next()
              )
         {
            // Create an item container
            TiXmlElement itemElement ("item");

            if ( *cursor->identity )
            {
               // Add an identity element and put the value in it
               TiXmlElement identityElement(IdentityKey.data());
               TiXmlText    identityValue(cursor->identity);
               identityElement.InsertEndChild(identityValue);
               itemElement.InsertEndChild(identityElement);
            }

            // add the domain element and put the value in it
            TiXmlElement domainElement(DomainKey.data());
            TiXmlText    domainValue(cursor->domain);
            domainElement.InsertEndChild(domainValue);
            itemElement.InsertEndChild(domainElement);

            // add the alias element and put the value in it
            TiXmlElement aliasElement(AliasKey.data());
            TiXmlText    aliasValue(cursor->alias);
            aliasElement.InsertEndChild(aliasValue);
            itemElement.InsertEndChild(aliasElement);

            // add this item (row) to the parent items container
            itemsElement.InsertEndChild ( itemElement );
         }

         // Commit rows to memory - multiprocess workaround
         mpFastDB->detach(0);
         mTableLoaded = true;
      }
   } // release mutex around database use

   // Attach the root node to the document
   document.InsertEndChild ( itemsElement );
   document.SaveFile ( pathName );

   return OS_SUCCESS;
}


void
CallerAliasDB::removeAllRows ()
{
   // Thread Local Storage
   if (mpFastDB != NULL)
   {
      mpFastDB->attach();

      dbCursor< CallerAliasRow > cursor(dbCursorForUpdate);

      if (cursor.select() > 0)
      {
         cursor.removeAllSelected();
      }
      // Commit rows to memory - multiprocess workaround
      mpFastDB->detach(0);

      // Table Data changed
      SIPDBManager::getInstance()->
         setDatabaseChangedFlag(DbName, TRUE);
   }
}

bool
CallerAliasDB::isLoaded()
{
   return mTableLoaded;
}

/// Get the caller alias for this combination of caller identity and target domain.
bool CallerAliasDB::getCallerAlias (
   const UtlString& identity, ///< identity of caller in 'user@domain' form (no scheme)
   const UtlString& domain,   /**< domain and optional port for target
                               *  ( 'example.com' or 'example.com:5099' ) */
   UtlString& callerAlias     /// returned alias
                     ) const
{
   /*
    * This first looks in the database for an exact match of identity and domain;
    *   if this match is found, the resulting alias is returned in callerAlias.
    * If no exact match is found, the database is then checked for a row containing
    *   a null (empty string) identity and the domain; this is a domain wildcard entry
    *   and it is returned in callerAlias.
    * If neither match is found, callerAlias is set to the null string.
    */
   callerAlias.remove(0);

   if (mpFastDB)
   {
      // Thread Local Storage
      mpFastDB->attach();

      // Search to see if we have a row with this exact combination
      dbQuery exactQuery;
      exactQuery="identity=",identity.data()," and domain=",domain.data();
      dbCursor< CallerAliasRow > exactCursor;
      if (exactCursor.select(exactQuery))
      {
         // found a match
         callerAlias.append(exactCursor->alias);
      }

      // Commit the rows to memory - multiprocess workaround
      mpFastDB->detach(0);
   }

   // Returns true if an alias was found for this caller, false if not
   return ! callerAlias.isNull();
}
