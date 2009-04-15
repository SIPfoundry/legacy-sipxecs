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

int main(int argc, char *argv[])
{
   int i;

   char* execve_argv[argc+3];

   execve_argv[0] = (char *)"/bin/sh";
   execve_argv[1] = (char *)"-p";
   execve_argv[2] = (char *)SIPX_BINDIR "/sipx-time-manager";

   for (i = 1; i < argc; i++ )
   {
      execve_argv[i+2] = argv[i];
   }
   execve_argv[argc+2] = NULL;

   setuid(0);
   execve("/bin/sh", execve_argv, NULL);

   /* Not going to get here ...*/
   printf("ERROR\n");
   return -1;
}
