/**
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */

#include <jni.h>
#include <windows.h>
#include <iphlpapi.h>

/*
 * Class:     org_sipfoundry_commons_discovery_ArpTable
 * Method:    windowsLookup
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_sipfoundry_commons_discovery_ArpTable_windowsLookup(JNIEnv *env, jobject obj, jstring ipAddress) {
    PMIB_IPNETTABLE pIPNetTable = NULL;
    ULONG ulSize  = 0;
    PMIB_IPNETROW pIPNetRow;
    IN_ADDR ia;
    const char *address;
    char macAddress[32];
    unsigned int x;

    address = (*env)->GetStringUTFChars(env, ipAddress, NULL);
    macAddress[0] = 0;

    GetIpNetTable(pIPNetTable, &ulSize, TRUE);
    pIPNetTable = (PMIB_IPNETTABLE) malloc(sizeof(MIB_IPNETTABLE) * ulSize);
    if (pIPNetTable != NULL) {
        GetIpNetTable(pIPNetTable, &ulSize, TRUE);

        // Iterate through each entry in the ARP table, looking for a matching IP address.
        for (x = 0; x < pIPNetTable->dwNumEntries; x++) {
            pIPNetRow = &(pIPNetTable->table[x]);
            ia.S_un.S_addr = pIPNetRow->dwAddr;
            if (strcmp(address, inet_ntoa(ia)) == 0) {
                sprintf(macAddress, "%02x:%02x:%02x:%02x:%02x:%02x",
                pIPNetRow->bPhysAddr[0],
                pIPNetRow->bPhysAddr[1],
                pIPNetRow->bPhysAddr[2],
                pIPNetRow->bPhysAddr[3],
                pIPNetRow->bPhysAddr[4],
                pIPNetRow->bPhysAddr[5]);
                break;
            }
        }

        free(pIPNetTable);
        (*env)->ReleaseStringUTFChars(env, ipAddress, address);
    }

    return (*env)->NewStringUTF(env, macAddress);
}
