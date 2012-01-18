/*
 * Copyright (C) 2011 eZuce Inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the AGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.nattraversal;

import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class NatSettings extends PersistableSettings {
    private static final String BEHIND_NAT = "nat/behind-nat";

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("nattraversal/nattraversal.xml");
    }

    public int getXmlRpcPort() {
        return (Integer) getSettingTypedValue("relay-config/xml-rpc-port");
    }

    public boolean isBehindNat() {
        return (Boolean) getSettingTypedValue(BEHIND_NAT);
    }

    public void setBehindNat(boolean behind) {
        setSettingTypedValue(BEHIND_NAT, behind);
    }

    @Override
    public String getBeanId() {
        return "natSettings";
    }
}
