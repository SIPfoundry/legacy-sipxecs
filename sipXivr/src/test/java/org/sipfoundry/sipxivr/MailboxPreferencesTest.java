/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxivr;

import java.io.File;
import java.io.IOException;

import org.sipfoundry.sipxivr.MailboxPreferences;
import org.sipfoundry.sipxivr.MailboxPreferencesReader;
import org.sipfoundry.sipxivr.MailboxPreferencesWriter;

import junit.framework.TestCase;

public class MailboxPreferencesTest extends TestCase {
    public void testMailboxPreferencesWriter() throws IOException {
        // Start with empty preferences
        MailboxPreferences prefs = new MailboxPreferences();
        String emptyXml = 
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
            "\n" +
            "<prefs>\n" +
            "  <activegreeting>none</activegreeting>\n" +
            "</prefs>\n";

        MailboxPreferencesWriter mpw = new MailboxPreferencesWriter();
        File tempFile;
        tempFile = File.createTempFile("MailboxPreferencesTest", ".xml", new File("/tmp"));
        mpw.writeObject(prefs, tempFile);
        assertTrue(tempFile.exists());
        String contents = org.apache.commons.io.FileUtils.readFileToString(tempFile);
        assertEquals(emptyXml, contents);

        // Now set a greeting
        String standardXml = 
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
            "\n" +
            "<prefs>\n" +
            "  <activegreeting>standard</activegreeting>\n" +
            "</prefs>\n";

        prefs.getActiveGreeting().setGreetingType(GreetingType.STANDARD);
        tempFile.delete();
        mpw.writeObject(prefs, tempFile);
        assertTrue(tempFile.exists());
        contents = org.apache.commons.io.FileUtils.readFileToString(tempFile);
        assertEquals(standardXml, contents);

        tempFile.delete();
    }
    
    public void testMailboxPreferencesReader() throws IOException {
        String xml = 
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
            "\n" +
            "<prefs>\n" +
            "  <activegreeting>none</activegreeting>\n" +
            "</prefs>\n";

        File tempFile;
        tempFile = File.createTempFile("MailboxPreferencesTest", ".xml", new File("/tmp"));
        org.apache.commons.io.FileUtils.writeStringToFile(tempFile, xml);
        
        MailboxPreferencesReader mpr = new MailboxPreferencesReader();
        MailboxPreferences newPrefs = mpr.readObject(tempFile) ;
        assertEquals(GreetingType.NONE, newPrefs.getActiveGreeting().getGreetingType());

        xml = 
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
            "\n" +
            "<prefs>\n" +
            "  <activegreeting>standard</activegreeting>\n" +
            "</prefs>\n";
        org.apache.commons.io.FileUtils.writeStringToFile(tempFile, xml);
        newPrefs = mpr.readObject(tempFile) ;
        assertEquals(GreetingType.STANDARD, newPrefs.getActiveGreeting().getGreetingType());

        xml = 
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" +
            "\n" +
            "<prefs>\n" +
            "  <activegreeting>outofoffice</activegreeting>\n" +
            "</prefs>\n";
        org.apache.commons.io.FileUtils.writeStringToFile(tempFile, xml);
        newPrefs = mpr.readObject(tempFile) ;
        assertEquals(GreetingType.OUT_OF_OFFICE, newPrefs.getActiveGreeting().getGreetingType());
        
        tempFile.delete();
    }
}
