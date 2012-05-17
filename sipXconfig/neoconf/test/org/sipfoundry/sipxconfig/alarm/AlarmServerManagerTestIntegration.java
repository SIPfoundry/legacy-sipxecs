/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.alarm;


import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

public class AlarmServerManagerTestIntegration extends IntegrationTestCase {
    private AlarmServerManager m_alarmServerManager;
    private CoreContext m_coreContext;

    public void setAlarmServerManager(AlarmServerManager alarmServerManager) {
        m_alarmServerManager = alarmServerManager;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
    
    private static Map<AlarmDefinition,Alarm> index(List<Alarm> list) {
        Map<AlarmDefinition,Alarm> map = new HashMap<AlarmDefinition,Alarm>(list.size());
        for (Alarm a : list) {
            map.put(a.getAlarmDefinition(), a);
        }
        return map;
    }

    @Override
    protected void onSetUpInTransaction() throws Exception {
        loadDataSet("alarm/alarm.db.xml");
        loadDataSet("alarm/user-alarm.db.xml");
    }
    
    public void testGetAlarms() throws Exception {
        clear();
        sql("alarm/alarm-seed.sql");
        Map<AlarmDefinition,Alarm> alarms = index(m_alarmServerManager.getAlarms());
        assertEquals(4, alarms.get(AdminContext.ALARM_LOGIN_FAILED).getMinThreshold());
    }
    
    public void testSaveAlarms() throws Exception {
        clear();
        Alarm a = new Alarm(AdminContext.ALARM_LOGIN_FAILED);
        a.setMinThreshold(5);
        m_alarmServerManager.saveAlarms(Collections.singletonList(a));
        int actual = db().queryForInt("select min_threshold from alarm_code where alarm_code_id = 'LOGIN_FAILED'");
        assertEquals(5, actual);
    }

    public void testGetAlarmServer() throws Exception {
        AlarmServer alarmServer = m_alarmServerManager.getAlarmServer();
        assertNotNull(alarmServer);
        assertTrue(alarmServer.isAlarmNotificationEnabled());
        AlarmGroup alarmGroup = m_alarmServerManager.getAlarmGroupByName("default");
        assertNotNull(alarmGroup);
    }

    public void testGetAlarmGroups() throws Exception {
        List<AlarmGroup> groups = m_alarmServerManager.getAlarmGroups();
        assertEquals(4, groups.size());
        AlarmGroup group1 = groups.get(1);
        assertEquals("sale", group1.getName());
        assertEquals("Sales", group1.getDescription());
        assertEquals(true, group1.isEnabled());
        Set<User> users = group1.getUsers();
        assertEquals(2, users.size());
        Iterator<User> it = users.iterator();
        List<String> usersName = new ArrayList<String>();
        while (it.hasNext()) {
            User user = it.next();
            usersName.add(user.getName());
        }
        assertTrue(usersName.contains("user1"));
        assertTrue(usersName.contains("user2"));
    }

    public void testGetAlarmGroupById() throws Exception {
        AlarmGroup group = m_alarmServerManager.getAlarmGroupById(new Integer(101));
        assertEquals("eng", group.getName());
        assertEquals("Engineering", group.getDescription());
        assertEquals(false, group.isEnabled());
        Set<User> users = group.getUsers();
        assertEquals(1, users.size());
    }

    public void testSaveAlarmGroup() throws Exception {
        List<AlarmGroup> groups = m_alarmServerManager.getAlarmGroups();
        assertEquals(4, groups.size());

        AlarmGroup group = new AlarmGroup();
        group.setName("testing");
        group.setDescription("test");
        Set<User> users = new HashSet<User>();
        users.add(m_coreContext.loadUser(1003));
        group.setUsers(users);
        m_alarmServerManager.saveAlarmGroup(group);
        groups = m_alarmServerManager.getAlarmGroups();
        assertEquals(5, groups.size());
    }

    public void testSaveDuplicateNumberAlarmGroup() throws Exception {
        AlarmGroup groupWithDuplicateNumber = new AlarmGroup();
        groupWithDuplicateNumber.setName("maintenance");
        try {
            m_alarmServerManager.saveAlarmGroup(groupWithDuplicateNumber);
        } catch (UserException ex) {
            assertTrue(true);
        }
    }

    public void testPreventDefaultGroupDeletion() {
        // Add a group.  (To ensure deletion of groups doesn't stop at the default group.)
        AlarmGroup group = new AlarmGroup();
        group.setName("testing");
        group.setDescription("test");
        m_alarmServerManager.saveAlarmGroup(group);

        // Collect all the group IDs.
        Collection<Integer> groupsIds = new ArrayList<Integer>();
        for (AlarmGroup group1 : m_alarmServerManager.getAlarmGroups()) {
            groupsIds.add(group1.getId());
        }

        // Try to delete them all, only the default group should remain.
        m_alarmServerManager.removeAlarmGroups(groupsIds, m_alarmServerManager.getAlarms());
        assertEquals(1, m_alarmServerManager.getAlarmGroups().size());
    }
    
    public void testTrapReceivers() {
        AlarmTrapReceiver r = new AlarmTrapReceiver();
        r.setHostAddress("snmp.example.org");
        m_alarmServerManager.saveAlarmTrapReceiver(r);
        flush();
        db().queryForInt("select 1 from alarm_receiver where address = ? ", r.getHostAddress());
        List<AlarmTrapReceiver> receivers = m_alarmServerManager.getAlarmTrapReceivers();
        assertEquals(1, receivers.size());
        m_alarmServerManager.deleteAlarmTrapReceiver(receivers.get(0));
        flush();
        assertEquals(0, db().queryForInt("select count(*) from alarm_receiver"));
    }
}
