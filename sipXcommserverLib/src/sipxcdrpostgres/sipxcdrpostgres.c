//
// Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement. 
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
//////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <errno.h> 
#include <unistd.h>

int main(int argc, char *argv[])
{
   int i;

   char* execve_argv[argc+3];

   execve_argv[0] = (char *)SIPX_BINDIR "/sipxecs-pgcdraccess";
   printf("Directory path = %s\n", execve_argv[0]);

   for (i = 1; i < argc; i++ )
   {
      execve_argv[i] = argv[i];
   }
   execve_argv[argc] = NULL;

   int setidret;
   setidret = setuid(0);
   if (setidret == 0) 
   {
      execve(execve_argv[0], execve_argv, NULL);
   }
   else
   {
      printf("Error setting uid to root errvalue returned = %i", errno);
   }

   /* Not going to get here ...*/
   printf("ERROR\n");
   return -1;
}
