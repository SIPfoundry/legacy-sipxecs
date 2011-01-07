/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.acd;

import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hivemind.Messages;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.contrib.table.model.ITableColumnModel;
import org.apache.tapestry.contrib.table.model.ITableRendererSource;
import org.apache.tapestry.contrib.table.model.simple.SimpleTableColumn;
import org.apache.tapestry.contrib.table.model.simple.SimpleTableColumnModel;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.web.WebResponse;
import org.postgresql.util.PGInterval;
import org.sipfoundry.sipxconfig.acd.AcdContext;
import org.sipfoundry.sipxconfig.acd.AcdServer;
import org.sipfoundry.sipxconfig.acd.stats.AcdHistoricalStats;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.common.SqlInterval;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.ObjectSelectionModel;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.cdr.CdrHistory;
import org.sipfoundry.sipxconfig.site.common.DefaultTableValueRendererSource;


public abstract class AcdHistoryPage extends SipxBasePage implements PageBeginRenderListener {
    public static final String PAGE = "acd/AcdHistoryPage";
    private static final Log LOG = LogFactory.getLog(AcdHistoryPage.class);

    @InjectObject(value = "spring:acdHistoricalStats")
    public abstract AcdHistoricalStats getAcdHistoricalStats();

    public abstract int getReportIndex();

    public abstract Map<String, Object> getRow();

    @InjectObject(value = "service:tapestry.globals.WebResponse")
    public abstract WebResponse getResponse();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    public abstract String getReportName();

    public abstract void setReportName(String reportName);

    public abstract Object getAvailableReportsIndexItem();

    @Persist
    public abstract Date getStartTime();
    public abstract void setStartTime(Date startTime);

    @Persist
    public abstract Date getEndTime();
    public abstract void setEndTime(Date endTime);

    @Persist
    public abstract int getCurrentLocationId();
    public abstract void setCurrentLocationId(int id);

    @InjectObject(value = "spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @InjectObject(value = "spring:acdContext")
    public abstract AcdContext getAcdContext();

    @Persist
    public abstract LocationSelectionModel getLocationSelectionModel();
    public abstract void setLocationSelectionModel(LocationSelectionModel locationSelectionModel);

    public void showReport(String reportName) {
        setReportName(reportName);
    }

    public void pageBeginRender(PageEvent event) {
        String report = getReportName();
        if (report == null) {
            report = getAcdHistoricalStats().getReports().get(0);
            setReportName(report);
        }

        if (getEndTime() == null) {
            setEndTime(CdrHistory.getDefaultEndTime());
        }

        if (getStartTime() == null) {
            Date startTime = CdrHistory.getDefaultStartTime(getEndTime());
            setStartTime(startTime);
        }

        if (getStartTime().after(getEndTime())) {
            getValidator().record(new UserException("&message.invalidDates"), getMessages());
        }

        setLocationSelectionModel(new LocationSelectionModel(getAcdContext().getServers()));
    }

    public List<Map<String, Object>>getRows() {
        int locationId = getCurrentLocationId();
        if (locationId == 0) {
            locationId = (Integer) getLocationSelectionModel().getOption(0);
        }
        Location location = getLocationsManager().getLocation(locationId);
        return getAcdHistoricalStats().getReport(getReportName(), getStartTime(), getEndTime(), location);
    }

    public ITableColumnModel getColumns() {
        ITableRendererSource valueRenderer = new DefaultTableValueRendererSource();
        List<String> names = getAcdHistoricalStats().getReportFields(getReportName());
        MapTableColumn[] columns = new MapTableColumn[names.size()];
        for (int i = 0; i < columns.length; i++) {
            columns[i] = new MapTableColumn(getMessages(), getReportName(), names.get(i));
            columns[i].setValueRendererSource(valueRenderer);
        }

        return new SimpleTableColumnModel(columns);
    }

    public void export() {
        try {
            PrintWriter writer = TapestryUtils.getCsvExportWriter(getResponse(), getReportName() + ".csv");
            getAcdHistoricalStats().dumpReport(writer, getRows(), getPage().getLocale());
            writer.close();
        } catch (IOException e) {
            LOG.error("Error during ACD History report export " + getReportName(), e);
        }
    }

    public static class LocationSelectionModel extends ObjectSelectionModel {
        LocationSelectionModel(List<AcdServer> acdServers) {
            List<Location> locations = new ArrayList<Location>();
            for (int i = 0; i < acdServers.size(); i++) {
                locations.add(acdServers.get(i).getLocation());
            }

            setCollection(locations);
            setLabelExpression("fqdn");
            setValueExpression("id");
        }
    }

    public boolean isAcdServerPresent() {
        return 0 < getAcdContext().getServers().size();
    }
}

/**
 *  Get the column header from localized value from key built using what is effectively the
 * report name and sql result's column name
 */
class MapTableColumn extends SimpleTableColumn {
    private final Messages m_messages;
    private final String m_report;

    public MapTableColumn(Messages messages, String report, String columnName) {
        super(columnName);
        m_messages = messages;
        m_report = report;
        setSortable(true);
    }

    @Override
    public Object getColumnValue(Object objRow) {
        String columnName = getColumnName();
        Object ovalue = ((Map<String, Object>) objRow).get(columnName);
        Object value = ovalue;
        if (ovalue instanceof PGInterval) {
            // at least this is sortable
            value = new SqlInterval((PGInterval) ovalue);
        } else if (columnName.equals("agent_uri") || columnName.equals("queue_uri")) {
            if (ovalue != null) {
                value = SipUri.extractUser((String) ovalue);
            }
        }

        return value;
    }

    @Override
    public String getDisplayName() {
        return m_messages.getMessage(m_report + "." + getColumnName());
    }
}
