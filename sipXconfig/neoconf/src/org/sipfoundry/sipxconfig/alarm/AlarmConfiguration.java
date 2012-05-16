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
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.Predicate;
import org.apache.commons.collections.Transformer;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.YamlConfiguration;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.mail.MailManager;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;

public class AlarmConfiguration implements ConfigProvider {
    private AlarmServerManager m_alarmServerManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(Alarms.FEATURE, SnmpManager.FEATURE)) {
            return;
        }

        List<Alarm> alarms = m_alarmServerManager.getAlarms();
        Location[] locations = manager.getLocationManager().getLocations();
        AlarmServer alarmServer = m_alarmServerManager.getAlarmServer();
        String host = m_alarmServerManager.getHost();
        List<AlarmGroup> groups = m_alarmServerManager.getAlarmGroups();
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            Writer yml = new FileWriter(new File(dir, "alarms.yaml"));
            try {
                writeAlarms(yml, alarms);
            } finally {
                IOUtils.closeQuietly(yml);
            }

            Address smtp = manager.getAddressManager().getSingleAddress(MailManager.SMTP, location);
            Writer wtr = new FileWriter(new File(manager.getGlobalDataDirectory(), "sipxtrap-handler.yaml"));
            try {
                writeAlarmHandler(wtr, alarms, groups, alarmServer, host, smtp);
            } catch (IOException e) {
                IOUtils.closeQuietly(wtr);
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

    void writeAlarmHandler(Writer wtr, List<Alarm> alarms, List<AlarmGroup> groups, AlarmServer server, String host,
            Address smtp) throws IOException {
        YamlConfiguration c = new YamlConfiguration(wtr);
        c.startStruct("email");
        for (AlarmGroup g : groups) {
            Set<String> emails = new HashSet<String>();
            emails.addAll(g.getContactEmailAddresses());
            emails.addAll(g.getContactSmsAddresses());
            Set<User> users = g.getUsers();
            if (users != null) {
                for (User user : users) {
                    String email = user.getEmailAddress();
                    if (StringUtils.isNotBlank(email)) {
                        emails.add(email);
                    }
                    String altEmailAddress = user.getAlternateEmailAddress();
                    if (altEmailAddress != null) {
                        emails.add(altEmailAddress);
                    }
                }
            }
            c.writeInlineArray(g.getName(), emails);
        }
        c.endStruct();

        c.startStruct("alarm");
        for (final Alarm a : alarms) {
            Collection< ? > byGroup = CollectionUtils.select(groups, new Predicate() {
                public boolean evaluate(Object o) {
                    return a.getGroupName().equals(((AlarmGroup) o).getName());
                }
            });
            Collection< ? > groupIds = CollectionUtils.collect(byGroup, new Transformer() {
                public Object transform(Object o) {
                    return ((AlarmGroup) o).getName();
                }
            });
            if (!groupIds.isEmpty()) {
                c.writeInlineArray(a.getAlarmDefinition().getId(), groupIds);
            }
        }
        c.endStruct();

        String fromEmailAddress = defaultIfEmpty(server.getFromEmailAddress(), "postmaster@" + host);
        c.write("from", fromEmailAddress);
        c.write("smtp", smtp);
    }

    public void setAlarmServerManager(AlarmServerManager alarmServerManager) {
        m_alarmServerManager = alarmServerManager;
    }
}
