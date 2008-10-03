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

import java.io.File;
import java.util.List;

import org.sipfoundry.sipxconfig.IntegrationTestCase;
import org.sipfoundry.sipxconfig.admin.alarm.Alarm;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmServer;
import org.sipfoundry.sipxconfig.common.AlarmContextImpl;

public class AlarmContextImplTestIntegration extends IntegrationTestCase {
    private AlarmContextImpl m_alarmContext;

    public void setAlarmContextImpl(AlarmContextImpl alarmContext) {
        m_alarmContext = alarmContext;
    }

    public void testGetAlarmServer() throws Exception {
        AlarmServer alarmServer = m_alarmContext.getAlarmServer();
        assertTrue(alarmServer.isEmailNotificationEnabled());
        assertEquals("sipxpbxuser@localhost", alarmServer.getContacts().getAddresses().get(0));
    }

    public void testGetAlarmTypes() {
        List<Alarm> alarms = m_alarmContext.getAlarmTypes(getFilePath("alarms-config-test.xml"),
                getFilePath("alarms-strings-test.xml"));
        assertEquals(2, alarms.size());
        Alarm alarm = alarms.get(0);
        assertEquals("LOGIN_FAILED", alarm.getAlarmId());
        assertEquals("SPX00005", alarm.getCode());
        assertEquals("Login failed", alarm.getShortTitle());
        assertEquals("warning", alarm.getSeverity());
        assertTrue(alarm.isEmailEnabled());

        alarm = alarms.get(1);
        assertEquals("PROCESS_FAILED", alarm.getAlarmId());
        assertEquals("SPX00004", alarm.getCode());
        assertEquals("Process cannot be restarted!", alarm.getShortTitle());
        assertEquals("crit", alarm.getSeverity());
        assertTrue(!alarm.isEmailEnabled());
    }

    private String getFilePath(String fileName) {
        return new File(getClass().getResource(fileName).getPath()).getAbsolutePath();
    }

    public void testGetSipxUser() {
        assertEquals("sipxpbxuser", m_alarmContext.getSipxUser());
    }
}
