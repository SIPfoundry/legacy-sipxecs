/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.vm;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.custommonkey.xmlunit.XMLTestCase;
import org.sipfoundry.sipxconfig.TestHelper;

public class MailboxPreferencesTest extends XMLTestCase {
    
    private MailboxPreferencesReader m_reader;
    private MailboxPreferencesWriter m_writer;
    
    protected void setUp() {
        m_reader = new MailboxPreferencesReader();
        m_writer = new MailboxPreferencesWriter();
        m_writer.setVelocityEngine(TestHelper.getVelocityEngine());
    }
    
    public void testReadPreferences() {
        InputStream in = getClass().getResourceAsStream("200/mailboxprefs.xml");
        MailboxPreferences prefs = m_reader.readObject(new InputStreamReader(in));
        IOUtils.closeQuietly(in);      
        assertSame(MailboxPreferences.ActiveGreeting.OUT_OF_OFFICE, prefs.getActiveGreeting());
        assertEquals("dhubler@pingtel.com", prefs.getEmailAddress());
        assertTrue(prefs.isAttachVoicemailToEmail());
    }
    
    public void testGetValueOfById() {
        MailboxPreferences.ActiveGreeting actual = MailboxPreferences.ActiveGreeting.valueOfById("none");
        assertSame(MailboxPreferences.ActiveGreeting.NONE, actual);
    }
    
    public void testWritePreferences() throws Exception {
        StringWriter actual = new StringWriter();
        MailboxPreferences prefs = new MailboxPreferences();
        prefs.setEmailAddress("dhubler@pingtel.com");
        prefs.setActiveGreeting(MailboxPreferences.ActiveGreeting.OUT_OF_OFFICE);
        m_writer.writeObject(prefs, actual);
        InputStream expectedIn = getClass().getResourceAsStream("expected-mailboxprefs.xml");
        compareXML(actual.toString(), new InputStreamReader(expectedIn));
    }
    
    public void testReadWritePreferences() throws IOException {
        StringWriter buffer = new StringWriter();
        MailboxPreferences expected = new MailboxPreferences();
        expected.setEmailAddress("dhubler@pingtel.com");
        expected.setActiveGreeting(MailboxPreferences.ActiveGreeting.OUT_OF_OFFICE);
        m_writer.writeObject(expected, buffer);
        MailboxPreferences actual = m_reader.readObject(new StringReader(buffer.toString()));
        assertSame(expected.getActiveGreeting(), actual.getActiveGreeting());
    }

    public void testReadInitialPreferences() {
        InputStream in = getClass().getResourceAsStream("initial-mailboxprefs.xml");
        m_reader.readObject(new InputStreamReader(in));        
        IOUtils.closeQuietly(in);              
    }
}
