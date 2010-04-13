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

import java.io.File;
import java.util.Set;

import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.setting.BeanWithGroups;
import org.sipfoundry.sipxconfig.setting.SettingExpressionEvaluator;
import org.sipfoundry.sipxconfig.setting.SimpleDefinitionsEvaluator;

import static org.apache.commons.lang.StringUtils.isNotBlank;

public abstract class Device extends BeanWithGroups {

    private String m_beanId;

    private DeviceDefaults m_defaults;

    private String m_modelId;

    private ProfileGenerator m_profileGenerator;

    private String m_serialNumber;

    private DeviceVersion m_version;

    private Branch m_branch;

    protected ProfileContext createContext() {
        return new ProfileContext(this, getModel().getProfileTemplate());
    }

    /**
     * Default implementation generates a single profile file if profile file name is provided.
     */
    public void generateProfiles(ProfileLocation location) {
        beforeProfileGeneration();
        copyFiles(location);
        generateFiles(location);
    }

    /**
     * Method called before profiles are generated. Gives device a chance to modify its settings.
     */
    protected void beforeProfileGeneration() {
        return;
    }

    /**
     * Gets list of files to be copies and used profileGenerator to copy them over to provisioning
     * location
     *
     * @param location profile location
     */
    protected void copyFiles(ProfileLocation location) {
        String[] names = getModel().getStaticProfileNames();
        String dir = getModel().getModelDir();
        for (String name : names) {
            String sourceName = dir + File.separator + name;
            m_profileGenerator.copy(location, sourceName, name);
        }
    }

    /**
     * Default implementation gets the list of profile types and calls generate for each of those
     * types
     *
     * @param location profile location
     */
    public void generateFiles(ProfileLocation location) {
        Profile[] profileTypes = getProfileTypes();
        if (profileTypes == null) {
            return;
        }
        for (Profile profile : profileTypes) {
            profile.generate(this, location);
        }
    }

    /**
     * By default no filter will be used.
     */
    protected ProfileFilter getProfileFilter() {
        return null;
    }

    /**
     * Delegate to device model to setup a set of definitions.
     *
     * Definitions can be used in settings descriptor "if" and "unless" statements.
     */
    public Set getModelDefinitions() {
        return getModel().getDefinitions(getDeviceVersion());
    }

    /**
     * Return the list of profile types for this device is generating.
     *
     * The intention is to use this method to check if there are any profiles that can be
     * previewed. It's OK to return null or just a subset of profiles here.
     *
     * @return list
     */
    public Profile[] getProfileTypes() {
        String profileFilename = getProfileFilename();
        if (profileFilename == null) {
            return null;
        }
        return new Profile[] {
            new Profile(profileFilename)
        };
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

    public Branch getBranch() {
        return m_branch;
    }

    public ProfileGenerator getProfileGenerator() {
        return m_profileGenerator;
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

    public void setSerialNumber(String serialNumber) {
        m_serialNumber = serialNumber;
    }

    public void setBranch(Branch branch) {
        m_branch = branch;
    }

    public void restart() {
        // If no action is required for a restart then do nothing
    }

    public String getNiceName() {
        StringBuilder jn = new StringBuilder();
        if (this instanceof NamedObject) {
            NamedObject namedDevice = (NamedObject) this;
            String name = namedDevice.getName();
            if (isNotBlank(name)) {
                jn.append(name);
                jn.append("/");
            }
        }
        String serialNumber = getSerialNumber();
        if (isNotBlank(serialNumber)) {
            jn.append(serialNumber);
        } else {
            jn.append(getModel().getLabel());
        }
        return jn.toString();
    }

    public ProfileLocation getProfileLocation() {
        return getModel().getDefaultProfileLocation();
    }
}
