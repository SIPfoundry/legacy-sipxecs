/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.gateway.audiocodes;

import java.io.InputStream;
import java.util.Set;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.easymock.classextension.EasyMock;
import org.easymock.classextension.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.BeanFactoryModelSource;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.gateway.GatewayModel;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingSet;

public class MediantGatewayTestDb extends TestCase {
    private AudioCodesModel m_model;
    private MediantGateway m_gateway;

    protected void setUp() throws Exception {
        BeanFactoryModelSource<GatewayModel> modelSource = (BeanFactoryModelSource<GatewayModel>) TestHelper
                .getApplicationContext().getBean("nakedGatewayModelSource");
        m_model = (AudioCodesModel) modelSource.getModel("audiocodesMP1X4_4_FXO");
        m_gateway = (MediantGateway) TestHelper.getApplicationContext().getBean(m_model.getBeanId());
        m_gateway.setModelId(m_model.getModelId());
        m_gateway.setSerialNumber("FT0123456");
    }

    public void testModel() {
        assertSame(m_model, m_gateway.getModel());
        Set definitions = m_model.getDefinitions();        
        assertTrue(definitions.contains("fxo"));
        assertFalse(definitions.contains("fxs"));
    }
    
    public void testGenerateProfiles() throws Exception {
        m_gateway.setName("ac_gateway");
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(m_gateway);
        m_gateway.generateProfiles();

        InputStream expectedProfile = getClass().getResourceAsStream("mp-gateway.ini");
        assertNotNull(expectedProfile);
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();
        
        String actual = location.toString();

        String expectedLines[] = StringUtils.split(expected, "\n");
        String actualLines[] = StringUtils.split(actual, "\n");

        int len = Math.min(actualLines.length, expectedLines.length);
        for (int i = 0; i < len; i++) {
            assertEquals(expectedLines[i], actualLines[i]);
        }
        assertEquals(expectedLines.length, actualLines.length);        
        assertEquals(expected, actual);
    }

    public void testPrepareSettings() throws Exception {
        assertSame(m_model, m_gateway.getModel());

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
}
