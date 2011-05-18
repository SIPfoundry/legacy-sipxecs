//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifdef WIN32

// SYSTEM INCLUDES
#include <windows.h>
#include <winreg.h>
#include <stdio.h>

// APPLICATION INCLUDES
#include <os/wnt/getWindowsDNSServers.h>
#include <os/HostAdapterAddress.h>
#include <os/OsSocket.h>
#include <os/OsLogger.h>

// DEFINES
#define MAXNUM_DNS_ENTRIES 40

//used by getWindowsVersion
#define WINDOWS_VERSION_ERROR 0
#define WINDOWS_VERSION_98    1
#define WINDOWS_VERSION_NT4   2
#define WINDOWS_VERSION_2000  3

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
// GLOBALS
static DWORD (WINAPI *GetNetworkParams)(PFIXED_INFO, PULONG);
DWORD GetAdaptersInfo(
  PIP_ADAPTER_INFO pAdapterInfo,
  PULONG pOutBufLen
);

static DWORD (WINAPI *sipxGetAdaptersInfo)(
  PIP_ADAPTER_INFO pAdapterInfo,
  PULONG pOutBufLen
);

//retrieves the current windows version and returns
//one of the WINDOWS_VERSION definitions.
static int getWindowsVersion()
{
    OSVERSIONINFO osInfo;
   int retVal = WINDOWS_VERSION_ERROR;

    //try to figure out what version of windows we are running
    osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

   if (GetVersionEx(&osInfo))
    {

       //check if it's the right version of windows
       if (osInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
       {
           if (osInfo.dwMinorVersion > 0) //true if it is 98
            retVal = WINDOWS_VERSION_98;
       }
       else
       if (osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
       {
           if (osInfo.dwMajorVersion == 4)
           {
            retVal = WINDOWS_VERSION_NT4;
           }
           else
           if (osInfo.dwMajorVersion > 4)
           {
            retVal = WINDOWS_VERSION_2000;
           }
       }
   }

   return retVal;
}


//loads the iphlpapi.dll and sets any func pointers we may need
static HMODULE loadIPHelperAPI()
{
   HMODULE hRetModule = NULL;
    char caFullDLLPath[256];

    //first try loading it using the systems path
    hRetModule = LoadLibrary("iphlpapi.dll");
   if (hRetModule == NULL)
   {
        //if that fails, (it shouldn't), try using the GetSystemPath func
        GetSystemDirectory(caFullDLLPath,sizeof(caFullDLLPath));
        strcat(caFullDLLPath,"\\iphlpapi.dll");

        //try again
        hRetModule = LoadLibrary(caFullDLLPath);

        //ok, I give up...where the heck did they put the iphlpapi.dll???????
      if (!hRetModule)
      {
         Os::Logger::instance().log(FAC_KERNEL, PRI_ERR, "Cannot find iphlpapi.dll!\n");
      }
    }

   if (hRetModule)
   {
       //now find that function!
       *(FARPROC*)&GetNetworkParams = GetProcAddress(hRetModule,"GetNetworkParams");
       if (GetNetworkParams == NULL)
       {
         Os::Logger::instance().log(FAC_KERNEL, PRI_ERR, "Could not get the proc address to GetNetworkParams!\n");
         FreeLibrary(hRetModule);
         hRetModule = NULL;
       }

       *(FARPROC*)&sipxGetAdaptersInfo = GetProcAddress(hRetModule,"GetAdaptersInfo");
       if (sipxGetAdaptersInfo == NULL)
       {
         Os::Logger::instance().log(FAC_KERNEL, PRI_ERR, "Could not get the proc address to sipxGetAdaptersInfo!\n");
         FreeLibrary(hRetModule);
         hRetModule = NULL;
       }

   }

   return hRetModule;
}


static int getIPHelperDNSEntries(char DNSServers[][MAXIPLEN], int max)
{
   int ipHelperDNSServerCount = 0;
    PFIXED_INFO pNetworkInfo;
    PIP_ADDR_STRING pAddrStr;
    DWORD dwNetworkInfoSize;
    DWORD retErr;
   int windowsVersion;
   HMODULE hModule = NULL;

   windowsVersion = getWindowsVersion();


    if (windowsVersion == WINDOWS_VERSION_98   ||
       windowsVersion >= WINDOWS_VERSION_2000)
    {
      GetNetworkParams = NULL;

      hModule = loadIPHelperAPI();

      if (hModule && GetNetworkParams)
      {
          //force size to 0 so the GetNetworkParams gets the correct size
          dwNetworkInfoSize = 0;

         retErr = GetNetworkParams( NULL, &dwNetworkInfoSize );
            if( retErr == ERROR_BUFFER_OVERFLOW )
         {
             // Allocate memory from sizing information
             if( ( pNetworkInfo = (PFIXED_INFO)GlobalAlloc( GPTR, dwNetworkInfoSize ) ) != NULL )
            {
               // Get actual network params
                if( ( retErr = GetNetworkParams( pNetworkInfo, &dwNetworkInfoSize ) ) == 0 )
               {

                   //point to the server list
                   pAddrStr = &(pNetworkInfo->DnsServerList);

                   //walk the list of IP addresses
                   while( pAddrStr && ipHelperDNSServerCount < max )
                   {
                      //copy one of the ip addresses
                       strcpy(DNSServers[ipHelperDNSServerCount++],pAddrStr->IpAddress.String);
                       pAddrStr = pAddrStr->Next;
                   }

                  //free the memory
                   GlobalFree(pNetworkInfo);   // handle to global memory object
               }
               else
               {
                   Os::Logger::instance().log(FAC_KERNEL, PRI_ERR,  "DNS ERROR: GetNetworkParams failed with error %d\n", retErr );
                   GlobalFree(pNetworkInfo);   // handle to global memory object
                }
            }
            else
            {
                Os::Logger::instance().log(FAC_KERNEL, PRI_ERR,  "DNS ERROR: Memory allocation error\n" );
             }
         }
         else
                Os::Logger::instance().log(FAC_KERNEL, PRI_ERR,  "DNS ERROR: GetNetworkParams sizing failed with error %d\n", retErr );

         FreeLibrary(hModule);
         hModule = NULL;
      }

   }

   return ipHelperDNSServerCount;
}


static int getDNSEntriesFromRegistry(char regDNSServers[][MAXIPLEN], int max)
{
   int retRegDNSServerCount = 0;
    const char *strParametersKey    = "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters";
    const char *strDhcpNameServerValue       = "DhcpNameServer";
    const char *strNameServerValue           = "NameServer";
    char *token;  //used for parsing the ip addresses
    HKEY hKey;
    BYTE    data[255];
    DWORD    cbData;
    DWORD    dataType;
    char *ptr = NULL;   //pointer to ip addresses when parsing
   DWORD err;

   err = RegOpenKeyEx(
              HKEY_LOCAL_MACHINE,         // handle to open key
              strParametersKey,  // subkey name
              0,   // reserved
              KEY_READ, // security access mask
              &hKey    // handle to open key
              );

   if (err == ERROR_SUCCESS)
   {
       cbData = sizeof(data);

      err = RegQueryValueEx(
                  hKey,                      // handle to key
                  strDhcpNameServerValue,    // value name
                  0,                         // reserved
                  &dataType,                 // type buffer
                  data,                      // data buffer
                  &cbData);                  // size of data buffer

      if (err != ERROR_SUCCESS)
       {
           //try a different value
           err = RegQueryValueEx(
                     hKey,                // handle to key
                     strNameServerValue,  // value name
                     0,                   // reserved
                     &dataType,           // type buffer
                     data,                // data buffer
                     &cbData);            // size of data buffer

       }
   }

   if (err == ERROR_SUCCESS)
   {
       //we need to break it up on NT.  It puts all the IP's on one line.
       //it may not be a space, which I set as default, ...lets check for a ','
       if (strstr((char *)data,","))
           token = ",";
       else
         token = " ";

       //find the first token
       ptr = strtok((char *)data,token);
       while (ptr != NULL && retRegDNSServerCount < max)
       {
           strncpy(regDNSServers[retRegDNSServerCount++],ptr,MAXIPLEN);

           //search for the next one
           ptr = strtok(NULL,token);
       }
   }
   else
   {
      Os::Logger::instance().log(FAC_KERNEL, PRI_ERR, "Error reading values from registry in func: getDNSEntriesFromRegistry\n");
   }

   return retRegDNSServerCount;
}

extern "C" int getWindowsDNSServers(char DNSServers[][MAXIPLEN], int max)
{
    int     finalDNSServerCount = 0; //number of dns entries returned to user
    int     ipHelperDNSServerCount = 0; //number of dns entries found through ipHelperAPI
    int     regDNSServerCount = 0; //num entries found in registry
   char regDNSServers[MAXNUM_DNS_ENTRIES][MAXIPLEN]; //used to store registry DNS entries
   int i,j;  //general purpose looping variables
   int swapPos = 0; //location to move the DNS entries that match the registry

   //retrieve the DNS entries from a MS provided DLL
   //This func will also load the dll if on win98 or NT 2000
   ipHelperDNSServerCount = getIPHelperDNSEntries(DNSServers,max);
   finalDNSServerCount = ipHelperDNSServerCount;

   //We always search the registry now...
   //Before we only did it for NT, but because
   //we want the most recently used entries at the top, we will consult
   //the registry and use the list retrieved and sort it.
    regDNSServerCount = getDNSEntriesFromRegistry(regDNSServers,max);


   //now walk through the entries found through the registry
   //and make sure the registry entries are at the top
   //NOTE: We do this because windows seem to be hanging on to old entries.
   //      This is causing really bad timeouts when doing a DNS search.
   //      If you use the registry, we should be getting the newest entries
   //      and moving those to the top of the DNS list
   if (ipHelperDNSServerCount && regDNSServerCount)
   {
      for (i = 0; i < ipHelperDNSServerCount;i++)
      {
         for (j = 0; j < regDNSServerCount;j++)
         {
            if (strcmp(DNSServers[i],regDNSServers[j]) == 0 && i != j)
            {
               char tmpdns[MAXIPLEN];
               //save off the original
               strcpy(tmpdns,DNSServers[swapPos]);
               //copy the zero index entry to the that location
               strcpy(DNSServers[swapPos],DNSServers[i]);
               //copy the saved to the old location
               strcpy(DNSServers[i],tmpdns);
               swapPos++;
            }
         }
      }
   }

   //if we only found reg entries and no ipHelper entries , then we need to return those
   //to the user (The ones from the registry)
   if (regDNSServerCount && !ipHelperDNSServerCount)
   {

      finalDNSServerCount = regDNSServerCount;
      //copy to final list
      for (i = 0; i < finalDNSServerCount; i++)
         strcpy(DNSServers[i],regDNSServers[i]);
   }

    //return the number of DNS entries found
    return finalDNSServerCount;
}


bool getContactAdapterName(char* szAdapter, const char* szIp)
{
    bool rc = false;
    if (0 == strcmp(szIp, "127.0.0.1"))
    {
        rc = true;
        strcpy(szAdapter, "loopback");
        return rc;
    }
#ifdef _WIN32
    if (loadIPHelperAPI())
    {
        PIP_ADAPTER_INFO pIpAdapterInfo = (PIP_ADAPTER_INFO)malloc(sizeof(IP_ADAPTER_INFO) * MAX_IP_ADDRESSES);
        unsigned long outBufLen = sizeof(IP_ADAPTER_INFO) * MAX_IP_ADDRESSES;

        DWORD dwResult = sipxGetAdaptersInfo(pIpAdapterInfo, &outBufLen);

        if (ERROR_SUCCESS == dwResult)
        {
            char szAddr[16];

            memset((void*)szAddr, 0, sizeof(szAddr));
            rc = true;
            PIP_ADAPTER_INFO pNextInfoRecord = pIpAdapterInfo;
            unsigned int adapterId = 0;
            char szAdapterId[MAX_IP_ADDRESSES];
            bool bFound = false;
            while (pNextInfoRecord && !bFound)
            {
                sprintf(szAdapterId, "eth%u", adapterId);
                PIP_ADDRESS_STRING pNextAddress = &(pNextInfoRecord->IpAddressList.IpAddress);
                while (pNextAddress)
                {
                    strcpy(szAddr, pNextAddress->String);
                    if (strcmp(szAddr, szIp) == 0 || strcmp(szIp, "0.0.0.0") == 0)// if the target
                                                                             // matches this address
                                                                             // or if the target
                                                                             // is any
                    {
                        strcpy(szAdapter, szAdapterId);
                        bFound = true;
                        break;
                    }
                    if (pNextInfoRecord->IpAddressList.Next)
                    {
                        pNextAddress = &(pNextInfoRecord->IpAddressList.Next->IpAddress);
                    }
                    else
                    {
                        pNextAddress = NULL;
                    }
                }
                adapterId++;
                pNextInfoRecord = pNextInfoRecord->Next;
            }
        }
        free((void*)pIpAdapterInfo);
    }
#else
        rc = false;
#endif

    return rc;
}


bool getAllLocalHostIps(const HostAdapterAddress* localHostAddresses[], int &numAddresses)
{
    bool rc = false;

#ifdef _WIN32
    if (loadIPHelperAPI())
    {
        PIP_ADAPTER_INFO pIpAdapterInfo = (PIP_ADAPTER_INFO)malloc(sizeof(IP_ADAPTER_INFO) * MAX_IP_ADDRESSES);
        unsigned long outBufLen = sizeof(IP_ADAPTER_INFO) * MAX_IP_ADDRESSES;

        DWORD dwResult = sipxGetAdaptersInfo(pIpAdapterInfo, &outBufLen);

        if (ERROR_SUCCESS == dwResult)
        {
            char szAddr[16];

            memset((void*)szAddr, 0, sizeof(szAddr));
            rc = true;
            numAddresses = 0;

            PIP_ADAPTER_INFO pNextInfoRecord = pIpAdapterInfo;

            unsigned int adapterId = 0;
            char szAdapterId[MAX_IP_ADDRESSES];
            while (pNextInfoRecord)
            {
                sprintf(szAdapterId, "eth%u", adapterId);
                PIP_ADDRESS_STRING pNextAddress = &(pNextInfoRecord->IpAddressList.IpAddress);
                while (pNextAddress)
                {
                    strcpy(szAddr, pNextAddress->String);
                    // ignore the loopback address
                    if (strcmp(szAddr, "127.0.0.1") == 0 || strcmp(szAddr, "0.0.0.0") == 0)
                    {
                        if (pNextInfoRecord->IpAddressList.Next)
                        {
                            pNextAddress = &(pNextInfoRecord->IpAddressList.Next->IpAddress);
                        }
                        else
                        {
                            pNextAddress = NULL;
                        }
                        continue;
                    }

                    localHostAddresses[numAddresses] = new HostAdapterAddress(szAdapterId, szAddr);
                    numAddresses++;
                    if (pNextInfoRecord->IpAddressList.Next)
                    {
                        pNextAddress = &(pNextInfoRecord->IpAddressList.Next->IpAddress);
                    }
                    else
                    {
                        pNextAddress = NULL;
                    }
                }
                adapterId++;
                pNextInfoRecord = pNextInfoRecord->Next;
            }
        }
        free((void*)pIpAdapterInfo);
    }
 #else
    rc = false;
 #endif
    return rc;
}

#endif
