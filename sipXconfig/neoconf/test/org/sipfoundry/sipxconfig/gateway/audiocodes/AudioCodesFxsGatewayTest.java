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

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;

public class AudioCodesFxsGatewayTest extends TestCase {

    public void testGenerateTypicalProfiles() throws Exception {
        AudioCodesFxsModel model = new AudioCodesFxsModel();
        model.setBeanId("gwFxsAudiocodes");
        model.setFxs(true);
        model.setProfileTemplate("audiocodes/gateway.ini.vm");

        AudioCodesFxsGateway gateway = new AudioCodesFxsGateway();
        gateway.setModel(model);

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

        InputStream expectedProfile = getClass().getResourceAsStream("fxs-gateway.ini");
        assertNotNull(expectedProfile);
        String expected = IOUtils.toString(expectedProfile);

        assertEquals(expected, actual);
    }

}
