/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.setting.type;

import junit.framework.TestCase;

public class FileSettingTest extends TestCase {

    public void testAddZipExcludeEmpty() {
        FileSetting fs = new FileSetting();
        fs.addZipExclude("");
        assertTrue(fs.getZipExcludes().isEmpty());
        fs.addZipExclude(null);
        assertTrue(fs.getZipExcludes().isEmpty());
    }

    public void testAddZipExclude() {
        FileSetting fs = new FileSetting();
        fs.addZipExclude("aa/bb");
        fs.addZipExclude("aa/bb/cc/");

        assertEquals(2, fs.getZipExcludes().size());
        assertEquals("aa/bb/", fs.getZipExcludes().get(0));
        assertEquals("aa/bb/cc/", fs.getZipExcludes().get(1));
    }

    public void testAddZipExcludeCleanSlash() {
        FileSetting fs = new FileSetting();
        fs.addZipExclude("aa/bb///cc//dd");

        assertEquals(1, fs.getZipExcludes().size());
        assertEquals("aa/bb/cc/dd/", fs.getZipExcludes().get(0));
    }

    public void testAddZipDups() {
        FileSetting fs = new FileSetting();
        fs.addZipExclude("aa/bb///cc//dd");
        fs.addZipExclude("aa/bb/cc/dd");
        fs.addZipExclude("aa/bb/cc/dd/");

        assertEquals(1, fs.getZipExcludes().size());
        assertEquals("aa/bb/cc/dd/", fs.getZipExcludes().get(0));
    }

    public void testIsExcluded() {
        FileSetting fs = new FileSetting();
        fs.addZipExclude("aa/bb");
        fs.addZipExclude("aa/bb/cc/");
        fs.addZipExclude("aa/bb/cc/dd/");

        assertTrue(fs.isExcluded("aa/bb/c.txt"));
        assertTrue(fs.isExcluded("aa/bb/cc"));
        assertTrue(fs.isExcluded("aa/bb/dd"));
        assertFalse(fs.isExcluded("a/bb/dd"));
    }

}
