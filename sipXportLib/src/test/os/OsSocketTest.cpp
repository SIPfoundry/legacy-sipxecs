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
#include <os/OsTask.h>
#include <string.h>

class SocketsTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(SocketsTest);
    CPPUNIT_TEST(testAcceptTimeout);
    CPPUNIT_TEST(testWriteMsg);
    CPPUNIT_TEST(testWriteAndAcceptMsg);
    CPPUNIT_TEST_SUITE_END();


    int m_port;

public:
  
    void setUp() {
        int port = PORT_NONE;
	OsServerSocket s(50);
	m_port = s.getLocalHostPort();
	s.close();
    }

    /**
     * Test accept with various timeouts
     */
    void testAcceptTimeout()
    {
    	OsTime before;
    	OsTime after;

        OsServerSocket* server = new OsServerSocket(50, m_port);

        OsDateTime::getCurTime(before);
        OsSocket* serverClient = server->accept(50);
        OsTask::delay(16) ; // Wait a bit to deal for poor time resolution
        OsDateTime::getCurTime(after);
        CPPUNIT_ASSERT_MESSAGE("socket server accept returned unexpected data",
                               serverClient == NULL);
        CPPUNIT_ASSERT_MESSAGE("socket server accept returned before timeout",
        					   (after.cvtToMsecs() - before.cvtToMsecs()) >= 50);

        OsDateTime::getCurTime(before);
        serverClient = server->accept(500);
        OsTask::delay(16) ; // Wait a bit to deal for poor time resolution
        OsDateTime::getCurTime(after);
        CPPUNIT_ASSERT_MESSAGE("socket server accept returned unexpected data",
                               serverClient == NULL);
        CPPUNIT_ASSERT_MESSAGE("socket server accept returned before timeout",
        					   (after.cvtToMsecs() - before.cvtToMsecs()) >= 500);
        server->close();
        delete server;
    }

    /**
     * Open datagram socket and send a few bytes.
     *
     * NOTE: This can/will fail if /etc/hosts defines localhost as ::1 (as
     *       opposed to 127.0.0.1).
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
     *
     * NOTE: This can/will fail if /etc/hosts defines localhost as ::1 (as
     *       opposed to 127.0.0.1).
     */
    void testWriteAndAcceptMsg()
    {
    	// Create/Verify Sockets
        OsServerSocket* server = new OsServerSocket(50, m_port);
        CPPUNIT_ASSERT_MESSAGE("server socket failure",
                               server->isOk());

        OsSocket* client = new OsConnectionSocket(m_port, "localhost");
        CPPUNIT_ASSERT_MESSAGE("client socket failure",
                               client->isOk());

        OsSocket* serverClient = server->accept(1000);
        CPPUNIT_ASSERT_MESSAGE("socket server failed to accept connection",
                               serverClient != NULL);

        // Begin read/write test
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
