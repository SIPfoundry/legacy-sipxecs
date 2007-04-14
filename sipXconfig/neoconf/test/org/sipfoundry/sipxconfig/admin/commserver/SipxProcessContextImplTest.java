/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Command;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Process;

public class SipxProcessContextImplTest extends TestCase {
    private SipxProcessContextImpl m_processContextImpl;
    private Location m_location;
    private List m_requestUrls;

    protected void setUp() throws Exception {
        m_processContextImpl = new SipxProcessContextImpl() {
            protected InputStream getStatusStream(Location location) {
                return SipxProcessContextImplTest.class.getResourceAsStream("status.test.xml");
            }

            protected InputStream getTopologyAsStream() {
                return SipxProcessContextImplTest.class.getResourceAsStream("topology.test.xml");
            }

            protected InputStream invokeHttpGetRequest(String urlString) {
                m_requestUrls.add(urlString);
                return null;
            }
        };

        m_location = m_processContextImpl.getLocations()[0];
        m_requestUrls = new ArrayList();
    }

    public void testConstructRestartUrl() {
        String url = m_processContextImpl.constructCommandUrl(m_location,
                SipxProcessContext.Process.REGISTRAR, SipxProcessContext.Command.RESTART);

        assertEquals(
                "https://localhost:8091/cgi-bin/processmonitor/process.cgi?command=restart&process=SIPRegistrar",
                url);
    }

    public void testConstructStatusUrls() throws Exception {
        String url = m_processContextImpl.constructStatusUrl(m_location);
        assertEquals("https://localhost:8091/cgi-bin/processmonitor/process.cgi?command=status",
                url);
    }

    public void testConstructCommandUrls() throws Exception {
        String url = m_processContextImpl.constructCommandUrl(m_location, Process.AUTH_PROXY,
                Command.START);
        assertEquals(
                "https://localhost:8091/cgi-bin/processmonitor/process.cgi?command=start&process=SIPAuthProxy",
                url);
    }

    // Test getting status. Work with persisted XML output, rather than making a live call
    // to the server, to avoid requiring a server to be running. We override the method
    // getStatusStream of SipxProcessContextImpl for this purpose.
    public void testGetStatus() throws Exception {
        ServiceStatus[] status = m_processContextImpl.getStatus(m_location);
        assertEquals(8, status.length);
        assertEquals("ConfigServer", status[0].getServiceName());
    }

    public void testManageService() {

        m_processContextImpl.manageService(m_location, Process.REGISTRAR, Command.STOP);

        assertEquals(1, m_requestUrls.size());
        assertEquals(
                "https://localhost:8091/cgi-bin/processmonitor/process.cgi?command=stop&process=SIPRegistrar",
                m_requestUrls.get(0));
    }

    public void testManageServices() {
        Process[] processes = {
            Process.MEDIA_SERVER, Process.PRESENCE_SERVER
        };

        m_processContextImpl.manageServices(Arrays.asList(processes), Command.RESTART);

        assertEquals(4, m_requestUrls.size());
        assertEquals(
                "https://localhost:8091/cgi-bin/processmonitor/process.cgi?command=restart&process=MediaServer",
                m_requestUrls.get(0));
        assertEquals(
                "https://localhost:8091/cgi-bin/processmonitor/process.cgi?command=restart&process=PresenceServer",
                m_requestUrls.get(1));
        assertEquals(
                "https://192.168.0.27:8091/cgi-bin/processmonitor/process.cgi?command=restart&process=MediaServer",
                m_requestUrls.get(2));
        assertEquals(
                "https://192.168.0.27:8091/cgi-bin/processmonitor/process.cgi?command=restart&process=PresenceServer",
                m_requestUrls.get(3));
    }

    public void testManageServicesLocation() {
        Process[] processes = {
            Process.MEDIA_SERVER, Process.PRESENCE_SERVER
        };

        m_processContextImpl.manageServices(m_location, Arrays.asList(processes), Command.START);

        assertEquals(2, m_requestUrls.size());
        assertEquals(
                "https://localhost:8091/cgi-bin/processmonitor/process.cgi?command=start&process=MediaServer",
                m_requestUrls.get(0));
        assertEquals(
                "https://localhost:8091/cgi-bin/processmonitor/process.cgi?command=start&process=PresenceServer",
                m_requestUrls.get(1));

    }
}
