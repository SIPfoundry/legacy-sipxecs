//
// Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <os/OsFS.h>
#include <sys/poll.h>

// APPLICATION INCLUDES
#include "sipXecsService/StdinListener.h"

// DEFINES
// TYPEDEFS
// CONSTANTS

/// Class defining the interface for a service to receive messages from the supervisor

int StdinListenerTask::run(void *pArg)
{
   UtlString stdinMsg;
   int rc;
   while ( (rc=getInput(&stdinMsg)) > 0)
   {
      if (stdinMsg.length() > 0)
      {
         mOwner->gotInput(stdinMsg);
      }
   }
   OsSysLog::add( FAC_KERNEL, PRI_DEBUG, "StdinListenerTask: exiting with rc %d", rc);
   return 0;
}

int StdinListenerTask::getInput(UtlString* stdinMsg)
{
   int bytesRead = 0;
   int fds = 0;
   struct pollfd outputFds[1] = { {-1, 0, 0} };

   int stdoutFd = -1;           // The element of outputFds for stdout, or -1.
   if ( stdinMsg != NULL )
   {
      stdinMsg->remove(0);
      stdoutFd = fds;
      outputFds[fds].fd = fileno(stdin);
      outputFds[fds].events = POLLIN;
      outputFds[fds].revents = 0;
      fds++;
   }

   // We want to read everything available on the given fds before returning.
   // Call poll() until it returns for some reason other than EINTR.
   int rc;
   do
   {
      rc = poll(outputFds, fds, -1);
   } while (rc < 0 && errno == EINTR);
   if ( rc > 0 )
   {
      if (stdoutFd >= 0 && (outputFds[stdoutFd].revents & POLLIN) != 0)
      {
         if ( (rc=readAll( fileno(stdin), stdinMsg )) > 0)
         {
            bytesRead = rc;
         }
      }
   }

   return bytesRead;
}

/// Read all input which is available on the specified fd.
/// Returns -1 on error, or number of bytes read
int StdinListenerTask::readAll(int fd, UtlString* message)
{
   if ( message == NULL )
   {
      return -1;
   }

   char buf[1024]="";
   int rc;

   // read until all input has been read
   do
   {
      rc = TEMP_FAILURE_RETRY(read(fd, buf, sizeof(buf)));
      if (rc>0)
      {
         message->append(buf, rc);
      }
   } while (rc == sizeof(buf));

   // If output was obtained, return its length.
   // But if no output was obtained, return the 0 or -1 that
   // read() returned.
   if ( message->length() > 0)
   {
      rc = message->length();
   }
   return rc;
}

