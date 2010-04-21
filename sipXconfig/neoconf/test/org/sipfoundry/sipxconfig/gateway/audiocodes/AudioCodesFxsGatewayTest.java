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

    public void testGenerateTypicalProfiles() throws Exception {
        for (DeviceVersion v : AudioCodesFxsModel.VERSIONS) {
            doTestGenerateTypicalProfiles(v);
        }
    }

    public void doTestGenerateTypicalProfiles(DeviceVersion version) throws Exception {
        AudioCodesFxsModel model = new AudioCodesFxsModel();
        model.setBeanId("gwFxsAudiocodes");
        Set<String> features = new HashSet<String>();
        features.add("fxs");
        features.add("useProxySet0");
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

        gateway.setSerialNumber("001122334455");
        gateway.generateProfiles(location);
        String actual_lines[] = location.toString("001122334455.ini").split("\n");

        String expectedName = "fxs-gateway-" + version.getVersionId() + ".ini";
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
}
