//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <time.h>

#ifdef __pingtel_on_posix__
#include <netdb.h>
#endif

#ifdef _VXWORKS
#include <../config/pingtel/pingtel.h>
#include <resolvLib.h>
#endif

// APPLICATION INCLUDES
#include "os/OsExcept.h"
#include "os/OsNameDb.h"
#include "os/OsUtil.h"
#include "os/OsSocket.h"
#include "os/OsSysLog.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// STATIC VARIABLE INITIALIZATIONS
const UtlString OsUtil::NULL_OS_STRING;

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

/* ============================ MANIPULATORS ============================== */

// Scans a binary buffer in memory for instances of another binary buffer in memory.
char *OsUtil::memscan(const char *lookIn,
              int lookInLen,
              const char *lookFor,
              int lookForLen)
{
    char *found = NULL;
    if (lookIn != NULL && lookFor != NULL && lookInLen > 0 && lookForLen > 0)
    {
        char *p = (char *)memchr(lookIn, lookFor[0], lookInLen);
        while (p != NULL)
        {
            int distanceFromBeginning = (int)(p - lookIn);
            int distanceToEnd = (int)lookInLen - distanceFromBeginning;
            if (distanceToEnd < lookForLen)
            {
                break;
            }
            else if (memcmp(p, lookFor, lookForLen) == 0)
            {
                found = (char *)p;
                break;
            }
            else
            {
                p = (char *)memchr((const char *)(p + 1), lookFor[0], distanceToEnd);
            }
        }
    }

    return found;
}

// Insert a key-value pair into the name database.
// The key is constructed by concatenating rPrefix and rName.
// If exceptOnErr is TRUE, then upon encountering an error, this method
// will throw an exception. This is sometimes useful for indicating an
// error from within an object constructor.
OsStatus OsUtil::insertKeyValue(const UtlString& rPrefix,
                                const UtlString& rName,
                                void* value,
                                UtlBoolean exceptOnErr)
{
   OsNameDb* pDict;
   OsStatus  res;

   assert(rName != "" || rPrefix != "");

   pDict = OsNameDb::getNameDb();
   UtlString key = rPrefix + rName;
   res = pDict->insert(key, value);

   if (exceptOnErr)
   {
      switch (res)
      {
         case OS_SUCCESS:
            break;          // success, do nothing
         case OS_NAME_IN_USE:
            OsSysLog::add(FAC_KERNEL, PRI_ERR,
                               "OsUtil::insertKeyValue - "
                               "name already in use: " + rPrefix + rName);
            break;
         default:
            OsSysLog::add(FAC_KERNEL, PRI_ERR,
                               "OsUtil::insertKeyValue - "
                               "OsStatus = %d", res);
            break;
      }
   }

   return res;
}

// Remove the indicated key-value pair from the name database.
// The key is constructed by concatenating rPrefix and rName.
// If pValue is non-NULL, the value for the key-value pair is returned
// via pValue.
// Return OS_SUCCESS if the lookup is successful, return
// OS_NOT_FOUND if there is no match for the specified key.
OsStatus OsUtil::deleteKeyValue(const UtlString& rPrefix,
                                const UtlString& rName,
                                void** pValue)
{
   OsNameDb* pDict;

   assert(rName != "" || rPrefix != "");

   pDict = OsNameDb::getNameDb();
   UtlString key = rPrefix + rName;
   OsStatus res = pDict->remove(key, pValue);
   return res;
}

// Retrieve the value associated with the specified key.
// The key is constructed by concatenating rPrefix and rName.
// If pValue is non-NULL, the value is returned via pValue.
// Return OS_SUCCESS if the lookup is successful, return
// OS_NOT_FOUND if there is no match for the specified key.
OsStatus OsUtil::lookupKeyValue(const UtlString& rPrefix,
                                const UtlString& rName,
                                void** pValue)
{
   OsNameDb* pDict;

   assert(rName != "" || rPrefix != "");

   pDict = OsNameDb::getNameDb();
   return pDict->lookup(rPrefix + rName, pValue);
}

// Convert the value in rStr to an integer.
// Uses strtol() with base==0 to perform the conversion.
// Return OS_SUCCESS if the conversion was successful and set rInt to
// the converted value in rInt.  If the conversion failed, return
// OS_FAILED and set rInt to -1.
OsStatus OsUtil::convertUtlStringToInt(const UtlString& rStr, intptr_t& rInt)
{
   const char *pBegin = rStr.data();
   char *pEnd;
   intptr_t   i;

   pBegin = rStr.data();
   i = (intptr_t) strtol(pBegin, &pEnd, 0);

   // verify that the string is non-empty and we were able to parse all of it
   if (*pBegin && *pEnd == 0)
   {
      rInt = i;
      return OS_SUCCESS;
   }
   else
   {
      rInt = -1;
      return OS_FAILED;
   }
}

// :Check the designated ip address for validity:
//   1) 4 octets separated by '.'
//       2) Each octet is between 0 and 255
//   3) Address is not 0.0.0.0 or 255.255.255.255
// Return OS_SUCCESS if the addr is valid, otherwise return OS_INVALID
OsStatus OsUtil::checkIpAddress(const char* addr)
{
        OsStatus status = OS_INVALID ;

        if (    OsSocket::isIp4Address(addr) &&
                        (strcmp(addr, "0.0.0.0") != 0) &&
                        (strcmp(addr, "255.255.255.255") != 0)) {
                status = OS_SUCCESS ;
        }

        return status ;
}


// :Check the designated netmask for validity:
//   1) Between 255.0.0.0 and 255.255.255.254
// Return OS_SUCCESS if the addr is valid, otherwise return OS_INVALID
OsStatus OsUtil::checkNetmask(const char* netmask)
{
        OsStatus status = OS_INVALID ;
        unsigned char octets[4] ;

        // Make sure this is in the format nnn.nnn.nnn.nnn

        if (convertIpAddressToOctets(netmask, octets)) {

                // Check bounds cases: 255.* and last bit must be off
                if ((octets[0] == 0xFF) && (!(octets[3] & 0x01))) {
                        UtlBoolean bSet = TRUE ;
                        UtlBoolean bError = FALSE ;
                        int i ;

                        // Make sure the mask is contiguous
                        for (i=1;i<4;i++) {
                                for (int j=7; j>=0; j--) {
                                        if (bSet) {
                                                // Note when we encounter the first non-contiguous set bit
                                                if (!(octets[i] & (1 << j))) {
                                                        bSet = FALSE ;
                                                }
                                        } else {
                                                // Error we if find set bit after an unset bit.
                                                if ((octets[i] & (1 << j))) {
                                                        bError = TRUE ;
                                                        break ;
                                                }
                                        }
                                }
                        }
                        if (!bError)
                                status = OS_SUCCESS ;
                }
        }
        return status ;
}


// :Return TRUE if the destIpAddress is on the same logical network as
//   myIpAddr given the net mask myNetMask.  Returns FALSE otherwise.
UtlBoolean OsUtil::isSameNetwork(const char* destIpAddr,
                                                        const char* myIpAddr,
                                                        const char* myNetMask)
{
        UtlBoolean     bSame = TRUE ;
        unsigned char octetsDest[4] ;
        unsigned char octetsIp[4] ;
        unsigned char octetsNetMask[4] ;
        int           i ;


        // Sanity Check the input
        if (    OsUtil::convertIpAddressToOctets(destIpAddr, octetsDest) &&
                        OsUtil::convertIpAddressToOctets(myIpAddr, octetsIp) &&
                        OsUtil::convertIpAddressToOctets(myNetMask, octetsNetMask)) {

                // Make sure bits match where the NetMask is set...
                for (i=0;i<4;i++) {
                        for (int j=7; j>=0; j--) {
                                if (octetsNetMask[i] & (1 << j)) {
                                        if ((octetsIp[i] & (1 << j)) != (octetsDest[i] & (1 << j))) {
                                                bSame = FALSE ;
                                        }
                                }
                        }
                }
        } else
                bSame = FALSE ;

        return bSame ;
}

//returns OS_SUCCESS if the host repsonds within timeout
OsStatus OsUtil::checkDnsAvailability(char *dnsServer, OsTime timeout)
{
        OsStatus retval = OS_SUCCESS;
        struct hostent* server;
        UtlString temp_output_address;


#       if defined(_VXWORKS)
        char hostentBuf[512];
#       endif


#       if defined(_WIN32) || defined(__pingtel_on_posix__)
        server = gethostbyname(dnsServer);

#       elif defined(_VXWORKS)
        server = resolvGetHostByName((char*) dnsServer,
                                hostentBuf, sizeof(hostentBuf));
#       else
#       error Unsupported target platform.
#       endif //_VXWORKS

        if(! server)
        {
                osPrintf("DNS failed to lookup host: %s\n",dnsServer);
                retval = OS_DNS_UNAVAILABLE;
        }


        temp_output_address.remove(0);
        return retval;
}


//returns OS_SUCCESS if the host repsonds within timeout
OsStatus OsUtil::checkResponsiveDest(char *destHost, OsTime timeout)
{
        OsStatus retval = OS_SUCCESS;


        return retval;
}


/* ============================ ACCESSORS ================================= */

//:Return the current date.
// An example showing the date format is: "Wed 10/14/98"
void OsUtil::getCurDate(UtlString& dateStr, const struct tm* pCurTime,
                        int maxLen)
{
   int  len;
   char str[20];

   if (maxLen >= 16)
   {
      // Wed Oct 14, 1998
      len = strftime(str, 19, "%a %b %d, %Y", pCurTime);

      // convert all but the first character of the day to lowercase
      str[1] = tolower(str[1]);
      str[2] = tolower(str[2]);

      // convert all but the first character of the month to lowercase
      str[5] = tolower(str[5]);
      str[6] = tolower(str[6]);

   }
   else if (maxLen >= 12)
   {
      // Wed 10/14/98
      len = strftime(str, 19, "%a %m/%d/", pCurTime);
      // :HACK: getting the two digit year this way avoids a silly compiler warning in gcc when you use %y
      char tmpYear[5];
      strftime(tmpYear, 5, "%Y", pCurTime);
      strcat(str,tmpYear+2);

      // convert all but the first character of the day to lowercase
      str[1] = tolower(str[1]);
      str[2] = tolower(str[2]);
   }
   else if (maxLen >= 8)
   {
      // 10/14/98
      len = strftime(str, 19, "%a %m/%d/", pCurTime);
      // :HACK: getting the two digit year this way avoids a silly compiler warning in gcc when you use %y
      char tmpYear[5];
      strftime(tmpYear, 5, "%Y", pCurTime);
      strcat(str,tmpYear+2);
   }
   else
   {
      str[0] = '\0';      // can't return a useful date
   }                      //  using less than eight characters

   dateStr = str;
}

// Return the current time.
// An example showing the time format is: "8:03P"
void OsUtil::getCurTime(UtlString& timeStr, const struct tm* pCurTime,
                        int maxLen)
{
   int       hour;
   UtlBoolean isAm;
   char      str[10];

   // convert from 24 hour to 12 hour representation
   hour = pCurTime->tm_hour;
   isAm = (hour < 12);
   if (hour == 0) hour = 12;
   if (hour > 12) hour -= 12;

   if (maxLen >= 9)
   {
      sprintf(str, "%d:%02d:%02d%s", hour,
                   pCurTime->tm_min,
                   pCurTime->tm_sec,
                   isAm ? "A" : "P");
   }
   else if (maxLen >= 6)
   {
      sprintf(str, "%d:%02d%s", hour,
              pCurTime->tm_min,
                   isAm ? "A" : "P");
   }
   else
   {
      str[0] = '\0';      // can't return a useful time string
   }                      //  using less than six characters

   timeStr = str;
}

// Return the type of platform we are running on (e.g., PLATFORM_TCAS2)
int OsUtil::getPlatformType(void)
{
   int platform;

   platform = PLATFORM_UNKNOWN;

#if defined(_WIN32) /* ] [ */
   platform = PLATFORM_WIN32;
#elif defined(__MACH__)
   platform = PLATFORM_MACOSX;
#elif defined(__linux__) /* ] [ */
   platform = PLATFORM_LINUX;
#elif defined(sun) /* ] [ */
   platform = PLATFORM_SOLARIS;
#elif defined(__hpux) /* ] [ */
   platform = PLATFORM_HPUX;
#elif defined(__FreeBSD__) /* ] [ */
   platform = PLATFORM_FREEBSD;
#else /* ] [ */
#error Unexpected Platform/CPU type
#endif /* ] ]*/

   return platform;
}

int OsUtil::getProductType(void)
{
   int productType = PRODUCT_UNKNOWN;

#if defined(_WIN32) || defined(__pingtel_on_posix__)
   productType = PRODUCT_INSTANT_XPRESSA;
#else
   assert(FALSE);
   productType = PRODUCT_UNKNOWN;
#endif

   return productType;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */


// Convert a nnn.nnn.nnn.nnn ip address string into an array of 4 unsigned
//   chars.
// Returns FALSE on error otherwise TRUE
UtlBoolean OsUtil::convertIpAddressToOctets(const char* ipAddr, unsigned char octets[])
{
        UtlBoolean bSuccess = FALSE ;

        // Make sure the data is well formed before attempting...
        if (OsSocket::isIp4Address(ipAddr)){
                int      index = 0 ;
                int      lastIndex = 0 ;
                int      i ;

                // The data looks ok, so blindly parse away...
                UtlString strTest(ipAddr) ;
                for(i=0; (i<4) && (lastIndex != -1); i++) {
                        if (i > 0) {
                                index = strTest.index(".", lastIndex) ;
                                if (index != -1)
                                        index++ ;
                        }
                        if (index != -1) {
                                octets[i] = (unsigned char) strtoul(strTest(index, strTest.length()).data(), NULL, 10) ;
                        }
                        lastIndex = index ;
                }
                bSuccess = TRUE ;
                strTest.remove(0);
        }

        return bSuccess ;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
