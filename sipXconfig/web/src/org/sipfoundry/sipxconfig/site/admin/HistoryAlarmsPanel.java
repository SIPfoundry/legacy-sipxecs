/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.List;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.contrib.table.model.ITableColumn;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.apache.tapestry.services.ExpressionEvaluator;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmEvent;
import org.sipfoundry.sipxconfig.admin.monitoring.MonitoringContext;
import org.sipfoundry.sipxconfig.common.AlarmContext;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.cdr.CdrHistory;

public abstract class HistoryAlarmsPanel extends BaseComponent implements PageBeginRenderListener {
    private static final String CLIENT = "client";

    @InjectObject(value = "spring:alarmContextImpl")
    public abstract AlarmContext getAlarmContext();

    @InjectObject("spring:monitoringContext")
    public abstract MonitoringContext getMonitoringContext();

    @InjectObject(value = "service:tapestry.ognl.ExpressionEvaluator")
    public abstract ExpressionEvaluator getExpressionEvaluator();

    public abstract void setHost(String host);

    @Persist(value = CLIENT)
    public abstract String getHost();

    public abstract IPropertySelectionModel getHostModel();

    public abstract void setHostModel(IPropertySelectionModel model);

    @Persist(value = CLIENT)
    public abstract Date getStartDate();

    public abstract void setStartDate(Date date);

    @Persist(value = CLIENT)
    public abstract Date getEndDate();

    public abstract void setEndDate(Date date);

    public abstract AlarmEvent getCurrentRow();

    public abstract void setCurrentRow(AlarmEvent alarmEvent);

    public abstract List<AlarmEvent> getAlarmEvents();

    public abstract void setAlarmEvents(List<AlarmEvent> alarmEvents);

    @Persist
    public abstract HistoryAlarmsTableModel getHistoryAlarmsTableModel();

    public abstract void setHistoryAlarmsTableModel(HistoryAlarmsTableModel tableModel);

    public ITableColumn getDate() {
        return TapestryUtils
                .createDateColumn("date", getMessages(), getExpressionEvaluator(), getPage().getLocale());
    }

    public void pageBeginRender(PageEvent event) {
        if (getHostModel() == null) {
            List<String> hosts = getMonitoringContext().getAvailableHosts();
            Collections.sort(hosts);
            StringPropertySelectionModel model = new StringPropertySelectionModel(hosts.toArray(new String[hosts
                    .size()]));
            setHostModel(model);
            if (!hosts.contains(getHost()) && getHostModel().getOptionCount() > 0) {
                setHost(getHostModel().getLabel(0));
            }
        }

        if (getHost() == null && getHostModel().getOptionCount() > 0) {
            setHost(getHostModel().getLabel(0));
        }

        if (getEndDate() == null) {
            setEndDate(CdrHistory.getDefaultEndTime());
        }

        if (getStartDate() == null) {
            Date startTime = CdrHistory.getDefaultStartTime(getEndDate());
            setStartDate(startTime);
        }

        if (getAlarmEvents() == null) {
            setAlarmEvents(new ArrayList<AlarmEvent>());
        }

        if (getHistoryAlarmsTableModel() == null) {
            HistoryAlarmsTableModel tableModel = new HistoryAlarmsTableModel();
            tableModel.setAlarmContext(getAlarmContext());
            setHistoryAlarmsTableModel(tableModel);
        }
    }

    public void retrieveLogs() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        if (getStartDate().after(getEndDate())) {
            TapestryUtils.getValidator(getPage()).record(
                    new ValidatorException(getMessages().getMessage("message.invalidDates")));
            return;
        }

        getAlarmContext().reloadAlarms();

        getHistoryAlarmsTableModel().setHost(getHost());
        getHistoryAlarmsTableModel().setStartDate(getStartDate());
        getHistoryAlarmsTableModel().setEndDate(getEndDate());
    }
}
