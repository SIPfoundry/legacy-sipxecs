//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

#include <string.h>

// APPLICATION INCLUDES
#include "utl/UtlInt.h"
#include "utl/UtlSList.h"
#include "os/OsLock.h"
#include "os/OsDateTime.h"
#include "os/OsFS.h"
#include "os/OsSysLog.h"
#include "xmlparser/tinyxml.h"
#include "fastdb/fastdb.h"
#include "net/Url.h"

#include "sipXecsService/SipXecsService.h"
#include "sipdb/ResultSet.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/SubscriptionRow.h"
#include "sipdb/SubscriptionDB.h"

// DEFINES
REGISTER( SubscriptionRow );

// MACROS
#define TEST_DEBUG /* enable to turn on detailed database logs */

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

// Static Initializers
SubscriptionDB* SubscriptionDB::spInstance = NULL;
OsMutex         SubscriptionDB::sLockMutex (OsMutex::Q_FIFO);
const UtlString SubscriptionDB::gComponentKey("component");
const UtlString SubscriptionDB::gUriKey("uri");
const UtlString SubscriptionDB::gCallidKey("callid");
const UtlString SubscriptionDB::gContactKey("contact");
const UtlString SubscriptionDB::gNotifycseqKey("notifycseq");
const UtlString SubscriptionDB::gSubscribecseqKey("subscribecseq");
const UtlString SubscriptionDB::gExpiresKey("expires");
const UtlString SubscriptionDB::gEventtypekeyKey("eventtypekey"); // sic
const UtlString SubscriptionDB::gEventtypeKey("eventtype");
const UtlString SubscriptionDB::gIdKey("id");
const UtlString SubscriptionDB::gToKey("toUri");
const UtlString SubscriptionDB::gFromKey("fromUri");
const UtlString SubscriptionDB::gFileKey("file");
const UtlString SubscriptionDB::gKeyKey("key");
const UtlString SubscriptionDB::gRecordrouteKey("recordroute");
const UtlString SubscriptionDB::gAcceptKey("accept");
const UtlString SubscriptionDB::gVersionKey("version");
const UtlString SubscriptionDB::sComponentStatus("status");
const UtlString SubscriptionDB::sAcceptSimpleMessage("application/simple-message-summary");

const UtlString SubscriptionDB::nullString("");

// The 'type' attribute of the top-level 'items' element.
const UtlString SubscriptionDB::sType("subscription");

// The XML namespace of the top-level 'items' element.
const UtlString SubscriptionDB::sXmlNamespace("http://www.sipfoundry.org/sipX/schema/xml/subscription-00-00");

/* ============================ CREATORS ================================== */

SubscriptionDB::SubscriptionDB( const UtlString& name )
: mDatabaseName( name )
, mTableLoaded ( true )
{
    // Access the shared table databse
    SIPDBManager* pSIPDBManager = SIPDBManager::getInstance();
    m_pFastDB = pSIPDBManager->getDatabase(name);

    // The following logic is not correct and will not work properly if several processes
    // attempt to load at the same time.
    // It is circumvented by the fact that supervisor preloads the database before
    // any other processes are started.
    // If we are the first process to attach
    // then we need to load the DB
    int users = pSIPDBManager->getNumDatabaseProcesses(name);
    OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                  "SubscriptionDB::_ users = %d, mTableLoaded = %d",
                  users, mTableLoaded);
    if ( users == 1 || ( users > 1 && mTableLoaded == false ) )
    {
        OsSysLog::add(FAC_DB, PRI_DEBUG, "SubscriptionDB::_ about to load");
        mTableLoaded = false;
        // Load the file implicitly
        if (this->load() == OS_SUCCESS)
        {
           OsSysLog::add(FAC_DB, PRI_DEBUG, "SubscriptionDB::_ table successfully loaded");
        }
        // the SubscriptionDB is not replicated from sipXconfig, as
        // a result, make this table appear as being loaded regardless
        // of the load() result.
        mTableLoaded = true;
    }
    OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                  "SubscriptionDB::_ rows in table = %d",
                  getRowCount());
}

SubscriptionDB::~SubscriptionDB()
{
    OsSysLog::add(FAC_DB, PRI_DEBUG, "<><>## SubscriptionDB:: DESTRUCTOR");
}

/* ============================ MANIPULATORS ============================== */

void
SubscriptionDB::releaseInstance()
{
    OsSysLog::add(FAC_DB, PRI_DEBUG, "<><>## SubscriptionDB::releaseInstance() spInstance=%p", spInstance);

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
SubscriptionDB::load()
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

        OsSysLog::add(FAC_DB, PRI_DEBUG, "SubscriptionDB::load loading \"%s\"",
                      pathName.data());

        TiXmlDocument doc ( pathName );

        // Verify that we can load the file (i.e it must exist)
        if( doc.LoadFile() )
        {
            TiXmlNode * rootNode = doc.FirstChild ( "items" );
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
            // non-existence of the file is not an error for this DB
            if (OsFileSystem::exists(pathName))
            {
                OsSysLog::add(FAC_SIP, PRI_WARNING, "SubscriptionDB::load failed to load \"%s\"",
                              pathName.data());
                result = OS_FAILED;
            }
        }
    } else
    {
        OsSysLog::add(FAC_DB, PRI_ERR, "SubscriptionDB::load failed - no DB");
        result = OS_FAILED;
    }
    return result;
}

OsStatus
SubscriptionDB::store()
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

        int timeNow = (int)OsDateTime::getSecsSinceEpoch();
        itemsElement.SetAttribute( "timestamp", timeNow );

        // Thread Local Storage
        m_pFastDB->attach();

        // Search our memory for rows
        dbCursor< SubscriptionRow > cursor;

        // Select everything in the IMDB and add as item elements if present
        int rows = cursor.select();
        if ( rows > 0 )
        {
            OsSysLog::add( FAC_SIP, PRI_DEBUG
                          ,"SubscriptionDB::store writing %d rows"
                          ,rows
                          );

            // metadata contains column names
            dbTableDescriptor* pTableMetaData = &SubscriptionRow::dbDescriptor;

            do {
                // Create an item container
                TiXmlElement itemElement ("item");

                byte* base = (byte*)cursor.get();

                // Add the column name value pairs
                for ( dbFieldDescriptor* fd = pTableMetaData->getFirstField();
                      fd != NULL; fd = fd->nextField )
                {
                    // If the column name does not contain the
                    // "np_" prefix, we must persist it.
                    if ( strstr( fd->name, "np_" ) == NULL )
                    {
                        // Create the a column element named after the IMDB column name
                        TiXmlElement element (fd->name);

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
SubscriptionDB::getRowCount () const
{
   int count = 0;

   // Thread Local Storage
   m_pFastDB->attach();

   dbCursor< SubscriptionRow > cursor;

   dbQuery query;
   query="";
   count = cursor.select( query );

   // Commit rows to memory - multiprocess workaround
   m_pFastDB->detach(0);

   return count;
}

UtlBoolean
SubscriptionDB::insertRow (const UtlHashMap& nvPairs)
{
    // For columns that were added in version 3.8, supply the default values
    // if the input data does not supply one.

    // For other integer values, default to 0 if the datum is missing in the input.

    UtlString* cComponent = dynamic_cast <UtlString*> (nvPairs.findValue(&gComponentKey));
    UtlString* expStr = dynamic_cast <UtlString*> (nvPairs.findValue(&gExpiresKey));
    UtlString* cSubseqStr = dynamic_cast <UtlString*> (nvPairs.findValue(&gSubscribecseqKey));
    UtlString* cNotifySeqStr = dynamic_cast <UtlString*> (nvPairs.findValue(&gNotifycseqKey));
    UtlString* cVersionStr = dynamic_cast <UtlString*> (nvPairs.findValue(&gVersionKey));
    UtlString* cAcceptStr = dynamic_cast <UtlString*> (nvPairs.findValue(&gAcceptKey));

    // Get the remaining fields so that we can substitute the null string
    // if the fetched value is 0 (the null pointer) because the field
    // is not present in the disk file.
    UtlString* uri = dynamic_cast <UtlString*> (nvPairs.findValue(&gUriKey));
    UtlString* callId = dynamic_cast <UtlString*> (nvPairs.findValue(&gCallidKey));
    UtlString* contact = dynamic_cast <UtlString*> (nvPairs.findValue(&gContactKey));
    UtlString* eventTypeKey = dynamic_cast <UtlString*> (nvPairs.findValue(&gEventtypekeyKey));
    UtlString* eventType = dynamic_cast <UtlString*> (nvPairs.findValue(&gEventtypeKey));
    UtlString* id = dynamic_cast <UtlString*> (nvPairs.findValue(&gIdKey));
    UtlString* to = dynamic_cast <UtlString*> (nvPairs.findValue(&gToKey));
    UtlString* from = dynamic_cast <UtlString*> (nvPairs.findValue(&gFromKey));
    UtlString* key = dynamic_cast <UtlString*> (nvPairs.findValue(&gKeyKey));
    UtlString* recordRoute = dynamic_cast <UtlString*> (nvPairs.findValue(&gRecordrouteKey));

    // Note: identity inferred from the uri
    return insertRow (
        cComponent ? *cComponent : sComponentStatus,
        uri ? *uri : nullString,
        callId ? *callId : nullString,
        contact ? *contact : nullString,
        expStr ? atoi(expStr->data()) : 0,
        cSubseqStr ? atoi(cSubseqStr->data()) : 0,
        eventTypeKey ? *eventTypeKey : nullString,
        eventType ? *eventType : nullString,
        id ? *id : nullString,
        to ? *to : nullString,
        from ? *from : nullString,
        key ? *key : nullString,
        recordRoute ? *recordRoute : nullString,
        cNotifySeqStr ? atoi(cNotifySeqStr->data()) : 0,
        cAcceptStr ? *cAcceptStr : sAcceptSimpleMessage,
        cVersionStr ? atoi(cVersionStr->data()) : 0);
}

UtlBoolean
SubscriptionDB::insertRow (
    const UtlString& component,
    const UtlString& uri,
    const UtlString& callid,
    const UtlString& contact,
    const int& expires,
    const int& subscribeCseq,
    const UtlString& eventTypeKey,
    const UtlString& eventType,
    const UtlString& id,
    const UtlString& to,
    const UtlString& from,
    const UtlString& key,
    const UtlString& recordRoute,
    const int& notifyCseq,
    const UtlString& accept,
    const int& version)
{
    UtlBoolean result = FALSE;
    if ( !uri.isNull() && ( m_pFastDB != NULL ) )
    {
        int timeNow = (int)OsDateTime::getSecsSinceEpoch();

        // Thread Local Storage
        m_pFastDB->attach();

        // get rid of any expired subscriptions before searching
        removeExpiredInternal(component, timeNow);

        // Search for a matching row before deciding to update or insert
        // query all sessions (should only be one here)
        dbQuery existingQuery;

#       ifdef TEST_DEBUG
        OsSysLog::add(FAC_DB, PRI_DEBUG, "SubscriptionDB::insertRow <<<<<<<<<<<< query:\n"
                      "  component='%s'\n"
                      "  toUri='%s'\n"
                      "  fromUri='%s'\n"
                      "  callid='%s'\n"
                      "  eventtypekey='%s'\n"
                      "  eventtype='%s'\n"
                      "  id='%s'",
                      component.data(),
                      to.data(),
                      from.data(),
                      callid.data(),
                      eventTypeKey.data(),
                      eventType.data(),
                      ( id.isNull() ? SPECIAL_IMDB_NULL_VALUE : id.data())
                      );
#       endif

        // Query does not need to include component, because Call-Id and tags
        // are unique for all subscriptions.
        existingQuery =
              "toUri=",to,
              "and fromUri=",from,
              "and callid=",callid,
              "and eventtype=",eventType,
              "and id=",( id.isNull() ? SPECIAL_IMDB_NULL_VALUE : id.data() );

        dbCursor< SubscriptionRow > existingCursor( dbCursorForUpdate );
        int existing = existingCursor.select( existingQuery );
        if ( existing > 0 )
        {
#          ifdef TEST_DEBUG
           UtlString matchRows;
#          endif
            // Should only be one row, only updating this
            do
            {
#               ifdef TEST_DEBUG
                matchRows.append(" callid '");
                matchRows.append(existingCursor->callid);
                matchRows.append("' cseq ");
                char logCseq[10];
                sprintf(logCseq, "%d", existingCursor->subscribecseq);
                matchRows.append(logCseq);
                matchRows.append(" contact '");
                matchRows.append(existingCursor->contact);
                matchRows.append("'"); // :TODO:
#               endif
                // only update the row if the subscribe is newer
                // than the last IMDB update
                if ( existingCursor->subscribecseq < subscribeCseq )
                { // refreshing subscribe request
                    existingCursor->uri = uri;
                    existingCursor->expires = static_cast<const int&>(expires);
                    existingCursor->subscribecseq = subscribeCseq;
                    existingCursor->recordroute = recordRoute;
                    existingCursor->contact = contact;
                    existingCursor->version = version;
                    existingCursor.update();
#                   ifdef TEST_DEBUG
                    matchRows.append(" UPDATED");
#                   endif
                } // do nothing as the the input cseq is <= the db cseq
            } while ( existingCursor.nextAvailable() );

            OsSysLog::add(FAC_DB, PRI_DEBUG, "SubscriptionDB::insertRow found %d rows"
#                         ifdef TEST_DEBUG
                          ": %s"
#                         endif
                          ,existing
#                         ifdef TEST_DEBUG
                          ,matchRows.data()
#                         endif
                          );
        }
        else
        {
            // Insert new row
            SubscriptionRow row;
            row.component = component;
            row.callid = callid;
            row.contact = contact;
            row.expires = expires;
            row.uri = uri;
            row.subscribecseq = subscribeCseq;
            row.notifycseq = notifyCseq;
            row.eventtypekey = eventTypeKey;
            row.eventtype = eventType;
            row.id = ( id.isNull() ? SPECIAL_IMDB_NULL_VALUE : id.data() );
            row.fromUri = from;
            row.key = key;
            row.toUri = to;
            row.recordroute = recordRoute;
            row.accept = accept;
            row.version = version;
            insert (row);

#           ifdef TEST_DEBUG
            OsSysLog::add(FAC_DB, PRI_DEBUG, "SubscriptionDB::insertRow <<<<<<<<<<<< added:\n"
                          "  component='%s'\n"
                          "  to='%s'\n"
                          "  from='%s'\n"
                          "  callid='%s'\n"
                          "  eventtypekey='%s'\n"
                          "  eventtype='%s'\n"
                          "  id='%s'\n"
                          "  contact='%s'\n"
                          "  expires=%d\n"
                          "  uri='%s'\n"
                          "  subscribecseq=%d\n"
                          "  notifycseq=%d\n"
                          "  key='%s'\n"
                          "  recordroute='%s'\n"
                          "  accept='%s'\n"
                          "  version=%d",
                          component.data(),
                          to.data(),
                          from.data(),
                          callid.data(),
                          eventTypeKey.data(),
                          eventType.data(),
                          (id.isNull() ? SPECIAL_IMDB_NULL_VALUE : id.data()),
                          contact.data(),
                          expires,
                          uri.data(),
                          subscribeCseq,
                          notifyCseq,
                          key.data(),
                          recordRoute.data(),
                          accept.data(),
                          version
               );
#           endif
        }
        // Either did an insert or an update
        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);

        result = TRUE;
    }
    // Most likely an arg problem
    return result;
}

void
SubscriptionDB::removeRow (
    const UtlString& component,
    const UtlString& to,
    const UtlString& from,
    const UtlString& callid,
    const int& subscribeCseq )
{
    if ( m_pFastDB != NULL )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        dbCursor< SubscriptionRow > cursor(dbCursorForUpdate);

        // ensure we filter off of the subcribecseq since there
        // could be multiple removes from the IMDB and the one with
        // the highest cseq should be remain, note that the
        // < subscribeCseq comparison is important since under UDP conditions
        // the Status server may be busy and UDP may retransmit the
        // same message multiple times, this would cause the just subscribed row
        // to be incorrectly removed while the status server was sending down
        // its acknowledgement

#       ifdef TEST_DEBUG
        OsSysLog::add(FAC_DB, PRI_DEBUG, "SubscriptionDB::removeRow <<<<<<<<<<<< query:\n"
                      "  toUri='%s'\n"
                      "  fromUri='%s'\n"
                      "  callid='%s'\n"
                      "  subscribecseq='%d'",
                      to.data(),
                      from.data(),
                      callid.data(),
                      subscribeCseq
                      );
#       endif

        dbQuery query;
        // Query does not need to include component, because Call-Id and tags
        // are unique for all subscriptions.
        query="toUri=",to,
              "and fromUri=",from,
              "and callid=",callid,
              "and subscribecseq <",subscribeCseq;
        if (cursor.select(query) > 0)
        {
            cursor.removeAllSelected();
        }
        else
        {
           OsSysLog::add(FAC_DB, PRI_DEBUG, "SubscriptionDB::removeRow row not found:\n"
                         "to='%s' from='%s' callid='%s' cseq=%d",
                         to.data(), from.data(), callid.data(),
                         subscribeCseq
                         );
        }

        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }
}

void
SubscriptionDB::removeErrorRow (
   const UtlString& component,
   const UtlString& to,
   const UtlString& from,
   const UtlString& callid )
{
   if ( m_pFastDB != NULL )
   {
      // Thread Local Storage
      m_pFastDB->attach();

      dbCursor< SubscriptionRow > cursor(dbCursorForUpdate);

      dbQuery query;
      // Query does not need to include component, because Call-Id and tags
      // are unique for all subscriptions.
      query="toUri=",to,
         "and fromUri=",from,
         "and callid=",callid;
      if (cursor.select(query) > 0)
      {
#if 1 // :TODO: remove debug trace
         UtlString errorRows;
         do
         {
            errorRows.append("\n toUri '");
            errorRows.append(cursor->toUri);
            errorRows.append("' fromUri '");
            errorRows.append(cursor->fromUri);
            errorRows.append("' callid '");
            errorRows.append(cursor->callid);
            errorRows.append("'");
         } while (cursor.next());

         OsSysLog::add(FAC_DB, PRI_DEBUG, "SubscriptionDB::removeErrorRow rows: %s",
                       errorRows.data()
                       );
         cursor.removeAllSelected();
#endif  // :TODO: remove debug trace
      }
      else
      {
         OsSysLog::add(FAC_DB, PRI_DEBUG, "SubscriptionDB::removeErrorRow row not found:\n"
                       "to='%s' from='%s' callid='%s'",
                       to.data(), from.data(), callid.data()
                       );
      }

      // Commit rows to memory - multiprocess workaround
      m_pFastDB->detach(0);
   }
}

UtlBoolean
SubscriptionDB::subscriptionExists (
   const UtlString& component,
   const UtlString& to,
   const UtlString& from,
   const UtlString& callid,
   const int timeNow )
{
   UtlBoolean ret = FALSE;

   if ( m_pFastDB != NULL )
   {
      // Thread Local Storage
      m_pFastDB->attach();

      dbCursor< SubscriptionRow > cursor(dbCursorForUpdate);

      dbQuery query;
      // Query does not need to include component, because Call-Id and tags
      // are unique for all subscriptions.
      query="toUri=", to,
         "and fromUri=", from,
         "and callid=", callid,
         "and expires>=", timeNow;
      ret = cursor.select(query) > 0;

      // Commit rows to memory - multiprocess workaround
      m_pFastDB->detach(0);
   }

   return ret;
}

void
SubscriptionDB::removeRows (
   const UtlString& key )
{
    if ( !key.isNull() && (m_pFastDB != NULL) )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        dbCursor< SubscriptionRow > cursor(dbCursorForUpdate);

        dbQuery query;
        query="key=",key;
        if (cursor.select(query) > 0)
        {
            cursor.removeAllSelected();
        }
        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }
}

void
SubscriptionDB::removeAllRows ()
{
    if ( m_pFastDB != NULL )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        dbCursor< SubscriptionRow > cursor( dbCursorForUpdate );
        if (cursor.select() > 0)
        {
            cursor.removeAllSelected();
        }
        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }
}



void
SubscriptionDB::removeExpired( const UtlString& component,
                               const int timeNow )
{
   if (m_pFastDB != NULL)
   {
      // Thread Local Storage
      m_pFastDB->attach();

      // remove all expired subscriptions
      removeExpiredInternal(component, timeNow);

      // Commit rows to memory - multiprocess workaround
      m_pFastDB->detach(0);
   }
}

void
SubscriptionDB::getAllRows ( ResultSet& rResultSet ) const
{
    // Clear the results
    rResultSet.destroyAll();

    if ( m_pFastDB != NULL )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        dbCursor< SubscriptionRow > cursor;
        if ( cursor.select() > 0 )
        {
            do {
                UtlHashMap record;
                UtlString* componentValue =
                    new UtlString ( cursor->component );
                UtlString* uriValue =
                    new UtlString ( cursor->uri );
                UtlString* callidValue =
                    new UtlString ( cursor->callid );
                UtlString* contactValue =
                    new UtlString ( cursor->contact );
                UtlInt* expiresValue =
                    new UtlInt ( cursor->expires );
                UtlInt* subscribecseqValue =
                    new UtlInt ( cursor->subscribecseq );
                UtlString* eventtypekeyValue =
                    new UtlString ( cursor->eventtypekey );
                UtlString* eventtypeValue =
                    new UtlString ( cursor->eventtype );
                UtlString* idValue =
                    new UtlString ( cursor->id );
                UtlString* toValue =
                    new UtlString ( cursor->toUri );
                UtlString* fromValue =
                    new UtlString ( cursor->fromUri );
                UtlString* keyValue =
                    new UtlString ( cursor->key );
                UtlString* recordrouteValue =
                    new UtlString ( cursor->recordroute );
                UtlInt* notifycseqValue =
                    new UtlInt ( cursor->notifycseq );
                UtlString* acceptValue =
                    new UtlString ( cursor->accept );
                UtlInt* versionValue =
                    new UtlInt ( cursor->version );

                // Memory Leak fixes, make shallow copies of static keys
                UtlString* componentKey = new UtlString( gComponentKey );
                UtlString* uriKey = new UtlString( gUriKey );
                UtlString* callidKey = new UtlString( gCallidKey );
                UtlString* contactKey = new UtlString( gContactKey );
                UtlString* expiresKey = new UtlString( gExpiresKey );
                UtlString* subscribecseqKey = new UtlString( gSubscribecseqKey );
                UtlString* eventtypekeyKey = new UtlString( gEventtypekeyKey );
                UtlString* eventtypeKey = new UtlString( gEventtypeKey );
                UtlString* idKey = new UtlString( gIdKey );
                UtlString* toKey = new UtlString( gToKey );
                UtlString* fromKey = new UtlString( gFromKey );
                UtlString* keyKey = new UtlString( gKeyKey );
                UtlString* recordrouteKey = new UtlString( gRecordrouteKey );
                UtlString* notifycseqKey = new UtlString( gNotifycseqKey );
                UtlString* acceptKey = new UtlString( gAcceptKey );
                UtlString* versionKey = new UtlString( gVersionKey );

                record.insertKeyAndValue (
                    componentKey, componentValue);
                record.insertKeyAndValue (
                    uriKey, uriValue);
                record.insertKeyAndValue (
                    callidKey, callidValue);
                record.insertKeyAndValue (
                    contactKey, contactValue);
                record.insertKeyAndValue (
                    expiresKey, expiresValue);
                record.insertKeyAndValue (
                    subscribecseqKey, subscribecseqValue);
                record.insertKeyAndValue (
                    eventtypekeyKey, eventtypekeyValue);
                record.insertKeyAndValue (
                    eventtypeKey, eventtypeValue);
                record.insertKeyAndValue (
                    idKey, idValue);
                record.insertKeyAndValue (
                    toKey, toValue);
                record.insertKeyAndValue (
                    fromKey, fromValue);
                record.insertKeyAndValue (
                    keyKey, keyValue);
                record.insertKeyAndValue (
                    recordrouteKey, recordrouteValue);
                record.insertKeyAndValue (
                    notifycseqKey, notifycseqValue);
                record.insertKeyAndValue (
                    acceptKey, acceptValue);
                record.insertKeyAndValue (
                    versionKey, versionValue);

                rResultSet.addValue(record);
            } while (cursor.next());
        }
        // Commit the tx
        m_pFastDB->detach(0);
    }
}

void
SubscriptionDB::updateNotifyUnexpiredSubscription (
        const UtlString& component,
        const UtlString& to,
        const UtlString& from,
        const UtlString& callid,
        const UtlString& eventType,
        const UtlString& id,
        int timeNow,
        int updatedNotifyCseq,
        int version ) const
{
    if ( m_pFastDB != NULL )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        // Create an update cursor to purge the DB
        // of all expired contacts. This is a hack since this
        // should be implemented via a daemon garbage collector
        // thread.
        dbCursor< SubscriptionRow > cursor(dbCursorForUpdate);

#       ifdef TEST_DEBUG
        OsSysLog::add(FAC_DB, PRI_DEBUG, "SubscriptionDB::updateNotifyUnexpiredSubscription <<<<<<<<<<<< query:\n"
                      "  toUri='%s'\n"
                      "  callid='%s'\n"
                      "  eventtype='%s'\n"
                      "  id='%s'",
                      to.data(),
                      callid.data(),
                      eventType.data(),
                      ( id.isNull() ? SPECIAL_IMDB_NULL_VALUE : id.data())
                      );
#       endif

        dbQuery query;
        // Query does not need to include component, because Call-Id and tags
        // are unique for all subscriptions.
        // This query used to test the 'from' field, but it is not represented
        // consistently and so is hard to use.  In reality, the call-id should
        // be unique.  Yet another hassle that should be fixed by indexing
        // the subscriptions by call-id, to-tag, from-tag, event, and event-id.
        query="toUri=",to,
              "and callid=",callid,
              "and eventtype=",eventType,
              "and id=",(id.isNull() ? SPECIAL_IMDB_NULL_VALUE : id.data())
	  ;
        if ( cursor.select(query) > 0 )
        {
            do {
                // Purge any expired rows
                if ( cursor->expires < timeNow )
                {
                    // note cursor.remove() auto updates the database
                    cursor.remove();
                    OsSysLog::add(FAC_DB, PRI_DEBUG, "SubscriptionDB::updateNotifyUnexpiredSubscription removing expired row.");
                } else
                {
                   // Update the NotifySCeq Number.
                   cursor->notifycseq = updatedNotifyCseq;
                   // Update the XML internal version number.
                   cursor->version = version;
                   cursor.update();
                   OsSysLog::add(FAC_DB, PRI_DEBUG,
                                 "SubscriptionDB::updateNotifyUnexpiredSubscription updating notifycseq for '%s' to: %d",
                                 callid.data(), updatedNotifyCseq);
                }

                // Next replaced with nextAvailable - better when
                // selective updates applied to the cursor object
            } while ( cursor.nextAvailable() );
        }

        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }
}

void
SubscriptionDB::getUnexpiredContactsFieldsContaining (
   UtlString& substringToMatch,
   const int& timeNow,
   UtlSList& matchingContactFields ) const
{
   // Clear the results
   matchingContactFields.destroyAll();
   if ( m_pFastDB != NULL )
   {
       // Thread Local Storage
       m_pFastDB->attach();

       dbCursor< SubscriptionRow > cursor;
       UtlString queryString = "contact like '%" + substringToMatch + "%' and expires>";
       queryString.appendNumber( timeNow );
       dbQuery query;
       query = queryString;

       if ( cursor.select(query) > 0 )
       {
           // Copy all the unexpired contacts into the result list
           do
           {
               UtlString* contactValue = new UtlString(cursor->contact);
               matchingContactFields.append( contactValue );
           } while ( cursor.next() );
       }

        m_pFastDB->detach(0);
   }
   else
   {
      OsSysLog::add(FAC_DB, PRI_CRIT, "SubscriptionDB::getUnexpiredContactsFieldsContaining failed - no DB");
   }
}

UtlBoolean
SubscriptionDB::updateSubscribeUnexpiredSubscription (
        const UtlString& component,
        const UtlString& to,
        const UtlString& from,
        const UtlString& callid,
        const UtlString& eventType,
        const UtlString& id,
        const int& timeNow,
        const int& expires,
        const int& updatedSubscribeCseq ) const
{
    bool ret = FALSE;

    if ( m_pFastDB != NULL )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        // Create an update cursor to purge the DB
        // of all expired contacts. This is a hack since this
        // should be implemented via a daemon garbage collector
        // thread.
        dbCursor< SubscriptionRow > cursor(dbCursorForUpdate);

#       ifdef TEST_DEBUG
        OsSysLog::add(FAC_DB, PRI_DEBUG, "SubscriptionDB::updateSubscribeUnexpiredSubscription <<<<<<<<<<<< query:\n"
                      "  toUri='%s'\n"
                      "  fromUri='%s'\n"
                      "  callid='%s'\n"
                      "  eventtype='%s'\n"
                      "  id='%s'",
                      to.data(),
                      from.data(),
                      callid.data(),
                      eventType.data(),
                      ( id.isNull() ? SPECIAL_IMDB_NULL_VALUE : id.data())
                      );
#       endif

        dbQuery query;
        // Query does not need to include component, because Call-Id and tags
        // are unique for all subscriptions.
        query="toUri=",to,
              "and fromUri=",from,
              "and callid=",callid,
              "and eventtype=",eventType,
              "and id=",(id.isNull() ? SPECIAL_IMDB_NULL_VALUE : id.data())
	  ;
        if ( cursor.select(query) > 0 )
        {
            do {
                // Purge any expired rows
                if ( cursor->expires < timeNow )
                {
                    // note cursor.remove() auto updates the database
                    cursor.remove();
                } else
                {
                   cursor->expires = expires;
                   cursor->subscribecseq = updatedSubscribeCseq;
                   // Remember that we changed a row.
                   ret = true;
                   // Update to the new NotifySCeq Number
                   cursor.update();
                }

                // Next replaced with nextAvailable - better when
                // selective updates applied to the cursor object
            } while ( cursor.nextAvailable() );
        }
        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }

    return ret;
}

void
SubscriptionDB::getUnexpiredSubscriptions (
    const UtlString& component,
    const UtlString& key,
    const UtlString& eventType,
    const int& timeNow,
    ResultSet& rResultSet )
{
    // Clear the results
    rResultSet.destroyAll();

    if ( !key.isNull() && (m_pFastDB != NULL) )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        // remove all expired subscriptions
        removeExpiredInternal(component, timeNow);

        // Now select the remaining current events
        dbCursor< SubscriptionRow > cursor( dbCursorForUpdate );
        dbQuery query;
        query="key=",key,"and eventtype=",eventType;
        int foundRows = cursor.select(query);
        if ( foundRows > 0 )
        {
            do {
                UtlHashMap record;
                UtlString* componentValue =
                    new UtlString ( cursor->component );
                UtlString* uriValue =
                    new UtlString ( cursor->uri );
                UtlString* callidValue =
                    new UtlString ( cursor->callid );
                UtlString* contactValue =
                    new UtlString ( cursor->contact );
                UtlInt* expiresValue =
                    new UtlInt ( cursor->expires - timeNow );
                UtlInt* subscribecseqValue =
                    new UtlInt ( cursor->subscribecseq );
                UtlString* eventtypeValue =
                    new UtlString ( cursor->eventtype );
                UtlString* idValue = new UtlString(
                  (  (0 == strcmp(cursor->id, SPECIAL_IMDB_NULL_VALUE))
                   ? ""
                   : cursor->id
                   ));
                UtlString* toValue =
                    new UtlString ( cursor->toUri );
                UtlString* fromValue =
                    new UtlString ( cursor->fromUri );
                UtlString* keyValue =
                    new UtlString ( cursor->key );
                UtlString* recordrouteValue =
                    new UtlString ( cursor->recordroute );
                UtlInt* notifycseqValue =
                    new UtlInt ( cursor->notifycseq );
                UtlString* acceptValue =
                    new UtlString ( cursor->accept );

                // Memory Leak fixes, make shallow copies of static keys
                UtlString* componentKey = new UtlString( gComponentKey );
                UtlString* uriKey = new UtlString( gUriKey );
                UtlString* callidKey = new UtlString( gCallidKey );
                UtlString* contactKey = new UtlString( gContactKey );
                UtlString* expiresKey = new UtlString( gExpiresKey );
                UtlString* subscribecseqKey = new UtlString( gSubscribecseqKey );
                UtlString* eventtypeKey = new UtlString( gEventtypeKey );
                UtlString* idKey = new UtlString( gIdKey );
                UtlString* toKey = new UtlString( gToKey );
                UtlString* fromKey = new UtlString( gFromKey );
                UtlString* keyKey = new UtlString( gKeyKey );
                UtlString* recordrouteKey = new UtlString( gRecordrouteKey );
                UtlString* notifycseqKey = new UtlString( gNotifycseqKey );
                UtlString* acceptKey = new UtlString( gAcceptKey );

                record.insertKeyAndValue (
                    componentKey, componentValue);
                record.insertKeyAndValue (
                    uriKey, uriValue);
                record.insertKeyAndValue (
                    callidKey, callidValue);
                record.insertKeyAndValue (
                    contactKey, contactValue);
                record.insertKeyAndValue (
                    expiresKey, expiresValue);
                record.insertKeyAndValue (
                    subscribecseqKey, subscribecseqValue);
                record.insertKeyAndValue (
                    eventtypeKey, eventtypeValue);
                record.insertKeyAndValue (
                    idKey, idValue);
                record.insertKeyAndValue (
                    toKey, toValue);
                record.insertKeyAndValue (
                    fromKey, fromValue);
                record.insertKeyAndValue (
                    keyKey, keyValue);
                record.insertKeyAndValue (
                    recordrouteKey, recordrouteValue);
                record.insertKeyAndValue (
                    notifycseqKey, notifycseqValue);
                record.insertKeyAndValue (
                    acceptKey, acceptValue);

                rResultSet.addValue(record);
            } while ( cursor.nextAvailable() );
        }
        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }
}

// Update the From and To fields of entries containing
// a call-id and from-tag.  (The to-tag is not used, as
// this call may be adding the to-tag.)
void SubscriptionDB::updateToTag(
   const UtlString& callid,
   const UtlString& fromtag,
   const UtlString& totag
   ) const
{
   bool match_found = FALSE;
   OsSysLog::add(FAC_DB, PRI_DEBUG, "SubscriptionDB::updateToTag callid = '%s', fromtag = '%s', totag = '%s'",
                 callid.data(), fromtag.data(), totag.data());

   if ( m_pFastDB != NULL )
   {
      // Thread Local Storage
      m_pFastDB->attach();

      // Create an update cursor to purge the DB
      // of all expired contacts. This is a hack since this
      // should be implemented via a daemon garbage collector
      // thread.
      dbCursor< SubscriptionRow > cursor(dbCursorForUpdate);

      dbQuery query;
      // Query includes only the Call-Id.
      query="callid=",callid
         ;
      if ( cursor.select(query) > 0 )
      {
         do {
            UtlBoolean r;
            UtlString seen_tag;

            // Get the tag on the URI in the "from" column.
            Url from_uri(cursor->fromUri, FALSE);
            r = from_uri.getFieldParameter("tag", seen_tag);
            OsSysLog::add(FAC_DB, PRI_DEBUG, "SubscriptionDB::updateToTag cursor->fromUri = '%s', cursor->toUri = '%s', seen_tag = '%s'",
                          cursor->fromUri, cursor->toUri, seen_tag.data());

            // If it matches...
            if (r && seen_tag.compareTo(fromtag) == 0)
            {
               // Update the row by adding the to-tag (if it doesn't have one).
               Url toUri(cursor->toUri, FALSE); // parse as name-addr
               UtlString dummy;
               if (!toUri.getFieldParameter("tag", dummy))
               {
                  toUri.setFieldParameter("tag", totag);
                  toUri.toString(dummy); // un-parse as name-addr
                  OsSysLog::add(FAC_DB, PRI_DEBUG, "SubscriptionDB::updateToTag cursor->toUri = '%s'",
                                dummy.data());
                  cursor->toUri = dummy.data();
                  cursor.update();
               }
            }
         } while (!match_found && cursor.nextAvailable());
      }
      // Commit rows to memory - multiprocess workaround
      m_pFastDB->detach(0);
   }
}

// Find the full From and To values given from- and to-tags.
// Returns true if values are found, and sets "from" and "to".
UtlBoolean SubscriptionDB::findFromAndTo(
   const UtlString& callid,
   const UtlString& fromtag,
   const UtlString& totag,
   UtlString& from,
   UtlString& to) const
{
   bool match_found = FALSE;

   if ( m_pFastDB != NULL )
   {
      // Thread Local Storage
      m_pFastDB->attach();

      dbCursor< SubscriptionRow > cursor(dbCursorForUpdate);

      dbQuery query;
      // Query includes only the Call-Id.
      query="callid=",callid
         ;
      if ( cursor.select(query) > 0 )
      {
         do {
            UtlBoolean r;
            UtlString seen_tag;

            // Get the tag on the URI in the "from" column.
            Url fromUri(cursor->fromUri, FALSE);
            r = fromUri.getFieldParameter("tag", seen_tag);

            // If it matches...
            if (r && seen_tag.compareTo(fromtag) == 0)
            {
               // Get the tag on the URI in the "to" column.
               Url toUri(cursor->toUri, FALSE);
               r = toUri.getFieldParameter("tag", seen_tag);

               // If it matches...
               if (r && seen_tag.compareTo(totag) == 0)
               {
                  // We have found a match.  Record the full URIs.
                  match_found = TRUE;
                  from = cursor->fromUri;
                  to = cursor->toUri;
               }
            }
         } while (!match_found && cursor.nextAvailable());
      }
      // Commit rows to memory - multiprocess workaround
      m_pFastDB->detach(0);
   }

   return match_found;
}

// Get the maximum of the <version> values whose <uri> matches
// 'uri'.
int SubscriptionDB::getMaxVersion(
   const UtlString& uri) const
{
   int value = 0;

   if ( m_pFastDB != NULL )
   {
      // Thread Local Storage
      m_pFastDB->attach();

      dbCursor< SubscriptionRow > cursor(dbCursorForUpdate);

      dbQuery query;
      // Query includes only the Uri
      query="uri=",uri
         ;
      if ( cursor.select(query) > 0 )
      {
         do {
            int v = cursor->version;

            if (v > value)
            {
               value = v;
            }
         } while (cursor.nextAvailable());
      }
      // Commit rows to memory - multiprocess workaround
      m_pFastDB->detach(0);
   }

   return value;
}

/// Clean out any expired rows -- caller must hold lock and have db attached.
void SubscriptionDB::removeExpiredInternal( const UtlString& component,
                                            const int timeNow )
{
   dbCursor< SubscriptionRow > expireCursor( dbCursorForUpdate );
   dbQuery expireQuery;

   // Firstly purge all expired field entries from the DB that are expired for this identity
   // we can not purge all rows because a Notify should be to user
   expireQuery = "component=", component,
                 "and expires <", static_cast<const int>(timeNow);
   int expiring = expireCursor.select( expireQuery );
   if ( expiring > 0 )
   {
      OsSysLog::add(FAC_DB, PRI_DEBUG,
                    "SubscriptionDB::removeExpiredInternal removing %d expired subscriptions",
                    expiring
                    );

      expireCursor.removeAllSelected();
   }
}

bool
SubscriptionDB::isLoaded()
{
    return mTableLoaded;
}

SubscriptionDB*
SubscriptionDB::getInstance( const UtlString& name )
{
    // Critical Section here
    OsLock lock( sLockMutex );

    // See if this is the first time through for this process
    // Note that this being null => pgDatabase is also null
    if ( spInstance == NULL )
    {
        // Create the singleton class for clients to use
        spInstance = new SubscriptionDB ( name );
    }
    return spInstance;
}
