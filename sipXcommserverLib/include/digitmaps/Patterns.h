// 
// 
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _Patterns_h_
#define _Patterns_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "utl/UtlHashMap.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// Pattern matching routines for digits, IP addresses and subnets
class Patterns 
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:
	
/* ============================ CREATORS ================================== */

   Patterns();
     //:Default constructor

   virtual ~Patterns();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

/// Matches IPv4 addressess to CIDR specified subnets   
   virtual bool IPv4subnet(const UtlString ipv4, ///< An IPv4 dotted quad to be matched
                           const UtlString cidr  /**< A subnet specified in CIDR notation:
                                                  *  x.y.z.q/size
                                                  *  x.y.z/size
                                                  *  x.y/size
                                                  *  x/size
                                                  *      size is 1..32 */
                          ) const;
   /**<
    * @returns true if the ipv4 is in the subnet, false if not. 
    */

/// Matches FQDN to DNS wildcards
   virtual bool DnsWildcard(const UtlString fqdn, //< A fully qualified Domain Name (ala sipx.pingtel.com)
                            const UtlString wildcard /**< A "wildcard" domain name, where the *. prefix will match any sub-domain of that domain.
                                                      * ('*.pingtel.com')
                                                      * A lone '*' matches any */
                           );
   /**<
    * @returns true if the fqdn is in the wildcard domain, false if not. 
    */
                                                 
                                                

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   // A place to store pre-compiled patterns
   UtlHashMap mPatterns;


/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   
/*////////////////////////////////////////////////////////////////////////// */
};

/* ============================ INLINE METHODS ============================ */

#endif  // _Patterns_h_
