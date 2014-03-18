/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.admin;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Map;

import org.apache.commons.io.IOUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.mwi.Mwi;
import org.sipfoundry.sipxconfig.mwi.MwiSettings;
import org.sipfoundry.sipxconfig.parkorbit.ParkOrbitContext;
import org.sipfoundry.sipxconfig.parkorbit.ParkSettings;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.sipfoundry.sipxconfig.proxy.ProxySettings;
import org.sipfoundry.sipxconfig.registrar.Registrar;
import org.sipfoundry.sipxconfig.registrar.RegistrarSettings;
//import org.sipfoundry.sipxconfig.rls.RlsSettings;
import org.sipfoundry.sipxconfig.saa.SaaManager;
import org.sipfoundry.sipxconfig.saa.SaaSettings;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.springframework.beans.BeansException;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.beans.factory.ListableBeanFactory;
import org.springframework.beans.factory.annotation.Required;
/**
 * There are three resource limits to configure:
 * fd-soft
 * fd-hard
 * core-enabled
 * These can apply to the following processes: sipXpark, sipXproxy, sipXrls, sipXsaa, mwi, sipXregistrar
 * We have adminSettings that establishes defaults. But for each process setting page there are same resource limits
 * and each process can overwrite the defaults. Any time admin settings page can overwrite all resource
 * limits for all processes with defaults
 *
 * EXAMPLE of resource-limits.ini:
 *
 * sipxproxy-fd-soft = 32768
 * sipxpark-fd-soft = 32768
 * sipxpublisher-fd-soft = 32768
 * sipxregistry-fd-soft = 32768
 * sipxrls-fd-soft = 32768
 * sipxsaa-fd-soft = 32768

 * sipxproxy-fd-hard = 65536
 * sipxpark-fd-hard = 65536
 * sipxpublisher-fd-hard = 65536
 * sipxregistry-fd-hard = 65536
 * sipxrls-fd-hard = 65536
 * sipxsaa-fd-hard = 65536

 * sipxproxy-core-enabled = false
 * sipxpark-core-enabled = false
 * sipxpublisher-core-enabled = false
 * sipxregistry-core-enabled = false
 * sipxrls-core-enabled = false
 * sipxsaa-core-enabled = false
 */
public class ResLimitsConfiguration implements ConfigProvider, BeanFactoryAware {
    private static final Log LOG = LogFactory.getLog(ResLimitsConfiguration.class);

    private Mwi m_mwi;
    private ProxyManager m_proxyManager;
    private Registrar m_registrar;
    private SaaManager m_saaManager;
    private ParkOrbitContext m_parkOrbitContext;
    private AdminContext m_adminContext;
    private ListableBeanFactory m_beanFactory;
    private Collection<AbstractResLimitsConfig> m_resLimitsConfigs;
    private Collection<ResLimitPluginConfig> m_pluginFeatures;
    private AbstractResLimitsConfig m_proxyLimitsConfig;
    private AbstractResLimitsConfig m_publisherLimitsConfig;
    private AbstractResLimitsConfig m_registrarLimitsConfig;
    private AbstractResLimitsConfig m_saaLimitsConfig;
    private AbstractResLimitsConfig m_parkLimitsConfig;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        Collection<Feature> features = new ArrayList<Feature>();
        Collection<ResLimitPluginConfig> pluginFeatures = getPluginFeatures();
        for (ResLimitPluginConfig pluginFeature : pluginFeatures) {
            if (pluginFeature.getLimitsConfig() != null) {
                features.add(pluginFeature.getLocationFeature());
            }
        }
        features.add(ProxyManager.FEATURE);
        features.add(Mwi.FEATURE);
        features.add(Registrar.FEATURE);
        features.add(SaaManager.FEATURE);
        features.add(SaaManager.FEATURE);
        features.add(ParkOrbitContext.FEATURE);
        if (!request.applies(features)) {
            return;
        }
        final Writer w = createWriter(manager);
        try {
            LOG.info("Write FEATURED resource limits");
            writeFeaturedResourceLimits(w);
        } finally {
            IOUtils.closeQuietly(w);
        }
    }

    private void setResLimitsValues(PersistableSettings settings, Setting fdSoft, Setting fdHard, Setting coreEnabled) {
        settings.setSettingTypedValue("resource-limits/fd-soft", fdSoft.getTypedValue());
        settings.setSettingTypedValue("resource-limits/fd-hard", fdHard.getTypedValue());
        settings.setSettingTypedValue("resource-limits/core-enabled", coreEnabled.getTypedValue());
    }

    public void writeFeaturedResourceLimits(Writer w) throws IOException {
        m_proxyLimitsConfig.writeResourceLimits(w, m_proxyManager.getSettings());
        m_publisherLimitsConfig.writeResourceLimits(w, m_mwi.getSettings());
        m_registrarLimitsConfig.writeResourceLimits(w, m_registrar.getSettings());
        m_saaLimitsConfig.writeResourceLimits(w, m_saaManager.getSettings());
        m_parkLimitsConfig.writeResourceLimits(w, m_parkOrbitContext.getSettings());
        Collection<ResLimitPluginConfig> pluginFeatures = getPluginFeatures();
        AbstractResLimitsConfig resLimitsConfig = null;
        for (ResLimitPluginConfig pluginConfig : pluginFeatures) {
            resLimitsConfig = pluginConfig.getLimitsConfig();
            if (resLimitsConfig != null) {
                resLimitsConfig.writeResourceLimits(w, pluginConfig.getSettings());
            }
        }

    }

    private Writer createWriter(ConfigManager manager) throws IOException {
        File dir = manager.getGlobalDataDirectory();
        return new FileWriter(new File(dir, "resource-limits.ini"));
    }

    public void writeDefaultsResourceLimits(ConfigManager manager) throws IOException {
        final Writer w = createWriter(manager);
        try {
            LOG.info("Write DEFAULT resource limits");
            writeDefaultsResourceLimits(w);
        } finally {
            IOUtils.closeQuietly(w);
        }
    }

    protected void writeDefaultsResourceLimits(Writer w) throws IOException {
        Collection<AbstractResLimitsConfig> resLimitsConfigs = getResLimitsConfigs();
        AdminSettings settings = m_adminContext.getSettings();

        Setting fdSoft = settings.getSettings().getSetting("configserver-config/fd-soft");
        Setting fdHard = settings.getSettings().getSetting("configserver-config/fd-hard");
        Setting coreEnabled = settings.getSettings().getSetting("configserver-config/core-enabled");

        for (AbstractResLimitsConfig resLimitsConfig : resLimitsConfigs) {
            resLimitsConfig.writeResourceLimits(w, fdSoft, fdHard, coreEnabled);
        }

        ProxySettings proxySettings = m_proxyManager.getSettings();
        setResLimitsValues(proxySettings, fdSoft, fdHard, coreEnabled);
        m_proxyManager.saveSettings(proxySettings);

        MwiSettings mwiSettings = m_mwi.getSettings();
        setResLimitsValues(mwiSettings, fdSoft, fdHard, coreEnabled);
        m_mwi.saveSettings(mwiSettings);

        RegistrarSettings registrarSettings = m_registrar.getSettings();
        setResLimitsValues(registrarSettings, fdSoft, fdHard, coreEnabled);
        m_registrar.saveSettings(registrarSettings);

        SaaSettings saaSettings = m_saaManager.getSettings();
        setResLimitsValues(saaSettings, fdSoft, fdHard, coreEnabled);
        m_saaManager.saveSettings(saaSettings);

        ParkSettings parkSettings = m_parkOrbitContext.getSettings();
        setResLimitsValues(parkSettings, fdSoft, fdHard, coreEnabled);
        m_parkOrbitContext.saveSettings(parkSettings);
        Collection<ResLimitPluginConfig> pluginFeatures = getPluginFeatures();
        AbstractResLimitsConfig resLimitsConfig = null;
        PersistableSettings pluginSettings = null;
        for (ResLimitPluginConfig pluginConfig : pluginFeatures) {
            resLimitsConfig = pluginConfig.getLimitsConfig();
            if (resLimitsConfig != null) {
                pluginSettings = pluginConfig.getSettings();
                setResLimitsValues(pluginSettings, fdSoft, fdHard, coreEnabled);
                pluginConfig.saveSettings(pluginSettings);
            }
        }
    }

    private Collection<AbstractResLimitsConfig> getResLimitsConfigs() {
        if (m_resLimitsConfigs == null) {
            Map<String, AbstractResLimitsConfig> resLimitsConfigsMap = m_beanFactory.
                    getBeansOfType(AbstractResLimitsConfig.class, false, false);
            m_resLimitsConfigs = resLimitsConfigsMap.values();
        }
        return m_resLimitsConfigs;
    }

    private Collection<ResLimitPluginConfig> getPluginFeatures() {
        if (m_pluginFeatures == null) {
            Map<String, ResLimitPluginConfig> resLimitsConfigsMap = m_beanFactory.
                    getBeansOfType(ResLimitPluginConfig.class, false, false);
            m_pluginFeatures = resLimitsConfigsMap.values();
        }
        return m_pluginFeatures;
    }

    /**
     * Used in tests to inject custom resLimits collection
     * @param resLimitsConfigs
     */
    public void setResLimitsConfigs(Collection<AbstractResLimitsConfig> resLimitsConfigs) {
        m_resLimitsConfigs = resLimitsConfigs;
    }

    @Required
    public void setMwi(Mwi mwi) {
        m_mwi = mwi;
    }

    @Required
    public void setProxyManager(ProxyManager proxyManager) {
        m_proxyManager = proxyManager;
    }

    @Required
    public void setRegistrar(Registrar registrar) {
        m_registrar = registrar;
    }

    @Required
    public void setSaaManager(SaaManager saaManager) {
        m_saaManager = saaManager;
    }

    @Required
    public void setAdminContext(AdminContext adminContext) {
        m_adminContext = adminContext;
    }

    @Required
    public void setParkOrbitContext(ParkOrbitContext parkOrbitContext) {
        m_parkOrbitContext = parkOrbitContext;
    }

    @Required
    public void setProxyLimitsConfig(AbstractResLimitsConfig proxyLimitsConfig) {
        m_proxyLimitsConfig = proxyLimitsConfig;
    }

    @Required
    public void setPublisherLimitsConfig(AbstractResLimitsConfig publisherLimitsConfig) {
        m_publisherLimitsConfig = publisherLimitsConfig;
    }

    @Required
    public void setRegistrarLimitsConfig(AbstractResLimitsConfig registrarLimitsConfig) {
        m_registrarLimitsConfig = registrarLimitsConfig;
    }

    @Required
    public void setSaaLimitsConfig(AbstractResLimitsConfig saaLimitsConfig) {
        m_saaLimitsConfig = saaLimitsConfig;
    }

    @Required
    public void setParkLimitsConfig(AbstractResLimitsConfig parkLimitsConfig) {
        m_parkLimitsConfig = parkLimitsConfig;
    }

    @Override
    public void setBeanFactory(BeanFactory beanFactory) throws BeansException {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }
}
