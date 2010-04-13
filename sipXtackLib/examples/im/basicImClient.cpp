//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <stdio.h>
#if defined(_WIN32)
# include <io.h>
# define STDIN_FILENO 0 /* can't find where windows defines this */
#else
# include <unistd.h>
#endif

// APPLICATION INCLUDES
#include <net/SipUserAgent.h>
#include <net/SipPimClient.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS
const char* toAor = NULL;
const char* fromAor = NULL;
int sipPort = 5060;

/* ============================ FUNCTIONS ================================= */

void usage(const char* name)
{
    printf("Usage:\n\tname -f <fromAor> -t <toAor> -p <sipPort>\n");
}

int parseArgs(int argc, const char* argv[])
{
    int argIndex = 1;
    while(argIndex < argc)
    {
        if(argv[argIndex][0] == '-')
        {
            switch(argv[argIndex][1])
            {
            case 'f':
                argIndex++;
                fromAor = argv[argIndex];

                break;

            case 'p':
                argIndex++;
                sipPort = atoi(argv[argIndex]);

                break;

            case 't':
                argIndex++;
                toAor = argv[argIndex];

                break;

            default:
                printf("Unknown option: %s\n", argv[argIndex]);
                usage(argv[0]);
                exit(1);
                break;
            }
        }

        else
        {
            printf("Unexpected argument: %s\n", argv[argIndex]);
            usage(argv[0]);
            exit(1);
        }

        argIndex++;

    }

    return(0);
}

void imTextPrinter(const UtlString& fromAddress,
                                 const char* textMessage,
                                 int textLength,
                                 const SipMessage& messageRequest)
{
    Url fromUrl(fromAddress);
    UtlString displayName;
    fromUrl.getDisplayName(displayName);
    if(displayName.isNull())
    {
        fromUrl.getUserId(displayName);
    }

    printf("\n%s: %s\n", displayName.data(), textMessage);
}

int main(int argc, const char* argv[])
{

    if(argc < 6)
    {
        printf("not enough arguments\n");
        usage(argv[0]);
        exit(1);
    }

    // Get the options and parameters
    parseArgs(argc, argv);

    printf("From: %s\n", fromAor ? fromAor : "null");
    printf("To: %s\n", toAor ? toAor : "null");
    printf("SIP Port: %d\n", sipPort);

    // Create a user agent
    SipUserAgent userAgent(sipPort, sipPort);

    // Create an IM client
    Url toUrl(toAor);
    Url fromUrl(fromAor);
    UtlString fromDisplay;
    fromUrl.getDisplayName(fromDisplay);
    if(fromDisplay.isNull())
    {
        fromUrl.getUserId(fromDisplay);
    }

    if(fromDisplay.isNull())
    {
        fromDisplay = "me";
    }

    SipPimClient imClient(userAgent, fromUrl);
    imClient.start();

    // Register a call back to print out the incoming message
    imClient.setIncomingImTextHandler(imTextPrinter);

    // Prompt if not running in batch mode
    UtlBoolean doPrompt = isatty(STDIN_FILENO);
    if ( doPrompt )
    {
       printf("Enter IM text or -h for help\n");
       printf("%s: ", fromDisplay.data());
    }

    // Main loop: collect text and send it
    UtlBoolean done = FALSE;
    UtlBoolean isCommand = FALSE;
    char* commandLine = NULL;
    char buffer[1024];

    while(!done &&
        (commandLine = fgets(buffer,1023,stdin)))
    {

        isCommand = FALSE;
        if(*commandLine == '-')
        {
            switch(commandLine[1])
            {
            case 'a': // away
                isCommand = TRUE;
                // set the state to closed or away
                break;

            case 'h':
            case '?':
                isCommand = TRUE;
                printf("Enter: \n\t-h for help\n\t-q to quit\n");
                break;

            case 'p': // present
                isCommand = TRUE;
                // set the state to open or present
                break;

            case 'q':
                done = TRUE;
                isCommand = TRUE;
                break;

            default:
                break;
            }
        }

        // Assume it is text to send to the other end
        if(!isCommand && *commandLine)
        {
            int sipStatusCode = -1;
            UtlString sipStatusText;

            imClient.sendPagerMessage(toUrl, commandLine,
                sipStatusCode, sipStatusText);

            if(sipStatusCode >= 300)
            {
                printf("Failed to send message: %s\n",
                    sipStatusText.data());
            }
        }

        if ( doPrompt )
        {
            printf("%s: ", fromDisplay.data());
        }
    }
    return(0);
}


/* ============================ TESTING =================================== */
