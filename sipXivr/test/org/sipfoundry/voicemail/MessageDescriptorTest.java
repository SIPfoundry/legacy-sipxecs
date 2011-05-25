/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.voicemail;

import java.io.File;
import java.io.IOException;
import java.util.TimeZone;

import junit.framework.TestCase;

import org.apache.commons.io.FileUtils;
import org.sipfoundry.voicemail.MessageDescriptor.Priority;

public class MessageDescriptorTest extends TestCase {
    File m_testdir;
    
    protected void setUp() throws Exception {
        super.setUp();
        m_testdir = new File("/tmp/MessageDescriptorTest/");
        if (m_testdir.isDirectory()) {
            FileUtils.forceDelete(m_testdir);
        }
        m_testdir.mkdir();
    }

    protected void tearDown() throws Exception {
        super.tearDown();
        if (m_testdir.isDirectory()) {
            FileUtils.forceDelete(m_testdir);
        }
    }
    
    public void testMessageDescriptorWriter() throws IOException {
        // Start with empty MessageDescriptor
        MessageDescriptor md = new MessageDescriptor();
        String emptyXml = 
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
            "\n" +
            "<messagedescriptor>\n" +
            "  <id/>\n" +
            "  <from/>\n" +
            "  <durationsecs/>\n" +
            "  <timestamp/>\n" +
            "  <subject/>\n" +
            "  <priority>normal</priority>\n" +
            "</messagedescriptor>\n";

        MessageDescriptorWriter mdw = new MessageDescriptorWriter();
        File tempFile;
        tempFile = File.createTempFile("MessageDescriptorTest", ".xml", m_testdir);
        mdw.writeObject(md, tempFile);
        assertTrue(tempFile.exists());
        String contents = org.apache.commons.io.FileUtils.readFileToString(tempFile);
        assertEquals(emptyXml, contents);

        // Now set a priority and an Id
        String normalXml = 
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
            "\n" +
            "<messagedescriptor>\n" +
            "  <id>woof@dog</id>\n" +
            "  <from/>\n" +
            "  <durationsecs/>\n" +
            "  <timestamp/>\n" +
            "  <subject/>\n" +
            "  <priority>normal</priority>\n" +
            "</messagedescriptor>\n";

        md.setId("woof@dog");
        md.setPriority(Priority.NORMAL);
        tempFile.delete();
        mdw.writeObject(md, tempFile);
        assertTrue(tempFile.exists());
        contents = org.apache.commons.io.FileUtils.readFileToString(tempFile);
        assertEquals(normalXml, contents);

        // Now add a from address (watch the escaping!)
        String fromXml = 
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
            "\n" +
            "<messagedescriptor>\n" +
            "  <id>woof@dog</id>\n" +
            "  <from>\"Andy Spitzer\" &lt;sip:woof@pingtel.com;dog=yes&gt;;fluffy</from>\n" +
            "  <durationsecs/>\n" +
            "  <timestamp/>\n" +
            "  <subject/>\n" +
            "  <priority>normal</priority>\n" +
            "</messagedescriptor>\n";

        md.setFromUri("\"Andy Spitzer\" <sip:woof@pingtel.com;dog=yes>;fluffy");
        tempFile.delete();
        mdw.writeObject(md, tempFile);
        assertTrue(tempFile.exists());
        contents = org.apache.commons.io.FileUtils.readFileToString(tempFile);
        assertEquals(fromXml, contents);

        // Now add a timestamp
        String timeXml = 
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
            "\n" +
            "<messagedescriptor>\n" +
            "  <id>woof@dog</id>\n" +
            "  <from>\"Andy Spitzer\" &lt;sip:woof@pingtel.com;dog=yes&gt;;fluffy</from>\n" +
            "  <durationsecs/>\n" +
            "  <timestamp>Tue, 10-Jun-1997 12:00:00 AM EDT</timestamp>\n" +
            "  <subject/>\n" +
            "  <priority>normal</priority>\n" +
            "</messagedescriptor>\n";

        TimeZone.setDefault(TimeZone.getTimeZone("America/New_York"));
        long timestamp = 865915200L*1000; // Happy Birthday, Alex!
        md.setTimestamp(timestamp);
        tempFile.delete();
        mdw.writeObject(md, tempFile);
        assertTrue(tempFile.exists());
        contents = org.apache.commons.io.FileUtils.readFileToString(tempFile);
        assertEquals(timeXml, contents);

        tempFile.delete();
    }

    public void testMessageDescriptorReader() throws IOException {
        String xml = 
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
            "\n" +
            "<messagedescriptor>\n" +
            "  <id/>\n" +
            "  <from/>\n" +
            "  <durationsecs/>\n" +
            "  <timestamp/>\n" +
            "  <subject/>\n" +
            "  <priority>normal</priority>\n" +
            "</messagedescriptor>\n";

        File tempFile;
        tempFile = File.createTempFile("MessageDescriptorTest", ".xml", m_testdir);
        org.apache.commons.io.FileUtils.writeStringToFile(tempFile, xml);
        
        MessageDescriptorReader mdr = new MessageDescriptorReader();
        MessageDescriptor md = mdr.readObject(tempFile) ;
        assertNull(md.getId());
        assertNull(md.getDurationSecs());
        assertEquals(Priority.NORMAL, md.getPriority());

        xml = 
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
            "\n" +
            "<messagedescriptor>\n" +
            "  <id>woof@dog</id>\n" +
            "  <from/>\n" +
            "  <durationsecs/>\n" +
            "  <timestamp/>\n" +
            "  <subject/>\n" +
            "  <priority>normal</priority>\n" +
            "</messagedescriptor>\n";
        org.apache.commons.io.FileUtils.writeStringToFile(tempFile, xml);
        md = mdr.readObject(tempFile) ;
        assertEquals("woof@dog",md.getId());
        assertNull(md.getDurationSecs());
        assertEquals(Priority.NORMAL, md.getPriority());
        
        tempFile.delete();
    }
}
