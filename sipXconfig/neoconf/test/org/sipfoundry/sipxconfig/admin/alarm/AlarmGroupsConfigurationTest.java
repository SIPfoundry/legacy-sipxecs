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

import java.util.ArrayList;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;
import static java.util.Arrays.asList;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.service.SipxAlarmService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.service.SipxServiceTestBase;

public class AlarmGroupsConfigurationTest extends SipxServiceTestBase {
    private AlarmGroupsConfiguration m_alarmGroupsConf;

    @Override
    public void setUp() throws Exception {
        m_alarmGroupsConf = new AlarmGroupsConfiguration();
        m_alarmGroupsConf.setVelocityEngine(TestHelper.getVelocityEngine());
        m_alarmGroupsConf.setTemplate("alarms/alarm-groups.vm");

        SipxAlarmService alarmService = new SipxAlarmService();
        SipxServiceManager sipxServiceManager = EasyMock.createMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxAlarmService.BEAN_ID);
        EasyMock.expectLastCall().andReturn(alarmService).anyTimes();
        EasyMock.replay(sipxServiceManager);
        m_alarmGroupsConf.setSipxServiceManager(sipxServiceManager);
    }

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

        AlarmServerManager alarmServerManager = EasyMock.createMock(AlarmServerManager.class);
        alarmServerManager.getAlarmGroups();
        EasyMock.expectLastCall().andReturn(asList(group1)).anyTimes();
        EasyMock.replay(alarmServerManager);
        m_alarmGroupsConf.setAlarmServerManager(alarmServerManager);

        assertCorrectFileGeneration(m_alarmGroupsConf, "alarm-groups-test.xml");
    }

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

        AlarmServerManager alarmServerManager = EasyMock.createMock(AlarmServerManager.class);
        alarmServerManager.getAlarmGroups();
        EasyMock.expectLastCall().andReturn(asList(group1)).anyTimes();
        EasyMock.replay(alarmServerManager);
        m_alarmGroupsConf.setAlarmServerManager(alarmServerManager);

        assertCorrectFileGeneration(m_alarmGroupsConf, "alarm-groups-users-test.xml");
    }
}
