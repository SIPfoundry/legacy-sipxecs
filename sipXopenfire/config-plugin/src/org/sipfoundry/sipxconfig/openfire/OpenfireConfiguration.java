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

import org.apache.commons.io.FileUtils;
import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapManager;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapSystemSettings;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.cfgmgt.YamlConfiguration;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.event.WebSocket;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.imbot.ImBot;
import org.sipfoundry.sipxconfig.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.setting.Group;
import org.springframework.beans.factory.annotation.Required;

public class OpenfireConfiguration implements ConfigProvider, DaoEventListener {
    private OpenfireConfigurationFile m_config;
    private SipxOpenfireConfiguration m_sipxConfig;
    private XmppAccountInfo m_accountConfig;
    private LdapManager m_ldapManager;
    private ConfigManager m_configManager;
    private FeatureManager m_featureManager;
    private WebSocket m_websocket;
    private Openfire m_openfire;
    private String m_updateFile;
    private static final String AUTH_CLASSNAME_KEY = "provider.auth.className";

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(OpenfireImpl.FEATURE, LdapManager.FEATURE, LocalizationContext.FEATURE, ImBot.FEATURE)) {
            return;
        }

        Set<Location> locations = request.locations(manager);
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            boolean enabled = manager.getFeatureManager().isFeatureEnabled(OpenfireImpl.FEATURE, location);
            String datfile = "sipxopenfire.cfdat";
            String sipxopenfireClass = "sipxopenfire";
            if (!enabled) {
                ConfigUtils.enableCfengineClass(dir, datfile, false, sipxopenfireClass);
                continue;
            }
            ConfigUtils.enableCfengineClass(dir, datfile, true, sipxopenfireClass, "postgres");
            boolean consoleEnabled = (Boolean) m_openfire.getSettings().getSettingTypedValue("settings/console");
            ConfigUtils.enableCfengineClass(dir, "ofconsole.cfdat", consoleEnabled, "ofconsole");
            File f = new File(dir, "sipx.properties.part");
            if(!f.exists()) {
                f.createNewFile();
            }
            Writer wtr = new FileWriter(f);
            try {
                if (m_featureManager.isFeatureEnabled(WebSocket.FEATURE, location)) {
                    Address addr = m_configManager.getAddressManager().getSingleAddress(AdminContext.HTTP_ADDRESS);
                    write(wtr, location.getAddress(), m_websocket.getSettings().getWebSocketPort(), addr.toString());
                }

            } finally {
                IOUtils.closeQuietly(wtr);
            }
            Writer sipxopenfire = new FileWriter(new File(dir, "openfire.xml"));
            try {
                m_config.write(sipxopenfire);
            } finally {
                IOUtils.closeQuietly(sipxopenfire);
            }

            Writer multipleldap = new FileWriter(new File(dir, "multipleldap-openfire.xml"));
            try {
                m_config.writeMultipleLdapConfiguration(multipleldap);
            } finally {
                IOUtils.closeQuietly(multipleldap);
            }

            Writer openfire = new FileWriter(new File(dir, "sipxopenfire.xml"));
            try {
                m_sipxConfig.write(openfire, location);
            } finally {
                IOUtils.closeQuietly(openfire);
            }

            Writer account = new FileWriter(new File(dir, "xmpp-account-info.xml"));
            try {
                XmlFile config = new XmlFile(account);
                config.write(m_accountConfig.getDocument());
            } finally {
                IOUtils.closeQuietly(account);
            }

            Writer ofproperty = new FileWriter(new File(dir, "ofproperty.yaml"));
            try {
                writeOfPropertyConfig(ofproperty, m_openfire.getSettings());
            } finally {
                IOUtils.closeQuietly(ofproperty);
            }
        }
    }

    void write(Writer wtr, String wsAddress, int wsPort, String adminRestUrl) throws IOException {
        KeyValueConfiguration config = KeyValueConfiguration.equalsSeparated(wtr);
        config.write("websocket.address", wsAddress);
        config.write("websocket.port", wsPort);
        config.write("admin.rest.url", adminRestUrl);
    }

    void writeOfPropertyConfig(Writer w, OpenfireSettings settings) throws IOException {
        YamlConfiguration config = new YamlConfiguration(w);
        writeSettings(config, settings);
    }

    void writeSettings(YamlConfiguration config, OpenfireSettings settings) throws IOException {
        if (settings == null) {
            return;
        }

        config.writeSettings(settings.getOfProperty());
        LdapSystemSettings systemSettings = m_ldapManager.getSystemSettings();
        boolean isEnableOpenfireConfiguration = systemSettings.isEnableOpenfireConfiguration() && systemSettings.isConfigured();
        if (isEnableOpenfireConfiguration) {
            config.write(AUTH_CLASSNAME_KEY, m_config.getProviderLdapAuthClassName());
        } else {
            config.write(AUTH_CLASSNAME_KEY, m_config.getProviderAuthClassName());
        }
    }

    public void setConfig(OpenfireConfigurationFile config) {
        m_config = config;
    }

    public void setLdapManager(LdapManager ldapManager) {
        m_ldapManager = ldapManager;
    }

    public void setSipxConfig(SipxOpenfireConfiguration sipxConfig) {
        m_sipxConfig = sipxConfig;
    }

    private void checkReplicate(Object entity) {
        if (entity instanceof User || entity instanceof Conference || entity instanceof Group) {
            m_configManager.configureEverywhere(OpenfireImpl.FEATURE);
            try {
                FileUtils.touch(new File(m_updateFile));
            } catch (IOException ex) {
                // do nothing, will replicate next time
            }

        }
    }

    @Override
    public void onDelete(Object entity) {
        checkReplicate(entity);
    }

    @Override
    public void onSave(Object entity) {
        checkReplicate(entity);
    }

    public void setAccountConfig(XmppAccountInfo accountConfig) {
        m_accountConfig = accountConfig;
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

    @Required
    public void setUpdateFile(String file) {
        m_updateFile = file;
    }
}
