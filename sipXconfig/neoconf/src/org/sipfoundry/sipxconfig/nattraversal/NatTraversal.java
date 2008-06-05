/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.nattraversal;

import org.sipfoundry.sipxconfig.setting.AbstractSetting;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class NatTraversal extends BeanWithSettings {
    public static final String BEAN_NAME = "natTraversal";
    public static final String INFO_AGGRESSIVENESS = "nattraversal-info/aggressiveness";
    public static final String INFO_MAXCONCRELAYS = "nattraversal-info/max-concurrent-relays";
    public static final String INFO_PUBLICADDRESS = "nattraversal-info/publicaddress";

    private String m_settingsFile;
    private boolean m_enabled;
    private boolean m_behindnat;

    public NatTraversal() {
        super();
        m_settingsFile = "nattraversal.xml";
    }

    protected Setting loadSettings() {
        Setting settings = getModelFilesContext().loadModelFile(m_settingsFile, "nattraversal");
        return settings;
    }

    public AbstractSetting getInfoPublicAddress() {
        return (AbstractSetting) getSettings().getSetting(INFO_PUBLICADDRESS);
    }

    public AbstractSetting getInfoAggressiveness() {
        return (AbstractSetting) getSettings().getSetting(INFO_AGGRESSIVENESS);
    }

    public AbstractSetting getInfoMaxConcRelays() {
        return (AbstractSetting) getSettings().getSetting(INFO_MAXCONCRELAYS);
    }

    public boolean isBehindnat() {
        return m_behindnat;
    }

    public void setBehindnat(boolean behindnat) {
        m_behindnat = behindnat;
    }

    public boolean isEnabled() {
        return m_enabled;
    }

    public void setEnabled(boolean enabled) {
        m_enabled = enabled;
    }

}
