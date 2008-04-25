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

import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDevice;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

import static org.sipfoundry.sipxconfig.gateway.Gateway.AddressTransport.NONE;
import static org.sipfoundry.sipxconfig.gateway.Gateway.AddressTransport.UDP;

public class SipTrunk extends Gateway {
    public static final String BEAN_ID = "gwSipTrunk";

    private static final int DEFAULT_PORT = 5060;

    public SipTrunk() {
    }

    public SipTrunk(GatewayModel model) {
        super(model);
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("siptrunk.xml", "commserver");
    }

    @Override
    public void initialize() {
        addDefaultBeanSettingHandler(new Defaults(this));
    }

    @Override
    public String getRoute() {
        SbcDevice sbcDevice = getSbcDevice();
        if (sbcDevice != null) {
            StringBuffer route = new StringBuffer(sbcDevice.getAddress());
            if (sbcDevice.getPort() > 0 && sbcDevice.getPort() != DEFAULT_PORT) {
                route.append(':');
                route.append(sbcDevice.getPort());
            }

            return route.toString();
        }
        return null;
    }

    public static class Defaults {
        private final SipTrunk m_trunk;

        public Defaults(SipTrunk trunk) {
            m_trunk = trunk;
        }

        @SettingEntry(path = "itsp-account/outbound-transport")
        public String getOutboundTransport() {
            AddressTransport transport = m_trunk.getAddressTransport();
            if (transport.equals(NONE)) {
                // force default to be UDP - NONE is not supported
                transport = UDP;
            }
            return transport.getName().toUpperCase();
        }

        @SettingEntry(paths = { "itsp-account/outbound-proxy", "itsp-account/proxy-domain" })
        public String getOutboundProxy() {
            return m_trunk.getAddress();
        }

        @SettingEntry(path = "itsp-account/outbound-proxy-port")
        public int getOutboundProxyPort() {
            return m_trunk.getAddressPort();
        }
    }
}
