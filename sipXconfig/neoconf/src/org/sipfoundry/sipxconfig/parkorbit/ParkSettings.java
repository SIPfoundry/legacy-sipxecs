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
package org.sipfoundry.sipxconfig.parkorbit;

import java.util.Arrays;
import java.util.Collection;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class ParkSettings extends PersistableSettings implements DeployConfigOnEdit {

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxpark/sipxpark.xml");
    }

    public int getSipTcpPort() {
        return (Integer) getSettingTypedValue("park-config/SIP_PARK_TCP_PORT");
    }

    public int getSipUdpPort() {
        return (Integer) getSettingTypedValue("park-config/SIP_PARK_UDP_PORT");
    }

    public int getRtpPort() {
        return (Integer) getSettingTypedValue("park-config/SIP_PARK_RTP_PORT");
    }

    public int getMaxSessions() {
        return (Integer) getSettingTypedValue("park-config/SIP_PARK_MAX_SESSIONS");
    }

    @Override
    public String getBeanId() {
        return "parkSettings";
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Arrays.asList((Feature) ParkOrbitContext.FEATURE, (Feature) ProxyManager.FEATURE);
    }
}
