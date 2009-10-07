/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.alarm;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmGroup;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmServerManager;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmServer;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;

public class AlarmServerManagerTestIntegration extends IntegrationTestCase {
    private AlarmServerManager m_alarmServerManager;
    private CoreContext m_coreContext;

    public void setAlarmServerManager(AlarmServerManager alarmServerManager) {
        m_alarmServerManager = alarmServerManager;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    @Override
    protected void onSetUpInTransaction() throws Exception {
        loadDataSet("admin/alarm/alarm.db.xml");
        loadDataSet("admin/alarm/user-alarm.db.xml");
    }

    public void testGetAlarmServer() throws Exception {
        AlarmServer alarmServer = m_alarmServerManager.getAlarmServer();
        assertTrue(alarmServer.isEmailNotificationEnabled());
        AlarmGroup alarmGroup = m_alarmServerManager.getAlarmGroupByName("default");
        assertEquals("sipxpbxuser@localhost", alarmGroup.getEmailAddresses().get(0));
    }

    public void testGetAlarmGroups() throws Exception {
        List<AlarmGroup> groups = m_alarmServerManager.getAlarmGroups();
        assertEquals(3, groups.size());
        AlarmGroup group1 = groups.get(0);
        assertEquals("sale", group1.getName());
        assertEquals("Sales", group1.getDescription());
        assertEquals(true, group1.isEnabled());
        Set<User> users = group1.getUsers();
        assertEquals(2, users.size());
        Iterator it = users.iterator();
        List<String> usersName = new ArrayList<String>();
        while (it.hasNext()) {
            User user = (User) it.next();
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

    public void testDeleteAlarmGroupsById() throws Exception {
        List<AlarmGroup> groups = m_alarmServerManager.getAlarmGroups();
        assertEquals(3, groups.size());
        List<Integer> groupsIds = new ArrayList<Integer>();
        groupsIds.add(101);
        groupsIds.add(102);
        m_alarmServerManager.deleteAlarmGroupsById(groupsIds);
        groups = m_alarmServerManager.getAlarmGroups();

        // 2 alarm groups should disappear
        assertEquals(1, groups.size());
    }

    public void testSaveAlarmGroup() throws Exception {
        List<AlarmGroup> groups = m_alarmServerManager.getAlarmGroups();
        assertEquals(3, groups.size());

        AlarmGroup group = new AlarmGroup();
        group.setName("testing");
        group.setDescription("test");
        Set<User> users = new HashSet<User>();
        users.add(m_coreContext.loadUser(1003));
        group.setUsers(users);
        m_alarmServerManager.saveAlarmGroup(group);
        groups = m_alarmServerManager.getAlarmGroups();
        assertEquals(4, groups.size());
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

    public void testClear() throws Exception {
        m_alarmServerManager.clear();

        // they should be gone
        List<AlarmGroup> groups = m_alarmServerManager.getAlarmGroups();
        assertEquals(0, groups.size());
    }
}
