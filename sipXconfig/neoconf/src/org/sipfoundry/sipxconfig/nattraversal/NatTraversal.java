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

import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.bridge.BridgeSbc;
import org.sipfoundry.sipxconfig.setting.BeanWithGroups;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class NatTraversal extends BeanWithGroups {

    public static final String RTP_PORT_START = "nattraversal-symmitron/port-range-start";
    public static final String RTP_PORT_END = "nattraversal-symmitron/port-range-end";

    public static final String BEAN_NAME = "natTraversal";
    private String m_settingsFile;
    private boolean m_enabled;
    private boolean m_behindnat;
    private String m_logDirectory;
    private SbcDeviceManager m_sbcDeviceManager;
    private String m_proxyAddress;
    private String m_proxyServerSipHostport;

    public NatTraversal() {
        super();
        m_settingsFile = "nattraversal.xml";
    }

    protected Setting loadSettings() {
        Setting settings = getModelFilesContext().loadModelFile(m_settingsFile, "nattraversal");
        return settings;
    }

    @Override
    public void initialize() {
        addDefaultBeanSettingHandler(new Defaults(this));
    }

    public void setSbcDeviceManager(SbcDeviceManager sbcDeviceManager) {
        m_sbcDeviceManager = sbcDeviceManager;
    }

    public SbcDeviceManager getSbcDeviceManager() {
        return m_sbcDeviceManager;
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

    public static class Defaults {
        private NatTraversal m_natTraversal;

        Defaults(NatTraversal natTraversal) {
            m_natTraversal = natTraversal;
        }

        @SettingEntry(path = "nattraversal-symmitron/port-range")
        public String getRtpPortRange() {
            String start = m_natTraversal.getSettingValue(RTP_PORT_START);
            String end = m_natTraversal.getSettingValue(RTP_PORT_END);
            return String.format("%s:%s", start, end);
        }

        @SettingEntry(path = "nattraversal-bridge/mediarelayexternaladdress")
        public String getSymmitronLocalAddress() {
            //Don't make sbc bridge a member in NatTraversal because you may end up having two bridge
            //references in the hibernate session when sbc bridge makes port range validation for instance
            BridgeSbc bridge = m_natTraversal.getSbcDeviceManager().getBridgeSbc();
            return bridge != null ? bridge.getSettingValue("bridge-configuration/external-address")
                    : m_natTraversal.getProxyAddress();
        }

        @SettingEntry(path = "nattraversal-bridge/mediarelaynativeaddress")
        public String getSymmitronExternalAddress() {
            //Don't make sbc bridge a member in NatTraversal because you may end up having two bridge
            //references in the hibernate session when sbc bridge makes port range validation for instance
            BridgeSbc bridge = m_natTraversal.getSbcDeviceManager().getBridgeSbc();
            return bridge != null ? bridge.getSettingValue("bridge-configuration/local-address")
                    : m_natTraversal.getProxyAddress();
        }

        @SettingEntry(path = "nattraversal-symmitron/log-directory")
        public String getLogDirectory() {
            return m_natTraversal.getLogDirectory();
        }

    }

    public void setLogDirectory(String logDirectory) {
        m_logDirectory = logDirectory;
    }

    public String getLogDirectory() {
        return m_logDirectory;
    }
    public String getProxyAddress() {
        return m_proxyAddress;
    }

    public void setProxyAddress(String proxyAddress) {
        m_proxyAddress = proxyAddress;
    }

    public String getProxyServerSipHostport() {
        return m_proxyServerSipHostport;
    }

    public void setProxyServerSipHostport(String proxyServerSipHostport) {
        m_proxyServerSipHostport = proxyServerSipHostport;
    }



}
