/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */

package org.sipfoundry.sipxconfig.saa;

import static org.junit.Assert.assertEquals;

import java.io.InputStream;
import java.io.StringWriter;
import java.util.Arrays;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.junit.Test;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class SaaConfigurationTest {

    @Test
    public void testSaaConfig() throws Exception {
        SaaConfiguration out = new SaaConfiguration();
        StringWriter actual = new StringWriter();
        SaaSettings settings = new SaaSettings();
        settings.setModelFilesContext(TestHelper.getModelFilesContext());
        out.writeSaaConfig(actual, settings, "1.1.1.1");
        //this tests only data that is generated through sipxconfig, and not data generated through cfengine
        InputStream expected = getClass().getResourceAsStream("expected-sipxsaa-config");
        assertEquals(IOUtils.toString(expected), actual.toString());
    }

    @Test
    public void testXml() throws Exception {
        SaaConfiguration out = new SaaConfiguration();
        out.setVelocityEngine(TestHelper.getVelocityEngine());

        User firstSharedUser = new User();
        firstSharedUser.setUserName("sharedline");
        User secondSharedUser = new User();
        secondSharedUser.setUserName("321");
        List<User> users = Arrays.asList(firstSharedUser, secondSharedUser);

        StringWriter actual = new StringWriter();
        out.writeAppearance(actual, users, "example.org");
        InputStream expected = getClass().getResourceAsStream("expected-appearance-groups.xml");
        assertEquals(IOUtils.toString(expected), actual.toString());
    }
}
