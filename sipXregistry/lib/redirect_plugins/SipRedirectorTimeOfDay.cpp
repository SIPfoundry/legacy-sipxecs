//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <utl/UtlRegex.h>
#include <utl/UtlTokenizer.h>
#include "os/OsDateTime.h"
#include "os/OsSysLog.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/ResultSet.h"
#include "sipdb/AliasDB.h"
#include "SipRedirectorTimeOfDay.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const char * SipRedirectorTimeOfDay::SIPX_TIMEOFDAY_PARAMETER = "sipx-ValidTime";

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


// Static factory function.
extern "C" RedirectPlugin * getRedirectPlugin(const UtlString& instanceName)
{
   return new SipRedirectorTimeOfDay(instanceName);
}

// Constructor
SipRedirectorTimeOfDay::SipRedirectorTimeOfDay(const UtlString& instanceName) :
   RedirectPlugin(instanceName)
{
   mLogName.append("[");
   mLogName.append(instanceName);
   mLogName.append("] SipRedirectorTimeOfDay");
}

// Destructor
SipRedirectorTimeOfDay::~SipRedirectorTimeOfDay()
{
}

// Read config information.
void SipRedirectorTimeOfDay::readConfig(OsConfigDb& configDb)
{
}

// Initializer
OsStatus
SipRedirectorTimeOfDay::initialize(OsConfigDb& configDb,
                                 int redirectorNo,
                                 const UtlString& localDomainHost)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "%s::initialize", mLogName.data());
   return OS_SUCCESS;
}

// Finalizer
void
SipRedirectorTimeOfDay::finalize()
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "%s::finalize entered", mLogName.data());
}

// method to convert a hexidecimal string value to an integer
bool SipRedirectorTimeOfDay::from_string(int & value, const UtlString& s)
{
   char* end;
   value = strtol(s.data(), &end, 16);
   return (end - s.data() == (int)s.length());
}

RedirectPlugin::LookUpStatus
SipRedirectorTimeOfDay::lookUp(
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
   return processContactList(contactList);
}

RedirectPlugin::LookUpStatus SipRedirectorTimeOfDay::processContactList(ContactList& contactList)
{
   // Get the number of contacts in the contact list
   int numContactsInList = contactList.entries();

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "%s::processContactList %d contacts found",
                 mLogName.data(), numContactsInList);

   UtlString contact;

   // Iterate over the existing contact headers in a last-to-first order
   // It is important that the iteration is in this order
   // to allow some contact headers to be removed in the process
   // without impacting the operation of the loop
   for ( int contactIndex = numContactsInList - 1;
         contactIndex >= 0;
         contactIndex-- )
   {
      UtlString contact;
      if (!contactList.get(contactIndex, contact))
      {
         OsSysLog::add(FAC_SIP, PRI_WARNING,
                       "%s::processContactList getContactEntry failed for index %d",
                       mLogName.data(), contactIndex);
      }
      else
      {
         Url contactUri(contact);

         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "%s::processContactList contact %d '%s'",
                       mLogName.data(), contactIndex, contact.data());

         UtlString timeOfDayString;
         if (!contactUri.getFieldParameter(SIPX_TIMEOFDAY_PARAMETER, timeOfDayString))
         {
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "%s::processContactList %s param not found",
                          mLogName.data(), SIPX_TIMEOFDAY_PARAMETER);
         }
         else
         {
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "%s::processContactList %s param found: '%s'",
                          mLogName.data(), SIPX_TIMEOFDAY_PARAMETER, timeOfDayString.data());

            if (isCurrentTimeValid(timeOfDayString))
            {
               // Modify the contact in the list to remove the validity param
               contactUri.removeFieldParameter(SIPX_TIMEOFDAY_PARAMETER);
               // and put the modified contact back into the message
               UtlString modifiedContact;
               contactUri.toString(modifiedContact);
               contactList.set( contactIndex, modifiedContact, *this );
               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                             "%s::processContactList modified contact is: '%s'",
                             mLogName.data(), modifiedContact.data());
            }
            else
            {
               // Remove the contact header alltogether
               UtlBoolean removed = contactList.remove( contactIndex, *this );
               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                             "%s::processContactList attempt to remove contact %d rsp %d",
                             mLogName.data(), contactIndex, removed);
            }
         }
      }
   }
   return RedirectPlugin::SUCCESS;
}

/**
 * TimeString format: start1:end1:start2:date2:...startn:endn
 * start and end are minutes from Sunday 00:00 UTC expressed as hex numbers
 * "0000:01E4:0200:024F"
 */
UtlBoolean SipRedirectorTimeOfDay::isCurrentTimeValid(UtlString const & validityString)
{
   bool inContactInterval = false;

   unsigned long osCurTimeSinceEpoch = OsDateTime::getSecsSinceEpoch();
   //
   // (SecondsSinceEpoch/60)%10080 = minutes from 00:00 Sunday UTC
   // 10080=24*60*7 - minutes in one week; 4320=3*24*60 - minutes between Sunday and Thursday
   // Jan 1 1970 was a Thursday.  Adjust to obtain minutes from Sunday
   unsigned short minFromSunday = (osCurTimeSinceEpoch/60 - 4320)%10080;

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "SipRedirectorTimeOfDay::isCurrentTimeValid curMinFromSunday=0x%x", minFromSunday);

   UtlTokenizer tokenizer(validityString);

   UtlString startMinString, endMinString;
   bool validTokens = true;
   while (validTokens && ! inContactInterval && tokenizer.next(startMinString, ":"))
   {
      int startMinutes;
      if (!from_string(startMinutes, startMinString))
      {
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "SipRedirectorTimeOfDay::isCurrentTimeValid invalid interval start: '%s' value: '%s'",
                       validityString.data(), startMinString.data());
         validTokens = false;
      }
      else if (!tokenizer.next(endMinString, ":"))
      {
         OsSysLog::add(FAC_SIP, PRI_ERR,
                       "SipRedirectorTimeOfDay::isCurrentTimeValid no interval end: '%s'",
                       validityString.data());
         validTokens = false;
      }
      else
      {
         int endMinutes;
         if (!from_string(endMinutes, endMinString))
         {
            OsSysLog::add(FAC_SIP, PRI_ERR,
                          "SipRedirectorTimeOfDay::isCurrentTimeValid invalid interval end: '%s' value: '%s'",
                          validityString.data(), endMinString.data());
            validTokens = false;
         }
         else
         {
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "SipRedirectorTimeOfDay::isCurrentTimeValid interval: start: '%s' end: '%s'",
                          startMinString.data(), endMinString.data());

            if ( startMinutes <= minFromSunday && minFromSunday <= endMinutes )
            {
               inContactInterval = true;
            }
         }
      }
   }
   return inContactInterval;
}

const UtlString& SipRedirectorTimeOfDay::name( void ) const
{
   return mLogName;
}
