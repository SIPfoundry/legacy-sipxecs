/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.io.IOException;

import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.ConfigFileStorage;
import org.sipfoundry.sipxconfig.setting.Setting;

public class SipxServer extends BeanWithSettings implements Server {
    private static final String DOMAIN_NAME = "domain/SIPXCHANGE_DOMAIN_NAME";
    private static final String PAGING_LOG_LEVEL = "Logging/log.level";

    private String m_configDirectory;
    private ConfigFileStorage m_storage;
    private String m_mohUser;

    public void setMohUser(String mohUser) {
        m_mohUser = mohUser;
    }

    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("server.xml", "commserver");
    }

    /**
     * Still need to call <code>applySettings</code> save to send to disk
     */
    public void setDomainName(String domainName) {
        setSettingValue(DOMAIN_NAME, domainName);
    }

    public void applySettings() {
        try {
            m_storage.flush();
        } catch (IOException e) {
            // TODO: catch and report as User Exception
            throw new RuntimeException(e);
        }
    }

    public void resetSettings() {
        m_storage.reset();
    }

    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;
        m_storage = new ConfigFileStorage(m_configDirectory);
        setValueStorage(m_storage);
    }

    public String getMusicOnHoldUri(String domainName) {
        return SipUri.format(m_mohUser, domainName, false);
    }

    public String getPagingLogLevel() {
        return getSettingValue(PAGING_LOG_LEVEL);
    }
}
