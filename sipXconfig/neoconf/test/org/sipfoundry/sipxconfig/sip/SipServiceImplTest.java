/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.sip;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;

public class SipServiceImplTest extends TestCase {

    public void testSip() {
        // TODO: real tests...
    }

    public void REQUIRES_RUNNING_PROXY_testSend() throws Exception {
        ReadSipMessage rdr = new ReadSipMessage();

        String msg = "NOTIFY sipuaconfig@localhost.com SIP/2.0\r\n" + "Content-Length: 0\r\n" + "\r\n";

        rdr.shutdown();
        assertEquals(msg, rdr.msg);
    }

    static class ReadSipMessage extends Thread {

        String msg;

        ServerSocket m_server;

        ReadSipMessage() throws IOException {
            m_server = new ServerSocket(5060);
            start();
        }

        void shutdown() throws Exception {
            m_server.close();
            join();
        }

        public void run() {
            try {
                Socket socket = m_server.accept();
                msg = IOUtils.toString(socket.getInputStream());
            } catch (IOException e) {
                throw new RuntimeException("Failure to read socket", e);
            }
        }
    }
}
