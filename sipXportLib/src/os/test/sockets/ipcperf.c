//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
#ifdef _VXWORKS

// SYSTEM INCLUDES
#include <resolvLib.h>
#include <inetLib.h>
#include <ioLib.h>
#include <msgQLib.h>
#include <netinet/in.h>
#include <selectLib.h>
#include <sockLib.h>
#include <stdio.h>
#include <string.h>
#include <taskLib.h>
#include <tickLib.h>
#include <vxWorks.h>
#include <netdb.h>
#include <dhcp/dhcpc.h>

#define USE_DGRAM_SOCKET  1
#define USE_STREAM_SOCKET 2
#define USE_MSGQ          3

#define INVALID_SOCKET_DESCRIPTOR -1
static const int INVALID_SOCKET = -1;

MSG_Q_ID gMsgQForA;
MSG_Q_ID gMsgQForB;
int      gMsgsRcvdA;
int      gMsgsRcvdB;
int      gStartTicks;
int      gStopTicks;
int      gStopTest;
int      gTestType;
int      gIsTaskReadyA;
int      gIsTaskReadyB;
int              gSocketDescriptorA;
int              gSocketDescriptorB;


/*********************************************************
        dgramTaskBody
**********************************************************/
int createDatagramSocket(int serverPort,
                                                 char *serverName,
                                                 int *pSocketDescriptor)
{
        int error = 0;
        struct sockaddr_in localAddr;

        // Create the socket
        *pSocketDescriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if(*pSocketDescriptor == INVALID_SOCKET_DESCRIPTOR)
        {
                printf("socket(%s:%d) call failed with error: %d\n",
                        serverName, serverPort, errno);
                return errno;
        }


        localAddr.sin_family = AF_INET;
        localAddr.sin_port = htons(serverPort);
        localAddr.sin_addr.s_addr=htonl(INADDR_ANY); /* Allow IP in on */

        error = bind( *pSocketDescriptor, (struct sockaddr*) &localAddr,
                        sizeof(localAddr));

        if(error == INVALID_SOCKET_DESCRIPTOR)
        {
                close(*pSocketDescriptor);
                printf("bind call failed in createDatagramSocket with error: %d\n", errno);
                return errno;
        }

        return 0;
}

void dgramTaskBody(char which,
                                   int socketDescriptorSnd,
                                   int socketDescriptorRcv,
                                   int *pReadyFlag,
                                   int *pRcvCnt)
{
        int readBuf;
        int writeBuf;
        int numBytes = 0;
        int bufLen = sizeof(writeBuf);
        struct sockaddr_in from;
        int len = sizeof(from);

        *pRcvCnt = 0;
        *pReadyFlag = TRUE;

        /*
        * Messages are 4-byte integers.  The following loop relays messages
        * between the two tasks.  The relay continues until gStopTest is found
        * to be TRUE
        */
        while (!gStopTest)
        {
                numBytes = recvfrom(socketDescriptorRcv, (char*)&readBuf, bufLen, 0, (struct sockaddr*)&from, &len);
                if (numBytes < 0)
                {
                        printf("recv() in task %c returned %d\n", which, numBytes);
                        close(socketDescriptorRcv);
                        return;
                }
                (*pRcvCnt)++;

                writeBuf = 1;
                numBytes = sendto(socketDescriptorSnd, (char*)&writeBuf, bufLen, 0, (struct sockaddr*)&from, len);
                if (numBytes < 0)
                {
                        printf("send() in task %c returned %d\n", which, numBytes);
                        close(socketDescriptorSnd);
                        return;
                }

        }

        taskDelay(1);
}

/*********************************************************
        strmTaskBody
**********************************************************/
int createListenSocket(int serverPort,
                                           int connectionQueueSize,
                                           int *pSocketDescriptor)
{
        int error = 0;
        struct sockaddr_in localAddr;

   /* Create the socket*/
   *pSocketDescriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

   if ( (*pSocketDescriptor) == INVALID_SOCKET)
   {
                error = errno;
                printf("socket call failed with error: 0x%x \n", error);
                *pSocketDescriptor = INVALID_SOCKET_DESCRIPTOR;
                return error;
   }

   /* Bind to a specific server port if given*/

   localAddr.sin_family = AF_INET;
   localAddr.sin_port = htons(serverPort);
   localAddr.sin_addr.s_addr=htonl(INADDR_ANY); /* Allow IP in on*/
   /* any of this hosts addresses or NICs.*/
   error = bind( *pSocketDescriptor, (struct sockaddr*) &localAddr,
         sizeof(localAddr));

   if(error == INVALID_SOCKET)
   {
      printf("bind call failed in createListenSocket with error: %d\n", errno);
          close(*pSocketDescriptor);
      *pSocketDescriptor = INVALID_SOCKET_DESCRIPTOR;
          return error;
   }

   /* Setup the queue for connection requests*/
   error = listen(*pSocketDescriptor,  connectionQueueSize);
   if (error)
   {
      printf("listen call failed with error: %d\n", errno);
          close(*pSocketDescriptor);
     *pSocketDescriptor = INVALID_SOCKET_DESCRIPTOR;
   }
   return error;
}

int createConnectionSocket(int serverPort,
                                                   char *pServerName,
                                                   int *pSocketDescriptor)
{
        struct in_addr* serverAddr;
        struct hostent* server;
        struct sockaddr_in serverSockAddr;
        char hostentBuf[512];

        /* Connect to a remote host if given*/
        if(!pServerName || strlen(pServerName) == 0)
        {
                pServerName = "127.0.0.1";
        }


        /* Create the socket*/
        *pSocketDescriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(*pSocketDescriptor == INVALID_SOCKET)
        {
                *pSocketDescriptor = INVALID_SOCKET_DESCRIPTOR;
                printf("socket call failed with error: 0x%xt\n", errno);
                return errno;
        }

        server = resolvGetHostByName(pServerName,
                                hostentBuf, sizeof(hostentBuf));

        if(!server)
        {
                close(*pSocketDescriptor);
                printf("DNS failed to lookup host: %s\n", pServerName);
                return errno;
        }

        serverAddr = (struct in_addr*) (server->h_addr);
        serverSockAddr.sin_family = server->h_addrtype;
        serverSockAddr.sin_port = htons(serverPort);
        serverSockAddr.sin_addr.s_addr = (serverAddr->s_addr);

        if(connect(*pSocketDescriptor, (struct sockaddr*) &serverSockAddr,
                sizeof(serverSockAddr)))
        {
                close(*pSocketDescriptor);
                printf("connect call failed with error: %d\n", errno);
                return errno;
        }

        return 0;
}

void strmListenTaskBody(char which,
                                  char* serverName,
                                  int listenPort,
                                  int connectionQueueSize,
                  int *pReadyFlag,
                                  int *pRcvCnt)
{
        int retVal;
        int readBuf;
        int writeBuf;
        int numBytes;
        int bufLen;
        struct sockaddr addr;
        int addrlen = sizeof(struct sockaddr);
        int connectionSocket;
        struct linger optval;

        retVal = createListenSocket(listenPort, connectionQueueSize, &gSocketDescriptorA);
        if (retVal)
        {
                printf("createListenSocket() in task %c returned %d\n", which, retVal);
                return;
        }

   /*
    * Messages are 4-byte integers.  The following loop relays messages
    * between the two tasks.  The relay continues until gStopTest is found
    * to be TRUE
    */

        bufLen = sizeof(writeBuf);

        optval.l_onoff = 1;
        optval.l_linger = 0;

        *pRcvCnt = 0;
        *pReadyFlag = 1;
        while (!gStopTest)
        {
                connectionSocket = accept(gSocketDescriptorA, &addr, &addrlen);

                setsockopt(connectionSocket, SOL_SOCKET, SO_LINGER, &optval, sizeof (optval));

                if (gSocketDescriptorA <= 0)
                {
                        printf("accept() in task %c returned %d\n", which, numBytes);
                        close(gSocketDescriptorA);
                        return;
                }

                while(!gStopTest)
                {
                        numBytes = recv(connectionSocket, (char*)&readBuf, bufLen, 0);
                        if (numBytes < 0)
                        {
                                printf("recv() in task %c returned %d\n", which, numBytes);
                                close(connectionSocket);
                                return;
                        }
                        (*pRcvCnt)++;

                        writeBuf = 1;
                        numBytes = send(connectionSocket, (char*)&writeBuf, bufLen, 0);
                        if (numBytes < 0)
                        {
                                printf("send() in task %c returned %d\n", which, numBytes);
                                close(connectionSocket);
                                return;
                        }
                }

        }

        taskDelay(1);
        close(connectionSocket);
        close(gSocketDescriptorA);
        *pReadyFlag = 0;
}

void strmConnectTaskBody(char which,
                                  char* serverName,
                                  int connectionPort,
                  int *pReadyFlag,
                                  int *pRcvCnt)
{
        int retVal;
        int readBuf;
        int writeBuf;
        int numBytes;
        int bufLen;

        retVal = createConnectionSocket(connectionPort, serverName, &gSocketDescriptorB);
        if (retVal)
        {
                printf("createConnectionSocket in task %c returned %d\n", which, retVal);
                return;
        }

   *pRcvCnt = 0;
   *pReadyFlag = 1;

   /*
    * Messages are 4-byte integers.  The following loop relays messages
    * between the two tasks.  The relay continues until gStopTest is found
    * to be TRUE
    */

        bufLen = sizeof(writeBuf);

        while (!gStopTest)
        {
                writeBuf = 1;
                numBytes = send(gSocketDescriptorB, (char*)&writeBuf, bufLen, 0);
                if (numBytes < 0)
                {
                        printf("send() in task %c returned %d\n", which, numBytes);
                        close(gSocketDescriptorB);
                        return;
                }

                (*pRcvCnt)++;
                numBytes = recv(gSocketDescriptorB, (char*)&readBuf, bufLen, 0);
                if (numBytes < 0)
                {
                        printf("recv() in task %c returned %d\n", which, numBytes);
                        close(gSocketDescriptorB);
                        return;
                }

        }

        numBytes = recv(gSocketDescriptorB, (char*)&readBuf, bufLen, 0);
        taskDelay(1);
        close(gSocketDescriptorB);
        *pReadyFlag = 0;
}


void msgQTaskBody(char which, MSG_Q_ID *pRcvQ, MSG_Q_ID *pSndQ,
                  int *pReadyFlag, int *pRcvCnt)
{
   int readBuf;
   int retVal;
   int writeBuf;

   *pRcvCnt = 0;
   *pRcvQ = msgQCreate(2, 4, MSG_Q_PRIORITY);
   if (*pRcvQ == NULL)
   {
      printf("Couldn't create receive msgQ for task %c\n", which);
      return;
   }
   *pReadyFlag = TRUE;

   /*
    * Messages are 4-byte integers.  The following loop relays messages
    * between the two tasks.  The relay continues until gStopTest is found
    * to be TRUE
    */
   while (!gStopTest)
   {
      retVal = msgQReceive(*pRcvQ, (char *) &readBuf, 4, -1);
      if (retVal <= 0)
      {
         printf("msgQReceive() in task %c returned %d\n", which, retVal);
         return;
      }

      (*pRcvCnt)++;
      writeBuf = 1;
      retVal = msgQSend(*pSndQ, (char *) &writeBuf, sizeof(writeBuf), -1, MSG_PRI_NORMAL);
      if (retVal < 0)
      {
         printf("msgQSend() in task %c returned %d\n", which, retVal);
         return;
      }
   }

   taskDelay(1);
   msgQDelete(*pRcvQ);
}



void ipcPerfStart(int testType)
{
        int retVal;
        int writeBuf = 1;
        int numBytes;

        struct sockaddr_in toSockAddress;
        char* ipAddress = "127.0.0.1";
        int port = 8080;

        if ((testType != USE_DGRAM_SOCKET) &&
           (testType != USE_STREAM_SOCKET) &&
           (testType != USE_MSGQ))
        {
          printf("Usage: ipcPerfStart(testType)\n");
          printf("       testType = 1 ==> use datagram sockets\n");
          printf("       testType = 2 ==> use stream sockets\n");
          printf("       testType = 3 ==> use message queues\n");
          return;
        }

        gTestType     = testType;
        gIsTaskReadyA = FALSE;
        gIsTaskReadyB = FALSE;
        gStopTest     = FALSE;

        if (testType == USE_DGRAM_SOCKET)
        {
                 retVal = createDatagramSocket(8080, 0, &gSocketDescriptorA);
                 if (retVal || gSocketDescriptorA < 0)
                 {
                         printf("Couldn't create datagram socket, returned %d\n", retVal);
                         return;
                 }

                 retVal = createDatagramSocket(8020, 0, &gSocketDescriptorB);
                 if (retVal || gSocketDescriptorB < 0)
                 {
                         printf("Couldn't create datagram socket, returned %d\n", retVal);
                         return;
                 }

                retVal = sp(dgramTaskBody,
                                  'A', gSocketDescriptorA, gSocketDescriptorB, &gIsTaskReadyA, &gMsgsRcvdA);
                if (retVal < 0)
                {
                        printf("Couldn't spawn task A, sp() returned %d\n", retVal);
                        return;
                }

                retVal = sp(dgramTaskBody,
                                  'B', gSocketDescriptorB, gSocketDescriptorA, &gIsTaskReadyB, &gMsgsRcvdB);
                if (retVal < 0)
                {
                        printf("Couldn't spawn task B, sp() returned %d\n", retVal);
                        return;
                }
        }
        else if (testType == USE_STREAM_SOCKET)
        {
                retVal = sp(strmListenTaskBody,
                                  'A', 0, 8888, 64, &gIsTaskReadyA, &gMsgsRcvdA);
                if (retVal < 0)
                {
                        printf("Couldn't spawn task A, sp() returned %d\n", retVal);
                        return;
                }

                retVal = sp(strmConnectTaskBody,
                                  'B', 0, 8888, &gIsTaskReadyB, &gMsgsRcvdB);
                if (retVal < 0)
                {
                        printf("Couldn't spawn task B, sp() returned %d\n", retVal);
                        return;
                }
        }
        else if (testType == USE_MSGQ)
        {
          retVal = sp(msgQTaskBody,
                                  'A', &gMsgQForA, &gMsgQForB, &gIsTaskReadyA, &gMsgsRcvdA);
          if (retVal < 0)
          {
                 printf("Couldn't spawn task A, sp() returned %d\n", retVal);
                 return;
          }

          retVal = sp(msgQTaskBody,
                                  'B', &gMsgQForB, &gMsgQForA, &gIsTaskReadyB, &gMsgsRcvdB);
          if (retVal < 0)
          {
                 printf("Couldn't spawn task B, sp() returned %d\n", retVal);
                 return;
          }
        }

        while (!gIsTaskReadyA || !gIsTaskReadyB)
          taskDelay(1);

        gStartTicks = tickGet();
        if (testType == USE_DGRAM_SOCKET)
        {
                toSockAddress.sin_family = AF_INET;
                toSockAddress.sin_port = htons(port);
                toSockAddress.sin_addr.s_addr = inet_addr(ipAddress);

                writeBuf = 1;
                numBytes = sendto(gSocketDescriptorA,
                                                        (char*)&writeBuf,
                                                        sizeof(writeBuf),
                                                        0,
                                                        (struct sockaddr*) &toSockAddress,
                                                        sizeof(struct sockaddr_in));
                if (numBytes < 0)
                {
                        printf("datagram socket sendto() in ipcPerfStart() returned %d\n", numBytes);
                        close(gSocketDescriptorA);
                        return;
                }
        }
        else if (testType == USE_STREAM_SOCKET)
        {
        }
        else if (testType == USE_MSGQ)
        {
          /* start the relay off by sending a message to task A */
          retVal = msgQSend(gMsgQForA, (char *) &writeBuf, sizeof(writeBuf), -1, MSG_PRI_NORMAL);
          if (retVal < 0)
          {
                 printf("msgQSend() in ipcPerfStart() returned %d\n", retVal);
                 return;
          }
        }
}

void ipcPerfStop()
{
        int ticks;
        int numMsgs;
        float average = 0;

   gStopTicks = tickGet();
   if (gTestType == USE_DGRAM_SOCKET)
   {
                gStopTest = TRUE;
                taskDelay(2);
                close(gSocketDescriptorA);
                close(gSocketDescriptorB);
   }
   else if (gTestType == USE_STREAM_SOCKET)
   {
                gStopTest = TRUE;
                taskDelay(2);
                close(gSocketDescriptorA);
                close(gSocketDescriptorB);
   }
   else if (gTestType == USE_MSGQ)
   {
      gStopTest = TRUE;
   }

   taskDelay(2);
   ticks = gStopTicks - gStartTicks;
   numMsgs = gMsgsRcvdA + gMsgsRcvdB;
   if (ticks)
                average = ((float) numMsgs) / ((float) ticks);

   printf("TestType = %d\n", gTestType);
   printf("Test duration = %d ticks\n", ticks);
   printf("Num msgs sent = %d\n", numMsgs);
   printf("Average = %f msgs per tick\n", average);
   gMsgsRcvdA = 0;
   gMsgsRcvdB = 0;
   gStopTicks = 0;
   gStartTicks = tickGet();
}

#endif // _VXWORKS
