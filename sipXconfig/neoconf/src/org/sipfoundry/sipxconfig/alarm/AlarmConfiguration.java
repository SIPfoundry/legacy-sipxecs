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
import java.io.Writer;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.apache.velocity.VelocityContext;
import org.apache.velocity.app.VelocityEngine;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.YamlConfiguration;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.springframework.beans.factory.annotation.Required;

public class AlarmConfiguration implements ConfigProvider {
    private AlarmServerManager m_alarmServerManager;
    private VelocityEngine m_velocityEngine;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(Alarms.FEATURE)) {
            return;
        }

        List<Alarm> alarms = m_alarmServerManager.getAlarms();
        AlarmServer alarmServer = m_alarmServerManager.getAlarmServer();
        String host = m_alarmServerManager.getHost();
        Location[] locations = manager.getLocationManager().getLocations();
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);

            Writer yml = new FileWriter(new File(dir, "alarms.yaml"));
            try {
                writeAlarms(yml, alarms);
            } finally {
                IOUtils.closeQuietly(yml);
            }

            Writer alarmsGroupsXml = new FileWriter(new File(dir, "alarm-groups.xml"));
            try {
                // alters groups collection so get fresh copy for each location. clone could be ok too.
                List<AlarmGroup> groups = m_alarmServerManager.getAlarmGroups();
                writeAlarmGroupsXml(alarmsGroupsXml, groups);
            } finally {
                IOUtils.closeQuietly(alarmsGroupsXml);
            }

            Writer alarmsConfigXml = new FileWriter(new File(dir, "alarm-config.xml"));
            try {
                writeAlarmConfigXml(alarmsConfigXml, alarmServer, host);
            } finally {
                IOUtils.closeQuietly(alarmsConfigXml);
            }
        }
    }

    void writeAlarms(Writer w, List<Alarm> alarms) throws IOException {
        YamlConfiguration c = new YamlConfiguration(w);
        for (Alarm a : alarms) {
            c.startStruct(a.getAlarmDefinition().getId());
            c.write(":groupName", a.getGroupName());
            c.write(":minThreshold", a.getMinThreshold());
            c.endStruct();
        }
    }

    void writeAlarmGroupsXml(Writer wtr, List<AlarmGroup> groups) throws IOException {
        VelocityContext context = new VelocityContext();
        setContactAddresses(groups);
        context.put("groups", groups);
        write(wtr, context, "alarms/alarm-groups.vm");
    }

    void writeAlarmConfigXml(Writer wtr, AlarmServer server, String host) throws IOException {
        VelocityContext context = new VelocityContext();
        context.put("enabled", server.isAlarmNotificationEnabled());
        String fromEmailAddress = defaultIfEmpty(server.getFromEmailAddress(), "postmaster@" + host);
        context.put("fromEmailAddress", fromEmailAddress);
        context.put("hostName", host);
        write(wtr, context, "commserver/alarm-config.vm");
    }

    void write(Writer wtr, VelocityContext context, String template) throws IOException {
        try {
            m_velocityEngine.mergeTemplate(template, context, wtr);
        } catch (Exception e) {
            throw new IOException(e);
        }
        wtr.flush();
    }

    void setContactAddresses(List<AlarmGroup> alarmGroups) {
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
    }

    @Required
    public void setVelocityEngine(VelocityEngine velocityEngine) {
        m_velocityEngine = velocityEngine;
    }

    public void setAlarmServerManager(AlarmServerManager alarmServerManager) {
        m_alarmServerManager = alarmServerManager;
    }
}
