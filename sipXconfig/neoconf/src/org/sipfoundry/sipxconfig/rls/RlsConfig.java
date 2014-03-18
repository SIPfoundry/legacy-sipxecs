/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.rls;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.admin.AbstractResLimitsConfig;
import org.sipfoundry.sipxconfig.admin.ResLimitPluginConfig;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.feature.LocationFeature;
import org.sipfoundry.sipxconfig.im.ImManager;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.annotation.Required;

public class RlsConfig implements ConfigProvider, DaoEventListener, BeanFactoryAware, ResLimitPluginConfig {
    private Rls m_rls;
    private ResourceLists m_lists;
    private ConfigManager m_configManager;
    private BeanFactory m_factory;
    private AbstractResLimitsConfig m_rlsLimitsConfig;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(Rls.FEATURE)) {
            return;
        }

        Set<Location> locations = request.locations(manager);

        RlsSettings settings = m_rls.getSettings();
        boolean xmppPresenceEnabled = false;
        if (manager.getFeatureManager().isFeatureEnabled(ImManager.FEATURE)) {
            ImManager imManager = m_factory.getBean(ImManager.class);
            xmppPresenceEnabled = imManager.isPresenceEnabled();
        }
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            boolean enabled = manager.getFeatureManager().isFeatureEnabled(Rls.FEATURE, location);
            ConfigUtils.enableCfengineClass(dir, "sipxrls.cfdat", enabled, "sipxrls");
            if (!enabled) {
                continue;
            }

            Writer wtr = new FileWriter(new File(dir, "sipxrls-config.part"));
            try {
                write(wtr, settings, location, manager.getDomainManager().getDomain());
            } finally {
                IOUtils.closeQuietly(wtr);
            }

            //Write park resource limits separately to notify rls that needs to get restarted
            //All resource limits for all services are globally aggregated and replicated in ResLimitsConfiguration.java
            //The replication of resource-limits.ini is effective on each node that runs
            //at least one of the processes: mwi, registrar, proxy, park, rls, saa
            Writer rlsResLimitsWriter = new FileWriter(new File(dir, "resource-limits-rls.ini"));
            try {
                m_rlsLimitsConfig.writeResourceLimits(rlsResLimitsWriter, settings);
            } finally {
                IOUtils.closeQuietly(rlsResLimitsWriter);
            }

            FileWriter xmlwtr = new FileWriter(new File(dir, "resource-lists.xml"));
            XmlFile listsXml = new XmlFile(xmlwtr);
            listsXml.write(m_lists.getDocument(xmppPresenceEnabled));
            IOUtils.closeQuietly(xmlwtr);
        }
    }

    void write(Writer wtr, RlsSettings settings, Location location, Domain domain) throws IOException {
        KeyValueConfiguration config = KeyValueConfiguration.colonSeparated(wtr);
        config.writeSettings(settings.getSettings().getSetting("rls-config"));
        config.write("SIP_RLS_BIND_IP", location.getAddress());
        config.write("SIP_RLS_DOMAIN_NAME", domain.getName());
        config.write("SIP_RLS_AUTHENTICATE_REALM", domain.getSipRealm());
    }

    @Required
    public void setRls(Rls rls) {
        m_rls = rls;
    }

    @Required
    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    @Required
    public void setRlsLists(ResourceLists lists) {
        m_lists = lists;
    }

    @Override
    public void onDelete(Object entity) {
        onChange(entity);
    }

    @Override
    public void onSave(Object entity) {
        onChange(entity);
    }

    public void onChange(Object entity) {
        if (m_configManager.getFeatureManager().isFeatureEnabled(Rls.FEATURE) && entity instanceof User) {
            m_configManager.configureEverywhere(Rls.FEATURE);
        }
    }

    @Override
    public void setBeanFactory(BeanFactory factory) {
        m_factory = factory;
    }

    @Required
    public void setRlsLimitsConfig(AbstractResLimitsConfig rlsLimitsConfig) {
        m_rlsLimitsConfig = rlsLimitsConfig;
    }

    @Override
    public LocationFeature getLocationFeature() {
        return Rls.FEATURE;
    }

    @Override
    public AbstractResLimitsConfig getLimitsConfig() {
        return m_rlsLimitsConfig;
    }

    @Override
    public PersistableSettings getSettings() {
        return m_rls.getSettings();
    }

    @Override
    public void saveSettings(PersistableSettings settings) {
        m_rls.saveSettings((RlsSettings) settings);
    }
}
