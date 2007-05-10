/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone.polycom;

import java.io.File;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.List;

import org.custommonkey.xmlunit.Diff;
import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.device.ProfileGenerator;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class ApplicationConfigurationTest extends XMLTestCase {

    private PolycomPhone phone;
    private ProfileGenerator m_pg;
    private MemoryProfileLocation m_location;

    protected void setUp() throws Exception {
        XMLUnit.setIgnoreWhitespace(true);
        phone = new PolycomPhone();
        phone.getModel().setMaxLineCount(6);
        PhoneTestDriver.supplyTestData(phone);

        m_location = new MemoryProfileLocation();

        VelocityProfileGenerator pg = new VelocityProfileGenerator();
        pg.setVelocityEngine(TestHelper.getVelocityEngine());
        m_pg = pg;
    }

    public void testGenerateProfile() throws Exception {
        String root = TestHelper.getTestDirectory() + "/testPolycom";

        ApplicationConfiguration app = new ApplicationConfiguration(phone, root);

        m_pg.generate(m_location, app, "profile");

        InputStream expectedPhoneStream = getClass().getResourceAsStream(
                "expected-macaddress.cfg");
        Reader expectedXml = new InputStreamReader(expectedPhoneStream);
        Reader generatedXml = new StringReader(m_location.toString());

        // helpful debug
        // System.out.println(out.toCharArray());

        Diff phoneDiff = new Diff(expectedXml, generatedXml);
        assertXMLEqual(phoneDiff, true);
        expectedPhoneStream.close();
    }

    public void testGetUniqueDirectory() {
        String root = TestHelper.getTestDirectory() + "/testGetUniqueDirectory-"
                + System.currentTimeMillis();
        new File(root).mkdirs();
        String base = "000000000000";

        // test 1 digit
        String expected = base + ".0001";
        File expectedDir = new File(root, expected);
        List stale = new ArrayList();
        String actualDir = ApplicationConfiguration.getNextDirectorySequence(root, base, stale);
        assertEquals(expected, actualDir);
        assertTrue(expectedDir.mkdirs());
        assertEquals(0, stale.size());

        for (int i = 2; i < 15; i++) {
            stale.clear();
            String nextDir = ApplicationConfiguration.getNextDirectorySequence(root, base, stale);
            File d = new File(root, nextDir);
            d.mkdir();
        }

        // test 2 digits
        expected = base + ".0015";
        stale.clear();
        actualDir = ApplicationConfiguration.getNextDirectorySequence(root, base, stale);
        assertEquals(expected, actualDir);
        assertEquals(14, stale.size()); // 14 because one based and 15th not stale yet
    }

    public void testDeleteStale() throws Exception {
        String root = TestHelper.getTestDirectory() + "/testDeleteStale";
        File rootDir = new File(root);

        ApplicationConfiguration app0001 = new ApplicationConfiguration(phone, root);
        assertEquals("0004f200e06b.0001", app0001.getDirectory());
        File f = new File(rootDir, app0001.getDirectory());
        f.mkdirs();

        ApplicationConfiguration app0002 = new ApplicationConfiguration(phone, root);
        assertEquals("0004f200e06b.0002", app0002.getDirectory());

        assertEquals(1, rootDir.list().length);
        app0002.deleteStaleDirectories();
        assertEquals(0, rootDir.list().length);
    }
}
