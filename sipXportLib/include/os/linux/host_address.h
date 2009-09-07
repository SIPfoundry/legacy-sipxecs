//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#ifdef __linux__

#ifdef __cplusplus
extern "C" {
#endif

unsigned int getExternalHostAddressLinux(void);
void getEthernetHWAddrLinux(char * address, int length);

#ifdef __cplusplus
}
#endif

#endif /* __linux__ */
