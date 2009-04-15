//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
/////////////////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

enum BOOL { FALSE, TRUE };
#define bool enum BOOL

bool fileExists(char*);

#define JVM "java"

int main(int argc, char *argv[])
{
   int i;
   char* jvmPath = NULL;
   char* execve_argv[argc+3];

   // We need to determine the location of the Java VM.
   // First, see if the SIP_JAVA_HOME environment points to something usefull.
   const char* sipxJavaHome = getenv("SIPX_JAVA_HOME");
   if (sipxJavaHome != NULL) {
     // Test a couple of directory tree alternatives, "/jre/bin" and "/bin".
     jvmPath = (char *) malloc(strlen(sipxJavaHome) + strlen("/jre/bin/") + strlen(JVM));
     strcpy(jvmPath, sipxJavaHome);
     strcat(jvmPath, "/jre/bin/");
     strcat(jvmPath, JVM);
     if (!fileExists(jvmPath)) {
       // That one did not work, try the other one.
       strcpy(jvmPath, sipxJavaHome);
       strcat(jvmPath, "/bin/");
       strcat(jvmPath, JVM);
       if (!fileExists(jvmPath)) {
         // That one did not work either.  Clean up and move on to the next method.
         free(jvmPath);
         jvmPath = NULL;
       }
     }
   }

   if (jvmPath == NULL) {
     // SIPX_JAVA_HOME was no help so see if it is somewhere in the "PATH".
     char* pathEnv = getenv("PATH");
     if (pathEnv != NULL) {
       // Walk the PATH environment, looking for a JVM.
       char* pathPrefix;
       pathPrefix = strtok(pathEnv, ":");
       while (pathPrefix != NULL) {
         jvmPath = (char *) malloc(strlen(pathPrefix) + strlen("/") + strlen(JVM));
         strcpy(jvmPath, pathPrefix);
         strcat(jvmPath, "/");
         strcat(jvmPath, JVM);
         if (fileExists(jvmPath)) {
           break;
         } else {
           // Not there, clean up and try again.
           free (jvmPath);
           jvmPath = NULL;
           pathPrefix = strtok(NULL, ":");
         }
       }
     }
   }

   if (jvmPath == NULL) {
     perror("ERROR - Unable to locate JVM");
     return -1;
   } else {
     execve_argv[0] = jvmPath;
     execve_argv[1] = (char *) "-jar";
     execve_argv[2] = (char *) SIPX_LIBDIR "/preflight.jar";

     for (i = 1; i < argc; i++ ) {
        execve_argv[i+2] = argv[i];
     }
     execve_argv[argc+2] = NULL;

     setuid(0);
     execve(jvmPath, execve_argv, NULL);

     // Only get here if execve failed.
     perror("ERROR - Failed to call execve");
     return -1;
   }
}


bool fileExists(char* filePath) {
   FILE* fp = fopen(filePath, "r");

   if (fp == NULL) {
     return FALSE;
   } else {
     fclose(fp);
     return TRUE;
   }
}
