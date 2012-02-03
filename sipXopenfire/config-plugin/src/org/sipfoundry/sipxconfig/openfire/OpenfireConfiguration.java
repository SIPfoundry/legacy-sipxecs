/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.openfire;

import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Writer;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapManager;
import org.sipfoundry.sipxconfig.bulk.ldap.LdapSystemSettings;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigManager;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigProvider;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigRequest;
import org.sipfoundry.sipxconfig.cfgmgt.ConfigUtils;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.event.DaoEventListener;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.conference.Conference;
import org.sipfoundry.sipxconfig.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.localization.LocalizationContext;

public class OpenfireConfiguration implements ConfigProvider, DaoEventListener {
    private OpenfireConfigurationFile m_config;
    private SipxOpenfireConfiguration m_sipxConfig;
    private XmppAccountInfo m_accountConfig;
    private LdapManager m_ldapManager;
    private ConfigManager m_configManager;

    @Override
    public void replicate(ConfigManager manager, ConfigRequest request) throws IOException {
        if (!request.applies(OpenfireImpl.FEATURE, LdapManager.FEATURE, LocalizationContext.FEATURE)) {
            return;
        }
        
        if (request.applies(LdapManager.FEATURE)) {
            LdapSystemSettings settings = m_ldapManager.getSystemSettings();
            boolean isEnableOpenfireConfiguration = settings.isEnableOpenfireConfiguration() && settings.isConfigured();
            if (!isEnableOpenfireConfiguration) {
                return;
            }
        }
        
        List<Location> locations = manager.getFeatureManager().getLocationsForEnabledFeature(OpenfireImpl.FEATURE);
        for (Location location : locations) {
            File dir = manager.getLocationDataDirectory(location);
            boolean enabled = manager.getFeatureManager().isFeatureEnabled(OpenfireImpl.FEATURE, location);
            ConfigUtils.enableCfengineClass(dir, "sipxopenfire.cfdat", "sipxopenfire", enabled);
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
}
