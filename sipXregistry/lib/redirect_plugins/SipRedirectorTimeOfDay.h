//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef SIPREDIRECTORTIMEOFDAY_H
#define SIPREDIRECTORTIMEOFDAY_H

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "registry/RedirectPlugin.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Plugin::Factory method to get the instance of this plugin.
extern "C" RedirectPlugin* getRedirectPlugin(const UtlString& name);

/**
 * SipRedirectorTimeOfDay is a singleton class performing filtering of contacts
 * if contacts contain time of day parameter.
 *
 * TimeOfDay parameter is a proprietary field parameter appearing in a contact header.
 * It has the format:
 * sipx-ValidTime="start1:end1:start2:end2:start3:end3:...:startn:endn"
 * where start-i and end-i are string representations of 16-bit hexadecimal numbers
 * without leading prefix "0x"
 * Each hexadecimal number is a number of minutes from midnight 00:00 Sunday UTC.
 * start-i:end-i represents a single period of time (in munites) during the week,
 * where the contact is valid end-i inclusive.
 * SipRedirectorTimeOfDay redirector inspects each contact header and determines
 * whether the current day/time falls into one of the intervals.
 * If it does, the sipx-ValidTie parameter is removed, otherwise the contact
 * header is eliminated from the response.
 *
 * @note
 * SipRedirectorTimeOfDay is packaged into a separate shared object, which is loaded
 * by SipRedirectServer at runtime.
 * In order to test dynamic loading of SipRedirectorTimeOfDay as part of UnitTest
 * infrastructure all methods to be called by UnitTest suite are to be declared
 * virtual.
 *
 */
class SipRedirectorTimeOfDay : public RedirectPlugin
{
  public:

   virtual void readConfig(OsConfigDb& configDb);

   virtual OsStatus initialize(OsConfigDb& configDb,
                               int redirectorNo,
                               const UtlString& localDomainHost);

   virtual void finalize();

   virtual RedirectPlugin::LookUpStatus lookUp(
      const SipMessage& message,
      const UtlString& requestString,
      const Url& requestUri,
      const UtlString& method,
      ContactList& contactList,
      RequestSeqNo requestSeqNo,
      int redirectorNo,
      SipRedirectorPrivateStorage*& privateStorage,
      ErrorDescriptor& errorDescriptor);

   virtual const UtlString& name( void ) const;

  private:

   friend class SipRedirectorTimeOfDayTest;
   friend RedirectPlugin* getRedirectPlugin(const UtlString& name);

   /// constructor - callable only from getRedirectPlugin factory routine.
   explicit SipRedirectorTimeOfDay(const UtlString& instanceName);

   ~SipRedirectorTimeOfDay();

   /// Method to convert string to integer.
   bool from_string(int & value,       ///< takes the result of conversion
                    const UtlString& s ///< string encoding of hex value
                    );
   ///< @return true iff s was successfully decoded as a hexidecimal integer

   /// This method does the actual filtering.
   virtual RedirectPlugin::LookUpStatus processContactList(ContactList& contactList);

   /// Check whether current time/day falls into one of the intervals.
   virtual UtlBoolean isCurrentTimeValid(UtlString const & timeOfDayString);
   ///< @return true when the current time is within a valid interval for the URI

   // String to use in place of class name in log messages:
   // "[instance] class".
   UtlString mLogName;

   static const char * SIPX_TIMEOFDAY_PARAMETER;

   // @cond INCLUDENOCOPY
   /*
    * No copy constructor or assignment operator
    */
   SipRedirectorTimeOfDay(const SipRedirectorTimeOfDay& instance);
   SipRedirectorTimeOfDay & operator=(const SipRedirectorTimeOfDay& instance);
   // @endcond
};

#endif // SIPREDIRECTORTIMEOFDAY_H
