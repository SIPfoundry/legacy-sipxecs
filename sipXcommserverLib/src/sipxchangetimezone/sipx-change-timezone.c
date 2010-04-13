//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


#define CLOCK_FILE "/etc/sysconfig/clock"
#define LOCALTIME "/etc/localtime"
#define LOCALTIMEOLD "/etc/localtime.old"
#define ZONEINFO_DIR "/usr/share/zoneinfo/"

int main(int argc, char *argv[])
{
   FILE * clk_fp;
   int pid, status;
   char tmpStr[200];
   char * child_execve_argv[5];

   switch ((pid=fork())) {
      case -1:
      {
         printf("ERROR\n");
         return -1;
      }
      case 0:
      {
         // Child process gets here.
         child_execve_argv[0] = (char *)MV_BINARY;
         child_execve_argv[1] = (char *)"-f";
         child_execve_argv[2] = (char *)LOCALTIME;
         child_execve_argv[3] = (char *)LOCALTIMEOLD;
         child_execve_argv[4] = NULL;

         execve(child_execve_argv[0], child_execve_argv, NULL);

         /* Not going to get here ...*/
         printf("ERROR\n");
         return -1;
      }
   }

   // We're the parent
   waitpid(pid, &status, 0); // wait for the child

   //
   // argv[1] has the new ZONE string.
   // So set ZONE="argv[1]" in clock file.
   //
   clk_fp = fopen(CLOCK_FILE, "w");
   fprintf(clk_fp, "ZONE=\"%s\"\nUTC=true\n", argv[1]);
   fclose(clk_fp);

   child_execve_argv[0] = (char *)CP_BINARY;
   sprintf(tmpStr, "%s%s", ZONEINFO_DIR, argv[1]);
   child_execve_argv[1] = tmpStr;
   child_execve_argv[2] = (char *)LOCALTIME;
   child_execve_argv[3] = NULL;

   execve(child_execve_argv[0], child_execve_argv, NULL);

   /* Not going to get here ...*/
   printf("ERROR\n");
   return -1;
}
