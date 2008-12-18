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
import java.util.Set;

import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.springframework.beans.factory.annotation.Required;

public abstract class SipxService extends BeanWithSettings {

    private String m_processName;
    private String m_beanId;
    private String m_modelName;
    private String m_modelDir;
    private List<SipxServiceConfiguration> m_configurations;
    private String m_sipPort;
    private String m_logDir;
    private String m_confDir;
    private String m_voicemailHttpsPort;
    private DomainManager m_domainManager;
    private Set<SipxServiceBundle> m_bundles;
    private boolean m_restartable = true;

    public String getProcessName() {
        return m_processName;
    }

    @Required
    public void setProcessName(String processName) {
        m_processName = processName;
    }

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

    /**
     * @param restartable true if it can/should be restarted from sipXconfig
     */
    public void setRestartable(boolean restartable) {
        m_restartable = restartable;
    }

    public boolean isRestartable() {
        return m_restartable;
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

    public String getSipPort() {
        return m_sipPort;
    }

    public void setSipPort(String sipPort) {
        this.m_sipPort = sipPort;
    }

    public String getRealm() {
        return m_domainManager.getAuthorizationRealm();
    }

    public String getDomainName() {
        return m_domainManager.getDomain().getName();
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

    @Required
    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    public void setBundles(Set<SipxServiceBundle> bundles) {
        m_bundles = bundles;
    }

    public Set<SipxServiceBundle> getBundles() {
        return m_bundles;
    }

    public boolean inBundle(SipxServiceBundle bundle) {
        return m_bundles != null && m_bundles.contains(bundle);
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

    @Override
    public final int hashCode() {
        return new HashCodeBuilder().append(getBeanId()).toHashCode();
    }

    @Override
    public final boolean equals(Object other) {
        if (!(other instanceof SipxService)) {
            return false;
        }
        if (this == other) {
            return true;
        }
        SipxService proc = (SipxService) other;
        return new EqualsBuilder().append(getBeanId(), proc.getBeanId()).isEquals();
    }
}
