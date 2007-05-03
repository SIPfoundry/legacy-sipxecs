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
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class AudioCodesDigitalGatewayTest extends TestCase {

    public void testGenerateTypicalProfiles() throws Exception {
        AudioCodesModel model = new AudioCodesModel();
        model.setBeanId("gwAudiocodes");
        model.setModelId("audiocodes");
        model.setFxs(false);
        model.setFxo(false);
        model.setDigital(true);
        model.setMaxPorts(4);
        model.setProfileTemplate("audiocodes/mp-gateway.ini.vm");
        
        AudioCodesGateway gateway = new AudioCodesGateway();
        gateway.setModel(model);
        gateway.addPort(new FxoPort());
        gateway.setSerialNumber("001122334455");
        
        gateway.setModelFilesContext(TestHelper.getModelFilesContext());
        
        gateway.setDefaults(PhoneTestDriver.getDeviceDefaults());
        
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(gateway);
        
        
        // call this to inject dummy data
        
        gateway.generateProfiles();
        
        String actual = location.toString();
        System.err.println(actual);
        
        InputStream expectedProfile = getClass().getResourceAsStream("mp-gateway-digital.ini");
        assertNotNull(expectedProfile);
        String expected = IOUtils.toString(expectedProfile);

        assertEquals(expected, actual);
    }
}
