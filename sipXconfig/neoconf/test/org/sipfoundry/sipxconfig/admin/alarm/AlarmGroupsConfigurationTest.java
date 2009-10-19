/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.alarm;

import java.io.InputStream;
import java.util.LinkedHashSet;
import java.util.Set;
import static java.util.Arrays.asList;

import junit.framework.TestCase;
import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.AbstractConfigurationFile;
import org.sipfoundry.sipxconfig.common.User;

public class AlarmGroupsConfigurationTest extends TestCase {
    public void testGenerateAlarmServer() throws Exception {
        AlarmGroupsConfiguration alarmGroupsConf = new AlarmGroupsConfiguration();
        alarmGroupsConf.setVelocityEngine(TestHelper.getVelocityEngine());
        alarmGroupsConf.setTemplate("alarms/alarm-groups.vm");

        AlarmGroup group1 = new AlarmGroup();
        group1.setName("emergency");
        group1.setEmailAddresses(asList("test_email1@example.com"));
        group1.setSmsAddresses(asList("test_sms1@example.com"));

        alarmGroupsConf.generate(asList(group1));

        String generatedXml = AbstractConfigurationFile.getFileContent(alarmGroupsConf, null);
        InputStream referenceXmlStream = getClass().getResourceAsStream("alarm-groups-test.xml");
        assertEquals(IOUtils.toString(referenceXmlStream), generatedXml);
    }

    public void testGenerateAlarmServerWithUsers() throws Exception {
        AlarmGroupsConfiguration alarmGroupsConf = new AlarmGroupsConfiguration();
        alarmGroupsConf.setVelocityEngine(TestHelper.getVelocityEngine());
        alarmGroupsConf.setTemplate("alarms/alarm-groups.vm");

        User user1 = new User();
        user1.setUniqueId();
        user1.setEmailAddress("test_email1@example.com");

        User user2 = new User();
        user2.setUniqueId();
        user2.setAlternateEmailAddress("test_email2@example.com");

        Set<User> users = new LinkedHashSet<User>();
        users.add(user1);
        users.add(user2);

        AlarmGroup group1 = new AlarmGroup();
        group1.setName("emergency");
        group1.setUsers(users);

        alarmGroupsConf.generate(asList(group1));

        String generatedXml = AbstractConfigurationFile.getFileContent(alarmGroupsConf, null);
        InputStream referenceXmlStream = getClass().getResourceAsStream("alarm-groups-users-test.xml");
        assertEquals(IOUtils.toString(referenceXmlStream), generatedXml);
    }
}
