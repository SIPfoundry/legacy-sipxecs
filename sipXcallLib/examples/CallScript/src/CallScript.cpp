//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#define DEFAULT_SIP_PORT 5960
#define DEFAULT_RTP_PORT 9000
#define INPUT_LINE_SIZE 200
#define MAX_PAUSE_DURATION_MS (60*1000)

#include "os/OsDefs.h"
#include <assert.h>
#include <time.h>

#if defined(_WIN32)
#  include <windows.h>
#  define SLEEP(milliseconds) Sleep(milliseconds)
#else
#  include <unistd.h>
#  define SLEEP(milliseconds) usleep((milliseconds)*1000)
#endif

#include "tapi/sipXtapi.h"
#include "tapi/sipXtapiEvents.h"

SIPX_INST g_hInst = NULL;
SIPX_LINE g_hLine = 0;
SIPX_CALL g_hCall = 0;

// Print usage message
void usage(const char* szExecutable)
{
   fprintf(stderr, "\nUsage:\n");
   fprintf(stderr, "   %s <options> var=value ...\n", szExecutable);
   fprintf(stderr, "\n");
   fprintf(stderr, "Options:\n");
   fprintf(stderr, "   -p SIP port (default = 5060)\n");
   fprintf(stderr, "   -r RTP port start (default = 9000)\n");
   fprintf(stderr, "   -l label to prefix to output\n");
   fprintf(stderr, "\n");
}


// Parse arguments
bool parseArgs(int argc, char *argv[], int* pSipPort, int* pRtpPort,
               const char** pszLabel)
{
   for (int i=1; i<argc; i++)
   {
      if (strcmp(argv[i], "-p") == 0)
      {
         if ((i+1) < argc)
         {
            *pSipPort = atoi(argv[++i]);
         }
         else
         {
            break; // Error
         }
      }
      else if (strcmp(argv[i], "-r") == 0)
      {
         if ((i+1) < argc)
         {
            *pRtpPort = atoi(argv[++i]);
         }
         else
         {
            break; // Error
         }
      }
      else if (strcmp(argv[i], "-l") == 0)
      {
         if ((i+1) < argc)
         {
            *pszLabel = strdup(argv[++i]);
         }
         else
         {
            break; // Error
         }
      }
      else if (strchr(argv[i], '=') != NULL)
      {
         // Argument contains '=', so is a variable substitution.
      }
      else
      {
         return 0;
      }
   }

   return 1;
}


// Place a call to szSipUrl as szFromIdentity
bool placeCall(const char* szSipUrl, const char* szFromIdentity)
{
    printf("<-> Placing call to \"%s\" as \"%s\"\n", szSipUrl, szFromIdentity);

    sipxLineAdd(g_hInst, szFromIdentity, false, &g_hLine);
    sipxCallCreate(g_hInst, g_hLine, &g_hCall);
    sipxCallConnect(g_hCall, szSipUrl);

    SLEEP(5*1000);

    return true;
}


// Drop call, clean up resources
bool shutdownCall()
{
    printf("<-> Shutting down Call\n");

    sipxCallDestroy(g_hCall);
    sipxLineRemove(g_hLine);

    SLEEP(2*1000);

    return true;
}


// Play a series of tones
bool playTones(char* szPlayTones)
{
    bool bRC = true;

    while (*szPlayTones)
    {
        int toneId = *szPlayTones++;

        if (    (toneId >= '0' && toneId <= '9') ||
                (toneId == '#') || (toneId == '*') || toneId == ',')
        {
            if (toneId == ',')
            {
                printf("<-> Playtone: Sleeping for 2 seconds\n");
                SLEEP(2000);
            }
            else
            {
                printf("<-> Playtone: %c\n", toneId);
                SLEEP(250);
                sipxCallStartTone(g_hCall, (TONE_ID) toneId, true, true);
                SLEEP(500);
                sipxCallStopTone(g_hCall);
            }
        }
        else
        {
            bRC = false;
            break;
        }

    }

    return bRC;
}


bool playFile(char* szFile)
{
    sipxCallPlayFile(g_hCall, szFile, true, true);

    return true;
}


int main(int argc, char* argv[])
{
   // Port numbers to use for SIP and RTP.
   int iSipPort = DEFAULT_SIP_PORT, iRtpPort = DEFAULT_RTP_PORT;
   // Label to put on output lines.
   const char* szLabel = "";
   // Buffer for input lines.
   char input_line_raw[INPUT_LINE_SIZE];
   // After variables have been substituted.
   char input_line[INPUT_LINE_SIZE];
   // Pointers to the first two tokens on the input line.
   char *token1, *token2;
   char* p;
   char* scanner;

   // Parse Arguments
   if (!(parseArgs(argc, argv, &iSipPort, &iRtpPort, &szLabel) &&
         portIsValid(iSipPort) && portIsValid(iRtpPort)))
   {
      usage(argv[0]);
      exit(1);
   }

   // Initialize the random number generator.
   // Using getpid() ensures that the seed value differs for every process.
   // Multiplying it by time() ensures that the seed values differ a
   // great deal even for processes whose PID's differ by only 1.
   srandom(getpid() * time(NULL));

   // initialize sipx TAPI-like API
   sipxInitialize(&g_hInst, iSipPort, iSipPort, iRtpPort);

   // Read and execute lines from stdin.
   for (;;)
   {
      int i, j, k, var_name_length, value_length = 0;

      // Read a line.
      p = fgets(input_line_raw, sizeof (input_line), stdin);
      // If we see EOF, exit successfully.
      if (p == NULL)
      {
         exit(0);
      }

      // Substitute variables.
      // i will scan input_line_raw, j will scan input_line.
      // fgets always puts a NUL at the end of the data in input_line_raw.
      for (i = 0, j = 0; input_line_raw[i] != '\0';)
      {
         // Check for a variable substitution.
         if (input_line_raw[i] == '$' && input_line_raw[i+1] == '{')
         {
            // Find and substitute the variable.
            // Find the end of the variable name.
            p = strchr(&input_line_raw[i+2], '}');
            if (p == NULL)
            {
               fprintf(stderr,
                      "%sUnable to find the end of variable reference '%s'\n",
                      szLabel, &input_line_raw[i]);
               exit(1);
            }
            // Find the last matching argument.
            var_name_length = p - &input_line_raw[i+2];
            for (k = argc - 1; k > 0; k--)
            {
               if (strncmp(&input_line_raw[i+2], argv[k],
                           var_name_length) == 0 &&
                   argv[k][var_name_length] == '=')
               {
                  // This is the variable, substitute its value.
                  // Ensure there is room for it in input_line.
                  value_length = strlen(&argv[k][var_name_length+1]);
                  if (j + value_length >= INPUT_LINE_SIZE - 1)
                  {
                     input_line[j] = '\0';
                     fprintf(stderr,
                             "%sLine too long after substitutions: %s\n",
                             input_line);
                     exit(1);
                  }
                  break;
               }
            }
            // (After for(k = ...).)
            // Check if the variable name was found.
            if (k == 0)
            {
               fprintf(stderr, "%sNo value given for variable '%.*s'\n",
                       var_name_length, &input_line_raw[i+2]);
               exit(1);
            }
            strcpy(&input_line[j], &argv[k][var_name_length+1]);
            j += value_length;
            i += var_name_length + 3;
         }
         else
         {
            // Copy this character.
            // Ensure there is room for it in input_line.
            if (j >= INPUT_LINE_SIZE - 1)
            {
               input_line[j] = '\0';
               fprintf(stderr, "%sLine too long after substitutions: %s\n",
                       input_line);
               exit(1);
            }
            input_line[j++] = input_line_raw[i++];
         }
      }
      // Add a NUL to the end of input_line.  We have arranged that there will
      // always be space for it.
      input_line[j] = '\0';

#if 0
      fprintf(stderr, "Raw: %s", input_line_raw);
      fprintf(stderr, "Ckd: %s", input_line);
      fprintf(stderr, "\n");
#endif

      // Find the first non-whitespace character.
      p = input_line + strspn(input_line, " \t\f\r");
      // If it is newline, this line is empty and should be ignored.
      // If it is #, this line is a comment and should be ignored.
      // If it is NUL, this line is too long and this segment should
      // be ignored.
      if (*p == '\n' || *p == '#' || *p == '\0')
      {
         continue;
      }

      // Get the first two tokens.
      token1 = strtok_r(p, " \t\f\r\n", &scanner);
      token2 = strtok_r(NULL, " \t\f\r\n", &scanner);
      // If the first token is absent, ignore this line.  (Should never
      // happen.)
      if (token1 == NULL)
      {
         continue;
      }

      // Process the first token to see what command it is.
      if (strcmp(token1, "call") == 0)
      {
         // The CALL command.
         if (token2 == NULL)
         {
            fprintf(stderr, "%sCALL command requires SIP URL.\n", szLabel);
            exit(1);
         }
         fprintf(stderr, "%scall %s\n", szLabel, token2);
         if (!placeCall(token2, "\"Demo\" <sip:demo@example.bogus>"))
         {
            // Failed to start the call.
            fprintf(stderr, "%sUnable to initiate call to '%s'\n", szLabel,
                    token2);
            exit(1);
         }
      }
      else if (strcmp(token1, "tones") == 0)
      {
         // The TONES command.
         if (token2 == NULL)
         {
            fprintf(stderr, "%sTONES command requires a string.\n", szLabel);
            exit(1);
         }
         fprintf(stderr, "%stones %s\n", szLabel, token2);
         if (!playTones(token2))
         {
            fprintf(stderr, "%sUnable to play tones '%s'\n", szLabel, token2);
            exit(1);
         }
      }
      else if (strcmp(token1, "file") == 0)
      {
         // The FILE command.
         if (token2 == NULL)
         {
            fprintf(stderr, "%sFILE command requires a file name.\n", szLabel);
            exit(1);
         }
         fprintf(stderr, "%sfile %s\n", szLabel, token2);
         if (!playFile(token2))
         {
            fprintf(stderr, "%sUnable to play file '%s'\n", szLabel, token2);
            exit(1);
         }
      }
      else if (strcmp(token1, "pause") == 0)
      {
         // The PAUSE command.
         if (token2 == NULL)
         {
            fprintf(stderr, "%sPAUSE command requires a wait duration.\n",
                    szLabel);
            exit(1);
         }
         if (strcmp(token2, "random") == 0)
         {
            // The PAUSE RANDOM command.
            char *token3 = strtok_r(NULL, " \t\f\r\n", &scanner);
            if (token3 == NULL)
            {
               fprintf(stderr,
                       "%sPAUSE RANDOM command requires a wait duration.\n",
                       szLabel);
               exit(1);
            }

            char* endptr;
            long int v = strtol(token3, &endptr, 10);
            if (*endptr != '\0' || v < 0 || v > MAX_PAUSE_DURATION_MS)
            {
               fprintf(stderr,
                       "%sInvalid time in PAUSE RANDOM '%s'\n", szLabel,
                       token3);
               exit(1);
            }
            long int wait = random() % v;
            fprintf(stderr, "%spause random %ld (pause %ld)\n", szLabel, v,
                    wait);
            SLEEP(wait);
         }
         else
         {
            // The plain PAUSE command
            char* endptr;
            long int v = strtol(token2, &endptr, 10);
            if (*endptr != '\0' || v < 0 || v > MAX_PAUSE_DURATION_MS)
            {
               fprintf(stderr, "%sInvalid time in PAUSE '%s'\n", szLabel,
                       token2);
               exit(1);
            }
            fprintf(stderr, "%spause %ld\n", szLabel, v);
            SLEEP(v);
         }
      }
      else if (strcmp(token1, "hangup") == 0)
      {
         // The HANGUP command.
         fprintf(stderr, "%shangup\n", szLabel);
         if (!shutdownCall())
         {
            fprintf(stderr, "%sUnable to shutdown call\n", szLabel);
         }
      }
      else
      {
         // Unknown command.
         fprintf(stderr, "%sUnknown command: '%s'\n", szLabel, token1);
         exit(1);
      }
   }
}


#if !defined(_WIN32)

// Dummy definition of JNI_LightButton() to prevent the reference in
// sipXcallLib from producing an error.

void JNI_LightButton(long)
{

}

#endif /* !defined(_WIN32) */
