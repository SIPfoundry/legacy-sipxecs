/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.gateway.acme;

import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class AcmeGateway extends Gateway {

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("acme-gateway.xml", getModel().getModelDir());
    }

    @Override
    public void initialize() {
        addDefaultBeanSettingHandler(new AcmeDefaults(getDefaults()));
    }

    public static class AcmeDefaults {
        private DeviceDefaults m_defaults;

        AcmeDefaults(DeviceDefaults defaults) {
            m_defaults = defaults;
        }

        @SettingEntry(path = "basic/proxyAddress")
        public String getProxyAddress() {
            return m_defaults.getProxyServerAddr();
        }

    }

    @Override
    public String getProfileFilename() {
        return getSerialNumber() + ".ini";
    }
}
