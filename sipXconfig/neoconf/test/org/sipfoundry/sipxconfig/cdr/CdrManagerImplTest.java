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

import java.io.InputStream;
import java.io.OutputStreamWriter;
import java.io.StringWriter;
import java.io.Writer;
import java.net.URL;
import java.rmi.RemoteException;
import java.sql.ResultSet;
import java.sql.Timestamp;
import java.text.FieldPosition;
import java.text.Format;
import java.text.ParsePosition;
import java.util.Calendar;
import java.util.List;
import java.util.TimeZone;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.time.DateFormatUtils;
import org.apache.commons.lang.time.DateUtils;
import org.easymock.EasyMock;
import org.easymock.IArgumentMatcher;
import org.easymock.internal.matchers.InstanceOf;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.cdr.Cdr.Termination;
import org.sipfoundry.sipxconfig.cdr.CdrManagerImpl.CdrsResultReader;
import org.sipfoundry.sipxconfig.cdr.CdrManagerImpl.ColumnInfo;
import org.sipfoundry.sipxconfig.cdr.CdrManagerImpl.ColumnInfoFactory;
import org.sipfoundry.sipxconfig.cdr.CdrManagerImpl.DefaultColumnInfoFactory;
import org.sipfoundry.sipxconfig.cdr.CdrSearch.Mode;
import org.sipfoundry.sipxconfig.service.SipxCallResolverService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.springframework.context.ApplicationContext;
import org.springframework.jdbc.core.RowCallbackHandler;

import static org.easymock.EasyMock.anyObject;
import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.reportMatcher;
import static org.easymock.EasyMock.verify;

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

    public void _testGetJson() throws Exception {
        ApplicationContext app = TestHelper.getApplicationContext();
        CdrManager cdrManager = (CdrManager) app.getBean(CdrManager.CONTEXT_BEAN_NAME);
        Writer writer = new OutputStreamWriter(System.err);
        cdrManager.dumpCdrsJson(writer);
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
        reportMatcher(matcher);
        return null;
    }

    public void testProcessRow() throws Exception {
        TimeZone tz = DateUtils.UTC_TIME_ZONE;
        Calendar calendar = Calendar.getInstance(tz);

        ResultSet rs = createMock(ResultSet.class);

        rs.getString("caller_aor");
        expectLastCall().andReturn("caller");
        rs.getString("callee_aor");
        expectLastCall().andReturn("callee");

        rs.getTimestamp(eq("start_time"), eqTimeZone(calendar));
        expectLastCall().andReturn(new Timestamp(0));
        rs.getTimestamp(eq("connect_time"), eqTimeZone(calendar));
        expectLastCall().andReturn(new Timestamp(1));
        rs.getTimestamp(eq("end_time"), eqTimeZone(calendar));
        expectLastCall().andReturn(new Timestamp(2));

        rs.getInt("failure_status");
        expectLastCall().andReturn(404);

        rs.getString("termination");
        expectLastCall().andReturn("I");

        replay(rs);

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

        verify(rs);
    }

    public void testColumnInfo() throws Exception {
        ResultSet rs = createMock(ResultSet.class);
        for (int i = 0; i < ColumnInfo.FIELDS.length; i++) {
            rs.findColumn((String) anyObject());
            expectLastCall().andReturn(i);
        }
        replay(rs);

        ColumnInfoFactory ciFactory = new DefaultColumnInfoFactory(TimeZone.getDefault());
        ColumnInfo[] infos = ciFactory.create(rs);

        assertEquals(ColumnInfo.FIELDS.length, infos.length);
        for (int i = 0; i < infos.length; i++) {
            assertEquals(i, infos[i].getIndex());
            assertEquals(ColumnInfo.TIME_FIELDS[i], infos[i].isTimestamp());
        }
        verify(rs);
    }

    public void testCdrsCsvWriter() throws Exception {
        TimeZone tz = DateUtils.UTC_TIME_ZONE;
        Calendar calendar = Calendar.getInstance(tz);

        Timestamp timestamp = new Timestamp(System.currentTimeMillis());
        String dateStr = String.format("\"%s\",", DateFormatUtils.ISO_DATETIME_TIME_ZONE_FORMAT.format(timestamp));

        ResultSet rs = createMock(ResultSet.class);
        for (int i = 0; i < ColumnInfo.FIELDS.length; i++) {
            rs.findColumn((String) anyObject());
            expectLastCall().andReturn(i);
        }

        rs.getString(0);
        expectLastCall().andReturn("caller");
        rs.getString(1);
        expectLastCall().andReturn("callee");

        rs.getTimestamp(eq(2), eqTimeZone(calendar));
        expectLastCall().andReturn(timestamp);
        rs.getTimestamp(eq(3), eqTimeZone(calendar));
        expectLastCall().andReturn(timestamp);
        rs.getTimestamp(eq(4), eqTimeZone(calendar));
        expectLastCall().andReturn(timestamp);

        rs.getString(5);
        expectLastCall().andReturn("404");

        rs.getString(6);
        expectLastCall().andReturn("I");

        rs.getString(7);
        expectLastCall().andReturn("0000-0000");

        replay(rs);

        StringWriter writer = new StringWriter();

        ColumnInfoFactory columnInforFactory = new DefaultColumnInfoFactory(tz);
        CdrsWriter handler = new CdrsCsvWriter(writer, columnInforFactory);
        handler.writeHeader();
        handler.processRow(rs);
        handler.writeFooter();

        assertEquals("callee_aor,caller_aor,start_time,connect_time,end_time,failure_status,termination,call_id\n"
                + "\"caller\",\"callee\"," + dateStr + dateStr + dateStr + "\"404\",\"I\",\"0000-0000\"\n", writer
                .toString());

        verify(rs);
    }

    public void testCdrsJsonWriter() throws Exception {
        TimeZone tz = DateUtils.UTC_TIME_ZONE;
        Calendar calendar = Calendar.getInstance(tz);

        Timestamp timestamp = new Timestamp(0);

        ResultSet rs = createMock(ResultSet.class);
        for (int i = 0; i < ColumnInfo.FIELDS.length; i++) {
            rs.findColumn((String) anyObject());
            expectLastCall().andReturn(i);
        }

        rs.getString(0);
        expectLastCall().andReturn("\"Brian Ferry\"<sip:1111@example.org>");
        rs.getString(1);
        expectLastCall().andReturn("sip:callee@example.com");

        rs.getTimestamp(eq(2), eqTimeZone(calendar));
        expectLastCall().andReturn(timestamp);
        rs.getTimestamp(eq(3), eqTimeZone(calendar));
        expectLastCall().andReturn(timestamp);
        rs.getTimestamp(eq(4), eqTimeZone(calendar));
        expectLastCall().andReturn(timestamp);

        rs.getString(5);
        expectLastCall().andReturn("404");

        rs.getString(6);
        expectLastCall().andReturn("I");

        rs.getString(7);
        expectLastCall().andReturn("0000-0000");

        replay(rs);

        StringWriter writer = new StringWriter();

        Format testDateFormat = new Format() {
            public StringBuffer format(Object obj, StringBuffer toAppendTo, FieldPosition pos) {
                return toAppendTo.append("2008-05-02T06:29:08-04:00");
            }

            public Object parseObject(String source, ParsePosition pos) {
                return null;
            }
        };

        DefaultColumnInfoFactory ciFactory = new DefaultColumnInfoFactory(tz);
        ciFactory.setDateFormat(testDateFormat);
        ciFactory.setAorFormat(CdrsJsonWriter.AOR_FORMAT);
        CdrsWriter handler = new CdrsJsonWriter(writer, ciFactory);
        handler.writeHeader();
        handler.processRow(rs);
        handler.writeFooter();

        InputStream expectedJson = getClass().getResourceAsStream("cdrs.test.json");
        assertNotNull(expectedJson);

        assertEquals(IOUtils.toString(expectedJson), writer.toString());

        verify(rs);
    }

    public void testCdrsCsvWriterNullConnectTime() throws Exception {
        TimeZone tz = DateUtils.UTC_TIME_ZONE;
        Calendar calendar = Calendar.getInstance(tz);

        Timestamp timestamp = new Timestamp(System.currentTimeMillis());
        String dateStr = String.format("\"%s\",", DateFormatUtils.ISO_DATETIME_TIME_ZONE_FORMAT.format(timestamp));

        ResultSet rs = createMock(ResultSet.class);
        for (int i = 0; i < ColumnInfo.FIELDS.length; i++) {
            rs.findColumn((String) anyObject());
            expectLastCall().andReturn(i);
        }

        rs.getString(0);
        expectLastCall().andReturn("caller");
        rs.getString(1);
        expectLastCall().andReturn("callee");

        rs.getTimestamp(eq(2), eqTimeZone(calendar));
        expectLastCall().andReturn(timestamp);
        rs.getTimestamp(eq(3), eqTimeZone(calendar));
        expectLastCall().andReturn(null);
        rs.getTimestamp(eq(4), eqTimeZone(calendar));
        expectLastCall().andReturn(timestamp);

        rs.getString(5);
        expectLastCall().andReturn("404");

        rs.getString(6);
        expectLastCall().andReturn("I");

        rs.getString(7);
        expectLastCall().andReturn("0000-0000");

        replay(rs);

        StringWriter writer = new StringWriter();

        ColumnInfoFactory columnInforFactory = new DefaultColumnInfoFactory(tz);
        RowCallbackHandler handler = new CdrsCsvWriter(writer, columnInforFactory);
        handler.processRow(rs);

        assertEquals("\"caller\",\"callee\"," + dateStr + "\"\"," + dateStr + "\"404\",\"I\",\"0000-0000\"\n", writer
                .toString());

        verify(rs);
    }

    public void testGetCdrService() throws Exception {
        final String host = "cdr.example.org";
        final int port = 9999;

        CdrManagerImpl out = new CdrManagerImpl();

        CdrServiceProvider serviceProvider = EasyMock.createMock(CdrServiceProvider.class);
        serviceProvider.getCdrService(new URL("http", host, port, StringUtils.EMPTY));
        EasyMock.expectLastCall().andReturn(new CdrService() {
            public ActiveCall[] getActiveCalls() throws RemoteException {
                return null;
            }
        });
        out.setCdrServiceProvider(serviceProvider);

        SipxCallResolverService sipxCallResolverService = new SipxCallResolverService(){
            public String getAgentAddress() {
                return host;
            }
        };
        sipxCallResolverService.setAgentPort(port);
        SipxServiceManager sipxServiceManager = EasyMock.createMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxCallResolverService.BEAN_ID);
        EasyMock.expectLastCall().andReturn(sipxCallResolverService).anyTimes();
        out.setSipxServiceManager(sipxServiceManager);

        EasyMock.replay(serviceProvider, sipxServiceManager);

        CdrService cdrService = out.getCdrService();
        assertNotNull(cdrService);
        EasyMock.verify(serviceProvider, sipxServiceManager);
    }
}
