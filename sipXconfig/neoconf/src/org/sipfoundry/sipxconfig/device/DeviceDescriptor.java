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

import java.util.HashSet;
import java.util.Set;

import org.apache.commons.lang.StringUtils;

public abstract class DeviceDescriptor {
    private String m_modelFilePath;

    private String m_beanId;

    private String m_label;

    private String m_modelId;

    /**
     * By default we accept MAC address as serial number. Device plugin developers can define
     * other regular expressions to accept serial number in a format specific for a device.
     */
    private String m_serialNumberPattern = "^[a-f\\d]{12}$";

    private DeviceVersion[] m_versions = new DeviceVersion[0];

    public DeviceDescriptor() {
    }

    public DeviceDescriptor(String beanId) {
        setBeanId(beanId);
    }

    public DeviceDescriptor(String beanId, String modelId) {
        this(beanId);
        setModelId(modelId);
    }

    public void setBeanId(String beanId) {
        m_beanId = beanId;
    }

    public void setLabel(String label) {
        m_label = label;
    }

    public void setModelId(String modelId) {
        m_modelId = modelId;
    }

    /**
     * Spring bean name, NOTE: You cannot reuse java class to multiple spring beans. One
     * class/bean but a spring bean can handle multiple models
     */
    public String getBeanId() {
        return m_beanId;
    }

    /**
     * User identifiable label for this model
     */
    public String getLabel() {
        return m_label;
    }

    public String getModelId() {
        return m_modelId;
    }

    /**
     * If non-empty pattern is specified than assume serial number is needed.
     */
    public boolean getHasSerialNumber() {
        return StringUtils.isNotBlank(getSerialNumberPattern());
    }

    protected void setVersions(DeviceVersion[] versions) {
        m_versions = versions;
    }

    public DeviceVersion[] getVersions() {
        return m_versions;
    }

    public String getSerialNumberPattern() {
        return m_serialNumberPattern;
    }

    public void setSerialNumberPattern(String serialNumberPattern) {
        m_serialNumberPattern = serialNumberPattern;
    }

    /**
     * This function is called to transform serial number entered by the user to the format that
     * is accepted by sipXconfig. It is called before the serial number is verified against
     * regular expression pattern.
     * 
     * @param raw serial number as entered by the user or
     * @return cleaned serial number
     */
    public String cleanSerialNumber(String raw) {
        if (raw == null) {
            return null;
        }
        String clean = raw.toLowerCase();
        // remove spaces, dashes and colons
        return clean.replaceAll("[-:\\s]*", "");
    }

    /**
     * @return File name with upload settings describing files to upload. Relative to /etc/sipxpbx
     */
    public String getModelFilePath() {
        return m_modelFilePath;
    }

    public void setModelFilePath(String modelFilePath) {
        m_modelFilePath = modelFilePath;
    }

    /**
     * When loading the settings model.
     * 
     * Example If you can add "not-extinct" the following setting will not be loaded. Phone model
     * id and version are added by default.
     * 
     * &lt;setting name="dinosaur" unless="not-extinct"/&gt;
     */
    public Set getDefinitions() {
        Set definitions = new HashSet();
        definitions.add(getModelId());
        return definitions;
    }
}
