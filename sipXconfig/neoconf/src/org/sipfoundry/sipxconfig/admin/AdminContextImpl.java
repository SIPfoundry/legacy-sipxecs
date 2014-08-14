/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.admin;

import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.address.AddressType;
import org.sipfoundry.sipxconfig.alarm.AlarmDefinition;
import org.sipfoundry.sipxconfig.alarm.AlarmProvider;
import org.sipfoundry.sipxconfig.alarm.AlarmServerManager;
import org.sipfoundry.sipxconfig.backup.ArchiveDefinition;
import org.sipfoundry.sipxconfig.backup.ArchiveProvider;
import org.sipfoundry.sipxconfig.backup.BackupManager;
import org.sipfoundry.sipxconfig.backup.BackupPlan;
import org.sipfoundry.sipxconfig.backup.BackupSettings;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.firewall.DefaultFirewallRule;
import org.sipfoundry.sipxconfig.firewall.FirewallManager;
import org.sipfoundry.sipxconfig.firewall.FirewallProvider;
import org.sipfoundry.sipxconfig.setting.BeanWithSettingsDao;
import org.sipfoundry.sipxconfig.snmp.ProcessDefinition;
import org.sipfoundry.sipxconfig.snmp.ProcessProvider;
import org.sipfoundry.sipxconfig.snmp.SnmpManager;
import org.springframework.beans.factory.annotation.Required;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

/**
 * Backup provides Java interface to backup scripts
 */
public class AdminContextImpl extends HibernateDaoSupport implements AdminContext, AddressProvider, ProcessProvider,
        AlarmProvider, FirewallProvider, ArchiveProvider {
    private static final Collection<AddressType> ADDRESSES = Arrays.asList(new AddressType[] {
        HTTP_ADDRESS, HTTP_ADDRESS_AUTH, HTTPS_ADDRESS_AUTH, SIPXCDR_DB_ADDRESS
    });
    private static final String BACKUP_COMMAND = "sipxconfig-archive --backup %s";
    private static final String RESTORE_COMMAND = "sipxconfig-archive --restore %s";
    private LocationsManager m_locationsManager;
    private DomainManager m_domainManager;
    private BeanWithSettingsDao<AdminSettings> m_settingsDao;
    private StringBuilder m_backup = new StringBuilder(BACKUP_COMMAND);
    private StringBuilder m_restore = new StringBuilder(RESTORE_COMMAND);

    @Override
    public Collection<Address> getAvailableAddresses(AddressManager manager, AddressType type, Location requester) {
        if (!ADDRESSES.contains(type)) {
            return null;
        }
        Location location = m_locationsManager.getPrimaryLocation();
        Address address = null;
        if (type.equals(HTTP_ADDRESS)) {
            address = new Address(HTTP_ADDRESS, location.getAddress(), HTTP_ADDRESS.getCanonicalPort());
        } else if (type.equals(HTTP_ADDRESS_AUTH)) {
            address = new Address(HTTP_ADDRESS_AUTH, location.getAddress(), HTTP_ADDRESS_AUTH.getCanonicalPort());
        } else if (type.equals(SIPXCDR_DB_ADDRESS)) {
            address = new Address(SIPXCDR_DB_ADDRESS, location.getAddress());
        } else if (type.equals(HTTPS_ADDRESS_AUTH)) {
            address = new Address(HTTPS_ADDRESS_AUTH, location.getAddress());
        }
        return Collections.singleton(address);
    }

    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    @Override
    public Collection<ProcessDefinition> getProcessDefinitions(SnmpManager manager, Location location) {
        return (location.isPrimary() ? Collections.singleton(ProcessDefinition.sipxByRegex("sipxconfig",
                ".*-Dprocname=sipxconfig.*")) : null);
    }

    @Override
    public Collection<AlarmDefinition> getAvailableAlarms(AlarmServerManager manager) {
        return Arrays.asList(new AlarmDefinition[]{ALARM_LOGIN_FAILED, ALARM_DNS_LOOKUP});
    }

    @Override
    public Collection<DefaultFirewallRule> getFirewallRules(FirewallManager manager) {
        return Arrays.asList(new DefaultFirewallRule(HTTP_ADDRESS), new DefaultFirewallRule(HTTP_ADDRESS_AUTH),
            new DefaultFirewallRule(HTTPS_ADDRESS_AUTH), new DefaultFirewallRule(SIPXCDR_DB_ADDRESS));
    }

    @Override
    public Collection<ArchiveDefinition> getArchiveDefinitions(BackupManager manager, Location location,
            BackupPlan plan, BackupSettings settings) {
        if (!location.isPrimary()) {
            return null;
        }

        buildArchiveCommands(plan, settings);
        ArchiveDefinition def = new ArchiveDefinition(ARCHIVE, m_backup.toString(), m_restore.toString());
        return Collections.singleton(def);
    }

    protected void initBaseCommands() {
        m_backup = new StringBuilder(BACKUP_COMMAND);
        m_restore = new StringBuilder(RESTORE_COMMAND);
    }

    protected void buildArchiveCommands(BackupPlan plan, BackupSettings settings) {
        //Reset backup/restore commands to original values to avoid additional params to be added multiple times
        initBaseCommands();
        if (plan != null && settings != null) {
            if (!plan.isIncludeDeviceFiles()) {
                m_backup.append(" --no-device-files");
            }
            if (settings.isKeepDomain()) {
                m_restore.append(" --domain ")
                         .append(m_domainManager.getDomain().getName());
            }
            if (settings.isKeepFqdn()) {
                m_restore.append(" --fqdn ")
                         .append(m_locationsManager.getPrimaryLocation().getFqdn());
            }
            String resetPin = settings.getResetPin();
            String resetPassword = settings.getResetPassword();
            if (settings.isDecodePins()) {
                if (StringUtils.isBlank(resetPin) || StringUtils.isBlank(resetPassword)) {
                    throw new UserException("&error.passwordsRequired");
                }
                m_restore.append(" --crack-pin ").append(resetPin);
                m_restore.append(" --crack-passwd ").append(resetPassword);
                m_restore.append(" --crack-pin-len ").append(settings.getDecodePinLen());
            } else {
                if (StringUtils.isNotBlank(resetPin)) {
                    m_restore.append(" --reset-pin ").append(resetPin);
                }
                if (StringUtils.isNotBlank(resetPassword)) {
                    m_restore.append(" --reset-password ").append(resetPassword);
                }
            }
        }
    }

    protected void setBackup(StringBuilder backup) {
        m_backup = backup;
    }

    protected StringBuilder getRestore() {
        return m_restore;
    }

    protected void setRestore(StringBuilder restore) {
        m_restore = restore;
    }

    @Override
    public AdminSettings getSettings() {
        return m_settingsDao.findOrCreateOne();
    }

    @Override
    public void saveSettings(AdminSettings settings) {
        m_settingsDao.upsert(settings);
    }

    public void setSettingsDao(BeanWithSettingsDao<AdminSettings> settingsDao) {
        m_settingsDao = settingsDao;
    }

    @Override
    public String getPasswordPolicy() {
        return getSettings().getSelectedPolicy();
    }

    @Override
    public String getDefaultPassword() {
        return getSettings().getDefaultPassword();
    }

    @Override
    public String getDefaultVmPin() {
        return getSettings().getDefaultVmPin();
    }

    @Override
    public int getAge() {
        return getSettings().getAge();
    }

    @Override
    public int getPageImportSize() {
        return getSettings().getPageImportSize();
    }

    @Override
    public boolean isDisable() {
        return getSettings().isDisable();
    }

    @Override
    public boolean isDelete() {
        return getSettings().isDelete();
    }

    @Override
    public boolean isAuthAccName() {
        return getSettings().isAuthAccName();
    }

    public boolean isAuthEmailAddress() {
        return getSettings().isAuthEmailAddress();
    }

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    @Override
    public boolean isSystemAuditEnabled() {
        return getSettings().isSystemAuditEnabled();
    }
}
