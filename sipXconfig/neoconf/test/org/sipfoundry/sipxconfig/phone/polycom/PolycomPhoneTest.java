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

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.FileSystemProfileLocation;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.phone.RestartException;
import org.sipfoundry.sipxconfig.phone.polycom.PolycomPhone.FormatFilter;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.upload.UploadManager;
import org.sipfoundry.sipxconfig.upload.UploadSpecification;

public class PolycomPhoneTest extends TestCase {

    private PolycomPhone m_phone;

    private PhoneTestDriver m_tester;

    private String m_root;

    private FileSystemProfileLocation m_location;

    protected void setUp() {
        m_root = TestHelper.getTestDirectory() + "/polycom";
        m_phone = new PolycomPhone();
        m_phone.setModel(new PolycomModel());
        m_tester = PhoneTestDriver.supplyTestData(m_phone);
        m_phone.setTftpRoot(m_root);

        m_location = new FileSystemProfileLocation();
        m_location.setParentDir(m_root);

        VelocityProfileGenerator profileGenerator = TestHelper.getProfileGenerator();
        m_phone.setProfileGenerator(profileGenerator);

        IMocksControl uploadControl = EasyMock.createNiceControl();
        UploadManager uploadManager = uploadControl.createMock(UploadManager.class);
        uploadManager.getSpecification("polycomFirmware");
        uploadControl.andReturn(new UploadSpecification());
        m_phone.setUploadManager(uploadManager);
        uploadControl.replay();
    }

    public void testVersionArray() {
        assertSame(new PolycomModel().getVersions()[0], PolycomModel.VER_1_6);
    }

    public void testGenerateProfiles() throws Exception {
        ApplicationConfiguration cfg = new ApplicationConfiguration(m_phone, m_root);
        m_phone.generateProfiles(m_location);

        // content of profiles is tested in individual base classes of ConfigurationTemplate
        File file = new File(m_root, cfg.getAppFilename());
        assertTrue(file.exists());

        m_phone.removeProfiles(m_location);
        assertFalse(file.exists());
    }

    public void testRestartFailureNoLine() throws Exception {
        m_phone.getLines().clear();
        try {
            m_phone.restart();
            fail();
        } catch (RestartException re) {
            assertTrue(true);
        }
    }

    public void testRestart() throws Exception {

        String uri = m_phone.getLine(0).getUri();
        assertEquals("\"Joe User\"<sip:juser@sipfoundry.org>", uri);

        m_phone.restart();
        m_tester.sipControl.verify();
    }

    public void testLineDefaults() throws Exception {
        Setting settings = m_tester.getPrimaryLine().getSettings();
        Setting address = settings.getSetting("reg/server/1/address");
        assertEquals("sipfoundry.org", address.getValue());
    }

    public void testLineDefaultsNoUser() throws Exception {
        Line secondLine = m_phone.createLine();
        m_phone.addLine(secondLine);
        Setting settings = secondLine.getSettings();
        Setting userId = settings.getSetting("reg/auth.userId");
        assertEquals(null, userId.getValue());
    }

    public void testFormat() throws Exception {
        InputStream in = getClass().getResourceAsStream("unformatted.xml");
        ByteArrayOutputStream out = new ByteArrayOutputStream();

        FormatFilter format = new PolycomPhone.FormatFilter();
        format.copy(in, out);
        Reader expected = new InputStreamReader(getClass().getResourceAsStream("formatted.xml"));
        Reader actual = new StringReader(out.toString());
        IOUtils.contentEquals(expected, actual);
    }

    public void testDayOfWeek() {
        assertEquals(2, PolycomPhoneDefaults.dayOfWeek(0));
        assertEquals(1, PolycomPhoneDefaults.dayOfWeek(6));
    }
}
