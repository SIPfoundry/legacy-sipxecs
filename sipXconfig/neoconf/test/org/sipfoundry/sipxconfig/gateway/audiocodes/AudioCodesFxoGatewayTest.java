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
import org.easymock.classextension.EasyMock;
import org.easymock.classextension.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.gateway.FxoPort;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingSet;

public class AudioCodesFxoGatewayTest extends TestCase {
    private ModelFilesContext m_modelFilesContext;
    private AudioCodesModel m_model;
    private AudioCodesGateway m_gateway;

    protected void setUp() throws Exception {
        m_modelFilesContext = TestHelper.getModelFilesContext();
        m_model = new AudioCodesModel();
        m_model.setBeanId("gwAudiocodes");
        m_model.setModelId("audiocodes");
        m_model.setModelDir("audiocodes");
        Set<String> features = new HashSet<String>();
        features.add("fxo");
        m_model.setSupportedFeatures(features);
        m_model.setMaxPorts(4);
        m_model.setProfileTemplate("audiocodes/gateway-%s.ini.vm");
        String configDirectory = TestHelper.getSysDirProperties().getProperty("audiocodesGatewayModel.configDirectory");
        m_model.setConfigDirectory(configDirectory);

        m_gateway = new AudioCodesFxoGateway();
        m_gateway.setModel(m_model);
        m_gateway.setModelFilesContext(m_modelFilesContext);
        m_gateway.setDefaults(PhoneTestDriver.getDeviceDefaults());
    }

    public void testGenerateTypicalProfiles50() throws Exception {
        doTestGenerateTypicalProfiles(AudioCodesModel.REL_5_0);
    }

    public void testGenerateTypicalProfiles52() throws Exception {
        doTestGenerateTypicalProfiles(AudioCodesModel.REL_5_2);
    }

    public void doTestGenerateTypicalProfiles(DeviceVersion version) throws Exception {
        m_gateway.setDeviceVersion(version);
        for (int i = 0; i < 2; i++) {
            FxoPort trunk = new FxoPort();
            m_gateway.addPort(trunk);
        }

        m_gateway.setSerialNumber("001122334455");
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(m_gateway);

        // call this to inject dummy data

        m_gateway.generateProfiles(location);

        String actual = location.toString();

        InputStream expectedProfile = getClass().getResourceAsStream("fxo-gateway-" + version.getVersionId() + ".ini");
        assertNotNull(expectedProfile);
        String expected = IOUtils.toString(expectedProfile);

        assertEquals(expected, actual);
    }

    public void testGeneratePrevieProfiles() {
        for (int i = 0; i < 2; i++) {
            FxoPort trunk = new FxoPort();
            m_gateway.addPort(trunk);
        }

        m_gateway.setSerialNumber("001122334455");
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(m_gateway);

        m_gateway.generateFiles(location);
        assertTrue(location.toString().length() > 0);
    }

    public void testPrepareSettings() throws Exception {
        IMocksControl defaultsCtrl = EasyMock.createControl();
        DeviceDefaults defaults = defaultsCtrl.createMock(DeviceDefaults.class);
        defaults.getDomainName();
        defaultsCtrl.andReturn("mysipdomain.com").anyTimes();

        defaultsCtrl.replay();

        m_gateway.setDefaults(defaults);
        assertEquals("mysipdomain.com", m_gateway.getSettingValue("SIP_Proxy_Registration/ProxyIP"));

        defaultsCtrl.verify();
    }

    public void testGetSettings() throws Exception {
        Setting settings = m_gateway.getSettings();
        assertEquals("14", settings.getSetting("SIP_DTMF/MaxDigits").getValue());
        assertTrue(settings instanceof SettingSet);
        SettingSet root = (SettingSet) settings;
        SettingSet currentSettingSet = (SettingSet) root.getSetting("SIP_DTMF");
        assertEquals("14", currentSettingSet.getSetting("MaxDigits").getValue());
    }

    public void testGetActiveCalls() throws Exception {
        assertEquals(0, m_gateway.getMaxCalls());

        final int n = 3;
        for (int i = 0; i < n; i++) {
            FxoPort trunk = new FxoPort();
            m_gateway.addPort(trunk);
        }

        assertEquals(n, m_gateway.getMaxCalls());
    }

    public void testGetPortLabelTest() {
        FxoPort trunk = new FxoPort();
        m_gateway.addPort(trunk);

        assertEquals("Port", trunk.getLabel());
        m_model.setPortLabelSettings("Port/Extension", "Port/Extension");
        m_model.setPortLabelFormat("%s - (%s)");
        assertEquals("operator - (operator)", trunk.getLabel());
    }
}
