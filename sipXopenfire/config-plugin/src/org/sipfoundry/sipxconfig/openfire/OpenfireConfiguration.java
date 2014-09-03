/**
 *
 *
 * Copyright (c) 2011 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.openfire;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.Set;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.cfgmgt.LoggerKeyValueConfiguration;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.event.WebSocket;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.im.ImManager;
import org.sipfoundry.sipxconfig.imbot.ImBot;
import org.sipfoundry.sipxconfig.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingUtil;
import org.springframework.beans.factory.annotation.Required;

public class OpenfireConfiguration implements ConfigProvider {
    protected static final String DAT_FILE = "sipxopenfire.cfdat";
    protected static final String SIPXOPENFIRE_CLASS = "sipxopenfire";
    protected static final String SIPXOPENFIRE_CONFIG_CLASS = "sipxofconfig";

    private OpenfireConfigurationFile m_config;
    private SipxOpenfireConfiguration m_sipxConfig;
    private ConfigManager m_configManager;
    private FeatureManager m_featureManager;
    private WebSocket m_websocket;
    private Openfire m_openfire;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (applies(request)) {
            writeConfigFiles(manager, request);

            touchImFile();
        }
    }

    protected static boolean applies(ConfigRequest request) {
        return request.applies(ImManager.FEATURE, LdapManager.FEATURE, LocalizationContext.FEATURE, ImBot.FEATURE,
                LocationsManager.FEATURE);
    }

    protected void writeConfigFiles(ConfigManager manager, ConfigRequest request) throws IOException {
        Set<Location> locations = request.locations(manager);
        boolean configNode = true;
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            boolean enabled = manager.getFeatureManager().isFeatureEnabled(ImManager.FEATURE, location);
            if (!enabled) {
                disableIm(dir);
                continue;
            }
            enableIm(dir, configNode);
            configNode = false;
            OpenfireSettings settings = m_openfire.getSettings();
            boolean consoleEnabled = (Boolean) settings.getSettingTypedValue("settings/console");
            boolean presenceEnabled = (Boolean) settings.getSettingTypedValue("settings/enable-presence");
            ConfigUtils.enableCfengineClass(dir, "ofconsole.cfdat", consoleEnabled, "ofconsole");

            Setting openfireSettings = settings.getSettings().getSetting("settings");
            String log4jFileName = "log4j-openfire.properties.part";
            String[] logLevelKeys = {"log4j.logger.org.sipfoundry.openfire",
                                     "log4j.logger.org.sipfoundry.sipxconfig"};
            SettingUtil.writeLog4jSetting(openfireSettings, dir, log4jFileName, logLevelKeys);

            File f = new File(dir, "sipx.properties.part");
            if (!f.exists()) {
                f.createNewFile();
            }
            Writer wtr = new FileWriter(f);
            try {
                boolean isWsEnabled = m_featureManager.isFeatureEnabled(WebSocket.FEATURE, location);
                Address addr = m_configManager.getAddressManager().getSingleAddress(AdminContext.HTTP_ADDRESS);
                write(wtr, isWsEnabled, location.getAddress(), m_websocket.getSettings()
                        .getWebSocketPort(), addr.toString());
            } finally {
                IOUtils.closeQuietly(wtr);
            }

            Writer ofproperty = new FileWriter(new File(dir, "openfire.properties.part"));
            try {
                m_config.writeOfPropertyConfig(ofproperty, m_openfire.getSettings());
                writeLdapProps(ofproperty);
            } finally {
                IOUtils.closeQuietly(ofproperty);
            }

            Writer openfire = new FileWriter(new File(dir, "sipxopenfire.xml"));
            try {
                m_sipxConfig.write(openfire, location);
            } finally {
                IOUtils.closeQuietly(openfire);
            }
        }
    }

    protected void writeLdapProps(Writer ofproperty) throws IOException {
        m_config.writeOfLdapPropertyConfig(ofproperty, m_openfire.getSettings());
    }

    @SuppressWarnings("static-method")
    protected void enableIm(File dir, boolean configNode) throws IOException {
        if (configNode) {
            ConfigUtils.enableCfengineClass(dir, DAT_FILE, true, SIPXOPENFIRE_CLASS, SIPXOPENFIRE_CONFIG_CLASS);
        } else {
            ConfigUtils.enableCfengineClass(dir, DAT_FILE, true, SIPXOPENFIRE_CLASS);
        }

    }

    @SuppressWarnings("static-method")
    protected void disableIm(File dir) throws IOException {
        ConfigUtils.enableCfengineClass(dir, DAT_FILE, false, SIPXOPENFIRE_CLASS, SIPXOPENFIRE_CONFIG_CLASS);
    }

    protected void touchImFile() {
        // touch xmpp_update.xml on every location where openfire runs
        m_openfire.touchXmppUpdate(m_featureManager.getLocationsForEnabledFeature(ImManager.FEATURE));
    }

    private static void write(Writer wtr, boolean wsEnabled, String wsAddress, int wsPort,
            String adminRestUrl) throws IOException {
        LoggerKeyValueConfiguration config = LoggerKeyValueConfiguration.equalsSeparated(wtr);
        if (wsEnabled) {
            config.write("websocket.address", wsAddress);
            config.write("websocket.port", wsPort);
            config.write("admin.rest.url", adminRestUrl);
        }
    }

    public void setConfig(OpenfireConfigurationFile config) {
        m_config = config;
    }

    public void setSipxConfig(SipxOpenfireConfiguration sipxConfig) {
        m_sipxConfig = sipxConfig;
    }

    public void setConfigManager(ConfigManager configManager) {
        m_configManager = configManager;
    }

    @Required
    public void setFeatureManager(FeatureManager featureManager) {
        m_featureManager = featureManager;
    }

    @Required
    public void setWebsocket(WebSocket websocket) {
        m_websocket = websocket;
    }

    @Required
    public void setOpenfire(Openfire openfire) {
        m_openfire = openfire;
    }

    public OpenfireConfigurationFile getConfig() {
        return m_config;
    }
}
