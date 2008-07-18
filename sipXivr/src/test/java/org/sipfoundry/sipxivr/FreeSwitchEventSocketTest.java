/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxivr;

import java.io.BufferedReader;
import java.io.StringReader;

import org.sipfoundry.sipxivr.FreeSwitchEvent;
import org.sipfoundry.sipxivr.FreeSwitchEventSocket;

import junit.framework.TestCase;

public class FreeSwitchEventSocketTest extends TestCase {

    private FreeSwitchEventSocket m_fse;

    public void setResponse(String response) {
        m_fse = new FreeSwitchEventSocket(null);
        m_fse.setIn(new BufferedReader(new StringReader(response)));
    }

    public void testGetResponse() {
        setResponse("Holy Fluff");
        FreeSwitchEvent event = m_fse.awaitEvent();
        assertEquals("Holy Fluff", event.getResponse().elementAt(0));
    }

    public void testContentLength() {
        setResponse(String.format("Content-Length: 10%n%n1234567890"));
        FreeSwitchEvent event = m_fse.awaitEvent();
        assertEquals("Content-Length: 10", event.getResponse().elementAt(0));
        assertEquals("1234567890", event.getContent().toString());
    }

    public void testExtraCrap() {
        setResponse(String.format("Content-Length: 1%n%n1NotherHeader:dog%nAndAnother%n%n"));
        FreeSwitchEvent event = m_fse.awaitEvent();
        assertEquals("Content-Length: 1", event.getResponse().elementAt(0));
        assertEquals("1", event.getContent().toString());
        event = m_fse.awaitEvent();
        assertEquals("NotherHeader:dog", event.getResponse().elementAt(0));
        assertEquals("dog", event.getHeader("NotherHeader"));
        assertEquals("AndAnother", event.getResponse().elementAt(1));
    }

    public void testVariables() {
        setResponse(String
                .format("Channel-Username: 1001%nChannel-Dialplan: XML%nChannel-Caller-ID-Name: 1001%nChannel-Caller-ID-Number: 1001%nChannel-Network-Addr: 10.0.1.241%nChannel-Destination-Number: 886%nChannel-Unique-ID: 40117b0a-186e-11dd-bbcd-7b74b6b4d31e"));
        FreeSwitchEvent event = m_fse.awaitEvent();
        m_fse.setVariables(event.getResponse());
        assertNull(m_fse.getVariable("Not There"));
        assertEquals("1001", m_fse.getVariable("Channel-Username"));
        assertEquals("10.0.1.241", m_fse.getVariable("Channel-Network-Addr"));
        assertEquals("10.0.1.241", m_fse.getVariable("channel-network-addr"));
    }
}
