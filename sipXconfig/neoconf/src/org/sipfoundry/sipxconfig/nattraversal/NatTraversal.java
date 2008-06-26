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

import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class NatTraversal extends BeanWithSettings {
    public static final String BEAN_NAME = "natTraversal";
    public static final String INFO_AGGRESSIVENESS = "nattraversal-info/aggressiveness";
    public static final String INFO_MAXCONCRELAYS = "nattraversal-info/max-concurrent-relays";
    public static final String INFO_PUBLICADDRESS = "nattraversal-info/publicaddress";
    public static final String INFO_STUNSERVER = "nattraversal-info/stun-server-address";
    public static final String INFO_REFRESHINTERVAL = "nattraversal-info/stun-refresh-interval";

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

    public Setting getInfoPublicAddress() {
        return getSettings().getSetting(INFO_PUBLICADDRESS);
    }

    public Setting getInfoAggressiveness() {
        return getSettings().getSetting(INFO_AGGRESSIVENESS);
    }

    public Setting getInfoMaxConcRelays() {
        return getSettings().getSetting(INFO_MAXCONCRELAYS);
    }

    public Setting getInfoSTUNServer() {
        return getSettings().getSetting(INFO_STUNSERVER);
    }

    public Setting getInfoSTUNRefreshInterval() {
        return getSettings().getSetting(INFO_REFRESHINTERVAL);
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
