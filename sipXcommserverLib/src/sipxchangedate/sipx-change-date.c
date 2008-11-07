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
   argv[0] = (char *)DATE_BINARY;
   execve(argv[0], argv, NULL);

   /* Not going to get here ...*/
   printf("ERROR\n");
   return -1;
}
