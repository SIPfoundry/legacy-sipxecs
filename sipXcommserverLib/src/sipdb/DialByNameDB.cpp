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
#include "utl/UtlTokenizer.h"
#include "fastdb/fastdb.h"
#include "sipdb/ResultSet.h"
#include "sipXecsService/SipXecsService.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/DialByNameRow.h"
#include "sipdb/DialByNameDB.h"
#include "sipdb/PermissionDB.h"
#include "sipdb/CredentialDB.h"

REGISTER( DialByNameRow );

// Static Initializers
DialByNameDB* DialByNameDB::spInstance = NULL;
OsMutex  DialByNameDB::sLockMutex (OsMutex::Q_FIFO);
UtlString DialByNameDB::gNp_identityKey("np_identity");
UtlString DialByNameDB::gNp_contactKey("np_contact");
UtlString DialByNameDB::gNp_digitsKey("np_digits");

// GLOBAL VARS
const char digitmap [] = {
    '2','2','2',        // a,b,c
    '3','3','3',        // d,e,f
    '4','4','4',        // g,h,i
    '5','5','5',        // j,k,l
    '6','6','6',        // m,n,o
    '7','7','7','7',    // p,q,r,s
    '8','8','8',        // t,u,v
    '9','9','9','9'     // w,x,y,z
};

/* ============================ CREATORS ================================== */

DialByNameDB::DialByNameDB( const UtlString& name ) :
    mDatabaseName( name )
{
    // Access the shared table databse
    SIPDBManager* pSIPDBManager = SIPDBManager::getInstance();
    m_pFastDB = pSIPDBManager->getDatabase(name);

    // If we are the first process to attach
    // then we need to load the DB
    int users = pSIPDBManager->getNumDatabaseProcesses(name);
    OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                  "DialByNameDB::_ users = %d, mTableLoaded = %d",
                  users, mTableLoaded);
    if ( users == 1 || ( users > 1 && mTableLoaded == false ) )
    {
        OsSysLog::add(FAC_DB, PRI_DEBUG, "DialByNameDB::_ about to load");
        mTableLoaded = false;
        // Load the file implicitly
        if (this->load() == OS_SUCCESS)
        {
           mTableLoaded = true;
           OsSysLog::add(FAC_DB, PRI_DEBUG, "DialByNameDB::_ table successfully loaded");
        }
    }
    OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                  "DialByNameDB::_ rows in table = %d",
                  getRowCount());
}

DialByNameDB::~DialByNameDB()
{
    OsSysLog::add(FAC_DB, PRI_DEBUG, "<><>## DialByNameDB:: DESTRUCTOR");
}

/* ============================ MANIPULATORS ============================== */

void
DialByNameDB::releaseInstance()
{
    OsSysLog::add(FAC_DB, PRI_DEBUG, "<><>## DialByNameDB:: releaseInstance() spInstance=%p", spInstance);

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
DialByNameDB::load() const
{
    // Critical Section here
    OsLock lock( sLockMutex );
    OsStatus result = OS_SUCCESS;

    if ( m_pFastDB != NULL )
    {
        // Clean out the existing DB rows before loading
        // a new set from persistent storage
        removeAllRows ();

        // Query all Identities with 'AutoAttendant' permission set
        PermissionDB * pPermissionDB = PermissionDB::getInstance();
        ResultSet permissionsResultSet;
        pPermissionDB->getIdentities ( "AutoAttendant", permissionsResultSet );

        CredentialDB * pCredentialDB = CredentialDB::getInstance();
        ResultSet credentialsResultSet;

        UtlString identity, permission;
        int numAutoAttendees = permissionsResultSet.getSize();
        for (int index = 0; index < numAutoAttendees; index++)
        {
            // get the next identity
            UtlString identityKey("identity");
            UtlHashMap record;
            permissionsResultSet.getIndex( index, record );
            UtlString identity = *((UtlString*)record.findValue(&identityKey));

            Url identityUrl (identity);
            pCredentialDB->
                getAllCredentials (
                    identityUrl,
                    credentialsResultSet );

            // we should only have one credential! we're
            // only interested in the uri column's display name
            if ( credentialsResultSet.getSize() == 1)
            {
                UtlString uriKey("uri");
                UtlHashMap record;
                credentialsResultSet.getIndex( 0, record );
                UtlString uri = *((UtlString*)record.findValue(&uriKey));

                // must have a display name present before inserting a row
                // @TODO convert to url and get display name
                UtlHashMap nvPairs;
                if (!uri.isNull())
                {
                    // Null Element value create a special
                    // char string we have key and value so insert
                    UtlString* contactValue =
                        new UtlString( uri );

                    // Memory Leak fixes, make shallow copies of static keys
                    UtlString* contactKey =
                        new UtlString( gNp_contactKey );

                    nvPairs.insertKeyAndValue (
                        contactKey, contactValue );
                }
                // Insert the item row into the IMDB
                insertRow ( nvPairs );
            }
        }

        // Reset the changed flags after a successful load
        SIPDBManager::getInstance()->
            setDatabaseChangedFlag("credential", FALSE);
        SIPDBManager::getInstance()->
            setDatabaseChangedFlag("permission", FALSE);
    } else
    {
        result = OS_FAILED;
    }
    return result;
}

OsStatus
DialByNameDB::store()
{
    // this is a no-op for DialByNameDB as its
    // backing store is actually the join of the credentials
    // and permission databases
    if ( m_pFastDB != NULL )
    {
        return OS_SUCCESS;
    } else
    {
        return OS_FAILED;
    }
}

UtlBoolean
DialByNameDB::insertRow ( const UtlHashMap& nvPairs ) const
{
    // Note we do not need the identity object here
    // as it is inferred from the uri
    return insertRow (
        Url( *(dynamic_cast <UtlString*> (nvPairs.findValue(&gNp_contactKey))) ));
}

UtlBoolean
DialByNameDB::insertRow ( const Url& contact ) const
{
    UtlBoolean result = FALSE;

    if ( m_pFastDB != NULL )
    {
        // Fetch the display name
        UtlString identity, displayName, contactString;
        contact.getIdentity( identity );
        contact.getDisplayName( displayName );
        contact.toString( contactString );

        // Make sure that the contact URL is valid and contains
        // a contactIdentity and a contactDisplayName
        if ( !identity.isNull() && !displayName.isNull() )
        {
            UtlSList dtmfStrings;
            getDigitStrings ( displayName, dtmfStrings );
            if ( !dtmfStrings.isEmpty() )
            {
                // Thread Local Storage
                m_pFastDB->attach();

                // Search for a matching row before deciding to update or insert
                dbCursor< DialByNameRow > cursor(dbCursorForUpdate);

                DialByNameRow row;

                dbQuery query;
                // Primary Key is identity
                query="np_identity=",identity;

                // Purge all existing entries associated with this identity
                if ( cursor.select( query ) > 0 )
                {
                    cursor.removeAllSelected();
                }

                // insert all dtmf combinations for this user
                unsigned int i;
                for (i=0; i<dtmfStrings.entries(); i++)
                {
                    UtlString* digits = (UtlString*)dtmfStrings.at(i);
                    row.np_contact = contactString;
                    row.np_identity = identity;
                    row.np_digits = digits->data();
                    insert (row);
                }
                // Commit rows to memory - multiprocess workaround
                m_pFastDB->detach(0);
            }
        }
    }
    return result;
}

UtlBoolean
DialByNameDB::getDigitStrings (
    const UtlString& displayName,
    UtlSList& rDTMFStrings ) const
{
    UtlString lowerString = displayName;
    lowerString.toLower();
    lowerString = lowerString.strip(UtlString::both, '"');
    UtlTokenizer next( lowerString );
    UtlString token;
    UtlSList names;

    // Parse the Display name into a list of names
    // The algorithm for breaking up the string is as follows
    // if the string has > 2 tokens (forget about , separators)
    // create multiple entries in the IMDB for the all combinations
    // so for example
    // John Peter Smith Jr would result in DTMF entries for
    // PeterSmithJrJohn, SmithJrJohnPeter and JrJohnPeterSmith
    // @JC Added - separator for MIT's Avery-Smith example

    while (next.next(token, "\t\n,- "))
    {
        names.insert ( new UtlString ( token ) );
    }

    size_t numNames = names.entries();

    if ( numNames > 0 )
    {
        UtlString reorderedString;
        unsigned int splitPosition = 1;
        do
        {
            unsigned int i;
            UtlString firstNames;
            for ( i=0; i<splitPosition; i++ )
            {
                firstNames += *(UtlString*)names.at(i);
            }

            UtlString lastNames;
            for ( i = splitPosition; i<numNames; i++)
            {
                lastNames += *(UtlString*)names.at(i);
            }

            // add the new string
            reorderedString = lastNames + firstNames;

            unsigned int len = reorderedString.length();

            // calculate thd DTMF digits for the display name
            // firstly strip all , 's and spaces
            UtlString digitString;
            for ( i = 0; i<len; i++ )
            {
                int offset = (int) ( reorderedString(i) - 'a' );
                // filter out white space and comma separators
                if ( (offset >= 0) && (offset < 26) )
                    digitString += digitmap[ offset ];
            }

            rDTMFStrings.insert ( new UtlString (digitString) );

            splitPosition++;
        } while ( splitPosition<numNames );
    }

    // call the desctuctors on all the temp strings
    names.destroyAll();
    return TRUE;
}

UtlBoolean
DialByNameDB::removeRow ( const Url& contact )
{
    UtlBoolean removed = FALSE;
    UtlString identity;
    contact.getIdentity(identity);

    if ( !identity.isNull() && (m_pFastDB != NULL) )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        dbCursor< DialByNameRow > cursor(dbCursorForUpdate);

        dbQuery query;
        query="np_identity=",identity;
        if ( cursor.select( query ) > 0 )
        {
            cursor.removeAllSelected();
            removed = TRUE;
        }
        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }
    return removed;
}

void
DialByNameDB::removeAllRows () const
{
    if (m_pFastDB != NULL )
    {
        // Thread Local Storage
        m_pFastDB->attach();

        dbCursor< DialByNameRow > cursor(dbCursorForUpdate);

        if (cursor.select() > 0)
        {
            cursor.removeAllSelected();
        }
        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }
}

int
DialByNameDB::getRowCount () const
{
   int count = 0;

   // Thread Local Storage
   m_pFastDB->attach();

   dbCursor< DialByNameRow > cursor;

   dbQuery query;
   query="";
   count = cursor.select( query );

   // Commit rows to memory - multiprocess workaround
   m_pFastDB->detach(0);

   return count;
}

void
DialByNameDB::getAllRows(ResultSet& rResultSet) const
{
    // Clear the results
    rResultSet.destroyAll();

    // Check the TableInfo table to see whether we need to reload
    // the Tables from the Credential/Permission tables
    if ( m_pFastDB != NULL )
    {
        SIPDBManager* pSIPDBManager = SIPDBManager::getInstance();
        if ( pSIPDBManager->getDatabaseChangedFlag( "credential" ) ||
             pSIPDBManager->getDatabaseChangedFlag( "permission" )  )
        {
            // Reload this IMDB and reset the changed flags
            // in both the credential and permission tables
            this->load();
        }

        // Thread Local Storage
        m_pFastDB->attach();

        dbCursor< DialByNameRow > cursor;
        if ( cursor.select() > 0 )
        {
            do {
                UtlHashMap record;
                UtlString* np_identityValue =
                    new UtlString ( cursor->np_identity );
                UtlString* np_contactValue =
                    new UtlString ( cursor->np_contact );
                UtlString* np_digitsValue =
                    new UtlString ( cursor->np_digits );

                // Memory Leak fixes, make shallow copies of static keys
                UtlString* np_identityKey = new UtlString( gNp_identityKey );
                UtlString* np_contactKey = new UtlString( gNp_contactKey );
                UtlString* np_digitsKey = new UtlString( gNp_digitsKey );

                record.insertKeyAndValue (
                    np_identityKey, np_identityValue );
                record.insertKeyAndValue (
                    np_contactKey, np_contactValue );
                record.insertKeyAndValue (
                    np_digitsKey, np_digitsValue );

                rResultSet.addValue(record);
            } while (cursor.next());
        }
        // Commit rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }
}

void
DialByNameDB::getContacts (
    const UtlString& digitString,
    ResultSet& rResultSet ) const
{
    // This should erase the contents of the existing resultset
    rResultSet.destroyAll();

    if ( !digitString.isNull() && (m_pFastDB != NULL) )
    {
        // Check the TableInfo table to see whether we need to reload
        // the Tables from the Credential/Permission tables
        SIPDBManager* pSIPDBManager = SIPDBManager::getInstance();
        if ( pSIPDBManager->getDatabaseChangedFlag( "credential" ) ||
             pSIPDBManager->getDatabaseChangedFlag( "permission" )  )
        {
            // Reload this IMDB and rese the changed flags
            // in the credential and permission tables
            this->load();
        }

        // Thread Local Storage
        m_pFastDB->attach();

        // Search to see if we have a Credential Row
        dbCursor< DialByNameRow > cursor;

        dbQuery query;
        UtlString queryString = "np_digits like '" + digitString + "%'";
        query = queryString;
        if ( cursor.select(query) > 0 ) {
            do {
                UtlHashMap record;
                UtlString* np_identityValue =
                    new UtlString ( cursor->np_identity );
                UtlString* np_contactValue =
                    new UtlString ( cursor->np_contact );
                UtlString* np_digitsValue =
                    new UtlString ( cursor->np_digits );

                // Memory Leak fixes, make shallow copies of static keys
                UtlString* np_identityKey = new UtlString( gNp_identityKey );
                UtlString* np_contactKey = new UtlString( gNp_contactKey );
                UtlString* np_digitsKey = new UtlString( gNp_digitsKey );

                record.insertKeyAndValue (
                    np_identityKey, np_identityValue );
                record.insertKeyAndValue (
                    np_contactKey, np_contactValue );
                record.insertKeyAndValue (
                    np_digitsKey, np_digitsValue );

                rResultSet.addValue(record);
            } while ( cursor.next() );
        }
        // Commit the rows to memory - multiprocess workaround
        m_pFastDB->detach(0);
    }
}

bool
DialByNameDB::isLoaded()
{
   return mTableLoaded;
}

DialByNameDB*
DialByNameDB::getInstance( const UtlString& name )
{
    // Critical Section here
    OsLock lock( sLockMutex );

    // See if this is the first time through for this process
    // Note that this being null => pgDatabase is also null
    if ( spInstance == NULL )
    {
        // Create the singleton class for clients to use
        spInstance = new DialByNameDB( name );
    }
    return spInstance;
}
