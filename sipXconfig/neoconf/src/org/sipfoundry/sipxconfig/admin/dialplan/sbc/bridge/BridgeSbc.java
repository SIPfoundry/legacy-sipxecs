/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan.sbc.bridge;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDevice;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.gateway.GatewayContext;
import org.sipfoundry.sipxconfig.gateway.SipTrunk;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;
import org.springframework.beans.factory.annotation.Required;

public class BridgeSbc extends SbcDevice {
    private GatewayContext m_gatewayContext;

    @Required
    public void setGatewayContext(GatewayContext gatewayContext) {
        m_gatewayContext = gatewayContext;
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("bridge-sbc.xml", "sipxbridge");
    }

    protected ProfileContext createContext() {
        return new Context(this, "sipxbridge/bridge.xml.vm");
    }

    public String getProfileFilename() {
        return "sipxbridge.xml";
    }

    @Override
    public void initialize() {
        addDefaultBeanSettingHandler(new Defaults(getDefaults(), this));
    }

    public List<SipTrunk> getMySipTrunks() {
        List<SipTrunk> trunks = new ArrayList<SipTrunk>();
        for (SipTrunk t : m_gatewayContext.getGatewayByType(SipTrunk.class)) {
            if (equals(t.getSbcDevice())) {
                trunks.add(t);
            }
        }
        return trunks;
    }

    public static class Context extends ProfileContext<BridgeSbc> {
        public Context(BridgeSbc device, String profileTemplate) {
            super(device, profileTemplate);
        }

        public Map<String, Object> getContext() {
            Map<String, Object> context = super.getContext();
            BridgeSbc device = getDevice();
            context.put("trunks", device.getMySipTrunks());
            return context;
        }
    }

    public static class Defaults {
        private DeviceDefaults m_defaults;
        private SbcDevice m_device;

        Defaults(DeviceDefaults defaults, SbcDevice device) {
            m_defaults = defaults;
            m_device = device;
        }

        @SettingEntry(paths = { "bridge-configuration/local-address", "bridge-configuration/external-address" })
        public String getLocalAddress() {
            return m_device.getAddress();
        }

        @SettingEntry(path = "bridge-configuration/local-port")
        public int getLocalPort() {
            return m_device.getPort();
        }

        @SettingEntry(path = "bridge-configuration/sipx-proxy-domain")
        public String getDomainName() {
            return m_defaults.getDomainName();
        }

        @SettingEntry(path = "bridge-configuration/rtp-port-range")
        public String getRtpPortRange() {
            String start = m_device.getSettingValue("bridge-configuration/rtp-port-range-start");
            String end = m_device.getSettingValue("bridge-configuration/rtp-port-range-end");
            return String.format("%s:%s", start, end);
        }

        @SettingEntry(path = "bridge-configuration/log-directory")
        public String getLogDirectory() {
            return m_defaults.getLogDirectory() + "/";
        }
    }
}
