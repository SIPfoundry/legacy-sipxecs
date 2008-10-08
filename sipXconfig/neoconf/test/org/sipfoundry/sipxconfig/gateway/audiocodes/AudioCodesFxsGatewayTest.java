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
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class AudioCodesFxsGatewayTest extends TestCase {

    public void testGenerateTypicalProfiles50() throws Exception {
        doTestGenerateTypicalProfiles(AudioCodesModel.REL_5_0);
    }

    public void testGenerateTypicalProfiles52() throws Exception {
        doTestGenerateTypicalProfiles(AudioCodesModel.REL_5_2);
    }

    public void doTestGenerateTypicalProfiles(DeviceVersion version) throws Exception {
        AudioCodesFxsModel model = new AudioCodesFxsModel();
        model.setBeanId("gwFxsAudiocodes");
        Set<String> features = new HashSet<String>();
        features.add("fxs");
        model.setSupportedFeatures(features);
        model.setProfileTemplate("audiocodes/gateway-%s.ini.vm");
        model.setModelDir("audiocodes");
        String configDirectory = TestHelper.getSysDirProperties().getProperty("audiocodesFxs.configDirectory");
        model.setConfigDirectory(configDirectory);

        AudioCodesFxsGateway gateway = new AudioCodesFxsGateway();
        gateway.setModel(model);
        gateway.setDeviceVersion(version);

        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(gateway);

        User u1 = new User();
        u1.setUserName("juser");
        u1.setFirstName("Joe");
        u1.setLastName("User");
        u1.setSipPassword("1234");

        User u2 = new User();
        u2.setUserName("buser");
        u2.setSipPassword("abcdef");
        u2.addAlias("432");

        // call this to inject dummy data
        PhoneTestDriver.supplyTestData(gateway, Arrays.asList(new User[] {
            u1, u2
        }));

        gateway.generateProfiles(location);

        String actual = location.toString();

        InputStream expectedProfile = getClass().getResourceAsStream("fxs-gateway-" + version.getVersionId() + ".ini");
        assertNotNull(expectedProfile);
        String expected = IOUtils.toString(expectedProfile);

        assertEquals(expected, actual);
    }
}
