/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acd.stats;

import java.io.File;
import java.io.IOException;
import java.io.Writer;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Date;
import java.util.List;
import java.util.Locale;
import java.util.Map;

import org.apache.commons.beanutils.locale.LocaleConvertUtils;
import org.apache.commons.lang.time.DateUtils;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.bulk.csv.CsvWriter;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.jdbc.core.ColumnMapRowMapper;
import org.springframework.jdbc.core.PreparedStatementCreator;
import org.springframework.jdbc.core.RowMapperResultSetExtractor;
import org.springframework.jdbc.core.support.JdbcDaoSupport;
import org.springframework.jdbc.support.rowset.SqlRowSet;
import org.springframework.jdbc.support.rowset.SqlRowSetMetaData;

public class AcdHistoricalStatsImpl extends JdbcDaoSupport implements AcdHistoricalStats,
        BeanFactoryAware, DaoEventListener {

    private ListableBeanFactory m_factory;
    private String m_reportScript;
    private Boolean m_enabled;
    private String m_exportDatePattern = "EEE, d MMM yyyy HH:mm:ss Z";
    private SipxReplicationContext m_sipxReplicationContext;
    private AcdHistoricalConfigurationFile m_acdHistoricalConfiguration;

    public List<String> getReports() {
        List<String> reports = Arrays.asList(m_factory
                .getBeanNamesForType(AcdHistoricalReport.class));
        return reports;
    }

    public void dumpReport(Writer writer, List<Map<String, Object>> reportData, Locale locale)
        throws IOException {

        // nothing ACD specific here, could be reused

        if (reportData.size() == 0) {
            return;
        }

        // column names
        CsvWriter csv = new CsvWriter(writer);
        Map<String, Object> row0 = reportData.get(0);
        csv.write(row0.keySet().toArray(new String[0]), false);

        // rows
        for (Map<String, Object> record : reportData) {
            Object[] recordData = record.values().toArray();
            String[] recordDataStrings = new String[recordData.length];
            for (int i = 0; i < recordData.length; i++) {

                // convert date field (if present) to appropriate locale
                if (recordData[i] instanceof Date) {
                    recordDataStrings[i] = LocaleConvertUtils.convert(recordData[i], locale,
                            m_exportDatePattern);
                } else {
                    recordDataStrings[i] = String.valueOf(recordData[i]);
                }
            }

            csv.write(recordDataStrings, false);
        }
    }

    /**
     * Default value is determined is report cron script is installed locally. You can enable
     * historic stats and configure it to talk to a remote database.
     */
    public boolean isEnabled() {
        if (m_enabled != null) {
            return m_enabled;
        }

        return isReportsInstalledLocally();
    }

    public void setEnabled(boolean enabled) {
        m_enabled = new Boolean(enabled);
    }

    boolean isReportsInstalledLocally() {
        return getReportScript() != null && new File(getReportScript()).exists();
    }

    public List<String> getReportFields(String reportName) {
        AcdHistoricalReport report = (AcdHistoricalReport) m_factory.getBean(reportName);

        Object[] sqlParameters = new Object[] {
            new Date(0), new Date(0), ""
        };
        SqlRowSet emptySet = getJdbcTemplate().queryForRowSet(report.getQuery() + " limit 0",
                sqlParameters);
        SqlRowSetMetaData meta = emptySet.getMetaData();
        List<String> names = Arrays.asList(meta.getColumnNames());
        return names;
    }

    public List<Map<String, Object>> getReport(String reportName, Date startTime, Date endTime, Location location) {
        AcdHistoricalReport report = (AcdHistoricalReport) m_factory.getBean(reportName);
        ColumnMapRowMapper columnMapper = new ColumnTransformer();
        RowMapperResultSetExtractor rowReader = new RowMapperResultSetExtractor(columnMapper);
        String locationFqdn = location.getFqdn();
        ReportStatement statement = null;
        if (locationFqdn != null) {
            statement = new ReportStatement(report.getQuery(), startTime, endTime, locationFqdn);
        } else {
            return new ArrayList<Map<String, Object>>();
        }

        return (List<Map<String, Object>>) getJdbcTemplate().query(statement, rowReader);
    }

    static class ColumnTransformer extends ColumnMapRowMapper {
        static final Calendar UTC = Calendar.getInstance(DateUtils.UTC_TIME_ZONE);
        static {
            UTC.setTimeInMillis(0);
        }

        protected Object getColumnValue(ResultSet rs, int index) throws SQLException {
            Object obj = rs.getObject(index);
            if (obj != null && obj instanceof java.sql.Timestamp) {
                // Timestamp is invalid, force timezone to utc
                obj = rs.getTimestamp(index, UTC);
            } else {
                obj = super.getColumnValue(rs, index);
            }

            return obj;
        }
    }

    static class ReportStatement implements PreparedStatementCreator {
        private String m_sql;
        private Timestamp m_from;
        private Timestamp m_to;
        private String m_locationFqdn;

        ReportStatement(String sql, Date from, Date to, String locationFqdn) {
            m_sql = sql;
            long fromMillis = from != null ? from.getTime() : 0;
            m_from = new Timestamp(fromMillis);
            long toMillis = to != null ? to.getTime() : System.currentTimeMillis();
            m_to = new Timestamp(toMillis);
            m_locationFqdn = locationFqdn;
        }

        public PreparedStatement createPreparedStatement(Connection con) throws SQLException {
            PreparedStatement ps = con.prepareStatement(m_sql);
            Calendar calendar = Calendar.getInstance(DateUtils.UTC_TIME_ZONE);
            ps.setTimestamp(1, m_from, calendar);
            ps.setTimestamp(2, m_to, calendar);
            ps.setString(3, m_locationFqdn);
            return ps;
        }
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_factory = (ListableBeanFactory) beanFactory;
    }

    public void setReportScript(String reportScript) {
        m_reportScript = reportScript;
    }

    public String getReportScript() {
        return m_reportScript;
    }

    /**
     * As defined http://java.sun.com/j2se/1.5.0/docs/api/java/text/SimpleDateFormat.html
     */
    public void setExportDatePattern(String exportDatePattern) {
        m_exportDatePattern = exportDatePattern;
    }

    public void onDelete(Object entity) {
        if (entity instanceof Location && isEnabled()) {
            m_sipxReplicationContext.replicate(m_acdHistoricalConfiguration);
        }
    }

    public void onSave(Object entity) {
        if (entity instanceof Location && isEnabled()) {
            m_sipxReplicationContext.replicate(m_acdHistoricalConfiguration);
        }
    }

    public void setSipxReplicationContext(SipxReplicationContext sipxReplicationContext) {
        m_sipxReplicationContext = sipxReplicationContext;
    }

    public void setAcdHistoricalConfiguration(
            AcdHistoricalConfigurationFile acdHistoricalConfiguration) {
        m_acdHistoricalConfiguration = acdHistoricalConfiguration;
    }

}
