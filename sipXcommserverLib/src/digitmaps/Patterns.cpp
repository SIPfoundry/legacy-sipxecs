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
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>


// APPLICATION INCLUDES
#include "digitmaps/Patterns.h"
#include "os/OsSysLog.h"
#include "utl/UtlRegex.h"
#include "utl/UtlString.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

/* //////////////////////////// PUBLIC //////////////////////////////////// */
// Constructor
Patterns::Patterns()
{
}

// Destructor
Patterns::~Patterns()
{
   mPatterns.destroyAll() ; 
}

/* ============================ MANIPULATORS ============================== */
/* ============================ CREATORS ================================== */
/* ============================ ACCESSORS ================================= */

   // a routeIPv4subnet matches if the subnet does
   // "cidr" is a subnet in CIDR notation (x.y.z.q/size)
   // "ipv4 is supposed to be an IPv4 dotted quad
bool Patterns::IPv4subnet(const UtlString ipv4, const UtlString cidr) const
{
   ssize_t slash ;
   int mask_size ;
   struct in_addr in_address ;
   struct in_addr in_network ;
   in_addr_t mask ;
   UtlString xmlNet ;

   if (inet_pton(AF_INET, ipv4, &in_address) <= 0)
   {
      return false ; // input wasn't a valid IPv4 address 
   }

   slash = cidr.index('/') ;
   if (slash == UTL_NOT_FOUND)
   {
      OsSysLog::add(FAC_SIP, PRI_WARNING, "Pattern:::IPv4subnet - pattern not in CIDR (dotted quad/Size) format.  (pattern=%s)", cidr.data());
      return false ; // pattern wasn't in CIDR format
   }

   xmlNet = cidr(0, slash) ;

   // Check of all dotted quads are in there.
   // Add 0's for missing elements.
   // Allows patterns like "192.168/16"
   ssize_t dot = xmlNet.index('.');
   if (dot == UTL_NOT_FOUND)
   {
      xmlNet.append(".0.0.0") ;
   }
   else
   {
      dot = xmlNet.index('.', dot+1) ;
      if (dot == UTL_NOT_FOUND)
      {
         xmlNet.append(".0.0") ;
      }
      else
      {
         dot = xmlNet.index('.', dot+1) ;
         if (dot == UTL_NOT_FOUND)
         {
            xmlNet.append(".0") ;
         }
      }
   }

   if (inet_pton(AF_INET, xmlNet.data(), &in_network) <= 0)
   {
      OsSysLog::add(FAC_SIP, PRI_WARNING, "Patterns::IPv4subnet - pattern not valid IPv4.  (pattern=%s, IPv4=%s)", cidr.data(), xmlNet.data());
      return false ; // element not valid IPv4
   }

   mask_size = atoi(cidr(slash+1, UtlString::UTLSTRING_TO_END)) ;
   if (mask_size <= 0 || mask_size > 32)
   {
      OsSysLog::add(FAC_SIP, PRI_WARNING, "Patterns::IPv4subnet - mask size is not 1->32.  (pattern=%s)", cidr.data());
      return false ; // bad size
   }

   // build the subnet bit mask in network order.
   // all the subnet bits are 1, all the addr bits are 0
   // eg. 192.168.0.0/16, mask is 0xffff0000, then put in
   // network order (cause in_addr_t's are in net order)
   mask = htonl((unsigned)~0 << 32-mask_size) ;

   // mask the address bits to 0s.  If the result
   // matches the subnet, this address is in that subnet
   if ((mask & in_address.s_addr) != in_network.s_addr)
   {
      return false ;
   }

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "Patterns::IPv4subnet - %s matches subnet %s/%d (mask=%08x in_network=%08x in_address=%08x", 
      ipv4.data(), 
      xmlNet.data(), mask_size,
      mask, in_network.s_addr, in_address.s_addr);

   return true;
}


             
   // a DnsWildcard matches if the domain name does
   // "wildcard" is a wildcard DNS (*.pingtel.com)
   // "fqdn is a FQDN
bool Patterns::DnsWildcard(const UtlString fqdn, const UtlString wildcard)
{
   if (wildcard == "*")
   {
      // Pattern of '*' matches everything
      return true;
   }

   if (wildcard.index("*.") != 0) 
   {
      // wildcard MUST start with *.
      OsSysLog::add(FAC_SIP, PRI_WARNING, "Patterns::DnsWildcard - wildcard must start with '*.'.  (wildcard=%s)", wildcard.data());
      return false;
   }

   // See if this pattern has been used before
   RegEx *re = (RegEx *)mPatterns.findValue(&wildcard) ;
 
   if (re == NULL)
   {                 
      UtlString exp ;

      // Convert wildcard notation into RegEx notation
      //    *.pingtel.com becomes:
      //  ^(.+\.)*\Qwoof.net\E(\.?)$
      // (zero or more subdomains) 'woof.net' (optional .)
      //
      // Matches (among others):
      //    pingtel.com pingtel.com. sipx.pingtel.com
      // Start with 0 or more non-whitespace then dot
      exp = "^(.+\\.)*" ; 
      exp.append("\\Q") ; // Start quotes
      exp.append(wildcard(2, UtlString::UTLSTRING_TO_END));
      exp.append("\\E") ; // End quotes
      exp.append("(\\.?)$") ; // Add optional trailing dot

      // Build a RegEx.  Set caseless matching
      re = new RegEx(exp, PCRE_CASELESS) ;

      // Save it for later use (saves time on reuse of
      // the same pattern later)
      mPatterns.insertKeyAndValue(new UtlString(wildcard), re) ;

      OsSysLog::add(FAC_SIP, PRI_DEBUG, "Patterns::DnsWildcard - wildcard=%s created regex=%s", wildcard.data(), exp.data());
   }

   // check if fqdn matches
   if (!re->Search(fqdn))
   {
      return false;
   }

   OsSysLog::add(FAC_SIP, PRI_DEBUG, "Patterns::DnsWildcard - %s matches domain %s",
                    fqdn.data(),
                    wildcard.data()) ;

   return true;

}
