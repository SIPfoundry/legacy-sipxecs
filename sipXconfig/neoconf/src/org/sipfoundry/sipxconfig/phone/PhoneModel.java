/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone;


import org.sipfoundry.sipxconfig.device.DeviceDescriptor;

public class PhoneModel extends DeviceDescriptor {

    private static final int DEFAULT_MAX_LINES = 4;

    private int m_maxLineCount = DEFAULT_MAX_LINES;

    private boolean m_externalLinesSupported = true;

    private boolean m_isEmergencyConfigurable;

    private String m_settingsFile = "phone.xml";

    private String m_lineSettingsFile = "line.xml";

    public PhoneModel() {
    }

    public PhoneModel(String beanId) {
        super(beanId);
    }

    public PhoneModel(String beanId, String modelId) {
        super(beanId, modelId);
    }

    public void setMaxLineCount(int maxLineCount) {
        m_maxLineCount = maxLineCount;
    }

    public int getMaxLineCount() {
        return m_maxLineCount;
    }

    public void setExternalLinesSupported(boolean externalLinesSupported) {
        m_externalLinesSupported = externalLinesSupported;
    }

    /**
     * Used to test if phone supports external lines.
     *
     * External line is a line that registers with a different SIP proxy than out default proxy.
     * Some phones support multiple lines but all of the have to be registered with the same
     * proxy. This flag needs to be switched to falso for such phones.
     *
     */
    public boolean isExternalLinesSupported() {
        return m_externalLinesSupported;
    }

    /**
     * Will device configure itself with 911-like settings by default if dial plans are configured
     * appropriately as described in emergency dial plan configuration web page.
     */
    public boolean isEmergencyConfigurable() {
        return m_isEmergencyConfigurable;
    }

    public void setEmergencyConfigurable(boolean isEmergencyConfigurable) {
        m_isEmergencyConfigurable = isEmergencyConfigurable;
    }

    public String getSettingsFile() {
        return m_settingsFile;
    }

    public void setSettingsFile(String settingsFile) {
        m_settingsFile = settingsFile;
    }

    public String getLineSettingsFile() {
        return m_lineSettingsFile;
    }

    public void setLineSettingsFile(String lineSettingsFile) {
        m_lineSettingsFile = lineSettingsFile;
    }
}
