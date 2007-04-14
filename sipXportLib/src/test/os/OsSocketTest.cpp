//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//
// $$
////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>

#include <os/OsFS.h>
#include <sipxunit/TestUtilities.h>
#include <os/OsTestUtilities.h>
#include <os/OsDatagramSocket.h>
#include <os/OsConnectionSocket.h>
#include <os/OsServerSocket.h>
#include <string.h>

class SocketsTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(SocketsTest);
    CPPUNIT_TEST(testWriteMsg);
    CPPUNIT_TEST(testWriteAndAcceptMsg);
    CPPUNIT_TEST_SUITE_END();


public:

    /**
     * Open datagram socket and send a few bytes.
     */
    void testWriteMsg()
    {
        OsSocket* s = new OsDatagramSocket(8020, "localhost");
        const char* msg = "hello\n";
        int len = strlen(msg);
        int bytesWritten = s->write(msg, len);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("write correct number of bytes", 
            bytesWritten, len);
        s->close();
        delete s;
    }

    /**
     * Start a client and server and send 2 messages over TCP thru them
     */
    void testWriteAndAcceptMsg()
    {
        OsServerSocket* server = new OsServerSocket(50, 8021);
        OsSocket* client = new OsConnectionSocket(8021, "localhost");
        OsSocket* serverClient = server->accept();

        CPPUNIT_ASSERT_MESSAGE("socket server failed to accept connection", 
                               serverClient != NULL);

        const char* msg = "hello\n";
        int len = strlen(msg) + 1; // +1 for NULL
        int bytesWritten = client->write(msg, len);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("write correct number of bytes", 
                bytesWritten, len);

        char recvBuf[1024];
        int bytesRead = serverClient->read(recvBuf, sizeof(recvBuf) - 1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("read correct number of bytes", 
                len, bytesRead);
        ASSERT_STR_EQUAL_MESSAGE("message same as was sent", msg, recvBuf);

        const char *resp = "bye";
        len = strlen(resp) + 1; // +1 for NULL
        bytesWritten = serverClient->write(resp, len);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("write correct number of bytes on 2nd msg", 
            len, bytesWritten);

        bytesRead = client->read(recvBuf, sizeof(recvBuf) - 1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("read correct number of bytes on 2nd msg", 
            len, bytesRead);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("write correct number of bytes on 2nd msg", 
            len, bytesWritten);

        ASSERT_STR_EQUAL_MESSAGE("2nd message same as was sent", 
                resp, recvBuf);

        serverClient->close();
        client->close();
        server->close();

        delete client;
        delete server;
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SocketsTest);

