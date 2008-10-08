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

public class AudioCodesBRIGatewayTest extends TestCase {
    private ModelFilesContext m_modelFilesContext;
    private AudioCodesGateway m_gateway;

    protected void setUp() throws Exception {
        m_modelFilesContext = TestHelper.getModelFilesContext();
        AudioCodesModel model = new AudioCodesModel();
        model.setBeanId("gwAudiocodes");
        model.setModelId("audiocodes");
        model.setModelDir("audiocodes");
        Set<String> features = new HashSet<String>();
        features.add("digital");
        features.add("bri");
        model.setSupportedFeatures(features);
        model.setMaxPorts(4);
        model.setProfileTemplate("audiocodes/gateway-%s.ini.vm");
        String configDirectory = TestHelper.getSysDirProperties().getProperty("audiocodesGatewayModel.configDirectory");
        model.setConfigDirectory(configDirectory);

        m_gateway = new AudioCodesBRIGateway();
        m_gateway.setModel(model);
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

        m_gateway.setSettingValue("SIP_general/SOURCENUMBERMAPIP2TEL", "*,0,$$,$$,$$,$$,*,1,*");
        m_gateway.setSettingValue("SIP_general/REMOVECLIWHENRESTRICTED", "1");
        m_gateway.setSettingValue("SIP_coders/CoderName/Type[0]", "g711Alaw64k");
        m_gateway.setSettingValue("SIP_coders/CoderName/Type[1]", "g729");
        m_gateway.setSettingValue("SIP_coders/CoderName/Type[2]", "g711Alaw64k");
        m_gateway.setSettingValue("SIP_coders/CoderName/Type[3]", "g711Alaw64k");
        m_gateway.setSettingValue("SIP_coders/CoderName/Type[4]", "g711Alaw64k");

        m_gateway.generateProfiles(location);

        String actual = location.toString();

        InputStream expectedProfile = getClass().getResourceAsStream("bri-gateway-" + version.getVersionId() + ".ini");
        assertNotNull(expectedProfile);
        String expected = IOUtils.toString(expectedProfile);

        assertEquals(expected, actual);
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
