/**
 *
 *
 * Copyright (c) 2014 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.dialplan.attendant;

import java.util.Arrays;
import java.util.Collection;

import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.dialplan.AutoAttendantManager;
import org.sipfoundry.sipxconfig.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.setting.PersistableSettings;
import org.sipfoundry.sipxconfig.setting.Setting;

public class AutoAttendantSettings extends PersistableSettings implements DeployConfigOnEdit {
    private static final String LIVE_DID = "liveAttendant/did";
    private static final String ENABLE_PREFIX = "liveAttendant/enablePrefix";
    private static final String DISABLE_PREFIX = "liveAttendant/disablePrefix";
    private static final String EXPIRE_TIME = "liveAttendant/expireTime";

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Arrays.asList((Feature) DialPlanContext.FEATURE, (Feature) AutoAttendantManager.FEATURE);
    }

    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("sipxvxml/global_aa.xml");
    }

    public String getLiveDid() {
        return getSettingValue(LIVE_DID);
    }

    public String getEnablePrefix() {
        return getSettingValue(ENABLE_PREFIX);
    }

    public String getDisablePrefix() {
        return getSettingValue(DISABLE_PREFIX);
    }

    public Integer getExpireTime() {
        return (Integer) getSettingTypedValue(EXPIRE_TIME);
    }

    @Override
    public String getBeanId() {
        return "aaSettings";
    }
}
