/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.device;

import java.util.Set;

import org.sipfoundry.sipxconfig.setting.BeanWithGroups;
import org.sipfoundry.sipxconfig.setting.SettingExpressionEvaluator;
import org.sipfoundry.sipxconfig.setting.SimpleDefinitionsEvaluator;

public abstract class Device extends BeanWithGroups {

    private String m_beanId;

    private DeviceDefaults m_defaults;

    private String m_modelId;

    private ProfileGenerator m_profileGenerator;

    private String m_profileTemplate;

    private String m_serialNumber;

    private DeviceVersion m_version;

    protected ProfileContext createContext() {
        return new ProfileContext(this, getProfileTemplate());
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

    protected Set getModelDefinitions() {
        Set definitions = getModel().getDefinitions();
        if (getDeviceVersion() != null) {
            definitions.add(getDeviceVersion().getVersionId());
        }
        return definitions;
    }

    public abstract DeviceDescriptor getModel();

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

    public String getModelLabel() {
        return getModel().getLabel();
    }

    public String getBeanId() {
        return m_beanId;
    }

    public DeviceDefaults getDefaults() {
        return m_defaults;
    }

    public DeviceVersion getDeviceVersion() {
        return m_version;
    }

    public String getModelId() {
        return m_modelId;
    }

    public String getProfileFilename() {
        return null;
    }

    public ProfileGenerator getProfileGenerator() {
        return m_profileGenerator;
    }

    protected String getProfileTemplate() {
        return m_profileTemplate;
    }

    public String getSerialNumber() {
        return m_serialNumber;
    }

    public void removeProfiles(ProfileLocation location) {
        location.removeProfile(getProfileFilename());
    }

    public void setBeanId(String beanId) {
        m_beanId = beanId;
    }

    public void setDefaults(DeviceDefaults defaults) {
        m_defaults = defaults;
    }

    public void setDeviceVersion(DeviceVersion version) {
        m_version = version;
    }

    public void setModelId(String modelId) {
        m_modelId = modelId;
    }

    public void setProfileGenerator(ProfileGenerator profileGenerator) {
        m_profileGenerator = profileGenerator;
    }

    public void setProfileTemplate(String phoneTemplate) {
        m_profileTemplate = phoneTemplate;
    }

    public void setSerialNumber(String serialNumber) {
        m_serialNumber = serialNumber;
    }
}
