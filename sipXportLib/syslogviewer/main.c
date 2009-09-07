//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#if defined(_WIN32)
#   include <io.h>
#elif defined(__pingtel_on_posix__)
#   include <unistd.h>
#endif

#define BUFFER_SIZE 8192

// Configuration variables.
// If 1, continuation lines produced by \n should be indented with a TAB.
static int indent = 0;
// If 1, long lines should be folded.
static int fold = 0;
// The line length for folding.
static int width = 80;
// Whether or not to pipe output through a pager when stdout is a tty
static int UsePager = 1;
const char* DefaultPager = "less";

// Processing state.
static char buffer[BUFFER_SIZE];
static int buffer_full = 0;

static pid_t pagerPid;

static enum {
   StartLine,
   InTime,
   InFields,
   InMessage,
   Backslash
} state;

// The number of print spaces in the current line that have been consumed.
static int position;
// If 1, the current output line was indented with a tab, and any
// folded lines from it should also have a tab.
static int line_tabbed;

static void flush_buffer(int fd)
{
   ssize_t dummy;
   dummy = write(fd, buffer, buffer_full);
   buffer_full = 0;
}

/* If folding is on, check the position in the line and break it if necessary
 * so there is room to insert another graphic character.
 */
static void check_position(int fd)
{
   if (fold && position == width)
   {
      // Must break the line.
      // Insert a newline.
      if (buffer_full == BUFFER_SIZE)
      {
         flush_buffer(fd);
      }
      buffer[buffer_full++] = '\n';
      position = 0;
      // Insert a tab if the line we are breaking had an added tab.
      if (line_tabbed)
      {
         if (buffer_full == BUFFER_SIZE)
         {
            flush_buffer(fd);
         }
         buffer[buffer_full++] = '\t';
         position = 8;
      }
      // Insert 4 spaces.
      if (buffer_full == BUFFER_SIZE)
      {
         flush_buffer(fd);
      }
      buffer[buffer_full++] = ' ';
      if (buffer_full == BUFFER_SIZE)
      {
         flush_buffer(fd);
      }
      buffer[buffer_full++] = ' ';
      if (buffer_full == BUFFER_SIZE)
      {
         flush_buffer(fd);
      }
      buffer[buffer_full++] = ' ';
      if (buffer_full == BUFFER_SIZE)
      {
         flush_buffer(fd);
      }
      buffer[buffer_full++] = ' ';
      position += 4;
   }
}

static void process_char(char ch, int fd)
{
   switch(state)
   {
   case StartLine:
      if (ch == '"')
      {
         state = InTime;
      }
      else
      {
         buffer[buffer_full++] = ch;
         position++;
      }
      break;

   case InTime:
      if (ch == '"')
      {
         state = InFields;
      }
      buffer[buffer_full++] = ch;
      position++;
      break;

   case InFields:
      if (ch == '"')
      {
         buffer[buffer_full++] = '\n';
         check_position(fd);
         state = InMessage;
         buffer[buffer_full++] = '\t';
         position = 0;
         line_tabbed = 1;
      }
      else
      {
         buffer[buffer_full++] = ch;
         position++;
      }
      break;

   case InMessage:
      if (ch == '\\')
      {
         state = Backslash;
      }
      else if (ch == '\n')
      {
         // Newline never forces a folding break, and position is 0 afterward.
         buffer[buffer_full++] = '\n';
         position = 0;
         line_tabbed = 0;
      }
      else if (ch == '"')
      {
         // Unescaped End of the message
         state = indent ? StartLine : InMessage;
         position = 0;
         line_tabbed = 0;
      }
      else
      {
         // Ordinary characters.
         check_position(fd);
         buffer[buffer_full++] = ch;
         position++;
      }
      break;

   case Backslash:
      if (ch == '\\')
      {
         check_position(fd);
         buffer[buffer_full++] = '\\';
         position++;
      }
      else if (ch == 'n')
      {
         // Insert newline.
         buffer[buffer_full++] = '\n';
         // Indent the continuation line if it was requested.
         if (indent)
         {
            if (buffer_full == BUFFER_SIZE)
            {
               flush_buffer(fd);
            }
            buffer[buffer_full++] = '\t';
            // Set the position to account for the tab.
            position = 8;
            line_tabbed = 1;
         }
         else
         {
            position = 0;
         }
      }
      else if (ch != 'r')
      {
         // Other backslash escapes are output as such.
         check_position(fd);
         buffer[buffer_full++] = '\\';
         position++;
         if (buffer_full == BUFFER_SIZE)
         {
            flush_buffer(fd);
         }
         check_position(fd);
         buffer[buffer_full++] = ch;
         position++;
      }
      state = InMessage;
      break;
   }
   if (buffer_full == BUFFER_SIZE)
      flush_buffer(fd);
}

void wait_for_pager()
{
   if (pagerPid)
   {
      int childStatus;
      waitpid(pagerPid, &childStatus, 0);
   }
}

int main(int argc, char * argv[])
{
   char input_buffer[BUFFER_SIZE];
   int i;
   int ifd = 0;
   int ofd = 1;

   for(i = 1; i < argc; i++)
   {
      if (   (!strcmp(argv[i], "-h"))
          || (!strcmp(argv[i], "--help"))
          )
      {
         const char* usage =
            "Usage:\n"
            "\t%s [-h | --help] [-i | --indent] [-f[nn]]\n"
            "\t\t[of=output]\n"
            "\t\t[input]\n"
            "\n"
            "\t--help\tPrint this help message.\n"
            "\t--indent\tIndent messages and continued lines.\n"
            "\t--no-pager\tDo not pipe the output through a pager, even if it is a tty.\n"
            "\t-f[nn]\tFold lines that are over nn (default 80) characters.\n"
            "\t\tImplies --indent.\n"
            "\tif=<file>\tinput file name (deprecated - use file name directly).\n"
            "\tof=<file>\toutput file name (defaults to stdout)\n"
            "\tinput - input file name (defaults to stdin)\n"
            "\n"
            "\tIf stdout is a tty, pipes output through the program specified\n"
            "\tby the PAGER environment variable (defaults to '%s').\n"
            ;
         fprintf(stderr, usage, argv[0], DefaultPager);

         return 0;
      }
      else if (   (strcmp(argv[i], "-i") == 0)
               || (strcmp(argv[i], "--indent") == 0)
               )
      {
         indent = 1;
      }
      else if (strcmp(argv[i], "--no-pager") == 0)
      {
         UsePager=0;
      }
      else if (!strncmp(argv[i], "if=", 3))
      {
         ifd = open(&argv[i][3], O_RDONLY);
         if (ifd == -1)
         {
            fprintf(stderr, "%s: %s\n", &argv[i][3], strerror(errno));
            return 1;
         }
      }
      else if (!strncmp(argv[i], "of=", 3))
      {
         ofd = open(&argv[i][3], O_WRONLY | O_CREAT, 0644);
         if (ofd < 0)
         {
            fprintf(stderr, "%s: %s\n", &argv[i][3], strerror(errno));
            return 1;
         }
      }
      else if (!strncmp(argv[i], "-f", 2))
      {
         fold = 1;
         indent = 1;
         if (strlen(argv[i]) > 2)
         {
            char* endptr;
            width = strtol(argv[i] + strlen("-f"), &endptr, 10);
            if (endptr != argv[i] + strlen(argv[i]))
            {
               fprintf(stderr, "Badly formatted width: '%s'\n",
                       argv[i]);
               return 1;
            }
            else if (width < 9)
            {
               fprintf(stderr, "Width value too small: %d\n", width);
               return 1;
            }
         }
      }
      else if (!strncmp(argv[i], "-", 1))
      {
         fprintf(stderr, "Unknown option: %s\n", argv[i]);
         return 1;
      }
      else
      {
         ifd = open(argv[i], O_RDONLY);
         if (ifd == -1)
         {
            fprintf(stderr, "%s: %s\n", argv[i], strerror(errno));
            return 1;
         }
      }
   }

   if (UsePager && isatty(ofd))
   {

      int pagerPipe[2] = {-1,-1};
      if (!pipe(pagerPipe))
      {
         pagerPid = fork();
         if (pagerPid)
         {
            // this is the parent, so change output to go to the child

            close(pagerPipe[0]); // don't need the read end of the pipe
            close(ofd);          // the pager child takes care of this one now
            ofd = pagerPipe[1];  // write output to the pager child

            atexit(wait_for_pager);
         }
         else
         {
            // this is the child that will become the pager
            close(ifd);          // close the original input - the parent reads that
            close(pagerPipe[1]); // don't need the write end of the pipe

            dup2(pagerPipe[0], STDIN_FILENO);  // the child reads from the read end of the pipe
            dup2(ofd,          STDOUT_FILENO); // the child writes to the original output

            const char* pager;
            if (!(pager = getenv("PAGER")))
            {
               pager = DefaultPager;
            }

            execlp( pager, pager, (char*)NULL );
         }
      }
      else
      {
         // pipe call failed
         fprintf(stderr, "failed to open pipe for pager: %s\n", strerror(errno));
         return 1;
      }
   }

   position = 0;
   line_tabbed = 0;
   i = read(ifd, input_buffer, BUFFER_SIZE);
   state = indent ? StartLine : InMessage;

   while(i && i != -1)
   {
      int j;
      for(j = 0; j != i; j++)
      {
         process_char(input_buffer[j], ofd);
      }
      i = read(ifd, input_buffer, BUFFER_SIZE);
   }

   flush_buffer(ofd);
   close(ofd);
   close(ifd);

   return 0;
}
