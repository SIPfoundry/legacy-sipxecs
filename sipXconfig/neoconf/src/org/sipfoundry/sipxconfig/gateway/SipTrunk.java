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

    public static final int DEFAULT_PORT = 5060;
    private static final String DEFAULT_TEMPLATE = "siptrunk.xml";
    private static final String DEFAULT_LOCATION = "commserver";

    public SipTrunk() {
    }

    public SipTrunk(GatewayModel model) {
        super(model);
    }

    @Override
    protected Setting loadSettings() {
        String template = DEFAULT_TEMPLATE;
        String location = DEFAULT_LOCATION;
        GatewayModel model = getModelId() != null ? getModel() : null;

        if (model instanceof SipTrunkModel) {
            template = ((SipTrunkModel) model).getItspTemplate();
            location = ((SipTrunkModel) model).getTemplateLocation();
        }
        return getModelFilesContext().loadModelFile(template, location);
    }

    @Override
    public void initialize() {
        GatewayModel model = getModelId() != null ? getModel() : null;

        if ((model == null)
                || (model instanceof SipTrunkModel
                && ((SipTrunkModel) model).getItspName().equals(SipTrunkModel.TEMPLATE_NONE))) {
            addDefaultBeanSettingHandler(new Defaults(this));
        }
    }

    @Override
    public String getRoute() {
        SbcDevice sbcDevice = getSbcDevice();
        if (sbcDevice != null) {
            return sbcDevice.getRoute();
        }
        return null;
    }

    public static class Defaults {
        private final SipTrunk m_trunk;

        public Defaults(SipTrunk trunk) {
            m_trunk = trunk;
        }

        @SettingEntry(path = "itsp-account/itsp-transport")
        public String getItspTransport() {
            AddressTransport transport = m_trunk.getAddressTransport();
            if (transport.equals(NONE)) {
                // force default to be UDP - NONE is not supported
                transport = UDP;
            }
            return transport.getName().toUpperCase();
        }

        @SettingEntry(paths = { "itsp-account/itsp-proxy-address", "itsp-account/itsp-proxy-domain" })
        public String getItspProxy() {
            return m_trunk.getAddress();
        }

        @SettingEntry(path = "itsp-account/itsp-proxy-listening-port")
        public int getItspProxyListeningPort() {
            return m_trunk.getAddressPort();
        }
    }
}
