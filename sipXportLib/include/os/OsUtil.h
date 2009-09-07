//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _OsUtil_h_
#define _OsUtil_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include <utl/UtlString.h>

#include "os/OsStatus.h"
#include "os/OsTime.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Miscellaneous useful static methods
class OsUtil
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum OsPlatformType
   {
      PLATFORM_UNKNOWN=-1,
      PLATFORM_BRUTUS=0,
      PLATFORM_TCAS1=1,
      PLATFORM_TCAS2=2,
      PLATFORM_TCAS3=3,
      PLATFORM_TCAS4=4,
      PLATFORM_TCAS5=5,
      PLATFORM_TCAS6=6,
      PLATFORM_TCAS7=7,
      PLATFORM_TCAS8=8,
      PLATFORM_FREEBSD=95,
      PLATFORM_HPUX=96,
      PLATFORM_MACOSX=97,
      PLATFORM_SOLARIS=98,
      PLATFORM_LINUX=99,
      PLATFORM_WIN32=100
   };

   enum OsProductType
   {
      PRODUCT_UNKNOWN=-1,
      PRODUCT_XPRESSA=0,
      PRODUCT_INSTANT_XPRESSA=2
   };

   static const UtlString NULL_OS_STRING;

   //! Search a buffer for first occurance of another buffer. binary or ascii
   static char *memscan(const char *lookIn,
                        int lookInLen,
                        const char *lookFor,
                        int lookForLen);

/* ============================ Name Database ============================= */

   static OsStatus insertKeyValue(const UtlString& rPrefix,
                                  const UtlString& rName,
                                  void* value,
                                  UtlBoolean exceptOnErr=TRUE);
     //:Insert a key-value pair into the name database
     // The key is constructed by concatenating rPrefix and rName.
     // If exceptOnErr is TRUE, then upon encountering an error, this method
     // will throw an exception. This is sometimes useful for indicating an
     // error from within an object constructor.

   static OsStatus deleteKeyValue(const UtlString& rPrefix,
                                  const UtlString& rName,
                                  void** pValue=NULL);
     //:Remove the indicated key-value pair from the name database
     // The key is constructed by concatenating rPrefix and rName.
     // If pValue is non-NULL, the value for the key-value pair is returned
     // via pValue.
     // Return OS_SUCCESS if the lookup is successful, return
     // OS_NOT_FOUND if there is no match for the specified key.

   static OsStatus lookupKeyValue(const UtlString& rPrefix,
                                  const UtlString& rName,
                                  void** pValue=NULL);
     //:Retrieve the value associated with the specified key
     // The key is constructed by concatenating rPrefix and rName.
     // If pValue is non-NULL, the value is returned via pValue.
     // Return OS_SUCCESS if the lookup is successful, return
     // OS_NOT_FOUND if there is no match for the specified key.

   static OsStatus convertUtlStringToInt(const UtlString& rStr, intptr_t& rInt);
     //:Convert the value in rStr to an integer.
     // Uses strtol() with base==0 to perform the conversion.
     // Return OS_SUCCESS if the conversion was successful and set rInt to
     // the converted value in rInt.  If the conversion failed, return
     // OS_FAILED and set rInt to -1.


        static OsStatus checkIpAddress(const char* addr) ;
     // :Check the designated ip address for validity:
         //   1) 4 octets separated by '.'
         //      2) Each octet is between 0 and 255
         //   3) Address is not 0.0.0.0 or 255.255.255.255
         // Return OS_SUCCESS if the addr is valid, otherwise return OS_INVALID


        static OsStatus checkNetmask(const char* netmask) ;
         // :Check the designated netmask for validity:
         //   1) Between 255.0.0.0 and 255.255.255.254
         // Return OS_SUCCESS if the addr is valid, otherwise return OS_INVALID


        static UtlBoolean isSameNetwork(const char* destIpAddr,
                                                                   const char* myIpAddr,
                                                                   const char* myNetMask) ;
         // :Return TRUE if the destIpAddress is on the same logical network as
         //    myIpAddr given netmask myNetMask.  Returns FALSE otherwise.


        //returns OS_SUCCESS if the host repsonds within timeout
        OsStatus checkDnsAvailability(char *dnsServer, OsTime timeout);


        //returns OS_SUCCESS if the host repsonds within timeout
        OsStatus checkResponsiveDest(char *destHost, OsTime timeout);


/* ============================ MANIPULATORS ============================== */

/* ============================ ACCESSORS ================================= */

   static void getCurDate(UtlString& dateStr,
                          const struct tm* pCurTime, int maxLen);
     //:Return the current date
     // An example showing the date format is: "Wed Oct 7 1998".

   static void getCurTime(UtlString& timeStr,
                          const struct tm* pCurTime, int maxLen);
     //:Return the current time
     // An example showing the time format is: "8:03 PM".

   static int getPlatformType(void);
     //:Return the type of platform we are running on (e.g., PLATFORM_TCAS2)

   static int getProductType(void);
     //:Return the type of platform we are running on (e.g., PRODUCT_XPRESSA)

/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   OsUtil();
     //:Default constructor (not implemented for this class)
     // We identify this as a protected method so that gcc doesn't complain
     // that the class only defines a private constructor and has no friends.

   virtual
   ~OsUtil();
     //:Destructor (not implemented for this class)
     // We identify this as a protected method so that gcc doesn't complain
     // that the class only defines a private destructor and has no friends.

   static UtlBoolean convertIpAddressToOctets(const char* ipAddr, unsigned char octets[]) ;
         // :Convert a nnn.nnn.nnn.nnn ip address into an array of 4 unsigned chars.
         // Returns FALSE on error otherwise TRUE



/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   OsUtil(const OsUtil& rOsUtil);
     //:Copy constructor (not implemented for this class)

   OsUtil& operator=(const OsUtil& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _OsUtil_h_
