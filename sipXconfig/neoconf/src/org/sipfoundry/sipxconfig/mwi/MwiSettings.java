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
package org.sipfoundry.sipxconfig.mwi;

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class MwiSettings extends PersistableSettings implements DeployConfigOnEdit {

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxstatus/sipxstatus.xml");
    }

    public int getTcp() {
        return ((Integer) getSettingTypedValue("status-config/SIP_STATUS_TCP_PORT")).intValue();
    }

    public int getUdpPort() {
        return ((Integer) getSettingTypedValue("status-config/SIP_STATUS_UDP_PORT")).intValue();
    }

    public int getHttpApiPort() {
        return ((Integer) getSettingTypedValue("status-config/SIP_STATUS_HTTP_PORT")).intValue();
    }

    public int getHttpsApiPort() {
        return ((Integer) getSettingTypedValue("status-config/SIP_STATUS_HTTPS_PORT")).intValue();
    }

    @Override
    public String getBeanId() {
        return "mwiSettings";
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) Mwi.FEATURE);
    }
}
