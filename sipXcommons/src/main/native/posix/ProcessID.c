/**
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */

#include <sys/types.h>
#include <unistd.h>
#include <jni.h>

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
    return JNI_VERSION_1_2;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved)
{
}

/*
 * Class:     org_sipfoundry_commons_util_ProcessID
 * Method:    getPid
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_org_sipfoundry_commons_util_ProcessID_getPid(JNIEnv *env, jclass class)
{
    int pid = getpid();

    return pid;
}
