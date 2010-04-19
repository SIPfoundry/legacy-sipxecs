/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.gateway.audiocodes;

import java.io.InputStream;
import java.util.HashSet;
import java.util.Set;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.gateway.FxoPort;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;

public class AudioCodesDigitalGatewayTest extends TestCase {
    private ModelFilesContext m_modelFilesContext;
    private AudioCodesGateway m_gateway;

    @Override
    protected void setUp() throws Exception {
        m_modelFilesContext = TestHelper.getModelFilesContext();
        AudioCodesModel model = new AudioCodesModel();
        model.setBeanId("gwAudiocodes");
        model.setModelId("audiocodes");
        model.setModelDir("audiocodes");
        Set<String> features = new HashSet<String>();
        features.add("trunkGateway");
        features.add("digital");
        model.setSupportedFeatures(features);
        model.setMaxPorts(4);
        model.setProfileTemplate("audiocodes/gateway-%s.ini.vm");
        String configDirectory = TestHelper.getSysDirProperties().getProperty("audiocodesGatewayModel.configDirectory");
        model.setConfigDirectory(configDirectory);

        m_gateway = new AudioCodesDigitalGateway();
        m_gateway.setModel(model);
        m_gateway.setModelFilesContext(m_modelFilesContext);
        m_gateway.setDefaults(PhoneTestDriver.getDeviceDefaults());
    }

    public void testGenerateTypicalProfiles() throws Exception {
        DeviceVersion[] versions = m_gateway.getModel().getVersions();
        for (DeviceVersion v : versions) {
            setUp();
            doTestGenerateTypicalProfiles(v);
        }
    }

    public void doTestGenerateTypicalProfiles(DeviceVersion version) throws Exception {
        m_gateway.setDeviceVersion(version);
        for (int i = 0; i < 2; i++) {
            FxoPort trunk = new FxoPort();
            m_gateway.addPort(trunk);
        }
        m_gateway.setSerialNumber("001122334455");
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(m_gateway);

        m_gateway.setSettingValue("SIP_general/SOURCENUMBERMAPIP2TEL","*,0,$$,$$,$$,$$,*,1,*");
        m_gateway.setSettingValue("SIP_general/REMOVECLIWHENRESTRICTED","1");
        m_gateway.setSettingValue("SIP_coders/CoderName", "g711Alaw64k|g729");
        m_gateway.setSettingValue("advanced_general/SAS/EnableSAS", "1");
        m_gateway.setSettingValue("advanced_general/SAS/SASLocalSIPUDPPort", "5440");
        m_gateway.setSettingValue("advanced_general/SAS/SASDefaultGatewayIP", "10.10.10.50");
        if(AudioCodesModel.REL_5_4 == version) {
            m_gateway.setSettingValue("advanced_general/SAS/SASLocalSIPTCPPort", "5440");
            m_gateway.setSettingValue("advanced_general/SAS/SASLocalSIPTLSPort", "5441");
            m_gateway.setSettingValue("advanced_general/SAS/SASRegistrationTime", "5");
            m_gateway.setSettingValue("advanced_general/SAS/SASShortNumberLength", "4");
        }
        m_gateway.setSettingValue("Network/NTPServerIP", "10.10.10.40");
        if(AudioCodesModel.REL_6_0 == version) {
           m_gateway.setSettingValue("tel2ip-call-routing/tel-to-ip-failover/ProxyAddress", "10.10.10.50:5080");
           m_gateway.setSettingValue("advanced_general/AdditionalIPs", "176.1.2.3 176.1.2.4");
        }

        m_gateway.generateProfiles(location);
        String actual_lines[] = location.toString("001122334455.ini").split("\n");

        String expectedName = "digital-gateway-" + version.getVersionId() + ".ini";
        InputStream expectedProfile = getClass().getResourceAsStream(expectedName);
        assertNotNull(version.getVersionId(), expectedProfile);
        String expected_lines[] = IOUtils.toString(expectedProfile).split("\n");

        for(int x=0; x < expected_lines.length; x++) {
            String line = expectedName + " line " + (x+1);
            assertTrue(line, x < actual_lines.length); // Generated too few lines?
            assertEquals(line, expected_lines[x], actual_lines[x]);
        }
        assertEquals(expectedName, expected_lines.length, actual_lines.length); // Generated too many lines?
    }

    public void testGetActiveCalls() throws Exception {
        assertEquals(0, m_gateway.getMaxCalls());

        final int n = 3;
        for (int i = 0; i < n; i++) {
            FxoPort trunk = new FxoPort();
            m_gateway.addPort(trunk);
            trunk.setSettingValue("Trunk/MaxChannel", Integer.toString(5 + i));

        }
        // 18 == (5 + 0) + (5 + 1) + (5 + 2)
        assertEquals(18, m_gateway.getMaxCalls());
    }

}
