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

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.classextension.EasyMock.replay;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Set;

import junit.framework.TestCase;
import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.cert.CertificateManager;
import org.sipfoundry.sipxconfig.device.FileSystemProfileLocation;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.phone.polycom.PolycomPhone.FormatFilter;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class PolycomPhone40Test extends TestCase {

    private PolycomPhone m_phone;

    private PhoneTestDriver m_tester;

    private FileSystemProfileLocation m_location;

    private final String m_root = TestHelper.getTestDirectory() + "/testPolycom";

    @Override
    protected void setUp() {
        m_phone = new PolycomPhone();
        PolycomModel model = new PolycomModel();
        m_phone.setModel(model);
        m_phone.setDeviceVersion(PolycomModel.VER_4_0_X);
        model.setModelId("polycomVVX500");
        Set<String> features = new HashSet<String>();
        features.add("disableCallList");
        features.add("intercom");
        features.add("voiceQualityMonitoring");
        features.add("VVX_500_CodecPref");
        features.add("desktopIntegration");
        features.add("exchangeIntegration");
        features.add("video");
        model.setSupportedFeatures(features);
        m_tester = PhoneTestDriver.supplyTestData(m_phone,true,false,false,true);
        m_location = new FileSystemProfileLocation();
        m_location.setParentDir(m_root);

        VelocityProfileGenerator profileGenerator = TestHelper.getProfileGenerator(TestHelper.getEtcDir());
        m_phone.setProfileGenerator(profileGenerator);
    }

    // Firmware version 1.6 is no longer supported. For firmware version 2.0 and beyound,
    // as the formats are compatible, thus version selection is no longer needed at this point.
    public void testVersionArray() {
        // assertSame(new PolycomModel().getVersions()[0], PolycomModel.VER_1_6);
    }

    public void testGenerateProfiles() throws Exception {
        AddressManager addressManager = createMock(AddressManager.class);
        addressManager.getAddresses(new AddressType("provisionService", "http://%s:%d/"));
        expectLastCall().andReturn(new ArrayList<Address>()).anyTimes();
        
        m_phone.setAddressManager(addressManager);
        ApplicationConfiguration cfg = new ApplicationConfiguration(m_phone);
        
        CertificateManager cm = createMock(CertificateManager.class);
        cm.getSelfSigningAuthorityText();
        expectLastCall().andReturn("empty");
        replay(addressManager, cm);
        m_phone.setCertificateManager(cm);
        m_phone.generateProfiles(m_location);

        File phonebook = new File(m_root, cfg.getDirectoryFilename());
        assertTrue(phonebook.exists());

        // content of profiles is tested in individual base classes of ConfigurationTemplate
        File appFile = new File(m_root, cfg.getAppFilename());
        assertTrue(appFile.exists());

        assertTrue(new File(m_root, m_phone.getSipInteropFilename()).exists());
        assertTrue(new File(m_root, m_phone.getAppsFilename()).exists());
        assertTrue(new File(m_root, m_phone.getSipBasicFilename()).exists());
        assertTrue(new File(m_root, m_phone.getFeaturesFilename()).exists());
        assertTrue(new File(m_root, m_phone.getRegAdvancedFilename()).exists());
        assertTrue(new File(m_root, m_phone.getRegionFilename()).exists());
        assertTrue(new File(m_root, m_phone.getSiteFilename()).exists());
        assertTrue(new File(m_root, m_phone.getVideoFilename()).exists());
        
        m_phone.removeProfiles(m_location);
        assertTrue(phonebook.exists());
        assertFalse(appFile.exists());
        assertFalse(new File(m_root, m_phone.getSipInteropFilename()).exists());
        assertFalse(new File(m_root, m_phone.getAppsFilename()).exists());
        assertFalse(new File(m_root, m_phone.getSipBasicFilename()).exists());
        assertFalse(new File(m_root, m_phone.getFeaturesFilename()).exists());
        assertFalse(new File(m_root, m_phone.getRegAdvancedFilename()).exists());
        assertFalse(new File(m_root, m_phone.getRegionFilename()).exists());
        assertFalse(new File(m_root, m_phone.getSiteFilename()).exists());
        assertFalse(new File(m_root, m_phone.getVideoFilename()).exists());
    }

    public void testRestartFailureNoLine() throws Exception {
        m_phone.getLines().clear();
        m_phone.restart();
        m_tester.getSipControl().verify();
    }

    public void testRestart() throws Exception {

        String uri = m_phone.getLine(0).getUri();
        assertEquals("\"Joe User\"<sip:juser@sipfoundry.org>", uri);

        m_phone.restart();
        m_tester.getSipControl().verify();
    }

    public void testLineDefaults() throws Exception {
        Setting settings = m_tester.getPrimaryLine().getSettings();
        Setting address = settings.getSetting("reg/server/1/address");
        assertEquals("sipfoundry.org", address.getValue());
        
        Setting missedCallTracking = settings.getSetting("call/missedCallTracking/enabled");
        assertFalse((Boolean)missedCallTracking.getTypedValue());
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

    public void testExternalLine() throws Exception {

        LineInfo li = new LineInfo();
        li.setDisplayName("George Bush");
        li.setUserId("2000");
        li.setRegistrationServer("example.org");
        li.setPassword("1234");
        li.setVoiceMail("1001");

        Line externalLine = m_phone.createLine();
        m_phone.addLine(externalLine);
        externalLine.setLineInfo(li);

        assertEquals("\"George Bush\"<sip:2000@example.org>", externalLine.getUri());
        assertEquals("2000", externalLine.getSettingValue("msg.mwi/subscribe"));
        assertEquals("contact", externalLine.getSettingValue("msg.mwi/callBackMode"));
        assertEquals("1001", externalLine.getSettingValue("msg.mwi/callBack"));
    }

}
