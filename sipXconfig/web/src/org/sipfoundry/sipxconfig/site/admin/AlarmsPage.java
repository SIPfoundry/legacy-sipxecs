/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.alarm.Alarm;
import org.sipfoundry.sipxconfig.admin.alarm.AlarmServer;
import org.sipfoundry.sipxconfig.common.AlarmContext;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.SelectMap;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class AlarmsPage extends BasePage implements PageBeginRenderListener {
    private static final String CONFIG_TAB = "configureAlarms";

    private static final String HISTORY_TAB = "historyAlarms";

    @InjectObject(value = "spring:alarmContextImpl")
    public abstract AlarmContext getAlarmContext();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    public abstract boolean isAdvanced();

    public abstract AlarmServer getAlarmServer();

    public abstract void setAlarmServer(AlarmServer server);

    public abstract List<Alarm> getAlarms();

    public abstract void setAlarms(List<Alarm> alarms);

    @Bean
    public abstract SelectMap getSelections();

    public abstract Alarm getCurrentRow();

    public abstract void setCurrentRow(Alarm alarm);

    @Persist
    @InitialValue(value = "literal:configureAlarms")
    public abstract String getTab();

    public Collection<String> getAvailableTabNames() {
        Collection<String> tabNames = new ArrayList<String>();
        tabNames.addAll(Arrays.asList(CONFIG_TAB, HISTORY_TAB));
        return tabNames;
    }

    public void pageBeginRender(PageEvent event) {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        AlarmServer alarmServer = getAlarmServer();
        if (alarmServer == null) {
            alarmServer = getAlarmContext().getAlarmServer();
            setAlarmServer(alarmServer);
        }

        List<Alarm> alarms = getAlarms();
        if (alarms == null) {
            alarms = getAlarmContext().getAlarmTypes(
                    getAlarmContext().getConfigDirectory() + "/alarms/sipXalarms-config.xml",
                    getAlarmContext().getAlarmsStringsDirectory() + "/sipXalarms-strings.xml");
            setAlarms(alarms);

            SelectMap selections = getSelections();
            for (Alarm alarm : alarms) {
                if (alarm.isEmailEnabled()) {
                    selections.setSelected(alarm, true);
                }
            }
        }
    }

    public void activate() {
        AlarmServer alarmServer = getAlarmServer();
        if (alarmServer.isEmailNotificationEnabled() && alarmServer.getContacts().isEmpty()) {
            getValidator().record(new UserException("&error.requiredEmail"), getMessages());
            return;
        }
        if (alarmServer.getContacts().size() > 5) {
            getValidator()
                    .record(new UserException("&error.maxNumberEmail"), getMessages());
            return;
        }

        getAlarmContext().deployAlarmConfiguration(getAlarmServer(), getAlarms());
    }
}
