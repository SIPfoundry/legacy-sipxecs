/**
 *
 *
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.nattraversal;

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class NatSettings extends PersistableSettings implements DeployConfigOnEdit {
    public static final int START_RTP_PORT = 30000;
    public static final int END_RTP_PORT = 31000;
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

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) NatTraversal.FEATURE);
    }
}
