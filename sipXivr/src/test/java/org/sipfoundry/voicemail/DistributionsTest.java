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

import junit.framework.TestCase;

import org.apache.commons.io.FileUtils;

public class DistributionsTest extends TestCase {
    File m_testdir;
    
    protected void setUp() throws Exception {
        super.setUp();
        m_testdir = new File("/tmp/DistributionsTest/");
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

    public void testDistributionsReader() throws IOException {
        String xml = 
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
            "<distributions>\n" +
            "  <list>\n" +
            "    <index>1</index>\n" +
            "    <destination>42</destination>" +
            "  </list>\n" +
            "</distributions>\n";

        File tempFile;
        tempFile = File.createTempFile("DistributionsTest", ".xml", m_testdir);
        org.apache.commons.io.FileUtils.writeStringToFile(tempFile, xml);
        
        DistributionsReader dr = new DistributionsReader();
        Distributions d = dr.readObject(tempFile) ;
        assertNull(d.getList("42"));
        assertNotNull(d.getList("1"));
        assertEquals("42", d.getList("1").get(0));

        xml = 
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
            "<distributions>\n" +
            "  <list>\n" +
            "    <index>1</index>\n" +
            "    <destination>142</destination>" +
            "    <destination>124</destination>" +
            "  </list>\n" +
            "  <list>\n" +
            "    <index>2</index>\n" +
            "    <destination>242</destination>" +
            "    <destination>224</destination>" +
            "  </list>\n" +
            "</distributions>\n";
        org.apache.commons.io.FileUtils.writeStringToFile(tempFile, xml);
        d = dr.readObject(tempFile) ;
        assertNull(d.getList("42"));
        assertNotNull(d.getList("1"));
        assertEquals("142", d.getList("1").get(0));
        assertEquals("124", d.getList("1").get(1));
        assertNotNull(d.getList("2"));
        assertEquals("242", d.getList("2").get(0));
        assertEquals("224", d.getList("2").get(1));
        
        tempFile.delete();
    }
}
