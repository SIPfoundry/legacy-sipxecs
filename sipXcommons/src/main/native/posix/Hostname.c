/**
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */

#include <sys/types.h>
#include <unistd.h>
#include <jni.h>

extern int gethostname(char *name, size_t len);

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
    return JNI_VERSION_1_2;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved)
{
}

/*
 * Class:     org_sipfoundry_commons_util_Hostname
 * Method:    getHostname
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_sipfoundry_commons_util_Hostname_getHostname(JNIEnv *env, jclass class)
{
    char name[255];
    size_t namelen = 255;
    int results;

    results = gethostname(name, namelen);
    if (results != 0)
    {
        name[0] = '\0';
    }

    return (*env)->NewStringUTF(env, name);
}
