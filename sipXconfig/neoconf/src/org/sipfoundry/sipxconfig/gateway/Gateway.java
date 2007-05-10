/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.gateway;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.device.Device;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.device.ProfileGenerator;
import org.sipfoundry.sipxconfig.device.ProfileLocation;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingExpressionEvaluator;
import org.sipfoundry.sipxconfig.setting.SimpleDefinitionsEvaluator;

/**
 * Gateway
 */
public class Gateway extends BeanWithSettings implements NamedObject, Device {
    private String m_name;

    private String m_address;

    private String m_prefix;

    private String m_description;

    private String m_beanId;

    private String m_modelId;

    private String m_serialNumber;

    private GatewayModel m_model;

    private ModelSource<GatewayModel> m_modelSource;

    private DeviceVersion m_version;

    private ProfileGenerator m_profileGenerator;

    private GatewayCallerAliasInfo m_callerAliasInfo = new GatewayCallerAliasInfo();

    private DeviceDefaults m_defaults;

    private List<FxoPort> m_ports = new ArrayList<FxoPort>();

    public Gateway() {
    }

    public Gateway(GatewayModel model) {
        setModel(model);
    }

    @Override
    public void initialize() {
    }

    @SuppressWarnings("unused")
    public void initializePort(FxoPort port) {
    }

    /**
     * Default implementation generates a single profile file if profile file name and profile
     * template are provided
     * 
     */
    public void generateProfiles(ProfileLocation location) {
        String profileFileName = getProfileFilename();
        ProfileContext context = createContext();
        m_profileGenerator.generate(location, context, profileFileName);
    }

    protected ProfileContext createContext() {
        return new ProfileContext(this, getProfileTemplate());
    }

    public void removeProfiles(ProfileLocation location) {
        location.removeProfile(getProfileFilename());
    }

    protected String getProfileTemplate() {
        return null;
    }

    protected String getProfileFilename() {
        return null;
    }

    public DeviceVersion getDeviceVersion() {
        return m_version;
    }

    public void setDeviceVersion(DeviceVersion version) {
        m_version = version;
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public String getAddress() {
        return m_address;
    }

    public void setAddress(String address) {
        m_address = address;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
    }

    public String getSerialNumber() {
        return m_serialNumber;
    }

    public void setSerialNumber(String serialNumber) {
        m_serialNumber = serialNumber;
    }

    public List<FxoPort> getPorts() {
        return m_ports;
    }

    public void setPorts(List<FxoPort> ports) {
        m_ports = ports;
    }

    public void setProfileGenerator(ProfileGenerator profileGenerator) {
        m_profileGenerator = profileGenerator;
    }

    protected ProfileGenerator getProfileGenerator() {
        return m_profileGenerator;
    }

    public String getBeanId() {
        return m_beanId;
    }

    public void setBeanId(String beanId) {
        m_beanId = beanId;
    }

    public void setModelId(String modelId) {
        m_modelId = modelId;
    }

    public void setModel(GatewayModel model) {
        m_model = model;
        m_modelId = m_model.getModelId();
    }

    public GatewayModel getModel() {
        if (m_model != null) {
            return m_model;
        }
        if (m_modelId == null) {
            throw new IllegalStateException("Model ID not set");
        }
        if (m_modelSource == null) {
            throw new IllegalStateException("ModelSource not set");
        }
        m_model = m_modelSource.getModel(m_modelId);
        return m_model;
    }

    public String getModelId() {
        return m_modelId;
    }

    public String getPrefix() {
        return m_prefix;
    }

    public void setPrefix(String prefix) {
        m_prefix = prefix;
    }

    /**
     * Used to set header parameter route in fallback rules when generating rules for this
     * gateway.
     */
    public String getRoute() {
        return null;
    }

    public GatewayCallerAliasInfo getCallerAliasInfo() {
        return m_callerAliasInfo;
    }

    public void setCallerAliasInfo(GatewayCallerAliasInfo callerAliasInfo) {
        m_callerAliasInfo = callerAliasInfo;
    }

    @Override
    protected Setting loadSettings() {
        return null;
    }

    public Setting loadPortSettings() {
        return null;
    }

    /**
     * Prepends gateway specific call pattern to call pattern.
     */
    public String getCallPattern(String callPattern) {
        if (StringUtils.isEmpty(m_prefix)) {
            return callPattern;
        }
        return m_prefix + callPattern;
    }

    protected Object clone() throws CloneNotSupportedException {
        Gateway clone = (Gateway) super.clone();
        clone.m_callerAliasInfo = (GatewayCallerAliasInfo) m_callerAliasInfo.clone();
        return clone;
    }

    public void setGatewayModelSource(ModelSource<GatewayModel> modelSource) {
        m_modelSource = modelSource;
    }

    protected Set getModelDefinitions() {
        Set definitions = getModel().getDefinitions();
        if (getDeviceVersion() != null) {
            definitions.add(getDeviceVersion().getVersionId());
        }
        return definitions;
    }

    /**
     * If SimepleDefinitionEvaluator is not what you need override this function to provide your
     * own expression evaluator to be used when loading settings model.
     * 
     * @return reusable copy of expression evaluator
     */
    protected SettingExpressionEvaluator getSettingsEvaluator() {
        Set defines = getModelDefinitions();
        return new SimpleDefinitionsEvaluator(defines);
    }

    public void setDefaults(DeviceDefaults defaults) {
        m_defaults = defaults;
    }

    public DeviceDefaults getDefaults() {
        return m_defaults;
    }

    public void addPort(FxoPort port) {
        List<FxoPort> ports = getPorts();
        int maxPorts = getModel().getMaxPorts();
        if (ports.size() >= maxPorts) {
            throw new MaxPortsException(maxPorts);
        }
        if (ports.add(port)) {
            port.setGateway(this);
        }
    }

    public void removePort(FxoPort port) {
        if (getPorts().remove(port)) {
            port.setGateway(null);
        }
    }

    private static class MaxPortsException extends UserException {
        public MaxPortsException(int max) {
            super("Maximum number of ports is {0}", max);
        }
    }
}
