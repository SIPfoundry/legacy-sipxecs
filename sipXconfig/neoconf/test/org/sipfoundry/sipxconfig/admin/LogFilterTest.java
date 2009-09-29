/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin;

import java.text.DateFormat;
import java.util.Date;

import org.sipfoundry.sipxconfig.login.LogFilter;
import org.sipfoundry.sipxconfig.login.LoginEvent;

import junit.framework.TestCase;

public class LogFilterTest extends TestCase {

    private static final String LOG_ENTRY = "\"2007-10-15 00:37:57,675\": INFO:login:78:LOGIN SUCCESS, user superadmin, remote IP 11.126.12.21";

    private LoginEvent m_loginEvent;

    protected void setUp() {
        m_loginEvent = new LoginEvent(LOG_ENTRY);
    }

    public void testIsLogEntryInQueryByType() {
        LogFilter successFilter = new LogFilter(LoginEvent.SUCCESS, null, null, null, null);
        LogFilter failedFilter = new LogFilter(LoginEvent.FAILURE, null, null, null, null);
        LogFilter allFilter = new LogFilter(null, null, null, null, null);
        assertTrue(successFilter.isLogEntryInQuery(m_loginEvent));
        assertFalse(failedFilter.isLogEntryInQuery(m_loginEvent));
        assertTrue(allFilter.isLogEntryInQuery(m_loginEvent));
    }

    public void testIsLogEntryInQueryByUser() {
        LogFilter superadminFilter = new LogFilter(null, null, null, "superadmin", null);
        assertTrue(superadminFilter.isLogEntryInQuery(m_loginEvent));
        LogFilter userFilter = new LogFilter(null, null, null, "user", null);
        assertFalse(userFilter.isLogEntryInQuery(m_loginEvent));
    }

    public void testIsLogEntryInQueryByIp() {
        LogFilter okIpFilter = new LogFilter(null, null, null, null, "11.126.12.21");
        assertTrue(okIpFilter.isLogEntryInQuery(m_loginEvent));
        LogFilter badIpFilter = new LogFilter(null, null, null, null, "11.126.12.2");
        assertFalse(badIpFilter.isLogEntryInQuery(m_loginEvent));
    }

    public void testIsLogEntryInDateRange() throws Exception {
        Date startDate = DateFormat.getInstance().parse("07/10/2006 4:5 PM, PDT");
        Date endDate = DateFormat.getInstance().parse("07/10/2008 4:5 PM, PDT");
        LogFilter dateRangeFilter = new LogFilter(null, startDate, endDate, null, null);
        assertTrue(dateRangeFilter.isLogEntryInQuery(m_loginEvent));
        endDate = DateFormat.getInstance().parse("07/10/2006 4:5 PM, PDT");
        dateRangeFilter = new LogFilter(null, startDate, endDate, null, null);
        assertFalse(dateRangeFilter.isLogEntryInQuery(m_loginEvent));
    }

    public void testIsLogEntryInAllQueries() throws Exception {
        Date startDate = DateFormat.getInstance().parse("15/10/2006 4:5 PM, PDT");
        Date endDate = DateFormat.getInstance().parse("15/10/2008 4:5 PM, PDT");
        LogFilter filter = new LogFilter(LoginEvent.SUCCESS, startDate, endDate, "superadmin",
                "11.126.12.21");
        assertTrue(filter.isLogEntryInQuery(m_loginEvent));
    }

    public void testFormatLogToRecord() {
        assertEquals("LOGIN SUCCESS, user superadmin, remote IP 11.126.12.21", LoginEvent
                .formatLogToRecord(LoginEvent.SUCCESS, "superadmin", "11.126.12.21"));
    }

    public void testFormatLogEntry() {
        assertEquals(
                "2007-10-15 00:37:57 | LOGIN SUCCESS | remote IP 11.126.12.21 | user superadmin ",
                m_loginEvent.formatLogEntry());
    }

}
