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
#include <unistd.h>


#define CLOCK_FILE "/etc/sysconfig/clock"
#define LOCALTIME "/etc/localtime"
#define ZONEINFO_DIR "/usr/share/zoneinfo/"

int main(int argc, char *argv[])
{
   FILE * clk_fp;
   char tmpStr[200];
   char * child_execve_argv[5];

   //
   // argv[1] has the new ZONE string.
   // So set ZONE="argv[1]" in clock file.
   //
   clk_fp = fopen(CLOCK_FILE, "w");
   fprintf(clk_fp, "ZONE=\"%s\"\nUTC=true\n", argv[1]);
   fclose(clk_fp);

   child_execve_argv[0] = (char *)LN_BINARY;
   child_execve_argv[1] = (char *)"-sf";
   sprintf(tmpStr, "%s%s", ZONEINFO_DIR, argv[1]);
   child_execve_argv[2] = tmpStr;
   child_execve_argv[3] = (char *)LOCALTIME;
   child_execve_argv[4] = NULL;

   execve(child_execve_argv[0], child_execve_argv, NULL);

   /* Not going to get here ...*/
   printf("ERROR\n");
   return -1;
}
