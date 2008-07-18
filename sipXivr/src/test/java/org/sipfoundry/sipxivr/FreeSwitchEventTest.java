/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxivr;

import java.util.Vector;

import org.sipfoundry.sipxivr.FreeSwitchEvent;

import junit.framework.TestCase;

public class FreeSwitchEventTest extends TestCase {

    private Vector<String> m_response1 = new Vector<String>();
    private String m_contentType1 = "Content-Type: command/reply";
    private String m_contentLength1 = "Content-Length: 26";
    private String m_content1 = "abcdefghijklmnopqrstuvwxyz";

    private Vector<String> m_response2 = new Vector<String>();
    private String m_contentType2 = "Content-Type: text/event-plain";
    private String m_contentLength2 = "Content-Length: 69";
    private String m_content2 = "Caller-Username: 207\nCaller-Dialplan: XML\nCaller-Caller-ID-Name: 207\n";

    public void setUp() {
        m_response1.add(m_contentType1);
        m_response1.add(m_contentLength1);

        m_response2.add(m_contentType2);
        m_response2.add(m_contentLength2);
    }

    public void testGetResponse() {
        FreeSwitchEvent event = new FreeSwitchEvent(m_response1, m_content1);
        assertEquals(m_response1, event.getResponse());
    }

    public void testGetContent() {
        FreeSwitchEvent event = new FreeSwitchEvent(m_response1, m_content1);
        assertEquals(m_content1, event.getContent());
    }

    public void testGetHeader() {
        FreeSwitchEvent event = new FreeSwitchEvent(m_response1, m_content1);
        assertEquals("command/reply", event.getHeader("content-type"));
    }

    public void testGetHeaderNoNull() {
        FreeSwitchEvent event = new FreeSwitchEvent(m_response1, m_content1);
        assertNotNull(event.getHeader("dog", "not null"));
    }

    public void testGetContentType() {
        FreeSwitchEvent event = new FreeSwitchEvent(m_response1, m_content1);
        assertEquals("command/reply", event.getContentType());
    }

    public void testParseEventContent() {
        FreeSwitchEvent event = new FreeSwitchEvent(m_response2, m_content2);
        assertEquals("207", event.getEventValue("Caller-Username"));
    }

    public void testGetEventValueNoNull() {
        FreeSwitchEvent event = new FreeSwitchEvent(m_response2, m_content2);
        assertNotNull(event.getEventValue("dog", "not null"));
    }

}
