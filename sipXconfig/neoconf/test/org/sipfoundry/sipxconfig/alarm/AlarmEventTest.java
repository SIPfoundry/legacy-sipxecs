/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.alarm;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.HashMap;
import java.util.Map;
import java.util.TimeZone;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.AdminContext;

public class AlarmEventTest extends TestCase {
    private static final String LOG_ENTRY = "\"2009-06-22T13:39:56.406437Z\":4:ALARM:CRIT:sipx.example.org:sipXbridge::"
            + "LOGIN_FAILED:\"The STUN Server 'stun.ezuce.com' is dysfunctional.\"";
    private static final String LOG_ENTRY_WITH_COLONS = "\"2009-06-22T13:39:56.406437Z\":4:ALARM:CRIT:sipx.example.org:sipXbridge::"
            + "LOGIN_FAILED:\"The STUN ::Server 'stun.ezuce.com' is dysfunctional: one, two, three.\"";
    
    private AlarmServerManager m_mgr;
    
    public void setUp() {
        m_mgr = createMock(AlarmServerManager.class);
        m_mgr.getAlarmDefinitions();
        Map<String,AlarmDefinition> defs = new HashMap<String,AlarmDefinition>();
        defs.put(AdminContext.ALARM_LOGIN_FAILED.getId(), AdminContext.ALARM_LOGIN_FAILED);
        expectLastCall().andReturn(defs).anyTimes();
        replay(m_mgr);        
    }

    public void testParseAlarmLog() throws Exception {
        AlarmEvent ae = AlarmEvent.parseEvent(m_mgr, LOG_ENTRY);
        assertEquals(AdminContext.ALARM_LOGIN_FAILED, ae.geAlarmDefinition());
        assertNotNull(ae.getDate());
        final DateFormat f = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        // alarm times are the UTC times from the alarms log
        f.setTimeZone(TimeZone.getTimeZone("UTC"));
        assertEquals("2009-06-22 13:39:56", f.format(ae.getDate()));
    }

    public void testParseAlarmLogWithColons() throws Exception {
        AlarmEvent ae = AlarmEvent.parseEvent(m_mgr, LOG_ENTRY_WITH_COLONS);
        assertEquals(AdminContext.ALARM_LOGIN_FAILED, ae.geAlarmDefinition());
        assertNotNull(ae.getDate());
        final DateFormat f = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        // alarm times are the UTC times from the alarms log
        f.setTimeZone(TimeZone.getTimeZone("UTC"));
        assertEquals("2009-06-22 13:39:56", f.format(ae.getDate()));
    }
}
