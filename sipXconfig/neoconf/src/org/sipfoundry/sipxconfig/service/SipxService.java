/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessModel.ProcessName;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public abstract class SipxService extends BeanWithSettings {

    private String m_beanId;
    private String m_modelName;
    private String m_modelDir;
    private List<SipxServiceConfiguration> m_configurations;
    private String m_ipAddress;
    private String m_sipPort;
    private String m_hostname;
    private String m_fullHostname;
    private String m_domainName;
    private String m_realm;
    private String m_logDir;
    private String m_confDir;
    private String m_voicemailHttpsPort;

    public abstract ProcessName getProcessName();
    
    public String getBeanId() {
        return m_beanId;
    }

    public void setBeanId(String beanId) {
        m_beanId = beanId;
    }
    
    public String getModelName() {
        return m_modelName;
    }

    public void setModelName(String modelName) {
        m_modelName = modelName;
    }

    public String getModelDir() {
        return m_modelDir;
    }

    public void setModelDir(String modelDir) {
        m_modelDir = modelDir;
    }

    public void setConfigurations(List<SipxServiceConfiguration> configurations) {
        m_configurations = configurations;
    }

    public List<SipxServiceConfiguration> getConfigurations() {
        if (m_configurations == null) {
            return Collections.EMPTY_LIST;
        }

        return m_configurations;
    }

    public String getIpAddress() {
        return m_ipAddress;
    }

    public void setIpAddress(String ipAddress) {
        this.m_ipAddress = ipAddress;
    }

    public String getSipPort() {
        return m_sipPort;
    }

    public void setSipPort(String sipPort) {
        this.m_sipPort = sipPort;
    }

    public String getHostname() {
        return m_hostname;
    }

    public void setHostname(String hostname) {
        m_hostname = hostname;
    }

    public String getFullHostname() {
        return m_fullHostname;
    }

    public void setFullHostname(String fullHostname) {
        m_fullHostname = fullHostname;
    }

    public String getDomainName() {
        return m_domainName;
    }

    public void setDomainName(String domainName) {
        m_domainName = domainName;
    }

    public String getRealm() {
        return m_realm;
    }

    public void setRealm(String realm) {
        m_realm = realm;
    }

    public String getVoicemailHttpsPort() {
        return m_voicemailHttpsPort;
    }

    public void setVoicemailHttpsPort(String httpsPort) {
        m_voicemailHttpsPort = httpsPort;
    }

    public void setLogDir(String logDir) {
        m_logDir = logDir;
    }

    public String getLogDir() {
        return m_logDir;
    }

    public void setConfDir(String confDir) {
        m_confDir = confDir;
    }

    public String getConfDir() {
        return m_confDir;
    }

    /**
     * Override this method to perform validation
     */
    public void validate() {
        // no validation is done by default
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile(m_modelName, m_modelDir);
    }
}
