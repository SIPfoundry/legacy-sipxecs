//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////

#include <utl/PtTest.h>
#include <os/OsDatagramSocket.h>
#include <os/OsConnectionSocket.h>
#include <os/OsServerSocket.h>
#include <string.h>

#define RECV_BUF_LEN 1024

#ifdef _VXWORKS
sockets_main()
#else
main(int argc, char* argv[])
#endif
{
        PT_TEST_BEGIN;

        OsSocket* s = new OsDatagramSocket(8020, "localhost");
        char* msg = "hello\n";
        int len = strlen(msg);
        int bytesWritten = s->write(msg, len);

        char recvBuf[RECV_BUF_LEN];

        char eMsg[100];
        char* resp;
        OsServerSocket* server = NULL;
        OsSocket* client = NULL;
        OsSocket* serverClient = NULL;
        int bytesRead;

        printf("wrote UDP %d of %d bytes\n", bytesWritten, len);
        PT_TEST_ASSERT(len == bytesWritten,
                "OsDatagramSOcket::write failed to write correct number of bytes",
                PT_TEST_CONTINUE);


        s->close();
        delete s;

        server = new OsServerSocket(50, 8080);
        client = new OsConnectionSocket(8080, "localhost");
    serverClient = server->accept();
        PT_TEST_ASSERT(serverClient != NULL,
                "socket server failed to accept connection",
                PT_TEST_ABORT);

    bytesWritten = client->write(msg, len);
        sprintf(eMsg, "wrote TCP %d of %d bytes", bytesWritten, len);
        PT_TEST_ASSERT(len == bytesWritten, eMsg, PT_TEST_CONTINUE);

        bytesRead = serverClient->read(recvBuf, RECV_BUF_LEN - 1);
        if(bytesRead > 0)
        {
                recvBuf[bytesRead] = '\0';
                printf("received %d of %d butes: \"%s\"\n", bytesRead, len, recvBuf);
        }
        PT_TEST_ASSERT(len == bytesRead, eMsg, PT_TEST_CONTINUE);
        PT_TEST_ASSERT(strcmp(msg, recvBuf) == 0, "message is not the same as was sent",
                                PT_TEST_CONTINUE);

        resp = "bye";
        len = strlen(resp);
        bytesWritten = serverClient->write(resp, len);
                sprintf(eMsg, "wrote TCP %d of %d bytes", bytesWritten, len);
        PT_TEST_ASSERT(len == bytesWritten, eMsg, PT_TEST_CONTINUE);
        bytesRead = client->read(recvBuf, RECV_BUF_LEN - 1);
        if(bytesRead > 0)
        {
                recvBuf[bytesRead] = '\0';
                printf("received %d of %d butes: \"%s\"\n", bytesRead, len, recvBuf);
        }
        PT_TEST_ASSERT(len == bytesRead, eMsg, PT_TEST_CONTINUE);
        PT_TEST_ASSERT(strcmp(resp, recvBuf) == 0, "response is not the same as was sent",
                                PT_TEST_CONTINUE);

        serverClient->close();
        client->close();
        server->close();
        delete serverClient;
        delete client;
        delete server;


        // Note: no semicolon
        PT_TEST_END
}
