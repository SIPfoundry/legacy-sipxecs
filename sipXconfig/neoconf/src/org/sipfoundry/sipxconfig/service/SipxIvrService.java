/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.service;

import java.io.File;

import org.springframework.beans.factory.annotation.Required;

public class SipxIvrService extends SipxService implements LoggingEntity {
    public static final String BEAN_ID = "sipxIvrService";

    public static final String LOG_SETTING = "ivr/log.level";
    public static final String HTTPS_PORT = "ivr/httpsPort";
    public static final String NAME_DIAL_PFX = "ivr/nameDialPrefix";
    public static final String DEFAULT_TUI = "ivr/defaultTui";

    private String m_vxmlDir;
    private String m_mailstoreDir;
    private String m_promptsDir;
    private String m_scriptsDir;
    private String m_docDir;
    private String m_cpuiDir;
    private String m_callPilotSettings;


    @Required
    public void setMailstoreDir(String mailstoreDirectory) {
        m_mailstoreDir = mailstoreDirectory;
    }

    public String getMailstoreDir() {
        return m_mailstoreDir;
    }

    @Required
    public void setPromptsDir(String promptsDirectory) {
        m_promptsDir = promptsDirectory;
    }

    public String getPromptsDir() {
        return m_promptsDir;
    }

    @Required
    public void setVxmlDir(String vxmlDirectory) {
        m_vxmlDir = vxmlDirectory;
    }

    public String getVxmlDir() {
        return m_vxmlDir;
    }

    @Required
    public void setScriptsDir(String scriptsDirectory) {
        m_scriptsDir = scriptsDirectory;
    }

    public String getScriptsDir() {
        return m_scriptsDir;
    }

    @Required
    public void setDocDir(String docDirectory) {
        m_docDir = docDirectory;
    }

    public String getDocDir() {
        return m_docDir;
    }

    public void setCpuiDir(String cpuiDirectory) {
        m_cpuiDir = cpuiDirectory;

        // If the cpui directory does not exist, it implies that CallPilot service
        // is not enabled, so hide the CallPilot related settings
        if (!((new File(m_cpuiDir)).exists())) {
            setHiddenSettings(getCallPilotSettings());
        }

    }

    //settings are delimitted by ",". i.e. "<setting1>, <setting2>, ..."
    public String getCallPilotSettings() {
        return m_callPilotSettings;
    }

    public void setCallPilotSettings(String callPilotSettings) {
        m_callPilotSettings = callPilotSettings;
    }

    @Override
    public String getLogSetting() {
        return LOG_SETTING;
    }

    @Override
    public void setLogLevel(String logLevel) {
        super.setLogLevel(logLevel);
    }

    @Override
    public String getLogLevel() {
        return super.getLogLevel();
    }

    @Override
    public String getLabelKey() {
        return super.getLabelKey();
    }

    public String getHttpsPort() {
        return getSettingValue(HTTPS_PORT);
    }

    public String getNameDialPrefix() {
        return getSettingValue(NAME_DIAL_PFX);
    }

    public String getDefaultTui() {
        return getSettingValue(DEFAULT_TUI);
    }
}
