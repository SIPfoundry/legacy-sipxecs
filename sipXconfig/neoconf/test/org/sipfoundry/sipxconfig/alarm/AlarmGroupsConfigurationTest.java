/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.alarm;

import static java.util.Arrays.asList;
import static org.junit.Assert.assertEquals;

import java.io.InputStream;
import java.io.StringWriter;
import java.util.ArrayList;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class AlarmGroupsConfigurationTest {
    private AlarmConfiguration m_alarmConf;

    @Before
    public void setUp() throws Exception {
        m_alarmConf = new AlarmConfiguration();
        m_alarmConf.setVelocityEngine(TestHelper.getVelocityEngine());
    }

    @Test
    public void testGenerateAlarmServer() throws Exception {
        AlarmGroup group1 = new AlarmGroup();
        group1.setName("emergency");
        group1.setEmailAddresses(asList("test_email1@example.com"));
        group1.setSmsAddresses(asList("test_sms1@example.com"));

        List<AlarmTrapReceiver> m_snmpAddresses = new ArrayList<AlarmTrapReceiver>();
        AlarmTrapReceiver alarmTrapReceiver = new AlarmTrapReceiver();
        alarmTrapReceiver.setCommunityString("ses");
        alarmTrapReceiver.setHostAddress("47.152.236.68");
        alarmTrapReceiver.setPort(162);
        m_snmpAddresses.add(alarmTrapReceiver);
        group1.setSnmpAddresses(m_snmpAddresses);

        StringWriter actual = new StringWriter();
        m_alarmConf.writeAlarmGroupsXml(actual, asList(group1));

        InputStream expected = getClass().getResourceAsStream("alarm-groups-test.xml");
        assertEquals(IOUtils.toString(expected), actual.toString());
    }

    @Test
    public void testGenerateAlarmServerWithUsers() throws Exception {
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

        List<AlarmTrapReceiver> m_snmpAddresses = new ArrayList<AlarmTrapReceiver>();
        AlarmTrapReceiver alarmTrapReceiver = new AlarmTrapReceiver();
        alarmTrapReceiver.setCommunityString("ses");
        alarmTrapReceiver.setHostAddress("47.152.236.68");
        alarmTrapReceiver.setPort(162);
        m_snmpAddresses.add(alarmTrapReceiver);
        group1.setSnmpAddresses(m_snmpAddresses);
        
        StringWriter actual = new StringWriter();
        m_alarmConf.writeAlarmGroupsXml(actual, asList(group1));

        InputStream expected = getClass().getResourceAsStream("alarm-groups-users-test.xml");
        assertEquals(IOUtils.toString(expected), actual.toString());
    }
}
