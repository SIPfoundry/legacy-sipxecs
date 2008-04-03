/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import java.io.IOException;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Formatter;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;

public class SipServiceImplTest extends TestCase {

    private SipServiceImpl m_sip;
    private DomainManager m_domainManager;

    protected void setUp() {
        m_sip = new SipServiceImpl();
        m_sip.setProxyPort(5061);
        m_sip.setProxyHost("proxy.example.org");
        m_sip.setFromServerName("config.example.org");

        Domain domain = new Domain();
        domain.setName("example.org");
        m_domainManager = EasyMock.createNiceMock(DomainManager.class);
        m_domainManager.getDomain();
        EasyMock.expectLastCall().andReturn(domain).anyTimes();
        EasyMock.replay(m_domainManager);
    }

    protected void tearDown() throws Exception {
        EasyMock.verify(m_domainManager);
    }

    public void testBranchId() {
        Formatter f1 = new Formatter();
        m_sip.formatServerVia(f1);

        Formatter f2 = new Formatter();
        m_sip.formatServerVia(f2);

        // branch id should make these differ
        assertNotSame(f1.toString(), f2.toString());
    }

    public void testGetVia() {
        String expectedVia = "Via: SIP/2.0/UDP config.example.org:5061;branch=[0-9a-f]{1,16}\r\n";

        Formatter f1 = new Formatter();
        m_sip.formatServerVia(f1);

        String actualVia = f1.toString();

        assertTrue("Via must match the pattern", actualVia.matches(expectedVia));
    }

    public void testDefaultFromPort() {
        m_sip.setFromServerName("donut");
        assertEquals("sip:sipuaconfig@donut:5061", m_sip.getServerUri());
        m_sip.setProxyPort(5000);
        assertEquals("sip:sipuaconfig@donut:5000", m_sip.getServerUri());
    }

    public void testCreateHeader() throws Exception {
        String expectedHeader = "NOTIFY sip:user@example.org:5061 SIP/2.0\r\n"
                + "Via: SIP/2.0/UDP config.example.org:5061;branch=[0-9a-f]{1,16}\r\n"
                + "From: sip:sipuaconfig@config.example.org:5061\r\n" + "To: sip:user@example.com\r\n"
                + "Date: [A-z]{3}, \\d{1,2} [A-z]{3} \\d{4} \\d{2}:\\d{2}:\\d{2} GMT\r\n"
                + "Call-ID: 90d3f2-[0-9a-f]{1,16}\r\n" + "CSeq: 1 NOTIFY\r\n"
                + "Contact: sip:config.example.org:5061\r\n" + "Event: check-sync\r\n" + "Content-Length: 20\r\n\r\n";

        StringBuilder f = new StringBuilder();
        m_sip.formatSipHeaders(f, "sip:user@example.com", "example.org", 5061, "Event: check-sync\r\n", 20);
        String sipHeaders = f.toString();

        System.err.println(expectedHeader);
        System.err.println(sipHeaders);

        assertTrue(sipHeaders.matches(expectedHeader));

    }

    public void testGetNotifyRequestUri() {
        String actual = m_sip.getNotifyRequestUri("a", "b.com", 5060);
        assertEquals("sip:a@b.com", actual);

        actual = m_sip.getNotifyRequestUri("a", "b.com", 5061);
        assertEquals("sip:a@b.com:5061", actual);
    }

    public void REQUIRES_RUNNING_PROXY_testSend() throws Exception {
        ReadSipMessage rdr = new ReadSipMessage();

        String msg = "NOTIFY sipuaconfig@localhost.com SIP/2.0\r\n" + "Content-Length: 0\r\n" + "\r\n";

        m_sip.send(msg.getBytes());

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
