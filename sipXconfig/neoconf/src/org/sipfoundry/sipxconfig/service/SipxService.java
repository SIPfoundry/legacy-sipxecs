/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.lang.builder.EqualsBuilder;
import org.apache.commons.lang.builder.HashCodeBuilder;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.LoggingManager;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.device.Model;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.springframework.beans.factory.annotation.Required;

import static org.springframework.dao.support.DataAccessUtils.singleResult;

public abstract class SipxService extends BeanWithSettings implements Model, ServiceLifeCycle {
    private static final Log LOG = LogFactory.getLog(SipxService.class);
    private static final String NO_LOCATION_FOUND_ERROR = "No location found for service: ";

    private String m_processName;
    private String m_beanId;
    private String m_modelName;
    private String m_modelDir;
    private List< ? extends ConfigurationFile> m_configurations;
    private List<SipxService> m_affectedServices;
    private String m_sipPort;
    private String m_logDir;
    private String m_confDir;
    private DomainManager m_domainManager;
    private LocationsManager m_locationsManager;
    private Set<SipxServiceBundle> m_bundles;
    private boolean m_restartable = true;
    private SipxServiceManager m_serviceManager;
    private LoggingManager m_loggingManager;

    /** If true sipXconfig will display default configuration page */
    private boolean m_editable;

    /** If true sipXconfig will enable group title on config page */
    private boolean m_groupTitleEnabled;

    private String m_hiddenSettings;

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

    public void setBeanName(String name) {
        setBeanId(name);
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

    @Required
    public void setSipxServiceManager(SipxServiceManager manager) {
        m_serviceManager = manager;
    }

    @Required
    public void setLocationsManager(LocationsManager locationsManager) {
        m_locationsManager = locationsManager;
    }

    public SipxServiceManager getSipxServiceManager() {
        return m_serviceManager;
    }

    public void setConfigurations(List< ? extends ConfigurationFile> configurations) {
        m_configurations = configurations;
    }

    public List< ? extends ConfigurationFile> getConfigurations() {
        if (m_configurations == null) {
            return Collections.EMPTY_LIST;
        }

        return m_configurations;
    }

    public void setAffectedServices(List<SipxService> affectedServices) {
        m_affectedServices = affectedServices;
    }

    public void appendAffectedServices(Collection<SipxService> services) {
        if (m_affectedServices != null) {
            services.addAll(m_affectedServices);
        }
    }

    /**
     * Returns a subset of configuration files for a service.
     *
     * @param safeOnly if true return only configurations that do not require restart
     * @return collection of configuration files
     */
    public List< ? extends ConfigurationFile> getConfigurations(boolean safeOnly) {
        List< ? extends ConfigurationFile> all = getConfigurations();
        if (!safeOnly) {
            return all;
        }
        List<ConfigurationFile> safe = new ArrayList<ConfigurationFile>();
        for (ConfigurationFile configurationFile : all) {
            if (!configurationFile.isRestartRequired()) {
                safe.add(configurationFile);
            }
        }
        return safe;
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

    public LocationsManager getLocationsManager() {
        return m_locationsManager;
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

    public void setEditable(boolean editable) {
        m_editable = editable;
    }

    public boolean isEditable() {
        return m_editable;
    }

    public void setGroupTitleEnabled(boolean groupTitleEnabled) {
        m_groupTitleEnabled = groupTitleEnabled;
    }

    public boolean isGroupTitleEnabled() {
        return m_groupTitleEnabled;
    }

    public void setHiddenSettings(String settings) {
        m_hiddenSettings = settings;
    }

    public String getHiddenSettings() {
        return m_hiddenSettings;
    }

    /**
     * @return list of addresses for all servers on which this service is installed
     */
    public List<String> getAddresses() {
        List<Location> locations = m_locationsManager.getLocationsForService(this);
        List<String> addresses = new ArrayList<String>();
        for (Location location : locations) {
            addresses.add(location.getAddress());
        }
        return addresses;
    }

    /**
     * This only works for services that have "one per cluster" restrictions. Exception is thrown
     * otherwise.
     *
     * It returns empty String if non of the locations have this service installed.
     *
     * @return single address of the server on which this service is installed
     */
    public String getAddress() {
        String address = (String) singleResult(getAddresses());
        if (address == null) {
            LOG.error(NO_LOCATION_FOUND_ERROR + getBeanId());
            // HACK: probably better to return null here
            address = StringUtils.EMPTY;
        }
        return address;
    }

    /**
     * This only works for services that have "one per cluster" restrictions. Exception is thrown
     * otherwise.
     *
     * It returns null if non of the locations have this service installed.
     *
     * @return single FQDN of the server on which this service is installed
     */
    public String getFqdn() {
        List<Location> locations = m_locationsManager.getLocationsForService(this);
        if (locations.isEmpty()) {
            LOG.error(NO_LOCATION_FOUND_ERROR + m_beanId);
            return null;
        }
        if (locations.size() > 1) {
            LOG.warn("Returning first location of the service running on multiple servers: " + m_beanId);
        }
        return locations.get(0).getFqdn();
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

    /**
     * Get bundles for this service installed on a given location
     *
     * @param location affected location
     * @return collection of bundles
     */
    public List<SipxServiceBundle> getBundles(Location location) {
        Collection<SipxServiceBundle> all = getBundles();
        List<SipxServiceBundle> installed = new ArrayList<SipxServiceBundle>();
        for (SipxServiceBundle bundle : all) {
            if (location.isBundleInstalled(bundle.getModelId())) {
                installed.add(bundle);
            }
        }
        return installed;
    }

    public boolean inBundle(SipxServiceBundle bundle) {
        return m_bundles != null && m_bundles.contains(bundle);
    }

    public void setLoggingManager(LoggingManager loggingManager) {
        m_loggingManager = loggingManager;
    }

    /**
     * Override this method to perform validation
     */
    public void validate() {
        // no validation is done by default
    }

    /**
     * Override this method if a warning notification should be prompted
     */
    public String getWarningKey() {
        return null;
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

    /**
     * Checks if service should be automatically started/enabled.
     *
     * Service is auto-enabled if any of the bundled to which it belongs is auto-enabled
     */
    public boolean isAutoEnabled() {
        if (m_bundles == null) {
            return false;
        }
        for (SipxServiceBundle bundle : m_bundles) {
            if (bundle.isAutoEnable()) {
                return true;
            }
        }
        return false;
    }

    /**
     * Should be overridden in specific service subclass if that specific subclass implements
     * LoggingEntity interface and it should return the path to the logging setting.
     */
    public String getLogSetting() {
        return null;
    }

    /**
     * Sets the log level of this service to the specified log level. This method will only return
     * the correct value if the concrete subclass provides an implementation of getLogSetting().
     *
     * Calling this method will cause this service object to be persisted to the
     * SipxServiceManager object
     */
    protected void setLogLevel(String logLevel) {
        if (logLevel != null && getLogSetting() != null) {
            if (!logLevel.equals(this.getSettingValue(getLogSetting()))) {
                m_loggingManager.getEntitiesToProcess().add((LoggingEntity) this);
                setSettingValue(getLogSetting(), logLevel);
                getSipxServiceManager().storeService(this);
            }
        }
    }

    /**
     * Get the log level of this service. This method will only return the correct value if the
     * concrete subclass provides an implementation of getLogSetting()
     */
    protected String getLogLevel() {
        if (getLogSetting() != null) {
            return getSettingValue(getLogSetting());
        }

        return null;
    }

    protected String getLabelKey() {
        return "label." + getBeanId();
    }

    public Object getParam(@SuppressWarnings("unused") String paramName) {
        return null;
    }

    public void onInit() {
        // intentionally empty
    }

    public void onDestroy() {
        // intentionally empty
    }

    public void onStart() {
        // intentionally empty
    }

    public void onStop() {
        // intentionally empty
    }

    public void onRestart() {
        // intentionally empty
    }

    public void afterReplication(Location location) {
        // intentionally empty
    }

    public void onConfigChange() {
        // intentionally empty
    }

    public boolean isAvailable() {
        return true;
    }
}
