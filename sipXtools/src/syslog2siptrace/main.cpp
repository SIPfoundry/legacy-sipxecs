//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
////////////////////////////////////////////////////////////////////////

// Cloned from syslogviewer

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#if defined(_WIN32)
#   include <io.h>
#   include <string.h>
#elif defined(__pingtel_on_posix__)
#   include <unistd.h>
#endif

#define BUFFER_SIZE 8192

#include <os/OsDefs.h>
#include <os/OsSysLog.h>
#include <net/NameValueTokenizer.h>
#include <net/SipMessage.h>

void writeMessageNodesBegin(int outputFileDescriptor)
{
    UtlString nodeBegin("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>\n<sipTrace>\n");
    ssize_t dummy;
    dummy = write(outputFileDescriptor, nodeBegin.data(), nodeBegin.length());
}

void writeMessageNodesEnd(int outputFileDescriptor)
{
    UtlString nodeEnd("</sipTrace>\n");
    ssize_t dummy;
    dummy = write(outputFileDescriptor, nodeEnd.data(), nodeEnd.length());
}

void writeBranchNodeBegin(int outputFileDescriptor)
{
    UtlString nodeBegin("\t<branchNode>\n");
    ssize_t dummy;
    dummy = write(outputFileDescriptor, nodeBegin.data(), nodeBegin.length());
}

void writeBranchNodeEnd(int outputFileDescriptor)
{
    UtlString nodeEnd("\t</branchNode>\n");
    ssize_t dummy;
    dummy = write(outputFileDescriptor, nodeEnd.data(), nodeEnd.length());
}

void writeBranchSetBegin(int outputFileDescriptor)
{
    UtlString nodeBegin("\t\t<branchIdSet>\n");
    ssize_t dummy;
    dummy = write(outputFileDescriptor, nodeBegin.data(), nodeBegin.length());
}

void writeBranchSetEnd(int outputFileDescriptor)
{
    UtlString nodeEnd("\t\t</branchIdSet>\n");
    ssize_t dummy;
    dummy = write(outputFileDescriptor, nodeEnd.data(), nodeEnd.length());
}

void writeBranchId(int outputFileDescriptor,
                   UtlString& branchId)
{
    NameValueTokenizer::frontBackTrim(&branchId, " \t\n\r");
    UtlString node("\t\t\t<branchId>");
    node.append(branchId);
    node.append("</branchId>\n");

    ssize_t dummy;
    dummy = write(outputFileDescriptor, node.data(), node.length());
}

void writeBranchNodeData(int outputFileDescriptor,
                         UtlString& time,
                         UtlString& source,
                         UtlString& destination,
                         UtlString& sourceAddress,
                         UtlString& destinationAddress,
                         UtlBoolean& isOutgoing,
                         UtlString& transactionId,
                         UtlString& frameId,
                         UtlString& method,
                         UtlString& responseCode,
                         UtlString& responseText,
                         UtlString& message)
{
    NameValueTokenizer::frontBackTrim(&time, " \t\n\r");
    NameValueTokenizer::frontBackTrim(&source, " \t\n\r");
    NameValueTokenizer::frontBackTrim(&destination, " \t\n\r");
    NameValueTokenizer::frontBackTrim(&sourceAddress, " \t\n\r");
    NameValueTokenizer::frontBackTrim(&destinationAddress, " \t\n\r");
    NameValueTokenizer::frontBackTrim(&transactionId, " \t\n\r");
    NameValueTokenizer::frontBackTrim(&frameId, " \t\n\r");
    NameValueTokenizer::frontBackTrim(&method, " \t\n\r");
    NameValueTokenizer::frontBackTrim(&responseCode, " \t\n\r");
    NameValueTokenizer::frontBackTrim(&responseText, " \t\n\r");
    //NameValueTokenizer::frontBackTrim(&message, " \t\n\r");

    UtlString node("\t\t<time>");
    node.append(time);
    node.append("</time>\n");

    if(!source.isNull())
    {
        node.append("\t\t<source>");
        node.append(source);
        node.append("</source>\n");
    }

    if(!destination.isNull())
    {
        node.append("\t\t<destination>");
        node.append(destination);
        node.append("</destination>\n");
    }

    node.append("\t\t<sourceAddress>");
    node.append(sourceAddress);
    node.append("</sourceAddress>\n");

    node.append("\t\t<destinationAddress>");
    node.append(destinationAddress);
    node.append("</destinationAddress>\n");

    node.append("\t\t<transactionId>");
    node.append(transactionId);
    node.append("</transactionId>\n");

    if(!method.isNull())
    {
        node.append("\t\t<method>");
        node.append(method);
        node.append("</method>\n");
    }
    else
    {
        node.append("\t\t<responseCode>");
        node.append(responseCode);
        node.append("</responseCode>\n");

        node.append("\t\t<responseText>");
        node.append(responseText);
        node.append("</responseText>\n");
    }

    node.append("\t\t<frameId>");
    node.append(frameId);
    node.append("</frameId>\n");

    if (isOutgoing)
    {
       node.append("\t\t<remoteHostPort>");
       node.append(destinationAddress);
       node.append("</remoteHostPort>\n");

       node.append("\t\t<isOutgoing>");
       node.append("true");
       node.append("</isOutgoing>\n");
    }
    else
    {
       node.append("\t\t<remoteHostPort>");
       node.append(sourceAddress);
       node.append("</remoteHostPort>\n");

       node.append("\t\t<isOutgoing>");
       node.append("false");
       node.append("</isOutgoing>\n");
    }

    node.append("\t\t<message><![CDATA[");
    node.append(message);
    node.append("]]></message>\n");

    ssize_t dummy;
    dummy = write(outputFileDescriptor, node.data(), node.length());
}

void getMessageData(UtlString& content,
                   UtlBoolean isOutgoing,
                   UtlString& date,
                   UtlString& hostname,
                   UtlString& eventCount,
                   int outputFileDescriptor)
{
    UtlString remoteHostPort;   // remote host-port
    UtlString remoteAddress;    // remote address from "Remote Host:"
    UtlString remotePort;       // remote port from "Port:"
    UtlString localHostPort;    // local host-port, if known
    UtlString localAddress;     // local address from "Remote Host:", if known
    UtlString localPort;        // local port from "Port:", if known
    UtlString topViaHostPort;   // host-port reported in top Via
    UtlBoolean topViaIsRemote;  // true if top Via describes the remote element
    UtlString message;
    UtlString branchId;
    UtlString transactionId;
    UtlString method;
    UtlString responseCode;
    UtlString responseText;
    UtlBoolean failed = FALSE;

    // Search for "----Remote Host:" to identify usable lines.
    ssize_t hostIndex = content.index("----Remote Host:");
    if (hostIndex > 0)
    {
        // Obtain remoteAddress and remotePort.

        hostIndex += 16;
        ssize_t hostEnd = content.index("----", hostIndex);
        remoteAddress.append(&(content.data()[hostIndex]),
                             hostEnd - hostIndex);

        size_t portIndex = hostEnd + 11;
        ssize_t portEnd = content.index("----", portIndex);
        remotePort.append(&(content.data()[portIndex]),
                          portEnd - portIndex);

        remoteHostPort = remoteAddress;
        remoteHostPort.append(":");
        remoteHostPort.append(remotePort);

        size_t messageIndex = portEnd + 5;
        ssize_t messageEnd;

        // Extract the SIP message text.

        if (isOutgoing)
        {
            messageEnd = content.index("--------------------END", messageIndex);
            // Record whether the send failed or not.
            failed = (content.index("User Agent failed to send message") !=
                      UTL_NOT_FOUND);
        }
        else
        {
            messageEnd = content.index("====================END", messageIndex);
        }
        if ((ssize_t) UTL_NOT_FOUND == messageEnd)
        {
           messageEnd = content.length();
        }
        message.append(&(content.data()[messageIndex]),
                          messageEnd - messageIndex);
        SipMessage sipMsg(message);

        // Perform processing dependent on whether the message is a
        // request or response.
        if (sipMsg.isResponse())
        {
           // Get the response code and text.
           sipMsg.getFirstHeaderLinePart(1, &responseCode);
           sipMsg.getFirstHeaderLinePart(2, &responseText);
           // Prepend FAILED if it is a failed transmission.
           if (failed)
           {
              responseCode = "FAILED " + responseCode;
           }
        }
        else
        {
            // Get the method.
            sipMsg.getRequestMethod(&method);

            // If it is a re-INVITE, change the method to "re-INVITE".
            if (method.compareTo("INVITE", UtlString::ignoreCase) == 0)
            {
               Url to;
               sipMsg.getToUrl(to);
               UtlString toTag;
               to.getFieldParameter("tag", toTag);
               if (!toTag.isNull())
               {
                  method = "re-INVITE";
               }
            }

            // Prepend FAILED if it is a failed transmission.
            if (failed)
            {
               method = "FAILED " + method;
            }
        }

        // Determine whether the top Via host-port describes the remote end
        // of this message.  That is, if this is an incoming request or an
        // outgoing response.
        // However, we can only get additional information if this is an incoming
        // request, since if it is an outgoing response, the remoteHostPort
        // is the same as the top Via host-port.
        topViaIsRemote = !isOutgoing && !sipMsg.isResponse();
        if (topViaIsRemote) 
        {
           // Extract the top Via host-port.

           UtlString protocol;
           int viaPortNum;
           sipMsg.getTopVia(&topViaHostPort,
                            &viaPortNum,
                            &protocol);
           topViaHostPort.append(":");

           /*
            * Translating an unspecified port to 5060 may not
            * always be correct:  if the host specifier is a name
            * rather than an address, then port _could_ be
            * resolved using RFC 3263, but at this time we know of
            * no implementations that do this (much less rely on
            * it).  Since leaving the viaPortNum as PORT_NONE
            * results in more confusing displays in the normal
            * case, this is a good compromise.
            */
           topViaHostPort.appendNumber(viaPortNum == PORT_NONE ? SIP_PORT : viaPortNum);
        }

        // Now get the local address/port, if it is present.
        hostIndex = content.index("----Local Host:");
        if ((ssize_t) UTL_NOT_FOUND != hostIndex)
        {
           hostIndex += 15;
           hostEnd = content.index("----", hostIndex);
           localAddress.append(&(content.data()[hostIndex]),
                               hostEnd - hostIndex);

           portIndex = hostEnd + 11;
           portEnd = content.index("----", portIndex);
           localPort.append(&(content.data()[portIndex]),
                             portEnd - portIndex);

           localHostPort = localAddress;
           localHostPort.append(":");
           localHostPort.append(localPort);
        }

        // transaction token: [C/A]cseq-number,call-id,from-tag,to-tag
        int cseq;
        UtlString cseqMethod;
        sipMsg.getCSeqField(&cseq, &cseqMethod);
        char numBuf[20];
        // Prepend to the CSeq number a "C" for the CANCEL transaction, and
        // an "A" for the ACK transaction.
        sprintf(numBuf, "%s%d",
                cseqMethod.compareTo("CANCEL", UtlString::ignoreCase) == 0 ? "C" :
                cseqMethod.compareTo("ACK", UtlString::ignoreCase) == 0 ? "A" :
                "",
                cseq);
        UtlString callId;
        sipMsg.getCallIdField(&callId);
        Url to;
        sipMsg.getToUrl(to);
        UtlString toTag;
        to.getFieldParameter("tag", toTag);
        Url from;
        sipMsg.getFromUrl(from);
        UtlString fromTag;
        from.getFieldParameter("tag", fromTag);

        transactionId = numBuf;
        transactionId.append(",");
        transactionId.append(callId);
        transactionId.append(",");
        transactionId.append(fromTag);
        transactionId.append(",");
        transactionId.append(toTag);

        // Write all the stuff out

        // Write out the node container start
        writeBranchNodeBegin(outputFileDescriptor);

        // Write out the branchId container start
        writeBranchSetBegin(outputFileDescriptor);

        // Write out the branchIds
        int viaIndex = 0;
        UtlString topVia;
        while(sipMsg.getViaFieldSubField(&topVia, viaIndex))
        {
            SipMessage::getViaTag(topVia.data(),
                                  "branch",
                                  branchId);
            writeBranchId(outputFileDescriptor, branchId);
            viaIndex++;
        }

        // Write out the branchId container finish
        writeBranchSetEnd(outputFileDescriptor);

        // Write out the rest of the node data
        writeBranchNodeData(outputFileDescriptor,
                            date,
                            // source
                            isOutgoing ? hostname :
                            (topViaIsRemote ? topViaHostPort : remoteHostPort),
                            // destination
                            isOutgoing ? remoteHostPort : hostname,
                            // sourceAddress
                            isOutgoing ?
                            (remoteHostPort.isNull() ? hostname : remoteHostPort) :
                            remoteHostPort,
                            // destinationAddress
                            isOutgoing ?
                            remoteHostPort :
                            (localHostPort.isNull() ? hostname : localHostPort),
                            // isOutgoing
                            isOutgoing,
                            // transactionId
                            transactionId,
                            // frameId
                            eventCount,
                            method,
                            responseCode,
                            responseText,
                            message);

        // Write out the node container finish
        writeBranchNodeEnd(outputFileDescriptor);
    }
}

void convertToXml(UtlString& bufferString, int outputFileDescriptor)
{
    UtlString date;
    UtlString eventCount;
    UtlString facility;
    UtlString priority;
    UtlString hostname;
    UtlString taskname;
    UtlString taskId;
    UtlString processId;
    UtlString content;

    OsSysLog::parseLogString(bufferString.data(),
                             date,
                             eventCount,
                             facility,
                             priority,
                             hostname,
                             taskname,
                             taskId,
                             processId,
                             content);

    if(facility.compareTo("OUTGOING") == 0)
    {
        hostname.append("-");
        hostname.append(processId);

        getMessageData(content,
                       TRUE,
                       date,
                       hostname,
                       eventCount,
                       outputFileDescriptor);


    }

    else if(facility.compareTo("INCOMING") == 0)
    {
        hostname.append("-");
        hostname.append(processId);

        getMessageData(content,
                       FALSE,
                       date,
                       hostname,
                       eventCount,
                       outputFileDescriptor);
    }
}


int main(int argc, char * argv[])
{
   int i;
   // Input file descriptor.  Default is stdin.
   int ifd = 0;
   // Output file descriptor.  Default is stdout.
   int ofd = 1;
   // Time limit strings.  Both tests are inclusive.  NULL means no test.
   char* before_test_string = NULL;
   char* after_test_string = NULL;

   // Parse the arguments.
   for(i = 1; i < argc; i++)
   {
      if(!strcmp(argv[i], "-h"))
      {
         // If an argument is -h, print the usage message and exit.
         fprintf(stderr, "Usage:\n\t%s [-h] [if=input] [of=output]\n",
                 argv[0]);
         return 0;
      }
      else if(!strncmp(argv[i], "if=", 3))
      {
         // if= designates the input file.
         ifd = open(&argv[i][3], O_RDONLY);
         if(ifd == -1)
         {
            fprintf(stderr, "%s: %s\n", &argv[i][3], strerror(errno));
            return 1;
         }
      }
      else if(!strncmp(argv[i], "of=", 3))
      {
         // of= designates the output file.
#ifdef _WIN32
         ofd = open(&argv[i][3], O_BINARY | O_WRONLY | O_CREAT, 0644);
#else
         /* No such thing as a "binary" file on POSIX */
         ofd = open(&argv[i][3], O_WRONLY | O_CREAT, 0644);
#endif
         if(ofd == -1)
         {
            fprintf(stderr, "%s: %s\n", &argv[i][3], strerror(errno));
            return 1;
         }
      }
      else if(!strncmp(argv[i], "--before=", 9))
      {
         // --before=xxx gives a time range.
         char* t = (char*) malloc(strlen(argv[i]) - 9 + 1 + 1 + 1);
         strcpy(t, "\"");
         strcat(t, argv[i] + 9);
         // Append '~' to the argument, so that all timestamps of which
         // 'timestamp' is a prefix compare as <= 'timestamp'.
         strcat(t, "~");
         // If this is less than the current before_test_string, use it.
         if (before_test_string == NULL || strcmp(t, before_test_string) < 0)
         {
            before_test_string = t;
         }
      }
      else if(!strncmp(argv[i], "--after=", 8))
      {
         // --after=xxx gives a time range.
         char* t = (char*) malloc(strlen(argv[i]) - 8 + 1 + 1);
         strcpy(t, "\"");
         strcat(t, argv[i] + 8);
         // If this is greater than the current after_test_string, use it.
         if (after_test_string == NULL || strcmp(t, after_test_string) > 0)
         {
            after_test_string = t;
         }
      }
      else
      {
         // All other options are errors.
         fprintf(stderr, "Unknown option: %s\n", argv[i]);
         return 1;
      }
   }

   writeMessageNodesBegin(ofd);

   char inputBuffer[BUFFER_SIZE + 1];
   UtlString bufferString;
   ssize_t lineLen;
   ssize_t nextLineStart;

   do
   {
      i = read(ifd, inputBuffer, BUFFER_SIZE);

      if(i > 0)
      {
         inputBuffer[i] = '\0';
         bufferString.append(inputBuffer);
      }

      do
      {
         lineLen =
            NameValueTokenizer::findNextLineTerminator(bufferString.data(),
                                                       bufferString.length(),
                                                       &nextLineStart);

         // If a new line was found
         if(nextLineStart > 0)
         {
            UtlString line;
            line.append(bufferString, lineLen);
            bufferString.remove(0, nextLineStart);

            // Test the string to see if the timestamp is in range.
            if ((before_test_string == NULL || line.compareTo(before_test_string) <= 0) &&
                (after_test_string == NULL || line.compareTo(after_test_string) >= 0))
            {
               // Write the line as XML.
               convertToXml(line, ofd);
            }
         }
      }
      while(nextLineStart > 0);
   } while(i && i != -1);

   // Last line without a newline
   if ((before_test_string == NULL || bufferString.compareTo(before_test_string) <= 0) &&
       (after_test_string == NULL || bufferString.compareTo(after_test_string) >= 0))
   {
      convertToXml(bufferString, ofd);
   }

   writeMessageNodesEnd(ofd);

   close(ofd);

   return 0;
}
