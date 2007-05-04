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

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.easymock.classextension.EasyMock;
import org.easymock.classextension.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.gateway.FxoPort;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingSet;

public class AudioCodesFxoGatewayTest extends TestCase {
    private ModelFilesContext m_modelFilesContext;
    private AudioCodesModel m_model;

    protected void setUp() throws Exception {
        m_modelFilesContext = TestHelper.getModelFilesContext();
        m_model = new AudioCodesModel();
        m_model.setBeanId("gwAudiocodes");
        m_model.setModelId("audiocodes");
        m_model.setFxo(true);
        m_model.setMaxPorts(4);
        m_model.setProfileTemplate("audiocodes/gateway.ini.vm");
    }

    public void testGenerateTypicalProfiles() throws Exception {
        Gateway gateway = new AudioCodesFxoGateway();
        gateway.setModel(m_model);

        for (int i = 0; i < 2; i++) {
            FxoPort trunk = new FxoPort();
            gateway.addPort(trunk);
        }

        gateway.setSerialNumber("001122334455");

        gateway.setModelFilesContext(m_modelFilesContext);
        gateway.setDefaults(PhoneTestDriver.getDeviceDefaults());

        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(gateway);

        // call this to inject dummy data

        gateway.generateProfiles();

        String actual = location.toString();

        InputStream expectedProfile = getClass().getResourceAsStream("fxo-gateway.ini");
        assertNotNull(expectedProfile);
        String expected = IOUtils.toString(expectedProfile);

        assertEquals(expected, actual);
    }

    public void testPrepareSettings() throws Exception {
        IMocksControl defaultsCtrl = EasyMock.createControl();
        DeviceDefaults defaults = defaultsCtrl.createMock(DeviceDefaults.class);
        defaults.getDomainName();
        defaultsCtrl.andReturn("mysipdomain.com").anyTimes();

        defaultsCtrl.replay();

        Gateway gateway = new AudioCodesFxoGateway();
        gateway.setModel(m_model);
        gateway.setModelFilesContext(m_modelFilesContext);
        gateway.setDefaults(defaults);

        assertEquals("mysipdomain.com", gateway.getSettingValue("SIP_Proxy_Registration/ProxyIP"));

        defaultsCtrl.verify();
    }

    public void testGetSettings() throws Exception {
        Gateway gateway = new AudioCodesFxoGateway();
        gateway.setModel(m_model);
        gateway.setModelFilesContext(m_modelFilesContext);

        Setting settings = gateway.getSettings();
        assertEquals("14", settings.getSetting("SIP_DTMF/MaxDigits").getValue());
        assertTrue(settings instanceof SettingSet);
        SettingSet root = (SettingSet) settings;
        SettingSet currentSettingSet = (SettingSet) root.getSetting("SIP_DTMF");
        assertEquals("14", currentSettingSet.getSetting("MaxDigits").getValue());
    }
}
