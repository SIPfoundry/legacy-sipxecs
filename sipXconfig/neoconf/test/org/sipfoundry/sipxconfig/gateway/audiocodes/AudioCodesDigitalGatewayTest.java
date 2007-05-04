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
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.gateway.FxoPort;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;

public class AudioCodesDigitalGatewayTest extends TestCase {
    private ModelFilesContext m_modelFilesContext;

    protected void setUp() throws Exception {
        m_modelFilesContext = TestHelper.getModelFilesContext();
    }

    public void testGenerateTypicalProfiles() throws Exception {
        AudioCodesModel model = new AudioCodesModel();
        model.setBeanId("gwAudiocodes");
        model.setModelId("audiocodes");
        model.setFxs(false);
        model.setFxo(false);
        model.setDigital(true);
        model.setMaxPorts(4);
        model.setProfileTemplate("audiocodes/gateway.ini.vm");

        Gateway gateway = new AudioCodesDigitalGateway();
        gateway.setModel(model);

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

        InputStream expectedProfile = getClass().getResourceAsStream("digital-gateway.ini");
        assertNotNull(expectedProfile);
        String expected = IOUtils.toString(expectedProfile);

        assertEquals(expected, actual);
    }
}
