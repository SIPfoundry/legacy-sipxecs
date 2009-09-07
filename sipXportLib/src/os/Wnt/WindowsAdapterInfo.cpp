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


#include <stdlib.h>
#include "os/OsDefs.h"
#include <windows.h>
#include "os/wnt/WindowsAdapterInfo.h"
#include <os/wnt/getWindowsDNSServers.h>

        DWORD (WINAPI *GetAdaptersInfo)(PIP_ADAPTER_INFO, PULONG);
        DWORD (WINAPI *GetNetworkParams)(PFIXED_INFO, PULONG);

        AdapterInfoRec adapters[MAX_ADAPTERS];                          //used to store all the adapters it finds
        int AdapterCount = 0;

///////////////////////////////////////////
//
// isIPHLPAvail
// returns true if you can call functions in the windows iphlpapi
//
//
//
//returns:
//  true if win98, win2000, winnt (SP4 or above)
//
//
//////////////////////////////////////////

BOOL isIPHLPAvail()
{
        BOOL    retval = FALSE;

#ifdef WIN32

        static  BOOL iphlpapi_loaded = 0;  //TRUE if already loaded

        HMODULE hModule = NULL; //handle to the iphlpapi.dll
        BOOL    bIsWindows98 = FALSE;
        BOOL    bIsWindowsNT = FALSE;
        BOOL    bIsWindows2000 = FALSE;
        OSVERSIONINFO osInfo;
        char FullDLLPath[256];
        LPVOID lpMsgBuf; //for returning error codes

        if (iphlpapi_loaded)   //return true if we already loaded the dll.
                return TRUE;

        //try to figure out what version of windows we are running
        osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

        if (!GetVersionEx(&osInfo))
        {
                osPrintf("Could not retrieve OS version.\n");
                exit(1);
        }

        //check if it's the right version of windows
        if (osInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
        {
                if (osInfo.dwMinorVersion > 0) //true if it is 98
                        bIsWindows98 = TRUE;
        }
        else
        if (osInfo.dwPlatformId == VER_PLATFORM_WIN32_NT)
        {
                if (osInfo.dwMajorVersion == 4)
                {
                        bIsWindowsNT = TRUE;
                        osPrintf("Running windows NT 4.0\n");
                }
                else
                if (osInfo.dwMajorVersion > 4)
                {
                        bIsWindows2000 = TRUE;
                        osPrintf("Running windows 2000\n");
                }
                else
                {
                        osPrintf("Unknown version of windows.");
                        return retval;
                }
        }

        // Get sizing information about network params
        if (bIsWindows2000 || bIsWindows98)
        {
                //first try loading it using the systems path
                hModule = LoadLibrary("iphlpapi.dll");

                if (hModule == NULL)
                {
                        //if that fails, (it shouldn't), try using the GetSystemPath func
                        GetSystemDirectory(FullDLLPath,sizeof(FullDLLPath));
                        strcat(FullDLLPath,"\\iphlpapi.dll");

                        //try again
                        hModule = LoadLibrary(FullDLLPath);

                        //ok, I give up...where the heck did they put the iphlpapi.dll???????
                        if (hModule == NULL)
                        {
                                osPrintf("Could not load the DLL: iphlpaip.dll  (check that it's in your path)\n");
                                FormatMessage(
                                                                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                                                FORMAT_MESSAGE_FROM_SYSTEM |
                                                                FORMAT_MESSAGE_IGNORE_INSERTS,
                                                                NULL,
                                                                GetLastError(),
                                                                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                                                                (LPTSTR) &lpMsgBuf,
                                                                0,
                                                                NULL);
                                osPrintf("ERROR STRING: %s",(char *)lpMsgBuf);
                                return retval;
                        }

                }

                if (hModule)
                {
                        //now assign the function pointers

                        *(FARPROC*)&GetAdaptersInfo = GetProcAddress(hModule,"GetAdaptersInfo");
                        if (GetAdaptersInfo == NULL)
                        {
                                osPrintf("Could not get the proc address to GetAdaptersInfo!\n");
                                return FALSE;
                        }

                        //now find that function!
                        *(FARPROC*)&GetNetworkParams = GetProcAddress(hModule,"GetNetworkParams");
                        if (GetNetworkParams == NULL)
                        {
                                osPrintf("Could not get the proc address to GetNetworkParams!\n");
                                return FALSE;
                        }

                        osPrintf("Success loading DLL: iphlpaip.dll\n");
                        retval = TRUE;
                        iphlpapi_loaded = TRUE;  //set flag so we know not to load it again
                }

        }

#endif

        return retval;
}

///////////////////////////////////////////
//
// gets the domain name
//
// returns:  0 on failure  1 on success
//
//
//////////////////////////////////////////
int getWindowsDomainName(char *domain_name)
{
        DWORD Err;
        DWORD NetworkInfoSize;
        PFIXED_INFO pNetworkInfo;

        int retval = 0;

        *domain_name = '\0';

        if (isIPHLPAvail())  //inits iphlpapi and returns true if dll loaded
        {
                if (GetNetworkParams != NULL)
                {
                        //force size to 0 so the GetNetworkParams gets the correct size
                        NetworkInfoSize = 0;
                        if( ( Err = GetNetworkParams( NULL, &NetworkInfoSize ) ) != 0 )
                        {
                                if( Err != ERROR_BUFFER_OVERFLOW )
                                {
                                        printf( "GetNetworkParams sizing failed with error %d\n", Err );
                                        return 0;
                                }
                        }

                        // Allocate memory from sizing information
                        if( ( pNetworkInfo = (PFIXED_INFO)GlobalAlloc( GPTR, NetworkInfoSize ) ) == NULL )
                        {
                                printf( "Memory allocation error\n" );
                                return 0;
                        }

                        // Get actual network params
                        if( ( Err = GetNetworkParams( pNetworkInfo, &NetworkInfoSize ) ) != 0 )
                        {
                                printf( "GetNetworkParams failed with error %d\n", Err );
                                return 0;
                        }

                        strcpy(domain_name, pNetworkInfo->DomainName);

                        //free the memory
                        GlobalFree(pNetworkInfo);   // handle to global memory object

                }

        }

        return retval;
}

///////////////////////////////////////////
//
// lookupIpAddressByMacAddress
//
//
// mac_address: the mac address for which you want the ipaddress for
//
//returns:
//  ipaddress is the address for the given mac address
//
// -1 is returned if adapters could not be found
//  0 is returned on SUCCESS
//
//////////////////////////////////////////

int lookupIpAddressByMacAddress(char *mac_address, char *ipaddress)
{
        int retval = -1;
        for (int loop = 0; loop < AdapterCount; loop++)
        {
                if (memicmp(adapters[loop].MacAddress,mac_address,strlen(mac_address)) == 0)
                {
                        strcpy(ipaddress,adapters[loop].IpAddress);
                        retval = 0;
                }
        }

        return retval;
}

///////////////////////////////////////////
//
// getAdaptersInfo
//
//
// iInterface: the interface for which you want the ipaddress for
// pIpAddress: ip address returned.
//
//
// returns 0 on failure or number of adapters available
// 0 is returned if there are no adapters available.
//
//////////////////////////////////////////

int getAdaptersInfo()
{
        char MacAddressStr[256]; //mac address converted to a string
        char MacOneByteStr[10]; //used to hold one byte of mac address
        int retval = 0; //return -1 if no adapters found

        if (isIPHLPAvail())  //inits iphlpapi and returns true if dll loaded
        {
                //just return count if we already did this before
                if (AdapterCount)
                        return AdapterCount;

                IP_ADAPTER_INFO  *pAdapterInfo; //points to buffer hold linked list adapter info

                DWORD dwSize = (sizeof(IP_ADAPTER_INFO) * MAX_ADAPTERS) + sizeof(DWORD); //size for lots of adapters
                char *buffer = new char[dwSize];  //allocate space for lots of adapters
                if (buffer)
                {
                        pAdapterInfo = (IP_ADAPTER_INFO *)buffer;  //point to buffer
                        if (GetAdaptersInfo(
                                pAdapterInfo,  // buffer for mapping table
                                &dwSize) == NO_ERROR)                     // sort the table
                        {
                                while (pAdapterInfo)
                                {
                                        strcpy(adapters[AdapterCount].AdapterName, pAdapterInfo->Description);
                                        strcpy(adapters[AdapterCount].IpAddress, (const char *)pAdapterInfo->IpAddressList.IpAddress.String);

                                        //build mac address as a string
                                        *MacAddressStr = '\0';
                                        for (unsigned int loop = 0; loop < pAdapterInfo->AddressLength; loop++)
                                        {
                                                if (strlen(MacAddressStr))
                                                        strcat(MacAddressStr,"-");
                                                sprintf(MacOneByteStr,"%02X",pAdapterInfo->Address[loop]);
                                                strcat(MacAddressStr,MacOneByteStr);
                                        }
                                        strcpy((char *)adapters[AdapterCount].MacAddress, MacAddressStr);

                                        AdapterCount++;
                                        pAdapterInfo = pAdapterInfo->Next;
                                }

                                retval = AdapterCount;
                        }

                        delete [] buffer;
                }
        }

        return retval;
}

/*
void main(int argc, char **argv)
{

        int numInterfaces = getAdaptersInfo();

        printf("Number of interfaces: %d\n\n",numInterfaces);
        for (int loop = 0; loop < numInterfaces; loop ++)
        {
                printf("adapter name : %s \n",adapters[loop].AdapterName);
                printf("IP address   : %s \n",adapters[loop].IpAddress);
                printf("MAC address  : %s \n",adapters[loop].MacAddress);
        }

        printf("\n\n");

        char ipAddress[256];
        for (int loop2 = 0; loop2 < AdapterCount;loop2++)
        {
                printf("Looking up mac address %s\n",adapters[loop2].MacAddress);
                if (lookupIpAddressByMacAddress((char *)adapters[loop2].MacAddress,ipAddress) == 0)
                        printf("Found IP Address: %s \n",ipAddress);
                else
                        printf("Could not find mac address!\n");
        }
}
*/


#endif //WIN32
