//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////
#ifndef getDNSServers_h_
#define getDNSServers_h_

#ifdef __cplusplus

extern "C"
bool getAllLocalHostIps(const class HostAdapterAddress* localHostAddresses[],
                        int &numAddresses);
//: Return this host's ip addresses, as an array of UtlString references
//  Does not include the "loopback" address.

extern "C" bool getContactAdapterName(char* szAdapter, const char* szIp);
//: Returns a generated adapter name associated with the IP address

#endif

#endif // getDNSServers_h_
