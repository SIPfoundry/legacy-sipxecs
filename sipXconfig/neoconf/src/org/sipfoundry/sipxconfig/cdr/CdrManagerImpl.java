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

import java.io.IOException;
import java.io.Writer;
import java.net.MalformedURLException;
import java.net.URL;
import java.rmi.RemoteException;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.List;
import java.util.TimeZone;

import javax.xml.rpc.ServiceException;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.time.DateFormatUtils;
import org.apache.commons.lang.time.DateUtils;
import org.sipfoundry.sipxconfig.bulk.csv.CsvWriter;
import org.sipfoundry.sipxconfig.cdr.Cdr.Termination;
import org.sipfoundry.sipxconfig.common.UserException;
import org.springframework.dao.support.DataAccessUtils;
import org.springframework.jdbc.core.PreparedStatementCreator;
import org.springframework.jdbc.core.ResultReader;
import org.springframework.jdbc.core.RowCallbackHandler;
import org.springframework.jdbc.core.RowMapper;
import org.springframework.jdbc.core.SingleColumnRowMapper;
import org.springframework.jdbc.core.support.JdbcDaoSupport;

public class CdrManagerImpl extends JdbcDaoSupport implements CdrManager {
    static final String CALLEE_AOR = "callee_aor";
    static final String TERMINATION = "termination";
    static final String FAILURE_STATUS = "failure_status";
    static final String END_TIME = "end_time";
    static final String CONNECT_TIME = "connect_time";
    static final String START_TIME = "start_time";
    static final String CALLER_AOR = "caller_aor";

    private String m_cdrAgentHost;
    private int m_cdrAgentPort;
    private int m_csvLimit;

    /**
     * CDRs database at the moment is using 'timestamp' type to store UTC time. Postgres
     * 'timestamp' does not store any time zone information and JDBC driver for postgres would
     * interpret as local time. We pass TimeZone explicitely to force interpreting zoneless
     * timestamp as UTC timestamps.
     */
    private TimeZone m_tz = DateUtils.UTC_TIME_ZONE;

    public List<Cdr> getCdrs(Date from, Date to) {
        return getCdrs(from, to, new CdrSearch());
    }

    public List<Cdr> getCdrs(Date from, Date to, CdrSearch search) {
        return getCdrs(from, to, search, 0, 0);
    }

    public List<Cdr> getCdrs(Date from, Date to, CdrSearch search, int limit, int offset) {
        CdrsStatementCreator psc = new SelectAll(from, to, search, m_tz, limit, offset);
        CdrsResultReader resultReader = new CdrsResultReader(m_tz);
        return getJdbcTemplate().query(psc, resultReader);
    }

    /**
     * Current implementation only dumps at most m_csvLimit CDRs. This limitation is necessary due
     * to limitations of URLConnection used to download exported data to the client system.
     * 
     * See: http://bugs.sun.com/bugdatabase/view_bug.do?bug_id=4212479
     * http://bugs.sun.com/bugdatabase/view_bug.do?bug_id=5026745
     * 
     * If we had direct access to that connection we could try calling "setChunkedStreamingMode"
     * on it.
     */
    public void dumpCdrs(Writer writer, Date from, Date to, CdrSearch search) throws IOException {
        CdrsStatementCreator psc = new SelectAll(from, to, search, m_tz, m_csvLimit, 0);
        CdrsCsvWriter resultReader = new CdrsCsvWriter(writer, m_tz);
        try {
            getJdbcTemplate().query(psc, resultReader);
        } catch (RuntimeException e) {
            // unwrap IOException that might happen during reading DB
            if (e.getCause() instanceof IOException) {
                throw (IOException) e.getCause();
            }
            throw e;
        }
    }

    public int getCdrCount(Date from, Date to, CdrSearch search) {
        CdrsStatementCreator psc = new SelectCount(from, to, search, m_tz);
        RowMapper rowMapper = new SingleColumnRowMapper(Integer.class);
        List results = getJdbcTemplate().query(psc, rowMapper);
        return (Integer) DataAccessUtils.requiredUniqueResult(results);
    }

    public List<Cdr> getActiveCalls() {
        try {
            CdrService cdrService = getCdrService();
            ActiveCall[] activeCalls = cdrService.getActiveCalls();
            List<Cdr> cdrs = new ArrayList<Cdr>(activeCalls.length);
            for (ActiveCall call : activeCalls) {
                ActiveCallCdr cdr = new ActiveCallCdr();
                cdr.setCallerAor(call.getFrom());
                cdr.setCalleeAor(call.getTo());
                cdr.setStartTime(call.getStart_time().getTime());
                cdr.setDuration(call.getDuration());
                cdrs.add(cdr);
            }
            return cdrs;
        } catch (RemoteException e) {
            throw new UserException(e);
        }
    }

    public CdrService getCdrService() {
        try {
            URL url = new URL("http", m_cdrAgentHost, m_cdrAgentPort, StringUtils.EMPTY);
            return new CdrImplServiceLocator().getCdrService(url);
        } catch (ServiceException e) {
            throw new UserException(e);
        } catch (MalformedURLException e) {
            throw new UserException(e);
        }
    }

    public void setCdrAgentHost(String cdrAgentHost) {
        m_cdrAgentHost = cdrAgentHost;
    }

    public void setCdrAgentPort(int cdrAgentPort) {
        m_cdrAgentPort = cdrAgentPort;
    }

    public void setCsvLimit(int csvLimit) {
        m_csvLimit = csvLimit;
    }

    abstract static class CdrsStatementCreator implements PreparedStatementCreator {
        private static final String FROM = " FROM cdrs WHERE (? <= start_time) AND (start_time <= ?)";
        private static final String LIMIT = " LIMIT ? OFFSET ?";

        private Timestamp m_from;
        private Timestamp m_to;
        private CdrSearch m_search;
        private int m_limit;
        private int m_offset;
        private Calendar m_calendar;

        public CdrsStatementCreator(Date from, Date to, CdrSearch search, TimeZone tz) {
            this(from, to, search, tz, 0, 0);
        }

        public CdrsStatementCreator(Date from, Date to, CdrSearch search, TimeZone tz, int limit, int offset) {
            m_calendar = Calendar.getInstance(tz);
            long fromMillis = from != null ? from.getTime() : 0;
            m_from = new Timestamp(fromMillis);
            long toMillis = to != null ? to.getTime() : System.currentTimeMillis();
            m_to = new Timestamp(toMillis);
            m_search = search;
            m_limit = limit;
            m_offset = offset;
        }

        public PreparedStatement createPreparedStatement(Connection con) throws SQLException {
            StringBuilder sql = new StringBuilder(getSelectSql());
            sql.append(FROM);
            m_search.appendGetSql(sql);
            appendOrderBySql(sql);
            if (m_limit > 0) {
                sql.append(LIMIT);
            }
            PreparedStatement ps = con.prepareStatement(sql.toString());
            ps.setTimestamp(1, m_from, m_calendar);
            ps.setTimestamp(2, m_to, m_calendar);
            if (m_limit > 0) {
                ps.setInt(3, m_limit);
                ps.setInt(4, m_offset);
            }
            return ps;
        }

        public abstract String getSelectSql();

        protected void appendOrderBySql(StringBuilder sql) {
            m_search.appendOrderBySql(sql);
        }
    }

    static class SelectAll extends CdrsStatementCreator {
        public SelectAll(Date from, Date to, CdrSearch search, TimeZone tz, int limit, int offset) {
            super(from, to, search, tz, limit, offset);
        }

        public SelectAll(Date from, Date to, CdrSearch search, TimeZone tz) {
            super(from, to, search, tz);
        }

        @Override
        public String getSelectSql() {
            return "SELECT *";
        }
    }

    static class SelectCount extends CdrsStatementCreator {

        public SelectCount(Date from, Date to, CdrSearch search, TimeZone tz) {
            super(from, to, search, tz);
        }

        @Override
        public String getSelectSql() {
            return "SELECT COUNT(id)";
        }

        @Override
        protected void appendOrderBySql(StringBuilder sql) {
            // no ordering when selecting COUNT
        }
    }

    static class CdrsResultReader implements ResultReader {
        private List<Cdr> m_cdrs = new ArrayList<Cdr>();

        private Calendar m_calendar;

        public CdrsResultReader(TimeZone tz) {
            m_calendar = Calendar.getInstance(tz);
        }

        public List<Cdr> getResults() {
            return m_cdrs;
        }

        public void processRow(ResultSet rs) throws SQLException {
            Cdr cdr = new Cdr();
            cdr.setCalleeAor(rs.getString(CALLEE_AOR));
            cdr.setCallerAor(rs.getString(CALLER_AOR));
            Date startTime = rs.getTimestamp(START_TIME, m_calendar);
            cdr.setStartTime(startTime);
            Date connectTime = rs.getTimestamp(CONNECT_TIME, m_calendar);
            cdr.setConnectTime(connectTime);
            cdr.setEndTime(rs.getTimestamp(END_TIME, m_calendar));
            cdr.setFailureStatus(rs.getInt(FAILURE_STATUS));
            String termination = rs.getString(TERMINATION);
            cdr.setTermination(Termination.fromString(termination));
            m_cdrs.add(cdr);
        }
    }

    static class ColumnInfo {
        /** List of fields that will be exported to CDR */
        static final String[] FIELDS = {
            CALLEE_AOR, CALLER_AOR, START_TIME, CONNECT_TIME, END_TIME, FAILURE_STATUS, TERMINATION,
        };
        static final boolean[] TIME_FIELDS = {
            false, false, true, true, true, false, false
        };

        private int m_index;
        private boolean m_timestamp;

        public ColumnInfo(ResultSet rs, int i) throws SQLException {
            m_index = rs.findColumn(FIELDS[i]);
            m_timestamp = TIME_FIELDS[i];
        }

        public int getIndex() {
            return m_index;
        }

        public boolean isTimestamp() {
            return m_timestamp;
        }

        public static ColumnInfo[] create(ResultSet rs) throws SQLException {
            ColumnInfo[] fields = new ColumnInfo[FIELDS.length];
            for (int i = 0; i < fields.length; i++) {
                ColumnInfo ci = new ColumnInfo(rs, i);
                fields[i] = ci;
            }
            return fields;
        }
    }

    static class CdrsCsvWriter implements RowCallbackHandler {
        private CsvWriter m_csv;

        private ColumnInfo[] m_columns;

        private Calendar m_calendar = Calendar.getInstance(DateUtils.UTC_TIME_ZONE);

        public CdrsCsvWriter(Writer writer, TimeZone tz) throws IOException {
            m_calendar = Calendar.getInstance(tz);
            m_csv = new CsvWriter(writer);
            m_csv.write(ColumnInfo.FIELDS, false);
        }

        public void processRow(ResultSet rs) throws SQLException {
            if (m_columns == null) {
                m_columns = ColumnInfo.create(rs);
            }
            String[] row = new String[m_columns.length];
            for (int i = 0; i < row.length; i++) {
                if (m_columns[i].isTimestamp()) {
                    Date date = rs.getTimestamp(m_columns[i].getIndex(), m_calendar);
                    if (date != null) {
                        row[i] = DateFormatUtils.ISO_DATETIME_TIME_ZONE_FORMAT.format(date);
                    }

                } else {
                    row[i] = rs.getString(m_columns[i].getIndex());
                }
            }
            try {
                m_csv.write(row, true);
            } catch (IOException e) {
                new RuntimeException(e);
            }
        }
    }
}
