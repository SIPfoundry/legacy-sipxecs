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

import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.classextension.EasyMock.createMock;
import static org.easymock.classextension.EasyMock.replay;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.List;

import junit.framework.TestCase;
import org.springframework.orm.hibernate3.HibernateTemplate;

public class AlarmTypesParserTest extends TestCase {
    public void testGetAlarmTypes() {
        InputStream isConfig = getClass().getResourceAsStream("alarms-config-test.xml");
        InputStream isStrings = getClass().getResourceAsStream("alarms-strings-test.xml");

        List<Alarm> oldAlarms = new ArrayList<Alarm>();
        HibernateTemplate ht = createMock(HibernateTemplate.class);
        ht.findByNamedQueryAndNamedParam("alarmForAlarmId", "alarmId", "LOGIN_FAILED");
        expectLastCall().andReturn(oldAlarms).anyTimes();
        ht.findByNamedQueryAndNamedParam("alarmForAlarmId", "alarmId", "PROCESS_FAILED");
        expectLastCall().andReturn(oldAlarms).anyTimes();
        replay(ht);

        AlarmTypesParser parser = new AlarmTypesParser(ht);
        List<Alarm> alarms = parser.getTypes(isConfig, isStrings);

        assertEquals(2, alarms.size());
        Alarm alarm = alarms.get(0);
        assertEquals("LOGIN_FAILED", alarm.getAlarmId());
        assertEquals("SPX00005", alarm.getCode());
        assertEquals("Login failed", alarm.getShortTitle());
        assertEquals("warning", alarm.getSeverity());
        assertEquals("default", alarm.getGroupName());

        alarm = alarms.get(1);
        assertEquals("PROCESS_FAILED", alarm.getAlarmId());
        assertEquals("SPX00004", alarm.getCode());
        assertEquals("Process cannot be restarted!", alarm.getShortTitle());
        assertEquals("crit", alarm.getSeverity());
        assertEquals("disabled", alarm.getGroupName());
    }
}
