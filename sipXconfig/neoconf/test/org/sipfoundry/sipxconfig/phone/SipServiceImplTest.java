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

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;


public class SipServiceImplTest extends TestCase {
    
    private SipServiceImpl m_sip;
    
    protected void setUp() {
        m_sip = new SipServiceImpl();
    }    
    
    public void testBranchId() {
        // branch id should make these differ
        assertNotSame(m_sip.getServerVia(), m_sip.getServerVia());
    }
    
    public void testDefaultFromPort() {
        m_sip.setFromServerName("donut");
        assertEquals("sip:sipuaconfig@donut", m_sip.getServerUri());
        m_sip.setFromServerPort(5000);
        assertEquals("sip:sipuaconfig@donut:5000", m_sip.getServerUri());
    }
    
    public void testGetNotifyRequestUri() {
        String actual = m_sip.getNotifyRequestUri("b.com", "5060", "a");
        assertEquals("sip:a@b.com", actual);
        
        actual = m_sip.getNotifyRequestUri("b.com", "5061", "a");
        assertEquals("sip:a@b.com:5061", actual);

        actual = m_sip.getNotifyRequestUri("b.com", "", "a");
        assertEquals("sip:a@b.com", actual);

        actual = m_sip.getNotifyRequestUri("b.com", null, "a");
        assertEquals("sip:a@b.com", actual);
    }
    
    public void REQUIRES_RUNNING_PROXY_testSend() throws Exception {
        ReadSipMessage rdr = new ReadSipMessage();
        
        String msg = "NOTIFY sipuaconfig@localhost.com SIP/2.0\r\n" 
            + "Content-Length: 0\r\n" 
            + "\r\n";

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
