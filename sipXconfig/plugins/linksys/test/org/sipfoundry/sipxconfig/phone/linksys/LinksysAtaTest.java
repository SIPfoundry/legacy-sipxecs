/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.linksys;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class LinksysAtaTest extends TestCase {

    private Linksys m_ata2102;
    private Linksys m_ata3102;
    private Linksys m_ata8000;

    protected void setUp() {
        LinksysModel model2102 = new LinksysAtaModel();
        LinksysModel model3102 = new LinksysAtaModel();
        LinksysModel model8000 = new LinksysAtaModel();
        model2102.setMaxLineCount(2);
        model3102.setMaxLineCount(2);
        model8000.setMaxLineCount(8);
        model2102.setModelId("linksys2102");
        model3102.setModelId("linksys3102");
        model8000.setModelId("linksys8000");

        m_ata2102 = new LinksysAta();
        m_ata3102 = new LinksysAta();
        m_ata8000 = new LinksysAta();
        m_ata2102.setModel(model2102);
        m_ata3102.setModel(model3102);
        m_ata8000.setModel(model8000);
    }

    public void testGetSettings() {
        PhoneTestDriver.supplyTestData(m_ata2102);
        assertNotNull(m_ata2102.getSettings());
        PhoneTestDriver.supplyTestData(m_ata3102);
        assertNotNull(m_ata3102.getSettings());
        PhoneTestDriver.supplyTestData(m_ata8000);
        assertNotNull(m_ata8000.getSettings());
    }

    public void testGenerate2102Profiles() throws Exception {
        PhoneTestDriver.supplyTestData(m_ata2102);
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(m_ata2102);
        m_ata2102.generateFiles(location);
        String expected = IOUtils.toString(this.getClass().getResourceAsStream("spa2102.cfg"));
        assertEquals(expected, location.toString());
    }

    public void testGenerate3102Profiles() throws Exception {
        PhoneTestDriver.supplyTestData(m_ata3102);
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(m_ata3102);
        m_ata3102.generateFiles(location);
        String expected = IOUtils.toString(this.getClass().getResourceAsStream("spa3102.cfg"));
        assertEquals(expected, location.toString());
    }

    public void testGenerate8000Profiles() throws Exception {
        PhoneTestDriver.supplyTestData(m_ata8000);
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(m_ata8000);
        m_ata8000.generateFiles(location);
        String expected = IOUtils.toString(this.getClass().getResourceAsStream("spa8000.cfg"));
        assertEquals(expected, location.toString());
    }

    private class LinksysAtaModel extends LinksysModel {

        LinksysAtaModel() {
            super("LinksysAta");
            setProfileTemplate("linksys/ata-config.vm");
            setModelDir("linksys");
            setSettingsFile("ata.xml");
            setLineSettingsFile("ata-line.xml");
        }
    }
}
