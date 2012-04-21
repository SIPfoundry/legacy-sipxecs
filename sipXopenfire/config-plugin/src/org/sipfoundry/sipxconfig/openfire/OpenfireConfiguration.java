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
import org.sipfoundry.sipxconfig.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.event.WebSocket;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.imbot.ImBot;
import org.sipfoundry.sipxconfig.localization.LocalizationContext;
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

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(OpenfireImpl.FEATURE, LdapManager.FEATURE, LocalizationContext.FEATURE, ImBot.FEATURE)) {
            return;
        }

        /*if (request.applies(LdapManager.FEATURE)) {
            LdapSystemSettings settings = m_ldapManager.getSystemSettings();
            boolean isEnableOpenfireConfiguration = settings.isEnableOpenfireConfiguration() && settings.isConfigured();
            if (!isEnableOpenfireConfiguration) {
                return;
            }
        }*/

        Set<Location> locations = request.locations(manager);
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            boolean enabled = manager.getFeatureManager().isFeatureEnabled(OpenfireImpl.FEATURE, location);
            ConfigUtils.enableCfengineClass(dir, "sipxopenfire.cfdat", enabled, "sipxopenfire");
            if (!enabled) {
                continue;
            }
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
        }
    }

    void write(Writer wtr, String wsAddress, int wsPort, String adminRestUrl) throws IOException {
        KeyValueConfiguration config = KeyValueConfiguration.equalsSeparated(wtr);
        config.write("websocket.address", wsAddress);
        config.write("websocket.port", wsPort);
        config.write("admin.rest.url", adminRestUrl);
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
        if (entity instanceof User || entity instanceof Conference) {
            m_configManager.configureEverywhere(OpenfireImpl.FEATURE);
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
}
