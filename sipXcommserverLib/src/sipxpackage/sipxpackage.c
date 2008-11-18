//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[])
{
   char* execve_argv[3];

   execve_argv[0] = SIPX_BINDIR "/sipx-package.py";
   execve_argv[1] = argv[1];
   execve_argv[2] = NULL;

   // Run sipx-package.py as root
   setuid(0);
   execve(execve_argv[0], execve_argv, NULL);

   // Not going to get here ... hopefully
   printf("Error executing %s: %s\n", execve_argv[0], strerror(errno));

   return -1;
}
