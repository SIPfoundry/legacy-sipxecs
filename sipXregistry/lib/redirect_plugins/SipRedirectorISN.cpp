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
#include <sys/types.h>
#include <sys/types.h>
#include <regex.h>

// APPLICATION INCLUDES
#include "os/OsSysLog.h"
#include "os/OsDefs.h"
#include "SipRedirectorISN.h"
#include "net/SipSrvLookup.h"
#include "net/Url.h"

#if defined(_WIN32)
#       include "resparse/wnt/sysdep.h"
#       include <resparse/wnt/netinet/in.h>
#       include <resparse/wnt/arpa/nameser.h>
#       include <resparse/wnt/resolv/resolv.h>
#       include <winsock.h>
extern "C" {
#       include "resparse/wnt/inet_aton.h"
}
#elif defined(_VXWORKS)
#       include <stdio.h>
#       include <netdb.h>
#       include <netinet/in.h>
#       include <vxWorks.h>
/* Use local lnameser.h for info missing from VxWorks version --GAT */
/* lnameser.h is a subset of resparse/wnt/arpa/nameser.h                */
#       include <resolv/nameser.h>
#       include <resparse/vxw/arpa/lnameser.h>
/* Use local lresolv.h for info missing from VxWorks version --GAT */
/* lresolv.h is a subset of resparse/wnt/resolv/resolv.h               */
#       include <resolv/resolv.h>
#       include <resparse/vxw/resolv/lresolv.h>
/* #include <sys/socket.h> used sockLib.h instead --GAT */
#       include <sockLib.h>
#       include <resolvLib.h>
#       include <resparse/vxw/hd_string.h>
#elif defined(__pingtel_on_posix__)
#       include <arpa/inet.h>
#       include <netinet/in.h>
#       include <sys/socket.h>
#       include <resolv.h>
#       include <netdb.h>
#else
#       error Unsupported target platform.
#endif

#ifndef __pingtel_on_posix__
extern struct __res_state _sip_res;
#endif

#include "resparse/rr.h"

// The space allocated for returns from res_query.
#define DNS_RESPONSE_SIZE 4096

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

// Static factory function.
extern "C" RedirectPlugin* getRedirectPlugin(const UtlString& instanceName)
{
   return new SipRedirectorISN(instanceName);
}

// Constructor
SipRedirectorISN::SipRedirectorISN(const UtlString& instanceName) :
   RedirectPlugin(instanceName)
{
   mLogName.append("[");
   mLogName.append(instanceName);
   mLogName.append("] SipRedirectorISN");
}

// Destructor
SipRedirectorISN::~SipRedirectorISN()
{
}

// Read config information.
void SipRedirectorISN::readConfig(OsConfigDb& configDb)
{
   if (configDb.get("BASE_DOMAIN", mBaseDomain) != OS_SUCCESS ||
       mBaseDomain.isNull())
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT,
                    "%s::readConfig "
                    "BASE_DOMAIN parameter missing or empty",
                    mLogName.data());
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_INFO,
                    "%s::readConfig "
                    "BASE_DOMAIN is '%s'", mLogName.data(), mBaseDomain.data());
   }

   if (configDb.get("PREFIX", mPrefix) != OS_SUCCESS ||
       mPrefix.isNull())
   {
      OsSysLog::add(FAC_SIP, PRI_INFO,
                    "%s::readConfig "
                    "dialing prefix is empty", mLogName.data());
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_INFO,
                    "%s::readConfig "
                    "dialing prefix is '%s'", mLogName.data(), mPrefix.data());
   }
}

// Initializer
OsStatus
SipRedirectorISN::initialize(OsConfigDb& configDb,
                             int redirectorNo,
                             const UtlString& localDomainHost)
{
   // Remove this redirector from consideration if no base domain was
   // specified.
   return mBaseDomain.isNull() ? OS_FAILED : OS_SUCCESS;
}

// Finalizer
void
SipRedirectorISN::finalize()
{
}

RedirectPlugin::LookUpStatus
SipRedirectorISN::lookUp(
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
   bool status = false;

   // Get the user part.
   UtlString userId;
   requestUri.getUserId(userId);

   // Test if the user part is in the right format -- prefix followed by digits*digits
   const char* user = userId.data();
   int prefix_length = mPrefix.length();
   // Compare the prefix.
   int i;                       // Length of the extension part.
   if (strncmp(user, mPrefix.data(), prefix_length) == 0)
   {
      // Effectively delete the prefix from the user part.
      user += prefix_length;
      // Check the syntax of the remainder of the user.
      i = strspn(user, "0123456789");
      int j = 0;
      if (i > 0)
      {
         if (user[i] == '*')
         {
            j = strspn(user + i + 1, "0123456789");
            if (user[i + 1 + j] == '\0')
            {
               status = true;
            }
         }
      }
   }

   if (status)
   {
      // Format of user part is correct.  Look for NAPTR records.

      // Create the domain to look up.
      char domain[2 * strlen(user) + mBaseDomain.length()];
      {
         char* p = &domain[0];
         // Copy the extension, reversing it and following each digit with a period.
         for (int k = i; --k >= 0; )
         {
            *p++ = user[k];
            *p++ = '.';
         }
         // Append the ITAD and a period.
         strcpy(p, user + i + 1);
         strcat(p, ".");
         // Append the ITAD root domain.
         strcat(p, mBaseDomain.data());
      }
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "%s::lookUp user '%s' has ISN format, domain is '%s'",
                    mLogName.data(), user, domain);

      // To hold the return of res_query_and_parse.
      res_response* dns_response;
      const char* canonical_name;

      // Make the query and parse the response.
      SipSrvLookup::res_query_and_parse(domain, T_NAPTR, NULL, canonical_name, dns_response);

      if (dns_response != NULL)
      {
         // Search the list of RRs for the 'best' answer.
         // Initialize to values at least 2**16.
         int lowest_order_seen = 1 << 16, lowest_preference_seen = 1 << 16;
         int best_response = -1; // No response found..
         // Search the answer list.
         for (unsigned int i = 0; i < dns_response->header.ancount; i++)
         {
            if (dns_response->answer[i]->rclass == C_IN &&
                dns_response->answer[i]->type == T_NAPTR &&
                // Note we look for the canonical name now.
                strcasecmp(canonical_name, dns_response->answer[i]->name) == 0)
            {
               // A NAPTR record has been found.
               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                             "%s::LookUp "
                             "NAPTR record found '%s' %d %d %d %d '%s' '%s' '%s' '%s'",
                             mLogName.data(),
                             dns_response->answer[i]->name,
                             dns_response->answer[i]->rclass,
                             dns_response->answer[i]->type,
                             dns_response->answer[i]->rdata.naptr.order,
                             dns_response->answer[i]->rdata.naptr.preference,
                             dns_response->answer[i]->rdata.naptr.flags,
                             dns_response->answer[i]->rdata.naptr.services,
                             dns_response->answer[i]->rdata.naptr.regexp,
                             dns_response->answer[i]->rdata.naptr.replacement);
               // Accept the record if flags are 'u' and services are 'E2U+sip'.
               // Note that the value 'E2U+sip' is defined by RFC 3764
               // (SIP enumservice) not RFC 2915 (the original NAPTR RFC).
               if (strcasecmp(dns_response->answer[i]->rdata.naptr.flags, "u") == 0 &&
                   strcasecmp(dns_response->answer[i]->rdata.naptr.services, "E2U+sip") == 0)
               {
                  // Check that it has the lowest order and preference values seen so far.
                  if (dns_response->answer[i]->rdata.naptr.order < lowest_order_seen ||
                      (dns_response->answer[i]->rdata.naptr.order == lowest_order_seen &&
                       dns_response->answer[i]->rdata.naptr.preference < lowest_preference_seen))
                  {
                     best_response = i;
                     lowest_order_seen = dns_response->answer[i]->rdata.naptr.order;
                     lowest_preference_seen = dns_response->answer[i]->rdata.naptr.preference;
                  }
               }
            }
         }

         // At this point, best_response (if any) is the response we chose.
         if (best_response != -1)
         {
            char* p = dns_response->answer[best_response]->rdata.naptr.regexp;
            OsSysLog::add(FAC_SIP, PRI_DEBUG,
                          "%s::LookUp Using NAPTR rewrite '%s' for '%s'",
                          mLogName.data(), p, domain);
            // Enough space for the 'match' part of the regexp field.
            char match[strlen(p) + 1];
            // Pointer to the 'replace' part of the regexp field.
            char delim;
            const char* replace;
            int i_flag;
            if (res_naptr_split_regexp(p, &delim, match, &replace, &i_flag))
            {
               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                             "%s::LookUp match = '%s', replace = '%s', i_flag = %d",
                             mLogName.data(), match, replace, i_flag);
               // Split operation was successful.  Try to match.
               regex_t reg;
               int ret = regcomp(&reg, match, REG_EXTENDED | (i_flag ? REG_ICASE : 0));
               if (ret == 0)
               {
                  // NAPTR matches can have only 9 substitutions.
                  regmatch_t pmatch[9];
                  // regexec returns 0 for success.
                  // Though RFC 3761 and the ISN Cookbook don't say, it appears
                  // that the regexp is matched against the user-part of the SIP URI.
                  if (regexec(&reg, user, 9, pmatch, 0) == 0)
                  {
                     // Match was successful.  Construct the replacement string.
                     // Current usage is that the replacement string is the resulting URI,
                     // not the replacement into the original application-string.
                     char* result = res_naptr_replace(replace, delim, pmatch, user, 0);
                     OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                   "%s::LookUp result = '%s'",
                                   mLogName.data(), result);
                     // Note that the replacement string is not
                     // substituted back into the original string, but used
                     // alone as the destination URI.
                     // Parse result string into URI.
                     Url contact(result, TRUE);
                     // Almost all strings are parsable as SIP URIs with a sufficient
                     // number of components missing.  But we can check that a scheme
                     // was identified, and that a host name was found.
                     // (A string with sufficiently few punctuation characters appears to
                     // be a sip: URI with the scheme missing and only a host name, but
                     // the legal character set for host names is fairly narrow.)
                     UtlString h;
                     contact.getHostAddress(h);
                     if (contact.getScheme() != Url::UnknownUrlScheme && !h.isNull())
                     {
                        contactList.add(contact, *this);
                     }
                     else
                     {
                        OsSysLog::add(FAC_SIP, PRI_ERR,
                                      "%s::LookUp Bad result string '%s' - "
                                      "could not identify URI scheme and/or host name is null - "
                                      "for ISN translation of '%s'",
                                      mLogName.data(), result, requestString.data());
                     }
                     // Free the result string.
                     free(result);
                  }
                  else
                  {
                     OsSysLog::add(FAC_SIP, PRI_WARNING,
                                   "%s::LookUp NAPTR regexp '%s' does not match "
                                   "for ISN translation of '%s' - no contact generated",
                                   mLogName.data(), match, requestString.data());
                  }
                  // Free the parsed regexp structure.
                  regfree(&reg);
               }
               else
               {
                  OsSysLog::add(FAC_SIP, PRI_WARNING,
                                "%s::LookUp NAPTR regexp '%s' is syntactially invalid "
                                   "for ISN translation of '%s'",
                                mLogName.data(), match, requestString.data());
               }
            }
            else
            {
               OsSysLog::add(FAC_SIP, PRI_ERR,
                             "%s::LookUp cannot parse NAPTR regexp field '%s' "
                             "for ISN translation of '%s'",
                             mLogName.data(), p, requestString.data());
            }
         }
         else
         {
            OsSysLog::add(FAC_SIP, PRI_WARNING,
                          "%s::LookUp No usable NAPTR found for '%s'"
                          "for ISN translation of '%s'",
                          mLogName.data(), domain, requestString.data());
         }
      }
      else
      {
         OsSysLog::add(FAC_SIP, PRI_WARNING,
                       "%s::LookUp no NAPTR record found for domain '%s' "
                       "for ISN translation of '%s'",
                       mLogName.data(), domain, requestString.data());
      }

      // Free the result of res_parse if necessary.
      if (dns_response != NULL)
      {
         res_free(dns_response);
      }
      if (canonical_name != NULL && canonical_name != domain)
      {
         free((void*) canonical_name);
      }
   }

   return RedirectPlugin::SUCCESS;
}

const UtlString& SipRedirectorISN::name( void ) const
{
   return mLogName;
}
