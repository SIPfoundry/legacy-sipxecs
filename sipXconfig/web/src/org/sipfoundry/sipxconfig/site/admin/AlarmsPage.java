/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.util.ArrayList;
import java.util.List;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.sipfoundry.sipxconfig.admin.alarm.Alarm;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmGroup;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmServer;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmServerManager;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class AlarmsPage extends SipxBasePage implements PageBeginRenderListener {

    @InjectObject("spring:alarmServerManager")
    public abstract AlarmServerManager getAlarmServerManager();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Bean
    public abstract SelectMap getSelections();

    public abstract void setGroups(List<AlarmGroup> groups);

    public abstract IPropertySelectionModel getAlarmGroupModel();

    public abstract void setAlarmGroupModel(IPropertySelectionModel model);

    @Persist
    public abstract boolean isAdvanced();

    public abstract AlarmServer getAlarmServer();

    public abstract void setAlarmServer(AlarmServer server);

    public abstract List<Alarm> getAlarms();

    public abstract void setAlarms(List<Alarm> alarms);

    @Persist
    @InitialValue("literal:configureAlarms")
    public abstract String getTab();

    private IPropertySelectionModel createAlarmGroupModel(List<AlarmGroup> groups) {
        List<String> groupStrings = new ArrayList<String>();
        for (AlarmGroup group : groups) {
            groupStrings.add(group.getName());
        }

        // Create the drop-down list of alarm groups
        AlarmGroupSelectionModel typeModel = new AlarmGroupSelectionModel();
        typeModel.setGroups(groupStrings);

        return typeModel;
    }

    public void pageBeginRender(PageEvent event) {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        AlarmServer alarmServer = getAlarmServer();
        if (alarmServer == null) {
            alarmServer = getAlarmServerManager().getAlarmServer();
            setAlarmServer(alarmServer);
        }

        List<Alarm> alarms = getAlarms();
        if (alarms == null) {
            alarms = getAlarmServerManager().getAlarmTypes();
            setAlarms(alarms);

            SelectMap selections = getSelections();
            for (Alarm alarm : alarms) {
                if (alarm.getGroupName().equals("disabled")) {
                    selections.setSelected(alarm, true);
                }
            }
        }

        List<AlarmGroup> groups = getAlarmServerManager().getAlarmGroups();
        setGroups(groups);
        if (getAlarmGroupModel() == null) {
            setAlarmGroupModel(createAlarmGroupModel(groups));
        }
    }

    public void activate() {
        getAlarmServerManager().deployAlarmConfiguration(getAlarmServer(), getAlarms());
    }
}
