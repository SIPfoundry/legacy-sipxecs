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
import org.sipfoundry.sipxconfig.cfgmgt.KeyValueConfiguration;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.event.WebSocket;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.im.ImManager;
import org.sipfoundry.sipxconfig.imbot.ImBot;
import org.sipfoundry.sipxconfig.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.rls.Rls;
import org.sipfoundry.sipxconfig.setting.Group;
import org.springframework.beans.factory.annotation.Required;

public class OpenfireConfiguration implements ConfigProvider, DaoEventListener {
    private OpenfireConfigurationFile m_config;
    private SipxOpenfireConfiguration m_sipxConfig;
    private ConfigManager m_configManager;
    private FeatureManager m_featureManager;
    private WebSocket m_websocket;
    private Openfire m_openfire;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(ImManager.FEATURE, LdapManager.FEATURE, LocalizationContext.FEATURE, ImBot.FEATURE)) {
            return;
        }

        Set<Location> locations = request.locations(manager);
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            boolean enabled = manager.getFeatureManager().isFeatureEnabled(ImManager.FEATURE, location);
            String datfile = "sipxopenfire.cfdat";
            String sipxopenfireClass = "sipxopenfire";
            if (!enabled) {
                ConfigUtils.enableCfengineClass(dir, datfile, false, sipxopenfireClass);
                continue;
            }
            ConfigUtils.enableCfengineClass(dir, datfile, true, sipxopenfireClass, "postgres");
            OpenfireSettings settings = m_openfire.getSettings();
            boolean consoleEnabled = (Boolean) settings.getSettingTypedValue("settings/console");
            boolean presenceEnabled = (Boolean) settings.getSettingTypedValue("settings/enable-presence")
                    && manager.getFeatureManager().isFeatureEnabled(Rls.FEATURE);
            ConfigUtils.enableCfengineClass(dir, "ofconsole.cfdat", consoleEnabled, "ofconsole");
            File f = new File(dir, "sipx.properties.part");
            if (!f.exists()) {
                f.createNewFile();
            }
            Writer wtr = new FileWriter(f);
            try {
                boolean isWsEnabled = m_featureManager.isFeatureEnabled(WebSocket.FEATURE, location);
                Address addr = m_configManager.getAddressManager().getSingleAddress(AdminContext.HTTP_ADDRESS);
                write(wtr, presenceEnabled, isWsEnabled, location.getAddress(), m_websocket.getSettings()
                        .getWebSocketPort(), addr.toString());
            } finally {
                IOUtils.closeQuietly(wtr);
            }
            Writer sipxopenfire = new FileWriter(new File(dir, "openfire.xml"));
            try {
                m_config.write(sipxopenfire);
            } finally {
                IOUtils.closeQuietly(sipxopenfire);
            }

            Writer ofproperty = new FileWriter(new File(dir, "openfire.properties.part"));
            try {
                m_config.writeOfPropertyConfig(ofproperty, m_openfire.getSettings());
            } finally {
                IOUtils.closeQuietly(ofproperty);
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
        }
        //touch xmpp_update.xml on every location where openfire runs
        m_openfire.touchXmppUpdate(m_featureManager.getLocationsForEnabledFeature(ImManager.FEATURE));
    }

    private static void write(Writer wtr, boolean presence, boolean wsEnabled, String wsAddress, int wsPort, String adminRestUrl) throws IOException {
        KeyValueConfiguration config = KeyValueConfiguration.equalsSeparated(wtr);
        config.write("openfire.presence", presence);
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

    private void checkReplicate(Object entity) {
        if (entity instanceof User || entity instanceof Conference || entity instanceof Group) {
            if (m_configManager.getFeatureManager().isFeatureEnabled(ImManager.FEATURE)) {
                m_configManager.configureEverywhere(ImManager.FEATURE);
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
}
