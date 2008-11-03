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
#include "os/OsFS.h"
#include "os/OsLock.h"
#include "os/OsSysLog.h"
#include "net/Url.h"
#include "net/SipMessage.h"
#include "sipdb/HuntgroupDB.h"
#include "sipdb/ResultSet.h"
#include "SipRedirectorHunt.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* Hunt group q values are in the range 0.400 <= q < 0.600 */
#define HUNT_GROUP_MAX_Q 600
#define HUNT_RANGE_SIZE  200
#define HUNT_GROUP_MAX_CONTACTS 40

// Static factory function.
extern "C" RedirectPlugin* getRedirectPlugin(const UtlString& instanceName)
{
   return new SipRedirectorHunt(instanceName);
}

// Constructor
SipRedirectorHunt::SipRedirectorHunt(const UtlString& instanceName) :
   RedirectPlugin(instanceName),
   mHuntGroupsDefined(FALSE)
{
   mLogName.append("[");
   mLogName.append(instanceName);
   mLogName.append("] SipRedirectorHunt");
}

// Destructor
SipRedirectorHunt::~SipRedirectorHunt()
{
}

// Initializer
OsStatus
SipRedirectorHunt::initialize(OsConfigDb& configDb,
                              int redirectorNo,
                              const UtlString& localDomainHost)
{
   // Determine whether we have huntgroup supported
   // if the XML file contains 0 rows or the file
   // does not exist then disable huntgroup support
   ResultSet resultSet;
   HuntgroupDB::getInstance()->getAllRows(resultSet);
   mHuntGroupsDefined = resultSet.getSize() > 0;

   return mHuntGroupsDefined ? OS_SUCCESS : OS_FAILED;
}

// Finalizer
void
SipRedirectorHunt::finalize()
{
}

RedirectPlugin::LookUpStatus
SipRedirectorHunt::lookUp(
   const SipMessage& message,
   const UtlString& requestString,
   const Url& requestUri,
   const UtlString& method,
   ContactList& contactList,
   RequestSeqNo requestSeqNo,
   int redirectorNo,
   SipRedirectorPrivateStorage*& privateStorage,
   ErrorDescriptor& errorDescriptor)
{
   // Return immediately if the method is SUBSCRIBE, as the q values will
   // be stripped later anyway.
   if (method.compareTo(SIP_SUBSCRIBE_METHOD, UtlString::ignoreCase) == 0)
   {
      return RedirectPlugin::SUCCESS;
   }

   // Avoid doing any work if the request URI is not a hunt group.
   if (!HuntgroupDB::getInstance()->isHuntGroup(requestUri))
   {
      return RedirectPlugin::SUCCESS;
   }

   int numContacts = contactList.entries();

   numContacts = ( (numContacts <= HUNT_GROUP_MAX_CONTACTS)
                  ? numContacts
                  : HUNT_GROUP_MAX_CONTACTS);

   int* qDeltas = new int[numContacts]; // records random deltas already used.
   int deltasSet = 0; // count of entries filled in qDeltas 

   UtlString thisContact;
   for (int contactNum = 0;
        contactList.get( contactNum, thisContact );
        contactNum++)
   {
      Url contactUri( thisContact );
      UtlString qValue;

      if (!contactUri.getFieldParameter(SIP_Q_FIELD, qValue))
      {
         // this contact is not explicitly ordered, so generate a q value for it

         if (deltasSet < HUNT_GROUP_MAX_CONTACTS) // we only randomize this many
         {
            // pick a random delta, ensure it has not been used already
            bool duplicate=false;
            do 
            {
               // The rand man page says not to use low order bits this way, but
               // we're not doing security here, just sorting, so this is good enough,
               // and it's much faster than the floating point they suggest.
               int thisDelta = 1 + (rand() % HUNT_RANGE_SIZE); // 0 < thisDelta <= HUNT_RANGE_SIZE

               // check to see that thisDelta is unique in qDeltas
               int i;
               for (i = 0, duplicate=false; (!duplicate) && (i < deltasSet); i++)
               {
                  duplicate = (thisDelta == qDeltas[i]);
               }

               if (!duplicate)
               {
                  // it was unique, so use it
                  qDeltas[deltasSet] = thisDelta;
                  deltasSet++;

                  char temp[6];
                  sprintf(temp, "0.%03d", HUNT_GROUP_MAX_Q - thisDelta);
                  contactUri.setFieldParameter(SIP_Q_FIELD, temp);
                  contactList.set( contactNum, contactUri, *this );

                  OsSysLog::add( FAC_SIP, PRI_INFO,
                                 "%s::lookUp set q-value "
                                 "'%s' on '%s'\n",
                                 mLogName.data(), temp, thisContact.data());
               }
            } while(duplicate);
         }
         else
         {
            // We've randomized HUNT_GROUP_MAX_CONTACTS,
            // so from here on just set the q value to 0.0
            contactUri.setFieldParameter(SIP_Q_FIELD, "0.0");
            contactList.set( contactNum, contactUri, *this );
            OsSysLog::add( FAC_SIP, PRI_WARNING,
                           "%s::lookUp overflow - "
                           "set q=0.0 on '%s'\n",
                           mLogName.data(), thisContact.data());
         }
      }
      else
      {
         // thisContact had a q value set - do not modify it
      }
   } // for all contacts

   delete[] qDeltas;

   return RedirectPlugin::SUCCESS;
}

const UtlString& SipRedirectorHunt::name( void ) const
{
   return mLogName;
}
