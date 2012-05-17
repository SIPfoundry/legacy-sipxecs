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

import static java.lang.String.format;
import static org.apache.commons.lang.StringUtils.defaultIfEmpty;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.cfgmgt.CfengineModuleConfiguration;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.YamlConfiguration;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.mail.MailManager;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;
import org.springframework.context.MessageSource;

public class AlarmConfiguration implements ConfigProvider {
    private AlarmServerManager m_alarmServerManager;
    private MessageSource m_messageSource;

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
        List<AlarmTrapReceiver> fwd = m_alarmServerManager.getAlarmTrapReceivers();
        boolean enabled = manager.getFeatureManager().isFeatureEnabled(Alarms.FEATURE);
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            Writer cfdat = new FileWriter(new File(dir, "snmptrapd.cfdat"));
            try {
                writeCfdat(cfdat, enabled, fwd);
            } finally {
                IOUtils.closeQuietly(cfdat);
            }
            if (!enabled) {
                continue;
            }

            Writer yml = new FileWriter(new File(dir, "alarms.yaml"));
            try {
                writeAlarms(yml, alarms, Locale.getDefault());
            } finally {
                IOUtils.closeQuietly(yml);
            }

            Address smtp = manager.getAddressManager().getSingleAddress(MailManager.SMTP, location);
            Writer wtr = new FileWriter(new File(dir, "snmptrap-emails.yaml"));
            try {
                writeEmailHandlerConfig(wtr, alarms, groups, alarmServer, host, smtp);
            } finally {
                IOUtils.closeQuietly(wtr);
            }
        }
    }

    void writeCfdat(Writer w, boolean enabled, List<AlarmTrapReceiver> fwd) throws IOException {
        CfengineModuleConfiguration cfdat = new CfengineModuleConfiguration(w);
        cfdat.writeClass("snmptrap", true);
        cfdat.writeList("snmptrapdForward", fwd);
    }

    void writeAlarms(Writer w, List<Alarm> alarms, Locale l) throws IOException {
        YamlConfiguration c = new YamlConfiguration(w);
        for (Alarm a : alarms) {
            c.startStruct(a.getAlarmDefinition().getId());
            c.write(":groupName", a.getGroupName());
            c.write(":minThreshold", a.getMinThreshold());
            c.write(":summary",
                    m_messageSource.getMessage(format("alarm.%s.label", a.getAlarmDefinition().getId()), null, l));
            c.write(":resolution", m_messageSource.getMessage(
                    format("alarm.%s.resolution", a.getAlarmDefinition().getId()), null, l));
            c.endStruct();
        }
    }

    void writeEmailHandlerConfig(Writer wtr, List<Alarm> alarms, List<AlarmGroup> groups, AlarmServer server,
            String host, Address smtp) throws IOException {
        YamlConfiguration c = new YamlConfiguration(wtr);
        c.startStruct("emails");

        c.startStruct("sms.erb");
        for (AlarmGroup g : groups) {
            c.writeArray(g.getName(), g.getContactSmsAddresses());
        }
        c.endStruct();

        c.startStruct("email.erb");
        for (AlarmGroup g : groups) {
            Set<String> emails = new HashSet<String>();
            emails.addAll(g.getContactEmailAddresses());
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
            c.writeArray(g.getName(), emails);
        }
        c.endStruct();
        c.endStruct();

        c.startStruct("alarms");
        for (Alarm a : alarms) {
            c.write(a.getAlarmDefinition().getId(), a.getGroupName());
        }
        c.endStruct();

        c.write("from", defaultIfEmpty(server.getFromEmailAddress(), "postmaster@" + host));
        c.write("smtp", smtp);
    }

    public void setAlarmServerManager(AlarmServerManager alarmServerManager) {
        m_alarmServerManager = alarmServerManager;
    }

    public void setMessageSource(MessageSource messageSource) {
        m_messageSource = messageSource;
    }
}
