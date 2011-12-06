/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.alarm;

import static org.apache.commons.lang.StringUtils.defaultIfEmpty;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.springframework.beans.factory.annotation.Required;

public class AlarmConfiguration implements ConfigProvider {
    private AlarmServerManager m_alarmServerManager;
    private VelocityEngine m_velocityEngine;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (request.applies(Alarms.FEATURE)) {
            if (manager.getFeatureManager().isFeatureEnabled(Alarms.FEATURE)) {
                List<Alarm> alarms = m_alarmServerManager.getAlarmTypes();
                List<AlarmGroup> groups = getAlarmGroups();
                Location[] locations = manager.getLocationManager().getLocations();
                for (Location location : locations) {
                    File dir = manager.getLocationDataDirectory(location);

                    VelocityContext context = new VelocityContext();
                    new AlarmsXml().context(context, alarms);
                    write(context, new File(dir, "alarms.xml"), "alarms/alarms.vm");

                    context = new VelocityContext();
                    new AlarmGroupsXml().context(context, groups);
                    write(context, new File(dir, "alarm-groups.xml"), "alarms/alarm-groups.vm");

                    context = new VelocityContext();
                    new AlarmConfigXml().context(context);
                    write(context, new File(dir, "alarm-config.xml"), "commserver/alarm-config.vm");
                }
            }
        }
    }

    void write(VelocityContext context, File file, String template) throws IOException {
        FileWriter wtr = new FileWriter(file);
        IOUtils.closeQuietly(wtr);
        try {
            m_velocityEngine.mergeTemplate(template, context, wtr);
        } catch (Exception e) {
            throw new IOException(e);
        }
        wtr.flush();
    }

    List<AlarmGroup> getAlarmGroups() {
        List<AlarmGroup> alarmGroups = m_alarmServerManager.getAlarmGroups();
        for (AlarmGroup group : alarmGroups) {
            List<String> userEmailAddresses = new ArrayList<String>();
            Set<User> users = group.getUsers();
            for (User user : users) {
                String emailAddress = user.getEmailAddress();
                if (emailAddress != null) {
                    userEmailAddresses.add(emailAddress);
                }
                String altEmailAddress = user.getAlternateEmailAddress();
                if (altEmailAddress != null) {
                    userEmailAddresses.add(altEmailAddress);
                }
            }

            group.setUserEmailAddresses(userEmailAddresses);
        }
        return alarmGroups;
    }

    class AlarmsXml {
        void context(VelocityContext context, List<Alarm> alarms) {
            context.put("alarms", alarms);
        }
    }

    class AlarmGroupsXml {
        void context(VelocityContext context, List<AlarmGroup> groups) {
            context.put("groups", groups);
        }
    }

    class AlarmConfigXml {
        void context(VelocityContext context) {
            AlarmServer alarmServer = m_alarmServerManager.getAlarmServer();
            String host = m_alarmServerManager.getHost();
            context.put("enabled", alarmServer.isAlarmNotificationEnabled());
            String fromEmailAddress = defaultIfEmpty(alarmServer.getFromEmailAddress(), "postmaster@" + host);
            context.put("fromEmailAddress", fromEmailAddress);
            context.put("hostName", m_alarmServerManager.getHost());
        }
    }

    @Required
    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }
}
