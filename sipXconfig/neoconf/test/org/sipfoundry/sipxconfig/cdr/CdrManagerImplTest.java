/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.cdr;

import java.io.OutputStreamWriter;
import java.io.StringWriter;
import java.io.Writer;
import java.sql.ResultSet;
import java.sql.Timestamp;
import java.util.Calendar;
import java.util.List;
import java.util.TimeZone;

import org.apache.commons.lang.time.DateFormatUtils;
import org.apache.commons.lang.time.DateUtils;
import org.easymock.EasyMock;
import org.easymock.IArgumentMatcher;
import org.easymock.IMocksControl;
import org.easymock.internal.matchers.InstanceOf;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.cdr.Cdr.Termination;
import org.sipfoundry.sipxconfig.cdr.CdrManagerImpl.CdrsResultReader;
import org.sipfoundry.sipxconfig.cdr.CdrManagerImpl.ColumnInfo;
import org.sipfoundry.sipxconfig.cdr.CdrSearch.Mode;
import org.springframework.context.ApplicationContext;
import org.springframework.jdbc.core.RowCallbackHandler;

public class CdrManagerImplTest extends SipxDatabaseTestCase {

    // FIXME: reads from real CDR database - needs SIPXCDR_TEST
    public void _testGetCdrs() {
        ApplicationContext app = TestHelper.getApplicationContext();
        CdrManager cdrManager = (CdrManager) app.getBean(CdrManager.CONTEXT_BEAN_NAME);
        List<Cdr> cdrs = cdrManager.getCdrs(null, null, new CdrSearch(), null);
        assertTrue(cdrs.size() > 0);
    }

    public void _testGetCdrsCount() {
        ApplicationContext app = TestHelper.getApplicationContext();
        CdrManager cdrManager = (CdrManager) app.getBean(CdrManager.CONTEXT_BEAN_NAME);
        int size = cdrManager.getCdrCount(null, null, new CdrSearch(), null);
        assertTrue(size > 0);
    }

    public void _testDumpCdrs() throws Exception {
        ApplicationContext app = TestHelper.getApplicationContext();
        CdrManager cdrManager = (CdrManager) app.getBean(CdrManager.CONTEXT_BEAN_NAME);
        OutputStreamWriter writer = new OutputStreamWriter(System.err);
        cdrManager.dumpCdrs(writer, null, null, new CdrSearch(), null);
    }

    public void _testGetCsv() throws Exception {
        ApplicationContext app = TestHelper.getApplicationContext();
        CdrManager cdrManager = (CdrManager) app.getBean(CdrManager.CONTEXT_BEAN_NAME);
        Writer writer = new OutputStreamWriter(System.err);
        cdrManager.dumpCdrs(writer, null, null, new CdrSearch(), null);
        writer.flush();
    }

    public void _testGetCdrsSearch() {
        ApplicationContext app = TestHelper.getApplicationContext();
        CdrManager cdrManager = (CdrManager) app.getBean(CdrManager.CONTEXT_BEAN_NAME);
        CdrSearch cdrSearch = new CdrSearch();
        cdrSearch.setMode(Mode.ANY);
        cdrSearch.setTerm("154");
        List<Cdr> cdrs = cdrManager.getCdrs(null, null, cdrSearch, null);
        assertTrue(cdrs.size() > 0);
    }

    static class CalendarTimeZoneMatcher extends InstanceOf {

        private TimeZone m_tz;

        public CalendarTimeZoneMatcher(TimeZone tz) {
            super(Calendar.class);
            m_tz = tz;
        }

        public boolean matches(Object argument) {
            if (!super.matches(argument)) {
                return false;
            }
            Calendar cal = (Calendar) argument;
            return cal.getTimeZone().equals(m_tz);
        }
    }

    public static <T extends Calendar> T eqTimeZone(T cal) {
        IArgumentMatcher matcher = new CalendarTimeZoneMatcher(cal.getTimeZone());
        EasyMock.reportMatcher(matcher);
        return null;
    }

    public void testProcessRow() throws Exception {
        TimeZone tz = DateUtils.UTC_TIME_ZONE;
        Calendar calendar = Calendar.getInstance(tz);

        IMocksControl rsControl = EasyMock.createControl();
        ResultSet rs = rsControl.createMock(ResultSet.class);

        rs.getString("caller_aor");
        rsControl.andReturn("caller");
        rs.getString("callee_aor");
        rsControl.andReturn("callee");

        rs.getTimestamp(EasyMock.eq("start_time"), eqTimeZone(calendar));
        rsControl.andReturn(new Timestamp(0));
        rs.getTimestamp(EasyMock.eq("connect_time"), eqTimeZone(calendar));
        rsControl.andReturn(new Timestamp(1));
        rs.getTimestamp(EasyMock.eq("end_time"), eqTimeZone(calendar));
        rsControl.andReturn(new Timestamp(2));

        rs.getInt("failure_status");
        rsControl.andReturn(404);

        rs.getString("termination");
        rsControl.andReturn("I");

        rsControl.replay();

        CdrsResultReader reader = new CdrManagerImpl.CdrsResultReader(tz);
        reader.processRow(rs);

        List<Cdr> results = reader.getResults();
        assertEquals(1, results.size());
        Cdr cdr = results.get(0);
        assertEquals("callee", cdr.getCalleeAor());
        assertEquals("caller", cdr.getCallerAor());
        assertEquals(0, cdr.getStartTime().getTime());
        assertEquals(1, cdr.getConnectTime().getTime());
        assertEquals(2, cdr.getEndTime().getTime());
        assertEquals(404, cdr.getFailureStatus());
        assertEquals(Termination.IN_PROGRESS, cdr.getTermination());

        rsControl.verify();
    }

    public void testColumnInfo() throws Exception {
        IMocksControl rsControl = EasyMock.createControl();
        ResultSet rs = rsControl.createMock(ResultSet.class);
        for (int i = 0; i < ColumnInfo.FIELDS.length; i++) {
            rs.findColumn((String) EasyMock.anyObject());
            rsControl.andReturn(i);
        }
        rsControl.replay();

        ColumnInfo[] infos = ColumnInfo.create(rs);

        assertEquals(ColumnInfo.FIELDS.length, infos.length);
        for (int i = 0; i < infos.length; i++) {
            assertEquals(i, infos[i].getIndex());
            assertEquals(ColumnInfo.TIME_FIELDS[i], infos[i].isTimestamp());
        }
        rsControl.verify();
    }

    public void testCdrsCsvWriter() throws Exception {
        TimeZone tz = DateUtils.UTC_TIME_ZONE;
        Calendar calendar = Calendar.getInstance(tz);

        Timestamp timestamp = new Timestamp(System.currentTimeMillis());
        String dateStr = String.format("\"%s\",", DateFormatUtils.ISO_DATETIME_TIME_ZONE_FORMAT
                .format(timestamp));

        IMocksControl rsControl = EasyMock.createControl();
        ResultSet rs = rsControl.createMock(ResultSet.class);
        for (int i = 0; i < ColumnInfo.FIELDS.length; i++) {
            rs.findColumn((String) EasyMock.anyObject());
            rsControl.andReturn(i);
        }

        rs.getString(0);
        rsControl.andReturn("caller");
        rs.getString(1);
        rsControl.andReturn("callee");

        rs.getTimestamp(EasyMock.eq(2), eqTimeZone(calendar));
        rsControl.andReturn(timestamp);
        rs.getTimestamp(EasyMock.eq(3), eqTimeZone(calendar));
        rsControl.andReturn(timestamp);
        rs.getTimestamp(EasyMock.eq(4), eqTimeZone(calendar));
        rsControl.andReturn(timestamp);

        rs.getString(5);
        rsControl.andReturn("404");

        rs.getString(6);
        rsControl.andReturn("I");

        rsControl.replay();

        StringWriter writer = new StringWriter();

        RowCallbackHandler handler = new CdrManagerImpl.CdrsCsvWriter(writer, tz);
        handler.processRow(rs);

        assertEquals(
                "callee_aor,caller_aor,start_time,connect_time,end_time,failure_status,termination\n"
                        + "\"caller\",\"callee\"," + dateStr + dateStr + dateStr
                        + "\"404\",\"I\"\n", writer.toString());

        rsControl.verify();
    }
    
    public void testCdrsCsvWriterNullConnectTime() throws Exception {
        TimeZone tz = DateUtils.UTC_TIME_ZONE;
        Calendar calendar = Calendar.getInstance(tz);

        Timestamp timestamp = new Timestamp(System.currentTimeMillis());
        String dateStr = String.format("\"%s\",", DateFormatUtils.ISO_DATETIME_TIME_ZONE_FORMAT
                .format(timestamp));

        IMocksControl rsControl = EasyMock.createControl();
        ResultSet rs = rsControl.createMock(ResultSet.class);
        for (int i = 0; i < ColumnInfo.FIELDS.length; i++) {
            rs.findColumn((String) EasyMock.anyObject());
            rsControl.andReturn(i);
        }

        rs.getString(0);
        rsControl.andReturn("caller");
        rs.getString(1);
        rsControl.andReturn("callee");

        rs.getTimestamp(EasyMock.eq(2), eqTimeZone(calendar));
        rsControl.andReturn(timestamp);
        rs.getTimestamp(EasyMock.eq(3), eqTimeZone(calendar));
        rsControl.andReturn(null);
        rs.getTimestamp(EasyMock.eq(4), eqTimeZone(calendar));
        rsControl.andReturn(timestamp);

        rs.getString(5);
        rsControl.andReturn("404");

        rs.getString(6);
        rsControl.andReturn("I");

        rsControl.replay();

        StringWriter writer = new StringWriter();

        RowCallbackHandler handler = new CdrManagerImpl.CdrsCsvWriter(writer, tz);
        handler.processRow(rs);

        assertEquals(
                "callee_aor,caller_aor,start_time,connect_time,end_time,failure_status,termination\n"
                        + "\"caller\",\"callee\"," + dateStr + "\"\"," + dateStr
                        + "\"404\",\"I\"\n", writer.toString());

        rsControl.verify();
    }
    
}
