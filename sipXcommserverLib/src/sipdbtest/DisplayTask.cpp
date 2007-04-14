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
#include <iostream>
#include "utl/UtlInt.h"
#include "utl/UtlLongLongInt.h"
#include "utl/UtlSortedList.h"
#include "os/OsMsgQ.h"
#include "os/OsProcessIterator.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/AliasDB.h"
#include "sipdb/HuntgroupDB.h"
#include "sipdb/ExtensionDB.h"
#include "sipdb/PermissionDB.h"
#include "sipdb/RegistrationDB.h"
#include "sipdb/SubscriptionDB.h"
#include "sipdb/DialByNameDB.h"
#include "sipdb/CredentialDB.h"
#include "sipdb/RegistrationDB.h"
#include "sipdb/AuthexceptionDB.h"
#include "sipdb/SIPXAuthHelper.h"
#include "IMDBTaskMonitor.h"
#include "DisplayTask.h"

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
extern UtlString userKey;
extern UtlString permissionKey;
extern UtlString qvalueKey;
extern UtlString expiresKey;
extern UtlString primaryKey;
extern UtlString updateNumberKey;
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
extern UtlString np_identityKey;
extern UtlString np_contactKey;
extern UtlString np_digitsKey;
extern UtlString componentKey;
extern UtlString acceptKey;
extern UtlString versionKey;

using namespace std ;

DisplayTask::DisplayTask ( 
    const UtlString& rArgument, OsMsgQ& rMsgQ, OsEvent& rCommandEvent) : 
    IMDBWorkerTask( rArgument, rMsgQ, rCommandEvent )
{}

DisplayTask::~DisplayTask()
{}

int 
DisplayTask::run( void* runArg )
{
    osPrintf ("Starting Thread\n");
    OsSysLog::add(LOG_FACILITY, PRI_DEBUG, "Starting Thread\n");
    // Indicate that we're finished, the monitor thread
    // reads this flag and if it's still set
    setBusy (TRUE);

    showTableRows ( mArgument );

    UtlString databaseInfo;
    getDatabaseInfo ( databaseInfo );
    osPrintf ( "%s", databaseInfo.data() );
    OsSysLog::add(LOG_FACILITY, PRI_DEBUG, databaseInfo.data());

    setBusy (FALSE);

    // send a success message to the sleeping monitor
    notifyMonitor( USER_DISPLAY_SUCCESS_EVENT );

    cleanupIMDBResources();
    osPrintf ("Stopping Thread");
    OsSysLog::add(LOG_FACILITY, PRI_DEBUG, "Stopping Thread\n");
    return( TRUE );
}

// This function displays all rows from the table specified
// using the new cursor/resultset interface
int
DisplayTask::showTableRows (const UtlString& rTableName) const
{
   int exitCode  = EXIT_SUCCESS;
   ResultSet resultSet;
   if (rTableName.compareTo("processinfo" , UtlString::ignoreCase) == 0)
      {
         showProcessInfo(resultSet);
      } else if (rTableName.compareTo("credential" , UtlString::ignoreCase) == 0)
      {
         showCredentials(resultSet);
      } else if (rTableName.compareTo("huntgroup" , UtlString::ignoreCase) == 0) 
      {
         showHuntGroups(resultSet);
      } else if (rTableName.compareTo("authexception" , UtlString::ignoreCase) == 0) 
      {
         showAuthExceptions(resultSet);
      } else if (rTableName.compareTo("registration" , UtlString::ignoreCase) == 0)
      {
         showRegistrations(resultSet);
      } else if (rTableName.compareTo("alias" , UtlString::ignoreCase) == 0)
      {
         showAliases(resultSet);
      } else if (rTableName.compareTo("extension" , UtlString::ignoreCase) == 0)
      {
         showExtensions(resultSet);
      } else if (rTableName.compareTo("permission" , UtlString::ignoreCase) == 0)
      {
         showPermissions(resultSet);
      } else if (rTableName.compareTo("dialbyname" , UtlString::ignoreCase) == 0)
      {
         showDialByName(resultSet);
      } else if (rTableName.compareTo("subscription" , UtlString::ignoreCase) == 0) 
      {
         showSubscriptions(resultSet);
      } else
      {
         exitCode = EXIT_BADSYNTAX;
      }
   return exitCode;
}

void DisplayTask::showProcessInfo(ResultSet& resultSet) const
{
   UtlString processInfo;
   getProcessInfo(TRUE, processInfo);
   cout << processInfo.data() << endl;
}

void DisplayTask::showCredentials(ResultSet& resultSet) const
{
   CredentialDB::getInstance()->getAllRows (resultSet);
   for (int i=0; i< resultSet.getSize(); i++)
   {
      UtlHashMap record;
      resultSet.getIndex( i, record );
      UtlString uri = *((UtlString*)record.findValue(&uriKey));
      UtlString realm = *((UtlString*)record.findValue(&realmKey));
      UtlString userid = *((UtlString*)record.findValue(&useridKey));
      UtlString passtoken = *((UtlString*)record.findValue(&passtokenKey));
      UtlString pintoken = *((UtlString*)record.findValue(&pintokenKey));
      UtlString authtype = *((UtlString*)record.findValue(&authtypeKey));
      cout << "Credential Row" << endl;
      cout << "==============" << endl;
      cout << "uri:\t\t"     << uri.data()       << endl \
           << "realm:\t\t"   << realm.data()     << endl \
           << "userid:\t\t"  << userid.data()    << endl \
           << "passtoken:\t" << passtoken.data() << endl \
           << "pintoken:\t"  << pintoken.data()  << endl \
           << "authtype:\t"  << authtype.data()  << endl << endl;
   }
}

void DisplayTask::showHuntGroups(ResultSet& resultSet) const
{
   HuntgroupDB::getInstance()->getAllRows( resultSet );
   for (int i=0; i< resultSet.getSize(); i++)
   {
      UtlHashMap record;
      resultSet.getIndex( i, record );
      UtlString identity = *((UtlString*)record.findValue(&identityKey));
      cout << "Huntgroup Row" << endl \
           << "=============" << endl \
           << "identity:\t" << identity.data() << endl << endl;
   }
}

void DisplayTask::showAuthExceptions(ResultSet& resultSet) const
{
   AuthexceptionDB::getInstance()->getAllRows( resultSet );
   for (int i=0; i< resultSet.getSize(); i++)
   {
      UtlHashMap record;
      resultSet.getIndex( i, record );
      UtlString user = *((UtlString*)record.findValue(&userKey));
      cout << "Authexception Row" << endl \
           << "=================" << endl \
           << "user:\t" << user.data() << endl << endl;
   }
}

void DisplayTask::showRegistrations(ResultSet& resultSet) const
{
   // Find all unexpired contacts
   RegistrationDB::getInstance()->getAllRows ( resultSet );
   for (int i=0; i< resultSet.getSize(); i++)
   {
      UtlHashMap record;
      resultSet.getIndex( i, record );
      UtlString uri      = *((UtlString*)record.findValue(&uriKey));
      UtlString callid   = *((UtlString*)record.findValue(&callidKey));
      UtlString contact  = *((UtlString*)record.findValue(&contactKey));
      UtlString qvalue   = *((UtlString*)record.findValue(&qvalueKey));
      int cseq           = *((UtlInt*)record.findValue(&cseqKey));
      int expires        = *((UtlInt*)record.findValue(&expiresKey));
      UtlString primary  = *((UtlString*)record.findValue(&primaryKey));
      Int64 updateNumber = *((UtlLongLongInt*)record.findValue(&updateNumberKey));

      cout << "Registration Row" << endl \
           << "================" << endl \
           << "uri:\t\t"      << uri.data()     << endl \
           << "callid:\t\t"   << callid.data()  << endl \
           << "contact:\t"  << contact.data() << endl \
           << "qvalue:\t\t"   << qvalue.data()  << endl \
           << "cseq:\t\t"     << cseq           << endl \
           << "expires:\t"  << expires        << endl \
           << "primary:\t"  << primary        << endl \
           << "update #:\t"   << updateNumber   << endl << endl;
   }
}

void DisplayTask::showAliases(ResultSet& resultSet) const
{
   AliasDB::getInstance()->getAllRows( resultSet );
   for (int i=0; i< resultSet.getSize(); i++)
   {
      UtlHashMap record;
      resultSet.getIndex( i, record );
      UtlString aliasIdentity = *((UtlString*)record.findValue(&identityKey));
      UtlString contact = *((UtlString*)record.findValue(&contactKey));
      cout << "Alias Row" << endl \
           << "=========" << endl \
           << "aliasIdentity:\t" << aliasIdentity.data() << endl \
           << "contact:\t" << contact.data() << endl << endl;
   }
}

void DisplayTask::showExtensions(ResultSet& resultSet) const
{
   ExtensionDB::getInstance()->getAllRows( resultSet );

   for (int i=0; i< resultSet.getSize(); i++)
   {
      UtlHashMap record;
      resultSet.getIndex( i, record );
      UtlString uri = *((UtlString*)record.findValue(&uriKey));
      UtlString extension = *((UtlString*)record.findValue(&extensionKey));
      cout << "Extension Row" << endl \
           << "=============" << endl \
           << "uri:\t\t"      << uri.data()       << endl \
           << "extension:\t"  << extension.data() << endl << endl;
   }
}

void DisplayTask::showPermissions(ResultSet& resultSet) const
{
   // Find all unexpired contacts
   PermissionDB::getInstance()->getAllRows ( resultSet );

   UtlString identity, permission;
   for (int i=0; i< resultSet.getSize(); i++)
   {
      UtlHashMap record;
      resultSet.getIndex( i, record );
      UtlString permission = *((UtlString*)record.findValue(&permissionKey));
      UtlString identity = *((UtlString*)record.findValue(&identityKey));

      cout << "Permission Row"   << endl \
           << "=============="   << endl \
           << "identity:\t\t"    << identity.data()   << endl \
           << "permission:\t\t"  << permission.data() << endl << endl;
   }
}

void DisplayTask::showDialByName(ResultSet& resultSet) const
{
   // Find all unexpired contacts
   DialByNameDB::getInstance()->getAllRows ( resultSet );

   for (int i=0; i<resultSet.getSize(); i++)
   {
      UtlHashMap record;
      resultSet.getIndex( i, record );
      UtlString np_identity = *((UtlString*)record.findValue(&np_identityKey));
      UtlString np_contact = *((UtlString*)record.findValue(&np_contactKey));
      UtlString np_digits = *((UtlString*)record.findValue(&np_digitsKey));

      cout << "DialByName Row" << endl \
           << "==============" << endl \
           << "np_identity:\t" << np_identity.data() << endl \
           << "np_contact:\t"  << np_contact.data()  << endl  \
           << "np_digits:\t"   << np_digits.data()   << endl << endl;
   }
}

void DisplayTask::showSubscriptions(ResultSet& resultSet) const
{
   SubscriptionDB::getInstance()->getAllRows ( resultSet );

   for (int i=0; i< resultSet.getSize(); i++)
   {
      UtlHashMap record;
      resultSet.getIndex( i, record );
      UtlString component  = *((UtlString*)record.findValue(&componentKey));
      UtlString uri        = *((UtlString*)record.findValue(&uriKey));
      UtlString callid     = *((UtlString*)record.findValue(&callidKey));
      UtlString contact    = *((UtlString*)record.findValue(&contactKey));
      int expires          = ((UtlInt*)record.findValue(&expiresKey))->getValue();
      int subscribecseq    = ((UtlInt*)record.findValue(&subscribecseqKey))->getValue();
      UtlString eventtype  = *((UtlString*)record.findValue(&eventtypeKey));
      UtlString id         = *((UtlString*)record.findValue(&idKey));
      UtlString to         = *((UtlString*)record.findValue(&toKey));
      UtlString from       = *((UtlString*)record.findValue(&fromKey));
      UtlString key        = *((UtlString*)record.findValue(&keyKey));
      UtlString recordroute= *((UtlString*)record.findValue(&recordrouteKey));
      int notifycseq       = ((UtlInt*)record.findValue(&notifycseqKey))->getValue();
      UtlString accept     = *((UtlString*)record.findValue(&acceptKey));
      int version          = ((UtlInt*)record.findValue(&versionKey))->getValue();

      cout << "Subscription Row" << endl \
           << "================" << endl \
           << "component:\t\t"<< component.data()  << endl \
           << "uri:\t\t"      << uri.data()        << endl \
           << "callid:\t\t"   << callid.data()     << endl \
           << "contact:\t"    << contact.data()    << endl \
           << "expires:\t"    << expires           << endl \
           << "subscribecseq:\t"<< subscribecseq   << endl \
           << "eventtype:\t"  << eventtype.data()  << endl \
           << "id:\t"         << id.data()         << endl \
           << "to:\t\t"       << to.data()         << endl \
           << "from:\t\t"     << from.data()       << endl \
           << "key:\t\t"      << key.data()        << endl \
           << "recordroute:\t"<< recordroute.data()<< endl \
           << "notifycseq:\t" << notifycseq        << endl \
           << "accept:\t"     << accept            << endl \
           << "version:\t"    << version           << endl << endl;
   }
}
