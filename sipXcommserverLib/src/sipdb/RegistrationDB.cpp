//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdlib.h>

// APPLICATION INCLUDES
#include "utl/UtlInt.h"
#include "utl/UtlLongLongInt.h"
#include "utl/UtlSListIterator.h"
#include "os/OsLock.h"
#include "os/OsDateTime.h"
#include "os/OsFS.h"
#include "os/OsSysLog.h"
#include "os/OsTimeLog.h"
#include "net/Url.h"
#include "fastdb/fastdb.h"

#include "xmlparser/tinyxml.h"
#include "sipXecsService/SipXecsService.h"
#include "sipdb/RegistrationBinding.h"
#include "sipdb/RegistrationDB.h"
#include "sipdb/RegistrationRow.h"
#include "sipdb/ResultSet.h"
#include "sipdb/SIPDBManager.h"

REGISTER( RegistrationRow );


// Smart DB accessor that attaches the DB on construction, and detaches on destruction.
// This is necessary to protect process/thread-local storage integrity.
class SmartDbAccessor
{
public:
   SmartDbAccessor(dbDatabase* fastDB, int lineNo) : mFastDB(fastDB), mLineNo(lineNo)
      {
         if (mFastDB != NULL)
         {
            mFastDB->attach();
         }
         else
         {
            OsSysLog::add(FAC_DB, PRI_CRIT, "SmartDbAccessor:: line %d - no DB", lineNo);
         }
      }
   ~SmartDbAccessor()
      {
         if (mFastDB != NULL)
         {
            mFastDB->detach(0);
         }
      }

private:
   dbDatabase* mFastDB;
   int         mLineNo;
};

#define SMART_DB_ACCESS SmartDbAccessor accessor(m_pFastDB, __LINE__)


// Static Initializers
RegistrationDB* RegistrationDB::spInstance = NULL;
OsMutex         RegistrationDB::sLockMutex (OsMutex::Q_FIFO);

const UtlString RegistrationDB::gIdentityKey("identity");
const UtlString RegistrationDB::gUriKey("uri");
const UtlString RegistrationDB::gCallidKey("callid");
const UtlString RegistrationDB::gContactKey("contact");
const UtlString RegistrationDB::gQvalueKey("qvalue");
const UtlString RegistrationDB::gInstanceIdKey("instance_id");
const UtlString RegistrationDB::gGruuKey("gruu");
const UtlString RegistrationDB::gPathKey("path");
const UtlString RegistrationDB::gCseqKey("cseq");
const UtlString RegistrationDB::gExpiresKey("expires");
const UtlString RegistrationDB::gPrimaryKey("primary");
const UtlString RegistrationDB::gUpdateNumberKey("update_number");
const UtlString RegistrationDB::gInstrumentKey("instrument");

const UtlString RegistrationDB::nullString("");
const UtlString RegistrationDB::percent("%");

// The 'type' attribute of the top-level 'items' element.
const UtlString RegistrationDB::sType("registration");

// The XML namespace of the top-level 'items' element.
const UtlString RegistrationDB::sXmlNamespace("http://www.sipfoundry.org/sipX/schema/xml/registration-00-00");

/* ============================ CREATORS ================================== */

/*
 *Function Name: RegistrationDB Constructor
 *
 *Parameters:
 *
 *Description: This is a protected method as this is a singleton
 *
 *Returns:
 *
 */
RegistrationDB::RegistrationDB( const UtlString& name ) :
   mDatabaseName( name ),
   mTableLoaded ( true )
{
    // Access the shared table database
    SIPDBManager* pSIPDBManager = SIPDBManager::getInstance();
    m_pFastDB = pSIPDBManager->getDatabase(name);

    // If we are the first process to attach
    // then we need to load the DB
    int users = pSIPDBManager->getNumDatabaseProcesses(name);
    OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                  "RegistrationDB::_ users = %d, mTableLoaded = %d",
                  users, mTableLoaded);
    if ( users == 1 || ( users > 1 && mTableLoaded == false ) )
    {
        OsSysLog::add(FAC_DB, PRI_DEBUG, "RegistrationDB::_ about to load");
        mTableLoaded = false;
        // Load the file implicitly
        this->load();
        // the RegistrationDB is not replicated from sipXconfig, as
        // a result, make this table appear as being loaded regardless
        // of the load() result.
        mTableLoaded = true;
        OsSysLog::add(FAC_DB, PRI_DEBUG, "RegistrationDB::_ table successfully loaded");
    }
    OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                  "RegistrationDB::_ rows in table = %d",
                  getRowCount());
}

RegistrationDB::~RegistrationDB()
{
    OsSysLog::add(FAC_DB, PRI_DEBUG, "<><>## RegistrationDB:: DESTRUCTOR");
}

/* ============================ MANIPULATORS ============================== */

void
RegistrationDB::releaseInstance()
{
    OsSysLog::add(FAC_DB, PRI_DEBUG, "<><>## RegistrationDB:: releaseInstance() spInstance=%p", spInstance);

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
RegistrationDB::load()
{
    OsStatus result = OS_SUCCESS;

    if ( m_pFastDB != NULL )
    {
        UtlString fileName = mDatabaseName + ".xml";
        UtlString pathName = SipXecsService::Path(SipXecsService::DatabaseDirType,
                                                  fileName.data());

        OsSysLog::add(FAC_DB, PRI_DEBUG, "RegistrationDB::load loading \"%s\"",
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
           // non-existence of the file is not an error for this DB
           if (OsFileSystem::exists(pathName))
           {
                OsSysLog::add(FAC_DB, PRI_WARNING, "RegistrationDB::load TiXmlDocument::LoadFile() failed for file '%s'",
                        pathName.data());
                result = OS_FAILED;
           }
        }
    } else
    {
        OsSysLog::add(FAC_DB, PRI_CRIT, "RegistrationDB::load failed - no DB");
        result = OS_FAILED;
    }
    return result;
}

/// Garbage collect and persist database
///
/// Garbage collect - delete all rows older than the specified
/// time, and then write all remaining entries to the persistent
/// data store (xml file).
OsStatus RegistrationDB::cleanAndPersist( int newerThanTime )
{
    OsStatus result = OS_SUCCESS;

    if ( m_pFastDB != NULL )
    {
        SMART_DB_ACCESS;

        // Purge all expired field entries from the DB
        // Note: callid set to null indicates provisioned entries and
        //        these should not be removed
        dbCursor< RegistrationRow > expireCursor( dbCursorForUpdate );
        dbQuery query;
        query = "expires <", static_cast<const int&>(newerThanTime), " and (callid != '#')";
        int rows = expireCursor.select( query );
        if ( rows > 0 )
        {
            OsSysLog::add( FAC_SIP, PRI_DEBUG
                          ,"RegistrationDB::cleanAndPersist cleaning out %d rows"
                          ,rows
                          );
            expireCursor.removeAllSelected();
        }

        UtlString fileName = mDatabaseName + ".xml";
        UtlString pathName = SipXecsService::Path(SipXecsService::DatabaseDirType,
                                                  fileName.data());

        // Search our memory for rows
        dbCursor< RegistrationRow > cursor;

        // Select everything in the IMDB and add as item elements if present
        rows = cursor.select();
        if ( rows > 0 )
        {
            OsSysLog::add( FAC_SIP, PRI_DEBUG
                          ,"RegistrationDB::cleanAndPersist writing %d rows"
                          ,rows
                          );

            // Create and start time log.
            OsTimeLog time_log;
            time_log.addEvent("starting");

            // Create an empty document
            TiXmlDocument document;

            // Create a hard coded standalone declaration section
            document.Parse ("<?xml version=\"1.0\" standalone=\"yes\"?>");

            // Create the root node container
            TiXmlElement itemsElement ( "items" );
            itemsElement.SetAttribute( "type", sType.data() );
            itemsElement.SetAttribute( "xmlns", sXmlNamespace.data() );

            int timeNow = OsDateTime::getSecsSinceEpoch();
            itemsElement.SetAttribute( "timestamp", timeNow );

            // metadata contains column names
            dbTableDescriptor* pTableMetaData = &RegistrationRow::dbDescriptor;

            do {
                // Create an item container
                TiXmlElement itemElement ("item");

                byte* base = (byte*)cursor.get();

                // Add the column name value pairs
                for ( dbFieldDescriptor* fd = pTableMetaData->getFirstField();
                      fd != NULL; fd = fd->nextField )
                {
                    // if the column name does not contain the
                    // np_prefix we must persist it
                    if ( strstr( fd->name, "np_" ) == NULL )
                    {
                        // Create a column element named after the IMDB column name
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
            // Attach the root node to the document
            document.InsertEndChild ( itemsElement );

            // Log that we've finished building the XML.
            time_log.addEvent("XML built");

            document.SaveFile ( pathName );

            // Write logging
            time_log.addEvent("file written");
            UtlString text;
            time_log.getLogString(text);
            OsSysLog::add( FAC_SIP, PRI_DEBUG
                           ,"RegistrationDB::cleanAndPersist %s"
                           ,text.data()
               );
        }
        else
        {
            // database contains no rows so delete the file
            if ( OsFileSystem::exists ( pathName ) ) {
                 OsFileSystem::remove( pathName );
            }
        }
    }
    else
    {
        OsSysLog::add(FAC_DB, PRI_CRIT, "RegistrationDB::cleanAndPersist failed - no DB");
        result = OS_FAILED;
    }

    return result;
}

int
RegistrationDB::getRowCount () const
{
   int count = 0;

   // Thread Local Storage
   m_pFastDB->attach();

   dbCursor< RegistrationRow > cursor;

   dbQuery query;
   query="";
   count = cursor.select( query );

   // Commit rows to memory - multiprocess workaround
   m_pFastDB->detach(0);

   return count;
}

void
RegistrationDB::insertRow(const UtlHashMap& nvPairs)
{
    // For integer values, default to 0 if the datum is missing in the input.

    UtlString* expStr = dynamic_cast <UtlString*> (nvPairs.findValue(&gExpiresKey));
    UtlString* cseqStr = dynamic_cast <UtlString*> (nvPairs.findValue(&gCseqKey));

    // If the IMDB does not specify a Q-Value, "%" will be found here
    // (representing a null IMDB column).
    UtlString* qvalue = dynamic_cast <UtlString*> (nvPairs.findValue(&gQvalueKey));

    UtlString* updateNumberStr = dynamic_cast <UtlString*> (nvPairs.findValue(&gUpdateNumberKey));
    // Note that updateNumberStr is likely to start with 0x, so we
    // need the full functionality of strtoll here, not just a
    // decimal-to-binary conversion.  But strtoll is in C99, so it
    // should be OK.
    Int64 updateNumber =
       updateNumberStr ? strtoll(updateNumberStr->data(), NULL, 0) : 0;

    // Get the remaining fields so that we can substitute the null string
    // if the fetched value is 0 (the null pointer) because the field
    // is not present in the disk file.
    UtlString* contact = dynamic_cast <UtlString*> (nvPairs.findValue(&gContactKey));
    UtlString* callId = dynamic_cast <UtlString*> (nvPairs.findValue(&gCallidKey));
    UtlString* instanceId = dynamic_cast <UtlString*> (nvPairs.findValue(&gInstanceIdKey));
    UtlString* gruu = dynamic_cast <UtlString*> (nvPairs.findValue(&gGruuKey));
    UtlString* path = dynamic_cast <UtlString*> (nvPairs.findValue(&gPathKey));
    UtlString* primary = dynamic_cast <UtlString*> (nvPairs.findValue(&gPrimaryKey));

    UtlString* instrument = dynamic_cast <UtlString*> (nvPairs.findValue(&gInstrumentKey));

    // Note: identity inferred from the uri
    updateBinding(
       Url(*(dynamic_cast <UtlString*> (nvPairs.findValue(&gUriKey)))),
       contact ? *contact : nullString,
       qvalue ? *qvalue : percent,
       callId ? *callId : nullString,
       cseqStr ? atoi(cseqStr->data()) : 0,
       expStr ? atoi(expStr->data()) : 0,
       instanceId ? *instanceId : nullString,
       gruu ? *gruu : nullString,
       path ? *path : nullString,
       primary ? *primary : nullString,
       updateNumber,
       instrument ? *instrument : nullString
       );
}


void
RegistrationDB::updateBinding(const RegistrationBinding& reg)
{
   updateBinding(*(reg.getUri()),    // must not be null
                 *(reg.getContact() ? reg.getContact() : &nullString),
                 *(reg.getQvalue() ? reg.getQvalue() : &nullString),
                 *(reg.getCallId() ? reg.getCallId() : &nullString),
                 reg.getCseq(),
                 reg.getExpires(),
                 *(reg.getInstanceId() ? reg.getInstanceId() : &nullString),
                 *(reg.getGruu() ? reg.getGruu() : &nullString),
                 *(reg.getPath() ? reg.getPath() : &nullString),
                 *(reg.getPrimary() ? reg.getPrimary() : &nullString),
                 reg.getUpdateNumber(),
                 *(reg.getInstrument() ? reg.getInstrument() : &nullString));
}


void
RegistrationDB::updateBinding( const Url& uri
                              ,const UtlString& contact
                              ,const UtlString& qvalue
                              ,const UtlString& callid
                              ,const int& cseq
                              ,const int& expires
                              ,const UtlString& instance_id
                              ,const UtlString& gruu
                              ,const UtlString& path
                              ,const UtlString& primary
                              ,const Int64& update_number
                              ,const UtlString& instrument
                              )
{
    UtlString identity;
    uri.getIdentity(identity);
    UtlString fullUri = uri.toString();

    if ( !identity.isNull() && (m_pFastDB != NULL) )
    {
        SMART_DB_ACCESS;

        // Search for a matching row before deciding to update or insert
        dbCursor< RegistrationRow > cursor( dbCursorForUpdate );
        dbQuery query;

        query="np_identity=",identity,
            "  and contact=",contact;
        int existingBinding = cursor.select( query ) > 0;
        RegistrationRow row;
        switch ( existingBinding )
        {
            default:
                // Should not happen - there should either be 1 or none
                OsSysLog::add( FAC_SIP, PRI_ERR,
                              "RegistrationDB::updateBinding %d bindings for %s -> %s"
                              ,existingBinding, identity.data(), contact.data()
                              );
                // recover by clearing them out
                cursor.removeAllSelected();
                // and falling through to insert the new one

            case 0:
                // Insert new row
                row.np_identity   = identity;
                row.uri           = fullUri;
                row.callid        = callid;
                row.cseq          = cseq;
                row.contact       = contact;
                row.qvalue        = qvalue;
                row.expires       = expires;
                row.instance_id   = instance_id;
                row.gruu          = gruu;
                row.path          = path;
                row.primary       = primary;
                row.update_number = update_number;
                row.instrument    = instrument;
                insert (row);
                break;

            case 1:
                // this id->contact binding exists - update it
                cursor->uri           = fullUri;
                cursor->callid        = callid;
                cursor->cseq          = cseq;
                cursor->qvalue        = qvalue;
                cursor->expires       = expires;
                cursor->instance_id   = instance_id;
                cursor->gruu          = gruu;
                cursor->path          = path;
                cursor->primary       = primary;
                cursor->update_number = update_number;
                cursor->instrument    = instrument;
                cursor.update();
                break;
        }
    }
    else
    {
        OsSysLog::add( FAC_SIP, PRI_ERR,
                      "RegistrationDB::updateBinding bad state %s %p"
                      ,identity.data(), m_pFastDB
                      );
    }
}

/// clean out any bindings for this callid and older cseq values.
void
RegistrationDB::expireOldBindings( const Url& uri
                                  ,const UtlString& callid
                                  ,const int& cseq
                                  ,const int& timeNow
                                  ,const UtlString& primary
                                  ,const Int64& update_number
                                  )
{
    UtlString identity;
    uri.getIdentity(identity);

    int expirationTime = timeNow-1;

    if ( !identity.isNull() && ( m_pFastDB != NULL ) )
    {
        SMART_DB_ACCESS;
        dbCursor< RegistrationRow > cursor(dbCursorForUpdate);

        dbQuery query;
        query="np_identity=",identity,
           "and callid=",callid,
           "and cseq<",cseq,
           "and expires>=",expirationTime
           ;
        if (cursor.select(query) > 0)
        {
          do
            {
              cursor->expires = expirationTime;
              cursor->cseq = cseq;
              cursor->primary = primary;
              cursor->update_number = update_number;
              cursor.update();
            } while ( cursor.next() );
        }
    }
    else
    {
       OsSysLog::add(FAC_DB, PRI_CRIT, "RegistrationDB::expireOldBindings failed - no DB");
    }
}

/// expireAllBindings for this URI as of 1 second before timeNow
void RegistrationDB::expireAllBindings( const Url& uri
                                       ,const UtlString& callid
                                       ,const int& cseq
                                       ,const int& timeNow
                                       ,const UtlString& primary
                                       ,const Int64& update_number
                                       )
{
    UtlString identity;
    uri.getIdentity(identity);
    int expirationTime = timeNow-1;

    if ( !identity.isNull() && ( m_pFastDB != NULL ) )
    {
        SMART_DB_ACCESS;
        dbCursor< RegistrationRow > cursor(dbCursorForUpdate);
        dbQuery query;
        query="np_identity=",identity," and expires>=",expirationTime;

        if (cursor.select(query) > 0)
        {
           do
           {
              cursor->expires = expirationTime;
              cursor->callid  = callid;
              cursor->cseq    = cseq;
              cursor->primary = primary;
              cursor->update_number = update_number;
              cursor.update();
           } while ( cursor.next() );
        }
    }
    else
    {
       OsSysLog::add(FAC_DB, PRI_CRIT, "RegistrationDB::expireAllBindings failed - no DB");
    }
}

UtlBoolean
RegistrationDB::isOutOfSequence( const Url& uri
                                ,const UtlString& callid
                                ,const int& cseq
                                ) const
{
  UtlBoolean isOlder;

  UtlString identity;
  uri.getIdentity( identity );

  if ( !identity.isNull() && ( m_pFastDB != NULL) )
    {
      SMART_DB_ACCESS;
      dbCursor< RegistrationRow > cursor;
      dbQuery query;
      query="np_identity=",identity,
        " and callid=",callid,
        " and cseq>=",cseq;
      isOlder = ( cursor.select(query) > 0 );
    }
  else
    {
      OsSysLog::add( FAC_SIP, PRI_ERR,
                    "RegistrationDB::isOutOfSequence bad state @ %d %p"
                    ,__LINE__, m_pFastDB
                    );
      isOlder = TRUE; // will cause 500 Server Internal Error, which is true
    }

  return isOlder;
}


void
RegistrationDB::removeAllRows ()
{
    if ( m_pFastDB != NULL )
    {
        SMART_DB_ACCESS;
        dbCursor< RegistrationRow > cursor( dbCursorForUpdate );
        if (cursor.select() > 0)
        {
            cursor.removeAllSelected();
        }
    }
    else
    {
       OsSysLog::add(FAC_DB, PRI_CRIT, "RegistrationDB::removeAllRows failed - no DB");
    }
}

void
RegistrationDB::getAllRows ( ResultSet& rResultSet ) const
{
    // Clear out any previous records
    rResultSet.destroyAll();

    if ( m_pFastDB != NULL )
    {
        SMART_DB_ACCESS;
        dbCursor< RegistrationRow > cursor;
        if ( cursor.select() > 0 )
        {
            do {
                UtlHashMap record;
                UtlString* uriValue = new UtlString(cursor->uri);
                UtlString* callidValue = new UtlString(cursor->callid);
                UtlString* contactValue = new UtlString(cursor->contact);
                UtlInt* expiresValue = new UtlInt(cursor->expires);
                UtlInt* cseqValue = new UtlInt(cursor->cseq);
                UtlString* qvalueValue = new UtlString(cursor->qvalue);
                UtlString* primaryValue = new UtlString(cursor->primary);
                UtlLongLongInt* updateNumberValue = new UtlLongLongInt(cursor->update_number);
                UtlString* instrumentValue = new UtlString(cursor->instrument);

                // Memory Leak fixes, make shallow copies of static keys
                UtlString* uriKey = new UtlString(gUriKey);
                UtlString* callidKey = new UtlString(gCallidKey);
                UtlString* contactKey = new UtlString(gContactKey);
                UtlString* expiresKey = new UtlString(gExpiresKey);
                UtlString* cseqKey = new UtlString(gCseqKey);
                UtlString* qvalueKey = new UtlString(gQvalueKey);
                UtlString* primaryKey = new UtlString(gPrimaryKey);
                UtlString* updateNumberKey = new UtlString(gUpdateNumberKey);
                UtlString* instrumentKey = new UtlString(gInstrumentKey);

                record.insertKeyAndValue(uriKey, uriValue);
                record.insertKeyAndValue(callidKey, callidValue);
                record.insertKeyAndValue(contactKey, contactValue);
                record.insertKeyAndValue(expiresKey, expiresValue);
                record.insertKeyAndValue(cseqKey, cseqValue);
                record.insertKeyAndValue(qvalueKey, qvalueValue);
                record.insertKeyAndValue(primaryKey, primaryValue);
                record.insertKeyAndValue(updateNumberKey, updateNumberValue);
                record.insertKeyAndValue(instrumentKey, instrumentValue);

                rResultSet.addValue(record);
            } while (cursor.next());
        }
    }
    else
    {
       OsSysLog::add(FAC_DB, PRI_CRIT, "RegistrationDB::getAllRows failed - no DB");
    }
}

Int64
RegistrationDB::getMaxUpdateNumberForRegistrar(const UtlString& primaryRegistrar) const
{
   Int64 maxUpdateForPrimary = 0LL;

   if ( m_pFastDB != NULL )
   {
      SMART_DB_ACCESS;
      dbCursor<RegistrationRow> cursor;
      dbQuery query;
      query = "primary = ", primaryRegistrar, "order by update_number desc";

      int numRows = cursor.select(query);
      if (numRows > 0)
      {
         maxUpdateForPrimary = cursor->update_number;
      }
   }
   else
   {
      assert(false);    // when this method is called, the DB pointer should not be null
   }

   return maxUpdateForPrimary;
}

Int64
RegistrationDB::getNextUpdateNumberForRegistrar(const UtlString& primaryRegistrar,
                                                Int64            updateNumber) const
{
   Int64 nextUpdateNumber = 0LL;

   if ( m_pFastDB != NULL )
   {
      SMART_DB_ACCESS;
      dbCursor<RegistrationRow> cursor;
      dbQuery query;
      query = "primary = ", primaryRegistrar,
              " and update_number > ", updateNumber,
              " order by update_number asc";

      int numRows = cursor.select(query);
      if (numRows > 0)
      {
         nextUpdateNumber = cursor->update_number;
      }
   }
   else
   {
      assert(false);    // when this method is called, the DB pointer should not be null
   }

   return nextUpdateNumber;
}

int
RegistrationDB::getNextUpdateForRegistrar(const UtlString& primaryRegistrar,
                                          Int64            updateNumber,
                                          UtlSList&        bindings) const
{
   int numRows = 0;
   Int64 nextUpdateNumber = getNextUpdateNumberForRegistrar(primaryRegistrar, updateNumber);
   if (nextUpdateNumber > 0)
   {
      dbQuery query;
      query =
         "primary = ", primaryRegistrar,
         " and update_number = ", nextUpdateNumber;
         numRows = getUpdatesForRegistrar(query, bindings);
      if (numRows > 0)
      {
         OsSysLog::add(
            FAC_SIP, PRI_DEBUG
            ,"RegistrationDB::getNextUpdateForRegistrar"
            " found %d rows for %s with updateNumber > %0#16" FORMAT_INTLL "x"
            ,numRows
            ,primaryRegistrar.data()
            ,updateNumber);
      }
   }
   return numRows;
}

int
RegistrationDB::getNewUpdatesForRegistrar(const UtlString& primaryRegistrar,
                                          Int64            updateNumber,
                                          UtlSList&        bindings) const
{
   dbQuery query;
   query = "primary = ", primaryRegistrar, " and update_number > ", updateNumber;
   int numRows = getUpdatesForRegistrar(query, bindings);
   if (numRows > 0)
   {
      OsSysLog::add(
         FAC_SIP, PRI_DEBUG
         ,"RegistrationDB::getNewUpdatesForRegistrar"
         " found %d rows for %s with updateNumber > %0#16" FORMAT_INTLL "x"
         ,numRows
         ,primaryRegistrar.data()
         ,updateNumber);
   }
   return numRows;
}

int
RegistrationDB::getUpdatesForRegistrar(dbQuery&  query,
                                       UtlSList& bindings) const
{
   int numRows = 0;
   if ( m_pFastDB != NULL )
   {
      SMART_DB_ACCESS;
      dbCursor<RegistrationRow> cursor(dbCursorForUpdate);
      numRows = cursor.select(query);
      if (numRows > 0)
      {
         do {
            RegistrationBinding* reg = copyRowToRegistrationBinding(cursor);
            bindings.append(reg);
         }
         while (cursor.next());
      }
   }
   else
   {
      OsSysLog::add(FAC_DB, PRI_CRIT, "RegistrationDB::getNewUpdatesForRegistrar failed - no DB");
   }
   return numRows;
}

void
RegistrationDB::getUnexpiredContactsUser (
   const Url& uri,
   const int& timeNow,
   ResultSet& rResultSet) const
{
    // Clear the results
    rResultSet.destroyAll();

    UtlString identity;
    uri.getIdentity( identity );

    if ( !identity.isNull() && ( m_pFastDB != NULL) )
    {
        SMART_DB_ACCESS;
        dbCursor< RegistrationRow > cursor;
        dbQuery query;
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "RegistrationDB::getUnexpiredContactsUser "
                      "identity = '%s'",
                      identity.data());
        if (strncmp(identity.data(), GRUU_PREFIX,
                    sizeof (GRUU_PREFIX) - 1) == 0)
        {
           // This is a GRUU, search for it in the gruu column.
           UtlString search_string(identity);
           search_string.append(";" SIP_GRUU_URI_PARAM);
           query="gruu=",search_string," and expires>",timeNow;
           OsSysLog::add(FAC_DB, PRI_DEBUG,
                         "RegistrationDB::getUnexpiredContactsUser "
                         "recognized GRUU, searching for '%s'",
                         search_string.data());
        }
        else
        {
           // This is not a GRUU, search for it in the identity column.
           query="np_identity=",identity," and expires>",timeNow;
        }

        if ( cursor.select(query) > 0 )
        {
            // Copy all the unexpired contacts into the result hash
            do
            {
                UtlHashMap record;
                UtlString* uriValue = new UtlString(cursor->uri);
                UtlString* callidValue = new UtlString(cursor->callid);
                UtlString* contactValue = new UtlString(cursor->contact);
                UtlInt* expiresValue = new UtlInt(cursor->expires);
                UtlInt* cseqValue = new UtlInt(cursor->cseq);
                UtlString* qvalueValue = new UtlString(cursor->qvalue);
                UtlString* primaryValue = new UtlString(cursor->primary);
                UtlLongLongInt* updateNumberValue = new UtlLongLongInt(cursor->update_number);

                UtlString* instanceIdValue = new UtlString(cursor->instance_id);
                UtlString* gruuValue = new UtlString(cursor->gruu);
                UtlString* pathValue = new UtlString(cursor->path);
                UtlString* instrumentValue = new UtlString(cursor->instrument);
                OsSysLog::add(FAC_DB, PRI_DEBUG,
                              "RegistrationDB::getUnexpiredContactsUser Record found "
                              "uri = '%s', contact = '%s', instance_id = '%s', "
                              "gruu = '%s', path = '%s', instrument = '%s'",
                              uriValue->data(), contactValue->data(),
                              instanceIdValue->data(), gruuValue->data(),
                              pathValue->data(), instrumentValue->data());

                // Memory Leak fixes, make shallow copies of static keys
                UtlString* uriKey = new UtlString(gUriKey);
                UtlString* callidKey = new UtlString(gCallidKey);
                UtlString* contactKey = new UtlString(gContactKey);
                UtlString* expiresKey = new UtlString(gExpiresKey);
                UtlString* cseqKey = new UtlString(gCseqKey);
                UtlString* qvalueKey = new UtlString(gQvalueKey);
                UtlString* primaryKey = new UtlString(gPrimaryKey);
                UtlString* updateNumberKey = new UtlString(gUpdateNumberKey);

                UtlString* instanceIdKey = new UtlString(gInstanceIdKey);
                UtlString* gruuKey = new UtlString(gGruuKey);
                UtlString* pathKey = new UtlString(gPathKey);
                UtlString* instrumentKey = new UtlString(gInstrumentKey);

                record.insertKeyAndValue(uriKey, uriValue);
                record.insertKeyAndValue(callidKey, callidValue);
                record.insertKeyAndValue(contactKey, contactValue);
                record.insertKeyAndValue(expiresKey, expiresValue);
                record.insertKeyAndValue(cseqKey, cseqValue);
                record.insertKeyAndValue(qvalueKey, qvalueValue);
                record.insertKeyAndValue(primaryKey, primaryValue);
                record.insertKeyAndValue(updateNumberKey, updateNumberValue);
                record.insertKeyAndValue(instanceIdKey, instanceIdValue);
                record.insertKeyAndValue(gruuKey, gruuValue);
                record.insertKeyAndValue(pathKey, pathValue);
                record.insertKeyAndValue(instrumentKey, instrumentValue);

                rResultSet.addValue(record);

            } while ( cursor.next() );
        }
    }
    else
    {
       OsSysLog::add(FAC_DB, PRI_CRIT, "RegistrationDB::getUnexpiredContactsUser failed - no DB");
    }
}

void
RegistrationDB::getUnexpiredContactsInstrument (
   const char* instrument,
   const int& timeNow,
   ResultSet& rResultSet) const
{
    // Clear the results
    rResultSet.destroyAll();

    // An empty instrument specification has no contacts, by specification.
    if ( instrument && instrument[0] != '\0' && ( m_pFastDB != NULL) )
    {
        SMART_DB_ACCESS;
        dbCursor< RegistrationRow > cursor;
        dbQuery query;
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "RegistrationDB::getUnexpiredContactsInstrument "
                      "instrument = '%s'",
                      instrument);

        // Search for contacts using the instrument column.
        query="instrument=",instrument," and expires>",timeNow;

        if ( cursor.select(query) > 0 )
        {
            // Copy all the unexpired contacts into the result hash
            do
            {
                UtlHashMap record;
                UtlString* uriValue = new UtlString(cursor->uri);
                UtlString* callidValue = new UtlString(cursor->callid);
                UtlString* contactValue = new UtlString(cursor->contact);
                UtlInt* expiresValue = new UtlInt(cursor->expires);
                UtlInt* cseqValue = new UtlInt(cursor->cseq);
                UtlString* qvalueValue = new UtlString(cursor->qvalue);
                UtlString* primaryValue = new UtlString(cursor->primary);
                UtlLongLongInt* updateNumberValue = new UtlLongLongInt(cursor->update_number);

                UtlString* instanceIdValue = new UtlString(cursor->instance_id);
                UtlString* gruuValue = new UtlString(cursor->gruu);
                UtlString* pathValue = new UtlString(cursor->path);
                UtlString* instrumentValue = new UtlString(cursor->instrument);
                OsSysLog::add(FAC_DB, PRI_DEBUG,
                              "RegistrationDB::getUnexpiredContactsInstrument Record found "
                              "uri = '%s', contact = '%s', instance_id = '%s', "
                              "gruu = '%s', path = '%s', instrument = '%s'",
                              uriValue->data(), contactValue->data(),
                              instanceIdValue->data(), gruuValue->data(),
                              pathValue->data(), instrumentValue->data());

                // Memory Leak fixes, make shallow copies of static keys
                UtlString* uriKey = new UtlString(gUriKey);
                UtlString* callidKey = new UtlString(gCallidKey);
                UtlString* contactKey = new UtlString(gContactKey);
                UtlString* expiresKey = new UtlString(gExpiresKey);
                UtlString* cseqKey = new UtlString(gCseqKey);
                UtlString* qvalueKey = new UtlString(gQvalueKey);
                UtlString* primaryKey = new UtlString(gPrimaryKey);
                UtlString* updateNumberKey = new UtlString(gUpdateNumberKey);

                UtlString* instanceIdKey = new UtlString(gInstanceIdKey);
                UtlString* gruuKey = new UtlString(gGruuKey);
                UtlString* pathKey = new UtlString(gPathKey);
                UtlString* instrumentKey = new UtlString(gInstrumentKey);

                record.insertKeyAndValue(uriKey, uriValue);
                record.insertKeyAndValue(callidKey, callidValue);
                record.insertKeyAndValue(contactKey, contactValue);
                record.insertKeyAndValue(expiresKey, expiresValue);
                record.insertKeyAndValue(cseqKey, cseqValue);
                record.insertKeyAndValue(qvalueKey, qvalueValue);
                record.insertKeyAndValue(primaryKey, primaryValue);
                record.insertKeyAndValue(updateNumberKey, updateNumberValue);
                record.insertKeyAndValue(instanceIdKey, instanceIdValue);
                record.insertKeyAndValue(gruuKey, gruuValue);
                record.insertKeyAndValue(pathKey, pathValue);
                record.insertKeyAndValue(instrumentKey, instrumentValue);

                rResultSet.addValue(record);

            } while ( cursor.next() );
        }
    }
    else
    {
       OsSysLog::add(FAC_DB, PRI_CRIT, "RegistrationDB::getUnexpiredContactsInstrument failed - no DB");
    }
}

void
RegistrationDB::getUnexpiredContactsUserInstrument (
   const Url& uri,
   const char* instrument,
   const int& timeNow,
   ResultSet& rResultSet) const
{
    // Clear the results
    rResultSet.destroyAll();

    UtlString identity;
    uri.getIdentity( identity );

    if ( !identity.isNull() &&
         // An empty instrument specification has no contacts, by specification.
         instrument && instrument[0] != '\0' &&
         ( m_pFastDB != NULL) )
    {
        SMART_DB_ACCESS;
        dbCursor< RegistrationRow > cursor;
        dbQuery query;
        OsSysLog::add(FAC_SIP, PRI_DEBUG,
                      "RegistrationDB::getUnexpiredContactsUserInstrument "
                      "identity = '%s' instrument = '%s'",
                      identity.data(), instrument);

        // Search for contacts using the instrument column.
        query="np_identity=",identity," and instrument=",instrument," and expires>",timeNow;

        if ( cursor.select(query) > 0 )
        {
            // Copy all the unexpired contacts into the result hash
            do
            {
                UtlHashMap record;
                UtlString* uriValue = new UtlString(cursor->uri);
                UtlString* callidValue = new UtlString(cursor->callid);
                UtlString* contactValue = new UtlString(cursor->contact);
                UtlInt* expiresValue = new UtlInt(cursor->expires);
                UtlInt* cseqValue = new UtlInt(cursor->cseq);
                UtlString* qvalueValue = new UtlString(cursor->qvalue);
                UtlString* primaryValue = new UtlString(cursor->primary);
                UtlLongLongInt* updateNumberValue = new UtlLongLongInt(cursor->update_number);

                UtlString* instanceIdValue = new UtlString(cursor->instance_id);
                UtlString* gruuValue = new UtlString(cursor->gruu);
                UtlString* pathValue = new UtlString(cursor->path);
                UtlString* instrumentValue = new UtlString(cursor->instrument);
                OsSysLog::add(FAC_DB, PRI_DEBUG,
                              "RegistrationDB::getUnexpiredContactsUserInstrument Record found "
                              "uri = '%s', contact = '%s', instance_id = '%s', "
                              "gruu = '%s', path = '%s', instrument = '%s'",
                              uriValue->data(), contactValue->data(),
                              instanceIdValue->data(), gruuValue->data(),
                              pathValue->data(), instrumentValue->data());

                // Memory Leak fixes, make shallow copies of static keys
                UtlString* uriKey = new UtlString(gUriKey);
                UtlString* callidKey = new UtlString(gCallidKey);
                UtlString* contactKey = new UtlString(gContactKey);
                UtlString* expiresKey = new UtlString(gExpiresKey);
                UtlString* cseqKey = new UtlString(gCseqKey);
                UtlString* qvalueKey = new UtlString(gQvalueKey);
                UtlString* primaryKey = new UtlString(gPrimaryKey);
                UtlString* updateNumberKey = new UtlString(gUpdateNumberKey);

                UtlString* instanceIdKey = new UtlString(gInstanceIdKey);
                UtlString* gruuKey = new UtlString(gGruuKey);
                UtlString* pathKey = new UtlString(gPathKey);
                UtlString* instrumentKey = new UtlString(gInstrumentKey);

                record.insertKeyAndValue(uriKey, uriValue);
                record.insertKeyAndValue(callidKey, callidValue);
                record.insertKeyAndValue(contactKey, contactValue);
                record.insertKeyAndValue(expiresKey, expiresValue);
                record.insertKeyAndValue(cseqKey, cseqValue);
                record.insertKeyAndValue(qvalueKey, qvalueValue);
                record.insertKeyAndValue(primaryKey, primaryValue);
                record.insertKeyAndValue(updateNumberKey, updateNumberValue);
                record.insertKeyAndValue(instanceIdKey, instanceIdValue);
                record.insertKeyAndValue(gruuKey, gruuValue);
                record.insertKeyAndValue(pathKey, pathValue);
                record.insertKeyAndValue(instrumentKey, instrumentValue);

                rResultSet.addValue(record);

            } while ( cursor.next() );
        }
    }
    else
    {
       OsSysLog::add(FAC_DB, PRI_CRIT, "RegistrationDB::getUnexpiredContactsUserInstrument failed - no DB");
    }
}

void
RegistrationDB::getUnexpiredContactsFieldsContaining (
   UtlString& substringToMatch,
   const int& timeNow,
   UtlSList& matchingContactFields ) const
{
   // Clear the results
   matchingContactFields.destroyAll();
   if( m_pFastDB != NULL )
   {
       SMART_DB_ACCESS;
       dbCursor< RegistrationRow > cursor;
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
   }
   else
   {
      OsSysLog::add(FAC_DB, PRI_CRIT, "RegistrationDB::getUnexpiredContactsFieldsContaining failed - no DB");
   }
}

bool
RegistrationDB::isLoaded()
{
   return mTableLoaded;
}

RegistrationDB*
RegistrationDB::getInstance( const UtlString& name )
{
    // Critical Section here
    OsLock lock( sLockMutex );

    // See if this is the first time through for this process
    // Note that this being null => pgDatabase is also null
    if ( spInstance == NULL )
    {
        // Create the singleton class for clients to use
        spInstance = new RegistrationDB ( name );
    }
    return spInstance;
}

RegistrationBinding*
RegistrationDB::copyRowToRegistrationBinding(dbCursor<RegistrationRow>& cursor) const
{
   RegistrationBinding *reg = new RegistrationBinding();
   reg->setUri( cursor->uri );
   reg->setCallId( cursor->callid );
   reg->setContact( cursor->contact );
   reg->setQvalue( cursor->qvalue );
   reg->setInstanceId( cursor->instance_id );
   reg->setGruu( cursor->gruu );
   reg->setPath( cursor->path );
   reg->setCseq( cursor->cseq );
   reg->setExpires( cursor->expires );
   reg->setPrimary( cursor->primary );
   reg->setUpdateNumber( cursor->update_number );
   reg->setInstrument( cursor->instrument );
   return reg;
}

// Get all bindings expiring before newerThanTime.
void
RegistrationDB::getAllOldBindings(int newerThanTime,
                                  UtlSList& rAors) const
{
   // Empty the return value.
   rAors.destroyAll();

   if ( m_pFastDB != NULL )
   {
      SMART_DB_ACCESS;

      // Note: callid set to null indicates provisioned entries and
      //        these should not be removed
      dbQuery query;
      query = "expires <", static_cast<const int&>(newerThanTime), " and (callid != '#')";
      dbCursor< RegistrationRow > cursor;
      int rows = cursor.select( query );
      if (rows > 0)
      {
         // Add the data to rAors.
         rAors.append(new UtlString(cursor->uri));
         rAors.append(new UtlString(cursor->instrument));
      } while ( cursor.next() );
   }
   else
   {
      OsSysLog::add(FAC_DB, PRI_CRIT, "RegistrationDB::getAllOldBindings failed - no DB");
   }

   return;
}
