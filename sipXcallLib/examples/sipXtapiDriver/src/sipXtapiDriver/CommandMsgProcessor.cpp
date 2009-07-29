//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

#ifdef TEST
#include <assert.h>
#include "utl/UtlMemCheck.h"
#endif //TEST

#ifdef __pingtel_on_posix__
#include <stdlib.h>
#endif

// APPLICATION INCLUDES
#include <sipXtapiDriver/CommandMsgProcessor.h>
#include <net/SipUserAgent.h>
#include <os/OsServerTask.h>
#include <os/OsTask.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
CommandMsgProcessor::CommandMsgProcessor(SipUserAgent* sipUserAgent)
{
#ifdef TEST
   if (!sIsTested)
   {
      sIsTested = true;
      test();
   }
#endif //TEST

   userAgent = sipUserAgent;
   numRespondToMessages = 0;
   userAgent->addMessageConsumer(this);
   mpResponseMessage = NULL;
   mpLastResponseMessage = NULL;
}

// Copy constructor
CommandMsgProcessor::CommandMsgProcessor(const CommandMsgProcessor& rCommandMsgProcessor)
{
}

// Destructor
CommandMsgProcessor::~CommandMsgProcessor()
{
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
CommandMsgProcessor&
CommandMsgProcessor::operator=(const CommandMsgProcessor& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

UtlBoolean CommandMsgProcessor::handleMessage(OsMsg& eventMessage)
{
        int msgType = eventMessage.getMsgType();
        // int msgSubType = eventMessage.getMsgSubType();

        if(msgType == OsMsg::PHONE_APP)
        // && msgSubType == CP_SIP_MESSAGE)
        {
                osPrintf("CommandMsgProcessor::handleMessage Got a message\n");
                int messageType = ((SipMessageEvent&)eventMessage).getMessageStatus();

                const SipMessage* sipMsg = ((SipMessageEvent&)eventMessage).getMessage();
                UtlString callId;
                if(sipMsg)
                {
                        osPrintf("numRespondToMessages: %d isResponse: %d messageType: %d TransErro: %d\n",
                                numRespondToMessages, sipMsg->isResponse(), messageType,
                                SipMessageEvent::TRANSPORT_ERROR);
                        if((numRespondToMessages == -1 || numRespondToMessages > 0) &&
                                !sipMsg->isResponse() && messageType != SipMessageEvent::TRANSPORT_ERROR)
                        {
                                osPrintf("valid message\n");
                                if(numRespondToMessages > 0)
                                {
                                        numRespondToMessages--;
                                }

                                SipMessage response;
                if(mpResponseMessage)
                {
                    response = *mpResponseMessage;
                }
                response.setResponseData(sipMsg, responseStatusCode, responseStatusText.data());

            UtlString address;
            int port;
            UtlString protocol;
            UtlString tag;

            sipMsg->getToAddress(&address,
               &port,
               &protocol,
               NULL,
               NULL,
               &tag) ;

            if( tag.isNull())
            {
               int tagNum = rand();
                                   char tag[100];
                                   sprintf(tag, "%d", tagNum);
               UtlString tagWithDot(tag);
               tagWithDot.append(".34756498567498567");
                                   response.setToFieldTag(tagWithDot);
            }

                                UtlString msgBytes;
                                int msgLen;
                                response.getBytes(&msgBytes, &msgLen);
                                osPrintf("%s",msgBytes.data());

                if(mpLastResponseMessage)
                {
                    delete mpLastResponseMessage;
                    mpLastResponseMessage = NULL;
                }
                // Keep a copy of the last response sent
                mpLastResponseMessage = new SipMessage(response);

                                if(userAgent->send(response))
                                {
                                        osPrintf("Sent response\n");
                                }
                                else
                                {
                                        osPrintf("Send failed\n");
                                }
                        }
                }
        }
        return(TRUE);
}

/* ============================ ACCESSORS ================================= */
void CommandMsgProcessor::stopResponding()
{
        numRespondToMessages = 0;
}

void CommandMsgProcessor::startResponding(int responseCode, const char* responseText,
                                         int numMessagesToRespondTo)
{
        numRespondToMessages = numMessagesToRespondTo;
        responseStatusCode = responseCode;
        responseStatusText.remove(0);
        if(responseText)
        {
                responseStatusText.append(responseText);
        }
    if(mpResponseMessage) delete mpResponseMessage;
    mpResponseMessage = NULL;
}

void CommandMsgProcessor::startResponding(const char* filename, int numMessagesToRespondTo )
{
        UtlString messageBuffer;
        char buffer[1025];
        int bufferSize = 1024;
        int charsRead;
        UtlString file = filename;

        numRespondToMessages = numMessagesToRespondTo;

        if (!file.isNull())
        {
                //get header parameters from files
                FILE* sipMessageFile = fopen(filename, "r");
                if(sipMessageFile)
                {
                        //printf("opened file: \"%s\"\n", argv[1]);
                        do
                        {
                                charsRead = fread(buffer, 1, bufferSize, sipMessageFile);
                                if(charsRead > 0)
                                {
                                        messageBuffer.append(buffer, charsRead);
                                }
                        }
                        while(charsRead);
                        fclose(sipMessageFile);
                        //messageBuffer.strip(UtlString::trailing, '\n');
                        //messageBuffer.strip(UtlString::trailing, '\r');

                        // Make sure there is a NULL at the end
                        messageBuffer.append("\0");
            if(mpResponseMessage) delete mpResponseMessage;
            mpResponseMessage = new SipMessage(messageBuffer);
            responseStatusCode = mpResponseMessage->getResponseStatusCode();
            mpResponseMessage->getResponseStatusText(&responseStatusText);

                }
        }
        //responseStatusText.append(messageBuffer);
}

void CommandMsgProcessor::resendLastResponse()
{
    if(mpLastResponseMessage)
    {
        if(userAgent->send(*mpLastResponseMessage))
        {
            osPrintf("Sent response\n");
        }
        else
        {
            osPrintf("Send failed\n");
        }
    }
    else
    {
        osPrintf("No previous response\n");
    }

}
/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ TESTING =================================== */

#ifdef TEST

// Set to true after the tests have been executed once
bool CommandMsgProcessor::sIsTested = false;

// Test this class by running all of its assertion tests
void CommandMsgProcessor::test()
{

   UtlMemCheck* pUtlMemCheck = 0;
   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   testCreators();
   testManipulators();
   testAccessors();
   testInquiry();

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

// Test the creators (and destructor) methods for the class
void CommandMsgProcessor::testCreators()
{
   UtlMemCheck* pUtlMemCheck  = 0;


   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // test the default constructor (if implemented)
   // test the copy constructor (if implemented)
   // test other constructors (if implemented)
   //    if a constructor parameter is used to set information in an ancestor
   //       class, then verify it gets set correctly (i.e., via ancestor
   //       class accessor method.
   // test the destructor
   //    if the class contains member pointer variables, verify that the
   //    pointers are getting scrubbed.

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

// Test the manipulator methods
void CommandMsgProcessor::testManipulators()
{
   UtlMemCheck* pUtlMemCheck  = 0;

   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // test the assignment method (if implemented)
   // test the other manipulator methods for the class

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

// Test the accessor methods for the class
void CommandMsgProcessor::testAccessors()
{
   UtlMemCheck* pUtlMemCheck  = 0;

   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // body of the test goes here

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

// Test the inquiry methods for the class
void CommandMsgProcessor::testInquiry()
{
   UtlMemCheck* pUtlMemCheck  = 0;


   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // body of the test goes here

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

#endif //TEST

/* ============================ FUNCTIONS ================================= */
